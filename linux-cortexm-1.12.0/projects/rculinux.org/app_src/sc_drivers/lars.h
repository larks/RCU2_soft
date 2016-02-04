#ifndef __LARS_H_
#define __LARS_H_ 1

#include <stdint.h> /*Standard types definition*/

#ifdef __cplusplus
extern "C" {
#endif

/* IO definitions (access restrictions to peripheral registers) */
/**
    \defgroup CMSIS_glob_defs CMSIS Global Defines
 
    <strong>IO Type Qualifiers</strong> are used
    \li to specify the access to peripheral variables.
    \li for automatic generation of peripheral register debug information.
*/

#ifdef __cplusplus
  #define   __I     volatile             /*!< Defines 'read only' permissions                 */
#else
  #define   __I     volatile const       /*!< Defines 'read only' permissions                 */
#endif
#define     __O     volatile             /*!< Defines 'write only' permissions                */
#define     __IO    volatile             /*!< Defines 'read / write' permissions              */


/*
 * ==========================================================================
 * ---------- Interrupt Number Definition -----------------------------------
 * ==========================================================================
 */

typedef enum IRQn
{
/******  Cortex-M3 Processor Exceptions Numbers *********************************************************/
  NonMaskableInt_IRQn             = -14,      /*!< 2 Non Maskable Interrupt - Watchdog timeout interrupt*/
  HardFault_IRQn                  = -13,      /*!< 2 Hard Fault Interrupt                               */
  MemoryManagement_IRQn           = -12,      /*!< 4 Cortex-M3 Memory Management Interrupt              */
  BusFault_IRQn                   = -11,      /*!< 5 Cortex-M3 Bus Fault Interrupt                      */
  UsageFault_IRQn                 = -10,      /*!< 6 Cortex-M3 Usage Fault Interrupt                    */
  SVCall_IRQn                     = -5,       /*!< 11 Cortex-M3 SV Call Interrupt                       */
  DebugMonitor_IRQn               = -4,       /*!< 12 Cortex-M3 Debug Monitor Interrupt                 */
  PendSV_IRQn                     = -2,       /*!< 14 Cortex-M3 Pend SV Interrupt                       */
  SysTick_IRQn                    = -1,       /*!< 15 Cortex-M3 System Tick Interrupt                   */

/******  SmartFusion2 specific Interrupt Numbers *********************************************************/
  WdogWakeup_IRQn                 = 0,        /*!< WatchDog wakeup interrupt                            */
  RTC_Wakeup_IRQn                 = 1,        /*!< RTC wakeup interrupt                                 */
  SPI0_IRQn                       = 2,        /*!< SPI0 interrupt                                       */
  SPI1_IRQn                       = 3,        /*!< SPI1 interrupt                                       */
  I2C0_IRQn                       = 4,        /*!< I2C0 interrupt                                       */
  I2C0_SMBAlert_IRQn              = 5,        /*!< I2C0 SMBus Alert interrupt                           */
  I2C0_SMBus_IRQn                 = 6,        /*!< I2C0 SMBus Suspend interrupt                         */
  I2C1_IRQn                       = 7,        /*!< I2C1 interrupt                                       */
  I2C1_SMBAlert_IRQn              = 8,        /*!< I2C1 SMBus Alert interrupt                           */
  I2C1_SMBus_IRQn                 = 9,        /*!< I2C1 SMBus Suspend interrupt                         */
  UART0_IRQn                      = 10,       /*!< UART0 interrupt                                      */
  UART1_IRQn                      = 11,       /*!< UART1 interrupt                                      */
  EthernetMAC_IRQn                = 12,       /*!< Ethernet MAC interrupt                               */
  DMA_IRQn                        = 13,       /*!< Peripheral DMA interrupt                             */
  Timer1_IRQn                     = 14,       /*!< Timer1 interrupt                                     */
  Timer2_IRQn                     = 15,       /*!< Timer2 interrupt                                     */
  CAN_IRQn                        = 16,       /*!< CAN controller interrupt                             */
  ENVM0_IRQn                      = 17,       /*!< eNVM0 operation completion interrupt                 */
  ENVM1_IRQn                      = 18,       /*!< eNVM1 operation completion interrupt                 */
  ComBlk_IRQn                     = 19,       /*!< COM block interrupt                                  */
  USB_IRQn                        = 20,       /*!< USB interrupt                                        */
  USB_DMA_IRQn                    = 21,       /*!< USB DMA interrupt                                    */
  PLL_Lock_IRQn                   = 22,       /*!< PLL lock interrupt                                   */
  PLL_LockLost_IRQn               = 23,       /*!< PLL loss of lock interrupt                           */
  CommSwitchError_IR              = 24,       /*!< Communications Switch error interrupt                */
  CacheError_IRQn                 = 25,       /*!< Cache error interrupt                                */
  DDR_IRQn                        = 26,       /*!< DDR controller interrupt                             */
  HPDMA_Complete_IRQn             = 27,       /*!< High speed DMA transfer complete interrupt           */
  HPDMA_Error_IRQn                = 28,       /*!< High speed DMA transfer error interrupt              */
  ECC_Error_IRQn                  = 29,       /*!< ECC error detected                                   */
  MDDR_IOCalib_IRQn               = 30,       /*!< MDDR Calibration finished interrupt                  */
  FAB_PLL_Lock_IRQn               = 31,       /*!< G4MSSDDR Fabric PLL lock interrupt                   */
  FAB_PLL_LockLost_IRQn           = 32,       /*!< G4MSSDDR Fabric PLL lock lost interrupt              */
  FIC64_IRQn                      = 33,       /*!< FIC64 interrupt                                      */
  FabricIrq0_IRQn                 = 34,       /*!< FPGA fabric interrupt 0                              */
  FabricIrq1_IRQn                 = 35,       /*!< FPGA fabric interrupt 1                              */
  FabricIrq2_IRQn                 = 36,       /*!< FPGA fabric interrupt 2                              */
  FabricIrq3_IRQn                 = 37,       /*!< FPGA fabric interrupt 3                              */
  FabricIrq4_IRQn                 = 38,       /*!< FPGA fabric interrupt 4                              */
  FabricIrq5_IRQn                 = 39,       /*!< FPGA fabric interrupt 5                              */
  FabricIrq6_IRQn                 = 40,       /*!< FPGA fabric interrupt 6                              */
  FabricIrq7_IRQn                 = 41,       /*!< FPGA fabric interrupt 7                              */
  FabricIrq8_IRQn                 = 42,       /*!< FPGA fabric interrupt 8                              */
  FabricIrq9_IRQn                 = 43,       /*!< FPGA fabric interrupt 9                              */
  FabricIrq10_IRQn                = 44,       /*!< FPGA fabric interrupt 10                             */
  FabricIrq11_IRQn                = 45,       /*!< FPGA fabric interrupt 11                             */
  FabricIrq12_IRQn                = 46,       /*!< FPGA fabric interrupt 12                             */
  FabricIrq13_IRQn                = 47,       /*!< FPGA fabric interrupt 13                             */
  FabricIrq14_IRQn                = 48,       /*!< FPGA fabric interrupt 14                             */
  FabricIrq15_IRQn                = 49,       /*!< FPGA fabric interrupt 15                             */  
  GPIO0_IRQn                      = 50,       /*!< GPIO 0 interrupt                                     */
  GPIO1_IRQn                      = 51,       /*!< GPIO 1 interrupt                                     */
  GPIO2_IRQn                      = 52,       /*!< GPIO 2 interrupt                                     */
  GPIO3_IRQn                      = 53,       /*!< GPIO 3 interrupt                                     */
  GPIO4_IRQn                      = 54,       /*!< GPIO 4 interrupt                                     */
  GPIO5_IRQn                      = 55,       /*!< GPIO 5 interrupt                                     */
  GPIO6_IRQn                      = 56,       /*!< GPIO 6 interrupt                                     */
  GPIO7_IRQn                      = 57,       /*!< GPIO 7 interrupt                                     */
  GPIO8_IRQn                      = 58,       /*!< GPIO 8 interrupt                                     */
  GPIO9_IRQn                      = 59,       /*!< GPIO 9 interrupt                                     */
  GPIO10_IRQn                     = 60,       /*!< GPIO 10 interrupt                                    */
  GPIO11_IRQn                     = 61,       /*!< GPIO 11 interrupt                                    */
  GPIO12_IRQn                     = 62,       /*!< GPIO 12 interrupt                                    */
  GPIO13_IRQn                     = 63,       /*!< GPIO 13 interrupt                                    */
  GPIO14_IRQn                     = 64,       /*!< GPIO 14 interrupt                                    */
  GPIO15_IRQn                     = 65,       /*!< GPIO 15 interrupt                                    */
  GPIO16_IRQn                     = 66,       /*!< GPIO 16 interrupt                                    */
  GPIO17_IRQn                     = 67,       /*!< GPIO 17 interrupt                                    */
  GPIO18_IRQn                     = 68,       /*!< GPIO 18 interrupt                                    */
  GPIO19_IRQn                     = 69,       /*!< GPIO 19 interrupt                                    */
  GPIO20_IRQn                     = 70,       /*!< GPIO 20 interrupt                                    */
  GPIO21_IRQn                     = 71,       /*!< GPIO 21 interrupt                                    */
  GPIO22_IRQn                     = 72,       /*!< GPIO 22 interrupt                                    */
  GPIO23_IRQn                     = 73,       /*!< GPIO 23 interrupt                                    */
  GPIO24_IRQn                     = 74,       /*!< GPIO 24 interrupt                                    */
  GPIO25_IRQn                     = 75,       /*!< GPIO 25 interrupt                                    */ 
  GPIO26_IRQn                     = 76,       /*!< GPIO 26 interrupt                                    */
  GPIO27_IRQn                     = 77,       /*!< GPIO 27 interrupt                                    */
  GPIO28_IRQn                     = 78,       /*!< GPIO 28 interrupt                                    */
  GPIO29_IRQn                     = 79,       /*!< GPIO 29 interrupt                                    */
  GPIO30_IRQn                     = 80,       /*!< GPIO 30 interrupt                                    */
  GPIO31_IRQn                     = 81        /*!< GPIO 31 interrupt                                    */
} IRQn_Type;


/*----------------------------------------------------------------------------*/
/*---------------------------------- COMBLK ----------------------------------*/
/*----------------------------------------------------------------------------*/
typedef struct
{
    __IO uint32_t CONTROL;
    __IO uint32_t STATUS;
    __IO uint32_t INT_ENABLE;
    __IO uint32_t RESERVED;
    __IO uint32_t DATA8;
    __IO uint32_t DATA32;
    __IO uint32_t FRAME_START8;
    __IO uint32_t FRAME_START32;
} COMBLK_TypeDef;

/*----------------------------------------------------------------------------*/
/*------------------------------ System Registers ----------------------------*/
/*----------------------------------------------------------------------------*/
typedef struct
{
    __IO     uint32_t    ESRAM_CR;                                 /*0X0   */
    __IO     uint32_t    ESRAM_MAX_LAT_CR;                         /*0X4   */
    __IO     uint32_t    DDR_CR;                                   /*0X8   */
    __IO     uint32_t    ENVM_CR;                                  /*0XC   */
    __IO     uint32_t    ENVM_REMAP_BASE_CR;                       /*0X10  */
    __IO     uint32_t    ENVM_REMAP_FAB_CR;                        /*0X14  */
    __IO     uint32_t    CC_CR;                                    /*0X18  */
    __IO     uint32_t    CC_REGION_CR;                             /*0X1C  */
    __IO     uint32_t    CC_LOCK_BASE_ADDR_CR;                     /*0X20  */
    __IO     uint32_t    CC_FLUSH_INDX_CR;                         /*0X24  */
    __IO     uint32_t    DDRB_BUF_TIMER_CR;                        /*0X28  */
    __IO     uint32_t    DDRB_NB_ADDR_CR;                          /*0X2C  */
    __IO     uint32_t    DDRB_NB_SIZE_CR;                          /*0X30  */
    __IO     uint32_t    DDRB_CR;                                  /*0X34  */
    __IO     uint32_t    EDAC_CR;                                  /*0X38  */
    __IO     uint32_t    MASTER_WEIGHT0_CR;                        /*0X3C  */
    __IO     uint32_t    MASTER_WEIGHT1_CR;                        /*0X40  */
    __IO     uint32_t    SOFT_IRQ_CR;                              /*0X44  */
    __IO     uint32_t    SOFT_RST_CR;                              /*0X48  */
    __IO     uint32_t    M3_CR;                                    /*0X4C  */
    __IO     uint32_t    FAB_IF_CR;                                /*0X50  */
    __IO     uint32_t    LOOPBACK_CR;                              /*0X54  */
    __IO     uint32_t    GPIO_SYSRESET_SEL_CR;                     /*0X58  */
    __IO     uint32_t    GPIN_SRC_SEL_CR;                          /*0X5C  */
    __IO     uint32_t    MDDR_CR;                                  /*0X60  */
    __IO     uint32_t    USB_IO_INPUT_SEL_CR;                      /*0X64  */
    __IO     uint32_t    PERIPH_CLK_MUX_SEL_CR;                    /*0X68  */
    __IO     uint32_t    WDOG_CR;                                  /*0X6C  */
    __IO     uint32_t    MDDR_IO_CALIB_CR;                         /*0X70  */
    __IO     uint32_t    SPARE_OUT_CR;                             /*0X74  */
    __IO     uint32_t    EDAC_IRQ_ENABLE_CR;                       /*0X78  */
    __IO     uint32_t    USB_CR;                                   /*0X7C  */
    __IO     uint32_t    ESRAM_PIPELINE_CR;                        /*0X80  */
    __IO     uint32_t    MSS_IRQ_ENABLE_CR;                        /*0X84  */
    __IO     uint32_t    RTC_WAKEUP_CR;                            /*0X88  */
    __IO     uint32_t    MAC_CR;                                   /*0X8C  */
    __IO     uint32_t    MSSDDR_PLL_STATUS_LOW_CR;                 /*0X90  */
    __IO     uint32_t    MSSDDR_PLL_STATUS_HIGH_CR;                /*0X94  */
    __IO     uint32_t    MSSDDR_FACC1_CR;                          /*0X98  */
    __IO     uint32_t    MSSDDR_FACC2_CR;                          /*0X9C  */
    __IO     uint32_t    PLL_LOCK_EN_CR;                           /*0XA0  */
    __IO     uint32_t    MSSDDR_CLK_CALIB_CR;                      /*0XA4  */
    __IO     uint32_t    PLL_DELAY_LINE_SEL_CR;                    /*0XA8  */
    __IO     uint32_t    MAC_STAT_CLRONRD_CR;                      /*0XAC  */
    __IO     uint32_t    RESET_SOURCE_CR;                          /*0XB0  */
    __I      uint32_t    CC_DC_ERR_ADDR_SR;                        /*0XB4  */
    __I      uint32_t    CC_IC_ERR_ADDR_SR;                        /*0XB8  */
    __I      uint32_t    CC_SB_ERR_ADDR_SR;                        /*0XBC  */
    __I      uint32_t    CC_DECC_ERR_ADDR_SR;                      /*0XC0  */
    __I      uint32_t    CC_IC_MISS_CNT_SR;                        /*0XC4  */
    __I      uint32_t    CC_IC_HIT_CNT_SR;                         /*0XC8  */
    __I      uint32_t    CC_DC_MISS_CNT_SR;                        /*0XCC  */
    __I      uint32_t    CC_DC_HIT_CNT_SR;                         /*0XD0  */
    __I      uint32_t    CC_IC_TRANS_CNT_SR;                       /*0XD4  */
    __I      uint32_t    CC_DC_TRANS_CNT_SR;                       /*0XD8  */
    __I      uint32_t    DDRB_DS_ERR_ADR_SR;                       /*0XDC  */
    __I      uint32_t    DDRB_HPD_ERR_ADR_SR;                      /*0XE0  */
    __I      uint32_t    DDRB_SW_ERR_ADR_SR;                       /*0XE4  */
    __I      uint32_t    DDRB_BUF_EMPTY_SR;                        /*0XE8  */
    __I      uint32_t    DDRB_DSBL_DN_SR;                          /*0XEC  */
    __I      uint32_t    ESRAM0_EDAC_CNT;                          /*0XF0  */
    __I      uint32_t    ESRAM1_EDAC_CNT;                          /*0XF4  */
    __I      uint32_t    CC_EDAC_CNT;                              /*0XF8  */
    __I      uint32_t    MAC_EDAC_TX_CNT;                          /*0XFC  */
    __I      uint32_t    MAC_EDAC_RX_CNT;                          /*0X100 */
    __I      uint32_t    USB_EDAC_CNT;                             /*0X104 */
    __I      uint32_t    CAN_EDAC_CNT;                             /*0X108 */
    __I      uint32_t    ESRAM0_EDAC_ADR;                          /*0X10C */
    __I      uint32_t    ESRAM1_EDAC_ADR;                          /*0X110 */
    __I      uint32_t    MAC_EDAC_RX_ADR;                          /*0X114 */
    __I      uint32_t    MAC_EDAC_TX_ADR;                          /*0X118 */
    __I      uint32_t    CAN_EDAC_ADR;                             /*0X11C */
    __I      uint32_t    USB_EDAC_ADR;                             /*0X120 */
    __I      uint32_t    MM0_1_2_SECURITY;                         /*0X124 */
    __I      uint32_t    MM4_5_FIC64_SECURITY;                     /*0X128 */
    __I      uint32_t    MM3_6_7_8_SECURITY;                       /*0X12C */
    __I      uint32_t    MM9_SECURITY;                             /*0X130 */
    __I      uint32_t    M3_SR;                                    /*0X134 */
    __I      uint32_t    ETM_COUNT_LOW;                            /*0X138 */
    __I      uint32_t    ETM_COUNT_HIGH;                           /*0X13C */
    __I      uint32_t    DEVICE_SR;                                /*0X140 */
    __I      uint32_t    ENVM_PROTECT_USER;                        /*0X144 */
    __I      uint32_t    G4C_ENVM_STATUS;                          /*0X148 */
    __I      uint32_t    DEVICE_VERSION;                           /*0X14C */
    __I      uint32_t    MSSDDR_PLL_STATUS;                        /*0X150 */
    __I      uint32_t    USB_SR;                                   /*0X154 */
    __I      uint32_t    ENVM_SR;                                  /*0X158 */
    __I      uint32_t    SPARE_IN;                                 /*0X15C */
    __I      uint32_t    DDRB_STATUS;                              /*0X160 */
    __I      uint32_t    MDDR_IO_CALIB_STATUS;                     /*0X164 */
    __I      uint32_t    MSSDDR_CLK_CALIB_STATUS;                  /*0X168 */
    __I      uint32_t    WDOGLOAD;                                 /*0X16C */
    __I      uint32_t    WDOGMVRP;                                 /*0X170 */
    __I      uint32_t    USERCONFIG0;                              /*0X174 */
    __I      uint32_t    USERCONFIG1;                              /*0X178 */
    __I      uint32_t    USERCONFIG2;                              /*0X17C */
    __I      uint32_t    USERCONFIG3;                              /*0X180 */
    __I      uint32_t    FAB_PROT_SIZE;                            /*0X184 */
    __I      uint32_t    FAB_PROT_BASE;                            /*0X188 */
    __I      uint32_t    MSS_GPIO_DEF;                             /*0X18C */
    __IO     uint32_t    EDAC_SR;                                  /*0X190 */
    __IO     uint32_t    MSS_INTERNAL_SR;                          /*0X194 */
    __IO     uint32_t    MSS_EXTERNAL_SR;                          /*0X198 */
    __IO     uint32_t    WDOGTIMEOUTEVENT;                         /*0X19C */
    __IO     uint32_t    CLR_MSS_COUNTERS;                         /*0X1A0 */
    __IO     uint32_t    CLR_EDAC_COUNTERS;                        /*0X1A4 */
    __IO     uint32_t    FLUSH_CR;                                 /*0X1A8 */
    __IO     uint32_t    MAC_STAT_CLR_CR;                          /*0X1AC */
    __IO     uint32_t    IOMUXCELL_CONFIG[57];                     /*0X1B0 */
    __I      uint32_t    NVM_PROTECT_FACTORY;                      /*0X294 */
    __I      uint32_t    DEVICE_STATUS_FIXED;                      /*0X298 */
    __I      uint32_t    MBIST_ES0;                                /*0X29C */
    __I      uint32_t    MBIST_ES1;                                /*0X2A0 */
    __IO     uint32_t    MSDDR_PLL_STAUS_1;                        /*0X2A4 */
    __I      uint32_t    REDUNDANCY_ESRAM0;                        /*0X2A8 */
    __I      uint32_t    REDUNDANCY_ESRAM1;                        /*0X2AC */
    __I      uint32_t    SERDESIF;                                 /*0X2B0 */

} SYSREG_TypeDef;

#define SYSREG_ENVM0_SOFTRESET_MASK         ( (uint32_t)0x01u << 0u )
#define SYSREG_ENVM1_SOFTRESET_MASK         ( (uint32_t)0x01u << 1u )
#define SYSREG_ESRAM0_SOFTRESET_MASK        ( (uint32_t)0x01u << 2u )
#define SYSREG_ESRAM1_SOFTRESET_MASK        ( (uint32_t)0x01u << 3u )
#define SYSREG_MAC_SOFTRESET_MASK           ( (uint32_t)0x01u << 4u )
#define SYSREG_PDMA_SOFTRESET_MASK          ( (uint32_t)0x01u << 5u )
#define SYSREG_TIMER_SOFTRESET_MASK         ( (uint32_t)0x01u << 6u )
#define SYSREG_MMUART0_SOFTRESET_MASK       ( (uint32_t)0x01u << 7u )
#define SYSREG_MMUART1_SOFTRESET_MASK       ( (uint32_t)0x01u << 8u )
#define SYSREG_SPI0_SOFTRESET_MASK          ( (uint32_t)0x01u << 9u )
#define SYSREG_SPI1_SOFTRESET_MASK          ( (uint32_t)0x01u << 10u )
#define SYSREG_I2C0_SOFTRESET_MASK          ( (uint32_t)0x01u << 11u )
#define SYSREG_I2C1_SOFTRESET_MASK          ( (uint32_t)0x01u << 12u )
#define SYSREG_CAN_SOFTRESET_MASK           ( (uint32_t)0x01u << 13u )
#define SYSREG_USB_SOFTRESET_MASK           ( (uint32_t)0x01u << 14u )
#define SYSREG_COMBLK_SOFTRESET_MASK        ( (uint32_t)0x01u << 15u )
#define SYSREG_FPGA_SOFTRESET_MASK          ( (uint32_t)0x01u << 16u )
#define SYSREG_HPDMA_SOFTRESET_MASK         ( (uint32_t)0x01u << 17u )
#define SYSREG_FIC32_0_SOFTRESET_MASK       ( (uint32_t)0x01u << 18u )
#define SYSREG_FIC32_1_SOFTRESET_MASK       ( (uint32_t)0x01u << 19u )
#define SYSREG_GPIO_SOFTRESET_MASK          ( (uint32_t)0x01u << 20u )
#define SYSREG_GPIO_7_0_SOFTRESET_MASK      ( (uint32_t)0x01u << 21u )
#define SYSREG_GPIO_15_8_SOFTRESET_MASK     ( (uint32_t)0x01u << 22u )
#define SYSREG_GPIO_23_16_SOFTRESET_MASK    ( (uint32_t)0x01u << 23u )
#define SYSREG_GPIO_31_24_SOFTRESET_MASK    ( (uint32_t)0x01u << 24u )
#define SYSREG_MDDR_SOFTRESET_MASK          ( (uint32_t)0x01u << 25u )
#define SYSREG_FIC64_SOFTRESET_MASK         ( (uint32_t)0x01u << 26u )
/******************************************************************************/
/*                         Peripheral memory map                              */
/******************************************************************************/
#define UART0_BASE              0x40000000u
#define SPI0_BASE               0x40001000u
#define I2C0_BASE               0x40002000u
#define PDMA_BASE               0x40003000u
#define TIMER_BASE              0x40004000u
#define WATCHDOG_BASE           0x40005000u
#define H2F_IRQ_CTRL_BASE       0x40006000u
#define UART1_BASE              0x40010000u
#define SPI1_BASE               0x40011000u
#define I2C1_BASE               0x40012000u
#define GPIO_BASE               0x40013000u
#define HPDMA_BASE              0x40014000u
#define CAN_BASE                0x40015000u
#define COMBLK_BASE             0x40016000u
#define RTC_BASE                0x40017000u
#define DDR0_CFG_BASE           0x40020800u
#define DDR1_CFG_BASE           0x40021000u
#define CORE_SF2_CFG_BASE       0x40022000u
#define SERDES0_CFG_BASE        0x40028000u
#define SERDES1_CFG_BASE        0x4002C000u
#define SYSREG_BASE             0x40038000u
#define ETHERNET_BASE           0x40041000u
#define USB_BASE                0x40043000u
#define ENVM1_BASE              0x60080000u
#define ENVM2_BASE              0x600C0000u


/******************************************************************************/
/*                         Peripheral declaration                             */
/******************************************************************************/
#define UART0                   ((UART_TypeDef *) UART0_BASE)
#define SPI0                    ((SPI_TypeDef *) SPI0_BASE)
#define I2C0                    ((I2C_TypeDef *) I2C0_BASE)
#define I2C0_BITBAND            ((I2C_BitBand_TypeDef *) BITBAND_ADDRESS(I2C0_BASE))
#define MAC                     ((MAC_TypeDef *) ETHERNET_BASE)
#define PDMA                    ((PDMA_TypeDef *) PDMA_BASE)
#define TIMER                   ((TIMER_TypeDef *) TIMER_BASE)
#define TIMER_BITBAND           ((TIMER_BitBand_TypeDef *) BITBAND_ADDRESS(TIMER_BASE))
#define WATCHDOG                ((WATCHDOG_TypeDef *) WATCHDOG_BASE)
#define INTERRUPT_CTRL          ((INTERRUPT_CTRL_TypeDef *) H2F_IRQ_CTRL_BASE)
#define INTERRUPT_CTRL_BITBAND  ((INTERRUPT_CTRL_BitBand_TypeDef *) BITBAND_ADDRESS(H2F_IRQ_CTRL_BASE))
#define UART1                   ((UART_TypeDef *) UART1_BASE)
#define SPI1                    ((SPI_TypeDef *) SPI1_BASE)
#define I2C1                    ((I2C_TypeDef *) I2C1_BASE)
#define I2C1_BITBAND            ((I2C_BitBand_TypeDef *) BITBAND_ADDRESS(I2C1_BASE))
#define GPIO                    ((GPIO_TypeDef *) GPIO_BASE)
#define GPIO_BITBAND            ((GPIO_BitBand_TypeDef *) BITBAND_ADDRESS(GPIO_BASE))
#define HPDMA                   ((HPDMA_TypeDef *) HPDMA_BASE)
#define HPDMA_BITBAND           ((HPDMA_BitBand_TypeDef *) BITBAND_ADDRESS(HPDMA_BASE))
#define COMBLK                  ((COMBLK_TypeDef *) COMBLK_BASE)
#define RTC                     ((RTC_TypeDef *) RTC_BASE)
#define ENVM_1                  ((NVM_TypeDef *) ENVM1_BASE)
#define ENVM_2                  ((NVM_TypeDef *) ENVM2_BASE)
#define SYSREG                  ((SYSREG_TypeDef *) SYSREG_BASE)
#define MDDR                    ((MDDR_TypeDef *) DDR0_CFG_BASE)
#define FDDR                    ((FDDR_TypeDef *) DDR1_CFG_BASE) 
#define USB                     ((MSS_USB_TypeDef *) USB_BASE)
#define SERDES0                 ((SERDESIF_TypeDef *) SERDES0_CFG_BASE)
#define SERDES1                 ((SERDESIF_TypeDef *) SERDES1_CFG_BASE)
#define CORE_SF2_CFG            ((CoreSF2Config_TypeDef *) CORE_SF2_CFG_BASE)


#ifdef __cplusplus
}
#endif
#endif /* __LARS_H_ */




