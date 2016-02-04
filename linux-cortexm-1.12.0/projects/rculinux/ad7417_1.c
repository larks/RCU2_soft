#include <linux/hwmon.h>
#include <linux/hwmon-sysfs.h>
#include <linux/init.h>
#include <linux/i2c.h>
#include <linux/module.h>

MODULE_LICENSE( "GPL" );
MODULE_AUTHOR( "Eivind Askeland" );

#define CHIP_ADDRESS 0x2a   /* i2c address. */
#define BUS_NO       0      /* i2c bus. */

/* Internal representation of one chip. */
struct ad7417_data {
	s16             temp;
	s16             adc[4];
	struct mutex    lock;
	unsigned long   timestamp;
};


/*
 * I2C.
 *
 * Code for accesing the chip over i2c protocoll.
 */

#define CONFIG_REG  0x1
#define CONFIG2_REG 0x5
#define TEMP_REG    0x0
#define ADC_REG     0x4
/*
#define THYST_REG   0x2
#define TOTI_REG    0x3
*/

#define AD7417_CONFMASK 0xe0


static inline int
ad7417_init_chip( struct i2c_client *client )
{
  u8 buff[2];
  int n;
  int cfg;

  //buff[0] = CONFIG2_REG;
  // buff[1] = 0;
  //n = i2c_master_send( client, buff, 2 );
  //if ( n < 2 ) {
  //  dev_err( &client->dev, "Could not initialize chip. Errno %d\n", -n );
  //  return n;
  // }

  ///////////////////////////////////
  
  buff[0] = CONFIG_REG;
  buff[1] = 0;
  i2c_master_send( client, buff, 2 );
  i2c_master_recv( client, buff, 1 );
  cfg = buff[0];
  
  buff[0] = CONFIG_REG;
  buff[1] = cfg & 0xfe;
  n = i2c_master_send( client, buff, 2 );
  if ( n < 2 ) {
    dev_err( &client->dev, "Could not initialize chip. Errno %d\n", -n );
    return n;
  }

  
  return 0;
}

struct ad7417_data *
ad7417_update_chip( struct i2c_client *client )
{
  u8  buff[2];
  struct ad7417_data *data;
  int channel;
  uint8_t cfg;
  uint8_t tmp;
  uint8_t val1;
  uint8_t val2;

  data = i2c_get_clientdata( client );
  mutex_lock( &data->lock );
  
  /* TODO: find a proper constant to add. */
  if ( !time_after( jiffies, data->timestamp /* + whatever */ ) ) {
    mutex_unlock( &data->lock );
    return data;
  }

  
  /* Get the current setting */
  buff[0] = CONFIG_REG;
  i2c_master_send( client, buff, 1 );
  i2c_master_recv( client, buff, 1 );
  cfg = buff[0];


  /* Read temperature. */
  buff[0] = CONFIG_REG;
  buff[1] = (cfg & ~(7<<5));
  i2c_master_send( client, buff, 2 );

  buff[0] = TEMP_REG;
  i2c_master_send( client, buff, 1 );
  i2c_master_recv( client, buff, 2 );
  data->temp = (buff[0]<<8) | buff[1];
  /* Temperature is 10 bit two's complement, shifted << 6
   * with .25 deg C per bit. This converts it to 1/100 deg C, 16bits.
   * Division instead of shift preserves sign. */
  data->temp = (data->temp*50)/128;
  
  /* Read ADCs */
  for ( channel=1 ; channel<5 ; ++channel ) {

    buff[0] = CONFIG_REG;
    i2c_master_send( client, buff, 1 );
    i2c_master_recv( client, buff, 1 );
    cfg = buff[0];
    
    tmp = channel << 5;
    buff[0] = CONFIG_REG;
    buff[1] = (cfg & ~AD7417_CONFMASK) | ( tmp & AD7417_CONFMASK);
    //buff[1] = cfg |  tmp;
    i2c_master_send( client, buff, 2 );

    buff[0] = ADC_REG;
    i2c_master_send( client, buff, 1 );
    i2c_master_recv( client, buff, 2 );
    val1 = buff[0] & 0xFF;
    val2 = buff[1] & 0xFF;
    data->adc[channel-1] = val1*4+val2/64 ;

  }

  /* Restore config2 register. */
  buff[0] = CONFIG2_REG;
  buff[1] = 0;
  i2c_master_send( client, buff, 2 );
  
  mutex_unlock( &data->lock );
  
  return data;
}


/*
 * SYSFS.
 *
 * Code for creating entries in sysfs.
 */

static ssize_t
ad7417_show_temp( struct device *dev, struct device_attribute *attr, char *buff )
{
	struct ad7417_data *data;
	struct i2c_client  *client;

	client = to_i2c_client( dev );
	data = ad7417_update_chip( client );
	return snprintf( buff, PAGE_SIZE, "%d\n", data->temp );
}

static ssize_t
ad7417_show_adc( struct device *dev, struct device_attribute *devattr, char *buff )
{
	struct ad7417_data *data;
	struct i2c_client  *client;
	struct sensor_device_attribute *attr;

	attr = to_sensor_dev_attr( devattr );
	client = to_i2c_client( dev );

	data = ad7417_update_chip( client );

	return snprintf( buff, PAGE_SIZE, "%u\n", data->adc[attr->index] );
}

static SENSOR_DEVICE_ATTR( temperature, S_IRUGO, ad7417_show_temp, NULL, 0 );
static SENSOR_DEVICE_ATTR( adc1, S_IRUGO, ad7417_show_adc, NULL, 0 );
static SENSOR_DEVICE_ATTR( adc2, S_IRUGO, ad7417_show_adc, NULL, 1 );
static SENSOR_DEVICE_ATTR( adc3, S_IRUGO, ad7417_show_adc, NULL, 2 );
static SENSOR_DEVICE_ATTR( adc4, S_IRUGO, ad7417_show_adc, NULL, 3 );
static struct attribute *ad7417_attributes[] = {
	&sensor_dev_attr_temperature.dev_attr.attr,
	&sensor_dev_attr_adc1.dev_attr.attr,
	&sensor_dev_attr_adc2.dev_attr.attr,
	&sensor_dev_attr_adc3.dev_attr.attr,
	&sensor_dev_attr_adc4.dev_attr.attr,
	NULL,
};
static const struct attribute_group ad7417_grp = {
	.attrs = ad7417_attributes,
};


/*
 * Kernel module.
 *
 * Code for initializing the kernel module.
 */

static const struct i2c_device_id ad7417_id[] = {
	{ "ad7417_1", 0 },
	{ }
};
MODULE_DEVICE_TABLE( i2c, ad7417_id );

static int
ad7417_probe( struct i2c_client *client, const struct i2c_device_id *id )
{
	int err;
	struct ad7417_data *data;

	//dev_info( &client->dev, "Probing for %s\n", client->name );

	err = ad7417_init_chip( client );
	if ( err != 0 ) {
		return -ENODEV;
	}

	data = kzalloc( sizeof( struct ad7417_data ), GFP_KERNEL );
	if ( !data )
		return -ENOMEM;

	mutex_init( &data->lock );
	data->timestamp = jiffies;
	i2c_set_clientdata( client, data );

	err = sysfs_create_group( &client->dev.kobj, &ad7417_grp );
	if ( err ) {
		goto err_free;
		dev_err( &client->dev, "Could not create sysfs group. Errno: %d\n",
				-err );
	}

	return 0;

err_free:
	kfree( data );
	return err;
}

static int
ad7417_remove( struct i2c_client *client )
{
	sysfs_remove_group( &client->dev.kobj, &ad7417_grp );
	kfree( i2c_get_clientdata( client ) );
	return 0;
}

static struct i2c_driver ad7417_driver = {
	.driver = {
		.name = "ad7417_1",
	},
	.probe = ad7417_probe,
	.remove = ad7417_remove,
	.id_table = ad7417_id,
};

static struct i2c_board_info adc_board_info = {
	I2C_BOARD_INFO( "ad7417_1", CHIP_ADDRESS ),
};

static struct i2c_client *cli_handle;

static int __init
ad7417_init(void)
{
	int ret;

	ret = i2c_add_driver( &ad7417_driver );
	if ( ret < 0 )
		return ret;

	/* Probe for a chip at the specified address. */
	cli_handle = i2c_new_device( i2c_get_adapter(BUS_NO), &adc_board_info );

	return 0;
}

static void __exit
ad7417_exit(void)
{
	i2c_unregister_device( cli_handle );
	i2c_del_driver( &ad7417_driver );
}

module_init( ad7417_init );
module_exit( ad7417_exit );
