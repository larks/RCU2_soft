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

/*! 
 * @file     cmdInterpreter.h
 * @author   Lars Bratrud
 * @date
 * @brief    Command interpreter for the RCU shell
 *
 * @defgroup cmdinterpreter
 * Library for the RCU shell (rcu-sh) that interprets the command line
 * arguments at program invocation. 
 */

#ifndef _CMDINTERPRETER_H
#define _CMDINTERPRETER_H

#include <stdint.h>
#include <stdlib.h>

/*! 
 * Prints help for the commands
 * @return 0 if successful
 * 
*/
int printHelp();

/*!
 * Access registers on SmartFusion2
 * @return         Negative number when error, else it returns data in register.
 * @param address  Address to access.
 * @param data     Data to write, can be anything when only reading.
 * @param *rORw    Read ("r") or write ("w").
 * @ingroup cmdinterpreter
*/
uint32_t registerAccess(uint32_t address, uint32_t data, char * rORw);

/*!
 * Execute commands - main command parser.
 * @return         Nothing.
 * @param arg_count    Number of arguments.
 * @param **arguments  Array of arguments to interpret
 * @param *fp          Filepointer that describes where to print the results
                       in case we want to log things.  
 * @ingroup cmdinterpreter
*/
void executeCommands(uint32_t arg_count, char **arguments, FILE * fp);
unsigned int parseBinary(char *str);
unsigned int parseNumber(char *str);
//int printInfo();

#endif // _CMDINTERPRETER_H
