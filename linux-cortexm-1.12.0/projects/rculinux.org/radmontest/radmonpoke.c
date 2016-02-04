#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

void
usage(void)
{
	fprintf( stderr, "Usage: radmon register [value]\n" );
	fprintf( stderr, "  Register is in the range [0x00 .. 0x14] or 0xa5.\n" );
	fprintf( stderr, "  If a value is given it gets written to register.\n" );
	fprintf( stderr, "  The value in the register (after write) is printed to stdout.\n" );
	exit(0);
}


int
main( int argc, char **argv )
{
	uint16_t  reg;
	uint16_t  value;
	FILE     *f;

	if ( argc < 2 || argc > 3 )
		usage();

	if ( sscanf( argv[1], "%hi", &reg ) != 1 )
		usage();
	if ( reg > 21 && reg != 0xa5 )
		usage();

	if ( argc == 3 && sscanf( argv[2], "%hi", &value ) != 1 )
		usage();

	if ( argc == 2 && reg == 0xa5 ) {
		/* Can't read from reset. */
		printf( "0x00\n" );
		return 0;
	}

	f = fopen( "/dev/radmon", "wb+" );
	if ( !f ) {
		perror( "fopen( /dev/radmon )" );
		return 1;
	}

	if ( fseek( f, reg * 2, SEEK_SET ) != 0 ) {
		perror( "fseek(...)" );
		return 1;
	}
	if ( argc == 3 ) {
		if ( fwrite( &value, sizeof(value), 1, f ) != 1 ) {
			perror( "fwrite(...)" );
			return 1;
		}
		if ( reg == 0xa5 ) {
			/* Can't read from reset. */
			printf( "0x00\n" );
			return 0;
		}
		if ( fseek( f, reg * 2, SEEK_SET ) != 0 ) {
			perror( "fseek(...)" );
			return 1;
		}
	}

	if ( fread( &value, sizeof(value), 1, f ) != 1 ) {
		perror( "fread(...)" );
		return 1;
	}
	printf( "0x%02hx\n", value );

	fclose( f );

	return 0;
}
