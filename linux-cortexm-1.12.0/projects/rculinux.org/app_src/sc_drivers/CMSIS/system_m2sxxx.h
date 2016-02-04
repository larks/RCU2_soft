/*******************************************************************************
 * (c) Copyright 2012 Microsemi Corporation.  All rights reserved.
 * 
 *  SmartFusion2 CMSIS system initialization.
 *
 * SVN $Revision: 5184 $
 * SVN $Date: 2013-03-18 11:13:58 +0000 (Mon, 18 Mar 2013) $
 */

#ifndef SYSTEM_M2SXXX_H
#define SYSTEM_M2SXXX_H

#ifdef __cplusplus
extern "C" {
#endif 

/* Standard CMSIS global variables. */
extern uint32_t SystemCoreClock;    /*!< System Clock Frequency (Core Clock) */

/* SmartFusion specific clocks. */
extern uint32_t g_FrequencyPCLK0;   /*!< Clock frequency of APB bus 0. */  
extern uint32_t g_FrequencyPCLK1;   /*!< Clock frequency of APB bus 1. */
extern uint32_t g_FrequencyPCLK2;   /*!< Clock frequency of APB bus 2. */
extern uint32_t g_FrequencyFIC0;    /*!< Clock frequecny of FPGA fabric interface controller 1. */
extern uint32_t g_FrequencyFIC1;    /*!< Clock frequecny of FPGA fabric inteface controller 2. */
extern uint32_t g_FrequencyFIC64;   /*!< Clock frequecny of 64-bit FPGA fabric interface controller. */


/***************************************************************************//**
 * The SystemInit() is a standard CMSIS function called during system startup.
 * It is meant to perform low level hardware setup such as configuring PLLs. In
 * the case of SmartFusion these hardware setup operations are performed by the
 * chip boot which executed before the application started. Therefore this
 * function does not need to perform any hardware setup.
 */
void SystemInit(void);

/***************************************************************************//**
 * The SystemCoreClockUpdate() is a standard CMSIS function which can be called
 * by the application in order to ensure that the SystemCoreClock global
 * variable contains the up to date Cortex-M3 core frequency. Calling this
 * function also updates the global variables containing the frequencies of the
 * APB busses connecting the peripherals and the ACE frequency.
 */
void SystemCoreClockUpdate(void);

#ifdef __cplusplus
}
#endif

#endif
