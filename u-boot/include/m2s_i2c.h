/*
 * I2C driver for M2S 
 * Imported from SoftConsole project and maintained by T. Gunji 
 *
 */


#ifndef _M2S_I2C_H_
#define _M2S_I2C_H_

typedef enum mss_i2c_clock_divider {
  MSS_I2C_PCLK_DIV_256 = 0u,
  MSS_I2C_PCLK_DIV_224,
  MSS_I2C_PCLK_DIV_192,
  MSS_I2C_PCLK_DIV_160,
  MSS_I2C_PCLK_DIV_960,
  MSS_I2C_PCLK_DIV_120,
  MSS_I2C_PCLK_DIV_60,
    MSS_I2C_BCLK_DIV_8
} mss_i2c_clock_divider_t;


#define MSS_I2C_RELEASE_BUS     0x00u
#define MSS_I2C_HOLD_BUS        0x01u
#define MSS_I2C_SMBALERT_IRQ       0x01u
#define MSS_I2C_SMBSUS_IRQ         0x02u
#define MSS_I2C_NO_TIMEOUT  0u


typedef enum mss_i2c_status
  {
    MSS_I2C_SUCCESS = 0u,
    MSS_I2C_IN_PROGRESS,
    MSS_I2C_FAILED,
    MSS_I2C_TIMED_OUT
  } mss_i2c_status_t;

typedef enum mss_i2c_slave_handler_ret {
  MSS_I2C_REENABLE_SLAVE_RX = 0u,
    MSS_I2C_PAUSE_SLAVE_RX = 1u
} mss_i2c_slave_handler_ret_t;

typedef struct mss_i2c_instance mss_i2c_instance_t ;

typedef mss_i2c_slave_handler_ret_t 
(*mss_i2c_slave_wr_handler_t)( mss_i2c_instance_t *instance, uint8_t *, uint16_t );

typedef struct
{
  uint32_t RESERVED0[128];
  uint32_t SMB_SMBALERT_IE;
  uint32_t SMB_SMBSUS_IE;
  uint32_t SMB_SMB_IPMI_EN;
  uint32_t SMB_SMBALERT_NI;
  uint32_t SMB_SMBALERT_NO;
  uint32_t SMB_SMBSUS_NI;
  uint32_t SMB_SMBSUS_NO;
  uint32_t SMB_SMBus_Reset;
} I2C_SMBus_BitBand_TypeDef;




#endif /*_M2S_I2C_H_*/


