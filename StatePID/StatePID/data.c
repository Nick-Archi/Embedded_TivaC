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
#define MAX 255


//------------------------------------------
// Variables
//------------------------------------------
/*
 *  pingPong -> Array for holding right wall values
 *  bufferFlag -> flag to indicate which buffer to write to
 *  fullFlag -> flag to indicate when a buffer is full
 *  buffCount -> used to count till 60
 *  rightWallErr -> used to receive the right wall error value
 */
char pingPong1[65];//= {'b','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','e','\0'}; // array will hold 2 chars(command), 20 values, 11 spaces, + 1 null value
char pingPong2[65];//= {'a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','\0'};  // 2 chars, 20 * 2 chars, 11 chars, 1 char

extern float error;


int bufferFlag = true;
int buffCount = 4; // offset value so command takes [0] & [1]
int counter = 1;
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
//        int x = 0;writeCharToUart1('t');
//        for(x=0;x<65;x++){ writeCharToUart1(pingPong1[x]);}writeCharToUart1('p');
//            if(x>10)
//                writeCharToUart1('f');
//            if(x==60)writeCharToUart1('$');}
//        if(counter%20==0)
//        {WriteFrame(pingPong1);counter=0;}
//        counter++;
        //error=246;
        // get right wall value & convert to hex
        rightWallErr = (int32_t)error;

        // check if the value is negative or if it's > 255
        if(rightWallErr < 0){
            rightWallErr = abs(rightWallErr);
        }

        if(rightWallErr > MAX){
            rightWallErr = MAX; // cap the value at 255
        }
//        if(counter==1){writeCharToUart1('e');
//        writeCharToUart1('r');
//        writeCharToUart1(':');
//        writeCharToUart1(' ');
//        counter++;
 //       }

        // check what buffer to use
        /*
         * Converts the singed int -> hex value which is then stored
         * in the address location of the pingPong buffer. However, ex)
         * 246(decimal) => f6 (hex) => 66,36(in ascii) so this will take
         * up two indices...
         */
        if(bufferFlag){ // write to pingPong1

                    if(rightWallErr <= 15){
                        sprintf(&pingPong1[buffCount], "%c", ' ');
                        sprintf(&pingPong1[buffCount + 1], "%x", rightWallErr);
                        pingPong1[buffCount + 2] = ' '; // add the space
//                        writeCharToUart1(pingPong1[buffCount]);
//                        writeCharToUart1(pingPong1[buffCount+1]);
//                        writeCharToUart1(pingPong1[buffCount + 2]);
                    }
                    else{
                        sprintf(&pingPong1[buffCount], "%x", rightWallErr);
                        pingPong1[buffCount + 2] = ' '; // add the space
//                        writeCharToUart1(pingPong1[buffCount]);
//                        writeCharToUart1(pingPong1[buffCount+1]);
//                        writeCharToUart1(pingPong1[buffCount + 2]);
                    }

                    buffCount = buffCount + 3;
                }
                else{

                    if(rightWallErr <= 15){
                        sprintf(&pingPong2[buffCount], "%c", ' ');
                        sprintf(&pingPong2[buffCount + 1], "%x", rightWallErr);
                        pingPong2[buffCount + 2] = ' '; // add the space
//                        writeCharToUart1(pingPong2[buffCount]);
//                        writeCharToUart1(pingPong2[buffCount+1]);
//                        writeCharToUart1(pingPong2[buffCount + 2]);
                    }
                    else{
                        sprintf(&pingPong2[buffCount], "%x", rightWallErr);
                        pingPong2[buffCount + 2] = ' '; // add the space
//                        writeCharToUart1(pingPong2[buffCount]);
//                        writeCharToUart1(pingPong2[buffCount+1]);
//                        writeCharToUart1(pingPong2[buffCount + 2]);
                    }

                    buffCount = buffCount + 3;
                }

                // check if buffer is full
                if(buffCount == 64){

                    buffCount = 4;  // reset the value
//                    writeCharToUart1('\n');
//                    writeCharToUart1('\r');
//                    writeCharToUart1('e');
//                    writeCharToUart1('r');
//                    writeCharToUart1(':');
//                    writeCharToUart1(' ');
                    switch(bufferFlag){

                    case 0: // since pingPong2 is full switch to using pingPong1
                            bufferFlag = true;
                            pingPong2[0] = 'e';
                            pingPong2[1] = 'r';
                            pingPong2[2] = ' ';
                            pingPong2[3] = ' ';
                            pingPong2[64] = '\0';
                            break;

                    case 1:
                            bufferFlag = false;
                            pingPong1[0] = 'e';
                            pingPong1[1] = 'r';
                            pingPong1[2] = ' ';
                            pingPong1[3] = ' ';
                            pingPong2[64] = '\0';
                            break;

                    default:
                        bufferFlag = false;
                    }

                    // post TxDataSema
                    Semaphore_post(TxDataSema);

                }
    }

}

/*
 * ISR called by Timer to post DataSem
 */


/*
 *  Call StoreTxBufferPtr, to pass along the full buffer to pointer.
 */
void TxData(){

    while(true){

        Semaphore_pend(TxDataSema, BIOS_WAIT_FOREVER);
        // switch the bufferFlag, prepend command, add space too
        switch(!bufferFlag){

        case 0: // since pingPong2 is full switch to using pingPong1
                StoreTxBufferPtr_W(pingPong2);
                break;

        case 1:
                StoreTxBufferPtr_W(pingPong1);
                break;

        default:
            bufferFlag = false;
        }

    }
}
