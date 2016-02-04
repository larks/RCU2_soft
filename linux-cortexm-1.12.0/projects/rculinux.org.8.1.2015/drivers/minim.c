// macros like module_init(), module_exit() etc.
#include <linux/module.h>
// included for the module version to be compitable with the 
// kernel version it is going to be loaded into:
#include <linux/version.h>
// printk etc, equivalent to stdio.h in user space
#include <linux/kernel.h>
// APIs for registering major, minor range of device files
#include <linux/fs.h>
// Defines dev_t
#include <linux/types.h>
// Defines MAJOR(), MINOR() macros
#include <linux/kdev_t.h>
#include <linux/device.h>
#include <linux/cdev.h>
#include <asm/uaccess.h>

static dev_t first; // Global variable for first minor device number
static struct cdev c_dev; // Global variable for the char device structure
static struct class *c1; // Global variable for the device class

static char balls;

/*
* Here comes file operation functions:
*/
// File open
static int my_open(struct inode *i, struct file *f)
{
	printk(KERN_INFO "Driver: open()\n");
	return 0;
}
// File close 
static int my_close(struct inode *i, struct file *f)
{
	printk(KERN_INFO "Driver: close()\n");
	return 0;
}
// File read
static ssize_t my_read(struct file *f, char __user *buf, size_t len, loff_t *off)
{
	printk(KERN_INFO "Driver: read()\n");
	if(copy_to_user(buf, &balls, 1) !=  0)
		return -EFAULT;
	else
		return 1;
}
// File write
static ssize_t my_write(struct file *f, const char __user *buf, size_t len, loff_t *off)
{
	printk(KERN_INFO "Driver: write()\n");
	if(copy_from_user(&balls, buf+len-1, 1) !=  0)
		return -EFAULT;
	else
		return len;
}
// Define our custom file operation structure
static struct file_operations lars_fops =
{
	.owner = THIS_MODULE,
	.open = my_open,
	.release = my_close,
	.read = my_read,
	.write = my_write
};

/*
* Module operation functions
*/
static int __init hello_init(void)
{
	printk("Hello kernel! \n");
    if(alloc_chrdev_region(&first, 0, 1, "minimod") < 0){
        return -1;
    }
    if((c1 = class_create(THIS_MODULE, "chardrv")) == NULL){
    	unregister_chrdev_region(first, 1);
    	return -1;
    }
    if(device_create(c1, NULL, first, NULL, "mynull") == NULL){
    	class_destroy(c1);
    	unregister_chrdev_region(first, 1);
    	return -1;
    }
    // Initializes c_dev, remembering lars_fops, making it ready to add to the system with cdev_add.
    cdev_init(&c_dev, &lars_fops);
    // adds c_dev to the system, making it live immediately. A negative error code is returned on failure.
    if(cdev_add(&c_dev, first, 1) == -1){
    	device_destroy(c1, first);
    	class_destroy(c1);
    	unregister_chrdev_region(first, 1);
    	return -1;
    }
    // done
	return 0;
}

static void __exit hello_exit(void)
{
	cdev_del(&c_dev);
	device_destroy(c1, first);
	class_destroy(c1);
    unregister_chrdev_region(first, 1);
	printk("Bye bye kernel\n"); // printed when succesfully closed
}

module_init(hello_init);
module_exit(hello_exit);

MODULE_AUTHOR("Lars Bratrud, UiB, lars.bratrud@ift.uib.no");
MODULE_DESCRIPTION("Hello World Example");
MODULE_LICENSE("GPL");
