/*
 * comblk.h
 */

#ifndef _COMBLK_H_
#define _COMBLK_H_


/*----------------------------------------------------------------------------*/
/*---------------------------------- COMBLK ----------------------------------*/
/*----------------------------------------------------------------------------*/
 struct COMBLK_TypeDef 
 { 
   uint32_t CONTROL;            /* 0x00 */
   uint32_t STATUS;             /* 0x04 */
   uint32_t INT_ENABLE; /* 0x08 */
   uint32_t RESERVED;           /* 0x0C - not used */
   uint32_t DATA8;              /* 0x10 */
   uint32_t DATA32;             /* 0x14 */
   uint32_t FRAME_START8;       /* 0x18 */
   uint32_t FRAME_START32;      /* 0x1C */
 };

/*----------------------------------------------------------------------------*/
/*------------------------- M3 NVIC (core_cm3.h) -----------------------------*/
/*----------------------------------------------------------------------------*/
struct NVIC_TypeDef 
 { 
   uint32_t ISER[8];                 /*!< Offset: 0x000 (R/W)  Interrupt Set Enable Register           */
   uint32_t RSERVED0[24]; 
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


struct NVIC_INTR_CTRL_TypeDef
{
  uint32_t data;
};

struct SCB_TypeDef
{
  uint32_t CPUID;                   /*!< Offset: 0x000 (R/ )  CPUID Base Register                                   */
  uint32_t ICSR;                    /*!< Offset: 0x004 (R/W)  Interrupt Control and State Register                  */
  uint32_t VTOR;                    /*!< Offset: 0x008 (R/W)  Vector Table Offset Register                          */
  uint32_t AIRCR;                   /*!< Offset: 0x00C (R/W)  Application Interrupt and Reset Control Register      */
  uint32_t SCR;                     /*!< Offset: 0x010 (R/W)  System Control Register                               */
  uint32_t CCR;                     /*!< Offset: 0x014 (R/W)  Configuration Control Register                        */
  uint8_t  SHP[12];                 /*!< Offset: 0x018 (R/W)  System Handlers Priority Registers (4-7, 8-11, 12-15) */
  uint32_t SHCSR;                   /*!< Offset: 0x024 (R/W)  System Handler Control and State Register             */
  uint32_t CFSR;                    /*!< Offset: 0x028 (R/W)  Configurable Fault Status Register                    */
  uint32_t HFSR;                    /*!< Offset: 0x02C (R/W)  HardFault Status Register                             */
  uint32_t DFSR;                    /*!< Offset: 0x030 (R/W)  Debug Fault Status Register                           */
  uint32_t MMFAR;                   /*!< Offset: 0x034 (R/W)  MemManage Fault Address Register                      */
  uint32_t BFAR;                    /*!< Offset: 0x038 (R/W)  BusFault Address Register                             */
  uint32_t AFSR;                    /*!< Offset: 0x03C (R/W)  Auxiliary Fault Status Register                       */
  uint32_t PFR[2];                  /*!< Offset: 0x040 (R/ )  Processor Feature Register                            */
  uint32_t DFR;                     /*!< Offset: 0x048 (R/ )  Debug Feature Register                                */
  uint32_t ADR;                     /*!< Offset: 0x04C (R/ )  Auxiliary Feature Register                            */
  uint32_t MMFR[4];                 /*!< Offset: 0x050 (R/ )  Memory Model Feature Register                         */
  uint32_t ISAR[5];                 /*!< Offset: 0x060 (R/ )  Instruction Set Attributes Register                   */
  uint32_t RESERVED0[5];
  uint32_t CPACR;                   /*!< Offset: 0x088 (R/W)  Coprocessor Access Control Register                   */
};


#define COMBLK_BASE             0x40016000 
#define SYSREG_BASE             0x40038000 
 /******************************************************************************/
 /*                         Cortex M3 memory map                               */
 /******************************************************************************/
#define SCS_BASE            0xE000E000        /*!< System Control Space Base Address  */
#define NVIC_CTR_BASE       0xE000E004   /*!< NVIC Base Address                  */

#define COMBLK    ((volatile struct COMBLK_TypeDef *) COMBLK_BASE)
#define NVIC      ((volatile struct NVIC_TypeDef *) 0xE000E100)
#define SCB       ((volatile struct SCB_TypeDef *) 0xE000ED00)
#define NVIC_CTR  ((volatile struct NVIC_INTR_CTRL_TypeDef *) NVIC_CTR_BASE)


  /* Interrupt numbers */

#define ComBlk_IRQn                    19
#define ComBlk_SPI0                    2


struct MSMModule_TypeDef 
 { 
   uint32_t bootstrap_version;            /* 0x48 */
   uint32_t uboot_version;             /* 0x4C */
   uint32_t linux_version;             /* 0x50 */
   uint32_t rtos_version;             /* 0x54 */
};

#define MSMModule_BASE             0x50000048
#define MSMVERSION ((volatile struct MSMModule_TypeDef *) MSMModule_BASE)

#endif /* _COMBLK_H_ */
