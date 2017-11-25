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
#define MAX 255


//------------------------------------------
// Variables
//------------------------------------------
/*
 *	pingPong -> Array for holding right wall values
 *	distRt -> Distance value from right sensor
 *	bufferFlag -> flag to indicate which buffer to write to
 *	fullFlag -> flag to indicate when a buffer is full
 *	buffCount -> used to count to 20
 *	rightWallErr -> used to receive the right wall error value
 *	fullBufferPtr -> global pointer used to hold full buffer value
 */
char pingPong1[47];	// command value takes [0] & [1], Null takes [44], and colon is [2]
char pingPong2[47];

extern float error;
extern char comms[200]; // milestone 9

static int bufferFlag = true;
static int buffCount = 3; // offset value so command takes [0] & [1]

int32_t rightWallErr;

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
		rightWallErr = (int32_t)error;

		// check if the value is negative or if it's > 255
		if(rightWallErr < 0){
			rightWallErr = abs(rightWallErr);
		}

		if(rightWallErr > MAX){
			rightWallErr = MAX;	// cap the value at 255
		}


		// check what buffer to use
		/*
		 * Converts the singed int -> hex value which is then stored
		 * in the address location of the pingPong buffer. However, ex)
		 * 246(decimal) => f6 (hex) => 66,36(in ascii) so this will take
		 * up two indices...
		 */
		if(bufferFlag){ // write to pingPong1
			sprintf(&pingPong1[buffCount], "%x ", rightWallErr);
			buffCount = buffCount + 2;
		}
		else{
			sprintf(&pingPong2[buffCount], "%x ", rightWallErr);
			buffCount = buffCount + 2;
		}

		// check if buffer is full
		if(buffCount == 43){ // 20 * 2(values) + 3(offset)

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
 *	Call StoreTxBufferPtr, to pass along the full buffer to pointer.
 */
void TxData(){

	while(true){

		Semaphore_pend(TxDataSema, BIOS_WAIT_FOREVER);

		buffCount = 0;

		// switch the bufferFlag, prepend command, add null value, and CR/LF
		switch(bufferFlag){

		case 0: // switch to pingPong1
				bufferFlag = true;
				pingPong2[0] = 'E';
				pingPong2[1] = 'R';
//				pingPong2[2] = ':';
				pingPong2[2] = ' '; // add spaces for detection in writeFrame
				pingPong2[44] = '\0';	// end of data frame?
//				pingPong2[45] = '\n';
				pingPong2[45] = ' ';
//				pingPong2[46] = '\r';
				pingPong2[46] = ' ';
				StoreTxBufferPtr_W(pingPong2);
				break;

		case 1:
				bufferFlag = false;
				pingPong1[0] = 'E';
				pingPong1[1] = 'R';
//				pingPong1[2] = ':';
				pingPong1[2] = ' '; // add spaces for detection in writeFrame
				pingPong1[44] = '\0';
//				pingPong1[45] = '\n';
				pingPong1[45] = ' ';
//				pingPong1[46] = '\r';
				pingPong1[46] = ' ';
				StoreTxBufferPtr_W(pingPong1);
				break;

		default:
			bufferFlag = false;
		}

	}
}
