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
#include <asm/arch/comblk.h>
#include <malloc.h> 

void i2c_a2f_irq(void);
#define IRQ_I2C0  4



/*
 * I2C controller private data structure          
 */
struct i2c_a2f {
  struct i2c_msg*                 msg;            /* Current message */
  int                             msg_n;          /* Segments in msg */
  int                             msg_i;          /* Idx in a segment */
  volatile int                    msg_status;     /* Message status */
};

struct i2c_msg {
  u16 addr;     /* slave address                        */
  u16 flags;
  u16 len;              /* msg length                           */
  u8 *buf;              /* pointer to msg data                  */
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


#define I2C0_BASE      0x40002000
#define I2C_A2F        ((volatile struct i2c_a2f_regs *) I2C0_BASE)


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

#define I2C_M_RD                0x0001 

struct i2c_a2f *c ;
struct i2c_msg *m ; 

void i2c_init(int speed, int unused)
{
  
  static struct {
    unsigned int    d;
    unsigned int    b;
  } div[] =
      {{60,6},{120,5},{160,3},{192,2},{224,1},{256,0},{960,4},{0,0}};

  unsigned int v, b;
  if(speed<0 || speed>7){
    
  }


  b = div[speed].b;

  NVIC->ICER[((uint32_t)(IRQ_I2C0) >> 5)] = (1 << ((uint32_t)(IRQ_I2C0) & 0x1F));


  /*
   * Enable the controller  
   */
  v = I2C_A2F->ctrl;
  I2C_A2F->ctrl = v | I2C_A2F_CTRL_ENS1;
  
  /*       
   * Set up the clock rate 
   */
  v = I2C_A2F->ctrl;


  I2C_A2F->ctrl = (v
		   | (v & ~(1 << I2C_A2F_CTRL_CR0_SHFT))
		   | (((b >> 0) & 0x1) << I2C_A2F_CTRL_CR0_SHFT)
		   | (v & ~(1 << I2C_A2F_CTRL_CR1_SHFT))
		   | (((b >> 1) & 0x1) << I2C_A2F_CTRL_CR1_SHFT)
		   | (v & ~(1 << I2C_A2F_CTRL_CR2_SHFT))
		   | (((b >> 2) & 0x1) << I2C_A2F_CTRL_CR2_SHFT));

  /*  
   * Set the address for master-only operation   
   */
  I2C_A2F->addr= 0x0 ;//0x4e << 1;

  NVIC->ISER[((uint32_t)(IRQ_I2C0) >> 5)] = (1 << ((uint32_t)(IRQ_I2C0) & 0x1F));

}


static void i2c_release(struct i2c_a2f *c)
{

  /*
   * Clear various conditions that affect the controler and the bus 
   */
  unsigned int v;
  v = I2C_A2F->ctrl;
  I2C_A2F->ctrl = v &  ~(I2C_A2F_CTRL_SI | I2C_A2F_CTRL_STA |
			 I2C_A2F_CTRL_STO | I2C_A2F_CTRL_AA);

  /*  
   * Disable the controller     
   */
  v = I2C_A2F->ctrl;
  I2C_A2F->ctrl = v & ~I2C_A2F_CTRL_ENS1;

}


static int i2c_a2f_transfer(struct i2c_a2f *c)
{

  unsigned int ctrl;
  int ret = 0;
  unsigned int v;

  /*
   * Store the software parameters of the message.  
   * There will be used by the IRQ handler.
   */

  //c->msg = &m[0];
  //c->msg_i = 0;
  //c->msg_n = n;
  //c->msg_status = -100;


  /* 
   * Clear various conditions that affect the controler and the bus                                   
   */
  v = I2C_A2F->ctrl;
  I2C_A2F->ctrl = v &  ~(I2C_A2F_CTRL_SI | I2C_A2F_CTRL_STA |
			 I2C_A2F_CTRL_STO | I2C_A2F_CTRL_AA);
  /* 
   * A transfer is kicked off by initiating a start condition    
   */
  ctrl = I2C_A2F->ctrl;
  I2C_A2F->ctrl = ctrl | I2C_A2F_CTRL_STA;

  /*
   * interrupt should be working ....
   */
    
  while(1){
    if(c->msg_status>=0){
      break;
    }
  }

  ret = c->msg_status;

  printf("return code = 0x%x \n", c->msg_status);

  return ret;
}


void i2c_a2f_irq(void)
{
  //struct i2c_a2f *c = (struct i2c_a2f *) d;
  unsigned int ctrl = I2C_A2F->ctrl;
  unsigned int sta = I2C_A2F->status;
  unsigned int v ;

  //// this pointer is the buggy stuff!!
  //printf("send data = 0x%x 0x%x\n", c->msg->addr, (c->msg->flags & I2C_M_RD ? 1 : 0)); 
  printf("i2c_a2f_irq: 0x%x, 0x%x\n", ctrl, sta);

  /*   
   * Check if there is a serial interrupt event 
   * pending at the controller. Bail out if there is none    
   */

  if (!(ctrl & I2C_A2F_CTRL_SI)) {
    printf("i2c_a2f_irq: there is a serial interrupt event pending at the controller.\n");
    c->msg_status = 4;
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
    I2C_A2F->ctrl = ctrl & ~I2C_A2F_CTRL_STA;

    /*
     * Send out addr and direction       
    // */
    printf("send data = 0x%x 0x%x\n", c->msg->addr, (c->msg->flags & I2C_M_RD ? 1 : 0)); 
    I2C_A2F->data = (c->msg->addr << 1) |
      (c->msg->flags & I2C_M_RD ? 1 : 0);

    break;

  case I2C_A2F_STATUS_ADDR_W_NACK:
  case I2C_A2F_STATUS_ADDR_R_NACK:
  case I2C_A2F_STATUS_DATA_W_NACK:
    /*
     * No ack -> let's stop and report a failure   
     */
    I2C_A2F->ctrl = ctrl | I2C_A2F_CTRL_STO;
    c->msg_status = 1;
    break;

  case I2C_A2F_STATUS_ADDR_W_ACK:
  case I2C_A2F_STATUS_DATA_W_ACK:
    /*   
     * If there is more data to send, send it 
     */
    if (c->msg_i < c->msg->len) {
      I2C_A2F->data = c->msg->buf[c->msg_i];
      printf("data sending ....data=0x%x (sent data=0x%x, %d)\n", I2C_A2F->data, c->msg->buf[c->msg_i], c->msg_i);
      c->msg_i++;
    }

    /* 
     * If this is last transfer in the message,
     * send stop and report success    
     */
    else if (--(c->msg_n) == 0) {
      //writel(ctrl | I2C_A2F_CTRL_STO, &I2C_A2F(c)->ctrl);
      I2C_A2F->ctrl = ctrl | I2C_A2F_CTRL_STO;
      c->msg_status = 0;
    }

    /*
     * This is not the last transfer in the message. 
     * Advance to the next segment and initate a repeated start.    
     */

    else {
      c->msg++;
      c->msg_i = 0;
      I2C_A2F->ctrl = ctrl | I2C_A2F_CTRL_STA;
    }
    break;

  case I2C_A2F_STATUS_ADDR_R_ACK:
    /* 
     * Will be receiving the last byte from the slave. 
     * Return NACK to tell the slave to stop sending. 
     */
    if (c->msg_i + 1 == c->msg->len) {
      I2C_A2F->ctrl = ctrl & ~I2C_A2F_CTRL_AA;
    }

    /*
     * Will be receiving more data from the slave.
     * Return ACK to tell the slave to send more.
     */
    else {
      I2C_A2F->ctrl = ctrl | I2C_A2F_CTRL_AA;
    }

    break;

  case I2C_A2F_STATUS_DATA_R_ACK:
    /*                                                                               
     * Retrieve the data but                                                         
     * there is more data to receive in this transfer                                
     */
    c->msg->buf[c->msg_i++] = I2C_A2F->data;
    printf("I2C_A2F_STATUS_DATA_R_ACK: data=0x%x\n", I2C_A2F->data);
    /*                                                                               
     * Will be receiving the last byte from the slave.                               
     * Return NACK to tell the slave to stop sending.                                
     */
    if (c->msg_i + 1 == c->msg->len) {
      I2C_A2F->ctrl = ctrl & ~I2C_A2F_CTRL_AA;
    }

    /*                                                                               
     * Will be receiving more data from the slave.                                   
     * Return ACK to tell the slave to send more.                                    
     */
    else {
      I2C_A2F->ctrl = ctrl | I2C_A2F_CTRL_AA;
    }

    break;

  case I2C_A2F_STATUS_DATA_R_NACK:
    /*                                                                               
     * Retrieve the data but                                                         
     * this segment is done with                                                     
     */
    c->msg->buf[c->msg_i++] = I2C_A2F->data;
    printf("I2C_A2F_STATUS_DATA_R_NACK: data=0x%x\n", I2C_A2F->data);
    /*                                                                               
     * If this is last transfer in the message,                                      
     * send stop and report success                                                  
     */
    if (--(c->msg_n) == 0) {
      I2C_A2F->ctrl = ctrl | I2C_A2F_CTRL_STO;
      c->msg_status = 0;
    }

    /*                                                                               
     * This is not the last transfer in the message.                                 
     * Advance to the next segment                                                   
     * and initate a repeated start.                                                 
     */
    else {
      c->msg++;
      c->msg_i = 0;
      I2C_A2F->ctrl = ctrl | I2C_A2F_CTRL_STA;
    }

    break;

  default:

    /*                                                                               
     * Some error condition -> let's stop and report a failure                       
     */
    
    /*                                                                                       
     * Clear various conditions that affect the controler and the bus                        
     */
    v = I2C_A2F->ctrl;
    I2C_A2F->ctrl = v & ~(I2C_A2F_CTRL_SI | I2C_A2F_CTRL_STA |
			  I2C_A2F_CTRL_STO | I2C_A2F_CTRL_AA);

    c->msg_status = 2;

    break;
  }

  /*                                                                                       
   * Clear the serial interrupt condition                                                  
   */
  ctrl = I2C_A2F->ctrl;
  I2C_A2F->ctrl = ctrl & ~I2C_A2F_CTRL_SI;

}



//static int do_m2si2c(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
static int do_m2si2c(int argc, char *argv[])
{
  
  const char *cmd;
  u8 data = 0;
  u8 addr = 0x4e;
  char *endp;

  if (argc ==1 || argc>3)
    goto usage;
  
  cmd = argv[1];


  asm volatile ("cpsie i");

  printf("init i2c for EEPROM\n");
  i2c_init(3,0);

  c = (struct i2c_a2f*) malloc(sizeof(struct i2c_a2f));
  if(c==NULL){
    printf(" No memory allocation for i2c_a2f\n");
    goto done;
  }

  m = (struct i2c_msg*) malloc(sizeof(struct i2c_msg));
  if(m==NULL){
    printf(" No memory allocation for i2c_msg\n");
    goto done;
  }

  if(strcmp(cmd, "wr") == 0){
    if(argc!=3){
      goto usage;
    }

    data = simple_strtoul(argv[2], &endp, 16);
    printf("write data 0x%x for EEPROM chip addr=0x%x\n", data, addr);



    m->addr = addr;
    m->flags = 0x0; //write command
    m->len = 1;

    m->buf = (u8*) malloc(sizeof(u8)*m->len);
    if(m->buf==NULL){
      printf(" No memory allocation for i2c_msg->buf\n");
      goto done;
    }
    
    m->buf[0] = data;
    
    c->msg = m;
    c->msg_i = 0;
    c->msg_n = 1;
    c->msg_status = -100;
    
    printf("send data : data=0x%x\n", c->msg->buf[0]);
    i2c_a2f_transfer(c);

    if(m->buf!=NULL){
      free(m->buf);
    }

    goto done;

  }else if(strcmp(cmd, "rd") == 0){
    if(argc!=2){
      goto usage;
    }
    printf("read data at EEPROM chip addr=0x%x\n", addr);

    m->addr = addr;
    m->flags = 0x1; //read command
    m->len = 1;

    m->buf = (u8*) malloc(sizeof(u8)*m->len);
    if(m->buf==NULL){
      printf(" No memory allocation for i2c_msg->buf\n");
      goto done;
    }

    c->msg = m;
    c->msg_i = 0;
    c->msg_n = 1;
    c->msg_status = -100;
    printf("read data\n");
    i2c_a2f_transfer(c);
    
    printf("readback data = 0x%x\n", c->msg->buf[0]);

    if(m->buf!=NULL){
      free(m->buf);
    }

    goto done;

  }


 done:
  i2c_release(c);
  asm volatile ("cpsid i");
  printf("done\n");
  if(c!=NULL){
    free(c);
  }
  if(m!=NULL){
    free(m);
  }

  return 1;

 usage:
  i2c_release(c);
  asm volatile ("cpsid i");
  if(c!=NULL){
    free(c);
  }
  if(m!=NULL){
    free(m);
  }
  cmd_usage(cmdtp);
  return 1;

}

/*
U_BOOT_CMD(
           m2si2c, 3,      1,      do_m2si2c,
           "M2S_I2S Test: m2si2c rd, m2si2c wr data",
           "m2si2c rd, m2si2c wr data\n"
	   );
*/
