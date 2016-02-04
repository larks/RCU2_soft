#include <asm/uaccess.h>
#include <linux/init.h>
#include <linux/cdev.h>
#include <linux/fs.h>
#include <linux/module.h>
#include <linux/spi/spi.h>

MODULE_LICENSE( "GPL" );
MODULE_AUTHOR( "Eivind Askeland" );

#define SPI_BUS      0
#define CHIP_SELECT  1

#define BUS_SPEED_HZ 2500000
#define BUS_MODE     SPI_MODE_0

static const char name[] = "radmon";

static struct {
	struct spi_device *spi_device;
	dev_t              devt;
	struct cdev        cdev;
	struct class      *class;
} radmon;


/*
 * Register spi device for radmon.
 */
static int
radmon_probe( struct spi_device *spi_device )
{
	radmon.spi_device = spi_device;
	return 0;
}


static int
radmon_remove( struct spi_device *spi_device )
{
	radmon.spi_device = NULL;
	return 0;
}


static struct spi_driver radmon_driver = {
	.driver = {
		.name = name,
		.owner = THIS_MODULE,
	},
	.probe = radmon_probe,
	.remove = radmon_remove,
};


static int __init
radmon_add_device( void )
{
	struct spi_master *spi_master;
	struct spi_device *spi_device;
	int status;

	spi_master = spi_busnum_to_master( SPI_BUS );
	if ( !spi_master ) {
		printk( KERN_ALERT "Could not find master for spi bus %d.\n", SPI_BUS );
		return -1;
	}

	spi_device = spi_alloc_device( spi_master );
	if ( !spi_device ) {
		printk( KERN_ALERT "Could not allocate new device.\n" );
		return -1;
	}

	spi_device->chip_select = CHIP_SELECT;
	spi_device->max_speed_hz = BUS_SPEED_HZ;
	spi_device->mode = BUS_MODE;
	spi_device->irq = -1;
	spi_device->controller_state = NULL;
	spi_device->controller_data = NULL;
	spi_device->bits_per_word = 8;
	strlcpy( spi_device->modalias, name, SPI_NAME_SIZE );

	status = spi_add_device( spi_device );
	if ( status < 0 ) {
		printk( KERN_ALERT "Failed to add device. %d\n", status );
		spi_dev_put( spi_device );
	}

	radmon.spi_device = spi_device;

	put_device( &spi_master->dev );

	return status;
}


static void
radmon_rm_spidev(void)
{
	spi_unregister_device( radmon.spi_device );
	spi_unregister_driver( &radmon_driver );
}


/*
 * File opperations for radmon char-device.
 */
static int
radmon_open( struct inode *inode, struct file *file )
{
	file->f_pos = 0;
	return 0;
}


static uint16_t
radmon_read_reg( uint8_t reg )
{
	struct spi_transfer t = { 0 };
	struct spi_message  msg;
	uint32_t tx;
	uint32_t rx;

	tx = 0x00000001 | (reg<<8);
	rx = 0xdeadbeef;
	t.tx_buf = &tx;
	t.rx_buf = &rx;
	t.len = sizeof tx;
	spi_message_init( &msg );
	spi_message_add_tail( &t, &msg );
	spi_sync( radmon.spi_device, &msg );

	return ((rx&0x00ff0000)>>8) | ((rx&0xff000000)>>24);
}


static ssize_t
radmon_read( struct file *file, char __user *user_buff, size_t len, loff_t *off )
{
	int i;
	uint16_t buff[21];
	if ( *off < 0 )
		return -EBADFD;
	if ( *off >= sizeof(buff) )
		return 0;
	
	int start = *off;
	int end   = start + len;
	if ( end > sizeof(buff) )
		end = sizeof(buff);
	for ( i=0 ; i<21 ; ++i ) {
		if ( start <= i*2 && end > i*2 ) {
			buff[i] = radmon_read_reg( i );
		}
	}
	copy_to_user( user_buff, ((void*)buff)+start, end-start );
	*off += end-start;

	return end-start;
}


static void
radmon_write_reg( uint8_t reg, uint16_t value )
{
	struct spi_transfer t = { 0 };
	struct spi_message  msg;
	uint32_t tx;
	uint32_t rx;

	tx = (reg<<8) | ((value&0xff)<<24) | ((value&0xff00)<<8);
	rx = 0xdeadbeef;
	t.tx_buf = &tx;
	t.rx_buf = &rx;
	t.len = sizeof tx;
	spi_message_init( &msg );
	spi_message_add_tail( &t, &msg );
	spi_sync( radmon.spi_device, &msg );
}


static ssize_t
radmon_write( struct file *file, const char __user *user_buff, size_t len, loff_t *off )
{
	int start, end;
	uint16_t *buff;

	if ( *off < 0 )
		return -EBADFD;
	/*
	if ( *off >= 21*2 )
		return 0;
	*/
	start = *off;
	end = start + len;
	if ( end > 21 * 2 )
		end = 21 * 2;
	buff = user_buff - *off;
	/* Control register */
	if ( start == 0 && end >= 2 )
		radmon_write_reg( 0, buff[0] );
	/* Reset counters */
	if ( start <= 2 && end >= 4 )
		radmon_write_reg( 1, buff[1] );
	/* IRQ source */
	if ( start <= 8 && end >= 10 )
		radmon_write_reg( 4, buff[4] );
	/* sram pwr current threshold */
	if ( start <= 0x11 * 2 && end >= 0x11 * 2 + 2 )
		radmon_write_reg( 0x11, buff[0x11] );
	/* Device reset */
	if ( start <= 0xa5 * 2 && end >= 0xa5 * 2 + 2 )
		radmon_write_reg( 0xa5, buff[0xa5] );
	*off += end - start;

	return end - start;
}

static const struct file_operations radmon_fops = {
	.owner = THIS_MODULE,
	.read = radmon_read,
	.write = radmon_write,
	.open = radmon_open,
};


static int __init
radmon_init_chardev(void)
{
	int err;
	radmon.devt = MKDEV( 0, 0 );
	err = alloc_chrdev_region( &radmon.devt, 0, 1, name );
	if ( err < 0 ) {
		printk( KERN_ALERT "Failed to create device: %d.\n", err );
		return err;
	}

	cdev_init( &radmon.cdev, &radmon_fops );
	radmon.cdev.owner = THIS_MODULE;
	radmon.cdev.ops = &radmon_fops;

	radmon.class = class_create( THIS_MODULE, name );
	if ( !radmon.class ) {
		printk( KERN_ALERT "Failed to create class.\n" );
		err = -1;
		goto err1;
	}
	if ( !device_create( radmon.class, NULL, radmon.devt, NULL, name ) ) {
		printk( KERN_ALERT "Failed to create device.\n" );
		err = -1;
		goto err2;
	}

	err = cdev_add( &radmon.cdev, radmon.devt, 1 );
	if ( err < 0 ) {
		printk( KERN_ALERT "Failed to add device: %d.\n", err );
		goto err3;
	}

	return 0;

err3:
	device_destroy( radmon.class, radmon.devt );
err2:
	class_destroy( radmon.class );
err1:
	unregister_chrdev_region( radmon.devt, 1 );
	return err;
}


static void __exit
radmon_rm_chardev(void)
{
	cdev_del( &radmon.cdev );
	device_destroy( radmon.class, radmon.devt );
	class_destroy( radmon.class );
	unregister_chrdev_region( radmon.devt, 1 );
}


static int __init
radmon_init( void )
{
	int err;
	memset( &radmon, 0, sizeof(radmon) );

	err = spi_register_driver( &radmon_driver );
	if ( err < 0 ) {
		printk( KERN_ALERT "Failed to register driver: %d.\n", err );
		return err;
	}
	err = radmon_add_device();
	if ( err < 0 ) {
		return err;
	}

	err = radmon_init_chardev();
	if ( err < 0 ) {
		radmon_rm_spidev();
	}

	return err;
}


static void __exit
radmon_exit( void )
{
	radmon_rm_chardev();
	radmon_rm_spidev();
}


module_init( radmon_init );
module_exit( radmon_exit );
