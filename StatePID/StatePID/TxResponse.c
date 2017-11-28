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
#include <main.h>
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
int state;

enum states{start, move, space};

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
        WriteFrame(TXBufferPtr);
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
 *      data and insert the correct response
 */
void WriteFrame(char* bufferPointer){

    ptrC = bufferPointer;   // copy first address
        state = start;

        while(*ptrC != '\0'){
            switch(state){

                case start:
                    flag = 0;
                    state = move;
                    break;

                case move:
                    if(*ptrC == ' '){
                        state = space;
                        flag++;
                    }
                    else{
                        // print statement for char
                        writeCharToUart1(*ptrC);
                        ptrC++;
                    }
                    break;

                case space:
                    if(flag == 1){
                        // print statement for colon
                        writeCharToUart1(':');
                    }
                    else{
                        // print statement for space
                        writeCharToUart1(' ');
                    }
                    ptrC++;

                    if(*ptrC == '\0'){
                        // print statement for CR\LF
                        writeCharToUart1('\n');
                        writeCharToUart1('\r');
                    }
                    state = move;
                    break;

                default:
                    state = start;
            }
        }


}
