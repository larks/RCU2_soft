/*
 * linux/arch/arm/mach-m2s/spi.c
 *
 * Copyright (C) 2012 Yuri Tikhonov, Emcraft Systems
 * Copyright (C) 2013 Vladimir Khusainov, Emcraft Systems
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

#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/sysdev.h>
#include <linux/io.h>
#include <linux/mtd/physmap.h>
#include <linux/mtd/mtd.h>
#include <linux/mtd/partitions.h>
#include <linux/spi/spi.h>
#include <linux/spi/flash.h>
#include <mach/m2s.h>
#include <mach/platform.h>
#include <mach/clock.h>
#include <mach/spi.h>

/*
 * MSS PDMA is common for both SPI ports. So, actually, both SPI ports
 * at once couldn't work (PDMA resource will be busy). Should implement
 * a separate PDMA driver to fix this.
 */
#define PDMA_M2S_REGS		0x40003000
#define PDMA_M2S_IRQ		13

#define SPI0_M2S_ID		0
#define SPI0_M2S_REGS		0x40001000
#define SPI0_M2S_CLK		CLCK_PCLK0
#define SPI0_M2S_RX_DMA		0
#define SPI0_M2S_TX_DMA		1

#define SPI1_M2S_ID		1
#define SPI1_M2S_REGS		0x40011000
#define SPI1_M2S_CLK		CLCK_PCLK1
#define SPI1_M2S_RX_DMA		2
#define SPI1_M2S_TX_DMA		3

#if defined(CONFIG_M2S_MSS_SPI0)
static struct resource spi_m2s_dev0_resources[] = {
	{
		.start	= SPI0_M2S_REGS,
		.end	= SPI0_M2S_REGS + 1,
		.flags	= IORESOURCE_MEM,
	}, {
		.start	= PDMA_M2S_REGS,
		.end	= PDMA_M2S_REGS + 1,
		.flags	= IORESOURCE_MEM,
	}, {
               .start  = PDMA_M2S_IRQ,
               .flags  = IORESOURCE_IRQ,
	},
};

static struct spi_m2s_platform_data spi_m2s_dev0_data = {
	.dma_rx		= SPI0_M2S_RX_DMA,
	.dma_tx		= SPI0_M2S_TX_DMA,
};

static struct platform_device spi_m2s_dev0 = {
	.name           = "spi_m2s",
	.id             = SPI0_M2S_ID,
	.num_resources  = ARRAY_SIZE(spi_m2s_dev0_resources),
	.resource       = spi_m2s_dev0_resources,
};
#endif	/* CONFIG_M2S_MSS_SPI0 */

#if defined(CONFIG_M2S_MSS_SPI1)
static struct resource spi_m2s_dev1_resources[] = {
	{
		.start	= SPI1_M2S_REGS,
		.end	= SPI1_M2S_REGS + 1,
		.flags	= IORESOURCE_MEM,
	}, {
		.start	= PDMA_M2S_REGS,
		.end	= PDMA_M2S_REGS + 1,
		.flags	= IORESOURCE_MEM,
	}, {
		.start	= PDMA_M2S_IRQ,
		.flags	= IORESOURCE_IRQ,
	},
};

static struct spi_m2s_platform_data spi_m2s_dev1_data = {
	.dma_rx		= SPI1_M2S_RX_DMA,
	.dma_tx		= SPI1_M2S_TX_DMA,
};

static struct platform_device spi_m2s_dev1 = {
	.name		= "spi_m2s",
	.id		= SPI1_M2S_ID,
	.num_resources	= ARRAY_SIZE(spi_m2s_dev1_resources),
	.resource	= spi_m2s_dev1_resources,
};
#endif  /* CONFIG_M2S_MSS_SPI1 */

/*
 * Register the M2S specific SPI controllers and devices with the kernel.
 */
void __init m2s_spi_init(void)
{
	int	p = m2s_platform_get();

	/*
	 * Register platform device for SPI0 controller
	 */
#if defined(CONFIG_M2S_MSS_SPI0)
	spi_m2s_dev0_data.ref_clk = m2s_clock_get(SPI0_M2S_CLK);
	platform_set_drvdata(&spi_m2s_dev0, &spi_m2s_dev0_data);
	platform_device_register(&spi_m2s_dev0);
#endif

	/*
	 * Register platform device for SPI1 controller
	 */
#if defined(CONFIG_M2S_MSS_SPI1)
	spi_m2s_dev1_data.ref_clk = m2s_clock_get(SPI1_M2S_CLK);
	platform_set_drvdata(&spi_m2s_dev1, &spi_m2s_dev1_data);
	platform_device_register(&spi_m2s_dev1);
#endif

	/*
	 * Define SPI slave data structures for all connected SPI devices
	 */
#if defined(CONFIG_M2S_MSS_SPI0) || defined(CONFIG_M2S_MSS_SPI1)

	//if (p == PLATFORM_M2S_SOM || p == PLATFORM_M2S_FG484_SOM) {
	if (1){

#if defined(CONFIG_M2S_MSS_SPI0) && defined(CONFIG_MTD_M25P80)
		/*
		 * SPI Flash partitioning for on-module SPI Flash (SPI0):
		 * 0-1ffff:		U-boot environment
		 * 20000-3fffff:	Linux bootable image
		 * 400000-end of Flash:	JFFS2 filesystem
		 */
#		define M2S_SOM_SF_MTD_OFFSET		0x010000 /* 64 KB */
#		define M2S_SOM_SF_MTD_SIZE0		0x400000 /*  4 MB */
#		define M2S_SOM_SF_MTD_SIZE1		0xBF0000 /*~12 MB */
		static struct mtd_partition m2s_som_sf_mtd[] = {
			{
				.name = "spi_flash_uboot_env",
				.offset = 0,
				.size = M2S_SOM_SF_MTD_OFFSET,
			}, {
				.name = "spi_flash_linux_image",
				.offset = M2S_SOM_SF_MTD_OFFSET,
				.size = M2S_SOM_SF_MTD_SIZE0,
			}, {
				.name = "spi_flash_jffs2",
				.offset = M2S_SOM_SF_MTD_OFFSET +
					  M2S_SOM_SF_MTD_SIZE0,
				.size = M2S_SOM_SF_MTD_SIZE1,
			},
		};

		static struct flash_platform_data m2s_som_sf_data = {
			.name = "s25fl129p1",
			.parts = m2s_som_sf_mtd,
			.nr_parts = ARRAY_SIZE(m2s_som_sf_mtd),
			.type = "s25fl129p1",
		};

#endif

#if defined(CONFIG_M2S_MSS_SPI1) && defined(CONFIG_MTD_M25P80)

		/*
		 * SPI Flash partitioning for
		 * the first on-dongle SPI Flash (SPI1, CS0):
		 */
#		define FLASH_PART1_OFFSET__DONGLE1	(1024 * 1024 * 1)
#		define FLASH_SIZE__DONGLE1		(1024 * 1024 * 4)
		static struct mtd_partition
			spi_flash_partitions__dongle1[] = {
			{
				.name = "dongle1_part0",
				.size = FLASH_PART1_OFFSET__DONGLE1,
				.offset = 0,
			},
			{
				.name = "dongle1_part1",
				.size = FLASH_SIZE__DONGLE1 -
					FLASH_PART1_OFFSET__DONGLE1,
				.offset = FLASH_PART1_OFFSET__DONGLE1,
			},
		};

		/*
		 * SPI Flash data for the first on-dongle Flash
		 */
		static struct flash_platform_data
			spi_flash_data__dongle1 = {
			.name = "m25p32",
			.parts =  spi_flash_partitions__dongle1,
			.nr_parts =
			ARRAY_SIZE(spi_flash_partitions__dongle1),
			.type = "m25p32",
		};
#endif

#if defined(CONFIG_M2S_MSS_SPI1) && \
	(defined(CONFIG_MTD_M25P80) || defined(CONFIG_SPI_SPIDEV))

#if !defined(CONFIG_SPI_SPIDEV)

		/*
		 * SPI Flash partitioning for
		 * the second on-dongle SPI Flash (SPI1, CS1):
		 */
#		define FLASH_PART1_OFFSET__DONGLE2	(1024 * 1024 * 1)
#		define FLASH_SIZE__DONGLE2		(1024 * 1024 * 4)
		static struct mtd_partition
			spi_flash_partitions__dongle2[] = {
			{
				.name = "dongle2_part0",
				.size = FLASH_PART1_OFFSET__DONGLE2,
				.offset = 0,
			},
			{
				.name = "dongle2_part1",
				.size = FLASH_SIZE__DONGLE2 -
					FLASH_PART1_OFFSET__DONGLE2,
				.offset = FLASH_PART1_OFFSET__DONGLE2,
			},
		};

		/*
		 * SPI Flash data for the second on-dongle Flash
		 */
		static struct flash_platform_data
			spi_flash_data__dongle2 = {
			.name = "m25p32",
			.parts =  spi_flash_partitions__dongle2,
			.nr_parts =
			ARRAY_SIZE(spi_flash_partitions__dongle2),
			.type = "m25p32",
		};
#endif
#endif

		/*
		 * Array of registered SPI slaves
		 */
		static struct spi_board_info m2s_som_spi_board_info[] = {

#if defined(CONFIG_M2S_MSS_SPI0) && defined(CONFIG_MTD_M25P80)
		/*
		 * On-module SPI Flash (resides at SPI0,CS0)
		 */
		{
#if defined(CONFIG_SPI_SPIDEV)
		  .modalias = "spidev",
#else
		  .modalias = "m25p32",
#endif
			.max_speed_hz = 160000000/32,
			.bus_num = 0,
			.chip_select = 0,
			.platform_data = &m2s_som_sf_data,
			.mode = SPI_MODE_3,
		},
#endif

		/* added one more SPI Flash on the RCU2 by T. G */
#if defined(CONFIG_M2S_MSS_SPI0) && defined(CONFIG_MTD_M25P80)
		/*
		 * On-module SPI Flash (resides at SPI0,CS0)
		 */
		{
#if defined(CONFIG_SPI_SPIDEV)
		  .modalias = "spidev",
#else
		  .modalias = "m25p32",
#endif
			.max_speed_hz = 160000000/32,
			.bus_num = 0,
			.chip_select = 1,
			.platform_data = &m2s_som_sf_data,
			.mode = SPI_MODE_3,
		},
#endif


#if defined(CONFIG_M2S_MSS_SPI1) && defined(CONFIG_MTD_M25P80)
		/*
		 * On-dongle SPI Flash (resides at SPI1,CS0)
		 */
		{
			.modalias = "m25p32",
			.platform_data = &spi_flash_data__dongle1,
			.max_speed_hz = 25000000,
			.bus_num = 1,
			.chip_select = 0,
			.mode = SPI_MODE_3,
		},
#endif

#if defined(CONFIG_M2S_MSS_SPI1) && defined(CONFIG_MTD_M25P80)
		/*
		 * On-dongle SPI Flash (resides at SPI1,CS1)
		 */
		{
			/*
			 * Can be accessed either as an MTD Flash
			 * or using the SPI user-space interface
			 */
#if defined(CONFIG_SPI_SPIDEV)
			.modalias = "spidev",
#else
			.modalias = "m25p32",
			.platform_data = &spi_flash_data__dongle2,
#endif
			.max_speed_hz = 25000000,
			.bus_num = 1,
			.chip_select = 1,
			.mode = SPI_MODE_3,
		},
#endif
		};

		/*
		 * Register the SPI slaves with the SPI stack
		 */
		spi_register_board_info(m2s_som_spi_board_info,
			sizeof(m2s_som_spi_board_info) /
			sizeof(struct spi_board_info));

		//	} else if (p == PLATFORM_SF2_DEV_KIT) {
	} else{
#if defined(CONFIG_M2S_MSS_SPI0) && defined(CONFIG_MTD_M25P80)
		/*
		 * SPI Flash partitioning:
		 * 0-ffff:		U-boot environment
		 * 10000-3fffff:	Linux bootable image
		 * 400000-end of Flash:	JFFS2 filesystem
		 */
#		define SF2_DEV_KIT_SF_MTD_OFFSET	0x010000 /* 64 KB */
#		define SF2_DEV_KIT_SF_MTD_SIZE0		0x3F0000 /* ~4 MB */
#		define SF2_DEV_KIT_SF_MTD_SIZE1		0x400000 /*  4 MB */
		static struct mtd_partition sf2_dev_kit_sf_mtd[] = {
			{
				.name = "spi_flash_uboot_env",
				.offset = 0,
				.size = SF2_DEV_KIT_SF_MTD_OFFSET,
			}, {
				.name = "spi_flash_linux_image",
				.offset = SF2_DEV_KIT_SF_MTD_OFFSET,
				.size = SF2_DEV_KIT_SF_MTD_SIZE0,
			}, {
				.name = "spi_flash_jffs2",
				.offset = SF2_DEV_KIT_SF_MTD_OFFSET +
					  SF2_DEV_KIT_SF_MTD_SIZE0,
				.size = SF2_DEV_KIT_SF_MTD_SIZE1,
			},
		};

		static struct flash_platform_data sf2_dev_kit_sf_data = {
			.name = "at25df641",
			.parts = sf2_dev_kit_sf_mtd,
			.nr_parts = ARRAY_SIZE(sf2_dev_kit_sf_mtd),
			.type = "at25df641",
		};

		static struct spi_board_info sf2_dev_kit_sf_inf = {
			.modalias = "m25p32",
			.max_speed_hz = 166000000/4,
			.bus_num = 0,
			.chip_select = 0,
			.platform_data = &sf2_dev_kit_sf_data,
			.mode = SPI_MODE_3,
		};

		spi_register_board_info(&sf2_dev_kit_sf_inf, 1);
#endif
	}

#endif
}
