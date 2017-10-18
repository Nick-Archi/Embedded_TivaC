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
#include "inc/hw_adc.h"
#include "inc/hw_ints.h"
#include "inc/hw_sysctl.h"
#include "inc/hw_gpio.h"
#include "driverlib/debug.h"
#include "driverlib/sysctl.h"
#include "driverlib/adc.h"


void distFront();
void distRight();


int main(void)
{
    SysCtlClockSet(SYSCTL_SYSDIV_4 | SYSCTL_USE_PLL | SYSCTL_OSC_MAIN | SYSCTL_XTAL_16MHZ);



    /* enable ADC0 or ADC1 peripheral */
    SysCtlPeripheralEnable(SYSCTL_PERIPH_ADC0);

    /* Reset to Apply changes */
    SysCtlPeripheralReset(SYSCTL_PERIPH_ADC0);

    /* Disable the sequencer of ADC0, sequencer 3(one sample)*/
    ADCSequenceDisable(ADC0_BASE,3);

    /* Configure the sequencer */
    ADCSequenceConfigure(ADC0_BASE,3,ADC_TRIGGER_PROCESSOR,0);

    /* Configure steps for the sequence */
    ADCSequenceStepConfigure(ADC0_BASE,3,0,ADC_CTL_CH0/*AIN0*/|ADC_CTL_IE|ADC_CTL_END);

    /* Enable GPIO port E */
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE);

    /* Activate PE3 as ADC */
    GPIOPinTypeADC(GPIO_PORTE_BASE,GPIO_PIN_3);

    /*  */
    //    ADCIntRegister(ADC0_BASE,3,&ISR_ADC_Read);

    /* Enable interrupt for ADC0 */
    ADCIntEnable(ADC0_BASE,3);

    /* Re-enable the sequencer */
    ADCSequenceEnable(ADC0_BASE,3);

    /* Enable global interrupts */
    IntMasterEnable();

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

       distFront();
       distRight();
}

void writeStringToUart1(char* str)   //write a string to Uart0
{
    int i;
    for (i = 0; i < strlen(str); i++)
        UARTCharPut(UART1_BASE,str[i]);
}



void distFront(){
   // uint32_t voltage;
    uint32_t result;
   // while(1)
    //{
        /* Clear interrupt flag ADC0 sequencer 3*/
        ADCIntClear(ADC0_BASE,3);

        /* Post request for ADC converstion  */
        ADCProcessorTrigger(ADC0_BASE,3);

        /* Wait for conversion */
        while(!ADCIntStatus(ADC0_BASE,3,false)){}

        /* Get data */
        ADCSequenceDataGet(ADC0_BASE,3,&result);

        /* Convert digital back to analog */
       // voltage = result*.000805664;
        char  str[50];
        sprintf(str,"%u\n",result);
        writeStringToUart1(str);
       // printf("%u\t%u\n",result,voltage);
//    }
}

void distRight(){
    //uint32_t voltage;
    uint32_t result;
   // while(1)
    //{
        /* Clear interrupt flag ADC0 sequencer 3*/
        ADCIntClear(ADC0_BASE,3);

        /* Post request for ADC converstion  */
        ADCProcessorTrigger(ADC0_BASE,3);

        /* Wait for conversion */
        while(!ADCIntStatus(ADC0_BASE,3,false)){}

        /* Get data */
        ADCSequenceDataGet(ADC0_BASE,3,&result);

        /* Convert digital back to analog */
     //   voltage = result*.000805664;
        char  str[50];
        sprintf(str,"%u\n",result);
        writeStringToUart1(str);
       // printf("%u\t%u\n",result,voltage);
//    }
}
