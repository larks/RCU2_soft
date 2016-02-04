#include<unistd.h>
#include<sys/types.h>
#include<sys/mman.h>
#include<stdio.h>
#include<fcntl.h>
#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <linux/types.h>
#include <linux/spi/spidev.h>
//#include <linux/interrupt.h>

//#define NR_IRQS  83

//#define set_MSP(x)				\
//  __asm(  "msr   MSP, %0\n" : : "r" (x) )

///commands
#define DEVICE_CERTIFICATE_REQUEST_CMD  0;
#define SERIAL_NUMBER_REQUEST_CMD       1;
#define USERCODE_REQUEST_CMD            4;
#define DESIGNVER_REQUEST_CMD           5;
#define DIGEST_CHECK_REQUEST_CMD       23;
#define FLASH_FREEZE_REQUEST_CMD        2;
#define FLASH_FREEZE_SHUTDOWN_CMD     224;
#define ISP_PROGRAMMING_REQUEST_CMD    21;

/*------------------------------------------------------------------------------
 * Control register bit masks.
 */
#define CR_FLUSHOUT_MASK    0x01u
#define CR_SIZETX_MASK      0x04u
#define CR_ENABLE_MASK      0x10u
#define CR_LOOPBACK_MASK    0x20u


/*------------------------------------------------------------------------------
 * Status and interrupt enable registers bit masks.
 */
#define TXTOKAY_MASK    0x01u
#define RCVOKAY_MASK    0x02u

/*------------------------------------------------------------------------------
 * DATA8 register bit masks.
 */
#define DATA8_COMMAND_MASK  0x8000u

/*------------------------------------------------------------------------------
 * COMBLK driver states.
 */
#define COMBLK_IDLE             0u
#define COMBLK_TX_CMD           1u
#define COMBLK_TX_DATA          2u
#define COMBLK_WAIT_RESPONSE    3u
#define COMBLK_RX_RESPONSE      4u
#define COMBLK_TX_PAGED_DATA    5u

/*------------------------------------------------------------------------------
 * COMBLK address lists
 */
#define COMBLK_BASE    0x40016000 
#define CONTROL        0x00
#define STATUS         0x04
#define INT_ENABLE     0x08
#define DATA8          0x10
#define DATA32         0x14
#define FRAME_START8   0x18
#define FRAME_START32  0x1c

#define SYSREG_SOFT_RST_CR          0x40038048
#define SYSREG_ENVM_CR              0x4003800C
#define SYSREG_MSSDDR_FACC2_CR      0x4003809C
#define SYSREG_MSSDDR_FACC1_CR      0x40038098
#define STSREG_ESRAM_CR             0x40038000
#define STSREG_ENVM_REMAP_BASE_CR   0x40038010
     

/*
 * Service response lengths:
 */
#define STANDARD_SERV_RESP_LENGTH                       6u
#define SERIAL_NUMBER_SERV_RESP_LENGTH                  6u
#define USERCODE_SERV_RESP_LENGTH                       6u
#define DESIGNVER_SERV_RESP_LENGTH                      6u
#define DEVICE_CERT_SERV_RESP_LENGTH                    6u
#define SECONDARY_DEVICE_CERT_SERV_RESP_LENGTH          6u
#define ISP_PROG_SERV_RESP_LENGTH                       3u
#define DIGEST_CHECK_SERV_RESP_LENGTH                   2u
#define FLASH_FREEZE_SERV_RESP_LENGTH                   2u
#define FACC_STANDBY_SEL                                0u
#define MSS_25_50MHZ_EN                                 1u
#define MSS_1MHZ_EN                                     1u
#define FACC_STANDBY_SHIFT                              6u
#define MSS_25_50MHZ_EN_SHIFT                           9u
#define MSS_1MHZ_EN_SHIFT                               10u
#define FACC_STANDBY_SEL_MASK                           0x000001C0u
#define MSS_25_50MHZ_EN_MASK                            0x00000200u
#define MSS_1MHZ_EN_MASK                                0x00000400u


#define POR_DIGEST_ERROR_OPCODE         0xF1

/// user functions 
static void comblk_init(void);

static uint16_t wait_for_request_completion(void);
static void comblk_send_cmd_with_ptr(uint8_t cmd_opcode,
			      uint32_t cmd_params_ptr,
			      uint8_t * p_response,
			      uint16_t response_size);

static void comblk_send_paged_cmd(uint8_t * p_cmd,
				  uint16_t cmd_size,
				  uint8_t * p_response,
				  uint16_t response_size);

static uint8_t comblk_execute_service(uint8_t cmd_opcode,
				      uint8_t * cmd_params_ptr,
				      uint8_t * response,
				      uint16_t response_length);
static void comblk_irq_hander(void);
static void send_cmd_opcode(uint8_t opcode);
static uint32_t fill_tx_fifo(const uint8_t * p_cmd, uint32_t cmd_size);
static void handle_tx_okay_irq(void);
static void handle_rx_okay_irq(void);
void comblk_get_serial_number(uint8_t * p_serial_number);
void comblk_get_design_version(uint8_t * p_serial_number);

void data8_write(off_t addr);
void data8_read(off_t addr);
void data32_write(off_t addr);
void data32_read(off_t addr);
void complete_request(uint16_t response_length);


//// global variable
static int fd; 
static volatile uint8_t g_comblk_cmd_opcode = 0u;
static const uint8_t * g_comblk_p_cmd = 0u;
static volatile uint16_t g_comblk_cmd_size = 0u;
static const uint8_t * g_comblk_p_data = 0u;
static volatile uint32_t g_comblk_data_size = 0u;
static uint8_t * g_comblk_p_response = 0u;
static uint16_t g_comblk_response_size = 0u;
static volatile uint16_t g_comblk_response_idx = 0u;
static uint8_t g_request_in_progress = 0u;
static uint8_t g_comblk_state = COMBLK_IDLE;


static uint32_t g_read_data32 = 0u;
static uint8_t g_read_data8 = 0u;
static uint32_t g_write_data32 = 0u;
static uint8_t g_write_data8 = 0u;




/*---------------------------------------
 * SPI related variables 
 */
#define BUFFER_A_SIZE  256
#define NB_NIBBLES_IN_INT   8
#define NOTIFY 100*256
#define FLASH_MANUFACTURER_ID   (uint8_t)0x01   //RCU2
#define FLASH_DEVICE_ID         (uint8_t)0x40   //RCU2
#define SPI_DATA_ADDR           0x410000       /// spi data for the firmware
#define VERSION_ID_ADDR         381  
#define SPI_FILE_SIZE           2418896  //size of SPI File 
//#define SPI_FILE_SIZE           2896  //size of SPI File 

struct spi_ioc_transfer xfer[2];
/* ISP globals */
static long g_src_image_target_address = 0;
static uint8_t g_flash_rd_buf[BUFFER_A_SIZE+4];
static uint8_t buf[BUFFER_A_SIZE+4];
static uint8_t buf2[BUFFER_A_SIZE+4];

uint32_t page_read_handler(uint8_t const ** pp_next_page);
static uint32_t read_page_from_flash(uint8_t * g_buffer, uint32_t length);


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

static const char *device = "/dev/spidev0.0";
static uint8_t mode = 3;
static uint8_t bits = 8;
static uint32_t speed = 2000000;

static int fd_spi; 

void wait_ready(void);
void spi_global_unprotect(void);
void spi_read(int addr, uint8_t *buffer, int nbytes);
uint8_t verify(void);


#define MSS_SYS_PROG_AUTHENTICATE    0u
#define MSS_SYS_PROG_PROGRAM         1u
#define MSS_SYS_PROG_VERIFY          2u

#define NVM_FREQRNG_MASK        0x00001FE0
#define NVM_FREQRNG_MAX         ((uint32_t)0xFF << 5)

static uint8_t g_isp_response[ISP_PROG_SERV_RESP_LENGTH];

void MSS_SYS_start_isp(uint8_t mode);
static uint32_t g_initial_mssddr_facc1_cr = 0;
static uint32_t g_initial_envm_cr = 0x00001FF1U;
static uint32_t g_initial_mssddr_facc2_cr = 0x00;
static uint8_t g_mode = 0;
static uint8_t wait_for_clock_switch = 1;

/*
  Features that the old peekXX/pokeXX did not have:
  1. Support for 8/16/32 bit READ/WRITE in one function
  2. Support for decimal and binary values
  3. The value return is returned (to become the status code)
 */
int main(int argc, char **argv) {

  uint8_t exec_cmd = 0;
  unsigned char *start;
  unsigned char *chardat, charval;
  unsigned short *shortdat, shortval; 
  unsigned int *intdat, intval;
  int i;
  int ret;
  uint8_t serial_number[16];
  uint8_t design_version[2];

  uint32_t stack_ptr = 0x20010000;
  uint32_t reset_ptr = 0x00000375;
  uint32_t base_ptr =  0x60020000;


  if(argc!=2){
    fprintf(stderr,"Usage: ./isp_tool [c:a:p:v]\n");
    fprintf(stderr,"     c: check the comblk\n");
    fprintf(stderr,"     a: authentication (not supported now. will be available in the future)\n");
    fprintf(stderr,"     p: programming (not supported in the linux.)\n");
    fprintf(stderr,"     v: verify (not supported in the linux)\n");
    fprintf(stderr,"     r: remapping (just for my fun)...\n");
    return 0;
  }

  if(strcmp(argv[1],"c")==0){
    exec_cmd = 1;
  }else if(strcmp(argv[1],"a")==0){
    exec_cmd = 2;
  }else if(strcmp(argv[1],"p")==0){
    exec_cmd = 3;
  }else if(strcmp(argv[1],"v")==0){
    exec_cmd = 4;
  }else if(strcmp(argv[1],"r")==0){
    exec_cmd = 5;
  }    

  fd = open("/dev/mem", O_RDWR|O_SYNC);
  if (fd == -1) {
    perror("open(/dev/mem):");
    return 0;
  }  
    
  printf(" ----- comblk_init -------\n");
  comblk_init();
  printf(" ----- comblk_init done -------\n");

  if(exec_cmd == 1){
    printf("\n\n");
    printf(" ----- comblk_get_serial_number -------\n");
    comblk_get_serial_number(serial_number);
    printf(" ********* Serial Number ********** \n");
    printf(" At 0x%x, Serial Number = ",  (uint32_t)serial_number);
    for(i=0;i<16;i++){
      printf("%02x ", serial_number[i]);
    }
    printf(" \n **********************************\n");
    printf(" ----- comblk_get_serial_number done -------\n");
    
    printf("\n\n");
    printf(" ----- comblk_get_design_version -------\n");
    comblk_get_design_version(design_version);
    printf(" ********* Design Version ********** \n");
    printf(" At 0x%x, Design Version = ",  (uint32_t)design_version);
    for(i=0;i<2;i++){
      printf("%02x ", design_version[i]);
    }
    printf(" \n **********************************\n");
    printf(" ----- comblk_get_design_version done -------\n");
  }else if(exec_cmd == 2 || exec_cmd ==3 || exec_cmd == 4){  
    fd_spi = open(device, O_RDWR);
    if (fd_spi < 0){
      perror("can't open spi device");
      return 0;
    }
    /* spi mode */
    ret = ioctl(fd_spi, SPI_IOC_WR_MODE, &mode);
    if (ret == -1){perror("can't set spi mode"); return 0 ;}

    ret = ioctl(fd_spi, SPI_IOC_RD_MODE, &mode);
    if (ret == -1){perror("can't get spi mode"); return 0 ;}

    /* bits per word */
    ret = ioctl(fd_spi, SPI_IOC_WR_BITS_PER_WORD, &bits);
    if (ret == -1){perror("can't set bits per word"); return 0;}

    ret = ioctl(fd_spi, SPI_IOC_RD_BITS_PER_WORD, &bits);
    if (ret == -1){perror("can't get bits per word"); return 0;}

    /* max speed hz */
    ret = ioctl(fd_spi, SPI_IOC_WR_MAX_SPEED_HZ, &speed);
    if (ret == -1){perror("can't set max speed hz"); return 0;}

    ret = ioctl(fd_spi, SPI_IOC_RD_MAX_SPEED_HZ, &speed);
    if (ret == -1){perror("can't get max speed hz"); return 0;}

    printf("spi mode: %d\n", mode);
    printf("bits per word: %d\n", bits);
    printf("max speed: %d Hz (%d KHz)\n", speed, speed/1000);
    
    ret = verify();
    if(ret == -1){ perror("IOCTL or SPI Chip ID is wrong!"); return 0;}


    spi_global_unprotect();

    if(exec_cmd == 2){
      g_src_image_target_address = SPI_DATA_ADDR;
      //MSS_SYS_start_isp(MSS_SYS_PROG_AUTHENTICATE);
      printf("sorry this is not yet supported.... this will be available in the future\n");
    }else{
      printf("sorry this is not supported... ISP programming cannot be executed from Linux.\n");
    }

  }else if(exec_cmd==5){

    printf("this option is not yet supported\n");

    /*
    for(i=0;i<NR_IRQS; i++){
      disable_irq(i);
    }
    set_MSP(stack_pointer);

    addr = SYSRE_ESRAM_CR ;
    data32_read(addr);
    g_write_data32 = g_read_data32 & 0xFFFFFFFC; // disable the eSRAM mapping
    data32_write(addr);

    addr = SYSREG_ENVM_CR;
    data32_read(addr);
    g_write_data32 = g_read_data32 & 0xFFFFFFF0;
    data32_write(addr);

    data32_read(addr);
    g_write_data32 = g_read_data32 | 0x10;
    data32_write(addr);

    addr = SYSREG_ENVM_REMAP_BASE_CR ;
    g_write_data32 = (base_ptr & 0x3FFFF) | 1;  // enable the eNVM mapping
    data32_write(addr);

    ((void (*)())(reset_ptr))(); // jump to the reset pointer
    for (;;) { ; } // no way to reach this point
    */
  }


  close(fd);
  close(fd_spi);
  return intval;
}

void data32_write(off_t addr){
  off_t page;
  unsigned char *start;
  uint32_t *intdat;
  page = addr & 0xfffff000;
  start = mmap(0, getpagesize(), PROT_READ|PROT_WRITE, MAP_SHARED, fd, page);
  if (start == MAP_FAILED) {
    perror("mmap:");
    return ;
  }
  intdat = (uint32_t *)(start + (addr & 0xfff));
  *intdat = g_write_data32;
}

void data8_write(off_t addr){
  off_t page;
  unsigned char *start;
  uint8_t *intdat;
  page = addr & 0xfffff000;
  start = mmap(0, getpagesize(), PROT_READ|PROT_WRITE, MAP_SHARED, fd, page);
  if (start == MAP_FAILED) {
    perror("mmap:");
    return ;
  }  
  intdat = (uint8_t *)(start + (addr & 0xfff));
  *intdat = g_write_data8;
}

void data32_read(off_t addr){
  off_t page;
  unsigned char *start;
  uint32_t *intdat;
  page = addr & 0xfffff000;
  start = mmap(0, getpagesize(), PROT_READ|PROT_WRITE, MAP_SHARED, fd, page);
  if (start == MAP_FAILED) {
    perror("mmap:");
    return;
  }
  intdat = (uint32_t *)(start + (addr & 0xfff));
  g_read_data32 = *intdat;
  printf(" addr = 0x%x, data = 0x%x\n", addr, g_read_data32);

}

void data8_read(off_t addr){
  off_t page;
  unsigned char *start;
  uint8_t *intdat;
  page = addr & 0xfffff000;
  start = mmap(0, getpagesize(), PROT_READ|PROT_WRITE, MAP_SHARED, fd, page);
  if (start == MAP_FAILED) {
    perror("mmap:");
    return ;
  }
  intdat = (uint8_t *)(start + (addr & 0xfff));
  g_read_data8 = *intdat;
  //printf(" addr = 0x%x, data = 0x%x\n", addr, g_read_data8);

}

static void comblk_init(void){
  off_t addr; 
  uint8_t *intdat8;
  uint8_t intval;
  uint32_t intdat32;


  g_request_in_progress = 0u;
  g_comblk_cmd_opcode = 0u;
  g_comblk_state = COMBLK_IDLE;
  g_comblk_response_size = 0;

  ///// SYSREG reset for COMBLK
  addr = SYSREG_SOFT_RST_CR;
  data32_read(addr);
  g_write_data32 = g_read_data32 | (1<<15);
  data32_write(addr);

  data32_read(addr);
  g_write_data32 = g_read_data32 & ~(1<<15);
  data32_write(addr);

  data32_read(addr);


  ///// COMBLK->INT_ENABLE = 0u;
  g_write_data8 = 0x0;
  addr = COMBLK_BASE | INT_ENABLE;
  data8_write(addr);



  //// COMBLK->CONTROL &= ~CR_LOOPBACK_MASK;
  //// COMBLK->CONTROL |= CR_ENABLE_MASK;
  addr = COMBLK_BASE | CONTROL;
  data8_read(addr);
  g_write_data8 = g_read_data8 & (~CR_LOOPBACK_MASK);
  data8_write(addr);

  data8_read(addr);
  g_write_data8 = g_read_data8 | CR_ENABLE_MASK;
  data8_write(addr);
  



  //COMBLK->INT_ENABLE &= ~TXTOKAY_MASK;
  //COMBLK->INT_ENABLE |= RCVOKAY_MASK;
  addr = COMBLK_BASE | INT_ENABLE;
  data8_read(addr);
  g_write_data8 = g_read_data8 & (~TXTOKAY_MASK); 
  data8_write(addr);

  data8_read(addr);
  g_write_data8 = g_read_data8 | RCVOKAY_MASK;
  data8_write(addr);



  ////readout the registers
  addr = COMBLK_BASE | CONTROL;  data8_read(addr);
  addr = COMBLK_BASE | STATUS;  data8_read(addr);
  addr = COMBLK_BASE | INT_ENABLE;  data8_read(addr);


}

static uint16_t wait_for_request_completion(void)
{

}
static void comblk_send_cmd_with_ptr(uint8_t cmd_opcode, uint32_t cmd_params_ptr, uint8_t * p_response, uint16_t response_size)
{
  int tx_okay;
  off_t addr;

  g_comblk_state = COMBLK_IDLE;
  g_comblk_cmd_opcode = cmd_opcode;
  g_comblk_response_size = response_size;  
  g_comblk_p_response = p_response;  

  //COMBLK->INT_ENABLE = 0u;  
  addr = COMBLK_BASE | INT_ENABLE;
  g_write_data8 = 0x0;
  data8_write(addr);
  
  /*--------------------------------------------------------------------------
   * Send command opcode as a single byte write to the Tx FIFO.
   */
  send_cmd_opcode(g_comblk_cmd_opcode);

  /*--------------------------------------------------------------------------
   * Send the command parameters pointer to the Tx FIFO as a single 4 bytes
   * write to the Tx FIFO.
   */
  //COMBLK->CONTROL |= CR_SIZETX_MASK;
  addr = COMBLK_BASE | CONTROL;
  data8_read(addr);
  g_write_data8 = g_read_data8 | CR_SIZETX_MASK;
  data8_write(addr);

  /* Wait for space to become available in Tx FIFO. */
  addr = COMBLK_BASE | STATUS;
  do {
    data8_read(addr);
    tx_okay = g_read_data8 & TXTOKAY_MASK;
  } while(0 == tx_okay);
  
  /* Send command opcode. */
  addr = COMBLK_BASE | DATA32;
  g_write_data32 = cmd_params_ptr;
  data32_write(addr);


  //COMBLK->CONTROL &= ~CR_SIZETX_MASK;
  addr = COMBLK_BASE | CONTROL;
  data8_read(addr);
  g_write_data8 = g_read_data8 & ~CR_SIZETX_MASK; ;
  data8_write(addr);

  g_comblk_state = COMBLK_WAIT_RESPONSE;
  //--------------------------------------------------------------------------
  // Enable interrupt.
  //
  //COMBLK->INT_ENABLE |= RCVOKAY_MASK;
  addr = COMBLK_BASE | INT_ENABLE;
  data8_read(addr);
  g_write_data8 = g_read_data8 | RCVOKAY_MASK;    
  data8_write(addr);

  usleep(1000);
  comblk_irq_hander();


}


static void comblk_send_paged_cmd(uint8_t * p_cmd, uint16_t cmd_size,uint8_t * p_response, uint16_t response_size){
  uint32_t size_sent;
  uint8_t irq_enable = 0u;
  off_t addr; 

  //COMBLK->INT_ENABLE = 0u;
  addr = COMBLK_BASE | INT_ENABLE;
  g_write_data8 = 0x0;
  data8_write(addr);

  g_request_in_progress = 1u;
  g_comblk_cmd_opcode = p_cmd[0];
  g_comblk_p_cmd = p_cmd;
  g_comblk_cmd_size = cmd_size;
  g_comblk_p_data = 0;
  g_comblk_data_size = 0u;
  g_comblk_p_response = p_response;
  g_comblk_response_size = response_size;
  g_comblk_response_idx = 0u;

  send_cmd_opcode(g_comblk_cmd_opcode);
  size_sent = fill_tx_fifo(&p_cmd[1], cmd_size - 1);
  ++size_sent;    /* Adjust for opcode byte sent. */
  if(size_sent < cmd_size){
    g_comblk_cmd_size = g_comblk_cmd_size - (uint16_t)size_sent;
    g_comblk_p_cmd = &g_comblk_p_cmd[size_sent];
        
    g_comblk_state = COMBLK_TX_CMD;
    irq_enable = TXTOKAY_MASK | RCVOKAY_MASK;
  }else{
    g_comblk_cmd_size = 0u;
    g_comblk_state = COMBLK_TX_PAGED_DATA;
    irq_enable = TXTOKAY_MASK | RCVOKAY_MASK;
  }

  //COMBLK->INT_ENABLE |= irq_enable;
  addr = COMBLK_BASE | INT_ENABLE;
  data8_read(addr);
  g_write_data8 = g_read_data8 | irq_enable;
  data8_write(addr);

  usleep(1000);  
  comblk_irq_hander();
}


static void send_cmd_opcode(uint8_t opcode){
  uint32_t tx_okay;
  off_t addr;

  /* Set transmit FIFO to transfer bytes. */
  addr = COMBLK_BASE | CONTROL;
  data8_read(addr);
  g_write_data8 = g_read_data8 & ~CR_SIZETX_MASK;
  data8_write(addr);
    
  /* Wait for space to become available in Tx FIFO. */
  addr = COMBLK_BASE|STATUS;
  do {
    data8_read(addr);
    tx_okay = g_read_data8 & TXTOKAY_MASK;
  } while(0 == tx_okay);
    
  /* Send command opcode. */
  addr = COMBLK_BASE | FRAME_START8;
  g_write_data8 = opcode;
  data8_write(addr);

}

static void comblk_irq_hander(void){
  uint8_t status;
  uint8_t intval;
  uint8_t tx_okay;
  uint8_t rcv_okay;
  off_t addr;


  do{
    usleep(1000);
    //printf("comblk_irq_hander status\n");
    
    addr = COMBLK_BASE | STATUS;
    data8_read(addr);
    status = g_read_data8;
    
    addr = COMBLK_BASE | INT_ENABLE;
    data8_read(addr);
    intval = g_read_data8;
    
    status &= intval;
    tx_okay = status & TXTOKAY_MASK;
    if(tx_okay){
      handle_tx_okay_irq();
    }
  }while(!(tx_okay==0));


  g_comblk_state = COMBLK_WAIT_RESPONSE;

  do{
    usleep(1000);
    //printf("comblk_irq_hander status\n");
    
    addr = COMBLK_BASE | STATUS;
    data8_read(addr);
    status = g_read_data8;
    
    addr = COMBLK_BASE | INT_ENABLE;
    data8_read(addr);
    intval = g_read_data8;
    
    status &= intval;
    rcv_okay = status & RCVOKAY_MASK;
    if(rcv_okay){
      handle_rx_okay_irq();
    }    
  }while(!(rcv_okay==0));



}




static uint32_t fill_tx_fifo(const uint8_t * p_cmd,    uint32_t cmd_size){
  uint32_t tx_okay;
  uint32_t size_sent;
  off_t addr;

  off_t addr_data8;
  off_t page_data8;
  unsigned char *start_data8;
  uint8_t *intdat_data8;


  off_t addr_status;
  off_t page_status;
  unsigned char *start_status;
  uint8_t *intdat_status;


  /* Set transmit FIFO to transfer bytes. */
  
  //printf("fill_tx_fifo 0x%x, %d\n", p_cmd[0], cmd_size);

  addr = COMBLK_BASE | CONTROL;
  data8_read(addr);
  g_write_data8 = g_read_data8 & ~CR_SIZETX_MASK;
  data8_write(addr);
    
  size_sent = 0u;

  

  /// no use of data8_write and data8_read 
  //// status register
  addr_status = COMBLK_BASE|STATUS;
  page_status = addr_status & 0xfffff000;     
  start_status = mmap(0, getpagesize(), PROT_READ|PROT_WRITE, 
		      MAP_SHARED, fd, page_status);  
  intdat_status = (uint8_t *)(start_status + (addr_status & 0xfff));      
  tx_okay = (*intdat_status) & TXTOKAY_MASK;        

  //// data8 register
  addr_data8 = COMBLK_BASE|DATA8;   
  page_data8 = addr_data8 & 0xfffff000;
  start_data8 = mmap(0, getpagesize(), PROT_READ|PROT_WRITE, 
		     MAP_SHARED, fd, page_data8);
  intdat_data8 = (uint8_t *)(start_data8 + (addr_data8 & 0xfff));   



  while((tx_okay != 0) && (size_sent < cmd_size)){
    //addr = COMBLK_BASE|DATA8;
    //g_write_data8 = p_cmd[size_sent];
    *intdat_data8 = p_cmd[size_sent];   
    //data8_write(addr);
    ++size_sent;
  
    //addr = COMBLK_BASE|STATUS;
    //data8_read(addr);
    //tx_okay = (*intdat_status) & TXTOKAY_MASK; // this takes time...
  }

  //printf("size_sent = %d\n", size_sent);
  return size_sent;
}

static void handle_tx_okay_irq(void){
  
  //printf("handle_tx_okay_irq %d %d\n", g_comblk_state, g_comblk_cmd_size);
  off_t addr;
  
  switch(g_comblk_state){
    //----------------------------------------------------------------------
    // The TX_OKAY interrupt should only be enabled for states COMBLK_TX_CMD
    // and COMBLK_TX_DATA.
    //
  case COMBLK_TX_CMD:
    if(g_comblk_cmd_size > 0){
      uint32_t size_sent;
      size_sent = fill_tx_fifo(g_comblk_p_cmd, g_comblk_cmd_size);
      if(size_sent < g_comblk_cmd_size){
	g_comblk_cmd_size = g_comblk_cmd_size - (uint16_t)size_sent;
	g_comblk_p_cmd = &g_comblk_p_cmd[size_sent];
      }else{
	g_comblk_cmd_size = 0;
	if(g_comblk_data_size > 0){
	  g_comblk_state = COMBLK_TX_DATA;
	}else{
	  g_comblk_state = COMBLK_WAIT_RESPONSE;
	}
      }
    }else{
      //
      // This is an invalid situation indicating a bug in the driver
      // or corrupted memory.
      //
      //abort_current_cmd();
      }
    break;
  case COMBLK_TX_DATA:
    if(g_comblk_data_size > 0){
      uint32_t size_sent;
      size_sent = fill_tx_fifo(g_comblk_p_data, g_comblk_data_size);
      if(size_sent < g_comblk_data_size){
	g_comblk_data_size = g_comblk_data_size - size_sent;
	g_comblk_p_data = &g_comblk_p_data[size_sent];
      }else{
	addr = COMBLK_BASE | INT_ENABLE;
	g_write_data8 = g_read_data8 & ~TXTOKAY_MASK;
	data8_write(addr);
	g_comblk_state = COMBLK_WAIT_RESPONSE;
      }
    }else{
      //
      // This is an invalid situation indicating a bug in the driver
      // or corrupted memory.
      //
      //ASSERT(0);
	//abort_current_cmd();
    }
    break;
  case COMBLK_TX_PAGED_DATA:
    //
    // Read a page of data if required.
    //
    if(0u == g_comblk_data_size){
      //if(g_comblk_page_handler != 0){
      g_comblk_data_size = page_read_handler(&g_comblk_p_data);
      //printf("%d \n", g_comblk_data_size);
      if(0 == g_comblk_data_size){
	addr = COMBLK_BASE | INT_ENABLE;
	g_write_data8 = g_read_data8 & ~TXTOKAY_MASK;
	data8_write(addr);
	g_comblk_state = COMBLK_WAIT_RESPONSE;
      }
      //}else{
      //ASSERT(0);
      //	abort_current_cmd();
      //}
    }
      
    //
    // Transmit the page data or move to COMBLK_WAIT_RESPONSE state if
    // no further page data could be obtained by the call to the page
    // handler above.
    //
    if(0u == g_comblk_data_size){
      addr = COMBLK_BASE | INT_ENABLE;
      g_write_data8 = g_read_data8 & ~TXTOKAY_MASK;
      data8_write(addr);
      g_comblk_state = COMBLK_WAIT_RESPONSE;
    }else{
      uint32_t size_sent;
      size_sent = fill_tx_fifo(g_comblk_p_data, g_comblk_data_size);
      g_comblk_data_size = g_comblk_data_size - size_sent;
      g_comblk_p_data = &g_comblk_p_data[size_sent];
      //printf("size_sent = %d\n", size_sent);
    }
    break;
    
    /*----------------------------------------------------------------------
     * The TX_OKAY interrupt should NOT be enabled for states COMBLK_IDLE,
     * COMBLK_WAIT_RESPONSE and COMBLK_RX_RESPONSE.
     */
  case COMBLK_IDLE:
    /* Fall through */
  case COMBLK_WAIT_RESPONSE:
    /* Fall through */
  case COMBLK_RX_RESPONSE:
    /* Fall through */
  default:
    addr = COMBLK_BASE | INT_ENABLE;
    g_write_data8 = g_read_data8 & ~TXTOKAY_MASK;
    data8_write(addr);
    complete_request(0u);
    g_comblk_state = COMBLK_IDLE;
    break;
  }

}

static void handle_rx_okay_irq(void){
  uint16_t data16;
  uint16_t is_command;
  uint8_t data8;
  off_t addr;

  addr = COMBLK_BASE|DATA8;
  data32_read(addr);
  //printf(" handle_rx_okay_irq :%d, data8 = 0x%x\n", g_comblk_state, g_read_data32);
  data16 = (uint16_t) g_read_data32;
  is_command = data16 & DATA8_COMMAND_MASK;
  data8 = (uint8_t)data16;
  
  
  switch(g_comblk_state){
    //----------------------------------------------------------------------
    // MSS_COMBLK_init() enables the RCV_OKAY interrupt for the COMBLK_IDLE
    // state to receive the asynchronous power-on-reset from the system
    // controller.
    //
  case COMBLK_IDLE:
    if(is_command){
      if(data8 != POR_DIGEST_ERROR_OPCODE){
	uint8_t rxed_opcode;
	rxed_opcode = data8;
	//process_sys_ctrl_command(rxed_opcode);
      }
      else{  
	g_comblk_response_idx = 0;
	g_comblk_p_response[g_comblk_response_idx] = data8;
	g_comblk_response_idx++;
	g_comblk_p_response[g_comblk_response_idx] = 0x00u;                
	g_comblk_state = COMBLK_RX_RESPONSE;
      }
    }
    break;
    //----------------------------------------------------------------------
    // The RCV_OKAY interrupt should only be enabled for states
    // COMBLK_WAIT_RESPONSE and COMBLK_RX_RESPONSE. 
    //
  case COMBLK_WAIT_RESPONSE:
    if(is_command){
      uint8_t rxed_opcode;
      rxed_opcode = data8;
      if(rxed_opcode == g_comblk_cmd_opcode){
	g_comblk_response_idx = 0u;
	g_comblk_p_response[g_comblk_response_idx] = rxed_opcode;
	++g_comblk_response_idx;
	g_comblk_state = COMBLK_RX_RESPONSE;
      }else{
	//process_sys_ctrl_command(rxed_opcode);
      }
    }
    break;
    
  case COMBLK_RX_RESPONSE:
    if(is_command){
      uint8_t rxed_opcode;
      rxed_opcode = data8;
      //process_sys_ctrl_command(rxed_opcode);
    }
    else{
      if( g_comblk_p_response[g_comblk_response_idx-1] == POR_DIGEST_ERROR_OPCODE){
	g_comblk_p_response[g_comblk_response_idx] = data8;
	//process_sys_ctrl_command(g_comblk_p_response[g_comblk_response_idx-1]);
	g_comblk_state = COMBLK_IDLE;
      }else{
	if(g_comblk_response_idx < g_comblk_response_size){
	  uint8_t rxed_data;
	  
	  rxed_data = data8;
	  g_comblk_p_response[g_comblk_response_idx] = rxed_data;
	  ++g_comblk_response_idx;
	}
	
	if(g_comblk_response_idx == g_comblk_response_size){
	  complete_request(g_comblk_response_idx);
	  g_comblk_state = COMBLK_IDLE;
	}
      }
    }
    break;
    //----------------------------------------------------------------------
    // The RCV_OKAY interrupt should NOT be enabled for states
    // COMBLK_IDLE, COMBLK_TX_CMD and COMBLK_TX_DATA.
    //
  case COMBLK_TX_PAGED_DATA:
    // This is needed because when there is an error, we need to terminate loading the data 
    if(!is_command){
      g_comblk_p_response[1] = data8;
      complete_request(2u);
      g_comblk_state = COMBLK_IDLE;
    }else{
      uint8_t rxed_opcode;
      rxed_opcode = data8;
      //process_sys_ctrl_command(rxed_opcode);
    }
    break;
    
  case COMBLK_TX_CMD:
    // Fall through
  case COMBLK_TX_DATA:
    // Fall through 
    if(is_command){
      uint8_t rxed_opcode;
      rxed_opcode = data8;
      //process_sys_ctrl_command(rxed_opcode);
    }
    break;
    
  default:
    complete_request(0u);
    g_comblk_state = COMBLK_IDLE;
    break;
  }
  

}


void comblk_get_design_version(uint8_t * p_design_version)
{
  uint8_t response[DESIGNVER_SERV_RESP_LENGTH];
  uint8_t status;
  uint8_t opcode = DESIGNVER_REQUEST_CMD;;  

  status = comblk_execute_service(opcode,
				  p_design_version,
				  response,
				  DESIGNVER_SERV_RESP_LENGTH);
  
  //return status;
}


void comblk_get_serial_number(uint8_t * p_serial_number){
  uint8_t response[SERIAL_NUMBER_SERV_RESP_LENGTH];
  uint8_t status;    
  uint8_t opcode = SERIAL_NUMBER_REQUEST_CMD;
  
  status = comblk_execute_service(opcode,
				  p_serial_number,
				  response,
				  SERIAL_NUMBER_SERV_RESP_LENGTH);
  

  //return status;
}




static uint8_t comblk_execute_service(uint8_t cmd_opcode, uint8_t * cmd_params_ptr,  uint8_t * response, uint16_t response_length)
{
  uint8_t status;
  uint16_t actual_response_length;
  off_t addr;
    
  comblk_send_cmd_with_ptr(cmd_opcode,                    /* cmd_opcode */
			   (uint32_t)cmd_params_ptr,      /* cmd_params_ptr */
			   response,                      /* p_response */
			   response_length               /* response_size */
			   );



    
  
  actual_response_length = g_comblk_response_idx; //wait_for_request_completion();
  
  if((response_length == actual_response_length) && (cmd_opcode == response[0]))
    {
      status = response[1];
    }
  else
    {
      status = 200;
    }
    
  printf("comblk_execute_service result = %d (response[1]) %d (response length) %d (opcode)\n", status, g_comblk_response_idx, response[0]);

  return status;
}

void complete_request(uint16_t response_length)
{
  g_request_in_progress = 0u;
}




/**********************************
 * SPI related routines 
 */

uint8_t verify(void){
  int status; 
  int index;
  buf[0] =  DEVICE_ID_READ ;
  xfer[0].tx_buf = (unsigned long)buf;
  xfer[0].len = 1; /* Length of  command to write*/

  xfer[1].rx_buf = (unsigned long) buf2;
  xfer[1].len = 3; /* Length of Data to read */

  status = ioctl(fd_spi, SPI_IOC_MESSAGE(2), xfer);
  if (status < 0){
    perror("SPI_IOC_MESSAGE");
    return -1;
  }
  printf("SPI Chip ID: %02x %02x %02x\n", buf2[0], buf2[1], buf2[2]);

  if(!(buf2[0]==FLASH_MANUFACTURER_ID && buf2[1]==FLASH_DEVICE_ID)){
    return -1;
  }

  return 1;
}

void spi_global_unprotect(void){
  int status;

  //write enable 
  wait_ready();
  buf[0] = WRITE_ENABLE_CMD; //0x06;
  xfer[0].tx_buf = (unsigned long)buf;
  xfer[0].len = 1;
  status = ioctl(fd_spi, SPI_IOC_MESSAGE(1), xfer);
  if (status < 0){
    perror("SPI_IOC_MESSAGE");
    return;
  }
  
  //Unprotest sector 
  wait_ready();
  buf[0] = WRITE_STATUS1_OPCODE;
  buf[1] = 0;
  xfer[0].tx_buf = (unsigned long)buf;
  xfer[0].len = 2;
  status = ioctl(fd_spi, SPI_IOC_MESSAGE(1), xfer);
  if (status < 0){
    perror("SPI_IOC_MESSAGE");
    return;
  }
}

void wait_ready(void){
  int status;
  uint8_t ready_bit;
  buf[0] = READ_STATUS;

  do {
    xfer[0].tx_buf = (unsigned long)buf;
    xfer[0].len = 1;
    
    xfer[1].rx_buf = (unsigned long) buf2;
    xfer[1].len = 1; /* Length of Data to read */

    status = ioctl(fd_spi, SPI_IOC_MESSAGE(2), xfer);
    
    if (status < 0){
      perror("SPI_IOC_MESSAGE");
      return;
    }
    ready_bit = buf2[0];
    ready_bit = ready_bit & READY_BIT_MASK;
  } while(ready_bit == 1);

} 
uint32_t page_read_handler( uint8_t const ** pp_next_page){
  uint32_t length;

  length = read_page_from_flash(g_flash_rd_buf, BUFFER_A_SIZE);
  *pp_next_page = g_flash_rd_buf;

  
  printf("page_read_handler : 0x%x, %d, 0x%x (0x%x)\n", g_flash_rd_buf[0], length, g_src_image_target_address-SPI_DATA_ADDR, SPI_FILE_SIZE);
  
  return length;
}

static uint32_t read_page_from_flash(uint8_t * g_buffer, uint32_t size){
  
  int target_addr = g_src_image_target_address;
  int readout_size = size;

  

  ///if(g_src_image_target_address+readout_size > SPI_DATA_ADDR + SPI_FILE_SIZE){
  //readout_size = SPI_DATA_ADDR + SPI_FILE_SIZE-g_src_image_target_address;
  //}

  if(g_src_image_target_address>=SPI_DATA_ADDR + SPI_FILE_SIZE){
    return 0;
  }

  spi_read(target_addr, g_buffer, readout_size);
  //  printf("0x%x 0x%x, %d\n", g_src_image_target_address, SPI_DATA_ADDR, (g_src_image_target_address-SPI_DATA_ADDR)%(NOTIFY));
  //if((g_src_image_target_address-SPI_DATA_ADDR)%(NOTIFY) == 0){
  //  printf("0x%x 0x%x, %d\n", g_src_image_target_address, SPI_DATA_ADDR, (g_src_image_target_address-SPI_DATA_ADDR)%(NOTIFY));
  //  printf(".");
    //}
  g_src_image_target_address += readout_size;
  return readout_size;

}

void spi_read(int addr, uint8_t * g_buffer, int nbytes){
  int status; 
  int index=0;
  //memset(buf, 0, sizeof buf);
  buf[0] = READ_ARRAY_OPCODE; //0x03;
  buf[1] = ((addr >> 16) & 0xFF);
  buf[2] = ((addr >> 8) & 0xFF);
  buf[3] = (addr & 0xFF);

  xfer[0].tx_buf = (unsigned long)buf;
  xfer[0].len = 4; /* Length of  command to write*/

  xfer[1].rx_buf = (unsigned long)g_buffer;
  xfer[1].len = nbytes; /* Length of Data to read */

  status = ioctl(fd_spi, SPI_IOC_MESSAGE(2), xfer);
  if (status < 0){
    perror("SPI_IOC_MESSAGE");
    return;
  }

}

void MSS_SYS_start_isp(uint8_t mode){
  
  uint8_t isp_prog_request[2];
  off_t addr;
  uint8_t response_length = ISP_PROG_SERV_RESP_LENGTH;

  addr = SYSREG_ENVM_CR;
  data32_read(addr);
  g_initial_envm_cr = g_read_data32;  

  addr = SYSREG_MSSDDR_FACC1_CR; 
  data32_read(addr);
  g_initial_mssddr_facc1_cr = g_read_data32;

  addr = SYSREG_MSSDDR_FACC2_CR; 
  data32_read(addr);
  g_initial_mssddr_facc2_cr = g_read_data32;
  
  printf("SYSREG_ENVM_CR = 0x%x\n", g_initial_envm_cr);
  printf("SYSREG_MSSDDR_FACC1_CR = 0x%x\n", g_initial_mssddr_facc1_cr);
  printf("SYSREG_MSSDDR_FACC2_CR = 0x%x\n", g_initial_mssddr_facc2_cr);
  
  
  /*
   * Set the eNVM's frequency range to its maximum. This is required to ensure
   * successful eNVM programming on all devices.
   */
  //SYSREG->ENVM_CR = (g_initial_envm_cr & ~NVM_FREQRNG_MASK) | NVM_FREQRNG_MAX;
  g_write_data32 = (g_initial_envm_cr & ~NVM_FREQRNG_MASK) | NVM_FREQRNG_MAX;   
  addr = SYSREG_ENVM_CR;
  data32_write(addr);
  
  g_mode = mode;

  if(mode != MSS_SYS_PROG_AUTHENTICATE){
    /* Select output of MUX 0, MUX 1 and MUX 2 during standby */
    //SYSREG->MSSDDR_FACC2_CR = SYSREG->MSSDDR_FACC2_CR & ((uint32_t)(FACC_STANDBY_SEL << FACC_STANDBY_SHIFT) & FACC_STANDBY_SEL_MASK);
    
    /* Enable the signal for the 50 MHz RC oscillator */
    //SYSREG->MSSDDR_FACC2_CR = SYSREG->MSSDDR_FACC2_CR | ((uint32_t)(MSS_25_50MHZ_EN << MSS_25_50MHZ_EN_SHIFT) & MSS_25_50MHZ_EN_MASK);
    
    /* Enable the signal for the 1 MHz RC oscillator */
    //SYSREG->MSSDDR_FACC2_CR = SYSREG->MSSDDR_FACC2_CR | ((uint32_t)(MSS_1MHZ_EN << MSS_1MHZ_EN_SHIFT) & MSS_1MHZ_EN_MASK);
  }

  isp_prog_request[0] = ISP_PROGRAMMING_REQUEST_CMD;
  isp_prog_request[1] = mode;

  comblk_send_paged_cmd(isp_prog_request,                 /* p_cmd */
			sizeof(isp_prog_request),         /* cmd_size */
			g_isp_response,                   /* p_response */
			ISP_PROG_SERV_RESP_LENGTH        /* response_size */  
			);
  
 
    
  printf("MSS_SYS_start_isp = %d (response[1]) %d (response length) %d (opcode)\n", g_isp_response[1], g_comblk_response_idx, g_isp_response[0]);


}
