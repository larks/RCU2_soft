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

  unsigned int addr1 = parseNumber(argv[1]);
  unsigned int addr2 = parseNumber(argv[2]);


  unsigned int *start_r1 = (unsigned int*)addr1;
  unsigned int *end_r1 = (unsigned int*)addr2;
  unsigned int *addr = (unsigned int*)start_r1;

  printf("dump DDR memory (!=0x0) from 0x%x to 0x%x \n", addr1, addr2);

  for(addr = start_r1; addr<end_r1; addr++){
    //if(*addr!=0x0){
    //printf("%p 0x%x\n", addr, *addr);
    //}
    printf("write %p 0x%x", addr, *addr);
    *addr = 0xdeadbeaf;
    printf(" --> 0x%x\n", *addr);
  }
  
  // printf(" Done \n");

}
