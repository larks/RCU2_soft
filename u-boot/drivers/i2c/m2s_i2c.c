/*
 * i2c driver for Freescale mx31
 *
 * (c) 2007 Pengutronix, Sascha Hauer <s.hauer@pengutronix.de>
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <common.h>
#include <asm/io.h>


/*
 * I2C controller private data structure          
 */
struct i2c_a2f {
  int                             bus;            /* Bus (ID) */
  unsigned int                    regs_base;      /* Regs base (phys) */
  unsigned int                    regs_size;      /* Regs size */
  int                             irq;            /* IRQ # */
  unsigned int                    ref_clk;        /* Ref clock */
  unsigned int                    i2c_clk;        /* Bus clock */
  struct i2c_msg*                 msg;            /* Current message */
  int                             msg_n;          /* Segments in msg */
  int                             msg_i;          /* Idx in a segment */
  volatile int                    msg_status;     /* Message status */
};

struct i2c_a2f_regs {
  unsigned int                    ctrl;
  unsigned int                    status;
  unsigned int                    data;
  unsigned int                    addr;
  unsigned int                    smbus;
  unsigned int                    freq;
  unsigned int                    glitchreg;
};

struct i2c_msg {
  u16 addr;     /* slave address                        */
  u16 flags;
  u16 len;              /* msg length                           */
  u8 *buf;              /* pointer to msg data                  */
};



/*
 * Access handle for the control registers      
 */
#define I2C_A2F_REGS(r)                 ((volatile struct i2c_a2f_regs *)(r))
#define I2C_A2F(c)                      (I2C_A2F_REGS(c->regs))


/* 
 *  Some bits in various CSRs         
 */
#define I2C_A2F_CTRL_ENS1               (1<<6)
#define I2C_A2F_CTRL_STA                (1<<5)
#define I2C_A2F_CTRL_STO                (1<<4)
#define I2C_A2F_CTRL_SI                 (1<<3)
#define I2C_A2F_CTRL_AA                 (1<<2)
#define I2C_A2F_CTRL_CR0_SHFT           0
#define I2C_A2F_CTRL_CR1_SHFT           1
#define I2C_A2F_CTRL_CR2_SHFT           7


/*
 * I2C status codes applicable to master mode operation     
 */
#define I2C_A2F_STATUS_START            0x08
#define I2C_A2F_STATUS_REPSTART         0x10
#define I2C_A2F_STATUS_ADDR_W_ACK       0x18
#define I2C_A2F_STATUS_ADDR_W_NACK      0x20
#define I2C_A2F_STATUS_ADDR_R_ACK       0x40
#define I2C_A2F_STATUS_ADDR_R_NACK      0x48
#define I2C_A2F_STATUS_DATA_W_ACK       0x28
#define I2C_A2F_STATUS_DATA_W_NACK      0x30
#define I2C_A2F_STATUS_DATA_R_ACK       0x50
#define I2C_A2F_STATUS_DATA_R_NACK      0x58
#define I2C_A2F_STATUS_BUS_ERROR        0x00
#define I2C_A2F_STATUS_ARB_LOST         0x38


void i2c_init(int speed, int unused)
{
  
  static struct {
    unsigned int    d;
    unsigned int    b;
  } div[] =
      {{60,6},{120,5},{160,3},{192,2},{224,1},{256,0},{960,4},{0,0}};

  unsigned int v, b;
  int i;
  int ret = 0;
  if(speed<0 || speed>7){
    ret = -1;
  }


  b = div[speed].b;

  /*
   * Enable the controller  
   */
  v = readl(&I2C_A2F(c)->ctrl);
  writel(v | I2C_A2F_CTRL_ENS1, &I2C_A2F(c)->ctrl);
  
  /*       
   * Set up the clock rate 
   */
  v = readl(&I2C_A2F(c)->ctrl);
  writel(v
	 | (v & ~(1 << I2C_A2F_CTRL_CR0_SHFT))
	 | (((b >> 0) & 0x1) << I2C_A2F_CTRL_CR0_SHFT)
	 | (v & ~(1 << I2C_A2F_CTRL_CR1_SHFT))
	 | (((b >> 1) & 0x1) << I2C_A2F_CTRL_CR2_SHFT)
	 | (v & ~(1 << I2C_A2F_CTRL_CR2_SHFT))
	 | (((b >> 2) & 0x1) << I2C_A2F_CTRL_CR2_SHFT),
	 &I2C_A2F(c)->ctrl);

  /*  
   * Set the address for master-only operation   
   */
  writel(0x0, &I2C_A2F(c)->addr);

  ret  = 0;


  return ret;

}


static void i2c_release(struct i2c_a2f *c)
{

  /*
   * Clear various conditions that affect the controler and the bus 
   */
  writel(readl(&I2C_A2F(c)->ctrl) &
	 ~(I2C_A2F_CTRL_SI | I2C_A2F_CTRL_STA |
	   I2C_A2F_CTRL_STO | I2C_A2F_CTRL_AA),
	 &I2C_A2F(c)->ctrl);

  /*  
   * Disable the controller     
   */
  writel(readl(&I2C_A2F(c)->ctrl) & ~I2C_A2F_CTRL_ENS1,
         &I2C_A2F(c)->ctrl);

}


static int i2c_a2f_transfer(struct i2c_a2f *c, struct i2c_msg *m, int n)
{

  unsigned int ctrl;
  int ret = 0;

  /*
   * Store the software parameters of the message.  
   * There will be used by the IRQ handler.
   */

  c->msg = &m[0];
  c->msg_i = 0;
  c->msg_n = n;
  c->msg_status = -100;


  /* 
   * Clear various conditions that affect the controler and the bus                                   
   */
  writel(readl(&I2C_A2F(c)->ctrl) &
         ~(I2C_A2F_CTRL_SI | I2C_A2F_CTRL_STA |
           I2C_A2F_CTRL_STO | I2C_A2F_CTRL_AA),
         &I2C_A2F(c)->ctrl);

  /* 
   * A transfer is kicked off by initiating a start condition    
   */
  ctrl = readl(&I2C_A2F(c)->ctrl);
  writel(ctrl | I2C_A2F_CTRL_STA, &I2C_A2F(c)->ctrl);

  /*
   * interrupt should be working ....
   */
  
  
  while(1){
    if(c->msg_status==0){
      break;
    }
  }

  ret = c->msg_status;

  return ret;
}


static void i2c_a2f_irq(void *d)
{
  struct i2c_a2f *c = (struct i2c_a2f *) d;
  unsigned int ctrl = readl(&I2C_A2F(c)->ctrl);
  unsigned int sta = readl(&I2C_A2F(c)->status);

  /*   
   * Check if there is a serial interrupt event 
   * pending at the controller. Bail out if there is none    
   */

  if (!(ctrl & I2C_A2F_CTRL_SI)) {
    ret = -100;
    return ret;
  }

  /*
   * Implement the state machine defined by the I2C controller  
   */
  switch (sta) {

  case I2C_A2F_STATUS_START:
  case I2C_A2F_STATUS_REPSTART:
    /*   
     * Remove the start condition 
     */
    writel(ctrl & ~I2C_A2F_CTRL_STA, &I2C_A2F(c)->ctrl);

    /*
     * Send out addr and direction       
     */
    writel((c->msg->addr << 1) |
	   (c->msg->flags & I2C_M_RD ? 1 : 0),
	   &I2C_A2F(c)->data);

    break;

  case I2C_A2F_STATUS_ADDR_W_NACK:
  case I2C_A2F_STATUS_ADDR_R_NACK:
  case I2C_A2F_STATUS_DATA_W_NACK:
    /*
     * No ack -> let's stop and report a failure   
     */
    writel(ctrl | I2C_A2F_CTRL_STO, &I2C_A2F(c)->ctrl);
    c->msg_status = -10;
    break;

  case I2C_A2F_STATUS_ADDR_W_ACK:
  case I2C_A2F_STATUS_DATA_W_ACK:
    /*   
     * If there is more data to send, send it 
     */
    if (c->msg_i < c->msg->len) {
      writel(c->msg->buf[(c->msg_i)++],
	     &I2C_A2F(c)->data);
    }

    /* 
     * If this is last transfer in the message,
     * send stop and report success    
     */
    else if (--(c->msg_n) == 0) {
      writel(ctrl | I2C_A2F_CTRL_STO, &I2C_A2F(c)->ctrl);
      c->msg_status = 0;
    }

    /*
     * This is not the last transfer in the message. 
     * Advance to the next segment and initate a repeated start.    
     */

    else {
      c->msg++;
      c->msg_i = 0;
      writel(ctrl | I2C_A2F_CTRL_STA, &I2C_A2F(c)->ctrl);
    }
    break;

  case I2C_A2F_STATUS_ADDR_R_ACK:
    /* 
     * Will be receiving the last byte from the slave. 
     * Return NACK to tell the slave to stop sending. 
     */
    if (c->msg_i + 1 == c->msg->len) {
      writel(ctrl & ~I2C_A2F_CTRL_AA, &I2C_A2F(c)->ctrl);
    }

    /*
     * Will be receiving more data from the slave.
     * Return ACK to tell the slave to send more.
     */
    else {
      writel(ctrl | I2C_A2F_CTRL_AA, &I2C_A2F(c)->ctrl);
    }

    break;

  case I2C_A2F_STATUS_DATA_R_ACK:
    /*                                                                               
     * Retrieve the data but                                                         
     * there is more data to receive in this transfer                                
     */
    c->msg->buf[c->msg_i++] = readl(&I2C_A2F(c)->data);

    /*                                                                               
     * Will be receiving the last byte from the slave.                               
     * Return NACK to tell the slave to stop sending.                                
     */
    if (c->msg_i + 1 == c->msg->len) {
      writel(ctrl & ~I2C_A2F_CTRL_AA, &I2C_A2F(c)->ctrl);
    }

    /*                                                                               
     * Will be receiving more data from the slave.                                   
     * Return ACK to tell the slave to send more.                                    
     */
    else {
      writel(ctrl | I2C_A2F_CTRL_AA, &I2C_A2F(c)->ctrl);
    }

    break;

  case I2C_A2F_STATUS_DATA_R_NACK:
    /*                                                                               
     * Retrieve the data but                                                         
     * this segment is done with                                                     
     */
    c->msg->buf[c->msg_i++] = readl(&I2C_A2F(c)->data);

    /*                                                                               
     * If this is last transfer in the message,                                      
     * send stop and report success                                                  
     */
    if (--(c->msg_n) == 0) {
      writel(ctrl | I2C_A2F_CTRL_STO, &I2C_A2F(c)->ctrl);
      c->msg_status = 0;
      disable_intr = 1;
    }

    /*                                                                               
     * This is not the last transfer in the message.                                 
     * Advance to the next segment                                                   
     * and initate a repeated start.                                                 
     */
    else {
      c->msg++;
      c->msg_i = 0;
      writel(ctrl | I2C_A2F_CTRL_STA, &I2C_A2F(c)->ctrl);
    }

    break;

  default:

    /*                                                                               
     * Some error condition -> let's stop and report a failure                       
     */
    
    /*                                                                                       
     * Clear various conditions that affect the controler and the bus                        
     */
    writel(readl(&I2C_A2F(c)->ctrl) &
	   ~(I2C_A2F_CTRL_SI | I2C_A2F_CTRL_STA |
	     I2C_A2F_CTRL_STO | I2C_A2F_CTRL_AA),
	   &I2C_A2F(c)->ctrl);


    c->msg_status = -100;

    break;
  }

  /*                                                                                       
   * Clear the serial interrupt condition                                                  
   */
  ctrl = readl(&I2C_A2F(c)->ctrl);
  writel(ctrl & ~I2C_A2F_CTRL_SI, &I2C_A2F(c)->ctrl);

  return ret;

}

