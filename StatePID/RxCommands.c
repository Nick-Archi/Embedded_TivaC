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
char* ptrV;
char localCmdBuffer[42];

//------------------------------------------
// Functions
//------------------------------------------


/*
 *	Function to read in the frame via the UART. ReadFrame() strips the
 *	leading colon, ending checksum, and CR/LF as they are read in and processed.
 *	Meaning those characters/values are not stored in the localCmdBuffer.
 *	Inside the localCmdBuffer is just a null-terminated string of characters
 *	consisting of 2-character command followed optionally by argument characters.
 */
void ReadFrame(){ // non-interrupt




}
