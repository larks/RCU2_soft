/*
 * Low-level services to access the eNVM of SmartFusion
 *
 * (C) Copyright 2011
 * Vladimir Khusainov, Emcraft Systems, vlad@emcraft.com
 *
 * (C) Copyright 2012
 * Alexander Potashev, Emcraft Systems, aspotashev@emcraft.com
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <asm/delay.h>
#include <mach/regmap.h>

#define SYSREG    ((volatile struct SYSREG_TypeDef *) SYSREG_BASE)
#define NVM_FREQRNG_MASK                0xFFFFE01Fu
#define NVM_FREQRNG_MAX                 ((uint32_t)0xFF << 5u)     /* FREQRNG is set to 15. */

/*
 * The current code is good only for ENVM0.
 * It needs to be updated to work with ENVM1.
 * ...
 * eNVM control & status registers
 */
struct mss_envm {
	unsigned int		reserved_0_to_80[32];
	unsigned int		wdbuff[32];
	unsigned int		reserved_100_to_120[8];
	unsigned int		status;
	unsigned int		reserved_124_to_128[1];
	unsigned int		nv_page_status;
	unsigned int		nv_freq_rng;
	unsigned int		nv_dpd_b;
	unsigned int		nv_ce;
	unsigned int		reserved_138_to_140[2];
	unsigned int		page_lock_set;
	unsigned int		dwsize;
	unsigned int		cmd;
	unsigned int		reserved_14c_to_154[2];
	unsigned int		inten;
	unsigned int		clrhint;
	unsigned int		reserved_15c_to_1fc[40];
	unsigned int		reqaccess;
};

/*
 * eNVM registers access handle
 */
#define MSS_ENVM_REGS_BASE		0x60080000
#define MSS_ENVM			((volatile struct mss_envm *) \
					(MSS_ENVM_REGS_BASE))

/*
 * Base address of the eNVM Flash
 */
#define MSS_ENVM_BASE			0x60000000
#define MSS_ENVM_READ_BASE		0x60000000

/*
 * eNVM page parameters
 */
#define MSS_ENVM_PAGE_SIZE		128

/*
 * Various bit fields
 */
#define MSS_ENVM_REQACCESS_EXCL		0x1
#define MSS_ENVM_REQACCESS_BY_M3	0x5
#define MSS_ENVM_STATUS_READY		0x1
#define MSS_ENVM_CMD_PROGRAM_ADS	(0x08<<24)
#define MSS_ENVM_CMD_PROGRAM_A   	(0x05<<24)
#define MSS_ENVM_CMD_PROGRAM_D	        (0x06<<24)
#define MSS_ENVM_CMD_PROGRAM_S	        (0x07<<24)

/*
 * Initialize the eNVM interface
 */
void __init mss_envm_init(void)
{
  printk(KERN_INFO "ENVM: misc driver for m2s_mss_envm \n");
}

/*
 * Update an eNVM page.
 * @param page		eNVM page
 * @param part_begin	updated area start offset in page
 * @param part_end	updated area end offset in page
 * @param buf		data to write
 * @returns		0 -> success
 */
static int envm_write_page_part(
	unsigned int page, unsigned int part_begin,
	unsigned int part_end, unsigned char *buf)
{

  //printk(KERN_INFO "page=0x%x, part_begin=0x%x, part_end=0x%x \n",
  //page, part_begin, part_end);

  /*
   * If not writing a whole page, save the existing data in this page
   */
  if (part_begin != 0 || part_end != MSS_ENVM_PAGE_SIZE) {
    memcpy((void *) MSS_ENVM->wdbuff,
	   (const void *) ((unsigned char *) MSS_ENVM_BASE +
			   page * MSS_ENVM_PAGE_SIZE),
	   MSS_ENVM_PAGE_SIZE);
  }
  
  /*
   * Copy new data to the Write Buffer
   */
  /*
  memcpy((void *) ((unsigned char *) MSS_ENVM->wdbuff + part_begin),
	 buf, part_end - part_begin);
  */

  int i;
  for(i=0;i<part_end - part_begin;i=i+4){
    MSS_ENVM->wdbuff[i/4] = buf[i+3] << 24 | buf[i+2] << 16 |
      buf[i+1] << 8 | buf[i];
  }


  /*
   * Wait for Ready in Status
   */
  while (!(MSS_ENVM->status & MSS_ENVM_STATUS_READY));
  
  while(1){
    if((MSS_ENVM->status & MSS_ENVM_STATUS_READY)){
      break;
    }
  }

  /*
   * Issue the ProgramADS command (ProgramAd + ProgramDa + ProgramStart)
   */
  MSS_ENVM->cmd = MSS_ENVM_CMD_PROGRAM_ADS | (page * MSS_ENVM_PAGE_SIZE);

  /*
  MSS_ENVM->cmd = MSS_ENVM_CMD_PROGRAM_A | (page * MSS_ENVM_PAGE_SIZE);

  while (!(MSS_ENVM->status & MSS_ENVM_STATUS_READY));
  while (!(MSS_ENVM->status & MSS_ENVM_STATUS_READY));

  MSS_ENVM->cmd = MSS_ENVM_CMD_PROGRAM_D ;


  while (!(MSS_ENVM->status & MSS_ENVM_STATUS_READY));
  while (!(MSS_ENVM->status & MSS_ENVM_STATUS_READY));

  MSS_ENVM->cmd = MSS_ENVM_CMD_PROGRAM_S ;
  */


  /*
   * Wait for Ready in Status
   */
  while (!(MSS_ENVM->status & MSS_ENVM_STATUS_READY));
  while(1){
    if((MSS_ENVM->status & MSS_ENVM_STATUS_READY)){
      break;
    }
  }


  return 0;
}

/*
 * Write a data buffer to eNVM.
 * @param offset	eNVM offset
 * @param buf		data to write
 * @param size		how many bytes to write
 * @returns		number of bytes written
 */
int mss_envm_write(unsigned int offset, void *buf, unsigned int size)
{
  unsigned int page, first_page, last_page;
  
  int g_init_envm_cr = SYSREG->ENVM_CR;
  SYSREG->ENVM_CR = (g_init_envm_cr & NVM_FREQRNG_MASK) | NVM_FREQRNG_MAX;
  
  //printk(KERN_INFO "mss_envm_write at 0x%x size=0x%x, 0x%x\n", offset, size);
  
  /*
   * Open exlusive access to eNVM by Cortex-M3
   */
  MSS_ENVM->reqaccess = MSS_ENVM_REQACCESS_EXCL;
  while (MSS_ENVM->reqaccess != MSS_ENVM_REQACCESS_BY_M3);
  
  
  /*
   * Update eNVM, page by page
   */
  first_page = offset / MSS_ENVM_PAGE_SIZE;
  last_page = (offset + size - 1) / MSS_ENVM_PAGE_SIZE;
  
  for (page = first_page; page <= last_page; page++) {
    envm_write_page_part(page,
			 max(0, (int)offset - (int)page * MSS_ENVM_PAGE_SIZE),
			 min(MSS_ENVM_PAGE_SIZE,
			     (int)offset + (int)size -
			     (int)page * MSS_ENVM_PAGE_SIZE),
			 (unsigned char *)buf +
			 max((int)page * MSS_ENVM_PAGE_SIZE - (int)offset, 0));
  }
  
  /*
   * Disable access to eNVM by Cortex-M3 core
   */
  MSS_ENVM->reqaccess = ~MSS_ENVM_REQACCESS_EXCL;
  
  SYSREG->ENVM_CR = g_init_envm_cr;

  return (int)size;
}

/*
 * Read a data buffer from eNVM
 */
int mss_envm_read(unsigned int offset, void *buf, unsigned int size)
{

  int g_init_envm_cr = SYSREG->ENVM_CR;
  SYSREG->ENVM_CR = (g_init_envm_cr & NVM_FREQRNG_MASK) | NVM_FREQRNG_MAX;

  memcpy(buf, (void *) (MSS_ENVM_READ_BASE + offset), size);
  //printk(KERN_INFO " ENVM: mss_envm_read called...offset=0x%x, size=0x%x\n", MSS_ENVM_READ_BASE+offset, size);


  SYSREG->ENVM_CR = g_init_envm_cr;

  return size;

}
