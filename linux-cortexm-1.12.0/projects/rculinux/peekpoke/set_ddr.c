#include<unistd.h>
#include<sys/types.h>
#include<sys/mman.h>
#include<stdio.h>
#include<fcntl.h>

/*
 * MODE_CR bits
 */
#define REG_DDRC_MOBILE			7
#define REG_DDRC_SDRAM			6
#define REG_DDRC_DATA_BUS_WIDTH		0

/*
 * DYN_REFRESH_1_CR bits
 */
#define REG_DDRC_T_RFC_MIN		7
#define REG_DDRC_SELFREF_EN		5
#define REG_DDRC_REFRESH_TO_X32		0

/*
 * DYN_REFRESH_2_CR bits
 */
#define REG_DDRC_T_RFC_NOM_X32		3
#define REG_DDRC_REFRESH_BURST		0


/*
 * DYN_POWERDOWN_CR bits
 */
#define REG_DDRC_POWERDOWN_EN		1

/*
 * CKE_RSTN_CYCLES_1_CR bits
 */
#define REG_DDRC_PRE_CKE_X1024		8

/*
 * REG_DDRC_POST_CKE_X1024 bits
 */
#define REG_DDRC_POST_CKE_X1024		3

/*
 * DRAM_BANK_ACT_TIMING_CR bits
 */
#define REG_DDRC_T_RCD			10
#define REG_DDRC_T_CCD			7
#define REG_DDRC_T_RRD			4
#define REG_DDRC_T_RP			0

/*
 * DRAM_BANK_TIMING_PARAM_CR bits
 */
#define REG_DDRC_T_RC			6

/*
 * DRAM_RD_WR_LATENCY_CR bits
 */
#define REG_DDRC_WRITE_LATENCY		5
#define REG_DDRC_READ_LATENCY		0

/*
 * DRAM_MR_TIMING_PARAM_CR bits
 */
#define REG_DDRC_T_MRD			0

/*
 * DRAM_RAS_TIMING_CR bits
 */
#define REG_DDRC_T_RAS_MAX		5
#define REG_DDRC_T_RAS_MIN		0

/*
 * DRAM_RD_WR_TRNARND_TIME_CR bits
 */
#define REG_DDRC_RD2WR			5
#define REG_DDRC_WR2RD			0

/*
 * DFI_RDDATA_EN_CR bits
 */
#define REG_DDRC_DFI_T_RDDATA_EN	0

/*
 * DRAM_RD_WR_PRE_CR bits
 */
#define REG_DDRC_WR2PRE			5
#define REG_DDRC_RD2PRE			0

/*
 * DRAM_T_PD_CR bits
 */
#define REG_DDRC_T_XP			4
#define REG_DDRC_T_CKE			0

/*
 * PERF_PARAM_1_CR bits
 */
#define REG_DDRC_BURST_RDWR		13
#define REG_DDRC_PAGECLOSE		0x10

/*
 * PERF_PARAM_2_CR bits
 */
#define REG_DDRC_BURST_MODE		10

/*
 * DDRC_PWR_SAVE_1_CR bits
 */
#define REG_DDRC_POWERDOWN_TO_X32_SHIFT		1
#define REG_DDRC_POST_SELFREF_GAP_X32_SHIFT	6

/*
 * DDR Configuration registers
 */
struct ddr_regs {
	/*
	 * DDR Controller registers.
	 */
	struct {
	  int	DYN_SOFT_RESET_CR;
	  int	RESERVED0;
	  int	DYN_REFRESH_1_CR;
	  int	DYN_REFRESH_2_CR;
	  int	DYN_POWERDOWN_CR;
	  int	DYN_DEBUG_CR;
		int	MODE_CR;
		int	ADDR_MAP_BANK_CR;
		int	ECC_DATA_MASK_CR;
		int	ADDR_MAP_COL_1_CR;
		int	ADDR_MAP_COL_2_CR;
		int	ADDR_MAP_ROW_1_CR;
		int	ADDR_MAP_ROW_2_CR;
		int	INIT_1_CR;
		int	CKE_RSTN_CYCLES_1_CR;
		int	CKE_RSTN_CYCLES_2_CR;
		int	INIT_MR_CR;
		int	INIT_EMR_CR;
		int	INIT_EMR2_CR;
		int	INIT_EMR3_CR;
		int	DRAM_BANK_TIMING_PARAM_CR;
		int	DRAM_RD_WR_LATENCY_CR;
		int	DRAM_RD_WR_PRE_CR;
		int	DRAM_MR_TIMING_PARAM_CR;
		int	DRAM_RAS_TIMING_CR;
		int	DRAM_RD_WR_TRNARND_TIME_CR;
		int	DRAM_T_PD_CR;
		int	DRAM_BANK_ACT_TIMING_CR;
		int	ODT_PARAM_1_CR;
		int	ODT_PARAM_2_CR;
		int	ADDR_MAP_COL_3_CR;
		int	MODE_REG_RD_WR_CR;
		int	MODE_REG_DATA_CR;
		int	PWR_SAVE_1_CR;
		int	PWR_SAVE_2_CR;
		int	ZQ_LONG_TIME_CR;
		int	ZQ_SHORT_TIME_CR;
		int	ZQ_SHORT_INT_REFRESH_MARGIN_1_CR;
		int	ZQ_SHORT_INT_REFRESH_MARGIN_2_CR;
		int	PERF_PARAM_1_CR;
		int	HPR_QUEUE_PARAM_1_CR;
		int	HPR_QUEUE_PARAM_2_CR;
		int	LPR_QUEUE_PARAM_1_CR;
		int	LPR_QUEUE_PARAM_2_CR;
		int	WR_QUEUE_PARAM_CR;
		int	PERF_PARAM_2_CR;
		int	PERF_PARAM_3_CR;
		int	DFI_RDDATA_EN_CR;
		int	DFI_MIN_CTRLUPD_TIMING_CR;
		int	DFI_MAX_CTRLUPD_TIMING_CR;
		int	DFI_WR_LVL_CONTROL_1_CR;
		int	DFI_WR_LVL_CONTROL_2_CR;
		int	DFI_RD_LVL_CONTROL_1_CR;
		int	DFI_RD_LVL_CONTROL_2_CR;
		int	DFI_CTRLUPD_TIME_INTERVAL_CR;
		int	DYN_SOFT_RESET_CR2;
		int	AXI_FABRIC_PRI_ID_CR;
		int	SR;
		int	SINGLE_ERR_CNT_STATUS_SR;
		int	DOUBLE_ERR_CNT_STATUS_SR;
		int	LUE_SYNDROME_1_SR;
		int	LUE_SYNDROME_2_SR;
		int	LUE_SYNDROME_3_SR;
		int	LUE_SYNDROME_4_SR;
		int	LUE_SYNDROME_5_SR;
		int	LUE_ADDRESS_1_SR;
		int	LUE_ADDRESS_2_SR;
		int	LCE_SYNDROME_1_SR;
		int	LCE_SYNDROME_2_SR;
		int	LCE_SYNDROME_3_SR;
		int	LCE_SYNDROME_4_SR;
		int	LCE_SYNDROME_5_SR;
		int	LCE_ADDRESS_1_SR;
		int	LCE_ADDRESS_2_SR;
		int	LCB_NUMBER_SR;
		int	LCB_MASK_1_SR;
		int	LCB_MASK_2_SR;
		int	LCB_MASK_3_SR;
		int	LCB_MASK_4_SR;
		int	ECC_INT_SR;
		int	ECC_INT_CLR_REG;
	} ddrc;

	int	reserved0[(0x200 - 0x144) >> 2];

	/*
	 * DDR PHY configuration registers
	 */
	struct {
		int	DYN_BIST_TEST_CR;
		int	DYN_BIST_TEST_ERRCLR_1_CR;
		int	DYN_BIST_TEST_ERRCLR_2_CR;
		int	DYN_BIST_TEST_ERRCLR_3_CR;
		int	BIST_TEST_SHIFT_PATTERN_1_CR;
		int	BIST_TEST_SHIFT_PATTERN_2_CR;
		int	BIST_TEST_SHIFT_PATTERN_3_CR;
		int	DYN_LOOPBACK_TEST_CR;
		int	BOARD_LOOPBACK_CR;
		int	CTRL_SLAVE_RATIO_CR;
		int	CTRL_SLAVE_FORCE_CR;
		int	CTRL_SLAVE_DELAY_CR;
		int	DATA_SLICE_IN_USE_CR;
		int	LVL_NUM_OF_DQ0_CR;
		int	DQ_OFFSET_1_CR;
		int	DQ_OFFSET_2_CR;
		int	DQ_OFFSET_3_CR;
		int	DIS_CALIB_RST_CR;
		int	DLL_LOCK_DIFF_CR;
		int	FIFO_WE_IN_DELAY_1_CR;
		int	FIFO_WE_IN_DELAY_2_CR;
		int	FIFO_WE_IN_DELAY_3_CR;
		int	FIFO_WE_IN_FORCE_CR;
		int	FIFO_WE_SLAVE_RATIO_1_CR;
		int	FIFO_WE_SLAVE_RATIO_2_CR;
		int	FIFO_WE_SLAVE_RATIO_3_CR;
		int	FIFO_WE_SLAVE_RATIO_4_CR;
		int	GATELVL_INIT_MODE_CR;
		int	GATELVL_INIT_RATIO_1_CR;
		int	GATELVL_INIT_RATIO_2_CR;
		int	GATELVL_INIT_RATIO_3_CR;
		int	GATELVL_INIT_RATIO_4_CR;
		int	LOCAL_ODT_CR;
		int	INVERT_CLKOUT_CR;
		int	RD_DQS_SLAVE_DELAY_1_CR;
		int	RD_DQS_SLAVE_DELAY_2_CR;
		int	RD_DQS_SLAVE_DELAY_3_CR;
		int	RD_DQS_SLAVE_FORCE_CR;
		int	RD_DQS_SLAVE_RATIO_1_CR;
		int	RD_DQS_SLAVE_RATIO_2_CR;
		int	RD_DQS_SLAVE_RATIO_3_CR;
		int	RD_DQS_SLAVE_RATIO_4_CR;
		int	WR_DQS_SLAVE_DELAY_1_CR;
		int	WR_DQS_SLAVE_DELAY_2_CR;
		int	WR_DQS_SLAVE_DELAY_3_CR;
		int	WR_DQS_SLAVE_FORCE_CR;
		int	WR_DQS_SLAVE_RATIO_1_CR;
		int	WR_DQS_SLAVE_RATIO_2_CR;
		int	WR_DQS_SLAVE_RATIO_3_CR;
		int	WR_DQS_SLAVE_RATIO_4_CR;
		int	WR_DATA_SLAVE_DELAY_1_CR;
		int	WR_DATA_SLAVE_DELAY_2_CR;
		int	WR_DATA_SLAVE_DELAY_3_CR;
		int	WR_DATA_SLAVE_FORCE_CR;
		int	WR_DATA_SLAVE_RATIO_1_CR;
		int	WR_DATA_SLAVE_RATIO_2_CR;
		int	WR_DATA_SLAVE_RATIO_3_CR;
		int	WR_DATA_SLAVE_RATIO_4_CR;
		int	WRLVL_INIT_MODE_CR;
		int	WRLVL_INIT_RATIO_1_CR;
		int	WRLVL_INIT_RATIO_2_CR;
		int	WRLVL_INIT_RATIO_3_CR;
		int	WRLVL_INIT_RATIO_4_CR;
		int	WR_RD_RL_CR;
		int	RDC_FIFO_RST_ERRCNTCLR_CR;
		int	RDC_WE_TO_RE_DELAY_CR;
		int	USE_FIXED_RE_CR;
		int	USE_RANK0_DELAYS_CR;
		int	USE_LVL_TRNG_LEVEL_CR;
		int	DYN_CONFIG_CR;
		int	RD_WR_GATE_LVL_CR;
		int	DYN_RESET_CR;
		int	LEVELLING_FAILURE_SR;
		int	BIST_ERROR_1_SR;
		int	BIST_ERROR_2_SR;
		int	BIST_ERROR_3_SR;
		int	WRLVL_DQS_RATIO_1_SR;
		int	WRLVL_DQS_RATIO_2_SR;
		int	WRLVL_DQS_RATIO_3_SR;
		int	WRLVL_DQS_RATIO_4_SR;
		int	WRLVL_DQ_RATIO_1_SR;
		int	WRLVL_DQ_RATIO_2_SR;
		int	WRLVL_DQ_RATIO_3_SR;
		int	WRLVL_DQ_RATIO_4_SR;
		int	RDLVL_DQS_RATIO_1_SR;
		int	RDLVL_DQS_RATIO_2_SR;
		int	RDLVL_DQS_RATIO_3_SR;
		int	RDLVL_DQS_RATIO_4_SR;
		int	FIFO_1_SR;
		int	FIFO_2_SR;
		int	FIFO_3_SR;
		int	FIFO_4_SR;
		int	MASTER_DLL_SR;
		int	DLL_SLAVE_VALUE_1_SR;
		int	DLL_SLAVE_VALUE_2_SR;
		int	STATUS_OF_IN_DELAY_VAL_1_SR;
		int	STATUS_OF_IN_DELAY_VAL_2_SR;
		int	STATUS_OF_OUT_DELAY_VAL_1_SR;
		int	STATUS_OF_OUT_DELAY_VAL_2_SR;
		int	DLL_LOCK_AND_SLAVE_VAL_SR;
		int	CTRL_OUTPUT_FILTER_SR;
		int	RESERVED0;
		int	RD_DQS_SLAVE_DLL_VAL_1_SR;
		int	RD_DQS_SLAVE_DLL_VAL_2_SR;
		int	RD_DQS_SLAVE_DLL_VAL_3_SR;
		int	WR_DATA_SLAVE_DLL_VAL_1_SR;
		int	WR_DATA_SLAVE_DLL_VAL_2_SR;
		int	WR_DATA_SLAVE_DLL_VAL_3_SR;
		int	FIFO_WE_SLAVE_DLL_VAL_1_SR;
		int	FIFO_WE_SLAVE_DLL_VAL_2_SR;
		int	FIFO_WE_SLAVE_DLL_VAL_3_SR;
		int	WR_DQS_SLAVE_DLL_VAL_1_SR;
		int	WR_DQS_SLAVE_DLL_VAL_2_SR;
		int	WR_DQS_SLAVE_DLL_VAL_3_SR;
		int	CTRL_SLAVE_DLL_VAL_SR;
	} phy;

	int	reserved1[(0x400 - 0x3CC) >> 2];

	/*
	 * FIC-64 registers
	 */
	struct  {
		int	NB_ADDR_CR;
		int	NBRWB_SIZE_CR;
		int	WB_TIMEOUT_CR;
		int	HPD_SW_RW_EN_CR;
		int	HPD_SW_RW_INVAL_CR;
		int	SW_WR_ERCLR_CR;
		int	ERR_INT_ENABLE_CR;
		int	NUM_AHB_MASTERS_CR;
		int	LOCK_TIMEOUTVAL_1_CR;
		int	LOCK_TIMEOUTVAL_2_CR;
		int	LOCK_TIMEOUT_EN_CR;
	} fic;
};


struct mddr_cr{
  int mddr_cr;
};

struct ddrb_nb_size_cr{
  int ddrb_nb_size_cr;
};


int main(int argc, char **argv) {
  off_t addr, page;
  int fd,bits,dowrite=0,doread=1;
  unsigned char *start;
  unsigned char *chardat, charval;
  unsigned short *shortdat, shortval; 
  unsigned int *intdat, intval;
  
  struct ddr_regs	*ddr = (void *)0x40020000;
  struct mddr_cr        *mddr_cr = (void *)0x40038060;
  struct ddrb_nb_size_cr *ddrb_nb_size_cr = (void *)0x40038030;

  int secded = 0;

  if(strcmp(argv[1],"secded")==0){
    secded = 0;
  }else if(strcmp(argv[1],"no_secded")==0){
    secded = 1;
  }else{
    printf(" secded or no_secded\n");
    return 0;
  }

  mddr_cr->mddr_cr = (1 << 0);
  

  ddrb_nb_size_cr->ddrb_nb_size_cr = 0;
  

  ddr->ddrc.DYN_POWERDOWN_CR = (0 << REG_DDRC_POWERDOWN_EN);
  
  ddr->ddrc.DYN_SOFT_RESET_CR =			0;
  ddr->ddrc.DYN_REFRESH_1_CR =			0x27de;
  ddr->ddrc.DYN_REFRESH_2_CR =			0x30f;
  // ddr->ddrc.DYN_POWERDOWN_CR =			0x02; 
  ddr->ddrc.DYN_DEBUG_CR =			0x00;

  if(secded==1){
    ddr->ddrc.MODE_CR =				0x101; // this is without SECDED 
  }else if(secded==0){
    ddr->ddrc.MODE_CR =				0x115; // this is with SECDED 
  }

  ddr->ddrc.ADDR_MAP_BANK_CR =			0x999;
  ddr->ddrc.ECC_DATA_MASK_CR =			0x0000;
  ddr->ddrc.ADDR_MAP_COL_1_CR =			0x3333;
  ddr->ddrc.ADDR_MAP_COL_2_CR =			0xffff;
  ddr->ddrc.ADDR_MAP_ROW_1_CR =			0x8888;
  ddr->ddrc.ADDR_MAP_ROW_2_CR =			0x8ff;
  ddr->ddrc.INIT_1_CR =				0x0001;
  ddr->ddrc.CKE_RSTN_CYCLES_1_CR =		0x4242;
  ddr->ddrc.CKE_RSTN_CYCLES_2_CR =		0x8;
  ddr->ddrc.INIT_MR_CR =				0x520;
  ddr->ddrc.INIT_EMR_CR =				0x44;
  ddr->ddrc.INIT_EMR2_CR =			0x0000;
  ddr->ddrc.INIT_EMR3_CR =			0x0000;
  ddr->ddrc.DRAM_BANK_TIMING_PARAM_CR =		0xce0;
  ddr->ddrc.DRAM_RD_WR_LATENCY_CR =		0x86;
  ddr->ddrc.DRAM_RD_WR_PRE_CR =			0x235;
  ddr->ddrc.DRAM_MR_TIMING_PARAM_CR =		0x5c;
  ddr->ddrc.DRAM_RAS_TIMING_CR =			0x10f;
  ddr->ddrc.DRAM_RD_WR_TRNARND_TIME_CR =		0x178;
  ddr->ddrc.DRAM_T_PD_CR =			0x33;
  ddr->ddrc.DRAM_BANK_ACT_TIMING_CR =		0x1947;
  ddr->ddrc.ODT_PARAM_1_CR =			0x10;
  ddr->ddrc.ODT_PARAM_2_CR =			0x0000;
  ddr->ddrc.ADDR_MAP_COL_3_CR =			0x3300;
  // ddr->ddrc.DEBUG_CR =				0x3300; 
  ddr->ddrc.MODE_REG_RD_WR_CR =			0x0000;
  ddr->ddrc.MODE_REG_DATA_CR =			0x0000;
  ddr->ddrc.PWR_SAVE_1_CR =			0x506;
  ddr->ddrc.PWR_SAVE_2_CR =			0x0000;
  ddr->ddrc.ZQ_LONG_TIME_CR =			0x200;
  ddr->ddrc.ZQ_SHORT_TIME_CR =			0x40;
  ddr->ddrc.ZQ_SHORT_INT_REFRESH_MARGIN_1_CR =	0x12;
  ddr->ddrc.ZQ_SHORT_INT_REFRESH_MARGIN_2_CR =	0x2;

  if(secded==1){
    ddr->ddrc.PERF_PARAM_1_CR =			0x4000; // this is without SECDED 
  }else if(secded==0){
    ddr->ddrc.PERF_PARAM_1_CR =			0x4002; // this is with SECDED 
  }
  ddr->ddrc.HPR_QUEUE_PARAM_1_CR =		0x80f8;
  ddr->ddrc.HPR_QUEUE_PARAM_2_CR =		0x7;
  ddr->ddrc.LPR_QUEUE_PARAM_1_CR =		0x80f8;
  ddr->ddrc.LPR_QUEUE_PARAM_2_CR =		0x7;
  ddr->ddrc.WR_QUEUE_PARAM_CR =			0x200;
  ddr->ddrc.PERF_PARAM_2_CR =			0x400;
  ddr->ddrc.PERF_PARAM_3_CR =			0x0000;
  ddr->ddrc.DFI_RDDATA_EN_CR =			0x5;
  ddr->ddrc.DFI_MIN_CTRLUPD_TIMING_CR =		0x0003;
  ddr->ddrc.DFI_MAX_CTRLUPD_TIMING_CR =		0x0040;
  ddr->ddrc.DFI_WR_LVL_CONTROL_1_CR =		0x0000;
  ddr->ddrc.DFI_WR_LVL_CONTROL_2_CR =		0x0000;
  ddr->ddrc.DFI_RD_LVL_CONTROL_1_CR =		0x0000;
  ddr->ddrc.DFI_RD_LVL_CONTROL_2_CR =		0x0000;
  ddr->ddrc.DFI_CTRLUPD_TIME_INTERVAL_CR =	0x309;
  // ddr->ddrc.DYN_SOFT_RESET_CR2 =		0x4; //
  ddr->ddrc.AXI_FABRIC_PRI_ID_CR =		0x0000;
  ddr->ddrc.ECC_INT_CLR_REG =			0x0000;
  
  ddr->phy.DYN_BIST_TEST_CR =			0x0;
  ddr->phy.DYN_BIST_TEST_ERRCLR_1_CR =		0x0;
  ddr->phy.DYN_BIST_TEST_ERRCLR_2_CR =		0x0;
  ddr->phy.DYN_BIST_TEST_ERRCLR_3_CR =		0x0;
  ddr->phy.BIST_TEST_SHIFT_PATTERN_1_CR =		0x0;
  ddr->phy.BIST_TEST_SHIFT_PATTERN_2_CR =		0x0;
  ddr->phy.BIST_TEST_SHIFT_PATTERN_3_CR =		0x0;
  ddr->phy.DYN_LOOPBACK_TEST_CR =			0x0000;
  ddr->phy.BOARD_LOOPBACK_CR =			0x0;
  ddr->phy.CTRL_SLAVE_RATIO_CR =			0x80;
  ddr->phy.CTRL_SLAVE_FORCE_CR =			0x0;
  ddr->phy.CTRL_SLAVE_DELAY_CR =			0x0;
  if(secded==1){
    ddr->phy.DATA_SLICE_IN_USE_CR =			0xf;  // this is without SECDED //
  }else if(secded==0){
    ddr->phy.DATA_SLICE_IN_USE_CR =			0x13;  // this is with SECDED 
  }
  ddr->phy.LVL_NUM_OF_DQ0_CR =			0x0;
  ddr->phy.DQ_OFFSET_1_CR =			0x0;
  ddr->phy.DQ_OFFSET_2_CR =			0x0;
  ddr->phy.DQ_OFFSET_3_CR =			0x0;
  ddr->phy.DIS_CALIB_RST_CR =			0x0;
  ddr->phy.DLL_LOCK_DIFF_CR =			0xb;
  ddr->phy.FIFO_WE_IN_DELAY_1_CR =		0x0;
  ddr->phy.FIFO_WE_IN_DELAY_2_CR =		0x0;
  ddr->phy.FIFO_WE_IN_DELAY_3_CR =		0x0;
  ddr->phy.FIFO_WE_IN_FORCE_CR =			0x0;
  ddr->phy.FIFO_WE_SLAVE_RATIO_1_CR =		0x80;
  ddr->phy.FIFO_WE_SLAVE_RATIO_2_CR =		0x2004;
  ddr->phy.FIFO_WE_SLAVE_RATIO_3_CR =		0x100;
  ddr->phy.FIFO_WE_SLAVE_RATIO_4_CR =		0x8;
  ddr->phy.GATELVL_INIT_MODE_CR =			0x0;
  ddr->phy.GATELVL_INIT_RATIO_1_CR =		0x0;
  ddr->phy.GATELVL_INIT_RATIO_2_CR =		0x0;
  ddr->phy.GATELVL_INIT_RATIO_3_CR =		0x0;
  ddr->phy.GATELVL_INIT_RATIO_4_CR =		0x0;
  ddr->phy.LOCAL_ODT_CR =				0x1;
  ddr->phy.INVERT_CLKOUT_CR =			0x0;
  ddr->phy.RD_DQS_SLAVE_DELAY_1_CR =		0x0;
  ddr->phy.RD_DQS_SLAVE_DELAY_2_CR =		0x0;
  ddr->phy.RD_DQS_SLAVE_DELAY_3_CR =		0x0;
  ddr->phy.RD_DQS_SLAVE_FORCE_CR =		0x0;
  ddr->phy.RD_DQS_SLAVE_RATIO_1_CR =		0x4050;
  ddr->phy.RD_DQS_SLAVE_RATIO_2_CR =		0x501;
  ddr->phy.RD_DQS_SLAVE_RATIO_3_CR =		0x5014;
  ddr->phy.RD_DQS_SLAVE_RATIO_4_CR =		0x0;
  ddr->phy.WR_DQS_SLAVE_DELAY_1_CR =		0x0;
  ddr->phy.WR_DQS_SLAVE_DELAY_2_CR =		0x0;
  ddr->phy.WR_DQS_SLAVE_DELAY_3_CR =		0x0;
  ddr->phy.WR_DQS_SLAVE_FORCE_CR =		0x0;
  ddr->phy.WR_DQS_SLAVE_RATIO_1_CR =		0x0;
  ddr->phy.WR_DQS_SLAVE_RATIO_2_CR =		0x0;
  ddr->phy.WR_DQS_SLAVE_RATIO_3_CR =		0x0;
  ddr->phy.WR_DQS_SLAVE_RATIO_4_CR =		0x0;
  ddr->phy.WR_DATA_SLAVE_DELAY_1_CR =		0x0;
  ddr->phy.WR_DATA_SLAVE_DELAY_2_CR =		0x0;
  ddr->phy.WR_DATA_SLAVE_DELAY_3_CR =		0x0;
  ddr->phy.WR_DATA_SLAVE_FORCE_CR =		0x0;
  ddr->phy.WR_DATA_SLAVE_RATIO_1_CR =		0x50;
  ddr->phy.WR_DATA_SLAVE_RATIO_2_CR =		0x501;
  ddr->phy.WR_DATA_SLAVE_RATIO_3_CR =		0x5010;
  ddr->phy.WR_DATA_SLAVE_RATIO_4_CR =		0x0;
  ddr->phy.WRLVL_INIT_MODE_CR =			0x0;
  ddr->phy.WRLVL_INIT_RATIO_1_CR =		0x0;
  ddr->phy.WRLVL_INIT_RATIO_2_CR =		0x0;
  ddr->phy.WRLVL_INIT_RATIO_3_CR =		0x0;
  ddr->phy.WRLVL_INIT_RATIO_4_CR =		0x0;
  ddr->phy.WR_RD_RL_CR =				0x43;
  ddr->phy.RDC_FIFO_RST_ERRCNTCLR_CR =		0x0;
  ddr->phy.RDC_WE_TO_RE_DELAY_CR =		0x3;
  ddr->phy.USE_FIXED_RE_CR =			0x1;
  ddr->phy.USE_RANK0_DELAYS_CR =			0x1;
  ddr->phy.USE_LVL_TRNG_LEVEL_CR =		0x0;
  ddr->phy.DYN_CONFIG_CR =			0x0000;
  ddr->phy.RD_WR_GATE_LVL_CR =			0x0;

  ddr->phy.DYN_RESET_CR =				0x1;
  ddr->ddrc.DYN_SOFT_RESET_CR =			0x0001;




  /*

  if (argc < 3 || argc > 5) {
    fprintf(stderr,"Usage: peekpoke BIT_WIDTH ADDRESS <VALUE <x>>\n");
    fprintf(stderr,"<x> can be anything; supresses read-back on write\n");
    return 0;
  }
  sscanf(argv[1], "%d", &bits);
  if (bits != 8 && bits != 16 && bits != 32) {
    fprintf(stderr,"Error: BIT_WIDTH must be 8, 16, or 32\n");
    return 0;
  }
  addr = parseNumber(argv[2]);
  if (argc > 3 ) { // peekpoke BITS ADDRESS VALUE x
    intval = parseNumber(argv[3]);
    if (argc > 4) doread = 0;
    dowrite = 1;
  }
  fd = open("/dev/mem", O_RDWR|O_SYNC);
  if (fd == -1) {
    perror("open(/dev/mem):");
    return 0;
  }
  page = addr & 0xfffff000;
  start = mmap(0, getpagesize(), PROT_READ|PROT_WRITE, MAP_SHARED, fd, page);
  if (start == MAP_FAILED) {
    perror("mmap:");
    return 0;
  }
  if (bits == 8) {
    charval = (unsigned char)intval;
    chardat = start + (addr & 0xfff);
    if (dowrite) {
      *chardat = charval;
    }
    if (doread) {
      intval = (unsigned int)*chardat;
    }
  } else if (bits == 16) {
    shortval = (unsigned short)intval;
    shortdat = (unsigned short *)(start + (addr & 0xfff));
    if (dowrite) {
      *shortdat = shortval;
    }
    if (doread) {
      intval = (unsigned int)*shortdat;
    }
  } else { // bits == 32
    intdat = (unsigned int *)(start + (addr & 0xfff));
    if (dowrite) {
      *intdat = intval;
    }
    if (doread) {
      intval = *intdat;
    }
  }
  if (doread) {
    printf("0x%X\n", intval);
  }
  close(fd);
  return intval;
  */
}
