/*******************************************************************************
 * (c) Copyright 2012 Microsemi SoC Products Group.  All rights reserved.
 *
 * SmartFusion2 COMBLK access functions.
 *
 * SVN $Revision: 5615 $
 * SVN $Date: 2013-04-05 14:48:10 +0100 (Fri, 05 Apr 2013) $
 */
 
#include <linux/types.h>
#include <linux/init.h>
#include <linux/io.h>
#include <linux/module.h>
#include <linux/clk.h>
#include <linux/delay.h>
#include <mach/platform.h>
#include <mach/m2s.h>
#include <asm/hardware/nvic.h>
#include <asm/mach-types.h>
#include <asm/hardware/nvic.h>
#include <mach/regmap.h>
#include <mach/comblk.h>

const uint32_t aaaa=0xdeadbeaf;

////serdes0 and serdes1

typedef struct
{
    uint32_t * p_reg;
    uint32_t value;
} cfg_addr_value_pair_t;

const cfg_addr_value_pair_t g_m2s_serdes_0_config[] =
  {
    { (uint32_t*)( 0x40028000 + 0x2028 ), 0x10F } /* SYSTEM_CONFIG_PHY_MODE_1 */ ,
    { (uint32_t*)( 0x40028000 + 0x1198 ), 0x30 } /* LANE0_PHY_RESET_OVERRIDE */ ,
    { (uint32_t*)( 0x40028000 + 0x1000 ), 0x80 } /* LANE0_CR0 */ ,
    { (uint32_t*)( 0x40028000 + 0x1004 ), 0x20 } /* LANE0_ERRCNT_DEC */ ,
    { (uint32_t*)( 0x40028000 + 0x1008 ), 0xF8 } /* LANE0_RXIDLE_MAX_ERRCNT_THR */ ,
    { (uint32_t*)( 0x40028000 + 0x100c ), 0x80 } /* LANE0_IMPED_RATIO */ ,
    { (uint32_t*)( 0x40028000 + 0x1014 ), 0x13 } /* LANE0_PLL_M_N */ ,
    { (uint32_t*)( 0x40028000 + 0x1018 ), 0x1B } /* LANE0_CNT250NS_MAX */ ,
    { (uint32_t*)( 0x40028000 + 0x1024 ), 0x80 } /* LANE0_TX_AMP_RATIO */ ,
    { (uint32_t*)( 0x40028000 + 0x1030 ), 0x10 } /* LANE0_ENDCALIB_MAX */ ,
    { (uint32_t*)( 0x40028000 + 0x1034 ), 0x38 } /* LANE0_CALIB_STABILITY_COUNT */ ,
    { (uint32_t*)( 0x40028000 + 0x103c ), 0x70 } /* LANE0_RX_OFFSET_COUNT */ ,
    { (uint32_t*)( 0x40028000 + 0x11d4 ), 0x2 } /* LANE0_GEN1_TX_PLL_CCP */ ,
    { (uint32_t*)( 0x40028000 + 0x11d8 ), 0x2 } /* LANE0_GEN1_RX_PLL_CCP */ ,
    { (uint32_t*)( 0x40028000 + 0x1198 ), 0x0 } /* LANE0_PHY_RESET_OVERRIDE */ ,
    { (uint32_t*)( 0x40028000 + 0x1200 ), 0x1 } /* LANE0_UPDATE_SETTINGS */ ,
    { (uint32_t*)( 0x40028000 + 0x2028 ), 0xF0F } /* SYSTEM_CONFIG_PHY_MODE_1 */ 
};

#define SERDES_0_CFG_NB_OF_PAIRS 17u /* From sys_config_SERDESIF_0*/

const cfg_addr_value_pair_t g_m2s_serdes_1_config[] =
{
    { (uint32_t*)( 0x4002C000 + 0x2028 ), 0x80F } /* SYSTEM_CONFIG_PHY_MODE_1 */ ,
    { (uint32_t*)( 0x4002C000 + 0x1d98 ), 0x30 } /* LANE3_PHY_RESET_OVERRIDE */ ,
    { (uint32_t*)( 0x4002C000 + 0x1c00 ), 0x80 } /* LANE3_CR0 */ ,
    { (uint32_t*)( 0x4002C000 + 0x1c04 ), 0x20 } /* LANE3_ERRCNT_DEC */ ,
    { (uint32_t*)( 0x4002C000 + 0x1c08 ), 0xF8 } /* LANE3_RXIDLE_MAX_ERRCNT_THR */ ,
    { (uint32_t*)( 0x4002C000 + 0x1c0c ), 0x80 } /* LANE3_IMPED_RATIO */ ,
    { (uint32_t*)( 0x4002C000 + 0x1c14 ), 0x29 } /* LANE3_PLL_M_N */ ,
    { (uint32_t*)( 0x4002C000 + 0x1c18 ), 0x20 } /* LANE3_CNT250NS_MAX */ ,
    { (uint32_t*)( 0x4002C000 + 0x1c24 ), 0x80 } /* LANE3_TX_AMP_RATIO */ ,
    { (uint32_t*)( 0x4002C000 + 0x1c30 ), 0x10 } /* LANE3_ENDCALIB_MAX */ ,
    { (uint32_t*)( 0x4002C000 + 0x1c34 ), 0x38 } /* LANE3_CALIB_STABILITY_COUNT */ ,
    { (uint32_t*)( 0x4002C000 + 0x1c3c ), 0x70 } /* LANE3_RX_OFFSET_COUNT */ ,
    { (uint32_t*)( 0x4002C000 + 0x1dd4 ), 0x2 } /* LANE3_GEN1_TX_PLL_CCP */ ,
    { (uint32_t*)( 0x4002C000 + 0x1dd8 ), 0x22 } /* LANE3_GEN1_RX_PLL_CCP */ ,
    { (uint32_t*)( 0x4002C000 + 0x1d98 ), 0x0 } /* LANE3_PHY_RESET_OVERRIDE */ ,
    { (uint32_t*)( 0x4002C000 + 0x1e00 ), 0x1 } /* LANE3_UPDATE_SETTINGS */ ,
    { (uint32_t*)( 0x4002C000 + 0x2028 ), 0xF0F } /* SYSTEM_CONFIG_PHY_MODE_1 */ 
};
#define SERDES_1_CFG_NB_OF_PAIRS 17u


///commands
#define SERIAL_NUMBER_REQUEST_CMD   1;
#define DESIGN_VERSION_REQUEST_CMD  5;
#define DIGEST_CHECK_REQUEST_CMD    23;

#define SYSREG    ((volatile struct SYSREG_TypeDef *) SYSREG_BASE)
#define COMBLK    ((volatile struct COMBLK_TypeDef *) COMBLK_BASE)
#define NVIC      ((volatile struct NVIC_TypeDef *) NVIC_BASE)
#define DDR       ((volatile struct DDRMem_TypeDef *) 0x4002dc18)

#define DRAM0_START  0xA0000000;
#define DRAM0_END    0xB0000000;
#define DRAM1_START  0xB0000000;
#define DRAM1_END    0xC0000000;

/*==============================================================================
 *
 */
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

/*==============================================================================
 * COMBLK interrupt servcie routine.
 */
void ComBlk_IRQHandler(void);

/*==============================================================================
 * Local functions.
 */
static void abort_current_cmd(void);
static void send_cmd_opcode(uint8_t opcode);
static uint32_t fill_tx_fifo(const uint8_t * p_cmd, uint32_t cmd_size);
static void handle_tx_okay_irq(void);
static void handle_rx_okay_irq(void);
static void complete_request(uint16_t response_length);
static void process_sys_ctrl_command(uint8_t cmd_opcode);
static void request_completion_handler(uint8_t *p_response, uint16_t response_size);


/*==============================================================================
 * Global variables:
 */
static volatile uint8_t g_comblk_cmd_opcode = 0u;
static const uint8_t * g_comblk_p_cmd = 0u;
static volatile uint16_t g_comblk_cmd_size = 0u;
static const uint8_t * g_comblk_p_data = 0u;
static volatile uint32_t g_comblk_data_size = 0u;
static uint8_t * g_comblk_p_response = 0u;
static uint16_t g_comblk_response_size = 0u;
static volatile uint16_t g_comblk_response_idx = 0u;
static comblk_completion_handler_t g_comblk_completion_handler = 0;

/*typedef uint32_t (*comblk_page_handler_t)(uint8_t const ** pp_next_page);
redefinition error
*/
static uint32_t (*g_comblk_page_handler)(uint8_t const ** pp_next_page) = 0;

static uint8_t g_request_in_progress = 0u;
static uint8_t g_last_response_length = 0u;

static uint8_t g_comblk_state = COMBLK_IDLE;

static comblk_async_event_handler_t g_async_event_handler = 0;


/* Interrupt functions */
static inline void NVIC_EnableIRQ(uint32_t);
static inline void NVIC_DisableIRQ(uint32_t);
static inline void NVIC_ClearPendingIRQ(uint32_t);

/*==============================================================================
 *
 */
//void MSS_COMBLK_init(comblk_async_event_handler_t async_event_handler)
void MSS_COMBLK_init(void)
{
    /*
     * Disable and clear previous interrupts.
     */
    //NVIC_DisableIRQ(ComBlk_IRQn);
    NVIC->ICER[((uint32_t)(ComBlk_IRQn) >> 5)] = (1 << ((uint32_t)(ComBlk_IRQn) & 0x1F));
    COMBLK->INT_ENABLE = 0u;
    //NVIC_ClearPendingIRQ(ComBlk_IRQn);
    NVIC->ICPR[((uint32_t)(ComBlk_IRQn) >> 5)] = (1 << ((uint32_t)(ComBlk_IRQn) & 0x1F)); /* Clear pending interrupt */
    
    g_async_event_handler =  0; //async_event_handler;
    
    /*
     * Initialialize COMBLK driver state variables:
     */
    g_request_in_progress = 0u;
    g_comblk_cmd_opcode = 0u;
    g_comblk_p_cmd = 0u;
    g_comblk_cmd_size = 0u;
    g_comblk_p_data = 0u;
    g_comblk_data_size = 0u;
    g_comblk_p_response = 0u;
    g_comblk_response_size = 0u;
    g_comblk_response_idx = 0u;
    g_comblk_completion_handler = 0;
    
    g_comblk_state = COMBLK_IDLE;
    
    COMBLK->CONTROL |= CR_ENABLE_MASK;
    COMBLK->CONTROL &= ~CR_LOOPBACK_MASK;
    
    /*--------------------------------------------------------------------------
     * Enable receive interrupt to receive asynchronous events from the system
     * controller.
     */
    COMBLK->INT_ENABLE &= ~TXTOKAY_MASK;
    COMBLK->INT_ENABLE |= RCVOKAY_MASK;
    //NVIC_EnableIRQ(ComBlk_IRQn);
    NVIC->ISER[((uint32_t)(ComBlk_IRQn) >> 5)] = (1 << ((uint32_t)(ComBlk_IRQn) & 0x1F)); /* enable interrupt */

    printk(KERN_INFO "COMBLK: Control register = 0x%x", COMBLK->CONTROL);
    printk(KERN_INFO "COMBLK: SYSREG -> Soft_RST_CR = 0x%x", SYSREG->SOFT_RST_CR);

}

/*==============================================================================
 *
 */
void MSS_COMBLK_send_cmd_with_ptr
(
    uint8_t cmd_opcode,
    uint32_t cmd_params_ptr,
    uint8_t * p_response,
    uint16_t response_size,
    comblk_completion_handler_t completion_handler
)
{
    
  uint32_t tx_okay;
  printk(KERN_INFO "COMBLK: Status = 0x%x\n", COMBLK->STATUS);
  printk(KERN_INFO "COMBLK: Control = 0x%x\n", COMBLK->CONTROL);


    
    /*--------------------------------------------------------------------------
     * Disable and clear previous interrupts.
     */
    //NVIC_DisableIRQ(ComBlk_IRQn);
    NVIC->ICER[((uint32_t)(ComBlk_IRQn) >> 5)] = (1 << ((uint32_t)(ComBlk_IRQn) & 0x1F));
    
    COMBLK->INT_ENABLE = 0u;
    //NVIC_ClearPendingIRQ(ComBlk_IRQn);
    NVIC->ICPR[((uint32_t)(ComBlk_IRQn) >> 5)] = (1 << ((uint32_t)(ComBlk_IRQn) & 0x1F)); /* Clear pending interrupt */
    
    /*--------------------------------------------------------------------------
     * Abort current command if any.
     */
    abort_current_cmd();
    printk(KERN_INFO "COMBLK: aborted current cmd\n");
    
    /*--------------------------------------------------------------------------
     * Initialialize COMBLK driver state variables:
     */
    g_request_in_progress = 1u;
    g_comblk_cmd_opcode = cmd_opcode;
    g_comblk_p_cmd = 0u;
    g_comblk_cmd_size = 0u;
    g_comblk_p_data = 0u;
    g_comblk_data_size = 0u;
    g_comblk_p_response = p_response;
    g_comblk_response_size = response_size;
    g_comblk_response_idx = 0u;
    g_comblk_page_handler = 0u;
    g_comblk_completion_handler = completion_handler;
    
    /*--------------------------------------------------------------------------
     * Send command opcode as a single byte write to the Tx FIFO.
     */
    send_cmd_opcode(g_comblk_cmd_opcode); // Lars: Hangs here, not anymore?
    printk(KERN_INFO "COMBLK: MSS_COMBLK_send_cmd_with_ptr(): send_cmd_opcode() ok\n");
    
    /*--------------------------------------------------------------------------
     * Send the command parameters pointer to the Tx FIFO as a single 4 bytes
     * write to the Tx FIFO.
     */
    COMBLK->CONTROL |= CR_SIZETX_MASK;
    
    /* Wait for space to become available in Tx FIFO. */
    do {
        tx_okay = COMBLK->STATUS & TXTOKAY_MASK;
    } while(0u == tx_okay);

    
    printk(KERN_INFO "COMBLK: Status = 0x%x\n", COMBLK->STATUS);
    //printk(KERN_INFO " Control = 0x%x\n", COMBLK->CONTROL);
    
    /* Send command opcode. */
    COMBLK->DATA32  = cmd_params_ptr;
  
    msleep(1);
    printk(KERN_INFO "COMBLK: Data32 : 0x%x", cmd_params_ptr);
    printk(KERN_INFO "COMBLK: Status = 0x%x\n", COMBLK->STATUS);
    //printk(KERN_INFO " Control = 0x%x\n", COMBLK->CONTROL);


    COMBLK->CONTROL &= ~CR_SIZETX_MASK;    
    g_comblk_state = COMBLK_WAIT_RESPONSE;


    
    /*--------------------------------------------------------------------------
     * Enable interrupt.
     */
    COMBLK->INT_ENABLE |= RCVOKAY_MASK;
    //NVIC_EnableIRQ(ComBlk_IRQn);
    NVIC->ISER[((uint32_t)(ComBlk_IRQn) >> 5)] = (1 << ((uint32_t)(ComBlk_IRQn) & 0x1F)); /* enable interrupt */
    //NVIC->ISER[0] = (1 << ComBlk_IRQn);
    ComBlk_IRQHandler();
    printk(KERN_INFO "COMBLK: MSS_COMBLK_send_cmd_with_ptr() done\n");
    g_request_in_progress = 0u; // Lars: fixed?
}

/*==============================================================================
 *
 */
void MSS_COMBLK_send_cmd
(
    const uint8_t * p_cmd,
    uint16_t cmd_size,
    const uint8_t * p_data,
    uint32_t data_size,
    uint8_t * p_response,
    uint16_t response_size,
    comblk_completion_handler_t completion_handler
)
{
    uint32_t size_sent;
    
    //assert(cmd_size > 0);
    
    /*
     * Disable and clear previous interrupts.
     */
    //NVIC_DisableIRQ(ComBlk_IRQn);
    NVIC->ICER[((uint32_t)(ComBlk_IRQn) >> 5)] = (1 << ((uint32_t)(ComBlk_IRQn) & 0x1F));
    COMBLK->INT_ENABLE = 0u;
    //NVIC_ClearPendingIRQ(ComBlk_IRQn);
    NVIC->ICPR[((uint32_t)(ComBlk_IRQn) >> 5)] = (1 << ((uint32_t)(ComBlk_IRQn) & 0x1F)); /* Clear pending interrupt */
    
    /*
     * Abort current command if any.
     */
    abort_current_cmd();
    printk(KERN_INFO "COMBLK: aborted current cmd\n");

    /*
     * Initialialize COMBLK driver state variables:
     */
    g_request_in_progress = 1u;
    g_comblk_cmd_opcode = p_cmd[0];
    g_comblk_p_cmd = p_cmd;
    g_comblk_cmd_size = cmd_size;
    g_comblk_p_data = p_data;
    g_comblk_data_size = data_size;
    g_comblk_p_response = p_response;
    g_comblk_response_size = response_size;
    g_comblk_response_idx = 0u;
    g_comblk_page_handler = 0u;
    g_comblk_completion_handler = completion_handler;
    
    COMBLK->INT_ENABLE |= RCVOKAY_MASK;

    /*
     * Fill FIFO with command.
     */
    send_cmd_opcode(g_comblk_cmd_opcode);
    size_sent = fill_tx_fifo(&p_cmd[1], cmd_size - 1u);
    ++size_sent;    // Adjust for opcode byte sent. //
    if(size_sent < cmd_size)
    {
        g_comblk_cmd_size = g_comblk_cmd_size - (uint16_t)size_sent;
        g_comblk_p_cmd = &g_comblk_p_cmd[size_sent];
        
        g_comblk_state = COMBLK_TX_CMD;
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

    /*
     * Enable interrupt.
     */
    //NVIC_EnableIRQ(ComBlk_IRQn);
    NVIC->ISER[((uint32_t)(ComBlk_IRQn) >> 5)] = (1 << ((uint32_t)(ComBlk_IRQn) & 0x1F)); /* enable interrupt */
}

/*==============================================================================
 *
 */
void MSS_COMBLK_send_paged_cmd
(
    const uint8_t * p_cmd,
    uint16_t cmd_size,
    uint8_t * p_response,
    uint16_t response_size,
    uint32_t (*page_read_handler)(uint8_t const **),
    void (*completion_handler)(uint8_t *, uint16_t)
)
{
    uint32_t size_sent;
    uint8_t irq_enable = 0u;
    
    //assert(cmd_size > 0u);
    
    /*
     * Disable and clear previous interrupts.
     */
    //NVIC_DisableIRQ(ComBlk_IRQn);
    NVIC->ICER[((uint32_t)(ComBlk_IRQn) >> 5)] = (1 << ((uint32_t)(ComBlk_IRQn) & 0x1F));
    COMBLK->INT_ENABLE = 0u;
    //NVIC_ClearPendingIRQ(ComBlk_IRQn);
    NVIC->ICPR[((uint32_t)(ComBlk_IRQn) >> 5)] = (1 << ((uint32_t)(ComBlk_IRQn) & 0x1F)); /* Clear pending interrupt */
    
    /*
     * Abort current command if any.
     */
    abort_current_cmd();
    
    /*
     * Initialialize COMBLK driver state variables:
     */
    g_request_in_progress = 1u;
    g_comblk_cmd_opcode = p_cmd[0];
    g_comblk_p_cmd = p_cmd;
    g_comblk_cmd_size = cmd_size;
    g_comblk_p_data = 0;
    g_comblk_data_size = 0u;
    g_comblk_p_response = p_response;
    g_comblk_response_size = response_size;
    g_comblk_response_idx = 0u;
    g_comblk_page_handler = page_read_handler;
    g_comblk_completion_handler = completion_handler;
    
    /*
     * Fill FIFO with command.
     */
    send_cmd_opcode(g_comblk_cmd_opcode);
    size_sent = fill_tx_fifo(&p_cmd[1], cmd_size - 1u);
    ++size_sent;    /* Adjust for opcode byte sent. */
    if(size_sent < cmd_size)
    {
        g_comblk_cmd_size = g_comblk_cmd_size - (uint16_t)size_sent;
        g_comblk_p_cmd = &g_comblk_p_cmd[size_sent];
        
        g_comblk_state = COMBLK_TX_CMD;
        irq_enable = TXTOKAY_MASK | RCVOKAY_MASK;
    }
    else
    {
        g_comblk_cmd_size = 0u;
        g_comblk_state = COMBLK_TX_PAGED_DATA;
        irq_enable = TXTOKAY_MASK | RCVOKAY_MASK;
    }

    /*
     * Enable interrupt.
     */
    COMBLK->INT_ENABLE |= irq_enable;
    //NVIC_EnableIRQ(ComBlk_IRQn);
    NVIC->ISER[((uint32_t)(ComBlk_IRQn) >> 5)] = (1 << ((uint32_t)(ComBlk_IRQn) & 0x1F)); /* enable interrupt */
}

/*==============================================================================
 * COMBLK interrupt handler.
 */
void ComBlk_IRQHandler(void)
{
    uint8_t status;
    uint8_t tx_okay;
    uint8_t rcv_okay;
    
    status = (uint8_t)COMBLK->STATUS;
    
    /* Mask off interrupt that are not enabled.*/
    status &= COMBLK->INT_ENABLE;
    
    rcv_okay = status & RCVOKAY_MASK;

    printk(KERN_INFO "COMBLK: ComBLK_IRQHandler %02x %02x %d \n", COMBLK->STATUS, COMBLK->INT_ENABLE, status);

    if(rcv_okay)
    {
        handle_rx_okay_irq();
    }
        
    tx_okay = status & TXTOKAY_MASK;
    if(tx_okay)
    {
        handle_tx_okay_irq();
    }
}

/*==============================================================================
 *
 */
static void handle_tx_okay_irq(void)
{
    switch(g_comblk_state)
    {
      /*----------------------------------------------------------------------
         * The TX_OKAY interrupt should only be enabled for states COMBLK_TX_CMD
         * and COMBLK_TX_DATA. 
         */
        case COMBLK_TX_CMD:
            if(g_comblk_cmd_size > 0u)
            {
                uint32_t size_sent;
                size_sent = fill_tx_fifo(g_comblk_p_cmd, g_comblk_cmd_size);
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
	       //assert(0);
                abort_current_cmd();
            }
        break;
            
        case COMBLK_TX_DATA:
            if(g_comblk_data_size > 0u)
            {
                uint32_t size_sent;
                size_sent = fill_tx_fifo(g_comblk_p_data, g_comblk_data_size);
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
	      //assert(0);
                abort_current_cmd();
            }
        break;
           
        case COMBLK_TX_PAGED_DATA:
            /*
             * Read a page of data if required.
             */
            if(0u == g_comblk_data_size)
            {
                if(g_comblk_page_handler != 0)
                {
                    g_comblk_data_size = g_comblk_page_handler(&g_comblk_p_data);
                    if(0u == g_comblk_data_size)
                    {
                        COMBLK->INT_ENABLE &= ~TXTOKAY_MASK;
                        g_comblk_state = COMBLK_WAIT_RESPONSE;
                    }
                }
                else
                {
		  //assert(0);
                    abort_current_cmd();
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
                uint32_t size_sent;
                size_sent = fill_tx_fifo(g_comblk_p_data, g_comblk_data_size);
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
            complete_request(0u);
            g_comblk_state = COMBLK_IDLE;
        break;
    }
}

/*==============================================================================
 *
 */
static void handle_rx_okay_irq(void)
{
    uint16_t data16;
    uint16_t is_command;    

    do{
      data16 = (uint16_t)COMBLK->DATA8;
      is_command = data16 & DATA8_COMMAND_MASK;
            
      switch(g_comblk_state)
	{
	  /*----------------------------------------------------------------------
	   * The RCV_OKAY interrupt should only be enabled for states
	   * COMBLK_WAIT_RESPONSE and COMBLK_RX_RESPONSE. 
	   */
        case COMBLK_WAIT_RESPONSE:
	  if(is_command)
            {
	      uint8_t rxed_opcode;
	      rxed_opcode = (uint8_t)data16;
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
	      uint8_t rxed_opcode;
	      rxed_opcode = (uint8_t)data16;
	      process_sys_ctrl_command(rxed_opcode);
            }
	  else
            {
	      if(g_comblk_response_idx < g_comblk_response_size)
                {
		  uint8_t rxed_data;
                  
		  rxed_data = (uint8_t)data16;
		  g_comblk_p_response[g_comblk_response_idx] = rxed_data;
		  ++g_comblk_response_idx;
                }
	      
	      if(g_comblk_response_idx == g_comblk_response_size)
                {
		  complete_request(g_comblk_response_idx);
		  g_comblk_state = COMBLK_IDLE;
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
	      g_comblk_p_response[1] = (uint8_t)data16;
	      complete_request(2u);
	      g_comblk_state = COMBLK_IDLE;
            }
	  break;
        case COMBLK_IDLE:
	  /* Fall through */
        case COMBLK_TX_CMD:
	  /* Fall through */
        case COMBLK_TX_DATA:
	  /* Fall through */
	  if(is_command)
            {
	      uint8_t rxed_opcode;
	      rxed_opcode = (uint8_t)data16;
	      process_sys_ctrl_command(rxed_opcode);
            }
	  break;
	  
        default:
	  complete_request(0u);
	  g_comblk_state = COMBLK_IDLE;
	  break;
	}
    }while(g_comblk_response_idx<g_comblk_response_size);

}

/*==============================================================================
 *
 */
static void complete_request
(
    uint16_t response_length
)
{
  if(g_comblk_completion_handler != 0)
    {
      g_comblk_completion_handler(g_comblk_p_response, response_length);
      g_comblk_completion_handler = 0;
      g_request_in_progress = 0u;
    }
}

/*==============================================================================
 *
 */
static void abort_current_cmd(void)
{
  if(g_request_in_progress)
    {
      uint32_t flush_in_progress;
        
      /*
       * Call completion handler just in case we are in a multi threaded system
       * to avoid a task lockup.
       */
      complete_request(g_comblk_response_idx);
      
      /*
       * Flush the FIFOs
       */
      COMBLK->CONTROL |= CR_FLUSHOUT_MASK;
      do {
	flush_in_progress = COMBLK->CONTROL & CR_FLUSHOUT_MASK;         
      } while(flush_in_progress);
    }
}

/*==============================================================================
 *
 */
static void send_cmd_opcode
(
    uint8_t opcode
)
{
    uint32_t tx_okay;
    
    /* Set transmit FIFO to transfer bytes. */
    COMBLK->CONTROL &= ~CR_SIZETX_MASK;
    
    /* Wait for space to become available in Tx FIFO. */
    do {
        tx_okay = COMBLK->STATUS & TXTOKAY_MASK;
       // fprintf(stdout, "looooooooooop\n");
    } while(0u == tx_okay);
    
    /* Send command opcode. */
    COMBLK->FRAME_START8 = opcode;
    
    //printk(KERN_INFO "COMBLK->FRAME_START8 = 0x%x", COMBLK->FRAME_START8);

}

/*==============================================================================
 *
 */
static uint32_t fill_tx_fifo
(
    const uint8_t * p_cmd,
    uint32_t cmd_size
)
{
  volatile uint32_t tx_okay;
  uint32_t size_sent;
  
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

/*==============================================================================
 *
 */
static void process_sys_ctrl_command(uint8_t cmd_opcode)
{
    if(g_async_event_handler != 0)
    {
        g_async_event_handler(cmd_opcode);
    }
}


/* Functions from core_cm3.h */

/** \brief  Enable External Interrupt

    The function enables a device-specific interrupt in the NVIC interrupt controller.

    \param [in]      IRQn  External interrupt number. Value cannot be negative.
 */
static inline void NVIC_EnableIRQ(uint32_t IRQn)
{
  NVIC->ISER[((uint32_t)(IRQn) >> 5)] = (1 << ((uint32_t)(IRQn) & 0x1F)); /* enable interrupt */
}



/** \brief  Disable External Interrupt

    The function disables a device-specific interrupt in the NVIC interrupt controller.

    \param [in]      IRQn  External interrupt number. Value cannot be negative.
 */
static inline void NVIC_DisableIRQ(uint32_t IRQn)
{
  NVIC->ICER[((uint32_t)(IRQn) >> 5)] = (1 << ((uint32_t)(IRQn) & 0x1F)); /* disable interrupt */
}




/** \brief  Clear Pending Interrupt

    The function clears the pending bit of an external interrupt.

    \param [in]      IRQn  External interrupt number. Value cannot be negative.
 */
static inline void NVIC_ClearPendingIRQ(uint32_t IRQn)
{
  NVIC->ICPR[((uint32_t)(IRQn) >> 5)] = (1 << ((uint32_t)(IRQn) & 0x1F)); /* Clear pending interrupt */
}

/* user code applications */

static void request_completion_handler
(
 uint8_t *p_response,
 uint16_t  response_size
)
{
  printk(KERN_INFO "COMBLK: request_completion_handler");
  g_request_in_progress = 1;
  g_last_response_length = response_size;
}


void MSS_SYS_get_design_version(void)
{
  
  uint8_t p_design_version[2]={0,0};
  uint8_t cmd_opcode = 0;
  uint8_t response[6];
  uint8_t status;
  uint16_t response_length = 6;
  uint16_t actual_response_length;
  uint8_t inc=0;

  cmd_opcode = DESIGN_VERSION_REQUEST_CMD;
  g_request_in_progress = 1;
  g_last_response_length = 0;


  MSS_COMBLK_send_cmd_with_ptr(cmd_opcode,
			       (uint32_t) p_design_version,
			       response,
			       response_length,
			       request_completion_handler);

  /*
  while(g_request_in_progress){
    ;
  }
  */
  actual_response_length = g_last_response_length;
  
  if( (response_length == actual_response_length) && 
      (cmd_opcode == response[0]))
    {
      status = response[1];
    }
  else
    {
      status = 200;
    }

  printk(KERN_INFO "COMBLK: MSS_SYS_get_design_version result : status = %d, opcode = %d", status, response[0]);
  printk(KERN_INFO "COMBLK: Fabric Design Version = %02x %02x", 
	 p_design_version[0],p_design_version[1]);


  //////////////////////////
  ///// this is just for serdes0 and serdes1

  

  //printk(KERN_INFO "SERDES0 and SERDES1");
  //printk(KERN_INFO "0x%x", DDR->data);

  /*
  for(inc = 0; inc < SERDES_1_CFG_NB_OF_PAIRS; ++inc){
    printk(KERN_INFO "%p -- 0x%x", g_m2s_serdes_1_config[inc].p_reg, *g_m2s_serdes_1_config[inc].p_reg);
  }
  */

}

void MSS_SYS_get_serial_number(void)
{
  uint8_t p_serial_number[16]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
  uint8_t cmd_opcode = 0;
  uint8_t response[6];
  uint8_t status;
  uint16_t response_length = 6;
  uint16_t actual_response_length;

  cmd_opcode = SERIAL_NUMBER_REQUEST_CMD;
  g_request_in_progress = 1;
  g_last_response_length = 0;


  MSS_COMBLK_send_cmd_with_ptr(cmd_opcode,
			       (uint32_t) p_serial_number,
			       response,
			       response_length,
			       request_completion_handler);

  /*
  while(g_request_in_progress){
    ;
  }
  */
  actual_response_length = g_last_response_length;
  
  if( (response_length == actual_response_length) && 
      (cmd_opcode == response[0]))
    {
      status = response[1];
    }
  else
    {
      status = 200;
    }


  printk(KERN_INFO "COMBLK: MSS_SYS_get_serial_number result : status = %d, opcode = %d", status, response[0]);
  printk(KERN_INFO "COMBLK: Device serial number = %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x", 
	 p_serial_number[0],p_serial_number[1],p_serial_number[2],p_serial_number[3],
	 p_serial_number[4],p_serial_number[5],p_serial_number[6],p_serial_number[7],
	 p_serial_number[8],p_serial_number[9],p_serial_number[10],p_serial_number[11],
	 p_serial_number[12],p_serial_number[13],p_serial_number[14],p_serial_number[15]);

  //return status;   
}

void MSS_SYS_digest_check(void)
{
  uint8_t option = 0x01;
  uint8_t status;
  uint16_t actual_response_length;
  uint8_t digest_check_req[2];
  uint8_t response[2];
  
  //signal_request_start();

  digest_check_req[0] = DIGEST_CHECK_REQUEST_CMD;
  digest_check_req[1] = option;
  

  MSS_COMBLK_send_cmd( digest_check_req,
		       sizeof(digest_check_req),
		       0,
		       0,
		       response,
		       2,
		       request_completion_handler);


  //  while(g_request_in_progress){
  //  ;
  //}
  //actual_response_length = g_last_response_length;
  
  status = response[0];

  if((2 == actual_response_length) && 
     (digest_check_req[0] == response[0]))
    {
      status = response[1];
    }
  else
    {
      status = 200;
    }

  printk(KERN_INFO "COMBLK: MSS_SYS_check_digest : status = %d", status);

}


void comblk_test(void){
  printk(KERN_INFO "COMBLK: Retrieving serial number and Fabric design version through COMBLK\n");
}  

void MSS_DDR_copy(void){
  
  //printk(KERN_INFO "COPY DDR0 (0xA000.0000-0xAFFF.FFFF) to DDR1 (0xB000.0000 - 0xBFFF.FFFF) ..... ");
  /*  
  uint32_t *data_start = (uint32_t*)DRAM0_START;
  uint32_t *data_end = (uint32_t*)DRAM0_END;
  uint32_t *target_start = (uint32_t*)DRAM1_START;

  uint32_t *addr = data_start;
  uint32_t *target = target_start;
  
  for(addr=data_start; addr<data_end; addr++){
    *target = addr;
    target++;
  }
  */
  //printk(KERN_INFO "COPY DONE \n");
}




