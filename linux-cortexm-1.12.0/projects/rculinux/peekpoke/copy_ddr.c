#include<unistd.h>
#include<sys/types.h>
#include<sys/mman.h>
#include<stdio.h>
#include<fcntl.h>


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


  off_t offset = (0x600000 >> 2);
  unsigned int *start_r1 = parseNumber(argv[1]); 
  unsigned int *end_r1  = parseNumber(argv[2]);

  unsigned int *addr = (unsigned int*)start_r1;
  unsigned int *target_addr = (unsigned int*)start_r1;
  int cycle=0;

  printf("copy DDR memory : from %p to %p \n", start_r1, end_r1);

  for(addr = start_r1; addr<end_r1; addr++){
    target_addr = addr + offset;
    *target_addr = *addr;
    if(*target_addr != *addr){
      printf(" 0x%p=0x%x - 0x%p=0x%x\n", addr, *addr, target_addr, *target_addr);
    }
  }
  
  printf(" Done \n");

}
