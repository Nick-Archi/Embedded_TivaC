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
char* fullBufferPtr;


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




/*
 * Function to copy full buffer
 * function may wait on the TX_RWLock?
 */
void StoreTxBufferPtr_W(char* bufferPtr){

	fullBufferPtr = bufferPtr;

}
