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

struct spi_ioc_transfer xfer[2];
char buf[1024+4];
char tx_buf[1024+4];
char buf2[1024+4];
const int buffer_size=1024;
char data_buffer[1024];

static FILE *infile;
static FILE *outfile;
static int start_address;
static int end_address;
static int offset_address;

static char action[32] = "read";

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
static uint32_t speed = 5000000;
static uint16_t delay;
static uint8_t chip_select; 

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

void wait_ready(int fd, int debug)
{
  int status;
  uint8_t ready_bit;
  buf[0] = READ_STATUS;

  if(debug){
    printf("wait_ready ....");
  }

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

  if(debug){
    printf("....ok!\n");
  }
//printf("*** %d %02x \n", status, buf2[0]);
//  printf("*** %d %02x \n", status, buf2[1]);

}


void spi_global_unprotect(int fd)
{
  int status;


  //write enable 
  wait_ready(fd,0);
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
  wait_ready(fd,0);
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

  wait_ready(fd,0);
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

    if(in_buffer_idx%256==0){
      printf("data writing 0x%x/0x%x\n", in_buffer_idx, nbytes);
    }

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
    fseek(infile, in_buffer_idx + start_addr, SEEK_SET);
    fread(data_buffer, sizeof(char), nb_bytes_to_write, infile);
    //if(outfile!=NULL){
    //  fwrite(data_buffer, sizeof(unsigned char), nb_bytes_to_write, outfile );
    //}

   
    wait_ready(fd,0); //(fd, 1);
    buf[0] = WRITE_ENABLE_CMD; //0x06;
    xfer[0].tx_buf = (unsigned long)buf;
    xfer[0].len = 1;
    status = ioctl(fd, SPI_IOC_MESSAGE(1), xfer);
    if (status < 0)
      {
	perror("SPI_IOC_MESSAGE");
	return;
      }
    
    wait_ready(fd,0); //(fd,1)
    //write data 
    buf[0] = PROGRAM_PAGE_CMD; //0x02;
    buf[1] = ((target_addr >> 16) & 0xFF);
    buf[2] = ((target_addr >> 8) & 0xFF);;
    buf[3] = (target_addr & 0xFF);
    
    //for(i=0;i<nb_bytes_to_write;i++){
    //  printf("%d, %d, 0x%x\n", in_buffer_idx, nb_bytes_to_write, data_buffer[i]);
    //}

    write_cmd_data(fd, buf, 4, data_buffer, nb_bytes_to_write);

    target_addr += nb_bytes_to_write;
    in_buffer_idx += nb_bytes_to_write;
  }

  wait_ready(fd,0);
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
  wait_ready(fd,0);
  buf[0] = WRITE_ENABLE_CMD; //0x06;
  xfer[0].tx_buf = (unsigned long)buf;
  xfer[0].len = 1;
  status = ioctl(fd, SPI_IOC_MESSAGE(1), xfer);
  if (status < 0)
  {
    perror("SPI_IOC_MESSAGE");
    return;
  }

  wait_ready(fd,0);
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

  if(outfile!=NULL){
    fwrite(buf2, sizeof(unsigned char), nbytes, outfile );
  }
  //printf("env: %02x %02x %02x\n", buf[0], buf[1], buf[2]);
  //for(index=0; index<nbytes; index++){
  //printf("%d : buf2[%d] = ret: %02x \n", status, index, buf2[index]);
  //}

  //return buf2;
}


void spi_data_read(int fd, int start_addr, int end_addr, int offset_addr){


  int size = end_addr - start_addr;
  int size_readout = 0;
  int target_addr = start_addr + offset_addr;
  int readout_size = buffer_size;


  printf("spi_data_reading ... \n");

  while(size_readout <size){
    if(size - size_readout<buffer_size){
      readout_size = size - size_readout;
    }else{
      readout_size = buffer_size;
    }

    spi_read(fd, target_addr, readout_size);
    target_addr += readout_size;
    size_readout += readout_size;
  }
  printf("spi_data_read done\n");

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
    
    fseek(infile, size_readout + start_addr, SEEK_SET);
    fread(data_buffer, sizeof(char), readout_size, infile);
    
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
 

void print_usage(const char *prog)
{
  printf("usage: %s --write --cs={0,1} [start address] [end address] [SPI address offset] [input file] [clock speed in MHz]\n", prog);
  printf("usage: %s --read --cs={0,1} [start address] [end address] [SPI address offset]  [output file] [clock speed in MHz]\n", prog);
  printf("usage: %s --delete --cs={0,1} [start address] [end address] [SPI address offset] [clock speed in MHz]\n", prog);
  exit(1);
}

void parse_opts(int argc, char *argv[])
{

  int charval = 0;

  sscanf(argv[1], "%s", action);
  if(strcmp(action, "--write")==0){
    if(argc==8){
      if(strcmp(argv[2],"--cs=0")==0){
	chip_select=0;
      }else if(strcmp(argv[2],"--cs=1")==0){ 
	chip_select=1;
      }else{
	print_usage(argv[0]);
      }
      sscanf(argv[3],"0x%x", &start_address);
      sscanf(argv[4],"0x%x", &end_address);
      sscanf(argv[5],"0x%x", &offset_address);
      infile = fopen(argv[6],"rb");
      speed  = 1000000*atoi(argv[7]);
      printf(" ############### \n");
      printf("action : %s\n", action);
      printf("chip_select : %d\n", chip_select);
      printf("start address = 0x%x\n", start_address);
      printf("end address = 0x%x\n", end_address);
      printf("offset address for the SPI = 0x%x\n", offset_address);
      printf("io file = %s\n", argv[6]);
      printf("SPI speed in Hz = %d\n", speed);
      printf(" ############### \n");
    }else{
      print_usage(argv[0]);
    }
  }else if(strcmp(action,"--read")==0){
    if(argc==8){
      if(strcmp(argv[2],"--cs=0")==0){
	chip_select=0;
      }else if(strcmp(argv[2],"--cs=1")==0){ 
	chip_select=1;
      }else{
	print_usage(argv[0]);
      }
      sscanf(argv[3],"0x%x", &start_address);
      sscanf(argv[4],"0x%x", &end_address);
      sscanf(argv[5],"0x%x", &offset_address);
      outfile = fopen(argv[6],"wb");
      speed  = 1000000*atoi(argv[7]);
      printf(" ############### \n");
      printf("action : %s\n", action);
      printf("chip_select : %d\n", chip_select);
      printf("start address = 0x%x\n", start_address);
      printf("end address = 0x%x\n", end_address);
      printf("offset address for the SPI = 0x%x\n", offset_address);
      printf("io file = %s\n", argv[6]);
      printf("SPI speed in Hz = %d\n", speed);
      printf(" ############### \n");
    }else{
      print_usage(argv[0]);
    }
  }else if(strcmp(action,"--delete")==0){
    if(argc==7){
      if(strcmp(argv[2],"--cs=0")==0){
	chip_select=0;
      }else if(strcmp(argv[2],"--cs=1")==0){ 
	chip_select=1;
      }else{
	print_usage(argv[0]);
      }
      sscanf(argv[3],"0x%x", &start_address);
      sscanf(argv[4],"0x%x", &end_address);
      sscanf(argv[5],"0x%x", &offset_address);
      speed  = 1000000*atoi(argv[6]);
      printf(" ############### \n");
      printf("action : %s\n", action);
      printf("chip_select : %d\n", chip_select);
      printf("start address = 0x%x\n", start_address);
      printf("end address = 0x%x\n", end_address);
      printf("offset address for the SPI = 0x%x\n", offset_address);
      printf("SPI speed in Hz = %d\n", speed);
      printf(" ############### \n");
    }else{
      print_usage(argv[0]);
    }
  }else{
    print_usage(argv[0]);
  }


  printf(" Above lists are correct? Do you want to proceed? [Y/n] ");
  do{
    charval = getchar();
  }while(charval!=0x59 && charval!=0x6e);
  
  if(charval==0x6e){ ///n 
    printf("Abort... No update in the SPI Flash..\n");
    exit(1);
  }
 
}


int main(int argc, char *argv[])
{
  int ret = 0;
  int fd;
  int size = 0;

  parse_opts(argc, argv);

  if(chip_select==0){
    fd = open(device0, O_RDWR);
    if (fd < 0)
      pabort("can't open device");
  }else if(chip_select==1){
    fd = open(device1, O_RDWR);
    if (fd < 0)
      pabort("can't open device");
  }else{
    pabort("no chip select... exit...");
  }

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

  if (strcmp(action,"--write")==0) {
    fseek(infile, 0, SEEK_END );
    size = ftell( infile ) ;
    printf("size = 0x%x\n", size);
    if(end_address > size){
      end_address = size;
      printf(" end_address > file size --> end_address is reset = 0x%x\n", end_address);
    }
    printf(" ############### \n");
    spi_erase(fd, start_address, end_address, offset_address);
    spi_write(fd, start_address, end_address, offset_address);
    spi_data_verify(fd, start_address, end_address, offset_address);
  }else if(strcmp(action, "--read")==0){
    spi_data_read(fd, start_address, end_address, offset_address);
  }else if(strcmp(action, "--delete")==0){
    spi_erase(fd, start_address, end_address, offset_address);
  }


  if ( infile != NULL )
    fclose( infile );
  if ( outfile != NULL ){
    fclose( outfile );
  }


  close(fd);

  return ret;
}


