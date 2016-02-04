/*
 * rcubus.c
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <asm/uaccess.h>
#include <linux/fs.h>      // basic file structures and methods                                   
#include <linux/types.h>   // basic data types                                                    
#include <linux/sched.h>   // task schedule                                                        
#include <linux/errno.h>   // error codes                                                         
#include <linux/slab.h>    // kmalloc, kmfree                                                     
#include <asm/io.h>        // ioremap, ...                                                        
//#include <asm/uaccess.h>   // user space memory access (e.g. copy_to_user)                      
//#include <linux/poll.h>                                                                         
#include <linux/mm.h>      // virtual memory mapping                                              
#include <linux/ioport.h>  // detecting and reserving system resources                            
#include <linux/spinlock.h>
#include <asm/system.h>


/*
 * Driver verbosity level: 0->silent; >0->verbose
 */
static int rcubusc_debug = 1;

/*
 * User can change verbosity of the driver
 */
module_param(rcubusc_debug, int, S_IRUSR | S_IWUSR);
MODULE_PARM_DESC(rcubusc_debug, "RCU2 Bus driver verbosity level");

/*
 * Service to print debug messages
 */
#define d_printk(level, fmt, args...)				\
  if (rcubusc_debug >= level) printk(KERN_INFO "%s: " fmt,	\
				    __func__, ## args)

/*
 * Device major number
 */
static uint rcubusc_major = 178;

/*
 * User can change the major number
 */
module_param(rcubusc_major, uint, S_IRUSR | S_IWUSR);
MODULE_PARM_DESC(rcubusc_major, "RCU2 Bus driver major number");

/*
 * Device name
 */
static char * rcubusc_name = "rcubusc";

/*
 * Device access lock. Only one process can access the driver at a time
 */

static int rcubusc_lock = 0;

/*
 * Device "data"
 */
static char rcubusc_str[] = "This is the simplest loadable kernel module!!!! sample2.ko\n";
static char *rcubusc_end;

/*
 * address list for the communication to m2s-som
 */
 /*
static u32* apbbus_in_physaddr_MSM  = ((u32 *) 0x30000000);
static u32* apbbus_in_physaddr_TRM  = ((u32 *) 0x50000000);
static int apbbus_in_size   = 0xFFFFFFF;

static u32* apbbus_in_virtbase_TRM=NULL;
static u32* apbbus_in_virtbase_MSM=NULL;
*/
/*
module_param(apbbus_in_size, int, S_IRUSR | S_IWUSR);
module_param(apbbus_in_physaddr_MSM, uint, S_IRUSR | S_IWUSR);
module_param(apbbus_in_physaddr_TRM, uint, S_IRUSR | S_IWUSR);
*/

/*
 * RCU2 Bus Master Registers
*/
static u32* ADDR_REG = ((u32 *) 0x50001000); // RCU Bus Transaction Address

static u32 DATA_OUT_REG = 0x04; // RCU Bus Write Data
static u32 DATA_IN_REG = 0x08; // RCU Bus Read Data

static u32 CSR_REG = 0x0C; // TRX Control & Status Register
static u32 READ_CMD = 0x2;
static u32 WRITE_CMD = 0x3;

static u32 MODE_REG = 0x20; // TRX Mode Register
static u32 COMMAND_MODE = 0x1;
static u32 DIRECT_MODE = 0x0;

static u32 MODE = 0x0; /* default direct mode */
/* mode can be set in /sys/module/rcuc/parameters/MODE */
module_param(MODE, uint, S_IRUSR | S_IWUSR);

/* Export registers */
/*
TODO: Compiler warnings, don't understand why
module_param(ADDR_REG, uint, S_IRUSR | S_IWUSR);
module_param(DATA_OUT_REG, uint, S_IRUGO);
module_param(DATA_IN_REG, uint, S_IRUGO);
module_param(CSR_REG, uint, S_IRUGO);
module_param(MODE_REG, uint, S_IRUGO);
*/

/*
 * internal functions 
 */

static int dcs_read (u32 begin, u32 size, u32* buff)
{
  int i;
  u32 wordoffs=0;
  u32 byteoffs=0;
  for (i=0;i<size/4;i++){ // do the multiples of 4                                                  
    *(buff+wordoffs) = readl( (begin+i*4)); // shift four bits to the left
    wordoffs++;
  }
  i*=4;
  for (;i<size;i++) { // do the remaining bytes                                                     
    *(((char*)(buff+wordoffs))+byteoffs)=readb(begin+i);
    byteoffs++;
  }
  return 0;
}

static int dcs_write (u32 begin, u32 size, u32* buff)
{

  int i;
  u32 wordoffs=0;
  u32 byteoffs=0;

  for (i=0;i<size/4;i++){// do the multibles of 4  
    writel((u32*)(buff+wordoffs),(u32)(begin+i*4));
    wordoffs++;
  }
  i*=4;
  for (;i<size;i++) { // do the remaining bytes
    writeb((u32*)(((char*)(buff+wordoffs))+byteoffs), (u32)begin+i);
    byteoffs++;
  }

  return 0;
}


/*
 * Device open
 */
static int rcubusc_open(struct inode *inode, struct file *file)
{
  int ret = 0;

  /*
   * One process at a time
   */
  if (rcubusc_lock ++ > 0) {
    ret = -EBUSY;
    goto Done;
  }
  
  /*
   * Increment the module use counter
   */
  try_module_get(THIS_MODULE);
  
  /*
   * Do open-time calculations
   */
  rcubusc_end = rcubusc_str + strlen(rcubusc_str);
  
 Done:
  d_printk(2, "lock=%d\n", rcubusc_lock);
  return ret;
}

/*
 * Device close
 */
static int rcubusc_release(struct inode *inode, struct file *file)
{
  /*
   * Release device
   */
  rcubusc_lock = 0;
  
  /*
   * Decrement module use counter
   */
  module_put(THIS_MODULE);
  
  d_printk(2, "lock=%d\n", rcubusc_lock);
  return 0;
}

/* 
 * Device read
 */
static ssize_t rcubusc_read(struct file *filp, char *buffer,
			   size_t length, loff_t * offset)
{ 
  /*Init*/
  unsigned int len = 0;
  int ret = 0;
  u32 u32Count=0;
  u32 lPos=0;
  u32 u32ReadAddress = 0;
  u32* pu32Target = NULL;
  
    /*
   * Check that the user has supplied a valid buffer
   */
  if (! access_ok(0, buffer, length)) {
    ret = -EINVAL;
    goto Done;
  }
  
  /* Set */
  lPos = filp->f_pos;
  u32ReadAddress = lPos;
  u32Count = length;
  
  /* New scheme 
  	1. write to mode register
  	2. write address into ADDR_REG
  	3. if command mode, write command to CSR register
  	4. read results from DATA_IN_REG 
  */
  
  /* 1. Set target to mode register */
  pu32Target = ADDR_REG + (MODE_REG)/sizeof(u32); 
  dcs_write((u32)pu32Target, 1, &MODE); /* Set mode */
  
  /* 2. Set target to address register */
  pu32Target = ADDR_REG;                        
  dcs_write((u32)pu32Target, 2, &u32ReadAddress);
  
  /* 3. Check if Command mode */
  if(MODE == COMMAND_MODE){
	d_printk(1, "Command mode");
  	pu32Target = ADDR_REG + (CSR_REG)/sizeof(u32);
  	dcs_write((u32)pu32Target, 1, &READ_CMD);
  }
  
  /* 4. Read DATA_IN_REG */
  pu32Target = ADDR_REG + (DATA_IN_REG)/sizeof(u32); 
  dcs_read((u32)pu32Target, u32Count, (u32*) buffer); 
  
/*  d_printk(0, "read data=0x%x (address = %p)\n", *buffer, pu32Source); */

  /////////////////////////////////
  //  strncpy(buffer, addr, len);
  //  *offset += len;
  len = (int)u32Count;
  ret = len;

 Done:
  /* d_printk(0, "length=%d,len=%d,ret=%d\n", length, len, ret); */
  return ret;
}

/* 
 * Device write
 */
static ssize_t rcubusc_write(struct file *filp, const char *buffer,
			    size_t length, loff_t * offset)
{
  
  int iResult=0;
  u32 lPos=0;
  //u32 u32Offset=0;
  u32 u32Count= 0;
  u32* pu32Target=NULL;
  u32 u32WriteAddress=0;
  if (filp==NULL) {
    return -EFAULT;
  }
  lPos=filp->f_pos;
  
  u32Count=length;
  u32WriteAddress=lPos;

  /* 1. Set target to mode register */
  pu32Target = ADDR_REG + (MODE_REG)/sizeof(u32); 
  dcs_write((u32)pu32Target, 1, &MODE); /* Set mode */
  
  /* 2. Set target to address register */
  pu32Target = ADDR_REG;                        
  dcs_write((u32)pu32Target, 2, &u32WriteAddress); /* Set mode */
  
  /* 3. Write data into data out register */
  pu32Target = ADDR_REG + (DATA_OUT_REG)/sizeof(u32);
  dcs_write((u32)pu32Target, u32Count,  (u32*)buffer);
  
  /* 4. Check if Command mode */
  if(MODE == COMMAND_MODE){
  	d_printk(1, "Command mode");
  	pu32Target = ADDR_REG + (CSR_REG)/sizeof(u32);
  	dcs_write((u32)pu32Target, 1, &WRITE_CMD);
  }
  
  iResult=(int)u32Count;

/*  d_printk(3, "length=%d\n", length); */
  return iResult;
}


/*
 * Device seek
 */
static loff_t rcubusc_seek(struct file* filp, loff_t off, int ref)
{
  loff_t lPosition=0;
  if (filp){
    lPosition=off;
  }else{
    lPosition=-EFAULT;
  }
  if (lPosition>=0) filp->f_pos=lPosition;

  /* d_printk(0, "filp->f_pos = %d\n", (int)lPosition); */

  return lPosition;
}


/*
 * Device operations
 */
static struct file_operations rcubusc_fops = {
  .llseek = rcubusc_seek,
  .read = rcubusc_read,
  .write = rcubusc_write,
  .open = rcubusc_open,
  .release = rcubusc_release
};

static int __init rcubusc_init_module(void)
{
  int ret = 0;
  
  /*
   * check that the user has supplied a correct major number
   */
  if (rcubusc_major == 0) {
    printk(KERN_ALERT "%s: rcubusc_major can't be 0\n", __func__);
    ret = -EINVAL;
    goto Done;
  }
  
  /*
   * Register device
   */
  ret = register_chrdev(rcubusc_major, rcubusc_name, &rcubusc_fops);
  if (ret < 0) {
    printk(KERN_ALERT "%s: registering device %s with major %d "
	   "failed with %d\n",
	   __func__, rcubusc_name, rcubusc_major, ret);
    goto Done;
  }
 Done:
  d_printk(1, "name=%s,major=%d\n", rcubusc_name, rcubusc_major);
  
  return ret;
}
static void __exit rcubusc_cleanup_module(void)
{
  /*
   * Unregister device
   */
  unregister_chrdev(rcubusc_major, rcubusc_name);
  
  d_printk(1, "%s\n", "clean-up successful");
}

module_init(rcubusc_init_module);
module_exit(rcubusc_cleanup_module);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Lars Bratrud, lars.bratrud@cern.ch");
MODULE_DESCRIPTION("RCU2 Bus device driver");
