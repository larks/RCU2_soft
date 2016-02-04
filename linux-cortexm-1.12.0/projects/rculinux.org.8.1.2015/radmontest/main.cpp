#include <cstdio>
#include <iostream>
#include "Radmon.hpp"
#include "TestFixture.hpp"

int
main( int argc, char **argv )
{
	Radmon rm;
	TestFixture tf;

	try {
		// Test that registers have default values
		uint16_t control = rm.getControl();
		uint16_t irqMask = rm.getIRQMask();
		uint16_t pwrIth  = rm.getSramPwrCurrentTh();

		tf.warn( control == 0x01,
				"Control register should have default value 0x01" );

		tf.warn( irqMask,
				"IRQ mask should have default value 0x10" );

		tf.assert( rm.getVersion() == 0x11,
				"Version should be 0x11" );

		tf.warn( pwrIth,
				"sram pwr current threshold should have default value 0x3ff" );

		rm.setIRQMask( 0x3f );
		tf.assert( rm.getIRQMask() == 0x3f,
				"IRQ mask should keep values set" );
		rm.setIRQMask( irqMask );

		rm.setSramPwrCurrentTh( 0x300 );
		rm.getSramPwrCurrentTh();
		tf.assert( rm.getSramPwrCurrentTh() == 0x300,
				"Sram pwr current threshold should keep values set" );
		rm.setSramPwrCurrentTh( pwrIth );

		rm.setControl( 0x133 );
		tf.assert( rm.getControl() == 0x133,
				"Control register should keep values set" );
		uint16_t status = rm.getStatus();
		tf.assert( status == 0x302 || status == 0xc02 || status == 0x300 || status == 0xc00,
				"Status should be either 0x300, 0x302, 0xc00 or 0xc02 while running." );

		rm.setControl( control );

	} catch( std::string str ) {
		tf.assert( 0, "Caugth exception: `" + str + "' ABORTING" );
	}

	// Print a pretty report and exit failure unless tests pass
	tf.report();
	return tf.passed() ? 0 : 1;
}
