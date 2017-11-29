/*
 * main.c
 *
 *  Created on: Nov 1, 2017
 *      Author: Aurash
 */


//----------------------------------------
// BIOS header files
//----------------------------------------
#include <xdc/std.h>                    //mandatory - have to include first, for BIOS types
#include <ti/sysbios/BIOS.h>                //mandatory - if you call APIs like BIOS_start()
#include <xdc/runtime/Log.h>                //needed for any Log_info() call
#include <xdc/cfg/global.h>

#include <data.h>
#include <TxResponse.h>
#include <drive.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "inc/hw_gpio.h"
#include "inc/hw_ints.h"
#include "inc/hw_adc.h"
#include "inc/hw_uart.h"
#include "inc/hw_pwm.h"
#include "inc/hw_sysctl.h"
#include "driverlib/gpio.h"
#include "driverlib/pin_map.h"
#include "driverlib/sysctl.h"
#include "driverlib/uart.h"
#include "driverlib/pwm.h"
#include "driverlib/interrupt.h"
#include "driverlib/adc.h"
#include "driverlib/timer.h"
#include "driverlib/rom.h"
#include "driverlib/rom_map.h"
#include "driverlib/udma.h"
#include "driverlib/ssi.h"
#include "driverlib/systick.h"
#include "utils/uartstdio.h"
//#include "utils/uartstdio.c"
#include "driverlib/debug.h"
#include <string.h>
#include <main.h>

void writeStringToUart1(char* str);
void hard_initB();
void ledToggle();
void uart1int();
void adc_init();
void timerISR();
void Uturn();
static char pingPong1[65]= {'b','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','e','\0'}; // array will hold 2 chars(command), 20 values, 11 spaces, + 1 null value

volatile int16_t i16ToggleCount=0;
char comms[200];
int count = 0;
int limit=1700;int x=0;

int main(void) {

hard_initB();
adc_init();
memset(comms,'\0',sizeof(comms));
GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_6|GPIO_PIN_7, 0b10000000|0b01000000);
GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3, 0b00001110);
writeStringToUart1("Let's Talk: \n\r\t");
//for(x=0;x<65;x++){writeStringToUart1("a");    SysCtlDelay(SysCtlClockGet()/300);}
//Timer_start(DataClockFcn);
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
                ledToggle();Timer_start(DataClockFcn);
                i16ToggleCount++;
            }
            if(comms[count-2]=='b' && comms[count-1]=='b') {
                motorMove(200,200,1,1);
            }
            if(comms[count-2]=='s' && comms[count-1]=='t') {
                stopper=0;
            }
            if(comms[count-5]=='a' && comms[count-4]=='d') {
                char  adj[3];
                int i;
                for(i=2;i<count;i++){
                    adj[i-2]=comms[i];
                }
                kd=atof(adj);
            }
//            if(comms[count-5]=='l' && comms[count-4]=='i') {
//                          char  adj[3];
//                          int i;
//                          for(i=2;i<count;i++){
//                              adj[i-2]=comms[i];
//                          }
//                          light=atof(adj);
//                      }
            if(comms[count-5]=='a' && comms[count-4]=='p') {
                char  adj[3];
                int i;
                for(i=2;i<count;i++){
                    adj[i-2]=comms[i];
                }
                kp=atof(adj);
            }
            if(comms[count-5]=='a' && comms[count-4]=='i') {
                char  adj[3];
                int i;
                for(i=2;i<count;i++){
                    adj[i-2]=comms[i];
                }
                ki=atof(adj);
            }
            if(comms[count-2]=='d' && comms[count-1]=='r') {
                ReadWall_IR();
            }
            if(comms[count-2]=='d' && comms[count-1]=='f') {
                ReadFrontWall_US_W();
            }
            if(comms[count-2]=='f' && comms[count-1]=='f') {
                motorMove(200,200,0,0);
            }
            if(comms[count-2]=='s' && comms[count-1]=='s') {
                motorStop();
                //SysCtlPeripheralDisable(SYSCTL_PERIPH_TIMER2);
                Timer_stop(DriveClock);
            }
            if(comms[count-8]=='l' && comms[count-7]=='l') {
                            char  adj[6];
                            int i;
                            for(i=2;i<count;i++){
                                adj[i-2]=comms[i];
                            }
                            limit=atof(adj);
            }
            if(comms[count-2]=='p' && comms[count-1]=='d') {
                SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER2);
                //TimerEnable(TIMER2_BASE, TIMER_BOTH);
                //TimerEnable(TIMER2_BASE, TIMER_B);
                //                    Timer_start(timer0);
                //                    Timer_start(timer1);
                Timer_start(DriveClock);
                Timer_start(timer0);
            }
            if(comms[count-2]=='o' && comms[count-1]=='f')
                GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3, 0);
            UARTCharPutNonBlocking(UART1_BASE, '\n');
            UARTCharPutNonBlocking(UART1_BASE, '\r');
            //writeStringToUart1("You just said");
            //UARTCharPutNonBlocking(UART1_BASE, ' ');
            //writeStringToUart1(comms);
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


void DriveClockFunc(){
    Semaphore_post(DriveSema);
}
void DataClockFn(){

    Semaphore_post(DataSema);

}
void hard_initB() {
    SysCtlClockSet(SYSCTL_SYSDIV_5 | SYSCTL_USE_PLL | SYSCTL_OSC_MAIN | SYSCTL_XTAL_16MHZ);
    //
    // Enable Peripheral Clocks
    //
    //Set the clock
   // SysCtlClockSet(SYSCTL_SYSDIV_1 | SYSCTL_USE_OSC |   SYSCTL_OSC_MAIN | SYSCTL_XTAL_16MHZ);

       //This marks the beggining of timer int setup
//    uint32_t ui32Period;
//        // Timer 2 setup code
//        SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER2);           // enable Timer 2 periph clks
//        TimerConfigure(TIMER2_BASE, TIMER_CFG_PERIODIC);        // cfg Timer 2 mode - periodic
//
//        ui32Period = (SysCtlClockGet() /20);                     // period = CPU clk div 2 (500ms)
//        TimerLoadSet(TIMER2_BASE, TIMER_A, ui32Period);         // set Timer 2 period
//
//        TimerIntEnable(TIMER2_BASE, TIMER_TIMA_TIMEOUT);        // enables Timer 2 to interrupt CPU

        //TimerEnable(TIMER2_BASE, TIMER_A);                      // enable Timer 2
       //This marks the end of timer int setup

    //Configure PWM Clock to match system
    SysCtlPWMClockSet(SYSCTL_PWMDIV_1);

    SysCtlPeripheralEnable(SYSCTL_PERIPH_UART1);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOC);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
    //SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_PWM1); //enable PWM module 1
    GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3); //Enable GPIO output
    GPIOPinTypeGPIOOutput(GPIO_PORTB_BASE, GPIO_PIN_6|GPIO_PIN_7); //Enable GPIO pins PB6 and PB7
    //enable PWM pins PE4 and PE5
    GPIOPinTypePWM(GPIO_PORTA_BASE, GPIO_PIN_6 | GPIO_PIN_7);

    //Configure PWM pins PE4 and PE5
    GPIOPinConfigure(GPIO_PA6_M1PWM2);
    GPIOPinConfigure(GPIO_PA7_M1PWM3);



    //Configure PWM Options
    //PWM_GEN_1 Covers M1PWM2 and M1PWM3
    //PWM_GEN_2 Covers M1PWM4 and M1PWM5
    //PWM_GEN_3 Covers M1PWM6 and M1PWM7 See page 207 4/11/13 DriverLib doc
    PWMGenConfigure(PWM1_BASE, PWM_GEN_1, PWM_GEN_MODE_DOWN | PWM_GEN_MODE_NO_SYNC);

    //Set the Period (expressed in clock ticks)
    PWMGenPeriodSet(PWM1_BASE, PWM_GEN_1, 320);

    //Set PWM duty-50% (Period /2)
    PWMPulseWidthSet(PWM1_BASE, PWM_OUT_2,100);
    PWMPulseWidthSet(PWM1_BASE, PWM_OUT_3,100);

    // Enable the PWM generator
    PWMGenEnable (PWM1_BASE, PWM_GEN_1);

    // Turn on the Output pins
    PWMOutputState (PWM1_BASE, PWM_OUT_3_BIT | PWM_OUT_2_BIT, false);

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
        UARTCharPutNonBlocking(UART1_BASE,str[i]);
        memset(comms,'\0',sizeof(comms));
}

void writeCharToUart1(char str)   //write a string to Uart0
{
        UARTCharPutNonBlocking(UART1_BASE,str);
        SysCtlDelay(SysCtlClockGet()/30000);
    //UARTprintf("%c",str);
    memset(comms,'\0',sizeof(comms));
}
void adc_init(){

    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE);
    //enable ADC0
    SysCtlPeripheralEnable(SYSCTL_PERIPH_ADC0);
    //configure Pin PE3 (AIN0) for ADC usage
    GPIOPinTypeADC(GPIO_PORTE_BASE, GPIO_PIN_3);
    //Before Configuring ADC Sequencer 3, it should be OFF
    ADCSequenceDisable(ADC0_BASE, 3);
    //configure ADC sequencer
    ADCSequenceConfigure(ADC0_BASE, 3, ADC_TRIGGER_PROCESSOR, 0);
    //configure ADC sequencer steps
    ADCSequenceStepConfigure(ADC0_BASE, 3, 0, ADC_CTL_CH0|ADC_CTL_IE|ADC_CTL_END);
    //enable ADC sequencer
    ADCSequenceEnable(ADC0_BASE, 3);


    //enable ADC1
    SysCtlPeripheralEnable(SYSCTL_PERIPH_ADC1);
    //configure Pin PE2 (AIN1) for ADC usage
    GPIOPinTypeADC(GPIO_PORTE_BASE, GPIO_PIN_2);
    //Before Configuring ADC Sequencer 2, it should be OFF
    ADCSequenceDisable(ADC1_BASE, 3);
    //configure ADC sequencer
    ADCSequenceConfigure(ADC1_BASE, 3, ADC_TRIGGER_PROCESSOR, 0);
    //configure ADC sequencer steps
    ADCSequenceStepConfigure(ADC1_BASE, 3, 0, ADC_CTL_CH1|ADC_CTL_IE|ADC_CTL_END);
    //enable ADC sequencer
    ADCSequenceEnable(ADC1_BASE, 3);

}
