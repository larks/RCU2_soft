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
#include <string.h>
#include <malloc.h>
/*
  
  u-boot envriroment 
  size = 0x100 x 5 is enough
       = 0x500 = 1280

 */


#define ENV_HEADER_SIZE        (sizeof(uint32_t))
#define CONFIG_ENV_SECT_SIZE            0x1000
#define CONFIG_ENV_SIZE                 CONFIG_ENV_SECT_SIZE
#define ENV_SIZE (CONFIG_ENV_SIZE - ENV_HEADER_SIZE)

typedef struct environment_s {
  unsigned int        crc;            /* CRC32 over data bytes        */
  unsigned char   data[ENV_SIZE]; /* Environment data             */
} env_t;


struct spi_ioc_transfer xfer[2];
char buf[1024+4];
char tx_buf[1024+4];
uint8_t buf2[1024+4];
const int buffer_size=1024;
uint8_t data_buffer[4096];
uint8_t data_out_buffer[4096];


env_t *env;

static int chip_select;
static int start_address;
static int end_address;
static int offset_address;
static int command_id ;
static char target_var[32] = "bootcmd";
static char write_data[512] = "run flashboot";
static char env_filename[512]="tmp.dat";

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




static void pabort(const char *s)
{
  perror(s);
  abort();
}

static const char *device0 = "/dev/spidev0.0";
static const char *device1 = "/dev/spidev0.1";
static uint8_t mode = 3;
static uint8_t bits = 8;
static uint32_t speed = 1000000;
static uint16_t delay;


///from u-boot routine 

static const unsigned int crc32tab[256] = { 
  0x00000000, 0x77073096, 0xee0e612c, 0x990951ba,
  0x076dc419, 0x706af48f, 0xe963a535, 0x9e6495a3,
  0x0edb8832, 0x79dcb8a4, 0xe0d5e91e, 0x97d2d988,
  0x09b64c2b, 0x7eb17cbd, 0xe7b82d07, 0x90bf1d91,
  0x1db71064, 0x6ab020f2, 0xf3b97148, 0x84be41de,
  0x1adad47d, 0x6ddde4eb, 0xf4d4b551, 0x83d385c7,
  0x136c9856, 0x646ba8c0, 0xfd62f97a, 0x8a65c9ec,
  0x14015c4f, 0x63066cd9, 0xfa0f3d63, 0x8d080df5,
  0x3b6e20c8, 0x4c69105e, 0xd56041e4, 0xa2677172,
  0x3c03e4d1, 0x4b04d447, 0xd20d85fd, 0xa50ab56b,
  0x35b5a8fa, 0x42b2986c, 0xdbbbc9d6, 0xacbcf940,
  0x32d86ce3, 0x45df5c75, 0xdcd60dcf, 0xabd13d59,
  0x26d930ac, 0x51de003a, 0xc8d75180, 0xbfd06116,
  0x21b4f4b5, 0x56b3c423, 0xcfba9599, 0xb8bda50f,
  0x2802b89e, 0x5f058808, 0xc60cd9b2, 0xb10be924,
  0x2f6f7c87, 0x58684c11, 0xc1611dab, 0xb6662d3d,
  0x76dc4190, 0x01db7106, 0x98d220bc, 0xefd5102a,
  0x71b18589, 0x06b6b51f, 0x9fbfe4a5, 0xe8b8d433,
  0x7807c9a2, 0x0f00f934, 0x9609a88e, 0xe10e9818,
  0x7f6a0dbb, 0x086d3d2d, 0x91646c97, 0xe6635c01,
  0x6b6b51f4, 0x1c6c6162, 0x856530d8, 0xf262004e,
  0x6c0695ed, 0x1b01a57b, 0x8208f4c1, 0xf50fc457,
  0x65b0d9c6, 0x12b7e950, 0x8bbeb8ea, 0xfcb9887c,
  0x62dd1ddf, 0x15da2d49, 0x8cd37cf3, 0xfbd44c65,
  0x4db26158, 0x3ab551ce, 0xa3bc0074, 0xd4bb30e2,
  0x4adfa541, 0x3dd895d7, 0xa4d1c46d, 0xd3d6f4fb,
  0x4369e96a, 0x346ed9fc, 0xad678846, 0xda60b8d0,
  0x44042d73, 0x33031de5, 0xaa0a4c5f, 0xdd0d7cc9,
  0x5005713c, 0x270241aa, 0xbe0b1010, 0xc90c2086,
  0x5768b525, 0x206f85b3, 0xb966d409, 0xce61e49f,
  0x5edef90e, 0x29d9c998, 0xb0d09822, 0xc7d7a8b4,
  0x59b33d17, 0x2eb40d81, 0xb7bd5c3b, 0xc0ba6cad,
  0xedb88320, 0x9abfb3b6, 0x03b6e20c, 0x74b1d29a,
  0xead54739, 0x9dd277af, 0x04db2615, 0x73dc1683,
  0xe3630b12, 0x94643b84, 0x0d6d6a3e, 0x7a6a5aa8,
  0xe40ecf0b, 0x9309ff9d, 0x0a00ae27, 0x7d079eb1,
  0xf00f9344, 0x8708a3d2, 0x1e01f268, 0x6906c2fe,
  0xf762575d, 0x806567cb, 0x196c3671, 0x6e6b06e7,
  0xfed41b76, 0x89d32be0, 0x10da7a5a, 0x67dd4acc,
  0xf9b9df6f, 0x8ebeeff9, 0x17b7be43, 0x60b08ed5,
  0xd6d6a3e8, 0xa1d1937e, 0x38d8c2c4, 0x4fdff252,
  0xd1bb67f1, 0xa6bc5767, 0x3fb506dd, 0x48b2364b,
  0xd80d2bda, 0xaf0a1b4c, 0x36034af6, 0x41047a60,
  0xdf60efc3, 0xa867df55, 0x316e8eef, 0x4669be79,
  0xcb61b38c, 0xbc66831a, 0x256fd2a0, 0x5268e236,
  0xcc0c7795, 0xbb0b4703, 0x220216b9, 0x5505262f,
  0xc5ba3bbe, 0xb2bd0b28, 0x2bb45a92, 0x5cb36a04,
  0xc2d7ffa7, 0xb5d0cf31, 0x2cd99e8b, 0x5bdeae1d,
  0x9b64c2b0, 0xec63f226, 0x756aa39c, 0x026d930a,
  0x9c0906a9, 0xeb0e363f, 0x72076785, 0x05005713,
  0x95bf4a82, 0xe2b87a14, 0x7bb12bae, 0x0cb61b38,
  0x92d28e9b, 0xe5d5be0d, 0x7cdcefb7, 0x0bdbdf21,
  0x86d3d2d4, 0xf1d4e242, 0x68ddb3f8, 0x1fda836e,
  0x81be16cd, 0xf6b9265b, 0x6fb077e1, 0x18b74777,
  0x88085ae6, 0xff0f6a70, 0x66063bca, 0x11010b5c,
  0x8f659eff, 0xf862ae69, 0x616bffd3, 0x166ccf45,
  0xa00ae278, 0xd70dd2ee, 0x4e048354, 0x3903b3c2,
  0xa7672661, 0xd06016f7, 0x4969474d, 0x3e6e77db,
  0xaed16a4a, 0xd9d65adc, 0x40df0b66, 0x37d83bf0,
  0xa9bcae53, 0xdebb9ec5, 0x47b2cf7f, 0x30b5ffe9,
  0xbdbdf21c, 0xcabac28a, 0x53b39330, 0x24b4a3a6,
  0xbad03605, 0xcdd70693, 0x54de5729, 0x23d967bf,
  0xb3667a2e, 0xc4614ab8, 0x5d681b02, 0x2a6f2b94,
  0xb40bbe37, 0xc30c8ea1, 0x5a05df1b, 0x2d02ef8d,
};


unsigned int crc32(char *p, int len)
{
  unsigned int crcinit = 0;
  unsigned int crc = 0;

  crc = crcinit ^ 0xFFFFFFFF;
  for (; len--; p++) {
    crc = ((crc >> 8) & 0x00FFFFFF) ^ crc32tab[(crc ^ (*p)) & 0xFF];
  }
  return crc ^ 0xFFFFFFFF;
}


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

void write_cmd_data(int fd, char *cmd_buffer, int cmd_byte_size, char *data_buffer, 
		    int data_byte_size){

  int transfer_size;
  int idx = 0;
  int status;


  transfer_size = cmd_byte_size + data_byte_size;


  for(idx=0; idx<cmd_byte_size; ++idx){
    tx_buf[idx] = cmd_buffer[idx];
    //printf("0x%x\n", tx_buffer[idx]);
  }

  for(idx=0; idx<data_byte_size; idx++){
    tx_buf[idx+cmd_byte_size] = data_buffer[idx];
    //printf("%d %d 0x%x\n", data_byte_size, idx, data_buffer[idx]);
  }

  wait_ready(fd);
  xfer[0].tx_buf = (unsigned long)tx_buf;
  xfer[0].len = transfer_size;
  status = ioctl(fd, SPI_IOC_MESSAGE(1), xfer);
  if (status < 0)
    {
      perror("SPI_IOC_MESSAGE");
      return;
    }
}


void spi_write(int fd, int start_addr, int end_addr, int offset_addr)
{
  int nbytes = end_addr - start_addr;

  int status; 
  int index=0;

  int in_buffer_idx;
  int nb_bytes_to_write;
  int target_addr;


  int i=0;

  printf("spi_data_writing ....\n");

  in_buffer_idx = 0;
  nb_bytes_to_write = 256;
  target_addr = start_addr + offset_addr; 
  
  while(in_buffer_idx < nbytes){

    //printf("data writing 0x%x/0x%x\n", in_buffer_idx, nbytes);


    //write enable 
    /*
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

    //Unprotect sector 
    wait_ready(fd);
    buf[0] = UNPROTECT_SECTOR_OPCODE;
    buf[1] = ((target_addr >> 16) & 0xFF);
    buf[2] = ((target_addr >> 8) & 0xFF);;
    buf[3] = (target_addr & 0xFF);
    xfer[0].tx_buf = (unsigned long)buf;
    xfer[0].len = 4;
    status = ioctl(fd, SPI_IOC_MESSAGE(1), xfer);
    if (status < 0)
      {
	perror("SPI_IOC_MESSAGE");
	return;
      }
    

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
    */
    
    //memset(data_buffer, 0, sizeof(data_buffer));
    //printf(".");
    int size_left;
    nb_bytes_to_write =256; // - (target_addr & 0xFF);
    size_left = nbytes - in_buffer_idx;
    if(size_left < nb_bytes_to_write){
      nb_bytes_to_write = size_left;
    }

    
   
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
    
    wait_ready(fd);
    //write data 
    buf[0] = PROGRAM_PAGE_CMD; //0x02;
    buf[1] = ((target_addr >> 16) & 0xFF);
    buf[2] = ((target_addr >> 8) & 0xFF);;
    buf[3] = (target_addr & 0xFF);
    
    for(i=0;i<nb_bytes_to_write; i++){
      data_buffer[i] = data_out_buffer[in_buffer_idx+i];
    }


    write_cmd_data(fd, buf, 4, data_buffer, nb_bytes_to_write);
    target_addr += nb_bytes_to_write;
    in_buffer_idx += nb_bytes_to_write;
  }

  wait_ready(fd);
  buf[0] = WRITE_DISABLE_CMD; //0x02;               
  xfer[0].tx_buf = (unsigned long)buf;
  xfer[0].len = 1;
  status = ioctl(fd, SPI_IOC_MESSAGE(1), xfer);
  if (status < 0)
    {
      perror("SPI_IOC_MESSAGE");
      return;
    }

  printf("spi_write done\n");
  //printf("ret: %02x %02x %02x %02x\n", buf2[0], buf2[1], buf2[2], buf2[3]);
 
}


void spi_erase_4k(int fd, int addr)
{

  int status; 
  int index=0;

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

  wait_ready(fd);
  //erase data 
  buf[0] = ERASE_4K_BLOCK_OPCODE; //0x20;
  buf[1] = ((addr >> 16) & 0xFF);
  buf[2] = ((addr >> 8) & 0xFF);;
  buf[3] = (addr & 0xFF);

  xfer[0].tx_buf = (unsigned long)buf;
  xfer[0].len = 4; /* Length of  command to write*/
  
  status = ioctl(fd, SPI_IOC_MESSAGE(1), xfer);
  if (status < 0)
    {
      perror("SPI_IOC_MESSAGE");
      return;
    }
}


void spi_erase(int fd, int start_addr, int end_addr, int offset_addr){

  int target_addr = start_addr + offset_addr;
  int size_deleted = 0;
  int size = end_addr - start_addr; 
  int default_size = 4096;
  int delete_size = default_size;


  printf("erase ....");

  while(size_deleted < size){
    //if(size - size_deleted < delete_size){
    //  delete_size = size - size_deleted;
    //}
    spi_erase_4k(fd, target_addr);

    target_addr += delete_size;
    size_deleted += delete_size;
  }
  printf("delete done\n");
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


int spi_data_read(int fd, int start_addr, int end_addr, int offset_addr){


  int size = end_addr - start_addr;
  int size_readout = 0;
  int target_addr = start_addr + offset_addr;
  int readout_size = buffer_size;
  int i=0;

  printf("spi_data_reading ... \n");

  while(size_readout <size){
    if(size - size_readout<buffer_size){
      readout_size = size - size_readout;
    }else{
      readout_size = buffer_size;
    }

    spi_read(fd, target_addr, readout_size);
    for(i=0; i<readout_size;i++){
      data_buffer[i+size_readout] = buf2[i];
    }
    target_addr += readout_size;
    size_readout += readout_size;
  }
  printf("spi_data_read done. readout size = 0x%x\n", size_readout);

  return size_readout;

}

void spi_data_verify(int fd, int start_addr, int end_addr, int offset_addr){

  int size = end_addr - start_addr;
  int size_readout = 0;
  int target_addr = start_addr + offset_addr;
  int readout_size = 256;
  int i;

  printf("spi_data_verifing ... \n");

  while(size_readout <size){
    if(size - size_readout< 256){
      readout_size = size - size_readout;
    }else{
      readout_size = 256;
    }

    spi_read(fd, target_addr, readout_size);

    ////buf2
    

    for(i=0; i<readout_size; i++){
      data_buffer[i] = data_out_buffer[size_readout+i];
    }
    
    for(i=0; i<readout_size; i++){
      //printf("0x%x, 0x%x\n", buf2[i], data_buffer[i]);
      if(buf2[i] != data_buffer[i]){
	printf("data doesn't match at data address 0x%x, data in flash = 0x%x, data in file = 0x%x\n",
	       i+size_readout+start_addr, buf2[i], data_buffer[i]);
	pabort("abort : do writing again!");
      }
    }   
    target_addr += readout_size;
    size_readout += readout_size;
  }
  printf("spi_data_verify done. data in the flash is verified!\n");
  


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
 

void crc_checker(void){


  int size = 0x1000; //buffer_size; 
  int i=0;
  int crc_calc;

  for(i=0;i<ENV_SIZE;i++){
    env->data[i] = data_buffer[i+ENV_HEADER_SIZE];
  }
  env->crc = data_buffer[3] << 24 | data_buffer[2] << 16 
    | data_buffer[1] << 8 | data_buffer[0];

  crc_calc = crc32(env->data, ENV_SIZE);
  printf("crc32 of the environment data in the SPI Flash : %x\n", env->crc);
  printf("re-calculated crc32 by this program : %x\n", crc_calc);

  if(crc_calc != env->crc){
    pabort("Crc32 in the Flash and recalculate from data doesn't match. No overwrite. \n");
  }
    
}


void copy_data_buffer(const int data_size){

  int i=0;
  int size = data_size;

  for(i=0;i<size;i++){
    data_out_buffer[i] = data_buffer[i];
  }

  printf("copy_data_buffer : size = 0x%x\n", size);

}

void write_data_buffer(const char *target, const char *new_data){
 
  int size = 0x1000; //buffer_size; 
  int i=0;
  unsigned int crc_calc = 0;
  int length = strlen(target);
  int j=0;
  int flag=0;
  int start_buffer=0;
  int start_data_buffer=0;
  int start_end_buffer=0;
  int crc_data=0;
  int cval;
  int offset=0;

  for(i=0;i<ENV_SIZE;i++){
    env->data[i] = data_buffer[i+ENV_HEADER_SIZE];
  }
  env->crc = data_buffer[3] << 24 | data_buffer[2] << 16 
    | data_buffer[1] << 8 | data_buffer[0];

  crc_calc = crc32(env->data, ENV_SIZE);
  printf("crc32 of the environment data in the SPI Flash : %x\n", env->crc);
  printf("re-calculated crc32 by this program : %x\n", crc_calc);

  if(crc_calc != env->crc){
    pabort("Crc32 in the Flash and recalculate from data doesn't match. No overwrite. \n");
  }
    
  //////
  printf(" write the variable = %s (len=%d) data = %s (len=%d)\n", target, length, new_data, strlen(new_data));
  printf("crc = 0x%x\n", env->crc);
  flag = 0;
  for(i=4;i<size;i++){
    if(data_buffer[i]!=target[0]){
      continue;
    }else{
      flag=1;
      for(j=1;j<length;j++){
	if(data_buffer[i+j]!=target[j]){
	  flag = 0;
	  break;
	}
      }
      if(flag==1){
	if(data_buffer[i+length]==0x3d){ /// search for "="
	  start_buffer = i;
	  start_data_buffer = i+length+1;
	  break;
	}
      }
    }
  }

  //printf(" debug... flag=%d, start_addr = 0x%x, start_data_addr=0x%x, end_addr=0x%x\n",
  //flag, start_buffer, start_data_buffer, start_end_buffer);

 
  if(flag==1){
    for(i=start_buffer ; i<start_buffer+0x1000; i++){
      if(data_buffer[i]==0x0){
	start_end_buffer = i;
	printf("\n");
	break;
      }else{
	/*
	if(data_buffer[i]==0x3d){
	  start_data_buffer = i+1;
	}
	*/
	printf("%c",data_buffer[i]);
      }
    }
  }else{
    start_buffer=0;
    start_data_buffer=0;
    start_end_buffer=0;
  }

  ////address map summary
  //// start_buffer --> start address of variable name 
  //// start_data_buffer --> start address of data value
  //// start_end_buffer --> location of 0x00
  
  printf(" debug... flag=%d, start_addr = 0x%x, start_data_addr=0x%x, end_addr=0x%x\n",
  	 flag, start_buffer, start_data_buffer, start_end_buffer);
  printf(" ######## new environment ########\n");


  if(flag==1){
    for(i=0;i<start_data_buffer;i++){
      data_out_buffer[i] = data_buffer[i];
    }
    for(i=start_data_buffer; i< start_data_buffer + strlen(new_data); i++){
      data_out_buffer[i] = (uint8_t) new_data[i-start_data_buffer];
    }
    for(i=start_data_buffer + strlen(new_data); i<size;i++){
      data_out_buffer[i] = data_buffer[i-start_data_buffer-strlen(new_data)+start_end_buffer];
    }

  }else{ 
    for(i=0;i<size;i++){
      if(data_buffer[i]==0x00 && data_buffer[i+1]==0x00){
	start_buffer = i+1;
	break;
      }
    }
    for(i=0;i<start_buffer;i++){
      data_out_buffer[i] = data_buffer[i];
    } 

    printf(" No such variables in the Flash... Do you want to create? [Y/n] ");
    do{
      cval = getchar();
    }while(cval!=0x59 && cval!=0x6e);
    
    
    if(cval==0x6e){ /// No writing 
      for(i=start_buffer;i<size;i++){
	data_out_buffer[i] = data_buffer[i];
      }       
    }else if(cval==0x59){ ///New writing

      offset = start_buffer;
      for(i=offset;i<offset+strlen(target);i++){
	data_out_buffer[i] = target[i-offset];
      }
      offset += strlen(target);

      data_out_buffer[offset] = 0x3d;
      offset++;

      for(i=offset; i<offset+strlen(new_data);i++){
	data_out_buffer[i] = new_data[i-offset];
      }
      offset+= strlen(new_data);;


      for(i=offset;i<size; i++){
       	data_out_buffer[i] = data_buffer[i];
      }
    }
  } 
    
  //////

  for(i=0;i<ENV_SIZE;i++){
    env->data[i] = data_out_buffer[i+ENV_HEADER_SIZE];
  }

  crc_calc = crc32(env->data, ENV_SIZE);
  printf("new crc32  : %x\n", crc_calc);

  //// overwrite crc bit
  data_out_buffer[0] = crc_calc & 0xFF;
  data_out_buffer[1] = (crc_calc >> 8)  & 0xFF;
  data_out_buffer[2] = (crc_calc >> 16)  & 0xFF;
  data_out_buffer[3] = (crc_calc >> 24)  & 0xFF;


  ///// dump
  for(i=4;i<size;i++){
    if(data_out_buffer[i]==0x00){
      printf("\n");
    }else{
      printf("%c", data_out_buffer[i]);
    }
    if(data_out_buffer[i]==0x00 && data_out_buffer[i+1]==0x00){
      break;
    }
  }
  printf(" ################################ \n");

  //////
}

void print_data(const char *target){

  int size = 0x1000; //buffer_size; 
  int i=0;
  int j=0;
  int flag=0;
  int crc_data =0;
  printf(" ***** eNVM boot environment ***** \n");
  int length = strlen(target);
  int start_buffer=0;

  crc_data = data_buffer[3] << 24 | data_buffer[2] << 16 
    | data_buffer[1] << 8 | data_buffer[0];

  if(strcmp(target,"all")==0){
    printf("crc = 0x%x\n", crc_data);
    for(i=4;i<size;i++){
      if(data_buffer[i]==0x00){
	printf("\n");
      }else{
	printf("%c", data_buffer[i]);
      }
      if(data_buffer[i]==0x00 && data_buffer[i+1]==0x00){
	break;
      }
    }
  }else{
    printf(" dump the variable = %s (len=%d) \n", target, length);
    printf("crc = 0x%x\n", crc_data);
    flag = 0;
    for(i=4;i<size;i++){
      if(data_buffer[i]!=target[0]){
	continue;
      }else{
	if(data_buffer[i+length]==0x3d){
	  flag = 1;
	  start_buffer = i;
	}
      }
      for(j=1;j<length;j++){
	if(data_buffer[i+j]!=target[j]){
	  flag = 0;
	  break;
	}
      }

      if(flag==1){
	for(i=start_buffer ; i<start_buffer+0x1000; i++){
	  if(data_buffer[i]==0x0){
	    printf("\n");
	    break;
	  }else{
	    printf("%c",data_buffer[i]);
	  }
	}
      }
    }
  }

  printf(" ********************************** \n");

}

void load_default(int cs){
  
  unsigned int crc_calc;
  int i;

  char default_environment[]={
    "bootargs=m2s_platform=rcu-2.0 console=ttyS0,115200 panic=10 user_debug=31\0" \
    "bootdelay=3\0"							\
    "baudrate=115200\0"							\
    "hostname=rcu-2.0\0"						\
    "loadaddr=0xA0007FC0\0"						\
    "serverip=10.160.33.95\0"						\
    "image=rculinux.uImage\0"						\
    "spiaddr=0x10000\0"							\
    "spisize=400000\0"							\
    "spiprobe=sf probe 0\0"						\
    "addip=setenv bootargs ${bootargs}"					\
    " ip=${ipaddr}:${serverip}:${gatewayip}:"				\
    "${netmask}:${hostname}:eth0:off\0"					\
    "linux_spi0=wrlv 0; loadenv 0; run addip; sf probe 0; sf read ${loadaddr} ${spiaddr} ${spisize}; bootm ${loadaddr}\0" \
    "linux_spi1=wrlv 0; loadenv 1; run addip; sf probe 1; sf read ${loadaddr} ${spiaddr} ${spisize}; bootm ${loadaddr}\0" \
    "linux_net=wrlv 1; tftp ${loadaddr} ${image};run addip;bootm\0"	\
    "flashboot=run memtest; run linux_spi0; run linux_net; run linux_spi1; run linux_net; loadenv 2; run linux_net\0" \
    "netboot=run memtest; loadenv 0; run linux_net; loadenv 1; run linux_net; loadenv 2; run linux_net; run linux_spi0; run linux_spi1\0" \
    "memtest=mwr 0xA0000000 0xB0000000 0\0"				\
    "ISP_PROG_ADDR=0x410000\0"						\
    "ISP_PROG_SIZE=2418896\0"						\
    "DDL2_CONFIG_SPEED=2\0"						\
    "ispprog=isp apv\0"							\
    "ispauth=isp a; reset\0"						\
    "update=tftp ${loadaddr} ${image};run spiprobe;"			\
    "sf erase ${spiaddr} ${spisize};"					\
    "sf write ${loadaddr} ${spiaddr} ${filesize}\0"			\
    "bootcmd=run netboot\0"						\
      "ipaddr=10.1.5.200\0"						\
    "ethaddr=00:50:C2:DB:2C:9F\0"					\
    "stdin=serial\0"							\
    "stdout=serial\0"							\
    "stderr=serial\0"							\
    "ethact=M2S_MAC\0"      
    };




  ////////////
  memcpy(env->data, default_environment, sizeof(default_environment));
  crc_calc = crc32(env->data, ENV_SIZE);
  
  printf("calculated crc32 by this program : %x\n", crc_calc);

  //// overwrite crc bit
  data_out_buffer[0] = crc_calc & 0xFF;
  data_out_buffer[1] = (crc_calc >> 8)  & 0xFF;
  data_out_buffer[2] = (crc_calc >> 16)  & 0xFF;
  data_out_buffer[3] = (crc_calc >> 24)  & 0xFF;
  
  for(i=0;i<ENV_SIZE;i++){
    data_out_buffer[i+ENV_HEADER_SIZE] = env->data[i];
  }

  printf(" ******* \n");
  for(i=4; i<4+sizeof(default_environment); i++){
    if(data_out_buffer[i]==0x0){
      printf("\n");
    }else{
      printf("%c",data_out_buffer[i]);
    }
  }
  printf(" ******* \n");
  ///////////
}

void load_from_file(void){
  
  unsigned int crc_calc;
  int i;
  char lines[256];
  int size=0;
  

  FILE *fin = NULL;

  fin = fopen(env_filename,"r");
  if(fin==NULL){
    perror("No Env input file\n");
    return;
  }

  printf("read from = %s\n", env_filename);

  memset(env->data, 0x0, ENV_SIZE);

  while(fgets(lines, 256, fin)!=NULL){
   //printf("%s", lines);
    for(i=0;i<strlen(lines);i++){
      env->data[size] = lines[i];
      if(env->data[size] == 0x0A){
	env->data[size] = 0x00;
      }
      size++;
    }
  }
  fclose(fin);

  crc_calc = crc32(env->data, ENV_SIZE);

  //// overwrite crc bit
  data_out_buffer[0] = crc_calc & 0xFF;
  data_out_buffer[1] = (crc_calc >> 8)  & 0xFF;
  data_out_buffer[2] = (crc_calc >> 16)  & 0xFF;
  data_out_buffer[3] = (crc_calc >> 24)  & 0xFF;
  
  for(i=0;i<ENV_SIZE;i++){
    data_out_buffer[i+ENV_HEADER_SIZE] = env->data[i];
  }

  printf(" ******* \n");
  for(i=ENV_HEADER_SIZE; i<ENV_HEADER_SIZE+size; i++){
    if(data_out_buffer[i]==0x0){
      printf("\n");
    }else{
      printf("%c",data_out_buffer[i]);
    }
  }
  printf(" ******** \n");

}






void print_usage(const char *prog)
{
  printf("Usage: %s --cs=[0,1] --list\n", prog);
  printf("Usage: %s --cs=[0,1] --delete\n", prog);
  printf("Usage: %s --cs=[0,1] --write_default\n", prog);
  printf("Usage: %s --cs=[0,1] --write_default --file=[filename] \n", prog);
  printf("Usage: %s --cs=[0,1] --write [variable_name] [new value]\n", prog);
  printf("Usage: %s --cs=[0,1] --read [variable_name]\n", prog);
  printf("Usage: %s --cs=[0,1] --copy (cs=0: copy from 0 to 1, cs=1: copy from 1 to 0)\n", prog);
  exit(1);
}

void parse_opts(int argc, char *argv[])
{

  if(argc!=3 && argc!=4 && argc!=5){
    print_usage(argv[0]);
  }else{
    if(argc==3){
      if(strcmp(argv[1],"--cs=0")==0){
	chip_select=0;
      }else if(strcmp(argv[1],"--cs=1")==0){ 
	chip_select=1;
      }else{
	print_usage(argv[0]);
      }

      if(strcmp(argv[2],"--list")==0){
	command_id = 0;
      }else if(strcmp(argv[2],"--copy")==0){
	command_id = 3;
      }else if(strcmp(argv[2],"--write_default")==0){
	command_id = 4;
      }else if(strcmp(argv[2],"--delete")==0){
	command_id = 6;
      }else{
	print_usage(argv[0]);
      }

    }else if(argc==5){
      if(strcmp(argv[1],"--cs=0")==0){
	chip_select=0;
      }else if(strcmp(argv[1],"--cs=1")==0){ 
	chip_select=1;
      }else{
	print_usage(argv[0]);
      }

      if(strcmp(argv[2],"--write")==0){
	command_id = 1;
	sscanf(argv[3], "%s", target_var);
	sscanf(argv[4], "%[a-zA-Z0123456789.:- ;${}_,=]", write_data);
      }else{
	print_usage(argv[0]);
      }
    }else if(argc==4){
      if(strcmp(argv[1],"--cs=0")==0){
	chip_select=0;
      }else if(strcmp(argv[1],"--cs=1")==0){ 
	chip_select=1;
      }else{
	print_usage(argv[0]);
      }

      if(strcmp(argv[2],"--read")==0){
	command_id = 2;
	sscanf(argv[3], "%s", target_var);
      }else if(strcmp(argv[2],"--write_default")==0){
	command_id = 5;
	sscanf(argv[3], "--file=%s", env_filename);
      }else{
	print_usage(argv[0]);
      }
    }
  }

  start_address = 0x0;
  end_address = 0x1000;
  offset_address = 0x0;
  
  

  printf(" ############### \n");
  printf("chip select = 0x%x\n", chip_select);
  printf("stat address = 0x%x\n", start_address);
  printf("end address = 0x%x\n", end_address);
  printf("offset address for the SPI = 0x%x\n", offset_address);
  if(command_id==1){
    printf("variable = %s\n", target_var);
    printf("data = %s\n", write_data);
  }else if(command_id==5){
    printf("env input file = %s\n", env_filename);
  }
  
  printf(" ############### \n");
 
}


int spi_flash_probe(int fd){

  int ret=0;

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


  return ret;
}


int main(int argc, char *argv[])
{
  int ret = 0;
  int fd;
  int size = 0;
  int charval = 0;
  parse_opts(argc, argv);


  if(chip_select==0){
    printf("access to dev = %s\n", device0);
    fd = open(device0, O_RDWR);
    if (fd < 0)
      pabort("can't open device");
  }else if(chip_select==1){
    printf("access to dev = %s\n", device1);
    fd = open(device1, O_RDWR);
    if (fd < 0)
      pabort("can't open device");
  }else{
    pabort("no proper chip_select. exit...\n");
  }


  spi_flash_probe(fd);


  env = malloc(sizeof(struct environment_s));
  if(!env){
    printf("malloc of env failed\n");
    return 1;
  }

  verify(fd);
  spi_global_unprotect(fd);
  int rd_size = spi_data_read(fd, start_address, end_address, offset_address);

  if(command_id==0){  ///list
    print_data("all");
    crc_checker();
  }else if(command_id==6){ ///delte 
    spi_erase(fd, start_address, end_address, offset_address);
  }else if(command_id==3){ ///copy

    print_data("all");
    crc_checker();
    
    ///close device
    close(fd);
    ///open the other deivce
    if(chip_select==0){
      printf("TARGET: access to dev = %s\n", device1);
      fd = open(device1, O_RDWR);
      if (fd < 0)pabort("can't open device");
    }else if(chip_select==1){
      printf("TARGET: access to dev = %s\n", device0);
      fd = open(device0, O_RDWR);
      if (fd < 0)pabort("can't open device");
    }else{
      pabort("no proper chip_select. exit...\n");
    }
    

    spi_flash_probe(fd);
    
    ////////////////////
    copy_data_buffer(rd_size);

    printf(" Above lists are correct? Do you want to write? [Y/n] ");
    do{
      charval = getchar();
    }while(charval!=0x59 && charval!=0x6e);

    if(charval==0x59){ ///New writing
      spi_erase(fd, start_address, end_address, offset_address);
      spi_write(fd, start_address, end_address, offset_address);
      spi_data_verify(fd, start_address, end_address, offset_address);
    }else{
      printf("Abort... No update of uBoot environment..\n");
    }

  }else if(command_id==2){ ///read
    print_data(target_var);
  }else if(command_id==1){ ///write
    write_data_buffer(target_var, write_data);
    printf(" Above lists are correct? Do you want to write? [Y/n] ");
    do{
      charval = getchar();
    }while(charval!=0x59 && charval!=0x6e);

    if(charval==0x59){ ///New writing
      spi_erase(fd, start_address, end_address, offset_address);
      spi_write(fd, start_address, end_address, offset_address);
      spi_data_verify(fd, start_address, end_address, offset_address);
    }else{
      printf("Abort... No update of uBoot environment..\n");
    }
  }else if(command_id==4 || command_id==5){ //default_write
    if(command_id==4){
      load_default(chip_select);
    }else{
      load_from_file();
    }
    printf(" Above lists are correct? Do you want to write? [Y/n] ");
    do{
      charval = getchar();
    }while(charval!=0x59 && charval!=0x6e);

    if(charval==0x59){ ///New writing
      spi_erase(fd, start_address, end_address, offset_address);
      spi_write(fd, start_address, end_address, offset_address);
      spi_data_verify(fd, start_address, end_address, offset_address);
    }else{
      printf("Abort... No update of uBoot environment..\n");
    }
  }
  close(fd);
  free(env);
  printf("spidev_uboot is successfully done.\n");
  return ret;
}

