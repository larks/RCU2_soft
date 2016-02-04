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


#include "mrKernLogging.c"   // kernel debugging

/* Physical addresses */
static u32* trig_recv_phys_base = ((u32 *) 0x50040000);
static int  trig_recv_size     = 0x10; // size of 16 for testing 
/* Virtual addresses */
static u32* trig_recv_virtbase = NULL;

// Fixed major number
static uint rcuc_majorID = 254;

/* Parameters which can be changed at driver load */
//smodule_param(trig_recv_phys_base, long, 0);


// Access control
static int driver_lock = 0;


/*Internal functions*/
static int rcu_read(u32 begin, u32 size, u32* buff)
{
	int i;
	u32 wordoffset;
	for(i=0; i<size; i++){
		*(buff+wordoffset) = readl(begin+i);
		wordoffset++;
	}
	return 0;
}



/*
* Here comes file operation functions:
*/
// File open
static int rcuc_open(struct inode* inode, struct file* file_p)
{
	return 0;
}
// File close 
static int rcuc_close(struct inode* inode, struct file* file_p)
{
	return 0;
}
// File read
static ssize_t rcuc_read(struct file* file_p, 
                         char __user* buffer, 
                         size_t count, 
                         loff_t* f_pos)
{

/*
	// some stuff
	int iResult = 0;
	u32 lPosition=file_p->f_pos;
	u32 u32Offset=0;
	u32 u32Count=0;
	u32* pu32Source=NULL;
	
	if(file_p==NULL){ // check the file pointer
		printk("read: invalid file pointer\n");
		return -EFAULT;
	}
	if(file_p && buffer && count>=0 && lPosition>=0){
		printk("read: count=%d f_pos=0x%08x", count, lPosition);
		if(lPosition < trig_recv_size){
			if(count + lPosition <= trig_recv_size)
			  u32Count = count;
			else{
			  u32Count = trig_recv_size;
			  printk("read: read access exceeds the size of the register, truncate\n");
			}
		u32Offset = lPosition; // set internal offset to file position
		pu32Source = trig_recv_phys_base; // read from trigger receiver module
		}
		// Read data on request
		if(pu32Source!=NULL){
			u32Offset = u32Offset/sizeof(u32);
			pu32Source = pu32Source + u32Offset;
			rcu_read((u32)pu32Source, u32Count, (u32*) buffer);
			iResult = (int)u32Count;
			if(f_pos) *f_pos=file_p->f_pos+iResult;
			else printk("read: invalid param offset, skipping position increment\n");
		}
	}
	else{
		printk("read: invalid parameter\n");
		iResult=-EINVAL;
	}
	return iResult;
	*/
	int i;
	u32 wordoffset;
	u32 word;
	for(i=0; i<trig_recv_size; i++){
		//*(buffer+wordoffset) = readl(trig_recv_phys_base+i);
		word = readl(trig_recv_phys_base + i);
		if(copy_to_user(buffer, &word, 1)){return -EFAULT;}
		printk("wordoffset:%lx\n", (long unsigned int)word);
		wordoffset++;
	}
	return 0;
}
// File write
static ssize_t rcuc_write(struct file *file_p, 
                          const char __user *buffer, 
                          size_t length, 
                          loff_t* f_pos)
{
	printk("write not implemented\n");
	return 0;	
}
// Define our custom file operation structure
static struct file_operations rcuc_fops =
{
	.owner = THIS_MODULE,
	.open = rcuc_open,
	.release = rcuc_close,
	.read = rcuc_read,
	.write = rcuc_write
};

/*
* Module operation functions
*/
static int __init rcumodule_init(void)
{
	int iResult = 0; // holding result of operations
	u32 buffer;
	//u32 u32Count = 1;
	
	
	// Register the driver
	iResult = register_chrdev(rcuc_majorID, "rcuc", &rcuc_fops);
	if (iResult < 0) {
		printk(KERN_INFO "module init: can't register driver\n");
		//mrlogmessage(LOG_ERROR, KERN_ERR "module init: can't register driver");
	}
	else{
		printk(KERN_INFO "module init: success!\n");
		//mrlogmessage(LOG_INFO, KERN_INFO, "module init: success!");
		
		// Map physical address to virtual address, not needed, InitRealBuffers()
		if(trig_recv_size){
			trig_recv_virtbase = (u32*) ioremap_nocache( (u32 *)trig_recv_phys_base, 				(int)trig_recv_size );
		printk("Remapped TRGRECV from 0x%p to 0x%p\n", trig_recv_phys_base, trig_recv_virtbase);
		}
		// InitDriverLock()

	

	buffer = readl(trig_recv_phys_base);
	printk("read %lx, at 0x%p\n", (long unsigned int)buffer, trig_recv_phys_base);
//	iResult = initRealBuffers();
		
	}
	

	
	return iResult;
}

static void __exit rcumodule_exit(void)
{
    unregister_chrdev(rcuc_majorID, "rcuc");
    iounmap(trig_recv_virtbase);
	printk("Bye bye kernel\n"); // printed when succesfully closed
}

module_init(rcumodule_init);
module_exit(rcumodule_exit);

MODULE_AUTHOR("Lars Bratrud");
MODULE_DESCRIPTION("RCU2 registers driver");
MODULE_LICENSE("GPL");
