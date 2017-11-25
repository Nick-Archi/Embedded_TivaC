/*
 * TxResponse.c
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
#include <data.h>
#include <drive.h>
#include <TxResponse.h>

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
char* TXBufferPtr;
char* ptrC;

int flag;


//------------------------------------------
// Functions
//------------------------------------------

/*
 * TxResponse Task
 * call WriteFrame(char* bufferPointer) to transmit data found at
 * TX Buffer Pointer. Data is null terminated response string. WriteFrame's
 * state machine inserts colon, checksum, and CR/LF as it writes character
 * to UART
 */
void TxResponse(){

	while(true){

		Semaphore_pend(TxResponseSema, BIOS_WAIT_FOREVER);

	}

}


/*
 * Function to copy full buffer
 * function may wait on the TX_RWLock?
 */
void StoreTxBufferPtr_W(char* fullBufferPtr){

	TXBufferPtr = fullBufferPtr;
	Semaphore_post(TxResponseSema);


}


/*
 * Contains state machine to parse through data and look for ' ' (spaces) in the
 * 		data and insert the correct response
 */
void WriteFrame(char* bufferPointer){

	switch(state){

		case start:
			ptrC = bufferPointer;
			flag = 0;
			state = move; // go to "move" state
			break;

		case s1:
			if(flag == 1){

			}
			else if(flag == 2){

			}
			else if(flag == 3){

			}

			break;

		case check:
			if(*ptrC == ' '){	// detects space
				flag++;
				state = s1;
			}
			else{
				state = print;
			}
			break;

		case print:
					// print to the UART here...
			ptrC++;


			break;

		case stop:

			break;

		default:
			state = start;
	}

}
