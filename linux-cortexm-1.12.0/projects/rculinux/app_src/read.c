#include <stdio.h>
#include <errno.h>	/* Error numbers */
#include <stddef.h> /* offsetof() */
#include <stdint.h> /* int types */
#include <sys/mman.h> /* mmap */
#include <sys/stat.h>
#include <fcntl.h> /* O_RDWR */
#include <stdlib.h> /* exit() */
//#include "regmap.h" /* register definitions */
#include "sys_services.h"
#include "comblk.h"
//#include <asm/hardware/nvic.h>

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <asm/uaccess.h>
#include <linux/fs.h>      // basic file structures and methods                                   
#include <linux/types.h>   // basic data types                                                    
#include <linux/sched.h>   // task schedule                                                        
#include <linux/errno.h>   // error codes                                                         
#include <linux/slab.h>    // kmalloc, kmfree                                                     
#include <asm/io.h>        // ioremap, ...                                                        
//#include <asm/uaccess.h>   // user space memory access (e.g. copy_to_user)                      
//#include <linux/poll.h>                                                                         
#include <linux/mm.h>      // virtual memory mapping                                              
#include <linux/ioport.h>  // detecting and reserving system resources                            
#include <linux/spinlock.h>
#include <asm/system.h>


static int register_init(void);
static int register_exit(void);


#include <linux/compiler.h>

#define V7M_SCS                         0xe000e000
#define NVIC_INTR_CTRL                  (V7M_SCS + 0x004)
#define NVIC_SYSTICK_CTRL               (V7M_SCS + 0x010)
#define NVIC_SYSTICK_RELOAD             (V7M_SCS + 0x014)
#define NVIC_SYSTICK_CURRENT            (V7M_SCS + 0x018)
#define NVIC_SYSTICK_CALIBRATION        (V7M_SCS + 0x01c)
#define NVIC_SET_ENABLE                 (V7M_SCS + 0x100)
#define NVIC_CLEAR_ENABLE               (V7M_SCS + 0x180)
#define NVIC_SET_PENDING                (V7M_SCS + 0x200)
#define NVIC_CLEAR_PENDING              (V7M_SCS + 0x280)
#define NVIC_ACTIVE_BIT                 (V7M_SCS + 0x300)
#define NVIC_PRIORITY                   (V7M_SCS + 0x400)
#define NVIC_INTR_CTRL_STATE            (V7M_SCS + 0xd04)
#define NVIC_SOFTWARE_INTR              (V7M_SCS + 0xf00)


//	COMBLK_TypeDef * COMBLK;
//	SYSREG_TypeDef * SYSREG;

int main(void){
	/* variables */
  uint8_t serial_number[16];
  uint8_t user_code[4];
  uint8_t design_version[2];
  uint8_t device_certificate[512];
  uint8_t status;
  int i;
  
  
	
	/* isp file stuff */
/*
	FILE *fp;
	fp = fopen("my_file.isp", "rb");
	if(fp == NULL){fprintf(stderr, "Could not open isp file - errno: %d", ERRNO)}
*/

  register_init();
    
  fprintf(stdout, "CONTROL: 0x%x, STATUS: 0x%x, INT_ENABLE: 0x%x\n", 
	  COMBLK->CONTROL, COMBLK->STATUS, COMBLK->INT_ENABLE);
  fprintf(stdout, "Addresses -- COMBLK: %p, CTRL: %p, STATUS: %p, INT_ENABLE: %p\n",
	  (void*)COMBLK, &COMBLK->CONTROL, &COMBLK->STATUS, &COMBLK->INT_ENABLE);
  
  
  fprintf(stdout, "mmap init done.\n");
  MSS_SYS_init(MSS_SYS_NO_EVENT_HANDLER);
  fprintf(stdout, "MSS_SYS_init done.\n");
  /* Read device serial number (DSN) */
  status = MSS_SYS_get_serial_number(serial_number);
  fprintf(stdout, "Get serial done. Status: %d\n", status);
    /* Read design version */
  status = MSS_SYS_get_design_version(design_version);
  fprintf(stdout, "Get design version done. Status: %d\n", status);
  
  // if(MSS_SYS_SUCCESS == status)
  //{
  fprintf(stdout, "Device serial number: %s\n", (char *)serial_number);
  //for(i=0; i<16; i++){fprintf(stdout, "%d", serial_number[i]);}
  fprintf(stdout, "Design version: %s\n", (char*) design_version);
  //}	
  

  /*
    fprintf(stdout, "version: %x, SR: %x, SYSREG address: %x\n", 
    SYSREG->ENVM_STATUS, SYSREG->DEVICE_SR, SYSREG);
  */	
  
  
  register_exit();
  return 0;
}


/*
  static void isp_operation_menu(void)
{
	fprintf(stdout, "Hello\n");
}
*/

/* Initialize memories for ISP operations */
static int register_init(void)
{
  /* Open /dev/mem */
  int mem_fd = open("/dev/mem", O_RDWR | O_SYNC);
  int kmem_fd = open("/dev/kmem", O_RDWR | O_SYNC);
  if (mem_fd<0) {
    fprintf(stderr, "open(\"/dev/mem\") failed\n");
    exit(-1);
  }
  
  //nvic_init();
  
  /* Map COMBLK registers to mem */
  //struct COMBLK_TypeDef * COMBLK;
  fprintf(stdout, "COMBLK size: %d\n", sizeof(COMBLK)); // debug
  COMBLK = mmap(NULL, 96, PROT_READ|PROT_WRITE, MAP_SHARED, mem_fd, (off_t)COMBLK_BASE);
  if(COMBLK==MAP_FAILED) {
    fprintf(stderr, "COMBLK map failed\n");
    exit(-1);
  }
  else fprintf(stdout, "Mapped COMBLK\n");
  
  /* Map SYSREG to mem */
  //struct SYSREG_TypeDef * SYSREG;
  
  SYSREG = mmap(NULL, 692, PROT_READ|PROT_WRITE, MAP_SHARED, mem_fd, (off_t)SYSREG_BASE); // fixed mistake, forgot _BASE
  if(SYSREG==MAP_FAILED) {
    fprintf(stderr, "SYSREG map failed\n");
    exit(-1);
  }
  else fprintf(stdout, "Mapped SYSREG\n");
  /* Soft reset on comblk */
  SYSREG->SOFT_RST_CR = SYSREG_COMBLK_SOFTRESET_MASK;
  SYSREG->SOFT_RST_CR &= ~(SYSREG_COMBLK_SOFTRESET_MASK);
  fprintf(stdout, "reset comblk: 0x%x\n", SYSREG->SOFT_RST_CR);
  
  /* Map NVIC (not working, protected from user space i guess */
  //fprintf(stdout, "%#x\n", NVIC_BASE);
  //NVIC = mmap(NULL, 4096, PROT_READ|PROT_WRITE, MAP_FIXED, kmem_fd, (off_t)NVIC_BASE);
  //if(NVIC==MAP_FAILED) {
  //  fprintf(stderr, "NVIC map failed\n");
  //  exit(-1);
  // }
  //else fprintf(stdout, "Mapped NVIC\n");
  

  //nvic_init();
  //printf(stdout, "NVIC init done\n");

  /* New proposal: 
     
     nvic = open(/dev/NVIC);
     
     write("28Clear");
     write("28Enable");
     etc... ?
     
     close(nvic);
     
     
     
  */
  
  
  unsigned int max_irq;
  max_irq = ((readl(NVIC_INTR_CTRL) & 0x1f) + 1) * 32;



  
  
  	return 0;
}

static int register_exit(void)
{
  if( munmap(COMBLK, 96) < 0 ){
    fprintf(stderr, "munmap(COMBLK) failed\n");
    exit(-1);
  }
  else if( munmap(SYSREG, 692) < 0 ){
    fprintf(stderr, "munmap(SYSREG) failed\n");
    exit(-1);
  }
  
  else if( munmap(NVIC, 4096) < 0 ){
    fprintf(stderr, "munmap(SYSREG) failed\n");
    exit(-1);
  }
  
  else fprintf(stdout, "munmap() success\n");
  return 0;
}



/* Handling functions */
/*
uint32_t page_read_handler( uint8_t const ** pp_next_page )
{
	uint32_t length;
	length = read_page_from_disk( g_page_buffer, BUFFER_SIZE );
	*pp_next_page = g_page_buffer;
	return length;
}

static uint32_t read_page_from_disk(uint8_t * g_buffer, uint32_t length)
{
	uint32_t num_bytes;
	
	num_bytes = length;
	
	return num_bytes
}

static uint32_t read_page_from_flash(void)
{
	return 0;
}

void isp_completion_handler(uint32_t value)
{
	g_error_flag = value;
	g_isp_operation_busy = 0;
}
*/
