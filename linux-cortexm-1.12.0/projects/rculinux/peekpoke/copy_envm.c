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
  exit(0);
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


int main(int argc, char **argv) {


  unsigned int *start_r1 = parseNumber(argv[1]); 
  unsigned int *addr;
  FILE *infile;
  int file_size=0;
  unsigned int *data_buffer;

  infile = fopen(argv[2],"rb");
  if(infile==NULL){
    printf("no input file \n");
  }


  fseek(infile, 0, SEEK_END );
  file_size = ftell(infile);

  data_buffer = (char*) malloc(file_size);

  fseek(infile, 0, SEEK_SET);
  fread(data_buffer, sizeof(unsigned int), file_size, infile);

  int size=0;
  for(addr = start_r1; addr<start_r1+file_size; addr++){
    *addr = data_buffer[size];
    printf("write %p value = 0x%x\n", addr, *addr);
    size++;
  }
  
  printf(" Done \n");

}
