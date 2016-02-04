#define _POSIX_C_SOURCE 2
#include <stdio.h>
#include <unistd.h>
#include "Radmon.hpp"


int
main( int argc, char **argv )
{
	int    opt;
	bool   loop = false;
	Radmon rm;

	while (( opt = getopt( argc, argv, "l" ) ) != -1 ) {
		switch( opt ) {
		case 'l' :
			loop = true;
			break;
		default :
			fprintf( stderr, "Usage: %s [-l]\n", argv[0] );
			return 1;
		}
	}
	rm.print( true );
	while ( loop ) {
		sleep( 1 );
		rm.print();
	}

	return 0;
}
