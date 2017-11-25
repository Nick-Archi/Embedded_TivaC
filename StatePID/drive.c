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
#include "inc/hw_pwm.h"
#include "inc/hw_sysctl.h"
#include "driverlib/gpio.h"
#include "driverlib/pin_map.h"
#include "driverlib/sysctl.h"
#include "driverlib/pwm.h"
#include "driverlib/interrupt.h"
#include "driverlib/adc.h"
#include "driverlib/timer.h"
#include "driverlib/rom.h"
#include "driverlib/rom_map.h"
#include "driverlib/udma.h"
#include "driverlib/ssi.h"
#include "driverlib/systick.h"
#include "driverlib/debug.h"
#include <string.h>
#include <drive.h>





//void motorMove(unsigned long leftPower, unsigned long rightPower, int ,int);
//void motorStop();
//float distFront();
//float distRight();


float error_prior = 0;
float integral = 0;
int i=100;
float kp = .4;
float ki = .0002;
float kd = .0002;
uint32_t ui32ADC1Value[2] = {};
uint32_t ui32ADC0Value[2] = {};
uint32_t result=0;
uint32_t result1=0;
int prevLightStat = 0;
int length = 0;
int set = 0;
int setHigh = 0;
int transmit = 0;
int lightStat = 0;
int stopper=1;
int pid_state=1;
int spin =0;
int max=150;
float error;


//----------Functions------------------//
void ReadLightW();
void PID_start()
    {
        while(1){
        Semaphore_pend(DriveSema, BIOS_WAIT_FOREVER);
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
        float output = kp*error;//+kd*derivative+ki*integral;
        error_prior=error;
        if(output > max){
                output = max;
            }
            else if(output < (max * -1)){
                output = -max;
            }
        switch(pid_state){
        case 1: //follow 1


        if(distFrt>3000 && distRt>1000)
                 {
            pid_state=3;//uturn

                 }
        else if(output > 0){
                motorMove(100 + output,150,0,0);
            }

            else if(output < 0){
                motorMove(150, 100 + output * -1,0,0);
            }
//        else if(output>20 && output<1000)
//            {
//                motorMove(200,100,0,0);
//               // GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3, 2);
//            }
//        else if(output>1000   )
//             {
//                 pid_state=2;}//motorMove(250,50,0,0);}
//        else if(output<-20 && output>-1000 )
//            {
//                motorMove(100,200,0,0);
//               // GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3, 8);
//            }
//        else if(output<-1000  )
//            {
//                motorMove(50,250,0,0);}
//      else
//          {
//          motorMove(250,250,0,0);
//          }
        break;
        case 2: //itersection
            motorMove(250,30,0,0);
            pid_state=1;
            break;

        case 3: //uturn
                        spin=0;
                        while(spin<1 && stopper)
                                  {
                                            float f = distFront();
                                            if((f<850))spin++;
                                             motorMove(250,250,0,1);
                                  }
                        stopper=1;
                        error=0;
                        integral=0;
                        pid_state = 1;
                        break;
             }

             }

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

void ReadLightW() { //infraRed interrupt triggers every 60 micro-seconds
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
            if (lightStat == 1) {
                transmit = !transmit;
            }
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
        //Timer_stop(timer1);
        lightStat = 2;
        //if(lightStat != prevLightStat)
        //  UARTprintf("\nFully black line");
        //motorStop();
        length = 0;
    }
    //UARTCharPutNonBlocking(UART1_BASE, itoc(length));
    //memset(comms,'\0',sizeof(comms));
    memset(comms,'\0',sizeof(comms));
    prevLightStat = lightStat;

}



