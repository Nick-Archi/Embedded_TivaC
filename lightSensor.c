// -------------------------------------------------------//
// REFLECTANCE SENSOR
// ------------------------------------------------------//

/*
 * main.c
 *
 *  Created on: Nov 1, 2017
 *      Authors: Aurash, Rhema Ike, Nicholas Archibong
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
#include "utils/uartstdio.h"





void writeStringToUart1(char* str);
void hard_initB();
void uart1int();
void motorMove(unsigned long leftPower, unsigned long rightPower, int ,int);
void motorStop();
float distFront();
float distRight();
void adc_init();
void timerISR();
void Uturn();
void infraRed();
void infraRed2();
char itoc(int i);

volatile int16_t i16ToggleCount=0;
float error_prior = 0;
float integral = 0;
int length = 0;
int set = 0;
int i = 100;
float kp = .5;
float ki = .05;
float kd = .2;
uint32_t ui32ADC1Value[2] = {};
uint32_t ui32ADC0Value[2] = {};
uint32_t result=0;
uint32_t result1=0;
int count = 0;
int inCount = 0;
int stopper=1;
char comms[200];
int light = 5;
int limit = 0;
int start = 0;
int setHigh = 1;
int lightStat = 0;
int prevLightStat = 0;

int main(void) {

    hard_initB();
    adc_init();

    memset(comms,'\0',sizeof(comms));
    GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_6|GPIO_PIN_7, 0b10000000|0b01000000);
    //GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1, 0b00001110);
    writeStringToUart1("Let's Talk: \n\r\t");
    //UARTStdioConfig(1, 115200, 16000000);
    //UARTprintf("yooooooooooo%d", 1);
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
            if(comms[count-5]=='l' && comms[count-4]=='i') {
                char  adj[3];
                int i;
                for(i=2;i<count;i++){
                    adj[i-2]=comms[i];
                }
                light=atof(adj);
            }
            if(comms[count-8]=='l' && comms[count-7]=='l') {
                char  adj[6];
                int i;
                for(i=2;i<count;i++){
                    adj[i-2]=comms[i];
                }
                limit=atof(adj);
            }
            if(comms[count-5]=='a' && comms[count-4]=='p') {
                char  adj[3];
                int i;
                for(i=2;i<count;i++){
                    adj[1-2]=comms[i];
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
                //SysCtlPeripheralDisable(SYSCTL_PERIPH_TIMER2);
                Timer_stop(timer1);
            }
            if(comms[count-2]=='p' && comms[count-1]=='d') {
                SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER2);
                //TimerEnable(TIMER2_BASE, TIMER_BOTH);
                //TimerEnable(TIMER2_BASE, TIMER_B);
                //                    Timer_start(timer0);
                //                    Timer_start(timer1);
                //PWMOutputState (PWM1_BASE, PWM_OUT_3_BIT | PWM_OUT_2_BIT, true);
                Timer_start(timer0);
                //Timer_start(timer1);
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

//Function changes integer to character
char itoc(int i){
    char x = 'p';
    if(i < 10)
        x = i +'0';
    return x;
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
            motorMove(250,250,0,1);
        }
        stopper=1;
        error=0;
        integral=0;

    }
    else if(output>20 && output<500)
    {
        motorMove(225,125,0,0);
        // GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3, 2);
    }
    else if(output>500   )
    {
        motorMove(250,50,0,0);}
    else if(output<-20 && output>-1000 )
    {
        motorMove(125,225,0,0);
        // GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3, 8);
    }
    else if(output<-500  )
    {
        motorMove(50,250,0,0);}


    else{motorMove(250,250,0,0);}
    //      if(kp == 0) {//remove later
    //        writeStringToUart1("p change");
    //      }

}


//void Uturn()
//{
//
//               while(!(distFront()<300))
//                            {
//                                    motorMove(100,100,0,1);
//                         }
//}


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

    UARTFIFOEnable(UART1_BASE);
    UARTEnable(UART1_BASE);
    UARTStdioConfig(1, 115200, SysCtlClockGet());

    //MAKE SURE TO ENABLE IN THIS ORDER
    IntEnable(INT_UART1);
    UARTIntEnable(UART1_BASE, UART_INT_RT|UART_INT_RX);
    IntMasterEnable();
}

void writeStringToUart1(char* str)   //write a string to Uart0
{
    int i;
    for (i = 0; i < strlen(str); i++)
        UARTCharPutNonBlocking(UART1_BASE,str[i]);
    //memset(comms,'\0',sizeof(comms));
}

void infraRed2() {
    GPIOPinTypeGPIOInput(GPIO_PORTF_BASE, GPIO_PIN_2);
    if(GPIOPinRead(GPIO_PORTF_BASE, GPIO_PIN_2)){
        //GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_3,0b00001000);
        // Timer_stop(timer0);
    }
    else{
        GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_3,0);
        GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_3,0b00001000);
        void motorStop();
        Timer_stop(timer1);
        //Timer_stop(timer0);
    }


}

//----------------------InfraRed function ----------------------//
//light status Info
//lightStat 0: White surface
//ligthStat 1: Small black line
//lightStat 2: Thick black line


void infraRed() { //infraRed interrupt triggers every 60 micro-seconds
    // first I set the pin to high
    if(set == 0) {
        //start = 0;
        GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_2);
        GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_2|GPIO_PIN_1|GPIO_PIN_3,0b00000100);
        set = 1;    //assign 1 to set, so code doesn't set pin to high again unless on white surface
        //GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_2, 0);
    }
    if((setHigh%2) == 0)
        GPIOPinTypeGPIOInput(GPIO_PORTF_BASE, GPIO_PIN_2); //set GPIO pin t input
    if((setHigh%3) == 0) {
        //if (start) {    //skip code the very first time
        if(GPIOPinRead (GPIO_PORTF_BASE, GPIO_PIN_2) == 0) { //if black line is NOT read
            set = 0;
            length = 0;
            setHigh = 0;
            lightStat = 0;
            //GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_2);
            //GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_2, 0b00001110);
        }
        else {  //if black is NOT read increase length
            length++;
            setHigh = 1;
            GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_2);
            GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_2|GPIO_PIN_3, 0b00001110);
            lightStat = 1;
            //if(lightStat != prevLightStat)
            // UARTprintf("\nSemi black line");
        }
        //  }
    }
    setHigh++;

    //start = 1;  //set start to 1 so it never sets the GPIO to to low
    if (length > limit) { //stop motors if sensor continuously reads black
        GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_2|GPIO_PIN_3, 0b00001000);  //turn on green led to indicate this
        Timer_stop(timer0);
        Timer_stop(timer1);
        lightStat = 2;
        //if(lightStat != prevLightStat)
        //  UARTprintf("\nFully black line");
        motorStop();
        length = 0;
    }
    //UARTCharPutNonBlocking(UART1_BASE, itoc(length));
    //memset(comms,'\0',sizeof(comms));
    memset(comms,'\0',sizeof(comms));
    prevLightStat = lightStat;

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
