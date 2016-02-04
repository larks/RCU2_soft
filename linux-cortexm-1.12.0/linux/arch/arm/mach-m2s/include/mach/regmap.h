/*----------------------------------------------------------------------------*/
/*------------------------------ System Registers ----------------------------*\/ */
/*----------------------------------------------------------------------------*\/ */
struct SYSREG_TypeDef 
{ 
   uint32_t    ESRAM_CR;                                 /*0X0   */
   uint32_t    ESRAM_MAX_LAT_CR;                         /*0X4   */
   uint32_t    DDR_CR;                                   /*0X8   */
   uint32_t    ENVM_CR;                                  /*0XC   */
   uint32_t    ENVM_REMAP_BASE_CR;                       /*0X10  */
   uint32_t    ENVM_REMAP_FAB_CR;                        /*0X14  */
   uint32_t    CC_CR;                                    /*0X18  */
   uint32_t    CC_REGION_CR;                             /*0X1C  */
   uint32_t    CC_LOCK_BASE_ADDR_CR;                     /*0X20  */
   uint32_t    CC_FLUSH_INDX_CR;                         /*0X24  */
   uint32_t    DDRB_BUF_TIMER_CR;                        /*0X28  */
   uint32_t    DDRB_NB_ADDR_CR;                          /*0X2C  */
   uint32_t    DDRB_NB_SIZE_CR;                          /*0X30  */
   uint32_t    DDRB_CR;                                  /*0X34  */
   uint32_t    EDAC_CR;                                  /*0X38  */
   uint32_t    MASTER_WEIGHT0_CR;                        /*0X3C  */
   uint32_t    MASTER_WEIGHT1_CR;                        /*0X40  */
   uint32_t    SOFT_IRQ_CR;                              /*0X44  */
   uint32_t    SOFT_RST_CR;                              /*0X48  */
   uint32_t    M3_CR;                                    /*0X4C  */
   uint32_t    FAB_IF_CR;                                /*0X50  */
   uint32_t    LOOPBACK_CR;                              /*0X54  */
   uint32_t    GPIO_SYSRESET_SEL_CR;                     /*0X58  */
   uint32_t    GPIN_SRC_SEL_CR;                          /*0X5C  */
   uint32_t    MDDR_CR;                                  /*0X60  */
   uint32_t    USB_IO_INPUT_SEL_CR;                      /*0X64  */
   uint32_t    PERIPH_CLK_MUX_SEL_CR;                    /*0X68  */
   uint32_t    WDOG_CR;                                  /*0X6C  */
   uint32_t    MDDR_IO_CALIB_CR;                         /*0X70  */
   uint32_t    SPARE_OUT_CR;                             /*0X74  */
   uint32_t    EDAC_IRQ_ENABLE_CR;                       /*0X78  */
   uint32_t    USB_CR;                                   /*0X7C  */
   uint32_t    ESRAM_PIPELINE_CR;                        /*0X80  */
   uint32_t    MSS_IRQ_ENABLE_CR;                        /*0X84  */
   uint32_t    RTC_WAKEUP_CR;                            /*0X88  */
   uint32_t    MAC_CR;                                   /*0X8C  */
   uint32_t    MSSDDR_PLL_STATUS_LOW_CR;                 /*0X90  */
   uint32_t    MSSDDR_PLL_STATUS_HIGH_CR;                /*0X94  */
   uint32_t    MSSDDR_FACC1_CR;                          /*0X98  */
   uint32_t    MSSDDR_FACC2_CR;                          /*0X9C  */
   uint32_t    PLL_LOCK_EN_CR;                           /*0XA0  */
   uint32_t    MSSDDR_CLK_CALIB_CR;                      /*0XA4  */
   uint32_t    PLL_DELAY_LINE_SEL_CR;                    /*0XA8  */
   uint32_t    MAC_STAT_CLRONRD_CR;                      /*0XAC  */
   uint32_t    RESET_SOURCE_CR;                          /*0XB0  */
   uint32_t    CC_DC_ERR_ADDR_SR;                        /*0XB4  */
   uint32_t    CC_IC_ERR_ADDR_SR;                        /*0XB8  */
   uint32_t    CC_SB_ERR_ADDR_SR;                        /*0XBC  */
   uint32_t    CC_DECC_ERR_ADDR_SR;                      /*0XC0  */
   uint32_t    CC_IC_MISS_CNT_SR;                        /*0XC4  */
   uint32_t    CC_IC_HIT_CNT_SR;                         /*0XC8  */
   uint32_t    CC_DC_MISS_CNT_SR;                        /*0XCC  */
   uint32_t    CC_DC_HIT_CNT_SR;                         /*0XD0  */
   uint32_t    CC_IC_TRANS_CNT_SR;                       /*0XD4  */
   uint32_t    CC_DC_TRANS_CNT_SR;                       /*0XD8  */
   uint32_t    DDRB_DS_ERR_ADR_SR;                       /*0XDC  */
   uint32_t    DDRB_HPD_ERR_ADR_SR;                      /*0XE0  */
   uint32_t    DDRB_SW_ERR_ADR_SR;                       /*0XE4  */
   uint32_t    DDRB_BUF_EMPTY_SR;                        /*0XE8  */
   uint32_t    DDRB_DSBL_DN_SR;                          /*0XEC  */
   uint32_t    ESRAM0_EDAC_CNT;                          /*0XF0  */
   uint32_t    ESRAM1_EDAC_CNT;                          /*0XF4  */
   uint32_t    CC_EDAC_CNT;                              /*0XF8  */
   uint32_t    MAC_EDAC_TX_CNT;                          /*0XFC  */
   uint32_t    MAC_EDAC_RX_CNT;                          /*0X100 */
   uint32_t    USB_EDAC_CNT;                             /*0X104 */
   uint32_t    CAN_EDAC_CNT;                             /*0X108 */
   uint32_t    ESRAM0_EDAC_ADR;                          /*0X10C */
   uint32_t    ESRAM1_EDAC_ADR;                          /*0X110 */
   uint32_t    MAC_EDAC_RX_ADR;                          /*0X114 */
   uint32_t    MAC_EDAC_TX_ADR;                          /*0X118 */
   uint32_t    CAN_EDAC_ADR;                             /*0X11C */
   uint32_t    USB_EDAC_ADR;                             /*0X120 */
   uint32_t    MM0_1_2_SECURITY;                         /*0X124 */
   uint32_t    MM4_5_FIC64_SECURITY;                     /*0X128 */
   uint32_t    MM3_6_7_8_SECURITY;                       /*0X12C */
   uint32_t    MM9_SECURITY;                             /*0X130 */
   uint32_t    M3_SR;                                    /*0X134 */
   uint32_t    ETM_COUNT_LOW;                            /*0X138 */
   uint32_t    ETM_COUNT_HIGH;                           /*0X13C */
   uint32_t    DEVICE_SR;                                /*0X140 */
   uint32_t    ENVM_PROTECT_USER;                        /*0X144 */
   uint32_t    ENVM_STATUS;                              /*0X148 */
   uint32_t    DEVICE_VERSION;                           /*0X14C */
   uint32_t    MSSDDR_PLL_STATUS;                        /*0X150 */
   uint32_t    USB_SR;                                   /*0X154 */
   uint32_t    ENVM_SR;                                  /*0X158 */
   uint32_t    SPARE_IN;                                 /*0X15C */
   uint32_t    DDRB_STATUS;                              /*0X160 */
   uint32_t    MDDR_IO_CALIB_STATUS;                     /*0X164 */
   uint32_t    MSSDDR_CLK_CALIB_STATUS;                  /*0X168 */
   uint32_t    WDOGLOAD;                                 /*0X16C */
   uint32_t    WDOGMVRP;                                 /*0X170 */
   uint32_t    USERCONFIG0;                              /*0X174 */
   uint32_t    USERCONFIG1;                              /*0X178 */
   uint32_t    USERCONFIG2;                              /*0X17C */
   uint32_t    USERCONFIG3;                              /*0X180 */
   uint32_t    FAB_PROT_SIZE;                            /*0X184 */
   uint32_t    FAB_PROT_BASE;                            /*0X188 */
   uint32_t    MSS_GPIO_DEF;                             /*0X18C */
   uint32_t    EDAC_SR;                                  /*0X190 */
   uint32_t    MSS_INTERNAL_SR;                          /*0X194 */
   uint32_t    MSS_EXTERNAL_SR;                          /*0X198 */
   uint32_t    WDOGTIMEOUTEVENT;                         /*0X19C */
   uint32_t    CLR_MSS_COUNTERS;                         /*0X1A0 */
   uint32_t    CLR_EDAC_COUNTERS;                        /*0X1A4 */
   uint32_t    FLUSH_CR;                                 /*0X1A8 */
   uint32_t    MAC_STAT_CLR_CR;                          /*0X1AC */
   uint32_t    IOMUXCELL_CONFIG[57];                     /*0X1B0 */
   uint32_t    NVM_PROTECT_FACTORY;                      /*0X294 */
   uint32_t    DEVICE_STATUS_FIXED;                      /*0X298 */
   uint32_t    MBIST_ES0;                                /*0X29C */
   uint32_t    MBIST_ES1;                                /*0X2A0 */
   uint32_t    MSDDR_PLL_STAUS_1;                        /*0X2A4 */
   uint32_t    REDUNDANCY_ESRAM0;                        /*0X2A8 */
   uint32_t    REDUNDANCY_ESRAM1;                        /*0X2AC */
   uint32_t    SERDESIF;                                 /*0X2B0 */
  
 } ; 

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

 /*----------------------------------------------------------------------------*/
 /*---------------------------------- COMBLK ----------------------------------*/
 /*----------------------------------------------------------------------------*/
 struct COMBLK_TypeDef 
 { 
   uint32_t CONTROL;		/* 0x00 */
   uint32_t STATUS;		/* 0x04 */
   uint32_t INT_ENABLE;	/* 0x08 */
   uint32_t RESERVED;		/* 0x0C - not used */
   uint32_t DATA8; 		/* 0x10 */
   uint32_t DATA32;		/* 0x14 */
   uint32_t FRAME_START8;	/* 0x18 */
   uint32_t FRAME_START32;	/* 0x1C */
 };


 /*----------------------------------------------------------------------------*/
 /*------------------------- M3 NVIC (core_cm3.h) -----------------------------*/
 /*----------------------------------------------------------------------------*/
 struct NVIC_TypeDef 
 { 
   uint32_t ISER[8];                 /*!< Offset: 0x000 (R/W)  Interrupt Set Enable Register           */
   uint32_t ICER[8];                 /*!< Offset: 0x080 (R/W)  Interrupt Clear Enable Register         */
   uint32_t RSERVED1[24]; 
   uint32_t ISPR[8];                 /*!< Offset: 0x100 (R/W)  Interrupt Set Pending Register          */
   uint32_t RESERVED2[24];
   uint32_t ICPR[8];                 /*!< Offset: 0x180 (R/W)  Interrupt Clear Pending Register        */
   uint32_t RESERVED3[24];
   uint32_t IABR[8];                 /*!< Offset: 0x200 (R/W)  Interrupt Active bit Register           */
   uint32_t RESERVED4[56];
   uint8_t  IP[240];                 /*!< Offset: 0x300 (R/W)  Interrupt Priority Register (8Bit wide) */
   uint32_t RESERVED5[644];
   uint32_t STIR;                    /*!< Offset: 0xE00 ( /W)  Software Trigger Interrupt Register     */
 };  



struct DDRMem_TypeDef 
{ 
  uint32_t data; 
};

 /******************************************************************************/
 /*                         Peripheral memory map                              */
 /******************************************************************************/
#define COMBLK_BASE             0x40016000 
#define SYSREG_BASE             0x40038000 
 /******************************************************************************/
 /*                         Cortex M3 memory map                               */
 /******************************************************************************/
#define SCS_BASE            0xE000E000        /*!< System Control Space Base Address  */
#define NVIC_BASE           SCS_BASE +  0x0100  /*!< NVIC Base Address                  */

#define DDR_BASE            0xA0000000 

 //*****************************************************************************/
 /* bitband address calculation macro                             */
 //*****************************************************************************/
#define BITBAND_ADDRESS(X)  ((X & 0xF0000000U) + 0x02000000U + ((X & 0xFFFFFU) << 5))

  /* Interrupt numbers */

#define ComBlk_IRQn                    19


struct MSMModule_TypeDef 
{ 
  uint32_t bootstrap_version;            /* 0x48 */
  uint32_t uboot_version;             /* 0x4C */
  uint32_t linux_version;             /* 0x50 */
  uint32_t rtos_version;             /* 0x54 */
};

#define MSMModule_BASE             0x50000048
#define MSMVERSION ((volatile struct MSMModule_TypeDef *) MSMModule_BASE)

/******************************************************************************/
/*                         Peripheral declaration                             */
/******************************************************************************/

