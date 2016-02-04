#include <sys/types.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include "jamgpio.h"

/*Global variables*/
static unsigned long* gpio;
//static unsigned long* gpio_in;
//static unsigned long* gpio_out;

/*This function initialize the GPIO interface for the communication*/
void interface_init()
{
  
  int fd = open("/dev/mem", O_RDWR | O_SYNC);
  if (fd<0) {
  	if(GPIODBG >=1 ) fprintf(stderr, "open(\"/dev/mem\") failed\n");
    exit(-1);
  }
  else {if(GPIODBG >= 2) fprintf(stdout, "/dev/mem opened\n");}
//  else fprintf(stdout, "opened /dev/mem!\n");
  /*GPIO_CFG is mapped on the memory*/
  gpio = mmap(NULL, MAP_SIZE, PROT_READ|PROT_WRITE, MAP_SHARED, fd, (off_t)MSS_GPIO_CFG);
  if(gpio==MAP_FAILED)
  {
    if(GPIODBG>=1) fprintf(stderr, "GPIO map failed\n");
    exit(-1);
  }
  else {if(GPIODBG>=2) fprintf(stdout, "mapped gpio\n");}
/*  else fprintf(stdout, "gpio map success! 0x%x: value=0x%x\n", gpio, *gpio);
	fprintf(stdout, "GPIO_OUT_REG_EN: 0x%x\n", GPIO_OUT_REG_EN);
	fprintf(stdout, "GPIO_IN_REG_EN: 0x%x\n", GPIO_IN_REG_EN);
	fprintf(stdout, "GPIO_OUT_BUF_EN: 0x%x\n", GPIO_OUT_BUF_EN);
	fprintf(stdout, "GPIO_INT_EN: 0x%x\n", GPIO_INT_EN);
	
  *(gpio) = 0xBD;
  fprintf(stdout,"gpio: 0x%x\n", *gpio);
*/
  /*GPIO selection*/
  
  * (unsigned long *) (gpio + CTRL_GPIO2/sizeof(long)) |= GPIO_OUT_REG_EN;
  * (unsigned long *) (gpio + CTRL_GPIO2/sizeof(long)) &= ~(GPIO_IN_REG_EN);
  * (unsigned long *) (gpio + CTRL_GPIO3/sizeof(long)) |= GPIO_OUT_REG_EN;
  * (unsigned long *) (gpio + CTRL_GPIO3/sizeof(long)) &= ~(GPIO_IN_REG_EN);
  
 // fprintf(stdout, "GPIO3_cfg: 0x%x, val=0x%x\n", (gpio + CTRL_GPIO3/sizeof(long)), *(gpio + CTRL_GPIO3/sizeof(long)));
  
  * (unsigned long *) (gpio + CTRL_GPIO4/sizeof(long)) |= GPIO_OUT_REG_EN;
  * (unsigned long *) (gpio + CTRL_GPIO4/sizeof(long)) &= ~(GPIO_IN_REG_EN);
  // nTRST set high
  * (unsigned long *) (gpio + CTRL_GPIO5/sizeof(long)) |= GPIO_OUT_REG_EN;
  * (unsigned long *) (gpio + CTRL_GPIO5/sizeof(long)) |= GPIO_OUT_BUF_EN; // This does not work..
  * (unsigned long *) (gpio + CTRL_GPIO5/sizeof(long)) &= ~(GPIO_IN_REG_EN);
/*
  * (unsigned long *) (gpio + CTRL_GPIO5/sizeof(long)) &= ~(GPIO_IN_REG_EN | GPIO_OUT_BUF_EN);   
  * (unsigned long *) (gpio + CTRL_GPIO5/sizeof(long)) |= GPIO_OUT_REG_EN | GPIO_OUT_BUF_EN;
  * (unsigned long *) (gpio + CTRL_GPIO5/sizeof(long)) &= ~(GPIO_IN_REG_EN);
*/
  // Output
  * (unsigned long *) (gpio + CTRL_GPIO6/sizeof(long)) |= GPIO_IN_REG_EN;
  * (unsigned long *) (gpio + CTRL_GPIO6/sizeof(long)) &= ~(GPIO_OUT_REG_EN | GPIO_OUT_BUF_EN);
  
//  fprintf(stdout, "GPIO3_cfg: 0x%x, val=0x%x\n", (gpio + CTRL_GPIO6/sizeof(long)), *(gpio + CTRL_GPIO6/sizeof(long)));
  //close(fd);
  
}

void interface_exit()
{
  if(munmap(gpio,MAP_SIZE) < 0){
  	if(GPIODBG>=1) fprintf(stderr, "munmap() failed");
  	exit(-1);
  }
  else {
  	if(GPIODBG >= 2) fprintf(stdout, "munmap success\n");
  }
}

void gpio_set_tdi()
{
  *(gpio + MSS_GPIO_OUT/sizeof(long)) |= 0x8;
//  fprintf(stdout, "tdi=0x%x : 0x%x\n",(gpio + MSS_GPIO_OUT/sizeof(long)),*(gpio + MSS_GPIO_OUT/sizeof(long))); 
}

void gpio_clear_tdi()
{
  * (unsigned long *) (gpio + MSS_GPIO_OUT/sizeof(long)) &= ~(0x8);
}

void gpio_set_tms()
{
  * (unsigned long *) (gpio + MSS_GPIO_OUT/sizeof(long)) |= 0x10;
}

void gpio_clear_tms()
{
  * (unsigned long *) (gpio + MSS_GPIO_OUT/sizeof(long)) &= ~(0x10);
}

void gpio_set_tck()
{
  * (unsigned long *) (gpio + MSS_GPIO_OUT/sizeof(long)) |= 0x4;
   // fprintf(stdout, "%x,", *(gpio + MSS_GPIO_OUT/sizeof(long)));
}

void gpio_clear_tck()
{
  * (unsigned long *) (gpio + MSS_GPIO_OUT/sizeof(long)) &= ~(0x4);
}

void gpio_set_trst()
{
  * (unsigned long *) (gpio + MSS_GPIO_OUT/sizeof(long)) |= 0x20;
}
void gpio_clear_trst()
{
  * (unsigned long *) (gpio + MSS_GPIO_OUT/sizeof(long)) &= ~(0x20);
}


/*
void gpio_set_tdo()
{

}
*/
/*
void gpio_clear_tdo()
{
  //*(gpio3 + GPIO_DATAOUT/sizeof(long)) &= 0xffffbfff;
  //*(gpio5 + GPIO_DATAOUT/sizeof(long)) &= 0xfffeffff;
}
*/

int gpio_status_tdo()
{
  //printf("%d ", (*(gpio3 + GPIO_DATAIN/sizeof(long)) ));
  return (*(gpio + MSS_GPIO_IN/sizeof(long))>>6);
}

void lars_delay()
{
int i;
for(i=0; i<640000; i++);
for(i=0; i<640000; i++);
for(i=0; i<640000; i++);
}

