/************************************************************************
** U-boot board configuration file for the ALICE Time Projection Chamber RCU2
**
** This file is property of and copyright by the Experimental Nuclear 
** Physics Group, Dep. of Physics and Technology
** University of Bergen, Norway, 2014
** This file has been written by Lars Bratrud,
** Lars.Bratrud@cern.ch
**
** Permission to use, copy, modify and distribute this software and its  
** documentation strictly for non-commercial purposes is hereby granted  
** without fee, provided that the above copyright notice appears in all  
** copies and that both the copyright notice and this permission notice  
** appear in the supporting documentation. The authors make no claims    
** about the suitability of this software for any purpose. It is         
** provided "as is" without express or implied warranty.                 
**
*************************************************************************/

#ifndef __CONFIG_H
#define __CONFIG_H

/*
 * Disable debug messages
 */
//#undef DEBUG
#define DEBUG

/*
 * This is an ARM Cortex-M3 CPU core
 */
#define CONFIG_SYS_ARMCORTEXM3

/*
 * This is the Actel SmartFusion2 (aka M2S) device
 */
#define CONFIG_SYS_M2S

/*
 * System frequencies are defined by Libero, with no
 * known way (as of yet) to read them in run time. Hence,
 * we define them as build-time constants
 */
#define CONFIG_SYS_M2S_SYSREF		160000000
//#define CONFIG_SYS_M2S_SYSREF		50000000

/*
 * This is a specific revision of the board
 * Calling first version for revision 0A
 */
#if !defined(CONFIG_SYS_BOARD_REV)
#define CONFIG_SYS_BOARD_REV		0x0A
#endif

//#if (CONFIG_SYS_BOARD_REV!=0x1A) && (CONFIG_SYS_BOARD_REV!=0x2A)
//#error CONFIG_SYS_BOARD_REV must be 1A or 2A
//#endif
/*
#if (CONFIG_SYS_BOARD_REV==0x1A)
# define CONFIG_SYS_BOARD_REV_STR	"1A"
#else
# define CONFIG_SYS_BOARD_REV_STR	"2A"
#endif
*/
#define CONFIG_SYS_BOARD_REV_STR	"0A"
/*
 * Display CPU and Board information
 */
#define CONFIG_DISPLAY_CPUINFO		1
#define CONFIG_DISPLAY_BOARDINFO	1

/*
 * Monitor prompt
 */
#define CONFIG_SYS_PROMPT		"RCU-2.0 Uboot> "

/*
 * We want to call the CPU specific initialization
 */
#define CONFIG_ARCH_CPU_INIT

/*
 * This ensures that the SoC-specific cortex_m3_soc_init() gets invoked.
 */
#define CONFIG_ARMCORTEXM3_SOC_INIT

/*
 * Number of clock ticks in 1 sec
 */
#define CONFIG_SYS_HZ			1000

/*
 * Enable/disable h/w watchdog
 */
#undef CONFIG_HW_WATCHDOG

/*
 * No interrupts
 */
#undef CONFIG_USE_IRQ

/*
 * Cache configuration
 */
#define CONFIG_M2S_CACHE_ON

/*
 * Memory layout configuration
 */
#define CONFIG_MEM_NVM_BASE		0x00000000
#define CONFIG_MEM_NVM_LEN		(256 * 1024)

#if 0
/*
 * Define the constant below to build U-boot for running
 * from offset 0x20000 (128KB) in eNVM. When built this way,
 * use the following commands to test the newly built U-boot:
 * - tftp u-boot.bin
 * - cptf 20000 ${loadaddr} ${filesize} 0
 * - go 20375 (or check address of _start in u-boot.map)
 */
#define CONFIG_MEM_NVM_UBOOT_OFF	(128 * 1024)
#endif

#define CONFIG_MEM_RAM_BASE		0x20000000
#define CONFIG_MEM_RAM_LEN		(32 * 1024)
#define CONFIG_MEM_RAM_BUF_LEN		(16 * 1024)
#define CONFIG_MEM_MALLOC_LEN		(12 * 1024)
#define CONFIG_MEM_STACK_LEN		(4 * 1024)

/*
 * malloc() pool size
 */
#define CONFIG_SYS_MALLOC_LEN		CONFIG_MEM_MALLOC_LEN

/*
 * Configuration of the external memory
 */
#define CONFIG_NR_DRAM_BANKS		1
#define CONFIG_SYS_RAM_BASE		0xA0000000
#define CONFIG_SYS_RAM_BASE_2		0xB0000000
#define CONFIG_SYS_RAM_SIZE		(256 * 1024 * 1024)

/*
 * Configuration of the external Flash
 * No NOR Flash
 */
#define CONFIG_SYS_NO_FLASH

/*
 * Configure the SPI contoler device driver
 * FIFO Size is 64K, but leave 5 bytes for cmd[] + addr[]
 */
#define CONFIG_M2S_SPI			1
#define CONFIG_SPI_MAX_XF_LEN		65530

/*
 * Configure SPI Flash
 */

#define CONFIG_SPI_FLASH		1
#define CONFIG_SPI_FLASH_SPANSION	1
#define CONFIG_SPI_FLASH_BUS		0
#define CONFIG_SPI_FLASH_CS		0
#define CONFIG_SPI_FLASH_MODE		3
//#define CONFIG_SYS_FLASH_CFI_WIDTH	FLASH_CFI_16BIT /* Could this fix the bit shift issue? No? */
#define CONFIG_SPI_FLASH_SPEED		(CONFIG_SYS_M2S_SYSREF / 32) //32
//#define CONFIG_SPI_FLASH_SPEED		(CONFIG_SYS_M2S_SYSREF / 8) //32
#define CONFIG_SF_DEFAULT_SPEED		CONFIG_SPI_FLASH_SPEED
#define CONFIG_SF_DEFAULT_MODE		CONFIG_SPI_FLASH_MODE

#define CONFIG_USE_SPI_FLASH_2          1

/*
 * U-boot environment configuration
 */
#define CONFIG_ENV_IS_IN_SPI_FLASH	1
#define CONFIG_ENV_SECT_SIZE		0x1000
#define CONFIG_ENV_SIZE			CONFIG_ENV_SECT_SIZE
#define CONFIG_ENV_OFFSET		0x0
#define CONFIG_ENV_SPI_BUS		CONFIG_SPI_FLASH_BUS
#define CONFIG_ENV_SPI_CS		CONFIG_SPI_FLASH_CS
#define CONFIG_ENV_SPI_MAX_HZ		CONFIG_SPI_FLASH_SPEED
#define CONFIG_ENV_SPI_MODE		CONFIG_SPI_FLASH_MODE

#define CONFIG_INFERNO			1
#define CONFIG_ENV_OVERWRITE		1

#define BOARD_LATE_INIT                 1

/*
 * Location of kernel image and Co in SPI.
 * Linux MTD driver has no boot sectors support, so locate kernel
 * with 64K alignment
 */
#define CONFIG_ENV_IMG_OFFSET		0x10000

/*
 * Serial console configuration: MSS UART1
 */
#define CONFIG_SYS_NS16550		1
#undef CONFIG_NS16550_MIN_FUNCTIONS
#define CONFIG_SYS_NS16550_SERIAL	1
#define CONFIG_SYS_NS16550_REG_SIZE	(-4)
#define CONFIG_SYS_NS16550_CLK		clock_get(CLOCK_PCLK0)
#define CONFIG_CONS_INDEX		1
#define CONFIG_SYS_NS16550_COM1		0x40000000
#define CONFIG_BAUDRATE			115200 
#define CONFIG_SYS_BAUDRATE_TABLE	{ 9600, 19200, 38400, 57600, 115200 }

/*
 * Console I/O buffer size
 */
#define CONFIG_SYS_CBSIZE		256

/*
 * Print buffer size
 */
#define CONFIG_SYS_PBSIZE		(CONFIG_SYS_CBSIZE + \
					sizeof(CONFIG_SYS_PROMPT) + 16)

/*
 * Ethernet driver configuration
 */
#define CONFIG_NET_MULTI
#define CONFIG_M2S_ETH
#define CONFIG_M2S_ETH_MODE_SGMII

#define CONFIG_SYS_RX_ETH_BUFFER	2

/*
 * Use standard MII PHY API
 */
#define CONFIG_MII
#define CONFIG_SYS_FAULT_ECHO_LINK_DOWN

#define CONFIG_SYS_ALT_MEMTEST          1
#define CONFIG_SYS_MEMTEST_START	CONFIG_SYS_RAM_BASE
#define CONFIG_SYS_MEMTEST_END		(CONFIG_SYS_RAM_BASE + \
					CONFIG_SYS_RAM_SIZE)

/*
 * Needed by "loadb"
 */
#define CONFIG_SYS_LOAD_ADDR		CONFIG_SYS_RAM_BASE

/*
 * Monitor is actually in eNVM. In terms of U-Boot, it is neither "flash",
 * not RAM, but CONFIG_SYS_MONITOR_BASE must be defined.
 */
#define CONFIG_SYS_MONITOR_BASE		0x0

/*
 * Monitor is not in flash. Needs to define this to prevent
 * U-Boot from running flash_protect() on the monitor code.
 */
#define CONFIG_MONITOR_IS_IN_RAM	1

/*
 * I2C interface for EEPROM DIP SW
 */
//#define CONFIG_M2S_I2C 1
//#define CONFIG_SOFT_I2C 1
//#define CONFIG_SYS_I2C_SPEED   400000
//#define CONFIG_SYS_I2C_SLAVE   0x4e

/*
 * Enable all those monitor commands that are needed
 */
#include <config_cmd_default.h>
#undef CONFIG_CMD_BOOTD
#undef CONFIG_CMD_CONSOLE
#undef CONFIG_CMD_ECHO
#undef CONFIG_CMD_EDITENV
#undef CONFIG_CMD_FPGA
#undef CONFIG_CMD_IMI
#undef CONFIG_CMD_ITEST
#undef CONFIG_CMD_IMLS
#define CONFIG_CMD_LOADS
#undef CONFIG_CMD_MISC
#define CONFIG_CMD_NET
#define CONFIG_CMD_DHCP
#define CONFIG_CMD_ISP
//#define CONFIG_CMD_M2SI2C
#undef CONFIG_CMD_NFS
#undef CONFIG_CMD_SOURCE
#undef CONFIG_CMD_XIMG
#undef CONFIG_CMD_SOMTEST
#if defined(CONFIG_SPI_FLASH)
#define CONFIG_CMD_SF
#endif

/*
 * To save memory disable long help
 */
#undef CONFIG_SYS_LONGHELP

/*
 * Max number of command args
 */
#define CONFIG_SYS_MAXARGS		16

/*
 * Auto-boot sequence configuration
 */
#define CONFIG_BOOTDELAY		3
#define CONFIG_ZERO_BOOTDELAY_CHECK
/* #define CONFIG_HOSTNAME			m2s-som */
#define CONFIG_HOSTNAME	rcu-2.0
/*
#define CONFIG_BOOTARGS			"m2s_platform=m2s-som "\
					"console=ttyS0,115200 panic=10"
*/
#define CONFIG_BOOTARGS	"m2s_platform=rcu-2.0 "\
					"console=ttyS0,115200 panic=10 user_debug=31"
#define CONFIG_BOOTCOMMAND		"run netboot"

/*
 * Macro for the "loadaddr". The most optimal load address
 * for the non-compressed uImage is the kernel link address
 * (CONFIG_SYS_RAM_BASE + 0x8000) minus sizeof uImage header (0x40),
 * so the kernel start address would be loaded just to the right
 * place.
 */
#define UIMAGE_LOADADDR			0xA0007FC0

#define CONFIG_IPADDR                  10.1.5.120
#define CONFIG_ETHADDR                 00:50:C2:DB:2C:9F
#define CONFIG_SERVERIP1               10.160.33.95
#define CONFIG_SERVERIP2               10.160.33.96
//#define CONFIG_SERVERIP1               10.10.10.10
//#define CONFIG_SERVERIP2               10.10.10.10

/*
 * Short-cuts to some useful commands (macros)
 */
#define CONFIG_EXTRA_ENV_SETTINGS				\
	"loadaddr=" MK_STR(UIMAGE_LOADADDR) "\0"		\
	"serverip=10.160.33.95\0"				\
	"image=rculinux.uImage\0"				\
	"spiaddr=" MK_STR(CONFIG_ENV_IMG_OFFSET) "\0"		\
	"spisize=400000\0"					\
	"spiprobe=sf probe " MK_STR(CONFIG_SPI_FLASH_BUS) "\0"	\
	"addip=setenv bootargs ${bootargs}" 			\
	" ip=${ipaddr}:${serverip}:${gatewayip}:"		\
	"${netmask}:${hostname}:eth0:off\0" 			\
        "linux_spi0=wrlv 0; loadenv 0; run addip; sf probe 0; sf read ${loadaddr} ${spiaddr} ${spisize}; bootm ${loadaddr}\0"             \
	"linux_spi1=wrlv 0; loadenv 1; run addip; sf probe 1; sf read ${loadaddr} ${spiaddr} ${spisize}; bootm ${loadaddr}\0"	          \
	"linux_net=wrlv 1; tftp ${loadaddr} ${image};run addip;bootm\0"           \							  
	"flashboot=run memtest; run linux_spi0; run linux_net; run linux_spi1; run linux_net; loadenv 2; run linux_net\0"			\
	"netboot=run memtest; loadenv 0; run linux_net; loadenv 1; run linux_net; loadenv 2; run linux_net; run linux_spi0; run linux_spi1\0"	\
  	"memtest=mwr 0xA0000000 0xB0000000 0\0"	\
        "ISP_PROG_ADDR=0x410000\0"	\
        "ISP_PROG_SIZE=2418896\0" 	\
        "DDL2_CONFIG_SPEED=2\0" 	\
        "ispprog=isp apv\0"	\
        "ispauth=isp a; reset\0"	\
	"update=tftp ${loadaddr} ${image};run spiprobe;"	\
	"sf erase ${spiaddr} ${spisize};"			\
	"sf write ${loadaddr} ${spiaddr} ${filesize}\0"        \
        "stdin=serial\0"  \
        "stdout=serial\0"  \
        "stderr=serial\0"  \
        "ethact=M2S_MAC\0"      


/*
 * Linux kernel boot parameters configuration
 */
#define CONFIG_SETUP_MEMORY_TAGS
#define CONFIG_CMDLINE_TAG 1

#endif /* __CONFIG_H */
