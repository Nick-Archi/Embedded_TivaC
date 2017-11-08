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
#include "utils/uartstdio.c"
#include "driverlib/debug.h"
#include <string.h>





void writeStringToUart1(char* str);
void hard_initB();
void ledToggle();
void uart1int();
void motorMove(unsigned long leftPower, unsigned long rightPower, int ,int);
void motorStop();
float distFront();
float distRight();
void adc_init();
void timerISR();
void Uturn();

volatile int16_t i16ToggleCount=0;
float error_prior = 0;
float integral = 0;
int i=100;
float kp = .5;
float ki = .05;
float kd = .2;
uint32_t ui32ADC1Value[2] = {};
uint32_t ui32ADC0Value[2] = {};
uint32_t result=0;
uint32_t result1=0;
int count = 0;
int stopper=1;
char comms[200];

int main(void) {

hard_initB();
adc_init();
memset(comms,'\0',sizeof(comms));
GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_6|GPIO_PIN_7, 0b10000000|0b01000000);
GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3, 0b00001110);
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
                if(comms[count-2]=='b' && comms[count-1]=='b') {
                    motorMove(200,200,1,1);
                }
                if(comms[count-2]=='s' && comms[count-1]=='t') {
                                    stopper=0;
                                }
                if(comms[count-5]=='a' && comms[count-4]=='d') {
                    char  adj[3];
                    int i;
                    for(i=3;i<count;i++){
                           adj[0]=comms[count-i];
                                 }
                    kd=atof(adj);
                                }
                if(comms[count-5]=='a' && comms[count-4]=='p') {
                                    char  adj[3];
                                    int i;
                                    for(i=3;i<count;i++){
                                           adj[0]=comms[count-i];
                                                 }
                                    kp=atof(adj);
                                                }
                if(comms[count-5]=='a' && comms[count-4]=='i') {
                                    char  adj[3];
                                    int i;
                                    for(i=3;i<count;i++){
                                           adj[0]=comms[count-i];
                                                 }
                                    ki=atof(adj);
                                                }
                if(comms[count-2]=='d' && comms[count-1]=='r') {
                       distRight();
                  }
                if(comms[count-2]=='d' && comms[count-1]=='f') {
                    distFront();
                  }
                if(comms[count-2]=='f' && comms[count-1]=='f') {
                    motorMove(200,200,0,0);
                }
                if(comms[count-2]=='s' && comms[count-1]=='s') {
                    motorStop();
                    SysCtlPeripheralDisable(SYSCTL_PERIPH_TIMER2);
                }
                if(comms[count-2]=='p' && comms[count-1]=='d') {
                    SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER2);
                    TimerEnable(TIMER2_BASE, TIMER_A);
                }
                if(comms[count-2]=='o' && comms[count-1]=='f')
                GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3, 0);
                UARTCharPutNonBlocking(UART1_BASE, '\n');
                UARTCharPutNonBlocking(UART1_BASE, '\r');
                //writeStringToUart1("You just said");
                //UARTCharPutNonBlocking(UART1_BASE, ' ');
                //writeStringToUart1(comms);
                //writeStringToUart1("\n\rLet's Talk: \n\r\t");
                count = 0;
                memset(comms,'\0',sizeof(comms));
                break;
            }
            UARTCharPutNonBlocking(UART1_BASE,x);
            comms[count] = x;
            count++;
      }
}

void PID_start()
    {

        float error = 0;
        error = 1800 - distRight();
        integral = integral + (error*.050);
        if(integral<-1000)
            integral=-500;
        else if (integral>1000)
            integral=500;
        float derivative = (error-error_prior)/.050;
        float distFrt=distFront();
        float distRt=distRight();
        float output = kp*error+kd*derivative+ki*integral;
        error_prior=error;
        if(distFrt>3000 && distRt>1000)
                 {
//            writeStringToUart1("UTURN");
            int spin=0;
            while(spin<1 && stopper)
                      {
                                float f = distFront();
                                if((f<850))spin++;
                                 motorMove(35,35,0,1);
                      }
            stopper=1;
            error=0;
            integral=0;

                 }
        else if(output>20 && output<1000)
            {
                motorMove(200,100,0,0);
               // GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3, 2);
            }
        else if(output>1000   )
             {
                 motorMove(250,50,0,0);}
        else if(output<-20 && output>-1000 )
            {
                motorMove(100,200,0,0);
               // GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3, 8);
            }
        else if(output<-1000  )
            {
                motorMove(50,250,0,0);}
      else{motorMove(250,250,0,0);}

    }

//void Uturn()
//{
//
//               while(!(distFront()<300))
//                            {
//                                    motorMove(100,100,0,1);
//                         }
//}

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

void motorMove(unsigned long leftPower, unsigned long rightPower, int rightdir, int leftdir){

    PWMOutputState (PWM1_BASE, PWM_OUT_3_BIT | PWM_OUT_2_BIT, true);
    if( rightdir ) {//if direction is forward
        GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_6, 0b01000000);
        PWMPulseWidthSet(PWM1_BASE, PWM_OUT_3,rightPower);
    }
    else {
        GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_6, 0);
        PWMPulseWidthSet(PWM1_BASE, PWM_OUT_3,rightPower);
    }
    if( leftdir ) {//if direction is forward
        GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3, 0b00001110);
        PWMPulseWidthSet(PWM1_BASE, PWM_OUT_2,leftPower);
    }
    else {
        GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3, 0);
        PWMPulseWidthSet(PWM1_BASE, PWM_OUT_2,leftPower);
    }

}

void motorStop(){
    PWMOutputState (PWM1_BASE, PWM_OUT_3_BIT | PWM_OUT_2_BIT, false);
}

//void timerISR(){
//    TimerIntClear(TIMER2_BASE,TIMER_TIMA_TIMEOUT);
//    Swi_post(PID_time);
//}

float distRight(){
        //Clear interrupt flag
        ADCIntClear(ADC0_BASE, 3);
        //Trigger ADC
        ADCProcessorTrigger(ADC0_BASE, 3);
        //wait for conversion
        while(!ADCIntStatus(ADC0_BASE, 3, false)){}
        //retrieve data
        ADCSequenceDataGet(ADC0_BASE, 3, &result);


//        char  str[20];
//        snprintf(str,20,"Right: %u cm",result);
//        writeStringToUart1(str);
        return (float)result;
}

float distFront(){

          //Clear interrupt flag
          ADCIntClear(ADC1_BASE, 3);
          //Trigger ADC
          ADCProcessorTrigger(ADC1_BASE, 3);
          //wait for conversion
          while(!ADCIntStatus(ADC1_BASE, 3, false)){}
          //retrieve data
          ADCSequenceDataGet(ADC1_BASE, 3, &result1);

//          char  str[20];
//         snprintf(str,20,"Front: %u cm",result1);
//          writeStringToUart1(str);
         // result1= (2080*7)/(result1);
          return (float)result1;



}


void hard_initB() {
    SysCtlClockSet(SYSCTL_SYSDIV_5 | SYSCTL_USE_PLL | SYSCTL_OSC_MAIN | SYSCTL_XTAL_16MHZ);
    //
    // Enable Peripheral Clocks
    //
    //Set the clock
   // SysCtlClockSet(SYSCTL_SYSDIV_1 | SYSCTL_USE_OSC |   SYSCTL_OSC_MAIN | SYSCTL_XTAL_16MHZ);

       //This marks the beggining of timer int setup
    uint32_t ui32Period;
        // Timer 2 setup code
        SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER2);           // enable Timer 2 periph clks
        TimerConfigure(TIMER2_BASE, TIMER_CFG_PERIODIC);        // cfg Timer 2 mode - periodic

        ui32Period = (SysCtlClockGet() /20);                     // period = CPU clk div 2 (500ms)
        TimerLoadSet(TIMER2_BASE, TIMER_A, ui32Period);         // set Timer 2 period

        TimerIntEnable(TIMER2_BASE, TIMER_TIMA_TIMEOUT);        // enables Timer 2 to interrupt CPU

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
