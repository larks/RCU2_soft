/************************************************************************
**
**
** This file is property of and copyright by the Experimental Nuclear 
** Physics Group, Dep. of Physics and Technology
** University of Bergen, Norway, 2014
** This file has been written by Lars Bratrud,
** Lars.Bratrud@cern.ch
**
** Permission to use, copy, modify and distribute this software and its  
** documentation strictly for non-commercial purposes is hereby granted  
** without fee, provided that the above copyright notice appears in all  
** copies and that both the copyright notice and this permission notice  
** appear in the supporting documentation. The authors make no claims    
** about the suitability of this software for any purpose. It is         
** provided "as is" without express or implied warranty.                 
**
*************************************************************************/

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>
#include "cmdInterpreter.h"

const char * dev_name = "/dev/sample2";

unsigned int parseBinary(char *str) {
  unsigned int val = 0;
  
  if (*str == 'b') {
    str++;
    while (*str) {
      if (*str == '0') {
        val <<= 1;
      } else if (*str == '1') {
        val = (val << 1) + 1;
      } else {
        goto binaryError;
      }
    }
  }
  return val;
 binaryError:
  fprintf(stderr,"Unrecognized numeric value: %s\n",str);
  return 999;
}

unsigned int parseNumber(char *str) {
  unsigned int addr = 0;

  if (!sscanf(str, "0x%x", &addr)) {
    if (!sscanf(str, "%u", &addr)) {
      addr = parseBinary(str);
    }
  }
  return addr;
}

int printHelp()
{
  printf("  all parameters are hex values preceeded by a '0x' or decimal numbers\n");
  printf("  some of the commands have a short version, in most cases the first letter\n");
//  printf("  quit                   : q\n");
//  printf("  info                   : i\n");
  printf("  driver info            : d\n");
//  printf("  repeat previous command: p\n");
  printf("  single read rcu bus memory   : r[ead] 0x<address> (e.g.: r 0x7000)\n");
  printf("  multiple read rcu bus memory : r[ead] 0x<address> <dec no> (e.g.: r 0x7000 16)\n");
//  printf("                                 see 'hr\' for details and further options\n");
  printf("  single write rcu bus memory  : w[rite] 0x<address> 0x<data>\n");
  printf("                                 (e.g.: w 0x6800 0x34)\n");
//  printf("  multiple write with const    : w[rite] [-s,--swap] 0x<address> <dec no> 0x<data>\n");
//  printf("                                 (e.g.: w 0x6800 12 0x0a)\n");
/*  printf("  write file to rcu bus memory : w[rite] [-s,--swap] 0x<address> <fspec> 'filepath' <count> \n");
  printf("     optional format spec: -b4(default) -b2 -b1 binary 32,16,8 bit\n");
  printf("                           -b10 binary compressed 10 bit, -c ascii\n");
  printf("     optional number after the filepath specifies count for partially write\n");
  printf("     e.g.: w 0x6800 -c 'pedestal.dat' 512,  w 0x7000 'pgm.dat'\n");
*/
  printf("  sending a single command     : c 0x<address> (translated to w <address> 0x0\n");
/*
  printf("  check status of mem location : ? 0x<address> [[c,f] 0x<bitmask> 0x<pattern>]\n");
  printf("                                               [t n s(ec)/u(sec)]\n");
*/
  printf("  batch processing             : b[atch] 'filepath' [[-l] <count>,-i]\n");
/*
  printf("                                 see \'hb\' for details and further options\n");
  printf("  wait command                 : wait <n> s(ec)/u(sec)\n");
  printf("  log message                  : e[cho] [-o,-a <filename>] <message>\n");
  printf("     the message can contain a \'-t\' specifier to print a timestamp\n\n");
*/
//  printf("  flash memory commands        : flash <cmd> (try \'flash help\')\n");
//  printf("  selectmap commands           : sm[selectmap] <cmd> (try \'sm help\')\n");
//  printf("  firmware commands            : fw[firmware] <cmd> (try \'fw help\')\n");
//  printf("  read bus control register    :rcr[read-ctrlreg]\n");
  //printf("  check rcu bus memory: m 0x<address> 0x<size in byte>\n");
  //printf("  read from file\n");
//  printf("  -/+ profile: switch on/off profiling\n");
//  printf("  - turn off all messages\n");
//  printf("  + turn on default messages\n");
//  printf("  hd debug message info\n");
//  printf("  hr detailed help for the read-command\n");
  return 0;
}

void executeCommands(uint32_t arg_count, char **arguments, FILE * fp)
{
	uint32_t addr;
	uint32_t data = 0x0;
	int n;      /* Loop counter */
//	    ch;        /* character buffer */
	/* Go through arguments from last to first 
	 * This is done so that additional options
	 * are scanned before read/write options.
	 */
	for (n = arg_count-1; n >= 0; n-- )
	{
/*		fprintf(stdout, "n: %d\n", n);*/
		switch( (int)arguments[n][0] )
		{
		/* Read data in address */
		case 'r':
			if(arguments[n][1]) break; /* Temporary solution */
			char temp[2];
			temp[0] = arguments[n+2][0];
			temp[1] = arguments[n+2][1];
			//fprintf(stdout, "%s\n", arguments[n+1]);
				if(strcmp(temp, "0x") == 0){
					fprintf(stderr, "Read does not take a hex value...\n");
					exit(1);	
				}
			addr = parseNumber(arguments[n+1]);
			if(parseNumber(arguments[n+2])){
				uint32_t iCnt;
				for(iCnt = addr; iCnt < (addr+parseNumber(arguments[n+2])); iCnt++ ){
					data = registerAccess(iCnt, 0x0, "r");
					if(!fp) fp=stdout;
					fprintf(fp, "%#x: %#x\n", iCnt, data);
				}
			}
			else{
				data = registerAccess(addr, 0x0, "r");
				/* Print results */
				if(!fp) fp=stdout;
				fprintf(fp, "%#x: %#x\n", addr, data);
			}
			break;
		/* Write data to address */
		case 'w':
			if(arguments[n][1]) break; /* */
				addr = parseNumber(arguments[n+1]);
			if (arg_count < 3){printHelp(); break;}
			else
				data = parseNumber(arguments[n+2]);
				data = registerAccess(addr, data, "w");
				data = registerAccess(addr, 0x0, "r");
			/* Print results */
			if(!fp) fp=stdout;
				fprintf(fp, "%#x: %#x\n", addr, data);
			break;
		/* Write 0x0 to address */
		case 'c':
			if(arguments[n][1]) break;
			/* Write zero to register address */
			addr = parseNumber(arguments[n+1]);
			data = 0x0;
			data = registerAccess(addr, data, "w");
			data = registerAccess(addr, 0x0, "r"); /*read back value*/
			/* Print results */
			if(!fp) fp=stdout;
			fprintf(fp, "%#x: %#x\n", addr, data); /* The printing should happen outside the loop, this means we fill up a buffer*/
			break;
		/* Log message */
		case 'e':
			if(arguments[n][1]) break;
			if(!fp) fp=stdout;
			if( (arg_count > 3) && (arguments[n+2][0] != '-') )
				 fprintf(fp, "Too many arguments\n");
			else fprintf(fp, "%s\n", arguments[n+1]);
			break;
		case 's':
			if(arguments[n][1] != 'n') break;
			   //int eSize;
			   char * buffer;
			   FILE * eeprom;
			   size_t result;
			   eeprom = fopen("/sys/bus/i2c/devices/0-0054/eeprom", "rb");
			   if(eeprom == NULL){
			      fprintf(stderr, "Failed to open file\n");
			      exit(-1);
	           }
	           buffer = (char*) malloc(sizeof(char)*8);
	           result = fread(buffer, 1, 8, eeprom);
	           fprintf(stdout, "SN: %s\n", buffer);
	           fclose(eeprom);
	           free(buffer);
			break;
		/* Here comes the additional options */
		case '-':
			switch( (int)arguments[n][1] )
			{
			/* Write output to file */
			case 'o':
			case 'a':
				if (arg_count > n){
					const char * strMode = "w";
					if ((int)arguments[n][1] == 'a' ) strMode = "a";
					fp = fopen(arguments[n+1], strMode);
					if(fp == NULL){
						fprintf(stderr, "Could not open file %s for writing.\n", arguments[n+1]);
						exit(1);
					}
				}
				else fprintf(stderr, "Missing argument to option %s \n", arguments[n]);
				break;
			/* Format string */
			case 'f':
				if(arg_count > n){
					
				}
				else fprintf(stderr, "Missing argument to option %s \n", arguments[n]);
				break;
			}
			break;
		case '/': /* yeah, not really good - a lot of errors can occur*/
		case '0':
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
		case '#': /* Comments are thrown away */
			break;
		default:
			//fprintf(stderr, "Error, oops, we tripped\n");
			//exit(1);
			//return 1;
			break;
		}
	
	}
	//return 0;
}

/*
 * register access function
*/
uint32_t registerAccess(uint32_t address, uint32_t data, char * rORw)
{
		uint32_t c, x;
		int fd = -1;
		/* Read procedure */
		if (strcmp(rORw, "r") == 0) {
			if ((fd = open(dev_name, O_RDWR)) < 0) {
				fprintf(stderr, "unable to open %s: %s\n", dev_name, strerror(errno));
			    exit(-1);
				}
			/* First we seek */
			if ((x = lseek(fd, address, SEEK_SET)) < 0) {
				fprintf(stderr, "unable to seek %s: %s\n", dev_name, strerror(errno));
				exit(-1);
			}
			/* Second we read the requested memory */
			if ((x = read(fd, &c, 4)) < 0) {
				fprintf(stderr, "unable to read %s: %s\n", dev_name, strerror(errno));
	        	exit(-1);
			}
			if (fd >= 0) {
				close(fd);
			}	
			return c;
/*			fprintf(stdout, "0x%x\n", c); */
		}
		/* Write procedure */
		else if (strcmp(rORw, "w") == 0) {
			if ((fd = open(dev_name, O_RDWR)) < 0) {
				fprintf(stderr, "unable to open %s: %s\n", dev_name, strerror(errno));
			    exit(-1);
				}
			/* First we seek */
			if ((x = lseek(fd, address, SEEK_SET)) < 0) {
			fprintf(stderr, "unable to seek %s: %s\n", dev_name, strerror(errno));
			}
			/* Second we read the requested memory */
			if ((x = write(fd, (char*) data, sizeof(data))) < 0) {
				fprintf(stderr, "unable to write %s: %s\n", dev_name, strerror(errno));
			}
			/*fprintf(stdout, "0x%x\n", c);*/
			if (fd >= 0) {
				close(fd);
			}	
			return data;
		}
		/* That's the two options we have! */
		else {
			fprintf(stderr, "registerAccess(): Something went wrong trying to understand what you wanted to do with the memory\n");
			return -1;
		}
}

