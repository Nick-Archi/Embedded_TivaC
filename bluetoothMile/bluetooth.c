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
