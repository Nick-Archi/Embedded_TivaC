/*
 * RxCommands.c
 *
 * Local command buffer and other
 * variables are statically allocated here.
 */

//----------------------------------------
// BIOS header files
//----------------------------------------
#include <xdc/std.h>    //mandatory - have to include first, for BIOS types
#include <ti/sysbios/BIOS.h>  //mandatory - if you call APIs like BIOS_start()
#include <xdc/runtime/Log.h>  //needed for any Log_info() call
#include <xdc/cfg/global.h>


//------------------------------------------
// TivaWare Header Files
//------------------------------------------
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <data.h>
#include <drive.h>

#include "inc/hw_types.h"
#include "inc/hw_memmap.h"
#include "driverlib/sysctl.h"
#include "driverlib/gpio.h"
#include "inc/hw_ints.h"
#include "driverlib/interrupt.h"
#include "driverlib/timer.h"


//----------------------------------------
// Macros
//----------------------------------------
#define true 1
#define false 0


//------------------------------------------
// Variables
//------------------------------------------
int cmdLength;
int state;
static int count;

char* ptrC;
char localCmdBufferCopy[43]; // 20 values (40 indices), 2 command values, 1 null value

enum States {start, read, end}; // enum for state values

//------------------------------------------
// Functions
//------------------------------------------


/*
 *	ReadFrame() to read in the frame via the UART. ReadFrame() strips the
 *	leading colon, ending checksum, and CR/LF as they are read in and processed.
 *	Meaning those characters/values are not stored in the localCmdBuffer.
 *	Inside the localCmdBuffer is just a null-terminated string of characters
 *	consisting of 2-character command followed optionally by argument characters.
 *
 *	Variables:
 *	ptrC -> pointer used to copy the first address and use that to
 *		move and reference characters
 *	states -> used to move through state machine
 *	localCmdBufferCopy -> array used to hold the copy chars and later to
 *		return that address to localCmdBuffer
 *
 *
 *	copy from the recent command interpreter
 */
void ReadFrame(char* localCmdBuffer){ // non-interrupt

	switch(state){

		case start:
			ptrC = localCmdBuffer; // copy address of first value
			state = read; // move to next state
			break;

		case read:

			if(count < strlen(localCmdBuffer)){ // could maybe change this to look for '\0'

				if(*ptrC != '\r' || *ptrC != '\n'
						|| *ptrC != ':'){
					localCmdBufferCopy[count] = *ptrC; // copy value
					count++;
					ptrC++;
				}
				else{
					localCmdBufferCopy[count] = ' '; // erase the value?
					count++;
					ptrC++;
				}

			}
			else{ // done reading...move to next state
				state = end;
			}

			break;

		case end:

			localCmdBuffer = localCmdBufferCopy; // return from function?
			return;

		default:	// initialize state
			count = 0;
			state = start;
	}

} // end of ReadFrame non-interrupt


/*
 *	ReadFrame_i()
 */
void ReadFrame_i(char* localCmdBuffer){

}


/*
 *	StoreRecievedCommand_W(), gives new command buffer (localCmdBuffer)
 *		to command interpreter task. This function may wait on the
 *		CmdBuffer_RWLock
 *
 *	Posts NewCmdSema
 */
void StoreReceivedCommand_W(char* localCmdBuffer){

}
