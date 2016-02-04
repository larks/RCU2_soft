#include<unistd.h>
#include<sys/types.h>
#include<sys/mman.h>
#include<stdio.h>
#include<fcntl.h>




int main(int argc, char **argv) {


  char *buf[1000];
  int i;
  int num=0;
  for(i=0;i<1000;i++){
    buf[i] = (char*)malloc(100);
    
    if(buf[i]==NULL){
      printf("malloc fail\n");
    }


    if(buf[i]>=0xA1000000 && buf[i]<0xA1367ae0){
      printf(" ***** address %d --> %p \n", i, buf[i]);
    }else{
      printf("  address %d --> %p \n", i, buf[i]);
      num++;
    }

  }

  for(i=0;i<1000;i++){
    free(buf[i]);
  }

  printf(" number of mallocs outside 0xA100.0000 - 0xA136.7ae0 : %d \n", num);
  return 0;

}
