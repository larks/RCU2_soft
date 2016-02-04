#include <stdio.h>
#include <time.h>
#include <math.h>
#include <unistd.h>
#include <stdlib.h> /*exit()*/
#include <errno.h>
//#include "mrtimers.h"
int frequency_of_primes (int n) {
    int i,j;
    int freq=n-1;
    for (i=2; i<=n; ++i) for (j=sqrt(i);j>1;--j) if (i%j==0) {--freq; break;}
    return freq;
}

int timedWait(int iWaitSec, int iWaitMusec);
int main(void)
{
    
    fprintf(stdout, "clocks_per_sec=%lu\n", CLOCKS_PER_SEC);

	clock_t t;
	int f;
	t = clock();
	printf ("Calculating...\n");
	f = frequency_of_primes (999);
	printf ("The number of primes lower than 100,000 is: %d\n",f);
	
	t = clock() - t;
	printf ("It took me %lu clicks (%f seconds).\n", (long int)t,((float)t)/CLOCKS_PER_SEC);
/*
	t = clock();
	timedWait(3,0);
	t = clock() - t;
	printf("hello: %.2f",((float)t)/CLOCKS_PER_SEC);
*/

/*
	// Nanosleep
	struct timespec timeOut,remains;
	
	timeOut.tv_sec = 4;
	timeOut.tv_nsec = 10000;
	
	t = clock();
	int a;
	for(a=0; a<4; a++){
		fprintf(stdout, "Hello %d\n", a);
		fprintf(stdout, "before: %lu\n", (long int)clock());
		nanosleep(&timeOut, &remains);
		fprintf(stdout, "after: %lu\n", (long int)clock());
	}
	t = clock() - t;
	fprintf(stdout, "For loop took %lu clicks (%f seconds).\n",(long int)t,((float)t)/CLOCKS_PER_SEC);
*/
	
	FILE * test;
	long lSize;
	char * buffer;
	size_t result;
	
	test = fopen("/sys/bus/i2c/devices/0-0054/eeprom", "rb");
	if(test == NULL){
		fprintf(stderr, "Failed to open file\n");
		exit(-1);
	}
	fseek(test, 0, SEEK_END);
	lSize = ftell(test);
	rewind(test);
	fprintf(stdout, "Filesize: %lu\n", lSize);
	
	buffer = (char*) malloc(sizeof(char)*lSize);
	
	//char serial[] = {'C','A','F','E','B','A','B','E'};
	//fwrite(serial, sizeof(char), lSize, test);
	//fwrite(serial, sizeof(char), sizeof(serial), test);
	//rewind(test);
	
	if(buffer == NULL){
		fprintf(stderr, "Memory error\n");
		exit(-1);
	}
	result = fread(buffer, 1, lSize, test);
	/*
	if(result != lSize){
		fprintf(stderr, "something went wrong reading\n");
		exit(-1);
	}
	*/
	
	fprintf(stdout, "%s\n", buffer);
	
	fclose(test);
	
	
	
	return 0;
  
    
}
/*
int timedWait(int iWaitSec, int iWaitMusec)
{
  int iResult=0;
  // this is maybe overdoing it a little
  int iCurrSec=0;
  int iCurrMusec=0;
  int iTimer=startTimer(0); // timer of type ITIMER_REAL
  if (iTimer>0) {
    int iSleep=10;
    if (iWaitSec>0) iSleep=500000;
    else iSleep=iWaitMusec/4;
    do {
      usleep(iSleep);
      iResult=getTimerValue(iTimer, &iCurrSec, &iCurrMusec);
      //fprintf(stderr, "current timer val: %d sec  %d usec\n", iCurrSec, iCurrMusec);
    } while (iResult>=0 && (iCurrSec<iWaitSec || iCurrMusec<iWaitMusec));
    stopTimer(iTimer);
  } else {
    fprintf(stderr, "can not start timer, skip wait command\n");
    iResult=-EACCES;
  }
  return iResult;
}
*/
