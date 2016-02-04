#include <linux/module.h>	 // Module macros
#include <linux/version.h>
#include <linux/kernel.h>    // 
#include <linux/fs.h>        // File structures and methods
#include <linux/types.h>     // Datatypes
#include <linux/kdev_t.h>    // Major and minor macros
#include <linux/device.h>    // 
#include <linux/cdev.h>      // 
#include <asm/uaccess.h>     // user space memory access
#include <asm/io.h>          // ioremap etc.
#include <linux/moduleparam.h>
//#include <linux/arch/arm/mach-m2s/gpio.h>
#include <linux/gpio.h>

// Fixed major number
static uint radmonc_majorID = 253;

static u32* gpio_in_reg = ((u32 *)  0x40013084);
static u32* gpio_out_reg = ((u32 *) 0x40013088);
static u32* gpio_cfg_reg = ((u32*)  0x40013000);
// pin names

//static struct gpio jtag_radmon[] = {
//		{2, GPIOF_OUT_INIT_LOW, "Radmon TCK"}, /*TCK going to Radmon*/
//		{3, GPIOF_OUT_INIT_LOW, "Radmon TDO"}, /*TDO going to Radmon TDI*/
//		{4, GPIOF_OUT_INIT_LOW, "Radmon TMS"}, /*TMS*/
//		{5, GPIOF_OUT_INIT_HIGH, "Radmon TRST"}, /*nTRST going to Radmon*/
//		{6, GPIOF_IN_INIT_LOW, "Radmon TDI"} /*TDI to Radmon TDO*/
//};

/*!
 * Open module
 */
static int radmonjtag_open(struct inode* inode, struct file* file_p)
{
	//int i;
	//i = gpio_get_value(6);
	return 0;
}
/*!
 * Closing module
 */
static int radmonjtag_close(struct inode* inode, struct file* file_p)
{
	return 0;
}
/*!
 * Read module
 */
static ssize_t radmonjtag_read(struct file* file_p, 
                         char __user* buffer, 
                         size_t count, 
                         loff_t* f_pos)
{
	u8 byte;
	/* We are only caring about tdo from radmon, so this is sufficient */
	byte = readl(gpio_in_reg); // we are only reading one and same byte at all times
	if(copy_to_user(buffer, &byte, 1)) return -EFAULT;
	
	return count;
}

/*!
 * Write to module
 */
static ssize_t radmonjtag_write(struct file *file_p, 
                          const char __user *buffer, 
                          size_t count, 
                          loff_t* f_pos)
{
    u8 byte;
    if(copy_from_user(&byte, buffer+count-1, 1))
    	  return -EFAULT;
    outl(byte, gpio_out_reg);
	return 1;	
}

/*!
 * File operation structure
 */
static struct file_operations radmonc_fops =
{
	.owner = THIS_MODULE,
	.open = radmonjtag_open,
	.release = radmonjtag_close,
	.read = radmonjtag_read,
	.write = radmonjtag_write
};

/*!
 * Module operation functions
 */
static int __init radmonjtagmodule_init(void)
{
	int i;
	int iResult = 0; // holding result of operations
	iResult = register_chrdev(radmonc_majorID, "radmonc", &radmonc_fops);
	if (iResult < 0) {
		printk(KERN_INFO "radmonc: can't register driver\n");
		//mrlogmessage(LOG_ERROR, KERN_ERR "module init: can't register driver");
	}
	else{
		iResult = gpio_request(6, "TDI");
//		printk(KERN_INFO "radmonc: module registered, iResult=%d\n", iResult);
		iResult = gpio_direction_input(6);
//		printk("radmonc: gpio_direction, iResult=%d\n", iResult);
		// request outputs
		gpio_request(5, "TRST"); gpio_direction_output(5,0);
		gpio_request(4, "TMS"); gpio_direction_output(4,0);
		gpio_request(3, "TDO"); gpio_direction_output(3,0);
		gpio_request(2, "TCK"); gpio_direction_output(2,0);
	}			
	return iResult;
}

static void __exit radmonjtagmodule_exit(void)
{
	int iResult; int i;
	//int iResult;
    unregister_chrdev(radmonc_majorID, "radmonc");
    for (i=0; i<5; i++){
//		gpio_free(6);
		//if(iResult<0){printk(KERN_WARNING "radmonc: unable to request GPIO_%d\n", i);}
		//else {printk(KERN_INFO "freed GPIO: i=%d, iResult=%d\n", i, iResult);}
		gpio_free(i);
        printk("Freed GPIO_%d, iResult=%d\n", i, iResult);
	}

	printk("Bye bye kernel\n"); // printed when succesfully closed
}

module_init(radmonjtagmodule_init);
module_exit(radmonjtagmodule_exit);

MODULE_AUTHOR("Lars Bratrud");
MODULE_DESCRIPTION("Radmon JTAG driver");
MODULE_LICENSE("GPL");
