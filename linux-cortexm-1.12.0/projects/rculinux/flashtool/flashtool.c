#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/types.h>
#include <linux/spi/spidev.h>

#define SPI_READ          0x03
#define SPI_FAST_READ     0x0B
#define SPI_DUAL_READ     0x3B
#define SPI_QUAD_READ     0x6B
#define SPI_DUAL_READ_HP  0xBB
#define SPI_QUAD_READ_HP  0xEB
#define SPI_READ_ID       0x9F
#define SPI_READ_ID2      0x90
#define SPI_WRITE_ENABLE  0x06
#define SPI_WRITE_DISABLE 0x04
#define SPI_ERASE_4K      0x20
#define SPI_ERASE_8K      0x40
#define SPI_ERASE         0xD8
#define SPI_BULK_ERASE    0xBE
#define SPI_PROGRAM       0x02
#define SPI_QUAD_PROGRAM  0x32
#define SPI_READ_STATUS   0x05
#define SPI_WRITE_REG     0x01
#define SPI_READ_CONFIG   0x35
#define SPI_CLEAR_STATUS  0x30
#define SPI_POWER_DOWN    0xB9
#define SPI_WAKE_UP       0xAB
#define SPI_OTP_PROGRAM   0x42
#define SPI_OTP_READ      0x4B
#define SPI_BAR_WRITE     0x17
#define SPI_ERASE_SUSPEND 0x75
#define SPI_ERASE_RESUME  0x7A


static const char *device = "/dev/spidev0.0";
static uint8_t mode;
static uint8_t bits = 8;
static uint32_t speed = 500000;
static uint16_t delay;
static FILE    *infile;
static FILE    *outfile;
static char     action = 'r';
static int      address;
static int      verbosity = 0;


static void
pabort( const char *s )
{
	perror(s);
	exit( EXIT_FAILURE );
}

#define INFO( lvl, fmt, ... )                                          \
do                                                                     \
{                                                                      \
	if ( verbosity >= lvl )                                        \
		printf( fmt "\n", ##__VA_ARGS__ );                     \
} while ( 0 )


/* Send/recv n bytes. */
void
spi_tx_rx( int fd, uint8_t *tx, uint8_t *rx, size_t n )
{
	int ret;
	struct spi_ioc_transfer tr = {
		.tx_buf = (unsigned long)tx,
		.rx_buf = (unsigned long)rx,
		.len = n,
		.delay_usecs = delay,
		.speed_hz = speed,
		.bits_per_word = bits,
	};
	INFO( 3, "Sending message 0x%02x", tx[0] );
	ret = ioctl(fd, SPI_IOC_MESSAGE(1), &tr);
	if (ret == 1)
		pabort( "Can't send spi message." );
}


/* Wait until busy flag is cleared. */
void
spi_wait( int fd )
{
	uint8_t tx[] = { SPI_READ_STATUS, 0 };
	uint8_t rx[sizeof(tx)];
	do {
		INFO( 1, "Polling flash busy-flag." );
		spi_tx_rx( fd, tx, rx, sizeof(tx) );
	} while ( rx[1] & 1 );
}


/* Print content of status register */
void
spi_status( int fd )
{
	uint8_t tx[] = { SPI_READ_STATUS, 0 };
	uint8_t rx[sizeof(tx)];
	spi_tx_rx( fd, tx, rx, sizeof(tx) );
	INFO( 2, "Status register has value 0x%x", rx[1] );
}


/* Verify that read ID returns expected value. */
void
verify( int fd )
{
	uint8_t tx[] = { 0x9F, 0x00, 0xff, 0xff, };
	uint8_t rx[sizeof(tx)] = { 0 };
	uint8_t version[sizeof(rx)] = { 0xff, 0x01, 0x40, 0x17, };

	INFO( 1, "Verifying flash ID" );
	spi_tx_rx( fd, tx, rx, sizeof(tx) );
	INFO( 3, "Manufacturer: 0x%02x, Dev.type: 0x%02x, Capacity: 0x%02x", rx[1], rx[2], rx[3] );

	for ( int i=0; i<sizeof(rx); i++ ) {
		if ( rx[i] != version[i] )
			pabort( "Device version does not match expected value.\n" );
	}
}


/* Read a single page (256b) */
void
read_page( int fd, uint32_t addr )
{
	INFO( 1, "Reading page %x", addr );
	int ret;
	uint8_t tx[256 + 4] = { 0 };
	uint8_t rx[sizeof(tx)] = { 0 };
	tx[0] = SPI_READ;
	tx[1] = (addr >> 16) & 0xff;
	tx[2] = (addr >>  8) & 0xff;
	tx[3] = (addr >>  0) & 0xff;
	spi_tx_rx( fd, tx, rx, sizeof(tx) );

	/*
	fwrite( rx+4, 1, 256, outfile );
	*/
	for ( ret=0 ; ret<sizeof(rx)-4 ; ++ret ) {
		fprintf( outfile, "%02x", rx[ret+4] );
		if ( ret % 8 != 7 )
			fprintf( outfile, " " );
		else
			fprintf( outfile, "\n" );
	}
}


/* Erase one sector of memory (4kb) */
void
erase_sector( int fd, uint32_t addr )
{
	INFO( 1, "Erasing sector %x", addr );
	uint8_t tx[4] = { 0 };
	uint8_t rx[4] = { 0 };

	tx[0] = SPI_WRITE_ENABLE;
	spi_wait( fd );
	spi_tx_rx( fd, tx, rx, 2 );

	tx[0] = SPI_ERASE_4K;
	tx[1] = (addr >> 16) & 0xff;
	tx[2] = (addr >>  8) & 0xff;
	tx[3] = (addr >>  0) & 0xff;
	spi_tx_rx( fd, tx, rx, sizeof(tx) );
}


/* Write one page (256b) */
void
write_page( int fd, uint32_t addr )
{
	INFO( 1, "Writing page %x", addr );
	int ret;
	uint8_t tx[256 + 4] = { 0 };
	uint8_t rx[sizeof(tx)] = { 0 };

	tx[0] = SPI_WRITE_ENABLE;
	spi_wait( fd );
	spi_tx_rx( fd, tx, rx, 2 );
	spi_status( fd );

	tx[0] = SPI_PROGRAM;
	tx[1] = (addr >> 16) & 0xff;
	tx[2] = (addr >>  8) & 0xff;
	tx[3] = (addr >>  0) & 0xff;
	fread( tx+4, 1, 256, infile );
	spi_tx_rx( fd, tx, rx, sizeof(tx) );
}


static void
usage( const char *prog )
{
	printf("Usage: %s [-dsiowr] address\n", prog);
	puts("  -d    device to use (default /dev/spidev0.0)\n"
	     "  -s    max speed (Hz)\n"
	     "  -i    infile (default stdin)\n"
	     "  -o    outfile (deafult stdout)\n"
	     "  -w    perform a write\n"
	     "  -r    perform a read (default)\n"
	);
	exit(1);
}


static void
parse_opts( int argc, char *argv[] )
{
	infile = stdin;
	outfile = stdout;
	int c;
	while ((c = getopt(argc, argv, "d:s:i:o:wrv")) != EOF) {
		switch (c) {
		case 'd':
			device = optarg;
			break;
		case 's':
			speed = atoi(optarg);
			break;
		case 'i':
			infile = fopen( optarg, "rb" );
			if ( !infile )
				pabort( "Could not open file" );
			break;
		case 'o':
			outfile = fopen( optarg, "wb" );
			if ( !outfile )
				pabort( "Could not open file" );
			break;
		case 'w':
		case 'r':
			action = c;
			break;
		case 'v':
			++verbosity;
			break;
		default:
			usage(argv[0]);
			break;
		}
	}
	if ( (optind + 1) != argc )
		usage( argv[0] );

	if ( 1 != sscanf( argv[optind], "%i", &address ) )
		pabort( "Address not a number" );
}


int
main( int argc, char *argv[] )
{
	int ret = 0;
	int fd;

	parse_opts(argc, argv);

	fd = open(device, O_RDWR);
	if (fd < 0)
		pabort("can't open device");

	/*
	 * spi mode
	 */
	ret = ioctl(fd, SPI_IOC_WR_MODE, &mode);
	if (ret == -1)
		pabort("can't set spi mode");

	ret = ioctl(fd, SPI_IOC_RD_MODE, &mode);
	if (ret == -1)
		pabort("can't get spi mode");

	/*
	 * bits per word
	 */
	ret = ioctl(fd, SPI_IOC_WR_BITS_PER_WORD, &bits);
	if (ret == -1)
		pabort("can't set bits per word");

	ret = ioctl(fd, SPI_IOC_RD_BITS_PER_WORD, &bits);
	if (ret == -1)
		pabort("can't get bits per word");

	/*
	 * max speed hz
	 */
	ret = ioctl(fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed);
	if (ret == -1)
		pabort("can't set max speed hz");

	ret = ioctl(fd, SPI_IOC_RD_MAX_SPEED_HZ, &speed);
	if (ret == -1)
		pabort("can't get max speed hz");


	verify(fd);

	if ( action == 'w' ) {
		erase_sector( fd, address );
		write_page( fd, address );
	} else {
		read_page( fd, address );
	}

	if ( infile != stdin )
		fclose( infile );
	if ( outfile != stdout )
		fclose( outfile );
	close(fd);

	return ret;
}
