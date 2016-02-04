#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

int main()
{
	char ch_data = 0;
	
	//int a;
	FILE *fd;
	// wtest = 0x18;
	
	if((fd = open("/dev/radmonc", O_RDWR)) < 0){
		fprintf(stderr, "unable to open /dev/radmonc: %s\n", strerror(errno));
		return 0;
	}
	// try to read input first
	if(read(fd, &ch_data, 1)<0)
		fprintf(stderr, "oh no, could not read: %s\n", strerror(errno));
	else{
		//a = c;
		fprintf(stdout, "read from input: 0x%x\n", ch_data);
	}
	// try to write something to output
	ch_data = (char)(0x10 | 0x08);
	if(write(fd, &ch_data, 1) < 0)
		fprintf(stderr, "help! could not write: %s\n", strerror(errno));
	else
		fprintf(stdout, "wrote %x to output?\n", ch_data);
	

	while(1){
		ch_data = (char)(0x04 | 0x10);		
		write(fd, &ch_data, 1);
		ch_data = (char)(0x08);
		write(fd, &ch_data, 1);
	}
	
	
	if(fd>=0) close(fd);
	
	fprintf(stdout, "finished\n");
	return 0;
}
