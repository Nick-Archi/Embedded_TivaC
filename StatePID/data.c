/*
 * data.c
 *
 * Statically allocate variables for communication between AcquireData and
 * TxData, including the ping pong buffers.
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
/*
 *	pingPong -> Array for holding right wall values
 *	distRt -> Distance value from right sensor
 *	bufferFlag -> flag to indicate which buffer to write to
 *	fullFlag -> flag to indicate when a buffer is full
 *	buffCount -> used to count to 20
 *	rightWallDist -> used to receive the right wall distance
 *	fullBufferPtr -> global pointer used to hold full buffer value
 */
char pingPong1[20];
char pingPong2[20];
char* fullBufferPtr;

extern float distRt;

static int bufferFlag = true;
static int fullFlag = false;
static int buffCount = 0;

uint32_t rightWallDist;


//------------------------------------------
// Functions
//------------------------------------------

/*
 * AcquireData pends on the DataSema
 *
 * DataSem -> Called By Timer2 every 100ms once the robot passes the first
 * "data" line
 */
void AcquireData(){
	while(true){

		Semaphore_pend(DataSema, BIOS_WAIT_FOREVER);

		// get right wall value & convert to hex
		rightWallDist = (uint32_t)distRt;

		// check what buffer to use
		if(bufferFlag){ // write to first buffer
//			pingPong1[buffCount] = rightWallDist;
//			buffCount++;
			sprintf(pingPong1[buffCount], "%x ", rightWallDist);
			buffCount++;
		}
		else{
//			pingPong2[buffCount] = rightWallDist;
//			buffCount++;
			sprintf(pingPong2[buffCount], "%x ", rightWallDist);
			buffCount++;
		}

		// check if buffer is full
		if(buffCount == 20){

			// post TxDataSema
			Semaphore_post(TxDataSema);

		}
	}
}


/*
 * ISR called by Timer to post DataSem
 */
void DataClockFn(){

	Semaphore_post(DataSema);

}

/*
 * Function to copy full buffer
 * funciton may wait on the TX_RWLock?
 */
void StoreTxBufferPtr_W(char* bufferPtr){

	fullBufferPtr = bufferPtr;

}

/*
 *	Call StoreTxBufferPtr, to pass along the full buffer to pointer.
 */
void TxData(){

	while(true){

		Semaphore_pend(TxDataSema, BIOS_WAIT_FOREVER);

		buffCount = 0;

		// switch the bufferFlag
		switch(bufferFlag){

		case 0: // switch to pingPong2
				bufferFlag = true;
				StoreTxBufferPtr_W(pingPong1);
				break;

		case 1:
				bufferFlag = false;
				StoreTxBufferPtr_W(pingPong2);
				break;

		default:
			bufferFlag = false;
		}

	}
}
