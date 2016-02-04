#include <stdio.h>
#include <stdint.h>
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


const char *i2cdev="/dev/i2c-0"; 
int fd_i2c;

int sfp_info = 0x50;
int sfp_diag = 0x51;
uint8_t data_info[256];
uint8_t data_diag[256];

//////
int check_i2c(int i2caddr);
int detect(int i2caddr);
int opendev(const char *filep) ;
int set_i2c_slave(int slave);
void dump_i2c_slave(uint8_t *data);
void print_data(uint8_t *data);
float conv(uint8_t *data, uint16_t div, uint8_t sig);
void print_part(uint8_t *data, const char *ptype, const char *prefix, int start, int len);
void interpret_data(uint8_t *data);
void interpret_diag(uint8_t *diag);

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
int opendev(const char *filep) {
  int fp;
  fp = open (filep, O_RDWR);
  if(fp < 0) 
    perror(filep);
  return fp;
}

///////////////////////////////////////////////////
int set_i2c_slave(int slave){

  if(ioctl(fd_i2c,I2C_SLAVE, slave) < 0){
    printf("Error : ioctl I2C_SLAVE for 0x%x failed\n ", slave);
    perror("Error : ioctl I2C_SLAVE failed ");
    return -1;
  }
  
  if(set_slave_addr(fd_i2c, slave, 1)<0){
    printf("Error : set_slave_addr for 0x%x failed\n ", slave);
    perror("Error : set_slave_addr failed ");
    return -1;
  }
    


}

///////////////////////////////////////////////////
void dump_i2c_slave(uint8_t *data){
  int i=0;
  for(i=0;i<256;i++){
    data[i] = i2c_smbus_read_byte(fd_i2c);
  }

}

///////////////////////////////////////////////////
void print_data(uint8_t *data){
  int i=0;
  for(i=0; i <= 255; i += 16){
    printf("%02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X\n",
	   data[i], data[i+1], data[i+2], data[i+3], data[i+4],
	   data[i+5], data[i+6], data[i+7], data[i+8], data[i+9],
	   data[i+10], data[i+11], data[i+12], data[i+13], data[i+14],
	   data[i+15]);
  }
}

float conv(uint8_t *data, uint16_t div, uint8_t sig){

  if (sig){
    return ((int16_t)  ((data[0] << 8) + data[1]))/(float) div;
  } else {
    return ((uint16_t) ((data[0] << 8) + data[1]))/(float) div;
  }
}


void print_part(uint8_t *data, const char *ptype, const char *prefix, int start, int len){
  int i=0;
  printf("%s: ", prefix);
  for (i=start; i<start+len; i++){
    printf(ptype, data[i]);
  }
  printf("\n");
}

void interpret_data(uint8_t *data){
  print_part(&data[0], "%c",   "Vendor", 20, 16);
  print_part(&data[0], "%02X", "Vendor OUI", 37, 3);
  //print_part(&data[0], "%c",   "Rev", 56, 4);
  print_part(&data[0], "%c",   "Vendor Part Number", 40, 16);
  print_part(&data[0], "%c",   "Vendor Serial Number", 68, 16);
  print_part(&data[0], "%c",   "Vendor Data Code", 84,  6);

  printf("Typ: 0x%02X\n", data[0]);
  printf("Connector: 0x%02X\n", data[2]);
  printf("Bitrate: %u MBd\n", data[12]*100);
  printf("Wavelength: %u nm\n", data[60] * 256 + data[61]);
  printf("             %-6s %-6s %-6s %-6s %-6s\n", "SM", "OM1", "OM2", "OM3", "OM4");
  printf("Max length: %3u km %4u m %4u m %4u m %4u m\n", data[14], data[17]*10, data[16]*10, data[19]*10, data[18]*10);

  printf("Datacheck : ");
  uint8_t foo = 0;
  int i=0;
  for (i=0; i<= 62; i++){
    foo += data[i];
  }
  if (foo == data[63]){
    printf("OK 0x%02X\n", data[63]);
  } else {
    printf("ERROR: 0x%02X vs. 0x%02X\n", foo, data[63]);
  }
}

void interpret_diag(uint8_t *diag){
  printf("\n");
  /*
  printf("            Temperatur   VCC    TX bias   TX power  RX power  Laser Temp  TEC\n");
  printf("+ Error  :  % 7.2f C % 4.2f V  %5.2f mA % 2.3f mW  %.3f mW % 7.2f C % 5.2f mA\n", conv(&diag[0], 256, 1), conv(&diag[8], 10000, 0), conv(&diag[16], 500, 0), conv(&diag[24], 10000, 0), conv(&diag[32], 10000, 0), conv(&diag[40], 256, 1), conv(&diag[48], 10, 1));
  printf("+ Warning:  % 7.2f C % 4.2f V  %5.2f mA % 2.3f mW  %.3f mW % 7.2f C % 5.2f mA\n", conv(&diag[4], 256, 1), conv(&diag[12], 10000, 0), conv(&diag[20], 500, 0), conv(&diag[28], 10000, 0), conv(&diag[36], 10000, 0), conv(&diag[44], 256, 1), conv(&diag[52], 10, 1));
  printf("  Value  :  % 7.2f C % 4.2f V  %5.2f mA % 2.3f mW  %.3f mW % 7.2f C % 5.2f mA\n", conv(&diag[96], 256, 1), conv(&diag[98], 10000, 0), conv(&diag[100], 500, 0), conv(&diag[102], 10000, 0), conv(&diag[104], 10000, 0), conv(&diag[106], 256, 1), conv(&diag[108], 10, 1));
  printf("- Warning:  % 7.2f C % 4.2f V  %5.2f mA % 2.3f mW  %.3f mW % 7.2f C % 5.2f mA\n", conv(&diag[6], 256, 1), conv(&diag[14], 10000, 0), conv(&diag[22], 500, 0), conv(&diag[30], 10000, 0), conv(&diag[38], 10000, 0), conv(&diag[46], 256, 1), conv(&diag[54], 10, 1));
  printf("- Error  :  % 7.2f C % 4.2f V  %5.2f mA % 2.3f mW  %.3f mW % 7.2f C % 5.2f mA\n", conv(&diag[2], 256, 1), conv(&diag[10], 10000, 0), conv(&diag[18], 500, 0), conv(&diag[26], 10000, 0), conv(&diag[34], 10000, 0), conv(&diag[42], 256, 1), conv(&diag[50], 10, 1));
  */

  printf("            Temperatur   VCC    TX bias   TX power  RX power \n");
  printf("+ Error  :  % 7.2f C % 4.2f V  %5.2f mA % 2.3f mW  %.3f mW \n", conv(&diag[0], 256, 1), conv(&diag[8], 10000, 0), conv(&diag[16], 500, 0), conv(&diag[24], 10000, 0), conv(&diag[32], 10000, 0));
  printf("+ Warning:  % 7.2f C % 4.2f V  %5.2f mA % 2.3f mW  %.3f mW \n", conv(&diag[4], 256, 1), conv(&diag[12], 10000, 0), conv(&diag[20], 500, 0), conv(&diag[28], 10000, 0), conv(&diag[36], 10000, 0));
  printf("  Value  :  % 7.2f C % 4.2f V  %5.2f mA % 2.3f mW  %.3f mW \n", conv(&diag[96], 256, 1), conv(&diag[98], 10000, 0), conv(&diag[100], 500, 0), conv(&diag[102], 10000, 0), conv(&diag[104], 10000, 0));
  printf("- Warning:  % 7.2f C % 4.2f V  %5.2f mA % 2.3f mW  %.3f mW \n", conv(&diag[6], 256, 1), conv(&diag[14], 10000, 0), conv(&diag[22], 500, 0), conv(&diag[30], 10000, 0), conv(&diag[38], 10000, 0));
  printf("- Error  :  % 7.2f C % 4.2f V  %5.2f mA % 2.3f mW  %.3f mW \n", conv(&diag[2], 256, 1), conv(&diag[10], 10000, 0), conv(&diag[18], 500, 0), conv(&diag[26], 10000, 0), conv(&diag[34], 10000, 0));


}


///////////////////////////////////////////////////
//
//  MAIN
//
int main(int argc, char** argv) {



  fd_i2c = opendev(i2cdev);
  if (fd_i2c < 0) return -1;

  printf("info\n");


  set_i2c_slave(sfp_info);
  dump_i2c_slave(&data_info[0]);
  print_data(&data_info[0]);


  printf("diag\n");

  set_i2c_slave(sfp_diag);
  dump_i2c_slave(&data_diag[0]);
  print_data(&data_diag[0]);


  close(fd_i2c);

  interpret_data(&data_info[0]);
  interpret_diag(&data_diag[0]);

  return(0);
}
