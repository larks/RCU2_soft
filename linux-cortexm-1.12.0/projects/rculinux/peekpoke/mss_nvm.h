/*******************************************************************************
 * (c) Copyright 2011-2013 Microsemi SoC Products Group.  All rights reserved.
 * 
 * This file contains public APIs for SmartFusion2 eNVM software driver.
 * 
 * SVN $Revision: 6113 $
 * SVN $Date: 2014-02-10 18:35:17 +0530 (Mon, 10 Feb 2014) $
 */
/*=========================================================================*//**
  @mainpage SmartFusion2 MSS eNVM Bare Metal Driver.
  
  @section intro_sec Introduction
  The SmartFusion2 microcontroller subsystem (MSS) includes up to two embedded
  non-volatile memory (eNVM) blocks. Each of these eNVM blocks can be a maximum
  size of 256kB. This software driver provides a set of functions for accessing
  and controlling the MSS eNVM as part of a bare metal system where no operating
  system is available. The driver can be adapted for use as part of an operating
  system, but the implementation of the adaptation layer between the driver and
  the operating system's driver model is outside the scope of the driver.
  
  The MSS eNVM driver provides support for the following features:
    - eNVM write (program) operations.
    - eNVM page unlocking
  The MSS eNVM driver is provided as C source code.

  
  @section configuration Driver Configuration
  The size of the MSS eNVM varies with different SmartFusion2 device types. You
  must only use this driver to access memory locations within the valid MSS eNVM
  address space for the targeted device. The size of the valid MSS eNVM address
  space corresponds to the size of the MSS eNVM in the device. Some pages of the
  MSS eNVM may be write protected by the SmartFusion2 MSS configurator as part
  of the hardware design flow. The driver cannot unlock or write to these
  protected pages.
  The base address, register addresses and interrupt number assignment for the
  MSS eNVM blocks are defined as constants in the SmartFusion2 CMSIS HAL. You
  must ensure that the latest SmartFusion2 CMSIS HAL is included in the project
  settings of the software tool chain used to build your project and that it is
  generated into your project.

  @section theory_op Theory of Operation
  The total amount of eNVM available in a SmartFusion device ranges from 128kB
  to 512kB, provided in one or two physical eNVM blocks. The eNVM blocks are
  divided into pages, with each page holding 128 bytes of data. The MSS eNVM
  driver treats the entire eNVM as a contiguous memory space. It provides write
  access to all pages that are in the valid eNVM address range for the
  SmartFusion device and that are not write-protected. The driver imposes no
  restrictions on writing data across eNVM block or page boundaries. The driver
  supports random access writes to the eNVM memory. 

 *//*=========================================================================*/
#ifndef __MSS_NVM_H
#define __MSS_NVM_H

#include <stdint.h>
/******************************************************************************/
/* Public definitions                                                         */
/******************************************************************************/
/*******************************************************************************
 * Page Locking constants:
 */
/*
 * Indicates that the NVM_write() function should not lock the addressed pages
 * after programming the data.
 */
#define NVM_DO_NOT_LOCK_PAGE    0u

/*
 * Indicates that the NVM_write() function should lock the addressed pages after
 * programming the data.
 */
#define NVM_LOCK_PAGE           1u

/*******************************************************************************
  The nvm_status_t enumeration specifies the possible return values from the
  NVM_write() and NVM_unlock() functions.
  
    NVM_SUCCESS:
      Indicates that the programming was successful.
        
    NVM_PROTECTION_ERROR:
      Indicates that the operation could not be completed because of a
      protection error. This happens when attempting to program a page that was
      set as protected in the hardware flow.
      
    NVM_VERIFY_FAILURE:
      Indicates that one of the verify operations failed.
      
    NVM_PAGE_LOCK_ERROR:
      Indicates that the operation could not complete because one of the pages
      is locked. This may happen if the page was locked during a previous call
      to NVM_write() or if the page was locked in the hardware design flow.
      
    NVM_WRITE_THRESHOLD_ERROR:
      Indicates that the NVM maximum number of programming cycles has been
      reached.
      
    NVM_IN_USE_BY_OTHER_MASTER:
      Indicates that some other MSS AHB Bus Matrix master is accessing the NVM.
      This could be due to the FPGA logic or the system controller programming
      the NVM.
      
    NVM_INVALID_PARAMETER:
      Indicates that one of more of the function parameters has an invalid
      value. This is typically returned when attempting to write or unlock 
      the eNVM for invalid address, data pointer, lock page and more
      eNVM than is available on the device.
 */
typedef enum nvm_status
{
    NVM_SUCCESS = 0,
    NVM_PROTECTION_ERROR,
    NVM_VERIFY_FAILURE,
    NVM_PAGE_LOCK_ERROR,
    NVM_WRITE_THRESHOLD_ERROR,
    NVM_IN_USE_BY_OTHER_MASTER,
    NVM_INVALID_PARAMETER
} nvm_status_t;

/******************************************************************************/
/* Public variables                                                           */
/******************************************************************************/


/******************************************************************************/
/* Public function declarations                                               */
/******************************************************************************/

/***************************************************************************//**
  The NVM_write() function is used to program (or write) data into the eNVM.
  This function treats the two eNVM blocks contiguously, so a total of 512kB of
  memory can be accessed linearly. The start address and end address of the
  memory range to be programmed do not need to be page aligned. This function
  supports programming of data that spans multiple pages. This function is a
  blocking function.
  Note: The NVM_write() function performs a verify operation on each page 
        programmed to ensure the eNVM is programmed with the expected data.

  @param start_addr
    The start_addr parameter is the byte aligned start address in the eNVM
    address space, to which the data is to be programmed.

  @param pidata
    The pidata parameter is the byte aligned start address of a buffer 
    containing the data to be programmed.

  @param length
    The length parameter is the number of bytes of data to be programmed.

  @param lock_page
    The lock_page parameter specifies whether the pages that are programmed
    must be locked or not once programmed. Locking the programmed pages prevents
    them from being overwritten by mistake. Subsequent programming of these
    pages will require the pages to be unlocked prior to calling NVM_write().
    Allowed values for lock_page are:
        - NVM_DO_NOT_LOCK_PAGE
        - NVM_LOCK_PAGE
        
  @return
    This function returns NVM_SUCCESS on successful execution. 
    It returns one of the following error codes if the programming of the eNVM
    fails:
        - NVM_PROTECTION_ERROR
        - NVM_VERIFY_FAILURE
        - NVM_PAGE_LOCK_ERROR
        - NVM_WRITE_THRESHOLD_ERROR
        - NVM_IN_USE_BY_OTHER_MASTER
        - NVM_INVALID_PARAMETER
        
  Example:
  @code
    uint8_t idata[815] = {"Z"};
    status = NVM_write(0x0, idata, sizeof(idata), NVM_DO_NOT_LOCK_PAGE);
  @endcode
 */
nvm_status_t 
NVM_write
(
    uint32_t start_addr,
    const uint8_t * pidata,
    uint32_t length,
    uint32_t lock_page
);

/***************************************************************************//**
  The NVM_unlock() function is used to unlock the eNVM pages for a specified
  range of eNVM addresses in preparation for writing data into the unlocked
  locations. This function treats the two eNVM blocks contiguously, so a total
  of 512kB of memory can be accessed linearly. The start address and end address
  of the memory range to be unlocked do not need to be page aligned. This
  function supports unlocking of an eNVM address range that spans multiple
  pages. This function is a blocking function.

  @param start_addr
    The start_addr parameter is the byte aligned start address, in the eNVM
    address space, of the memory range to be unlocked.
    
  @param length
    The length parameter is the size in bytes of the memory range to be
    unlocked.

  @return
    This function returns NVM_SUCCESS on successful execution. 
    It returns one of the following error codes if the unlocking of the eNVM fails:
        - NVM_PROTECTION_ERROR
        - NVM_VERIFY_FAILURE
        - NVM_PAGE_LOCK_ERROR
        - NVM_WRITE_THRESHOLD_ERROR
        - NVM_IN_USE_BY_OTHER_MASTER
        - NVM_INVALID_PARAMETER
        
  The example code below demonstrates the intended use of the NVM_unlock()
  function:
  @code
    int program_locked_nvm(uint32_t target_addr, uint32_t length)
    {
        nvm_status_t status;
        int success = 0;
        
        status = NVM_unlock(target_addr, length);
        if(NVM_SUCCESS == status)
        {
            status = NVM_write(target_addr, buffer, length, NVM_LOCK_PAGE);
            if(NVM_SUCCESS == status)
            {
                success = 1; 
            }
        }
        return success;
    }
  @endcode
 */
nvm_status_t
NVM_unlock
(
    uint32_t start_addr,
    uint32_t length
);


/*----------------------------------------------------------------------------*/
/*------------------------------ System Registers ----------------------------*/
/*----------------------------------------------------------------------------*/
typedef struct
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

} SYSREG_TypeDef;

#define SYSREG_BASE             0x40038000u
#define ENVM1_BASE              0x60080000u
#define ENVM2_BASE              0x600C0000u



#endif /* __MSS_NVM_H */


