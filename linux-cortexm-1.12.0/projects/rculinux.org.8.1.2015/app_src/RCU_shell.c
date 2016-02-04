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
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "cmdInterpreter.h"

char **alloc_array(int);

/* File pointers */
FILE *fp = NULL; /* where to print results */
FILE * fd = NULL; /* for batch file */

int main(int argc, char **argv)
{
	/* Check for arguments and print help prompt */
	if(argc < 2){
		//fprintf(stderr, "");
		printHelp();
		return -1;
	}
	
	/* Check for comments */
	uint32_t argcBuffer = 0;
	while( argcBuffer != argc ){
		if( argv[argcBuffer][0] == '#')
			break;
		argcBuffer++;
	}
	/* check for read or write */
	if( (argv[1][0] == 'r') || /* read */
	    (argv[1][0] == 'w') || /* write */
	    (argv[1][0] == 'c') || /* write zero */
	    (argv[1][0] == 's') || /* Ask for serial number */
	    (argv[1][0] == 'e') ){ /* log message */
			executeCommands(argcBuffer-1, argv+1, fp);
	}
	/* Run command execution on each line in file */
	else if(argv[1][0] == 'b'){
		if ((fd = fopen(argv[2], "r")) < 0) {
			fprintf(stderr, "unable to open file\n");
		    exit(-1);
		}
		
		int kar; 
		int space_kar = 0; /* Character holder */
		uint32_t newline_count = 0; /* Line counter */
		unsigned long karteller = 0; /* Character counter */
		uint32_t strengteller = 0; /* String counter */
		uint32_t biggest = 0; /* Biggest number of strings */
		uint32_t storkar = 0; /* Hold the size of the largest string */
		uint8_t comment = 0; /* Indicate if we reached a comment */
		uint32_t current_pos = 0;
		uint32_t last_pos = ftell(fd);
		
		while ( (kar=fgetc(fd)) != EOF){
		if( kar == '#' ) comment = 1; /* we have a comment, stop counting */
		if( (kar != ' ') && (!comment) && (kar != '\r') ){
			karteller++;
			space_kar = 0;
		}
		if( (kar == ' ') && (!comment) ){
			if(space_kar<1){
				strengteller++;
			}
			++space_kar;
			if(karteller>storkar) storkar = karteller;
			karteller = 0;
		}
		if(kar == '\n'){
			strengteller++;
			newline_count++;
			if(strengteller>biggest) biggest=strengteller;
			strengteller=0;
			karteller = 0;
			comment = 0;
			current_pos = ftell(fd);
				//int l;
				char vikar[storkar];
				char **argBuf;
				argBuf = alloc_array(biggest);
				fseek(fd, last_pos, SEEK_SET); // seek to start of line
				last_pos = current_pos;
				while( (kar=fgetc(fd)) != '\n' ){
					if( kar == '#' ) comment = 1;
					if( (kar != ' ') && (!comment) && (kar != '\r') ){ 
						vikar[karteller] = kar;
						karteller++;
					}
					if( (kar == ' ') && (!comment) ){
						strncpy(argBuf[strengteller], vikar, karteller);
						strengteller++;
						karteller = 0;
						/*vikar = '\0';*/
						vikar[0] = '\0';
					}
				}
				strncpy(argBuf[strengteller], vikar, karteller);
/*				fprintf(stdout, "strengteller: %d\n", strengteller);*/
/*				int i;*/
/*				for(i=0; i<=strengteller; i++) fprintf(stdout, "argBuf[%d]: %s\n", i, argBuf[i]);*/
				fseek(fd, last_pos, SEEK_SET);
				executeCommands(strengteller+1, argBuf, fp);
				/* Free up */
				biggest = 0;
				storkar = 0;
				karteller = 0;
				comment = 0;
				strengteller = 0;
				free(argBuf);
		}
		}
		
	}
	else
		fprintf(stdout, "Command not recognized!\n");
	/* This is the end */
	if (fd != NULL) fclose(fd); /* close batch file */
 	if( (fp != stdout) && (fp != NULL) ) fclose(fp);
	return 0;
}

char **alloc_array(int columns) {
    char **a = (char **) calloc(columns, sizeof(char *));
    int i;
    for(i = 0; i < columns; i++) {
        a[i] = (char *) calloc(1, sizeof(char ));
    }
    return a;
}
