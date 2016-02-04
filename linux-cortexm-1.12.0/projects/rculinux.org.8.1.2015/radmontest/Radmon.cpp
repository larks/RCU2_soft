#include <cstdio>
#include "Radmon.hpp"

static const uint8_t REG_CONTROL        = 0x00;
static const uint8_t REG_RESET_COUNTERS = 0x01;
static const uint8_t REG_STATUS         = 0x02;
static const uint8_t REG_IRQ_SRC        = 0x03;
static const uint8_t REG_IRQ_MASK       = 0x04;
static const uint8_t REG_VERSION        = 0x05;
static const uint8_t REG_SEU_C1         = 0x06;
static const uint8_t REG_MBU_C1         = 0x07;
static const uint8_t REG_SEU_C2         = 0x08;
static const uint8_t REG_MBU_C2         = 0x09;
static const uint8_t REG_SEU_C3         = 0x0a;
static const uint8_t REG_MBU_C3         = 0x0b;
static const uint8_t REG_SEU_C4         = 0x0c;
static const uint8_t REG_MBU_C4         = 0x0d;
static const uint8_t REG_SEU_FIFO       = 0x0e;
static const uint8_t REG_MBU_FIFO       = 0x0f;
static const uint8_t REG_PWR_I          = 0x10;
static const uint8_t REG_PWR_I_TH       = 0x11;
static const uint8_t REG_TEMP           = 0x12;
static const uint8_t REG_VCC1           = 0x13;
static const uint8_t REG_VCC2           = 0x14;

Radmon::Radmon( const std::string &file )
{
	chardev = fopen( file.c_str(), "wb+" );
	if ( !chardev ) {
		throw std::string( "Could not open file " + file );
	}
}


Radmon::~Radmon()
{
	fclose( chardev );
}


uint16_t Radmon::getControl()          { return readRegister( REG_CONTROL );  }
uint16_t Radmon::getStatus()           { return readRegister( REG_STATUS );   }
uint16_t Radmon::getIRQSource()        { return readRegister( REG_IRQ_SRC );  }
uint16_t Radmon::getIRQMask()          { return readRegister( REG_IRQ_MASK ); }
uint16_t Radmon::getVersion()          { return readRegister( REG_VERSION );  }
uint16_t Radmon::getSEU_c1()           { return readRegister( REG_SEU_C1 );   }
uint16_t Radmon::getMBU_c1()           { return readRegister( REG_MBU_C1 );   }
uint16_t Radmon::getSEU_c2()           { return readRegister( REG_SEU_C2 );   }
uint16_t Radmon::getMBU_c2()           { return readRegister( REG_MBU_C2 );   }
uint16_t Radmon::getSEU_c3()           { return readRegister( REG_SEU_C3 );   }
uint16_t Radmon::getMBU_c3()           { return readRegister( REG_MBU_C3 );   }
uint16_t Radmon::getSEU_c4()           { return readRegister( REG_SEU_C4 );   }
uint16_t Radmon::getMBU_c4()           { return readRegister( REG_MBU_C4 );   }
uint16_t Radmon::getSEU_FIFO()         { return readRegister( REG_SEU_FIFO ); }
uint16_t Radmon::getMBU_FIFO()         { return readRegister( REG_MBU_FIFO ); }
uint16_t Radmon::getSramPwrCurrent()   { return readRegister( REG_PWR_I );    }
uint16_t Radmon::getSramPwrCurrentTh() { return readRegister( REG_PWR_I_TH ); }
uint16_t Radmon::getTemperature()      { return readRegister( REG_TEMP );     }
uint16_t Radmon::getVccCurrent1()      { return readRegister( REG_VCC1 );     }
uint16_t Radmon::getVccCurrent2()      { return readRegister( REG_VCC2 );     }

void Radmon::setControl( uint16_t val ){ writeRegister( REG_CONTROL, val );   }
void Radmon::setIRQMask( uint16_t val ){ writeRegister( REG_IRQ_MASK, val );  }
void Radmon::setSramPwrCurrentTh( uint16_t val ){ writeRegister( REG_PWR_I_TH, val ); }


uint16_t
Radmon::readRegister( uint8_t reg )
{
	uint16_t value;
	if ( fseek( chardev, reg * 2, SEEK_SET ) != 0 ) {
		throw std::string( "Could not seek" );
	}
	if ( fread( &value, sizeof(value), 1, chardev ) != 1 ) {
		throw std::string( "Could not read from register" );
	}
	return value;
}


void
Radmon::writeRegister( uint8_t reg, uint16_t value )
{
	if ( fseek( chardev, reg * 2, SEEK_SET ) != 0 ) {
		throw std::string( "Could not seek" );
	}
	if ( fwrite( &value, sizeof(value), 1, chardev ) != 1 ) {
		throw std::string( "Could not write to register" );
	}
}


void
Radmon::resetConters()
{
	writeRegister( REG_RESET_COUNTERS, 0 );
}


static inline float
temp( uint16_t t )
{
	return t / 4.0f;
}

static inline float
current( uint16_t i )
{
	return i * 2.5f / 1024.0f * 1000.0f;
}

static inline float
voltage( uint16_t u )
{
	return (4.9f/2.7f) * 2.5f * (u/1024.f);
}


void
Radmon::print( bool headline )
{
	if ( headline ) {
		printf( "Cont   Stat   IRQS   IRQM   Vers   SEU1  MBU1  SEU2  MBU2  SEU3  MBU3  "
				"SEU4  MBU4  SEUF  MBUF  RamI     RIth     Temp     VCC1     VCC2\n" );
	}
	printf( "0x%04x 0x%04x 0x%04x 0x%04x 0x%04x %05d %05d %05d %05d %05d "
			"%05d %05d %05d %05d %05d %08f %08f %08f %08f %08f\n",
			getControl(), getStatus(), getIRQSource(), getIRQMask(),
			getVersion(), getSEU_c1(), getMBU_c1(),
			getSEU_c2(), getMBU_c2(), getSEU_c3(), getMBU_c3(),
			getSEU_c4(), getMBU_c4(), getSEU_FIFO(), getMBU_FIFO(),
			current(getSramPwrCurrent()), current(getSramPwrCurrentTh()),
			temp(getTemperature()), voltage(getVccCurrent1()), voltage(getVccCurrent2()) );
}
