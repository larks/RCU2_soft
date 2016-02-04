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

typedef enum nvm_status{
  NVM_SUCCESS = 0,
  NVM_PROTECTION_ERROR,
  NVM_VERIFY_FAILURE,
  NVM_PAGE_LOCK_ERROR,
  NVM_WRITE_THRESHOLD_ERROR,
  NVM_IN_USE_BY_OTHER_MASTER,
  NVM_INVALID_PARAMETER
} nvm_status_t;

static uint32_t wait_nvm_ready(void);
static nvm_status_t get_error_code(uint32_t nvm_hw_status);
nvm_status_t NVM_unlock(uint32_t start_addr, uint32_t length);
unsigned int  envm_write(unsigned int offset, void * buf, unsigned int size);

/*
 * The current code is good only for ENVM0.
 * It needs to be updated to work with ENVM1.
 * ...
 * eNVM control & status registers
 */
struct mss_envm {
  unsigned int            reserved_0_to_80[32];
  unsigned int            wdbuff[32];
  unsigned int            reserved_100_to_120[8];
  unsigned int            status;
  unsigned int            reserved_124_to_128[1];
  unsigned int            nv_page_status;
  unsigned int            nv_freq_rng;
  unsigned int            nv_dpd_b;
  unsigned int            nv_ce;
  unsigned int            reserved_138_to_140[2];
  unsigned int            page_lock_set;
  unsigned int            dwsize;
  unsigned int            cmd;
  unsigned int            reserved_14c_to_154[2];
  unsigned int            inten;
  unsigned int            clrhint;
  unsigned int            reserved_15c_to_1fc[40];
  unsigned int            reqaccess;
};


#define  SYSREG_ENVM_CR              0x4003800C

/*
 * eNVM registers access handle
 */
#define MSS_ENVM_REGS_BASE              0x60080000
#define MSS_ENVM                        ((volatile struct mss_envm *) (MSS_ENVM_REGS_BASE))


/*
 * Base address of the eNVM Flash
 */
#define MSS_ENVM_BASE                   0x60000000

/*
 * eNVM Flash size.
 * TO-DO: this needs to be made a function of some build-time,
 * perhaps even run-time, parameter defining a SmartFusion chip model.
 */
#define MSS_ENVM_FLASH_SIZE             (1024 * 256)

/*
 * eNVM page parameters
 */
#define MSS_ENVM_PAGE_SIZE              128

/*
 * Various bit fields
 */
#define MSS_ENVM_REQACCESS_EXCL         0x1
#define MSS_ENVM_REQACCESS_BY_M3        0x5
#define MSS_ENVM_STATUS_READY           0x1

#define MSS_ENVM_CMD_PROGRAM_ADS        (0x08<<24)
#define VERIFY_ADS                      0x10000000  /* One shot verification with data in WD */
#define USER_UNLOCK                     0x13000000  /* User unlock */

#define WRITE_ERROR_MASK                (MSS_NVM_VERIFY_FAIL | \
					 MSS_NVM_EVERIFY_FAIL |	   \
					 MSS_NVM_WVERIFY_FAIL |	   \
					 MSS_NVM_PEFAIL_LOCK |	   \
					 MSS_NVM_WRCNT_OVER |	   \
					 MSS_NVM_WR_DENIED)

/*******************************************************************************
 * Combined status definitions
 * Below definitions should be used to decoded the bit encoded status returned 
 * by the function MSS_NVM_get_status().
 */
#define MSS_NVM_BUSY_B                  (1u)                    /* NVM is performing an internal operation */
#define MSS_NVM_VERIFY_FAIL             ((uint32_t)1 << 1u)     /* NVM verify operation failed */
#define MSS_NVM_EVERIFY_FAIL            ((uint32_t)1 << 2u)     /* NVM erase verify operation failed */
#define MSS_NVM_WVERIFY_FAIL            ((uint32_t)1 << 3u)     /* NVM write verify operation failed */
#define MSS_NVM_PEFAIL_LOCK             ((uint32_t)1 << 4u)     /* NVM program / erase operation failed due to page lock */
#define MSS_NVM_WRCNT_OVER              ((uint32_t)1 << 5u)     /* NVM write count overflowed */
#define MSS_NVM_WR_DENIED               ((uint32_t)1 << 18u)    /* NVM write is denied due to protection */




//char _mem_ram_buf_size, _mem_ram_buf_base;

//#define SOC_RAM_BUFFER_BASE     (ulong)(&_mem_ram_buf_base)
//#define SOC_RAM_BUFFER_SIZE     (ulong)(&_mem_ram_buf_size)

/*
 * Update an eNVM page.
 * Note that we need for this function to reside in RAM since it
 * will be used to self-upgrade U-boot in eNMV.
 * @param page          eNVM page
 * @param part_begin    updated area start offset in page
 * @param part_end      updated area end offset in page
 * @param buf           data to write
 * @returns             0 -> success
 */
unsigned int envm_write_page_part(
				  unsigned int page, unsigned int part_begin,
				  unsigned int part_end, unsigned char *buf,
				  uint32_t * p_status)
{

  volatile uint32_t ctrl_status;
  uint32_t errors;
  uint32_t length_written;
  *p_status = 0u;

  if (part_begin != 0 || part_end != MSS_ENVM_PAGE_SIZE) {
    memcpy((void *) MSS_ENVM->wdbuff,
	   (const void *) ((unsigned char *) MSS_ENVM_BASE +
			   page * MSS_ENVM_PAGE_SIZE),
	   MSS_ENVM_PAGE_SIZE);
  }

  /*
   * Copy new data to the Write Buffer
   */
  memcpy((void *) ((unsigned char *) MSS_ENVM->wdbuff + part_begin),
	 buf, part_end - part_begin);

  /*
   * Wait for Ready in Status
   */
  while (!(MSS_ENVM->status & MSS_ENVM_STATUS_READY));
  while (!(MSS_ENVM->status & MSS_ENVM_STATUS_READY));

  length_written = part_end - part_begin;


  MSS_ENVM->page_lock_set = 0x0;;
  MSS_ENVM->cmd = USER_UNLOCK | (page * MSS_ENVM_PAGE_SIZE);
  /*
   * Issue the ProgramADS command (ProgramAd + ProgramDa + ProgramStart)
   */
  MSS_ENVM->cmd = MSS_ENVM_CMD_PROGRAM_ADS | (page * MSS_ENVM_PAGE_SIZE);

  /*
   * Wait for Ready in Status
   */
  while (!(MSS_ENVM->status & MSS_ENVM_STATUS_READY));

  /* Wait for NVM to become ready. */
  ctrl_status = wait_nvm_ready();
  /* Check for errors. */
  errors = ctrl_status & WRITE_ERROR_MASK;
  if(errors){
    length_written = 0u;
    *p_status = MSS_ENVM->status;
  }else{
    /* Perform a verify. */
  
    MSS_ENVM->cmd = VERIFY_ADS | (page * MSS_ENVM_PAGE_SIZE);
    /* Wait for NVM to become ready. */
    while (!(MSS_ENVM->status & MSS_ENVM_STATUS_READY));
    ctrl_status = wait_nvm_ready();
    /* Check for errors. */
    errors = ctrl_status & WRITE_ERROR_MASK;
    if(errors){
      length_written = 0u;
      *p_status = MSS_ENVM->status;
    }
  }

  return length_written;
}

int max(int a, int b){
  if(a>b) return a;
  else return b;
}

int min(int a, int b){
  if(a>b) return b;
  else return a;
}
/*
 * Write a data buffer to eNVM.
 * Note that we need for this function to reside in RAM since it
 * will be used to self-upgrade U-boot in eNMV.
 * @param offset        eNVM offset
 * @param buf           data to write
 * @param size          how many bytes to write
 * @returns             number of bytes writter
 */
unsigned int envm_write(unsigned int offset, void *buf, unsigned int size)
{

  unsigned int page, first_page, last_page;
  nvm_status_t status;
  unsigned int written_size=0;


  /*
   * Open exlusive access to eNVM by Cortex-M3
   */
  MSS_ENVM->reqaccess = MSS_ENVM_REQACCESS_EXCL;
  while (MSS_ENVM->reqaccess != MSS_ENVM_REQACCESS_BY_M3);

  printf("reqaccess = 0x%x\n", MSS_ENVM->reqaccess);
  /*
   * Update eNVM, page by page
   */
  first_page = offset / MSS_ENVM_PAGE_SIZE;
  last_page = (offset + size - 1) / MSS_ENVM_PAGE_SIZE;


  for (page = first_page; page <= last_page; page++) {
    uint32_t length_written;
    uint32_t nvm_hw_status = 0u;

    length_written = envm_write_page_part(page,
					  max(0, offset - page * MSS_ENVM_PAGE_SIZE),
					  min(MSS_ENVM_PAGE_SIZE,
					      offset + size - page * MSS_ENVM_PAGE_SIZE),
					  (unsigned char *)buf +
					  max(page * MSS_ENVM_PAGE_SIZE - offset, 0),
					  &nvm_hw_status);
    
    if(0u == length_written){
      status = get_error_code(nvm_hw_status);
      printf("cptf (page=0x%x)::get_error_code = 0x%x (0x%x)\n", page,status, nvm_hw_status);
    }else{
      printf("cptf (page=0x%x)::verified size=0x%x\n", page, length_written);
    }
    written_size += length_written;
  }

  /*
   * Disable access to eNVM by Cortex-M3 core
   */
  MSS_ENVM->reqaccess = ~MSS_ENVM_REQACCESS_EXCL;
  
  //return size;

  return written_size;

}

/**************************************************************************//**
  Generate error code based on NVM status value.  
  The hardware nvm status passed as parameter is expected to be masked using the
  following mask:
                (MSS_NVM_VERIFY_FAIL | \
                 MSS_NVM_EVERIFY_FAIL | \
                 MSS_NVM_WVERIFY_FAIL | \
                 MSS_NVM_PEFAIL_LOCK | \
                 MSS_NVM_WRCNT_OVER | \
                 MSS_NVM_WR_DENIED)
*/
static nvm_status_t get_error_code(uint32_t nvm_hw_status)
{
  nvm_status_t status;
    
  if(nvm_hw_status & MSS_NVM_WR_DENIED)
    {
      status = NVM_PROTECTION_ERROR;
    }
  else if(nvm_hw_status & MSS_NVM_WRCNT_OVER)
    {
      status = NVM_WRITE_THRESHOLD_ERROR;
    }
  else if(nvm_hw_status & MSS_NVM_PEFAIL_LOCK)
    {
      status = NVM_PAGE_LOCK_ERROR;
    }
  else
    {
      status = NVM_VERIFY_FAILURE;
    }
    
  return status;
}


/**************************************************************************
* Wait for NVM to become ready from busy state
*/
static uint32_t wait_nvm_ready(void) 
{
  volatile uint32_t ctrl_status;
  uint32_t ctrl_ready;
  uint32_t inc;
    
  /*
   * Wait for NVM to become ready.
   * We must ensure that we can read the ready bit set to 1 twice before we
   * can assume that the other status bits are valid. See SmartFusion2 errata.
   */
  for(inc = 0u; inc < 2u; ++inc)
    {
      do {
	ctrl_status = MSS_ENVM->status;
	ctrl_ready = ctrl_status  & MSS_NVM_BUSY_B;
      } while(0u == ctrl_ready);
    }
    
  return ctrl_status;
}

/*
 * Write the eNVM and, optionally, reset the CPU.
 * We need it in RAM so as to be able to update U-boot,
 * which itself runs from the eNVM.
 * NOTE: This function is only used locally, nevertheless, the "static"
 * attrubute was removed. This was done because otherwise GCC optimizes
 * this function to be inlined into do_cptf(), which is located in flash.
 * This makes the U-Boot crash while performing self-upgrade.
 */
int envm_write_and_reset(int dst, int src, int size, int do_reset)
{
  int ret = 0;

  /*
   * Copy the buffer to the destination.
   */
  int write_size = envm_write((int) dst, (void *) src, (int) size) ;
  if (write_size != size) {
    ret = -1;
    printf("envm_write failed. size written in the eNVM doesn't match. file size=0x%x. written size=0x%x...\n",
	   size, write_size);
    goto Done;
  }

  /*
   * If the user needs a reset, do the reset
   */
  if (do_reset) {
    /*
     * Cortex-M3 core reset.
     */
    //<reset_cpu(0);

    /*
     * Should never be here.
     */
  }

 Done:
  return ret;


}
/*
  Features that the old peekXX/pokeXX did not have:
  1. Support for 8/16/32 bit READ/WRITE in one function
  2. Support for decimal and binary values
  3. The value return is returned (to become the status code)

  cptf [offset addr] filename

 */


char *data_buffer;

int main(int argc, char **argv) {

  int dst;
  //int src = 0x20008000;// SOC_RAM_BUFFER_BASE;
  int size = 0x4000; //SOC_RAM_BUFFER_SIZE;
  int do_reset = 0;
  int ret = 0;
  int file_size=0;
  int i=0;
  int rd_size=0;

  FILE *infile;

  /*
   * Check that at least the destination is specified
   */
  if (argc != 3) {
    printf("%s: offset and eNVM file must be specified\n",
	   (char *) argv[0]);
    return ret;
  }

  /*
   * Parse the command arguments
   */
  dst = parseNumber(argv[1]); ///offset address 
  infile = fopen(argv[2],"rb");
  if(infile==NULL){
    printf("no input file \n");
  }

  ////open eNVM file and write to the DDR memory 

  fseek(infile, 0, SEEK_END );
  file_size = ftell(infile);

  data_buffer = (char*) malloc(file_size);

  fseek(infile, 0, SEEK_SET);
  rd_size = fread(data_buffer, sizeof(char), file_size, infile);
  
  printf("dst=0x%x, File_size = 0x%x, data loc %p, readout size=0x%x\n", dst, file_size, data_buffer, rd_size);

  for(i=0;i<128;i++){
    if(i%16==0){ 
      printf("\n");
    }
    printf("%02x",data_buffer[i]);
  }
  printf("\n");
  
  printf("%s: Updating eNVM. Please wait ...\n", (char *) argv[0]);

  size = file_size;
  
  if (envm_write_and_reset(dst, data_buffer, size, do_reset)) {
    printf("%s: nvm_write_and_reset failed\n", (char *) argv[0]);
  }else{
    printf("cptf done. please reboot the board... \n");
  }

  free(data_buffer);
  return ret;

}
