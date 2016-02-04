/*
 * m2s.h
 *
 * (C) Copyright 2012
 * Emcraft Systems, <www.emcraft.com>
 * Alexander Potashev <aspotashev@emcraft.com>
 * Vladimir Khusainov, <vlad@emcraft.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef _MACH_M2S_H_
#define _MACH_M2S_H_

/*
 * SYSREG registers. Refer to Microsemi's SF2 User's Guide for details.
 */
struct m2s_sysreg {
  unsigned int	esram_cr;
  unsigned int	esram_max_lat;
  unsigned int	ddr_cr;
  unsigned int	envm_cr;
  unsigned int	envm_remap_base_cr;
  unsigned int	envm_remap_fab_cr;
  unsigned int	cc_cr;
  unsigned int	cc_region_cr;
  unsigned int	cc_lock_base_addr_cr;
  unsigned int	cc_flush_indx_cr;
  unsigned int	ddrb_buf_timer_cr;
  unsigned int	ddrb_nb_addr_cr;
  unsigned int	ddrb_nb_size_cr;
  unsigned int	ddrb_cr;
  unsigned int	edac_cr;
  unsigned int	master_weight0_cr;
  unsigned int	master_weight_cr;
  unsigned int	soft_irq_cr;
  unsigned int	soft_reset_cr;
  unsigned int	m3_cr;
  unsigned int	fab_if_cr;
  unsigned int	loopback_cr;
  unsigned int	gpio_sysreset_sel_cr;
  unsigned int	gpin_src_sel_cr;
  unsigned int	mddr_cr;
  unsigned int	usb_io_input_sel_cr;
  unsigned int	periph_clk_mux_sel_cr;
  unsigned int	wdog_cr;
  unsigned int	mddr_io_calib_cr;
  unsigned int	reserved1;
  unsigned int	edac_irq_enable_cr;
  unsigned int	usb_cr;
  unsigned int	esram_pipeline_cr; //0x80 
  unsigned int	mss_irq_enable_cr; //0x84
  unsigned int	rtc_wakeup_cr;   //0x88
  unsigned int	mac_cr; //0x8c
  unsigned int	mssddr_pll_status_low_cr; //0x90
  unsigned int	mssddr_pll_status_high_cr; //0x94
  unsigned int	mssddr_facc1_cr;   //0x98
  unsigned int	mssddr_facc2_cr;   //0x9C
  unsigned int  PLL_LOCK_EN_CR;                           /*0XA0  */
  unsigned int  MSSDDR_CLK_CALIB_CR;                      /*0XA4  */
  unsigned int  PLL_DELAY_LINE_SEL_CR;                    /*0XA8  */
  unsigned int  MAC_STAT_CLRONRD_CR;                      /*0XAC  */
  unsigned int  RESET_SOURCE_CR;                          /*0XB0  */
  unsigned int  CC_DC_ERR_ADDR_SR;                        /*0XB4  */
  unsigned int  CC_IC_ERR_ADDR_SR;                        /*0XB8  */
  unsigned int  CC_SB_ERR_ADDR_SR;                        /*0XBC  */
  unsigned int  CC_DECC_ERR_ADDR_SR;                      /*0XC0  */
  unsigned int  CC_IC_MISS_CNT_SR;                        /*0XC4  */
  unsigned int  CC_IC_HIT_CNT_SR;                         /*0XC8  */
  unsigned int  CC_DC_MISS_CNT_SR;                        /*0XCC  */
  unsigned int  CC_DC_HIT_CNT_SR;                         /*0XD0  */
  unsigned int  CC_IC_TRANS_CNT_SR;                       /*0XD4  */
  unsigned int  CC_DC_TRANS_CNT_SR;                       /*0XD8  */
  unsigned int  DDRB_DS_ERR_ADR_SR;                       /*0XDC  */
  unsigned int    DDRB_HPD_ERR_ADR_SR;                      /*0XE0  */
  unsigned int    DDRB_SW_ERR_ADR_SR;                       /*0XE4  */
  unsigned int    DDRB_BUF_EMPTY_SR;                        /*0XE8  */
  unsigned int    DDRB_DSBL_DN_SR;                          /*0XEC  */
  unsigned int    ESRAM0_EDAC_CNT;                          /*0XF0  */
  unsigned int    ESRAM1_EDAC_CNT;                          /*0XF4  */
  unsigned int    CC_EDAC_CNT;                              /*0XF8  */
  unsigned int    MAC_EDAC_TX_CNT;                          /*0XFC  */
  unsigned int    MAC_EDAC_RX_CNT;                          /*0X100 */
  unsigned int    USB_EDAC_CNT;                             /*0X104 */
  unsigned int    CAN_EDAC_CNT;                             /*0X108 */
  unsigned int    ESRAM0_EDAC_ADR;                          /*0X10C */
  unsigned int    ESRAM1_EDAC_ADR;                          /*0X110 */
  unsigned int    MAC_EDAC_RX_ADR;                          /*0X114 */
  unsigned int    MAC_EDAC_TX_ADR;                          /*0X118 */
  unsigned int    CAN_EDAC_ADR;                             /*0X11C */
  unsigned int    USB_EDAC_ADR;                             /*0X120 */
  unsigned int    MM0_1_2_SECURITY;                         /*0X124 */
  unsigned int    MM4_5_FIC64_SECURITY;                     /*0X128 */
  unsigned int    MM3_6_7_8_SECURITY;                       /*0X12C */
  unsigned int    MM9_SECURITY;                             /*0X130 */
  unsigned int    M3_SR;                                    /*0X134 */
  unsigned int    ETM_COUNT_LOW;                            /*0X138 */
  unsigned int    ETM_COUNT_HIGH;                           /*0X13C */
  unsigned int    DEVICE_SR;                                /*0X140 */
  unsigned int    ENVM_PROTECT_USER;                        /*0X144 */
  unsigned int    ENVM_STATUS;                              /*0X148 */
  unsigned int    DEVICE_VERSION;                           /*0X14C */
  unsigned int    mssddr_pll_status;                        /*0X150 */
  unsigned int    USB_SR;                                   /*0X154 */
  unsigned int    ENVM_SR;                                  /*0X158 */
  unsigned int    SPARE_IN;                                 /*0X15C */
  unsigned int    DDRB_STATUS;                              /*0X160 */
  unsigned int    MDDR_IO_CALIB_STATUS;                     /*0X164 */
  unsigned int    MSSDDR_CLK_CALIB_STATUS;                  /*0X168 */
  unsigned int    WDOGLOAD;                                 /*0X16C */
  unsigned int    WDOGMVRP;                                 /*0X170 */
  unsigned int    USERCONFIG0;                              /*0X174 */
  unsigned int    USERCONFIG1;                              /*0X178 */
  unsigned int    USERCONFIG2;                              /*0X17C */
  unsigned int    USERCONFIG3;                              /*0X180 */
  unsigned int    FAB_PROT_SIZE;                            /*0X184 */
  unsigned int    FAB_PROT_BASE;                            /*0X188 */
  unsigned int    MSS_GPIO_DEF;                             /*0X18C */
  unsigned int    EDAC_SR;                                  /*0X190 */
  unsigned int    MSS_INTERNAL_SR;                          /*0X194 */
  unsigned int    MSS_EXTERNAL_SR;                          /*0X198 */
  unsigned int    WDOGTIMEOUTEVENT;                         /*0X19C */
  unsigned int    CLR_MSS_COUNTERS;                         /*0X1A0 */
  unsigned int    CLR_EDAC_COUNTERS;                        /*0X1A4 */
  unsigned int    FLUSH_CR;                                 /*0X1A8 */
  unsigned int    MAC_STAT_CLR_CR;                          /*0X1AC */
  unsigned int    IOMUXCELL_CONFIG[57];                     /*0X1B0 */
  unsigned int    NVM_PROTECT_FACTORY;                      /*0X294 */
  unsigned int    DEVICE_STATUS_FIXED;                      /*0X298 */
  unsigned int    MBIST_ES0;                                /*0X29C */
  unsigned int    MBIST_ES1;                                /*0X2A0 */
  unsigned int    MSDDR_PLL_STAUS_1;                        /*0X2A4 */
  unsigned int    REDUNDANCY_ESRAM0;                        /*0X2A8 */
  unsigned int    REDUNDANCY_ESRAM1;                        /*0X2AC */
  unsigned int    SERDESIF;                                 /*0X2B0 */
  //unsigned int	reserved_A0_150[44]; //0xA0 
  //unsigned int	mssddr_pll_status; //0x150
};

/*
 * SYSREG access handle
 */
#define M2S_SYSREG_BASE			0x40038000
#define M2S_SYSREG			((volatile struct m2s_sysreg *)\
					(M2S_SYSREG_BASE))

struct m2s_coresf2config {
    unsigned int   config_done;
    unsigned int   init_done;
    unsigned int   clr_init_done;
    /*TODO: Add rest of registers to this */
};

#define CORE_SF2_CFG_BASE		0x40022000u
#define CORE_SF2_CFG			\
	((volatile struct m2s_coresf2config *)CORE_SF2_CFG_BASE)

/*
 * Reference clocks enumeration
 */
enum clock {
	CLOCK_SYSREF,
	CLOCK_SYSTICK,
	CLOCK_DDR,
	CLOCK_PCLK0,
	CLOCK_PCLK1,
	CLOCK_FPGA,
	CLOCK_END
};

/*
 * Return a clock value for the specified refernce clock
 * @param clck		reference clock
 * @returns		clock value
 */
extern ulong clock_get(enum clock clck);



#endif /* _MACH_M2S_H_ */
