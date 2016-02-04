/*!
 * jamgpio.h
 */

#define MAP_SIZE 4096

/*Register maps*/
#define MSS_GPIO_CFG        0x40013000
//#define MSS_GPIO_IRQ        0x40013080 unused
#define MSS_GPIO_IN         0x84
#define MSS_GPIO_OUT        0x88

/* GPIO control registers offsets */
#define CTRL_GPIO2          0x08
#define CTRL_GPIO3          0x0C
#define CTRL_GPIO4          0x10
#define CTRL_GPIO5          0x14

#define CTRL_GPIO6          0x18

//#define DELAY_LARS 100000
#define GPIODBG 1

/*
 * GPIO configuration register bits
 */
#define GPIO_OUT_REG_EN     (1 << 0)
#define GPIO_IN_REG_EN      (1 << 1)
#define GPIO_OUT_BUF_EN     (1 << 2)
#define GPIO_INT_EN     (1 << 3)

#define TEST 0xBD


void interface_init();
void interface_exit();
void gpio_set_tdi();
void gpio_clear_tdi();
void gpio_set_tms();
void gpio_clear_tms();
void gpio_set_tck();
void gpio_clear_tck();
void gpio_set_trst();
void gpio_clear_trst();
//void gpio_set_tdo();
//void gpio_clear_tdo();
int gpio_status_tdo();
void lars_delay();
