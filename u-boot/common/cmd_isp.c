/*
 * Command for accessing SPI flash.
 *
 * Copyright (C) 2008 Atmel Corporation
 * Licensed under the GPL-2 or later.
 */


#include <common.h>
#include <serial.h>
#include <ns16550.h>
#include <asm/io.h>
#include <spi_flash.h>
#include <netdev.h>
#include <asm/arch/ddr.h>
#include <asm/arch/comblk.h>
#include <asm/arch/m2s.h>
#include <malloc.h>

/*------------------------------------------------------------------------------
 * delay routines...
 */
void delay(uint32_t time);

/*------------------------------------------------------------------------------
 * IRQ_handler in Comblk... This IRQ is registerd in cpu/arm_cortexm3/start.c
 */
void exec_ComBlk_IRQHandler(void);

/*------------------------------------------------------------------------------
 * some external functions to be used in this application 
 * serial console, clock frequency updates, run_command, reset
 * dram_init, dram_init_no_secded, and board_init to re-init the boards after FF is over
*/
static NS16550_t serial_ports = (NS16550_t)CONFIG_SYS_NS16550_COM1;
extern void NS16550_init(NS16550_t com_port, int baud_divisor);
extern void clock_update(enum clock clck, unsigned long val);
extern int run_command (const char *cmd, int flag);
extern int do_reset (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[]);		
extern int dram_init(void);
//extern int dram_init_no_secded(void);
extern int board_init(void);

/*------------------------------------------------------------------------------
 * user functions for the ISP Programming
 */
static void exec_abort_current_cmd(void);
static void exec_send_cmd_opcode(uint8_t opcode);
static u32 exec_fill_tx_fifo(const uint8_t * p_cmd, uint32_t cmd_size);
static void exec_handle_tx_okay_irq(void);
static void exec_handle_rx_okay_irq(void);
static void exec_complete_request(u16 response_length);
static void process_sys_ctrl_command(uint8_t cmd_opcode);
static void exec_clock_mss_learn(u32 m2s_sys_clock, u32 mode);
static void exec_spi_flash_probe(u32 speed);
static u32 exec_read_page_from_flash(u8 * g_buffer, u32 size);
static u32 exec_comblk_read_page_handler(u8 const ** pp_next_page);
static void exec_comblk_send_cmd_with_ptr(u8 cmd_opcode,
					  u32 cmd_params_ptr,
					  u8 * p_response,
					  u16 response_size);
static void exec_comblk_send_paged_cmd(u8 * p_cmd, 
				       u16 cmd_size,
				       u8 * p_response, 
				       u16 response_size);
void exec_comblk_get_serial_number_out(u8 *p_response);
static void exec_comblk_get_serial_number(void);
static void exec_comblk_get_design_version(void);
static void exec_clock_switch(void);
static void exec_start_isp(uint8_t mode);
static void exec_comblk_init(void);
static int do_isp_prog(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[]);
static int exec_spi_flash_read(long offset);
static void exec_clock_revert(void);

//////////////////////


/*------------------------------------------------------------------------------
 * For COMBLK transactions 
 */
#define SERIAL_NUMBER_REQUEST_CMD   1;
#define DESIGN_VERSION_REQUEST_CMD  5;
#define DIGEST_CHECK_REQUEST_CMD    23;

#define FLASH_FREEZE_REQUEST_CMD        2;
#define FLASH_FREEZE_SHUTDOWN_CMD     224;
#define ISP_PROGRAMMING_REQUEST_CMD    21;

#define MSS_SYS_PROG_AUTHENTICATE    0u
#define MSS_SYS_PROG_PROGRAM         1u
#define MSS_SYS_PROG_VERIFY          2u

#define NVM_FREQRNG_MASK        0x00001FE0
#define NVM_FREQRNG_MAX         ((uint32_t)0xFF << 5)


/*------------------------------------------------------------------------------
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

#define FLASH_FREEZE_SHUTDOWN_OPCODE                    0xE0
#define FLASH_FREEZE_EXIT_OPCODE                        0xE1
#define POR_DIGEST_ERROR_OPCODE                         0xF1

#define DELAY_MORE_THAN_10US        10000000

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

static volatile u8   g_last_response_length = 0u;
static volatile u8   g_comblk_state = COMBLK_IDLE;
static volatile u8   g_comblk_cmd_opcode = 0u;
static const u8  *g_comblk_p_cmd = 0u;
static volatile u16  g_comblk_cmd_size = 0u;
static const u8  *g_comblk_p_data = 0u;
static volatile u32  g_comblk_data_size = 0u;
static u8  *g_comblk_p_response = 0u;
static u16  g_comblk_response_size = 0u;
static volatile u16  g_comblk_response_idx = 0u;
static volatile u8 g_request_in_progress = 0u;
//static volatile u16 g_last_response_length = 0u;

static volatile u32 g_initial_envm_cr = 0x000000F1U;
static volatile u32 g_initial_mssddr_facc1_cr = 0xA402125;
static volatile u32 g_initial_mssddr_facc2_cr = 0x1F19;
static volatile u8 g_mode = 0;
static volatile u8 wait_for_clock_switch = 1;

static u8 g_isp_response[ISP_PROG_SERV_RESP_LENGTH];
static u8 design_version[2];

/*------------------------------------------------------------------------------
 * spi flash related variables
 */
#ifndef CONFIG_SF_DEFAULT_SPEED
# define CONFIG_SF_DEFAULT_SPEED	1000000
#endif
#ifndef CONFIG_SF_DEFAULT_MODE
# define CONFIG_SF_DEFAULT_MODE		SPI_MODE_3
#endif
static struct spi_flash *spi_flash;
#define BUFFER_A_SIZE  1024 
#define SPI_CLOCK_FF  50000000  //in FF state, 50MHz SPI clock is only the solution...


/*------------------------------------------------------------------------------
 * variables for the spi reading buffers, addresses, etc
 */
static long g_src_image_target_address = 0;
static u8 g_flash_rd_buf[1024];
static ulong SPI_FILE_SIZE = 0;
static ulong SPI_DATA_ADDR = 0;


/*------------------------------------------------------------------------------
 * clock settings in FF state 
 */
const u32 apb_divisors_mask = 0x00000EFCU;
//const u32 apb_divisors_mask = 0xa403100;


/*------------------------------------------------------------------------------
 * clock related variables
 */
static unsigned long clock_base[CLOCK_END];


/* ------------------------------------------------------------------------------
 * IPS programming results 
 */
static u8 isp_programming_results[3]; //={0xff,0xff,0xff};


/* ------------------------------------------------------------------------------
 *  For SECDED driver.... This is just template.... (should be moved somewhere else)
 */ 
//void exec_ECC_Error_IRQHandler(void);
//#define ECC_Error_IRQn   29


/* ------------------------------------------------------------------------------
 *  usart debug mode
 */ 
static u8 debug_uart ;

static u8 chip_select;

static unsigned int exec_clock_mss_divisor(unsigned int r, unsigned int s)
{
  unsigned int v, ret;

  /*
   * Get a 3-bit field that defines the divisor
   */
  v = (r & (0x7<<s)) >> s;

  /*
   * Translate the bit representation of the divisor to 
   * a value ready to be used in calculation of a clock.
   */
  switch (v) {
  case 0: ret = 1; break;
  case 1: ret = 2; break;
  case 2: ret = 4; break;
  case 4: ret = 8; break;
  case 5: ret = 16; break;
  case 6: ret = 32; break;
  default: ret = 1; break;
  }

  return ret;
}

static void exec_clock_mss_learn(u32 m2s_sys_clock, u32 mode)
{  

  u32 r1 = M2S_SYSREG->mssddr_facc1_cr;
  u32 r2 = M2S_SYSREG->mssddr_pll_status_low_cr;

  if(mode==0){ //normal operation

    /*
     * System reference clock is defined as a build-time constant.
     * This clock comes from the FPGA PLL and we can't determine
     * its value at run time. All clocks derived from CLK_BASE
     * can be calculated at run time (and we do just that).
     */
    
    clock_base[CLOCK_SYSREF] = m2s_sys_clock; //CONFIG_SYS_M2S_SYSREF;
    /*
     * Respectively:
     * M3_CLK_DIVISOR
     * DDR
     * APB0_DIVISOR
     * APB1_DIVISOR
     * FIC32_0_DIVISOR
     */
    clock_base[CLOCK_SYSTICK] = clock_base[CLOCK_SYSREF] / exec_clock_mss_divisor(r1, 9);
    clock_base[CLOCK_DDR] = clock_base[CLOCK_SYSREF] / exec_clock_mss_divisor(r2, 16);
    clock_base[CLOCK_PCLK0] = clock_base[CLOCK_SYSREF] / exec_clock_mss_divisor(r1, 2);
    clock_base[CLOCK_PCLK1] = clock_base[CLOCK_SYSREF] / exec_clock_mss_divisor(r1, 5);
    clock_base[CLOCK_FPGA] = clock_base[CLOCK_SYSREF] / exec_clock_mss_divisor(r1, 13);
  }else if(mode==1){ //standby FF state
    clock_base[CLOCK_SYSREF] = m2s_sys_clock;
    clock_base[CLOCK_SYSTICK] = m2s_sys_clock; 
    clock_base[CLOCK_DDR] = m2s_sys_clock; 
    clock_base[CLOCK_PCLK0] = m2s_sys_clock; 
    clock_base[CLOCK_PCLK1] = m2s_sys_clock; 
    clock_base[CLOCK_FPGA] = m2s_sys_clock; 

  }
}


static int exec_calc_divisor(NS16550_t port){

  u32 baudrate = 115200;
  u32 MODE_X_DIV = 16;
  u32 NS16550_CLK = clock_base[CLOCK_PCLK1];
  
  return (NS16550_CLK + (baudrate * (MODE_X_DIV / 2))) /
    (MODE_X_DIV * baudrate);
 
}

static void exec_spi_flash_probe(u32 speed)
{
  unsigned int bus = 0;
  unsigned int cs = chip_select;
  unsigned int mode = CONFIG_SF_DEFAULT_MODE;
  struct spi_flash *new;
    
  if (spi_flash)
    spi_flash_free(spi_flash);


  ///disable IRQ
  //NVIC_DisableIRQ( this_spi->irqn );
  NVIC->ICER[((uint32_t)(ComBlk_SPI0) >> 5)] = (1 << ((uint32_t)(ComBlk_SPI0) & 0x1F));

  /* reset SPI0 */
  M2S_SYSREG->soft_reset_cr |= (((u32) 0x01 << 9)); //SYSREG_SPI0_SOFTRESET_MASK;
  /* Clear any previously pended SPI0 interrupt */
  NVIC->ICPR[((uint32_t)(ComBlk_SPI0) >> 5)] = (1 << ((uint32_t)(ComBlk_SPI0) & 0x1F));
  /* Take SPI0 out of reset. */
  M2S_SYSREG->soft_reset_cr &= ~(((u32) 0x01 << 9)); //SYSREG_SPI0_SOFTRESET_MASK;

  new = spi_flash_probe(bus, cs, speed, mode);
  if (!new) {
    printf("Failed to initialize SPI flash at %u:%u\n", bus, cs);
    return ;
  }
  
  spi_flash = new;
  
  printf("%u KiB %s at %u:%u is now current device\n",
	spi_flash->size >> 10, spi_flash->name, bus, cs);
  


  ////dump the SPI related register
  /*
  struct m2s_spi_slave *s = container_of(slave, struct m2s_spi_slave, slave); 
  printf("MSS SPI %p\n", MSS_SPI(s));
  printf(" --- control = 0x%x \n", MSS_SPI(s)->control);
  printf(" --- txrxdf_size = 0x%x \n", MSS_SPI(s)->txrxdf_size);
  printf(" --- status = 0x%x \n", MSS_SPI(s)->status);
  printf(" --- int_clear = 0x%x \n", MSS_SPI(s)->int_clear);
  printf(" --- rx_data = 0x%x \n", MSS_SPI(s)->rx_data);
  printf(" --- tx_data = 0x%x \n", MSS_SPI(s)->tx_data);
  printf(" --- clk_gen = 0x%x \n", MSS_SPI(s)->clk_gen);
  printf(" --- slave_select = 0x%x \n", MSS_SPI(s)->slave_select);
  printf(" --- mis = 0x%x \n", MSS_SPI(s)->mis);
  printf(" --- ris = 0x%x \n", MSS_SPI(s)->ris);
  */

  //NVIC_EnableIRQ( this_spi->irqn );
  //NVIC->ISER[((uint32_t)(ComBlk_SPI0) >> 5)] = (1 << ((uint32_t)(ComBlk_SPI0) & 0x1F)); /* enable interrupt */

  return ;  
}



static int exec_spi_flash_read(long offset)
{

  unsigned long len = 64; //BUFFER_A_SIZE; //128;
  int ret;
  void *buf;
  
  printf("exec_spi_flash_read. flash at 0x%p (spi=%p)\n", spi_flash, spi_flash->spi);
  delay(10000);
  

  buf = (u8 *)malloc(len);
  if (!buf) {
    printf("Failed to map physical memory\n");
    return 1;
  }

  ret = spi_flash->read(spi_flash, offset, len, buf);
  if (ret) {
    printf("SPI flash read failed\n");
    return 0;
  }else{
    printf("data from SPI Flash (0x%p) = ", buf);
    delay(10000);
  }
  printf("\n");
  print_buffer(buf, buf, 4, len/4, 4);
  free(buf);
  return 1;  
}


static void process_sys_ctrl_command(u8 cmd_opcode)
{
  //g_async_event_handler(cmd_opcode);
  
  if (cmd_opcode == FLASH_FREEZE_SHUTDOWN_OPCODE){

    if(debug_uart==1){
      printf(" ***** FLASH_FREEZE_SHUTDOWN_OPCODE ***** \n");
    }

    u32 running_on_standby_clock;
    volatile u32 timeout;
    //u32 clock_divisor;
    //const u32 apb_divisors_mask = 0x00000EFC;      
    //
    // Wait for the System Controller to switch the system's clock
    // from the main clock to the  standby clock. This should take place
    // within 10us of receiving the shut-down event.
    //
    //timeout = DELAY_MORE_THAN_10US;

    //M2S_SYSREG->soft_reset_cr |= (((u32) 0x01 << 9)); //SYSREG_SPI0_SOFTRESET_MASK;


    timeout = 100000;
    do
      {
	running_on_standby_clock = M2S_SYSREG->mssddr_facc1_cr & 0x00001000;
	--timeout;
      }
    while ((running_on_standby_clock == 0) && (timeout != 0));


    //
    // Set the clock divisors to zero in order to set the AHB
    // to APB bridge's clock ratio between the AHB and APB busses to 1.
    // This is required to ensure correct bus transactions to the APB
    // peripherals while operating from the standby clock.
    //
    M2S_SYSREG->mssddr_facc1_cr &= ~apb_divisors_mask;
    //M2S_SYSREG->mssddr_facc1_cr |= apb_divisors_mask;
    

    
    //  M2S_SYSREG->mssddr_facc2_cr = 0x1e01; 

    //M2S_SYSREG->soft_reset_cr &= ~(((u32) 0x01 << 9)); //SYSREG_SPI0_SOFTRESET_MASK;

    ///exec_spi_flash_probe(CONFIG_SF_DEFAULT_SPEED);    

    
    //// uart and spi flash are already re-initialized in exec_clock_switch
    
    //exec_clock_mss_learn(50000000);
    //clock_divisor = exec_calc_divisor(serial_ports); 
    //NS16550_init(serial_ports, clock_divisor);
      



    /*
    printf(" acc1=0x%x, facc2=0x%x, clk0=%d, clk1=%d, SPI clock=%d timeout=%d\n", 
	   M2S_SYSREG->mssddr_facc1_cr, 
	   M2S_SYSREG->mssddr_facc2_cr, 
	   clk1, clk2, SPI_CLOCK_FF, timeout);
    printf("FLASH_FREEZE_SHUTDOWN_OPCODE\n");
    */
    //udelay(10000);

    //exec_clock_switch();

    //delay(10000);

    //exec_spi_flash_probe(SPI_CLOCK_FF);


  }else if(cmd_opcode == FLASH_FREEZE_EXIT_OPCODE){

    u32 running_on_standby_clock;
    volatile u32 timeout;
    //
    // Wait for the System Controller to switch the system's clock
    // from the standby clock to the main clock. This should take place
    // within 10us of receiving the shut-down event.
    //

    /*
    u32 pll_locked;    
    do {
      pll_locked = M2S_SYSREG->mssddr_pll_status & 0x1;
    } while(pll_locked);
    do {
      pll_locked = M2S_SYSREG->mssddr_pll_status & 0x2;
    } while(pll_locked==0x2);
    */

    //M2S_SYSREG->soft_reset_cr |= (((u32) 0x01 << 7)); 



    timeout = DELAY_MORE_THAN_10US;
    do
      {
	running_on_standby_clock = M2S_SYSREG->mssddr_facc1_cr & 0x00001000;
	--timeout;
      }
    while ((running_on_standby_clock != 0) && (timeout != 0));

    // Restore the MSS clock dividers to their normal operations value. 
    
    //M2S_SYSREG->mssddr_facc1_cr = g_initial_mssddr_facc1_cr;
    


    
    //M2S_SYSREG->soft_reset_cr &= ~(((u32) 0x01 << 7)); 

    
    //exec_clock_revert();

    //// this needs to be updated.......
    //exec_clock_mss_learn(50000000);
    //clock_divisor = exec_calc_divisor(serial_ports); 
    //NS16550_init(serial_ports, clock_divisor);
    /*
    printf(" facc1=0x%x, facc2=0x%x, clk0=%d, clk1=%d. SPI clock=%d\n", 
	   M2S_SYSREG->mssddr_facc1_cr, 
	   M2S_SYSREG->mssddr_facc2_cr, 
	   clk1, clk2, SPI_CLOCK_FF);
    printf("FLASH_FREEZE_EXIT_OPCODE\n");
    */
    //udelay(1000);
    //delay(10000);
    //exec_spi_flash_probe(CONFIG_SF_DEFAULT_SPEED);

  }else{
    //
    //if ((event_opcode == POR_DIGEST_ERROR_OPCODE) ||		       
    //((event_opcode >= TAMPER_ATTEMPT_DETECT_OPCODE_RANGE_MIN) &&	
    //(event_opcode <= TAMPER_FAILURE_DETECT_OPCODE_RANGE_MAX)) ||	
    //(event_opcode == TAMPER_CLOCK_MONITOR_ERROR_OPCODE) ||		
    //((event_opcode >= TAMPER_HARDWARE_MONITOR_ERROR_OPCODE_RANGE_MIN) && 
    //(event_opcode <= TAMPER_HARDWARE_MONITOR_ERROR_OPCODE_RANGE_MAX))){
    //;
    //}
  }
  
}


static void exec_comblk_init(void){

  printf("COMBLK: Control register = 0x%x\n", COMBLK->CONTROL);
  printf("COMBLK: SYSREG -> Soft_RST_CR = 0x%x\n", M2S_SYSREG->soft_reset_cr);

  /* init nvic */
  u32 max_irq = ((NVIC_CTR->data & 0x1f)+1)*32;
  u32 i;
  for(i = 0; i < max_irq / 32; i++){
    NVIC->ICER[i] = 0x0FFFFFFF;
  }
  for(i = 0; i < max_irq / 4; i++){
    NVIC->IP[i] = 0x0;
  }

  printf("NVIC (init) : max_irq = %d, ICER = 0x%x (%p), IP = 0x%x (%p)\n",
	 max_irq, NVIC->ICER[((uint32_t)(ComBlk_IRQn) >> 5)], 
	 &(NVIC->ICER[((uint32_t)(ComBlk_IRQn) >> 5)]),
	 NVIC->IP[((uint32_t)(ComBlk_IRQn) >> 5)], 
	 &(NVIC->IP[((uint32_t)(ComBlk_IRQn) >> 5)]));


  printf("NVIC : CPU ID = 0x%x, ICSR = 0x%x, INTR_CTRL = 0x%x at %x(%x)\n",
	 SCB->CPUID, SCB->ICSR, NVIC_CTR->data, (u32)NVIC_CTR, NVIC_CTR_BASE); 


  /* Clear enable */
  NVIC->ICER[((uint32_t)(ComBlk_IRQn) >> 5)] = (1 << ((uint32_t)(ComBlk_IRQn) & 0x1F));
  COMBLK->INT_ENABLE = 0u;
  NVIC->ICPR[((uint32_t)(ComBlk_IRQn) >> 5)] = (1 << ((uint32_t)(ComBlk_IRQn) & 0x1F)); /* Clear pending interrupt */
  
  g_comblk_cmd_opcode = 0u;
  g_comblk_p_cmd = 0u;
  g_comblk_cmd_size = 0u;
  g_comblk_p_data = 0u;
  g_comblk_data_size = 0u;
  g_comblk_p_response = 0u;
  g_comblk_response_size = 0u;
  g_comblk_response_idx = 0u;

  g_comblk_state = COMBLK_IDLE;

  
  COMBLK->CONTROL |= CR_ENABLE_MASK;
  COMBLK->CONTROL &= ~CR_LOOPBACK_MASK;
  
  COMBLK->INT_ENABLE &= ~TXTOKAY_MASK;
  COMBLK->INT_ENABLE |= RCVOKAY_MASK;

  /* Set enable register */
  NVIC->ISER[((uint32_t)(ComBlk_IRQn) >> 5)] = (1 << ((uint32_t)(ComBlk_IRQn) & 0x1F)); /* enable interrupt */

  printf("NVIC : ICER = 0x%x (%p), ICPR = 0x%x (%p), ISER = 0x%x (%p), ISPR = 0x%x (%p) ICSR = 0x%x, CCR=0x%x\n",
	 NVIC->ICER[((uint32_t)(ComBlk_IRQn) >> 5)], &(NVIC->ICER[((uint32_t)(ComBlk_IRQn) >> 5)]),
	 NVIC->ICPR[((uint32_t)(ComBlk_IRQn) >> 5)], &(NVIC->ICPR[((uint32_t)(ComBlk_IRQn) >> 5)]),
	 NVIC->ISER[((uint32_t)(ComBlk_IRQn) >> 5)], &(NVIC->ISER[((uint32_t)(ComBlk_IRQn) >> 5)]), 
	 NVIC->ISPR[((uint32_t)(ComBlk_IRQn) >> 5)], &(NVIC->ISPR[((uint32_t)(ComBlk_IRQn) >> 5)]), 
	 SCB->ICSR, SCB->CCR);

}

static void exec_send_cmd_opcode(uint8_t opcode)
{
  volatile u32 tx_okay;
  
  COMBLK->CONTROL &= ~CR_SIZETX_MASK;
  
  /* Wait for space to become available in Tx FIFO. */
  do {
    tx_okay = COMBLK->STATUS & TXTOKAY_MASK;
  } while(0u == tx_okay);
  
  COMBLK->FRAME_START8 = opcode;
  
}


static u32 exec_read_page_from_flash(u8 * g_buffer, u32 size){

  u32 readout_size = size;

  if(g_src_image_target_address+readout_size > SPI_DATA_ADDR + SPI_FILE_SIZE){
    readout_size = SPI_DATA_ADDR + SPI_FILE_SIZE-g_src_image_target_address;
  }                                                                                                                               

  if(g_src_image_target_address>=SPI_DATA_ADDR + SPI_FILE_SIZE){
    return 0;
  }

  spi_flash->read(spi_flash, g_src_image_target_address, readout_size, g_buffer);
  g_src_image_target_address += readout_size;
  return readout_size;

}


static u32 exec_comblk_read_page_handler(u8 const ** pp_next_page){
  u32 length;
  

  length = exec_read_page_from_flash(g_flash_rd_buf, BUFFER_A_SIZE);

  *pp_next_page = g_flash_rd_buf;
  
  if(debug_uart==1){
    printf("read_page_handler(%d): facc1=0x%x, facc2=0x%x, PLL=0x%x, 0x%x, %d, 0x%lx (0x%lx)\n", 
	   g_mode,  M2S_SYSREG->mssddr_facc1_cr, M2S_SYSREG->mssddr_facc2_cr,
	   M2S_SYSREG->mssddr_pll_status , 
	   g_flash_rd_buf[0], length, g_src_image_target_address-SPI_DATA_ADDR, 
	   SPI_FILE_SIZE);
  }
  return length;
}


static u32 exec_fill_tx_fifo(const u8 * p_cmd, u32 cmd_size){
  volatile u32 tx_okay;
  u32 size_sent;

  /* Set transmit FIFO to transfer bytes. */
  COMBLK->CONTROL &= ~CR_SIZETX_MASK;
  size_sent = 0u;
  tx_okay = COMBLK->STATUS & TXTOKAY_MASK;
  while((tx_okay != 0u) && (size_sent < cmd_size))
    {
      COMBLK->DATA8 = p_cmd[size_sent];
      ++size_sent;
      tx_okay = COMBLK->STATUS & TXTOKAY_MASK;
    }
    
  return size_sent;
}


static void exec_handle_tx_okay_irq(void){

  switch(g_comblk_state)
    {
      /*----------------------------------------------------------------------
       * The TX_OKAY interrupt should only be enabled for states COMBLK_TX_CMD   
       * and COMBLK_TX_DATA.
       */
    case COMBLK_TX_CMD:
      if(g_comblk_cmd_size > 0u)
	{
	  u32 size_sent;
	  size_sent = exec_fill_tx_fifo(g_comblk_p_cmd, g_comblk_cmd_size);
	  if(size_sent < g_comblk_cmd_size)
	    {
	      g_comblk_cmd_size = g_comblk_cmd_size - (uint16_t)size_sent;
	      g_comblk_p_cmd = &g_comblk_p_cmd[size_sent];
	    }
	  else
	    {
	      g_comblk_cmd_size = 0u;
	      if(g_comblk_data_size > 0u)
		{
		  g_comblk_state = COMBLK_TX_DATA;
		}
	      else
		{
		  g_comblk_state = COMBLK_WAIT_RESPONSE;
		}
	    }
	}
      else
	{
	  /*
	   * This is an invalid situation indicating a bug in the driver
	   * or corrupted memory.
	   */
	  //ASSERT(0);
	  exec_abort_current_cmd();
	}
      break;
            
    case COMBLK_TX_DATA:
      if(g_comblk_data_size > 0u)
	{
	  u32 size_sent;
	  size_sent = exec_fill_tx_fifo(g_comblk_p_data, g_comblk_data_size);
	  if(size_sent < g_comblk_data_size)
	    {
	      g_comblk_data_size = g_comblk_data_size - size_sent;
	      g_comblk_p_data = &g_comblk_p_data[size_sent];
	    }
	  else
	    {
	      COMBLK->INT_ENABLE &= ~TXTOKAY_MASK;
	      g_comblk_state = COMBLK_WAIT_RESPONSE;
	    }
	}
      else
	{
	  /*
	   * This is an invalid situation indicating a bug in the driver
	   * or corrupted memory.
	   */
	  //ASSERT(0);
	  exec_abort_current_cmd();
	}
      break;
           
    case COMBLK_TX_PAGED_DATA:
      /*
       * Read a page of data if required.
       */
      if(0u == g_comblk_data_size)
	{
	  if(g_src_image_target_address<SPI_DATA_ADDR + SPI_FILE_SIZE){
	    g_comblk_data_size = exec_comblk_read_page_handler(&g_comblk_p_data);
	    if(0u == g_comblk_data_size){
	      COMBLK->INT_ENABLE &= ~TXTOKAY_MASK;
	      g_comblk_state = COMBLK_WAIT_RESPONSE;
	    }
	  }else{
	    if(g_mode ==  MSS_SYS_PROG_VERIFY){
	      exec_complete_request(2u);
	    }else{
	      g_comblk_data_size = exec_comblk_read_page_handler(&g_comblk_p_data);
	      if(0u == g_comblk_data_size){
	    	COMBLK->INT_ENABLE &= ~TXTOKAY_MASK;
	    	g_comblk_state = COMBLK_WAIT_RESPONSE;
	      }
	    }
	  }
	}
                
      /*
       * Transmit the page data or move to COMBLK_WAIT_RESPONSE state if
       * no further page data could be obtained by the call to the page
       * handler above.
       */
      if(0u == g_comblk_data_size)
	{
	  COMBLK->INT_ENABLE &= ~TXTOKAY_MASK;
	  g_comblk_state = COMBLK_WAIT_RESPONSE;
	}
      else
	{
	  u32 size_sent;
	  size_sent = exec_fill_tx_fifo(g_comblk_p_data, g_comblk_data_size);
	  g_comblk_data_size = g_comblk_data_size - size_sent;
	  g_comblk_p_data = &g_comblk_p_data[size_sent];
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
      COMBLK->INT_ENABLE &= ~TXTOKAY_MASK;
      exec_complete_request(0u);
      g_comblk_state = COMBLK_IDLE;
      break;
    }
}


static void exec_handle_rx_okay_irq(void){

  u16 data16;
  u16 is_command;    
  u8 data8;
    
  data16 = (u16)COMBLK->DATA8;
  is_command = data16 & DATA8_COMMAND_MASK;
  data8 = (u8)data16;


  switch(g_comblk_state)
    {
      /*----------------------------------------------------------------------
       * MSS_COMBLK_init() enables the RCV_OKAY interrupt for the COMBLK_IDLE
       * state to receive the asynchronous power-on-reset from the system
       * controller.
       */
    case COMBLK_IDLE:
      if(is_command)
	{
	  if(data8 != POR_DIGEST_ERROR_OPCODE)
	    {
	      u8 rxed_opcode;
	      rxed_opcode = data8;
	      process_sys_ctrl_command(rxed_opcode);
	    }
	  else
	    {  
	      g_comblk_response_idx = 0;
	      g_comblk_p_response[g_comblk_response_idx] = data8;
	      g_comblk_response_idx++;
	      g_comblk_p_response[g_comblk_response_idx] = 0x00u;                
	      g_comblk_state = COMBLK_RX_RESPONSE;
	    }
	}
      break;


      /*----------------------------------------------------------------------
       * The RCV_OKAY interrupt should only be enabled for states
       * COMBLK_WAIT_RESPONSE and COMBLK_RX_RESPONSE. 
       */
    case COMBLK_WAIT_RESPONSE:
      if(is_command)
	{
	  u8 rxed_opcode;
	  rxed_opcode = (u8)data8;
	  if(rxed_opcode == g_comblk_cmd_opcode)
	    {
	      g_comblk_response_idx = 0u;
	      g_comblk_p_response[g_comblk_response_idx] = rxed_opcode;
	      ++g_comblk_response_idx;
	      g_comblk_state = COMBLK_RX_RESPONSE;
	    }
	  else
	    {
	      process_sys_ctrl_command(rxed_opcode);
	    }
	}
      break;
      
    case COMBLK_RX_RESPONSE:
      if(is_command)
	{
	  u8 rxed_opcode;
	  rxed_opcode = (u8)data8;
	  process_sys_ctrl_command(rxed_opcode);
	}
      else
	{
	  if(g_comblk_p_response[g_comblk_response_idx-1] == POR_DIGEST_ERROR_OPCODE)
	    {
	      g_comblk_p_response[g_comblk_response_idx] = data8;
	      process_sys_ctrl_command(g_comblk_p_response[g_comblk_response_idx-1]);
	      g_comblk_state = COMBLK_IDLE;
	    }
	  else
	    {
	      if(g_comblk_response_idx < g_comblk_response_size)
		{
		  u8 rxed_data;
		  
		  rxed_data = (u8)data8;
		  g_comblk_p_response[g_comblk_response_idx] = rxed_data;
		  ++g_comblk_response_idx;
		}
	      
	      if(g_comblk_response_idx == g_comblk_response_size)
		{
		  exec_complete_request(g_comblk_response_idx);
		  g_comblk_state = COMBLK_IDLE;
		}
	    }
	}
      break;
      
      /*----------------------------------------------------------------------
       * The RCV_OKAY interrupt should NOT be enabled for states
       * COMBLK_IDLE, COMBLK_TX_CMD and COMBLK_TX_DATA. 
       */
    case COMBLK_TX_PAGED_DATA:
      /* This is needed because when there is an error, we need to terminate loading the data */
      if(!is_command)
	{
	  g_comblk_p_response[1] = (u8)data8;
	  exec_complete_request(2u);
	  g_comblk_state = COMBLK_IDLE;
	}
      else
	{
	  uint8_t rxed_opcode;
	  rxed_opcode = data8;
	  process_sys_ctrl_command(rxed_opcode);
	}
      break;
      //case COMBLK_IDLE:
      /* Fall through */
    case COMBLK_TX_CMD:
      /* Fall through */
    case COMBLK_TX_DATA:
      /* Fall through */
      if(is_command)
	{
	  u8 rxed_opcode;
	  rxed_opcode = (u8)data8;
	  process_sys_ctrl_command(rxed_opcode);
	}
      break;
      
    default:
      exec_complete_request(0u);
      g_comblk_state = COMBLK_IDLE;
      break;
    }
  
}

static void exec_complete_request(u16 response_length){
  
  ////g_isp_completion_handler(p_response[1]);

  g_request_in_progress = 0;
  g_comblk_response_idx = response_length;

}


static void exec_abort_current_cmd(void){

  if(g_request_in_progress){

    uint32_t flush_in_progress;
        
    /*
     * Call completion handler just in case we are in a multi threaded system
     * to avoid a task lockup.
     */
    exec_complete_request(g_comblk_response_idx);
        
    /*
     * Flush the FIFOs
     */
    COMBLK->CONTROL |= CR_FLUSHOUT_MASK;
    do {
      flush_in_progress = COMBLK->CONTROL & CR_FLUSHOUT_MASK;
    } while(flush_in_progress);
  }
}

/*
static u16 wait_for_request_completion(void){

  while(g_request_in_progress){
    ;
  }

  return g_comblk_response_idx;

}
*/

void exec_ComBlk_IRQHandler(void){

  u8 status;
  u8 tx_okay;
  u8 rcv_okay;



  //delay(10000);  
  status = (u8)COMBLK->STATUS;
  
  /* Mask off interrupt that are not enabled.*/
  status &= COMBLK->INT_ENABLE;
  
  rcv_okay = status & RCVOKAY_MASK;
  
  //printf("COMBLK: ComBLK_IRQHandler %02x %02x %d \n", 
  // 	   COMBLK->STATUS, COMBLK->INT_ENABLE, status);
  
  /*
    printf("NVIC in IRQ Handler : ICER = 0x%x (%p), ICPR = 0x%x (%p), ISER = 0x%x (%p), ISPR = 0x%x (%p) ISCR = 0x%x, CCR=0x%x\n",
    NVIC->ICER[((uint32_t)(ComBlk_IRQn) >> 5)], &(NVIC->ICER[((uint32_t)(ComBlk_IRQn) >> 5)]),
    NVIC->ICPR[((uint32_t)(ComBlk_IRQn) >> 5)], &(NVIC->ICPR[((uint32_t)(ComBlk_IRQn) >> 5)]),
    NVIC->ISER[((uint32_t)(ComBlk_IRQn) >> 5)], &(NVIC->ISER[((uint32_t)(ComBlk_IRQn) >> 5)]), 
    NVIC->ISPR[((uint32_t)(ComBlk_IRQn) >> 5)], &(NVIC->ISPR[((uint32_t)(ComBlk_IRQn) >> 5)]), 
    SCB->ICSR, SCB->CCR);
    */
    
    
  if(rcv_okay)
    {
      //delay(10000);  
      exec_handle_rx_okay_irq();
    }
  
  tx_okay = status & TXTOKAY_MASK;
  if(tx_okay)
    {
      //delay(10000);  
      exec_handle_tx_okay_irq();
    }
  
}


static void exec_comblk_send_cmd_with_ptr(u8 cmd_opcode,
					  u32 cmd_params_ptr,
					  u8 * p_response,
					  u16 response_size){
  
  volatile u32 tx_okay;

  NVIC->ICER[((uint32_t)(ComBlk_IRQn) >> 5)] = (1 << ((uint32_t)(ComBlk_IRQn) & 0x1F));
  COMBLK->INT_ENABLE = 0u;
  NVIC->ICPR[((uint32_t)(ComBlk_IRQn) >> 5)] = (1 << ((uint32_t)(ComBlk_IRQn) & 0x1F)); /* Clear pending interrupt */

  /*--------------------------------------------------------------------------
   * Abort current command if any.
   */

  g_comblk_cmd_opcode = cmd_opcode;
  g_comblk_p_cmd = 0u;
  g_comblk_cmd_size = 0u;
  g_comblk_p_data = 0u;
  g_comblk_data_size = 0u;
  g_comblk_p_response = p_response;
  g_comblk_response_size = response_size;
  g_comblk_response_idx = 0u;

  exec_send_cmd_opcode(g_comblk_cmd_opcode); 
  
  COMBLK->CONTROL |= CR_SIZETX_MASK;
  
  /* Wait for space to become available in Tx FIFO. */
  do {
    tx_okay = COMBLK->STATUS & TXTOKAY_MASK;
  } while(0u == tx_okay);
  
  COMBLK->DATA32  = cmd_params_ptr;
  
  COMBLK->CONTROL &= ~CR_SIZETX_MASK;    
  g_comblk_state = COMBLK_WAIT_RESPONSE;


  COMBLK->INT_ENABLE |= RCVOKAY_MASK;
  NVIC->ISER[((uint32_t)(ComBlk_IRQn) >> 5)] = (1 << ((uint32_t)(ComBlk_IRQn) & 0x1F)); /* enable interrupt */

  //printf("IRQ active register = 0x%x (%p)\n", 
  //NVIC->IABR[((uint32_t)(ComBlk_IRQn) >> 5)], &(NVIC->IABR[((uint32_t)(ComBlk_IRQn) >> 5)]));

  //udelay(10000);
  
  delay(10000);
  
  //exec_ComBlk_IRQHandler();

}


static void exec_comblk_send_paged_cmd(u8 * p_cmd, 
				u16 cmd_size,
				u8 * p_response, 
				u16 response_size){
  
  u32 size_sent;
  u8 irq_enable = 0u;


  NVIC->ICER[((uint32_t)(ComBlk_IRQn) >> 5)] = (1 << ((uint32_t)(ComBlk_IRQn) & 0x1F));
  COMBLK->INT_ENABLE = 0u;
  NVIC->ICPR[((uint32_t)(ComBlk_IRQn) >> 5)] = (1 << ((uint32_t)(ComBlk_IRQn) & 0x1F)); /* Clear pending interrupt */

  /*--------------------------------------------------------------------------
   * Abort current command if any.
   */
  exec_abort_current_cmd();


  g_comblk_cmd_opcode = p_cmd[0];
  g_comblk_p_cmd = p_cmd;
  g_comblk_cmd_size = cmd_size;
  g_comblk_p_data = 0;
  g_comblk_data_size = 0u;
  g_comblk_p_response = p_response;
  g_comblk_response_size = response_size;
  g_comblk_response_idx = 0u;

  exec_send_cmd_opcode(g_comblk_cmd_opcode);
  size_sent = exec_fill_tx_fifo(&p_cmd[1], cmd_size - 1);
  ++size_sent;    /* Adjust for opcode byte sent. */
  if(size_sent < cmd_size){
    g_comblk_cmd_size = g_comblk_cmd_size - (u16)size_sent;
    g_comblk_p_cmd = &g_comblk_p_cmd[size_sent];

    g_comblk_state = COMBLK_TX_CMD;
    irq_enable = TXTOKAY_MASK | RCVOKAY_MASK;
  }else{
    g_comblk_cmd_size = 0u;
    g_comblk_state = COMBLK_TX_PAGED_DATA;
    irq_enable = TXTOKAY_MASK | RCVOKAY_MASK;
  }
  
  COMBLK->INT_ENABLE |= irq_enable;   
  NVIC->ISER[((uint32_t)(ComBlk_IRQn) >> 5)] = (1 << ((uint32_t)(ComBlk_IRQn) & 0x1F)); /* enable interrupt */

}

		      

void exec_comblk_get_serial_number_out(u8 *p_response){

  u8  p_serial_number[16]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
  u8  cmd_opcode = 0;
  u8  response[6];
  u8  status;
  u16 response_length = 6;
  u16 actual_response_length;
  u8  i=0;
  cmd_opcode = SERIAL_NUMBER_REQUEST_CMD;
  exec_comblk_send_cmd_with_ptr(cmd_opcode,
				(u32) p_serial_number,
				response,
				response_length);
  
  
  while(response_length!=g_comblk_response_idx){
    ;
  }


  actual_response_length = g_comblk_response_idx;

  if( (response_length == actual_response_length) && 
      (cmd_opcode == response[0])){
    status = response[1];
  }else{
    status = 200;
  }


  printf("COMBLK: MSS_SYS_get_serial_number result : status = %d, opcode = %d size=%d\n", status, response[0], g_comblk_response_idx);
  printf("COMBLK: Device serial number at 0x%x = %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x\n", 
	 (u32)p_serial_number, p_serial_number[0],p_serial_number[1],p_serial_number[2],p_serial_number[3],
	 p_serial_number[4],p_serial_number[5],p_serial_number[6],p_serial_number[7],
	 p_serial_number[8],p_serial_number[9],p_serial_number[10],p_serial_number[11],
	 p_serial_number[12],p_serial_number[13],p_serial_number[14],p_serial_number[15]);

  
  for(i=0;i<16;i++){
    p_response[i] = p_serial_number[i];
  }
}

static void exec_comblk_get_serial_number(void){
  
  u8  p_serial_number[16]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
  u8  cmd_opcode = 0;
  u8  response[6];
  u8  status;
  u16 response_length = 6;
  u16 actual_response_length;

  cmd_opcode = SERIAL_NUMBER_REQUEST_CMD;
  exec_comblk_send_cmd_with_ptr(cmd_opcode,
				(u32) p_serial_number,
				response,
				response_length);
  
  
  while(response_length!=g_comblk_response_idx){
    ;
  }


  actual_response_length = g_comblk_response_idx;

  if( (response_length == actual_response_length) && 
      (cmd_opcode == response[0])){
    status = response[1];
  }else{
    status = 200;
  }


  printf("COMBLK: MSS_SYS_get_serial_number result : status = %d, opcode = %d size=%d\n", status, response[0], g_comblk_response_idx);
  printf("COMBLK: Device serial number at 0x%x = %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x\n", 
	 (u32)p_serial_number, p_serial_number[0],p_serial_number[1],p_serial_number[2],p_serial_number[3],
	 p_serial_number[4],p_serial_number[5],p_serial_number[6],p_serial_number[7],
	 p_serial_number[8],p_serial_number[9],p_serial_number[10],p_serial_number[11],
	 p_serial_number[12],p_serial_number[13],p_serial_number[14],p_serial_number[15]);

}

static void exec_comblk_get_design_version(void)
{
  
  u8 p_design_version[2]={0,0};
  u8 cmd_opcode = 0;
  u8 response[6];
  u8 status;
  u16 response_length = 6;
  u16 actual_response_length;

  cmd_opcode = DESIGN_VERSION_REQUEST_CMD;
  g_last_response_length = 0;


  exec_comblk_send_cmd_with_ptr(cmd_opcode,
				(u32) p_design_version,
				response,
				response_length);


  
  while(response_length!=g_comblk_response_idx){
    ;
  }

  actual_response_length = g_comblk_response_idx;
  
  if( (response_length == actual_response_length) && 
      (cmd_opcode == response[0]))
    {
      status = response[1];
    }
  else
    {
      status = 200;
    }

  printf("COMBLK: MSS_SYS_get_design_version result : status = %d, opcode = %d, size = %d\n", status, response[0], g_comblk_response_idx);
  printf("COMBLK: Fabric Design Version at 0x%x = %02x %02x\n", 
	 (u32) p_design_version, p_design_version[0],p_design_version[1]);

  if(status==0){
    design_version[0] = p_design_version[0];
  }else{
    design_version[0] = 0xff;
  }
  
}

static void exec_clock_switch(void){
  
  u32 running_on_standby_clock;
  
  volatile u32 timeout;
  //volatile u32 timeout2;
  u32 clk1;
  u32 clk2;
  u32 clock_divisor;

  //delay(10000000);
  if(debug_uart==1){
    printf("change the clock..\n");
  }

  //M2S_SYSREG->rtc_wakeup_cr = 0x2 | 0x4;


  ////// disable FPGA/FIC0/FIC1
  //M2S_SYSREG->soft_reset_cr &= ~(((u32) 0x1 << 26));   
  //M2S_SYSREG->soft_reset_cr &= ~(((u32) 0x1 << 25));   
  //  M2S_SYSREG->soft_reset_cr &= ~(((u32) 0x1 << 16));   
  M2S_SYSREG->soft_reset_cr &= ~(((u32) 0x1 << 18));   
  M2S_SYSREG->soft_reset_cr &= ~(((u32) 0x1 << 19));   


  //udelay(10000);
  delay(10000000);
  M2S_SYSREG->envm_cr = (g_initial_envm_cr & ~NVM_FREQRNG_MASK) | NVM_FREQRNG_MAX;

  //delay(DELAY_MORE_THAN_10US);
  delay(10000000);
  M2S_SYSREG->mssddr_facc2_cr = M2S_SYSREG->mssddr_facc2_cr & ((u32)(FACC_STANDBY_SEL << FACC_STANDBY_SHIFT) & FACC_STANDBY_SEL_MASK);  
  delay(10000000);
  M2S_SYSREG->mssddr_facc2_cr = M2S_SYSREG->mssddr_facc2_cr | ((u32)(MSS_25_50MHZ_EN << MSS_25_50MHZ_EN_SHIFT) & MSS_25_50MHZ_EN_MASK);  
  delay(10000000);
  M2S_SYSREG->mssddr_facc2_cr = M2S_SYSREG->mssddr_facc2_cr | ((u32)(MSS_1MHZ_EN << MSS_1MHZ_EN_SHIFT) & MSS_1MHZ_EN_MASK);        


  if(M2S_SYSREG->mssddr_facc2_cr !=0x600){
    delay(10000000);
    M2S_SYSREG->mssddr_facc2_cr = M2S_SYSREG->mssddr_facc2_cr & ((u32)(FACC_STANDBY_SEL << FACC_STANDBY_SHIFT) & FACC_STANDBY_SEL_MASK);
    delay(10000000);
    M2S_SYSREG->mssddr_facc2_cr = M2S_SYSREG->mssddr_facc2_cr | ((u32)(MSS_25_50MHZ_EN << MSS_25_50MHZ_EN_SHIFT) & MSS_25_50MHZ_EN_MASK);
    delay(10000000);
    M2S_SYSREG->mssddr_facc2_cr = M2S_SYSREG->mssddr_facc2_cr | ((u32)(MSS_1MHZ_EN << MSS_1MHZ_EN_SHIFT) & MSS_1MHZ_EN_MASK);
  }


  //  M2S_SYSREG->mssddr_facc2_cr |= 0x2;

  delay(10000000);

  //M2S_SYSREG->mssddr_facc2_cr = 0x1e01; 

  //M2S_SYSREG->mssddr_facc2_cr & ((u32)(FACC_STANDBY_SEL << FACC_STANDBY_SHIFT) & FACC_STANDBY_SEL_MASK); 
  
  //delay(DELAY_MORE_THAN_10US);

  /// enable standby clock 
  /// maybe stack here??????

  //timeout2 = DELAY_MORE_THAN_10US;
  //do{
  //--timeout2;
  //}
  //while (timeout2 != 0U);

  //// this is Torsten's test
  /*
  M2S_SYSREG->mssddr_facc1_cr |= (1<<12);

  timeout = DELAY_MORE_THAN_10US;
  do
    {
      running_on_standby_clock = M2S_SYSREG->mssddr_facc1_cr & 0x00001000;
      --timeout;
    }
  while ((running_on_standby_clock == 0U) && (timeout != 0U));
  wait_for_clock_switch = 0;  //now clock is standby
  M2S_SYSREG->mssddr_facc1_cr &= ~apb_divisors_mask;  
  //// end of Torsten's test       
  */


  M2S_SYSREG->mssddr_facc1_cr |= (1<<12);

  timeout = DELAY_MORE_THAN_10US;
  do
    {
      running_on_standby_clock = M2S_SYSREG->mssddr_facc1_cr & 0x00001000;
      --timeout;
    }
  while ((running_on_standby_clock == 0U) && (timeout != 0U));


  /// this setting is necessary. otherwise programming fails... why?
  delay(10000000);
  M2S_SYSREG->mssddr_facc1_cr &= ~apb_divisors_mask;  
  delay(10000000);

  if(M2S_SYSREG->mssddr_facc1_cr!=0xa403101){
    delay(10000000);
    M2S_SYSREG->mssddr_facc1_cr &= ~apb_divisors_mask;
    delay(10000000);
  }


  if(debug_uart==1){
    exec_clock_mss_learn(50000000, 1);
    clock_divisor = exec_calc_divisor(serial_ports); 
    NS16550_init(serial_ports, clock_divisor);
    
    delay(1000000);
    clock_update(CLOCK_SYSREF, clock_base[CLOCK_SYSREF]);
    clock_update(CLOCK_SYSTICK, clock_base[CLOCK_SYSTICK]);
    clock_update(CLOCK_DDR, clock_base[CLOCK_DDR]);
    clock_update(CLOCK_PCLK0, clock_base[CLOCK_PCLK0]);
    clock_update(CLOCK_PCLK1, clock_base[CLOCK_PCLK1]);
    clock_update(CLOCK_FPGA, clock_base[CLOCK_FPGA]);
    
    clk1 = clock_base[CLOCK_PCLK0];
    clk2 = clock_get(CLOCK_PCLK0);
    
    delay(100000);
    NS16550_init(serial_ports, clock_divisor);
    /*
      printf(" facc1=0x%x, facc2=0x%x, clk0=%d, clk1=%d (div=%d). SPI clock=%d timeout=%d timeout2=%d\n", 
      M2S_SYSREG->mssddr_facc1_cr, 
      M2S_SYSREG->mssddr_facc2_cr, 
      clk1, clk2, clock_divisor, SPI_CLOCK_FF,
      timeout, timeout2);
    */
  }


  delay(10000);

}

static void exec_clock_revert(void){

  volatile u32 timeout;
  u32 running_on_standby_clock;
  u32 clock_divisor;
  
  //delay(DELAY_MORE_THAN_10US);


  //  M2S_SYSREG->mssddr_facc2_cr =  g_initial_mssddr_facc2_cr;    
  /*
  u32 pll_locked;    
  // Wait for fabric PLL to lock. //
  do {
    pll_locked = M2S_SYSREG->mssddr_pll_status & 0x1;
  } while(!pll_locked);
    
  // Negate MPLL bypass. 
  M2S_SYSREG->mssddr_pll_status_high_cr &= ~0x1;
    
  // Wait for MPLL to lock. 
  do {
    pll_locked = M2S_SYSREG->mssddr_pll_status & 0x2;
  } while(!pll_locked);

  */


  /// this is for Torsten's test 
  /*
  M2S_SYSREG->mssddr_facc1_cr &= ~(1<<12);
  
  timeout = DELAY_MORE_THAN_10US;
  do
    {
      running_on_standby_clock = M2S_SYSREG->mssddr_facc1_cr & 0x00001000;
      --timeout;
    }
  while ((running_on_standby_clock != 0U) && (timeout != 0U));
  
  
  M2S_SYSREG->mssddr_facc1_cr = g_initial_mssddr_facc1_cr;
  */
  //// end of Torsten's test


  //delay(10000000);
  ////clock is reverted 
  //M2S_SYSREG->envm_cr = g_initial_envm_cr ;
  
  
  //delay(10000);
  //M2S_SYSREG->mssddr_facc2_cr =  g_initial_mssddr_facc2_cr;    
  

  ////////////
  //// wait for the PLL clock is locked 
  ///////////
  /*
  u32 pll_locked;    
  // Wait for fabric PLL to lock. //
  do {
    pll_locked = M2S_SYSREG->mssddr_pll_status & 0x1;
  } while(!pll_locked);
    
  // Negate MPLL bypass. 
  M2S_SYSREG->mssddr_pll_status_high_cr &= ~0x1;
    
  // Wait for MPLL to lock. 
  do {
    pll_locked = M2S_SYSREG->mssddr_pll_status & 0x2;
  } while(!pll_locked);
  */

  
  
  exec_clock_mss_learn(160000000, 0);
  if(debug_uart==1){
    clock_divisor = exec_calc_divisor(serial_ports); 
    NS16550_init(serial_ports, clock_divisor);
    
    clock_update(CLOCK_SYSREF, clock_base[CLOCK_SYSREF]);
    clock_update(CLOCK_SYSTICK, clock_base[CLOCK_SYSTICK]);
    clock_update(CLOCK_DDR, clock_base[CLOCK_DDR]);
    clock_update(CLOCK_PCLK0, clock_base[CLOCK_PCLK0]);
    clock_update(CLOCK_PCLK1, clock_base[CLOCK_PCLK1]);
    clock_update(CLOCK_FPGA, clock_base[CLOCK_FPGA]);    
    
    delay(10000);
    NS16550_init(serial_ports, clock_divisor);
    printf(" facc1 = 0x%x, facc2=0x%x PLL=0x%x\n",
	   M2S_SYSREG->mssddr_facc1_cr, 
	   M2S_SYSREG->mssddr_facc2_cr, 
	   M2S_SYSREG->mssddr_pll_status);
  }
  delay(10000);

}

static void exec_start_isp(uint8_t mode){

  u8 isp_prog_request[2];

  /* 
   * Set the eNVM's frequency range to its maximum. This is required to ensure 
   * successful eNVM programming on all devices. 
   */
  //  M2S_SYSREG->envm_cr = (g_initial_envm_cr & ~NVM_FREQRNG_MASK) | NVM_FREQRNG_MAX;


  g_request_in_progress = 1;

  g_mode = mode;

  isp_prog_request[0] = ISP_PROGRAMMING_REQUEST_CMD;
  isp_prog_request[1] = mode;

  exec_comblk_send_paged_cmd(isp_prog_request,                 /* p_cmd */
  			     sizeof(isp_prog_request),         /* cmd_size */
  			     g_isp_response,                   /* p_response */
  			     ISP_PROG_SERV_RESP_LENGTH        /* response_size */
  			     );

  while(1){
    if(g_comblk_response_idx==2){
      break;
    }
  }

  //delay(DELAY_MORE_THAN_10US);

  /*
  printf("MSS_SYS_start_isp = %d (response[1]) %d (response length) %d (opcode) \n", 
	 g_isp_response[1], g_comblk_response_idx, g_isp_response[0]);
  */

  if(mode == MSS_SYS_PROG_AUTHENTICATE){
    isp_programming_results[0] = g_isp_response[1];
  }else if(mode == MSS_SYS_PROG_PROGRAM){
    isp_programming_results[1] = g_isp_response[1];
  }else if(mode == MSS_SYS_PROG_VERIFY){
    isp_programming_results[2] = g_isp_response[1];
  }


  if(g_mode == MSS_SYS_PROG_VERIFY){
    delay(1000000);
    /*
    ////clock is reverted 
    M2S_SYSREG->envm_cr = g_initial_envm_cr ;
    M2S_SYSREG->mssddr_facc1_cr = g_initial_mssddr_facc1_cr;
    
    delay(1000000);
    M2S_SYSREG->mssddr_facc2_cr =  g_initial_mssddr_facc2_cr;    

    volatile u32 timeout;
    u32 running_on_standby_clock;
    u32 clock_divisor;

    timeout = DELAY_MORE_THAN_10US;
    do
      {
	running_on_standby_clock = M2S_SYSREG->mssddr_facc1_cr & 0x00001000;
	--timeout;
      }
    while ((running_on_standby_clock != 0U) && (timeout != 0U));
    

    M2S_SYSREG->mssddr_facc1_cr = g_initial_mssddr_facc1_cr;


    exec_clock_mss_learn(160000000, 0);
    clock_divisor = exec_calc_divisor(serial_ports); 
    NS16550_init(serial_ports, clock_divisor);
    
    clock_update(CLOCK_SYSREF, clock_base[CLOCK_SYSREF]);
    clock_update(CLOCK_SYSTICK, clock_base[CLOCK_SYSTICK]);
    clock_update(CLOCK_DDR, clock_base[CLOCK_DDR]);
    clock_update(CLOCK_PCLK0, clock_base[CLOCK_PCLK0]);
    clock_update(CLOCK_PCLK1, clock_base[CLOCK_PCLK1]);
    clock_update(CLOCK_FPGA, clock_base[CLOCK_FPGA]);    
    
    udelay(1000);
    printf(" facc1 = 0x%x, facc2=0x%x\n",
	   M2S_SYSREG->mssddr_facc1_cr, 
	   M2S_SYSREG->mssddr_facc2_cr);
    */
  }
}


void delay(uint32_t val){

  volatile uint32_t timeout ;
  timeout = val;
  
  do{
    --timeout;
  }while (timeout != 0);

}

/*
void exec_ECC_Error_IRQHandler(void){


  printf("ECC_Error_IRQHandler called...\n");

  printf(" dump the registers \n");
  printf(" M2S_SYSREG->edac_cr = 0x%x\n", M2S_SYSREG->edac_cr);
  printf(" M2S_SYSREG->edac_irq_enable_cr = 0x%x\n", M2S_SYSREG->edac_irq_enable_cr);
  printf(" M2S_SYSREG->ESRAM0_EDAC_CNT = 0x%x\n", M2S_SYSREG->ESRAM0_EDAC_CNT);
  printf(" M2S_SYSREG->ESRAM1_EDAC_CNT = 0x%x\n", M2S_SYSREG->ESRAM1_EDAC_CNT);
  printf(" M2S_SYSREG->ESRAM0_EDAC_ADR = 0x%x\n", M2S_SYSREG->ESRAM0_EDAC_ADR);
  printf(" M2S_SYSREG->ESRAM1_EDAC_ADR = 0x%x\n", M2S_SYSREG->ESRAM1_EDAC_ADR);
  printf(" M2S_SYSREG->EDAC_SR = 0x%x\n", M2S_SYSREG->EDAC_SR);


  NVIC->ICPR[((uint32_t)(ECC_Error_IRQn) >> 5)] = (1 << ((uint32_t)(ECC_Error_IRQn) & 0x1F)); // Clear pending interrupt //
  NVIC->ICER[((uint32_t)(ECC_Error_IRQn) >> 5)] = (1 << ((uint32_t)(ECC_Error_IRQn) & 0x1F));



  //M2S_SYSREG->edac_irq_enable_cr &= ~(0xF);
  //printf(" disable IRQ in EDAC registers : M2S_SYSREG->edac_irq_enable_cr = 0x%x\n", M2S_SYSREG->edac_irq_enable_cr);

}
*/
static int do_isp_prog(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{

  char isp_result[20];
  const char *cmd;
  const char *s ; 
  //int inc=0;
  //int ival=0;
  /* need at least two arguments */
  if (argc < 2)
    goto usage;
  
  cmd = argv[1];

  chip_select = 0;
  if(strcmp(argv[2], "0") == 0){
    chip_select = 0;
  }else if(strcmp(argv[2], "1") == 0){
    chip_select = 1;
  }else{
    chip_select = 0;
  }

  printf(" *** SPI Flash chip_select = %d\n", chip_select);

  asm volatile ("cpsie i");
  

  g_initial_envm_cr = M2S_SYSREG->envm_cr;
  g_initial_mssddr_facc1_cr = M2S_SYSREG->mssddr_facc1_cr;
  g_initial_mssddr_facc2_cr = M2S_SYSREG->mssddr_facc2_cr;
  
  printf("SYSREG_ENVM_CR = 0x%x\n", g_initial_envm_cr);
  printf("SYSREG_MSSDDR_FACC1_CR = 0x%x\n", g_initial_mssddr_facc1_cr);
  printf("SYSREG_MSSDDR_FACC2_CR = 0x%x\n", g_initial_mssddr_facc2_cr);  
  delay(10000);


  isp_programming_results[0] = 0xFF;
  isp_programming_results[1] = 0xFF;
  isp_programming_results[2] = 0xFF;


  ///I always to change bootcmd --> flashboot

  if((s = getenv("ISP_PROG_SIZE"))!=NULL){
    SPI_FILE_SIZE = simple_strtoul (s, NULL, 10);
  }else{
    printf(" No ISP_PROG_SIZE is defined...\n");
    SPI_FILE_SIZE = 1223504;
    //goto done;
  }

  if((s = getenv("ISP_PROG_ADDR"))!=NULL){
    SPI_DATA_ADDR = simple_strtoul (s, NULL, 10);
  }else{
    printf(" No ISP_PROG_ADDR is defined...\n");
    SPI_DATA_ADDR = 0x200000;
    //goto done;
  }

  debug_uart=1;
  if((s = getenv("ISP_DEBUG"))!=NULL){
    debug_uart = simple_strtoul (s, NULL, 10);
    if(debug_uart==0){
      printf("ISP Programming Silent mode..\n");
    }else{
      printf("ISP Programming Debug mode..\n");
      debug_uart = 1;
    }
  }else{
    printf(" No ISP_DEBUG variable... ISP Debug mode ..\n");
    debug_uart = 1;
  }

  //////
  
  if(strcmp(cmd, "c") == 0){
    exec_comblk_init();
    //exec_clock_mss_learn(CONFIG_SYS_M2S_SYSREF);
    exec_comblk_get_serial_number();
    exec_comblk_get_design_version();
    //exec_spi_flash_probe(CONFIG_SF_DEFAULT_SPEED);
    //exec_spi_flash_read();
    //spi_flash_read(spi_flash, SPI_DATA_ADDR + VERSION_ID_ADDR, 1, &design_version[1]);
    //printf("Design version in the Fabric = 0x%x and SPI Flash = 0x%x\n",
    //design_version[0], design_version[1]);
    goto done;
  }else if(strcmp(cmd, "s") == 0){
    exec_comblk_init();
    exec_clock_switch();
    goto done;
  }else if(strcmp(cmd, "b") == 0){ //clock change 
    
    exec_comblk_init();
    exec_spi_flash_probe(CONFIG_SF_DEFAULT_SPEED);
    exec_spi_flash_read(SPI_DATA_ADDR);
    printf(" ######## \n");
    exec_clock_switch();
    printf(" ######## \n");
    exec_comblk_init();
    //exec_spi_flash_probe(SPI_CLOCK_FF);
    exec_spi_flash_read(SPI_DATA_ADDR);
    printf(" ######## \n");

    ////clock is reverted 
    exec_clock_revert();    
    goto done;
  }else if(strcmp(cmd, "t") == 0){
    //exec_comblk_init();
    //exec_clock_switch();
    exec_spi_flash_probe(SPI_CLOCK_FF);
    exec_spi_flash_read(SPI_DATA_ADDR);
    exec_spi_flash_read(SPI_DATA_ADDR+64);
    exec_spi_flash_read(SPI_DATA_ADDR+64*2);
    exec_spi_flash_read(SPI_DATA_ADDR+64*3);
    exec_spi_flash_read(SPI_DATA_ADDR+64*4);
    exec_spi_flash_read(SPI_DATA_ADDR+64*5);
    exec_spi_flash_read(SPI_DATA_ADDR+64*6);
    goto done;
  } else if(strcmp(cmd, "a") == 0){

    setenv("bootcmd","run flashboot");
    saveenv();

    exec_comblk_init();
    //exec_clock_mss_learn(CONFIG_SYS_M2S_SYSREF);
    exec_spi_flash_probe(CONFIG_SF_DEFAULT_SPEED);
    printf("Authentication... Please wait (5 min.)...\n");
    //exec_spi_flash_read();
    g_src_image_target_address = SPI_DATA_ADDR;
    exec_start_isp(MSS_SYS_PROG_AUTHENTICATE);    
      sprintf(isp_result,"A:0x%02x, P:0x%02x, V:0x%02x", 
	    isp_programming_results[0],
	    isp_programming_results[1],
	    isp_programming_results[2]);
    setenv("ISP_PROG_RESULT", isp_result);
    saveenv();
  
    goto done;
  }else if(strcmp(cmd, "p") == 0){
    exec_comblk_init();
    //exec_clock_mss_learn(CONFIG_SYS_M2S_SYSREF);
    exec_spi_flash_probe(CONFIG_SF_DEFAULT_SPEED);
    //exec_spi_flash_read();
    //udelay(10000);
    exec_clock_switch();
    //exec_spi_flash_probe(SPI_CLOCK_FF);
    //exec_spi_flash_read();
    //udelay(10000);
    delay(10000);
    g_src_image_target_address = SPI_DATA_ADDR;
    exec_start_isp(MSS_SYS_PROG_PROGRAM);
    goto done;
  }else if(strcmp(cmd, "v") == 0){

    setenv("bootcmd","run flashboot");
    saveenv();

    exec_comblk_init();
    exec_spi_flash_probe(CONFIG_SF_DEFAULT_SPEED);

    printf("Verification... Please wait (3 min.)...\n");
    //exec_clock_mss_learn(CONFIG_SYS_M2S_SYSREF);
    exec_clock_switch();

    //exec_spi_flash_probe(SPI_CLOCK_FF);
    //exec_spi_flash_read();
    //udelay(10000);
    delay(10000);
    g_src_image_target_address = SPI_DATA_ADDR;
    exec_start_isp(MSS_SYS_PROG_VERIFY);

    //exec_clock_revert();
    delay(DELAY_MORE_THAN_10US);
    /*
    sprintf(isp_result,"A:0x%02x, P:0x%02x, V:0x%02x", 
	    isp_programming_results[0],
	    isp_programming_results[1],
	    isp_programming_results[2]);
    printf("Results : %s\n", isp_result);
    setenv("ISP_PROG_RESULT", isp_result);
    setenv("bootcmd","run flashboot");
    saveenv();   
    */

    goto done;
  }else if(strcmp(cmd, "apv") == 0){
    setenv("bootcmd","run flashboot");
    saveenv();

    exec_comblk_init();    
    exec_spi_flash_probe(CONFIG_SF_DEFAULT_SPEED); 
    printf("Programmig and Verifying... Please wait (5 min.)...\n");
    exec_clock_switch();
    //exec_spi_flash_probe(SPI_CLOCK_FF); //this is necessary???

    //udelay(10000);
    //exec_spi_flash_read(SPI_DATA_ADDR);
    delay(10000);
    g_src_image_target_address = SPI_DATA_ADDR;
    exec_start_isp(MSS_SYS_PROG_PROGRAM);

    if(g_isp_response[1]!=0){
      delay(DELAY_MORE_THAN_10US);
      /*
      exec_clock_revert();
      delay(DELAY_MORE_THAN_10US);
      sprintf(isp_result,"A:0x%02x, P:0x%02x, V:0x%02x", 
	      isp_programming_results[0],
	      isp_programming_results[1],
	      isp_programming_results[2]);
      setenv("ISP_PROG_RESULT", isp_result);
      printf("Results : %s\n", isp_result);
      setenv("bootcmd","run flashboot");
      saveenv();
      */
      goto done;
    }

    g_src_image_target_address = SPI_DATA_ADDR;
    exec_start_isp(MSS_SYS_PROG_VERIFY);
    
    if(g_isp_response[1]!=0){
      delay(DELAY_MORE_THAN_10US);
      /*
      exec_clock_revert();
      delay(DELAY_MORE_THAN_10US);
      sprintf(isp_result,"A:0x%02x, P:0x%02x, V:0x%02x", 
	      isp_programming_results[0],
	      isp_programming_results[1],
	      isp_programming_results[2]);
      setenv("ISP_PROG_RESULT", isp_result);
      setenv("bootcmd","run flashboot");
      saveenv();      goto done;
      */
      goto done;
    }


    delay(DELAY_MORE_THAN_10US);
    //exec_clock_revert();
    delay(DELAY_MORE_THAN_10US);
    //do_reset (NULL, 0, 0, NULL);    

    /*
    sprintf(isp_result,"A:0x%02x, P:0x%02x, V:0x%02x", 
	    isp_programming_results[0],
	    isp_programming_results[1],
	    isp_programming_results[2]);
    setenv("ISP_PROG_RESULT", isp_result);
    setenv("bootcmd","run flashboot");
    saveenv();
    */
    goto done;
  }else if(strcmp(cmd, "r") == 0){
    exec_clock_revert();
    goto done;
  }else if(strcmp(cmd, "boot") == 0){
    goto boot;
  }else if(strcmp(cmd, "ddr") == 0){
    M2S_SYSREG->mddr_cr &= ~(1 << 0);    
    board_init();
    dram_init();
    //saveenv();
    goto done;
  }else if(strcmp(cmd, "ddr_no_secded") == 0){
    //    dram_init_no_secded();
    goto done;
  }else if(strcmp(cmd, "ddr_secded") == 0){
    dram_init();
    goto done;
  }

  /*
  else if(strcmp(cmd, "cltest") == 0){
    //exec_comblk_init();
    //exec_spi_flash_probe(CONFIG_SF_DEFAULT_SPEED);
    //exec_spi_flash_read(SPI_DATA_ADDR);
    u32 dump[20];
    M2S_SYSREG->soft_reset_cr |= (((u32) 0x01 << 9)); //SYSREG_SPI0_SOFTRESET_MASK;

    for(inc=0;inc<100;inc++){
      printf(" \n ######## clock switch/revert test cycle=%d\n", inc);
      printf(" write to memory \n");
      for(ival=0;ival<20;ival++){
	dump[ival] = ival;
      }
      printf(" dump the moemry before clock switch\n");
      for(ival=0;ival<20;ival++){
	printf(" 0x%x, ", dump[ival]);
      }
      printf(" \n re-write the memory in different clock \n");

      //M2S_SYSREG->soft_reset_cr |= (((u32) 0x01 << 7)); //SYSREG_SPI0_SOFTRESET_MASK;
      exec_clock_switch();
      //exec_comblk_init();
      //exec_spi_flash_probe(SPI_CLOCK_FF);
      //exec_spi_flash_read(SPI_DATA_ADDR);
      ////clock is reverted 
      for(ival=0;ival<20;ival++){
	dump[ival] = ival;
      }
      exec_clock_revert();          
      //M2S_SYSREG->soft_reset_cr &= ~(((u32) 0x01 << 7)); //SYSREG_SPI0_SOFTRESET_MASK;
      //exec_clock_mss_learn(160000000, 0);
      //u32 clock_divisor = exec_calc_divisor(serial_ports); 
      //NS16550_init(serial_ports, clock_divisor);

      printf(" dump the moemry after clock switch\n");
      for(ival=0;ival<20;ival++){
	printf(" 0x%x, ", dump[ival]);
      }
      
    }
    goto done;
  }
  */

  else{
    goto usage;
  }

 boot:
  asm volatile ("cpsid i");
  s = getenv ("bootcmd");  
  run_command (s, 0);           
  return 1;

 done:
  asm volatile ("cpsid i");
  return 1;
  
 usage:
  asm volatile ("cpsid i");
  cmd_usage(cmdtp);
  return 1;
}

U_BOOT_CMD(
	   isp,	3,	1,	do_isp_prog,
	   "ISP Programming",
	   "isp [c/a/p/v/apv]- c:check comblk, a:authentication\n"
);