/*
//---------------------------------------------------------------------------------
// Project: Blink TM4C BIOS Using Hwi (STARTER)
// Author: Eric Wilbur
// Date: June 2014
//
// Note: The function call TimerIntClear(TIMER2_BASE, TIMER_TIMA_TIMEOUT) HAS
//       to be in the ISR. This fxn clears the TIMER's interrupt flag coming
//       from the peripheral - it does NOT clear the CPU interrupt flag - that
//       is done by hardware. The author struggled figuring this part out - hence
//       the note. And, in the Swi lab, this fxn must be placed in the
//       Timer_ISR fxn because it will be the new ISR.
//
// Follow these steps to create this project in CCSv6.0:
// 1. Project -> New CCS Project
// 2. Select Template:
//    - TI-RTOS for Tiva-C -> Driver Examples -> EK-TM4C123 LP -> Example Projects ->
//      Empty Project
//    - Empty Project contains full instrumentation (UIA, RTOS Analyzer) and
//      paths set up for the TI-RTOS version of MSP430Ware
// 3. Delete the following files:
//    - Board.h, empty.c, EK_TM4C123GXL.c/h, empty_readme.txt
// 4. Add main.c from TI-RTOS Workshop Solution file for this lab
// 5. Edit empty.cfg as needed (to add/subtract) BIOS services, delete given Task
// 6. Build, load, run...
//----------------------------------------------------------------------------------


//----------------------------------------
// BIOS header files
//----------------------------------------
#include <xdc/std.h>  						//mandatory - have to include first, for BIOS types
#include <ti/sysbios/BIOS.h> 				//mandatory - if you call APIs like BIOS_start()
#include <xdc/runtime/Log.h>				//needed for any Log_info() call
#include <xdc/cfg/global.h> 				//header file for statically defined objects/handles


//------------------------------------------
// TivaWare Header Files
//------------------------------------------
#include <stdint.h>
#include <stdbool.h>

#include "inc/hw_types.h"
#include "inc/hw_memmap.h"
#include "driverlib/sysctl.h"
#include "driverlib/gpio.h"
#include "inc/hw_ints.h"
#include "driverlib/interrupt.h"
#include "driverlib/timer.h"


//----------------------------------------
// Prototypes
//----------------------------------------
void hardware_init(void);
void ledToggle(void);
void delay(void);


//---------------------------------------
// Globals
//---------------------------------------
volatile int16_t i16ToggleCount = 0;


//---------------------------------------------------------------------------
// main()
//---------------------------------------------------------------------------
void main(void)
{

   hardware_init();							// init hardware via Xware

   BIOS_start();

}


//---------------------------------------------------------------------------
// hardware_init()
//
// inits GPIO pins for toggling the LED
//---------------------------------------------------------------------------
void hardware_init(void)
{
	uint32_t ui32Period;

	//Set CPU Clock to 40MHz. 400MHz PLL/2 = 200 DIV 5 = 40MHz
	SysCtlClockSet(SYSCTL_SYSDIV_5|SYSCTL_USE_PLL|SYSCTL_XTAL_16MHZ|SYSCTL_OSC_MAIN);

	// ADD Tiva-C GPIO setup - enables port, sets pins 1-3 (RGB) pins for output
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
	GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3);

	// Turn on the LED
	GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3, 4);

	// Timer 2 setup code
	SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER2);			// enable Timer 2 periph clks
	TimerConfigure(TIMER2_BASE, TIMER_CFG_PERIODIC);		// cfg Timer 2 mode - periodic

	ui32Period = (SysCtlClockGet() /2);						// period = CPU clk div 2 (500ms)
	TimerLoadSet(TIMER2_BASE, TIMER_A, ui32Period);			// set Timer 2 period

	TimerIntEnable(TIMER2_BASE, TIMER_TIMA_TIMEOUT);		// enables Timer 2 to interrupt CPU

	TimerEnable(TIMER2_BASE, TIMER_A);						// enable Timer 2

}


//---------------------------------------------------------------------------
// ledToggle()
//
// toggles LED on Tiva-C LaunchPad
//---------------------------------------------------------------------------
void ledToggle(void)
{
    TimerIntClear(TIMER2_BASE, TIMER_TIMA_TIMEOUT);			// must clear timer flag FROM timer

	// LED values - 2=RED, 4=BLUE, 8=GREEN
	if(GPIOPinRead(GPIO_PORTF_BASE, GPIO_PIN_2))
	{
		GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3, 0);
	}
	else
	{
		GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3, 8|4|2);
	}

	//delay();												// create a delay of ~1/2sec

	i16ToggleCount += 1;									// keep track of #toggles

	Log_info1("LED TOGGLED [%u] TIMES",i16ToggleCount);		// send toggle count to UIA

}


//---------------------------------------------------------------------------
// delay()
//
// Creates a 500ms delay via TivaWare fxn
//---------------------------------------------------------------------------
void delay(void)
{
	 SysCtlDelay(6700000);		// creates ~500ms delay - TivaWare fxn

}

*/

/*
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "driverlib/gpio.h"
#include "driverlib/pin_map.h"
#include "driverlib/sysctl.h"
#include "driverlib/uart.h"
#include "inc/hw_ints.h"
#include "driverlib/interrupt.h"
#include <string.h>

void writeStringToUart1(char* str);
void hard_initB();
void ledToggle();

volatile int16_t i16ToggleCount=0;

int main(void) {

hard_initB();
writeStringToUart1("Let's Talk: \n");

 char *f= (char*)malloc(200);
 int i = 0;
 //int done=0;
 while (1)
 {

     while(UARTCharsAvail(UART1_BASE) )
     {

     char x =  UARTCharGet(UART1_BASE);
     if(x==10 || x==13)
     {
      if(f[i-2]=='L' && f[i-1]=='o'){ledToggle();i16ToggleCount++;}
      if(f[i-2]=='o' && f[i-1]=='f'){GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3, 0);}
      UARTCharPut(UART1_BASE, '\n');
      UARTCharPut(UART1_BASE, '\r');
      writeStringToUart1("You just said");
      UARTCharPut(UART1_BASE, '\n');
      UARTCharPut(UART1_BASE, '\r');
      UARTCharPut(UART1_BASE, '\t');
      writeStringToUart1(f);
      UARTCharPut(UART1_BASE, '\n');
      UARTCharPut(UART1_BASE, '\r');
      writeStringToUart1("Let's Talk: \n\r");
     }
     UARTCharPut(UART1_BASE,x);
     f[i]=x;
     i++;


 }
 }
}

void ledToggle(void)
{
    // LED values - 2=RED, 4=BLUE, 8=GREEN

        if(i16ToggleCount%3==0)
               GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3, 2);
        if(i16ToggleCount%3==1)
               GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3, 4);
        if(i16ToggleCount%3==2)
               GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3, 8);
    }

void hard_initB(){
    SysCtlClockSet(SYSCTL_SYSDIV_4 | SYSCTL_USE_PLL | SYSCTL_OSC_MAIN | SYSCTL_XTAL_16MHZ);
    //
    // Enable Peripheral Clocks
    //
    SysCtlPeripheralEnable(SYSCTL_PERIPH_UART1);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOC);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
    GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3);
    //
    // Enable port PC4 for UART1 U1RX
    //
    GPIOPinTypeUART(GPIO_PORTC_BASE, GPIO_PIN_4);
    GPIOPinConfigure(GPIO_PC4_U1RX);

    //
    // Enable port PC5 for UART1 U1TX
    //
    GPIOPinTypeUART(GPIO_PORTC_BASE, GPIO_PIN_5);
    GPIOPinConfigure(GPIO_PC5_U1TX);
    //
    UARTConfigSetExpClk(UART1_BASE, SysCtlClockGet(), 115200,
    (UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE | UART_CONFIG_PAR_NONE));
}


void writeStringToUart1(char* str)   //write a string to Uart0
{
    int i;
    for (i = 0; i < strlen(str); i++)
        UARTCharPut(UART1_BASE,str[i]);
}
*/










//----------------------------------------
// BIOS header files
//----------------------------------------
#include <xdc/std.h>  						//mandatory - have to include first, for BIOS types
#include <ti/sysbios/BIOS.h> 				//mandatory - if you call APIs like BIOS_start()
#include <xdc/runtime/Log.h>				//needed for any Log_info() call
#include <xdc/cfg/global.h>


#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "driverlib/gpio.h"
#include "driverlib/pin_map.h"
#include "driverlib/sysctl.h"
#include "driverlib/uart.h"
#include "inc/hw_ints.h"
#include "driverlib/interrupt.h"
#include <string.h>

void writeStringToUart1(char* str);
void hard_initB();
void ledToggle();
void uart1int();

volatile int16_t i16ToggleCount=0;
int count = 0;
char comms[200];

int main(void) {

hard_initB();
memset(comms,'\0',sizeof(comms));
writeStringToUart1("Let's Talk: \n\r\t");
BIOS_start();



}

void uart1int(void){
      uint32_t ui32Status;
      ui32Status = UARTIntStatus(UART1_BASE, true); //get interrupt status
      UARTIntClear(UART1_BASE, ui32Status);
      while(UARTCharsAvail(UART1_BASE) ) {
            char x =  UARTCharGetNonBlocking(UART1_BASE);
            if(x == 10 || x == 13){
                if(comms[count-2]=='L' && comms[count-1]=='o') {
                    ledToggle();
                    i16ToggleCount++;
                }
                if(comms[count-2]=='o' && comms[count-1]=='f')
                    GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3, 0);
                UARTCharPutNonBlocking(UART1_BASE, '\n');
                UARTCharPutNonBlocking(UART1_BASE, '\r');
                writeStringToUart1("You just said");
                UARTCharPutNonBlocking(UART1_BASE, ' ');
                writeStringToUart1(comms);
                writeStringToUart1("\n\rLet's Talk: \n\r\t");
                count = 0;
                memset(comms,'\0',sizeof(comms));
                break;
            }
            UARTCharPutNonBlocking(UART1_BASE,x);
            comms[count] = x;
            count++;
      }
}

void ledToggle(void)
{
    // LED values - 2=RED, 4=BLUE, 8=GREEN

        if(i16ToggleCount%3==0)
               GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3, 2);
        if(i16ToggleCount%3==1)
               GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3, 4);
        if(i16ToggleCount%3==2)
               GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3, 8);
    }

void hard_initB(){
    SysCtlClockSet(SYSCTL_SYSDIV_4 | SYSCTL_USE_PLL | SYSCTL_OSC_MAIN | SYSCTL_XTAL_16MHZ);
    //
    // Enable Peripheral Clocks
    //
    SysCtlPeripheralEnable(SYSCTL_PERIPH_UART1);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOC);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
    GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3);
    //
    // Enable port PC4 for UART1 U1RX
    //
    GPIOPinTypeUART(GPIO_PORTC_BASE, GPIO_PIN_4);
    GPIOPinConfigure(GPIO_PC4_U1RX);

    //
    // Enable port PC5 for UART1 U1TX
    //
    GPIOPinTypeUART(GPIO_PORTC_BASE, GPIO_PIN_5);
    GPIOPinConfigure(GPIO_PC5_U1TX);
    //
    UARTConfigSetExpClk(UART1_BASE, SysCtlClockGet(), 115200,
    (UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE | UART_CONFIG_PAR_NONE));


       //MAKE SUR TO ENABLE IN THIS ORDER
       IntEnable(INT_UART1);
       UARTIntEnable(UART1_BASE, UART_INT_RT|UART_INT_RX);
       IntMasterEnable();
}


void writeStringToUart1(char* str)   //write a string to Uart0
{
    int i;
    for (i = 0; i < strlen(str); i++)
        UARTCharPut(UART1_BASE,str[i]);
}







