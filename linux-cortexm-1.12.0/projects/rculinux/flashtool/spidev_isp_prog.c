/*
 * SPI testing utility (using spidev driver)
 *
 * Copyright (c) 2007  MontaVista Software, Inc.
 * Copyright (c) 2007  Anton Vorontsov 
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License.
 *
 * Cross-compile with cross-gcc -I/path/to/cross-kernel/include
 *
 */

#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/types.h>
#include <linux/spi/spidev.h>


struct spi_ioc_transfer xfer[2];
char buf[516];
char tx_buf[516];
char buf2[516];
const int buffer_size=256;
char data_buffer[256];

#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))


#define READ_ARRAY_OPCODE   0x03
#define DEVICE_ID_READ      0x9F


#define WRITE_ENABLE_CMD    0x06
#define WRITE_DISABLE_CMD   0x4
#define PROGRAM_PAGE_CMD    0x02
#define WRITE_STATUS1_OPCODE    0x01
#define CHIP_ERASE_OPCODE   0xc7
#define ERASE_4K_BLOCK_OPCODE   0x20
#define ERASE_32K_BLOCK_OPCODE  0x52
#define ERASE_64K_BLOCK_OPCODE  0xD8
#define READ_STATUS         0x05

#define DMA_TRANSFER_SIZE    32u
#define READY_BIT_MASK      0x01

#define UNPROTECT_SECTOR_OPCODE     0x39

#define DONT_CARE       0

#define NB_BYTES_PER_PAGE   256


#define SPI_WRITE_ADDR_BASE 0x700000
#define VERSION_NEW_ADDR 0x0
#define VERSION_OLD_ADDR 0x2
#define VERSION_SPI_ADDR 0x4
#define RESULT_AUTHENTICATION 0x10
#define RESULT_PROGRAMMING    0x12
#define RESULT_VERIFY 0x14


static void pabort(const char *s)
{
  perror(s);
  abort();
}

static const char *device = "/dev/spidev0.0";
static uint8_t mode = 3;
static uint8_t bits = 8;
static uint32_t speed = 2000000;
static uint16_t delay;


void print_status_reg(int fd)
{
  int status;
  buf[0] = READ_STATUS;

  xfer[0].tx_buf = (unsigned long)buf;
  xfer[0].len = 1;
  
  xfer[1].rx_buf = (unsigned long) buf2;
  xfer[1].len = 1; /* Length of Data to read */
  
  status = ioctl(fd, SPI_IOC_MESSAGE(2), xfer);
    
  if (status < 0)
    {
      perror("SPI_IOC_MESSAGE");
      return;
    }

  printf("status-reg 1 %d %02x \n", status, buf2[0]);
}

void wait_ready(int fd)
{
  int status;
  uint8_t ready_bit;
  buf[0] = READ_STATUS;

  do {
    xfer[0].tx_buf = (unsigned long)buf;
    xfer[0].len = 1;

    xfer[1].rx_buf = (unsigned long) buf2;
    xfer[1].len = 1; /* Length of Data to read */

    status = ioctl(fd, SPI_IOC_MESSAGE(2), xfer);
    
    if (status < 0)
      {
	perror("SPI_IOC_MESSAGE");
	return;
      }
    ready_bit = buf2[0];
    ready_bit = ready_bit & READY_BIT_MASK;
  } while(ready_bit == 1);
//printf("*** %d %02x \n", status, buf2[0]);
//  printf("*** %d %02x \n", status, buf2[1]);

}


void spi_global_unprotect(int fd)
{
  int status;


  //write enable 
  wait_ready(fd);
  buf[0] = WRITE_ENABLE_CMD; //0x06;
  xfer[0].tx_buf = (unsigned long)buf;
  xfer[0].len = 1;
  status = ioctl(fd, SPI_IOC_MESSAGE(1), xfer);
  if (status < 0)
  {
    perror("SPI_IOC_MESSAGE");
    return;
  }

  //Unprotest sector 
  wait_ready(fd);
  buf[0] = WRITE_STATUS1_OPCODE;
  buf[1] = 0;
  xfer[0].tx_buf = (unsigned long)buf;
  xfer[0].len = 2;
  status = ioctl(fd, SPI_IOC_MESSAGE(1), xfer);
  if (status < 0)
  {
    perror("SPI_IOC_MESSAGE");
    return;
  }

}

//////////
// Read n bytes from the 2 bytes add1 add2 address
//////////
 
void spi_read(int fd, int addr, int nbytes)
{
  int status; 
  int index=0;
  //memset(buf, 0, sizeof buf);
  buf[0] = READ_ARRAY_OPCODE; //0x03;
  buf[1] = ((addr >> 16) & 0xFF);
  buf[2] = ((addr >> 8) & 0xFF);
  buf[3] = (addr & 0xFF);

  xfer[0].tx_buf = (unsigned long)buf;
  xfer[0].len = 4; /* Length of  command to write*/

  xfer[1].rx_buf = (unsigned long) buf2;
  xfer[1].len = nbytes; /* Length of Data to read */

  status = ioctl(fd, SPI_IOC_MESSAGE(2), xfer);
  if (status < 0)
    {
      perror("SPI_IOC_MESSAGE");
      return;
    }

}


void spi_data_read(int fd, int start_addr, int size){


  int target_addr = start_addr;
  int readout_size = size;

  //printf("spi_data_reading ... \n");

  spi_read(fd, target_addr, readout_size);

  //printf("spi_data_read done\n");

}


void verify(int fd){
  int status; 
  int index;
  

  buf[0] =  DEVICE_ID_READ ;
  xfer[0].tx_buf = (unsigned long)buf;
  xfer[0].len = 1; /* Length of  command to write*/

  xfer[1].rx_buf = (unsigned long) buf2;
  xfer[1].len = 3; /* Length of Data to read */

  status = ioctl(fd, SPI_IOC_MESSAGE(2), xfer);
  if (status < 0)
    {
      perror("SPI_IOC_MESSAGE");
      return;
    }
  printf("SPI Chip ID: %02x %02x %02x\n", buf2[0], buf2[1], buf2[2]);

}
 

int main(int argc, char *argv[])
{
  int ret = 0;
  int fd;
  int size = 0;

  fd = open(device, O_RDWR);
  if (fd < 0)
    pabort("can't open device");

  /*
   * spi mode
   */
  ret = ioctl(fd, SPI_IOC_WR_MODE, &mode);
  if (ret == -1)
    pabort("can't set spi mode");


  ret = ioctl(fd, SPI_IOC_RD_MODE, &mode);
  if (ret == -1)
    pabort("can't get spi mode");

  /*
   * bits per word
   */
  ret = ioctl(fd, SPI_IOC_WR_BITS_PER_WORD, &bits);
  if (ret == -1)
    pabort("can't set bits per word");

  ret = ioctl(fd, SPI_IOC_RD_BITS_PER_WORD, &bits);
  if (ret == -1)
    pabort("can't get bits per word");

  /*
   * max speed hz
   */
  ret = ioctl(fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed);
  if (ret == -1)
    pabort("can't set max speed hz");

  ret = ioctl(fd, SPI_IOC_RD_MAX_SPEED_HZ, &speed);
  if (ret == -1)
    pabort("can't get max speed hz");

  printf("spi mode: %d\n", mode);
  printf("bits per word: %d\n", bits);
  printf("max speed: %d Hz (%d KHz)\n", speed, speed/1000);


  verify(fd);
  spi_global_unprotect(fd);

  
  spi_data_read(fd, SPI_WRITE_ADDR_BASE+VERSION_NEW_ADDR, 1);
  printf("Currently installed fabric design version = 0x%x\n", buf2[0]);
  spi_data_read(fd, SPI_WRITE_ADDR_BASE+VERSION_OLD_ADDR, 1);
  printf("Previously installed fabric design version = 0x%x\n", buf2[0]);
  spi_data_read(fd, SPI_WRITE_ADDR_BASE+VERSION_SPI_ADDR, 1);
  printf("Fabric design version in the SPI Flash = 0x%x\n", buf2[0]);
  spi_data_read(fd, SPI_WRITE_ADDR_BASE+RESULT_AUTHENTICATION, 1);
  printf("ISP Programming authentication = 0x%x\n", buf2[0]);
  spi_data_read(fd, SPI_WRITE_ADDR_BASE+RESULT_PROGRAMMING, 1);
  printf("ISP Programming programming = 0x%x\n", buf2[0]);
  spi_data_read(fd, SPI_WRITE_ADDR_BASE+RESULT_VERIFY, 1);
  printf("ISP Programming verification = 0x%x\n", buf2[0]);

  close(fd);

  return ret;
}


