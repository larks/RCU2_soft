/*
 * This is a user-space application that reads /dev/sample
 * and prints the read characters to stdout
 */

#include<stdint.h>
#include<unistd.h>
#include<stdio.h>
#include <stdlib.h>
#include<sys/types.h>
#include<linux/types.h>
#include<sys/mman.h>
#include<fcntl.h>
#include <sys/ioctl.h>
#include <string.h>
#include <errno.h>


unsigned int parseBinary(char *str) {
  unsigned int val = 0;
  
  if (*str == 'b') {
    str++;
    while (*str) {
      if (*str == '0') {
        val <<= 1;
      } else if (*str == '1') {
        val = (val << 1) + 1;
      } else {
        goto binaryError;
      }
    }
  }
  return val;
 binaryError:
  fprintf(stderr,"Unrecognized numeric value: %s\n",str);
  return 999;
}

unsigned int parseNumber(char *str) {
  unsigned int addr = 0;

  if (!sscanf(str, "0x%x", &addr)) {
    if (!sscanf(str, "%u", &addr)) {
      addr = parseBinary(str);
    }
  }
  return addr;
}


int main(int argc, char **argv)
{
  char * app_name = argv[0];
  char * dev_name = "/dev/envm";
  int ret = -1;
  int  fd=-1 ;

  int dst;
  int file_size=0;
  int rd_size=0;
  FILE *infile;
  char *data_buffer;
  char *data_rd_buffer;
  int bad_count=0;
  int i, x;
  /*
   * Check that at least the destination is specified
   */
  if (argc != 4) {
    printf("%s: [wr/rd] [offset] [eNVM file]\n",
           (char *) argv[0]);
    return ret;
  }
  
  /*
   * Parse the command arguments
   */
  char * cmd =  argv[1];
  int doread=0;
  int dowrite=0;
  if(strcmp(cmd,"rd")==0){
    doread=1;
  }
  if(strcmp(cmd,"wr")==0){
    dowrite=1;
  }


  dst = parseNumber(argv[2]); ///offset address 
  infile = fopen(argv[3],"rb");
  if(infile==NULL){
    printf("no input file \n");
  }

  ////open eNVM file and write to the DDR memory 

  fseek(infile, 0, SEEK_END );
  file_size = ftell(infile);
  
  data_buffer = (char*) malloc(file_size);
  data_rd_buffer = (char*) malloc(file_size);

  fseek(infile, 0, SEEK_SET);
  rd_size = fread(data_buffer, sizeof(char), file_size, infile);

  printf("dst=0x%x, File_size = 0x%x, data loc %p, readout size=0x%x\n", dst, file_size, data_buffer, rd_size);

  /*
   * Open the sample device RD | WR
   */
  if ((fd = open(dev_name, O_RDWR)) < 0) {
  //if ((fd = fopen(dev_name, "wb+")) < 0) {
    fprintf(stderr, "%s: unable to open %s: %s\n", 
	    app_name, dev_name, strerror(errno));
    goto Done;
  }
  
  /*
   * Read the sample device byte-by-byte
   */
  if ((x = lseek(fd, dst, SEEK_SET)) < 0) {
    fprintf(stderr, "%s: unable to seek %s: %s\n", 
	  app_name, dev_name, strerror(errno));
    goto Done;
  }


  if(dowrite==1){
    if ((x = write(fd, (char*)data_buffer, rd_size)) < 0) {
      fprintf(stderr, "%s: unable to write %s: %s\n", 
	      app_name, dev_name, strerror(errno));
      goto Done;
    }	
  }



  if(doread==1){
    if ((x = read(fd, data_rd_buffer, rd_size)) < 0) {
      fprintf(stderr, "%s: unable to read %s: %s\n", 
	      app_name, dev_name, strerror(errno));
      goto Done;
    }	
    for(i=0;i<rd_size;i++){
      if(data_rd_buffer[i] != data_buffer[i]){
	printf("data mismatch: data = 0x%x, written data = 0x%x at 0x%x\n", 
	       data_buffer[i], data_rd_buffer[i], i);
	bad_count++;
      }
    }
    printf("readback results. error = 0x%x\n", bad_count);
  }

  
 Done:
  free(data_buffer);
  free(data_rd_buffer);
  if (fd >= 0) {
    close(fd);
  }

  return ret;
}
