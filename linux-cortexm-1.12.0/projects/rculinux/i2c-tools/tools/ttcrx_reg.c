#include <stdio.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <sys/time.h>
#include <linux/types.h>
#include <errno.h>
#include <sys/poll.h> 
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <linux/i2c-dev.h>
#include "i2cbusses.h"


#define BCDDEZ(val) ((val & 0xf0) >> 4) * 10 + (val & 0xf)
#define DEZBCD(val) (((val) / 10) << 4) + ((val) % 10)
//#define TESTMODE
const char *i2cdev="/dev/i2c-0"; 
int fd_i2c;

const int i2caddr_pointer = 0x64;
const int i2caddr_data = 0x65;

#define TTCRX_RST (1<<5)        /* new (TK): 1: reset TTCRX, 0: operate TTCRX*/
#define TTCRX_SEL (1<<4)        /* new (TK): 0: use L1Accept sig as trigger(tristate) */
                                /*           1: use sw-pretrigger sig(activate driver) */

//////
int check_i2c(int i2caddr);
int detect(int i2caddr);
int read_register(int regnum) ;
int write_register(int regnum, int value) ;
int set_trigger_mode(int fd, int val);
int opendev(const char *filep) ;
int read_event_counter(void);
int ttcrxready(void) ;
int ttcrx_reset(void);

///////////////////////////////////////////////////
//
//  Check a I2C Address
//
int check_i2c(int i2caddr) {
  int verbose=0;
  int ret;
  int reg;
  int fd;

  if (verbose > 1)
    printf("Probing for I2C device on address %3d -> opening %s",i2caddr, i2cdev);

  if(fd_i2c < 0) {
    perror(i2cdev);
    printf("Please make sure /dev/i2c-0 file is present\n");
    return -1;
  }
  
  if(ioctl(fd_i2c,I2C_SLAVE, i2caddr) < 0){
    printf("Error : ioctl I2C_SLAVE for 0x%x failed\n ", i2caddr);
    perror("Error : ioctl I2C_SLAVE failed ");
    return -1;
  }

  return 0;
}

///////////////////////////////////////////////////
//
//  SCAN I2C Bus
//
int detect(int i2caddr) {
  int i;

  if (check_i2c(i2caddr) == 0) return i2caddr;

  //for (i=0;i<128;i++) 
  //if( check_i2c(i) == 0) return i;

  return -1;
}




///////////////////////////////////////////////////
//
//  READ TTCRX REGISTER
//
int read_register(int regnum) {
  int ret;
  
#ifdef TESTMODE
  return 42;
#endif


  //do{
    ret = ioctl(fd_i2c,I2C_SLAVE, i2caddr_pointer) ;
    //}while(ret<0);

    //do{
    ret = set_slave_addr(fd_i2c, i2caddr_pointer, 1);
    //}while(ret<0);
  
    //do{
    ret = i2c_smbus_write_byte(fd_i2c, regnum);
    //}while(ret<0);


    //do{
    ret = ioctl(fd_i2c,I2C_SLAVE, i2caddr_data) ;
    //}while(ret<0);

    //do{
    ret = set_slave_addr(fd_i2c, i2caddr_data, 1);
    //}while(ret<0);
  
    ret = i2c_smbus_read_byte(fd_i2c);
  
  return ret;
}

///////////////////////////////////////////////////
//
//  WRITE TTCRX REGISTER
//
int write_register(int regnum, int value) {
  int ret;

#ifdef TESTMODE
  return 0;
#endif
  

  //do{
    ret = ioctl(fd_i2c,I2C_SLAVE, i2caddr_pointer) ;
    //}while(ret<0);

    //do{
    ret = set_slave_addr(fd_i2c, i2caddr_pointer, 1);
    //}while(ret<0);

    //do{
    ret = i2c_smbus_write_byte(fd_i2c, regnum);
    //}while(ret<0);



    //do{
    ret = ioctl(fd_i2c,I2C_SLAVE, i2caddr_data) ;
    //}while(ret<0);

    //do{
    ret = set_slave_addr(fd_i2c, i2caddr_data, 1);
    //}while(ret<0);

    //do{
    ret = i2c_smbus_write_byte(fd_i2c, value);
    //}while(ret<0);

  return 0;
}


///////////////////////////////////////////////////
//
//  Set the trigger sequence 
//
int set_trigger_mode(int fd, int val){
  
  printf("not yet supported ...\n");

  return 1;
}
///////////////////////////////////////////////////
//
//  Reset 
//
int ttcrx_reset(void){

  /*
  int stat_addr=22;
  int ret = 0; 
  ret = read_register(stat_addr);
  ret = ret |  TTCRX_RST;
  write_register(stat_addr, ret);
  
  sleep(1);

  ret = read_register(stat_addr);
  ret = (ret & (~(TTCRX_RST)));
  printf("0x%x\n", ret);
  write_register(stat_addr, ret);
  */

  printf("not yet supported ...\n");
  
  return 1;

}



///////////////////////////////////////////////////
int opendev(const char *filep) {
  int fp;
#ifdef TESTMODE
  return 42;
#endif
  fp = open (filep, O_RDWR);
  if(fp < 0) 
    perror(filep);
  return fp;
}
///////////////////////////////////////////////////
// 1= ttcrx is ready, 0: it is not ready
int ttcrxready(void) {
  /*
  unsigned char rd=0;
  int fp;
  fp=opendev((char *)ttcrxdev);
  if (fp < 0) return 0; // strange but ok
  if(read(fp,&rd,1) != 1) { perror("Read 1 Byte TTCRX Status"); rd=0; }
  close(fp);
  return((int) rd);
  */

  ///look at the status register 
  int data = 0;
  data = read_register(22);
  if( (data>>5) & 0x7 ==  0x7){
    return 1;
  }else{
    printf("ttcrx status register = 0x%x\n", data);
    perror("ttcrx status register gives not ready...\n");
    return 0;
  }

}

///////////////////////////////////////////////////
//
//  Read Event Counter
//
int read_event_counter(void){
  int retval=0;
  retval  = read_register(28) << 16;
  retval += read_register(27) <<  8;
  retval += read_register(26);
  return retval;
}




///////////////////////////////////////////////////
//
//  MAIN
//
int main(int argc, char** argv) {
  int i;
  unsigned long int li_event_count;
  unsigned int i_bunchcross_count;
  char reg_array[20];
  char *argument;
  int revision=1;

  char ttcrx_lookup[20]={ 0,1,2,3,8,9,10,11,16,17,18,19,20,21,22,24,25,26,27,28 };
  int ttcrx_lookup_rev[29]={ 0,1,2,3,-4,-5,-6,-7,8,9,10,11,-12,-13,-14,-15,16,17,18,19,20,21,22,-23,24,25,26,27,28 };

  const char *ttcrx_regname[20];
  ttcrx_regname[ 0]="Fine Delay 1                    ";
  ttcrx_regname[ 1]="Fine Delay 2                    ";
  ttcrx_regname[ 2]="Coarse Delay                    ";
  ttcrx_regname[ 3]="Control                         ";
  ttcrx_regname[ 4]="Single error count <7:0>        ";
  ttcrx_regname[ 5]="Single error count <15:8>       ";
  ttcrx_regname[ 6]="Double error count <7:0>        ";
  ttcrx_regname[ 7]="SEU error count <15:8>          ";
  ttcrx_regname[ 8]="ID <7:0>                        ";
  ttcrx_regname[ 9]="MasterModeA <1:0>, ID <13:8>    ";
  ttcrx_regname[10]="MasterModeB <1:0>, I2C_ID <5:0> ";
  ttcrx_regname[11]="Config 1                        "; 
  ttcrx_regname[12]="Config 2                        "; 
  ttcrx_regname[13]="Config 3                        "; 
  ttcrx_regname[14]="Status                          "; 
  ttcrx_regname[15]="Bunch counter <7:0>             "; 
  ttcrx_regname[16]="Bunch counter <15:8>            "; 
  ttcrx_regname[17]="Event counter <7:0>             "; 
  ttcrx_regname[18]="Event counter <15:8>            "; 
  ttcrx_regname[19]="Event counter <23:16>           "; 


  if (argc == 1) {
    printf("Usage: %s <option>\n",argv[0]); 
    printf("\n");
    printf("Communication to TTCrx via i2c. Options:\n");
    printf("n         - dump TTCrx Registers numerical\n");
    printf("d         - dump TTCRx Registers with descriptions\n");
    printf("e         - printout event counter\n");
    printf("r         - reset TTCrx\n");
    printf("w val reg - Write val to register reg\n");
    printf("s         - Set Trigger output to Software (pretrigger from dcs2trap)\n");
    printf("t         - Set Trigger output to Hardware (L1Accept from TTCrx)\n");
    printf("o         - Set Trigger output to One-Shot-Trigger (Only one L1Accept from TTCrx)\n");
    printf("v         - Display version\n");

    printf("\n"); 
    printf("Example: %s w 0x93 3\n",argv[0]); 
    return 0;
  }




  fd_i2c = opendev(i2cdev);
  if (fd_i2c < 0) return -1;


  int i2caddr=detect(i2caddr_pointer);
  if (i2caddr == -1) {
    printf ("Detected no TTCRX on %d and not at any address\n",i2caddr_pointer);
    return -1;
  }
  i2caddr=detect(i2caddr_data);
  if (i2caddr == -1) {
    printf ("Detected no TTCRX on %d and not at any address\n",i2caddr_pointer);
    return -1;
  }




  argument=argv[1];
  if (argument[0]=='-') argument[0]=argument[1]; // skip "-"

  if ( (argument[0] != 's')) {
    if (ttcrxready()) 
      printf("TTCRX is READY\n");
    else
      printf("***\n*** WARNING: TTCRX is not READY\n***\n");
  }



  switch (argument[0]) {
  case 'v':
    printf("ttcrx revision: %s\n",revision);
    return 0;
    break;
    //////////////////////////////////////////// 
  case 'e': break;
  case 'n': break;
  case 'd': break;
  case 'w': break;
  case 's': 
  case 'o':
    //////////////////////////////////////////// 
    // switch to software mode trigger (1 -> /dev/ttcrx)
    {
      int fp,ret;
      char val=1;
      ret = set_trigger_mode(fd_i2c,val);
      if (ret != 1) { 
	perror("Select Trigger Source");
	return -1;
      }
      if (argument[0]=='s') 
	printf("Software Trigger selected\n");
      else
	printf("One-Shot-Trigger selected\n");
    }
    break;
    //////////////////////////////////////////// 
  case 't':
    //////////////////////////////////////////// 
    // switch to hardware mode trigger (0 -> /dev/ttcrx)
    {
      int fp,ret;
      char val=0;
      ret = set_trigger_mode(fd_i2c, val);
      if (ret != 1) { 
	perror("Select Trigger Source");
	return -1;
      }
      printf("Hardware Trigger selected\n");
    }

    break;
  }


 

  switch (argument[0]) {

  case 'r':
    ttcrx_reset();
    break;
    //////////////////////////////////////////// 
  case 'e':
    //////////////////////////////////////////// 
    printf("Event Counter: %d\n",read_event_counter());
    break;
    //////////////////////////////////////////// 
  case 'n':
    //////////////////////////////////////////// 
    for (i=0;i<20;i++) 
      printf("%s = %2d: 0x%02X\n", ttcrx_regname[i], ttcrx_lookup[i],read_register(ttcrx_lookup[i]));
    break;
    //////////////////////////////////////////// 
  case 'd':
    //////////////////////////////////////////// 
    for (i=0;i<20;i++) {
      reg_array[i]=read_register(ttcrx_lookup[i]);
      //      printf("%2d %s: 0x%02X\n",ttcrx_lookup[i], ttcrx_regname[i],read_register(ttcrx_lookup[i])); reduce num of HW reads
      printf("%2d %s: 0x%02X\n",ttcrx_lookup[i], ttcrx_regname[i],reg_array[i]);
    }
    i_bunchcross_count = (((reg_array[16] & 0xff) << 8) | ((reg_array[15] & 0xff)));
    printf("\nBunch counter:    %05d",i_bunchcross_count);
    printf("   hex:   0x%04X\n",i_bunchcross_count);
    li_event_count = (((reg_array[19] & 0xff) << 16) | ((reg_array[18] & 0xff) << 8) | ((reg_array[17] & 0xff)));
    printf("\nEvent counter: %08ld",li_event_count);
    printf("   hex: 0x%06lX\n\n",li_event_count);
    
    break;
    //////////////////////////////////////////// 
  case 'w':
    //////////////////////////////////////////// 
    if (argc != 4) {
      printf("Write needs two arguments\n");
      return -1;
    } else {
      int reg=-1;
      int val=0;
      sscanf(argv[3],"%i",&reg);
      sscanf(argv[2],"%i",&val);
      if ( (reg < 0) || ( reg > 28 ) ) {
	printf("Register %d out of range\n",reg);
	return -1;
      }
      if (ttcrx_lookup_rev[reg] < 0 ) { 
	printf("Register %d invalid\n",reg);
	return -1;
      }
      printf("Writing value 0x%02x to register %d\n",val,reg);
      write_register(reg,val);
    }
    break;    

  case 's': break;
  case 'o': break;
  case 't': break;
  default:
    printf("Unknown option: %c\n",argument[0]);
    break;
  } // switch argument
    

  close(fd_i2c);

  return(0);
}
