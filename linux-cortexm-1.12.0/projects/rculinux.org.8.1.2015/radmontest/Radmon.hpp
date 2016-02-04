#pragma once
#include <cstdint>
#include <string>

class Radmon {
public :
	Radmon( const std::string &file = "/dev/radmon" );
	~Radmon();
	uint16_t getControl();
	void     setControl( uint16_t );
	uint16_t getStatus();
	uint16_t getIRQSource();
	uint16_t getIRQMask();
	void     setIRQMask( uint16_t );
	uint16_t getVersion();
	uint16_t getSEU_c1();
	uint16_t getMBU_c1();
	uint16_t getSEU_c2();
	uint16_t getMBU_c2();
	uint16_t getSEU_c3();
	uint16_t getMBU_c3();
	uint16_t getSEU_c4();
	uint16_t getMBU_c4();
	uint16_t getSEU_FIFO();
	uint16_t getMBU_FIFO();
	uint16_t getSramPwrCurrent();
	uint16_t getSramPwrCurrentTh();
	void     setSramPwrCurrentTh( uint16_t );
	uint16_t getTemperature();
	uint16_t getVccCurrent1();
	uint16_t getVccCurrent2();

	void resetConters();
	void print( bool headline = false );
private :
	uint16_t readRegister( uint8_t reg );
	void     writeRegister( uint8_t reg, uint16_t value );
	FILE *chardev;
};
