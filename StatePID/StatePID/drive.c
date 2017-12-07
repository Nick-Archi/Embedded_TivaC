/*
 * CmdInt.c
 *
 *  Created on: Nov 1, 2017
 *      Author: Aurash Norouzi, Rhema Ike, Nicholas Archibong
 */


//----------------------------------------
// BIOS header files
//----------------------------------------
#include <xdc/std.h>                    //mandatory - have to include first, for BIOS types
#include <ti/sysbios/BIOS.h>                //mandatory - if you call APIs like BIOS_start()
#include <xdc/runtime/Log.h>                //needed for any Log_info() call
#include <xdc/cfg/global.h>

#include <main.h>
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


//----------------------------------------
// Macros
//----------------------------------------
#define true 1
#define false 0

//method prototypes
void motorMove(unsigned long leftPower, unsigned long rightPower, int ,int);
void motorStop();
float ReadFrontWall_US_W();
float ReadWall_IR();
void ReadLightW();


//------------------------------------------
// Drive Variables
//------------------------------------------
int lastPID=0; //control of last PID case value
int start = 0; //used for debugging
int spin =0;   //used for UTURN control
int max=150;    // max motor power value
int stopper=1;  //UTURN control variable
int pid_state=1; //initial PID case to start at


//------------------------------------------
// PID Variables
//------------------------------------------

//note the PID constants were found with Zieger Nichols Oscillation method
float kp = 1.1; //proportional gain constant
float ki = 0.25; //integral constant
float kd = .015; //derivative constant
float error_prior = 0; //last error value 
float integral = 0; //intregral term total
float error;       //error to be minimized
uint32_t result=0;  //right sensor result
uint32_t result1=0; //left sensor result


//------------------------------------------
// Light Sensor Variables
//------------------------------------------
extern int limit;
int i=100;
int prevLightStat = 0;
int length = 0;
int set = 0;
int setHigh = 0;
int transmit = 0;
int lightStat = 0;
int ccheck = 0;

//----------Functions------------------//

void PID_start()
    {
        while(1){
        // wait (forever) for the DriveSema to be posted
        Semaphore_pend(DriveSema, BIOS_WAIT_FOREVER);

        //minimize the error to a raw sensor value of 1800
        error = 1800 - ReadWall_IR();

        //compute the integral term for PID use with .05 mos 50ms
        integral = integral + (error * .050);

        // very important to account for reset windup
        if(integral<-1000)
            integral=-500;  //cap at -500
        else if (integral>1000)
            integral=500;  //cap at +500

        //calculate the derivative term for PID control with 50ms
        float derivative = (error-error_prior)/.050;

        //declare front wall error variable
        float distFrt=ReadFrontWall_US_W();
        
        //declare variable for right wall error
        float distRt=ReadWall_IR();

        //PID final calculation with PID
        //Note PID was tuned with Zieger Nichols Oscillation Method
        float output = kp*error+kd*derivative+ki*integral;
        
        //set error for next entrance into PID_start function
        error_prior=error;

        //cap the output to avoid errors with motor driver
        if(output > max)

            {
                output = max; //cap at defined max of 150
            }

        else if(output < (max * -1))
            
            {
                output = -max;  //cap at min of -150
            }

        //begin PID state machine 
        switch(pid_state){  

            //FOLLOW 1 case before the first thin line has been reached
			case 1: 

                        //set the last state to current for re-entry upon exit
            			lastPID = 1;  

                        // uturn/(left turn) case when front and right sensor are very close to wall
            			if(distFrt>3000 && distRt>1000)    
            					 
                        {
            				//move to the UTURN case (3)
                            pid_state=3; //break not needed due else if
            			}

                        //no uturn necessary if in [0,70] range move right
            			else if(output > 0 && output<70)
                                {   
            					       //increase the left motor speed with PID value
                                       // note a faster left motor is right turn
                                	   motorMove(100 + output,150,0,0);
            					}

                        //no uturn necessary if output below zero move left
            			else if(output < 0)

                                {
            						   //increase the right motor speed with magnitude of PID value
                                       // note a faster right motor is left turn
                                       motorMove(150, 100 + output * -1,0,0);
                                }

                        // for output larger than 70 were at an "intersection"
            			else if(output>70   )
            				    {
                                //move to the intersection case (2)
            					pid_state=2;
            			        
                                }

                        // check for crossing of start transmitting line
            			// transmit = 1 implies first thin line crossed
                         if(transmit==1)
                            {
                				//move to the follow 2 case to begin data TXRX
                                pid_state=4;

                                //start the DataClockFcn in order to post DataSema
                			    Timer_start(DataClockFcn);

                                //control variable (debugging)
                				start = 1;

                                //note that the first line was crossed for debugging
                				//writeStringToUart1("first line\n");
            			    }

            			// lightStat = 2 implies thick line was crossed (STOP)
                        if(lightStat==2)
                            {
                                //move to the STOP case at (5)
                				pid_state=5;
                			}

            			break;   //break from this case



			// This is the intersection case for hard right turns
            case 2: 

                        //hard right turn with over twice power to left motor
        				motorMove(230,100,0,0);

                        //ensure that state machine returns to correct FOLLOW case 
        				pid_state=lastPID;
        				
                        break; //break from intersection case



            //UTURN also defaulted in the case of left turns
			case 3:
				            
                        spin=0; //control variable to avoid spinout
						while(spin<1 && stopper) // enter once
								  {
									float f = ReadFrontWall_US_W();
									if((f<850)) //straightened out
                                        spin++; //avoid a spinout
                                    //max power to both motors, reverse one motor
									motorMove(250,250,0,1); 
								  }
                        //loop control variable (debugging)
						stopper=1;
						
                        //reset the error as we have changed course
                        error=0; 
						
                        //reset the integral as we have changed course
                        integral=0;

                        //ensure that the state machine returns to correct follow case
						pid_state = lastPID;

                        //break from the UTURN case
						break;

			//FOLLOW 2 case after the first thin line has been reached
            case 4:
				
                        //set the last state to current for re-entry upon exit
                        lastPID=4;

                        // uturn/(left turn) case when front and right sensor are very close to wall
                        if(distFrt>3000 && distRt>1000)    
                                 
                        {
                            //move to the UTURN case (3)
                            pid_state=3; //break not needed due to else if
                        }

				        //no uturn necessary if in [0,70] range move right
                        else if(output > 0 && output<70)
                                {   
                                       //increase the left motor speed with PID value
                                       // note a faster left motor is right turn
                                       motorMove(100 + output,150,0,0);
                                }


				        //no uturn necessary if output below zero move left
                        else if(output < 0)

                                {
                                       //increase the right motor speed with magnitude of PID value
                                       // note a faster right motor is left turn
                                       motorMove(150, 100 + output * -1,0,0);
                                }

                        // for output larger than 70 were at an "intersection"
                        else if(output>70   )
                                {
                                //move to the intersection case (2)
                                pid_state=2;
                                
                                }

                        // check for crossing of stop transmitting line
                        // transmit = 0 implies second thin line crossed
						if(!transmit)
                            {
                                //move to the follow 3 case to end data TXRX
    							pid_state=6;

                                //Stop the DataSema from being posted
    							Timer_stop(DataClockFcn);

                                //control variable (debugging)
    							start = 0;

                                //denote in UART that second thin line has been crossed (debugging)
    							//writeStringToUart1("\n\rsecond line\n\r");
						    }

						// lightStat = 2 implies thick line was crossed (STOP)
                        if(lightStat==2)
                            {
                                //move to the STOP case at (5)
                                pid_state=5;
                            }

                        break;   //break from this case

            //this is the full STOP case that ends PID and DriveSema
			case 5: 
                        //stop from posting DriveSema
				        Timer_stop(DriveClock);
				        
                        //stop the Motors
                        motorStop();

                        //break from the stop case
					    break;

            //FOLLOW 3 case after the second thin line has been reached
			case 6: 

                        //set the last state to current for re-entry upon exit
        				lastPID = 6;

				
                        // uturn/(left turn) case when front and right sensor are very close to wall
                        if(distFrt>3000 && distRt>1000)    
                                 
                        {
                            //move to the UTURN case (3)
                            pid_state=3; //break not needed due to else if
                        }

				        //no uturn necessary if in [0,70] range move right
                        else if(output > 0 && output<70)
                                {   
                                       //increase the left motor speed with PID value
                                       // note a faster left motor is right turn
                                       motorMove(100 + output,150,0,0);
                                }


				        //no uturn necessary if output below zero move left
                        else if(output < 0)

                                {
                                       //increase the right motor speed with magnitude of PID value
                                       // note a faster right motor is left turn
                                       motorMove(150, 100 + output * -1,0,0);
                                }

                        // for output larger than 70 were at an "intersection"
                        else if(output>70   )
                                {
                                //move to the intersection case (2)
                                pid_state=2;
                                
                                }

					   // lightStat = 2 implies thick line was crossed (STOP)
                        if(lightStat==2)
                            {
                                //move to the STOP case at (5)
                                pid_state=5;
                            }

                        break;   //break from this case
				 

                 } //close swtich statement

             } //close while loop
 
    }   //close PID_start function


void motorMove(unsigned long leftPower, unsigned long rightPower, int rightdir, int leftdir){

	//Enable PWM pins
    PWMOutputState (PWM1_BASE, PWM_OUT_3_BIT | PWM_OUT_2_BIT, true);
    if( rightdir ) {
		// Right wheel moves forward by setting GPIO pin 6 to HIGH
        GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_6, 0b01000000);
        PWMPulseWidthSet(PWM1_BASE, PWM_OUT_3,rightPower);
    }
    else { 
		// Right wheel moves backwards
        GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_6, 0);
        PWMPulseWidthSet(PWM1_BASE, PWM_OUT_3,rightPower);
    }
    if( leftdir ) { 
		// Left wheel moves forward by setting GPIO pin 1 to HIGH
        GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1, 0b00000010);
        PWMPulseWidthSet(PWM1_BASE, PWM_OUT_2,leftPower);
    }
    else {
		// Left wheel moves backwards 
        GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1, 0);
        PWMPulseWidthSet(PWM1_BASE, PWM_OUT_2,leftPower);
    }

}

void motorStop(){
	//Disable PWM pins 
    PWMOutputState (PWM1_BASE, PWM_OUT_3_BIT | PWM_OUT_2_BIT, false);
}

// Use ADC to pull value of the right distance sensor
float ReadWall_IR()

{
        
        //Clear interrupt flag
        ADCIntClear(ADC0_BASE, 3);
        
        //Trigger ADC
        ADCProcessorTrigger(ADC0_BASE, 3);
        
        //wait for conversion
        while(!ADCIntStatus(ADC0_BASE, 3, false)){}
        
        //retrieve data
        ADCSequenceDataGet(ADC0_BASE, 3, &result);

        //return the result as a float
        return (float)result;

}

float ReadFrontWall_US_W()

{

          //Clear interrupt flag
          ADCIntClear(ADC1_BASE, 3);
          
          //Trigger ADC
          ADCProcessorTrigger(ADC1_BASE, 3);
          
          //wait for conversion
          while(!ADCIntStatus(ADC1_BASE, 3, false)){}
          
          //retrieve data
          ADCSequenceDataGet(ADC1_BASE, 3, &result1);

          //return the result as a float
          return (float)result1;

}

//ReadLightW is the subroutine of a timer that has a period of 30 microsecs
void ReadLightW() { 
   
    //GPIO pin connected to light sensor (GPIO pin 2) is set to HIGH the very first time timer starts
    if(set == 0){
		// GPIO pin 3 is turned off so the tivaC LED will be blue
        GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_2);
        GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_2|GPIO_PIN_3,0b00000100);
        set = 1;    // 1 is assigned to set, so code doesn't set pin to high again unless on white surface
    }

	// GPIO pin 2 is set to be an input in order to detect when light sensor pin goes low
    if((setHigh%2) == 0)
        GPIOPinTypeGPIOInput(GPIO_PORTF_BASE, GPIO_PIN_2); //set GPIO pin t input

	
    if((setHigh%3) == 0){
		// If light sensor has discharged the light sensor is on white surface
        if(GPIOPinRead (GPIO_PORTF_BASE, GPIO_PIN_2) == 0) { 
			
            set = 0; // set is once again assigned a 0 so light sensor pin can be charged up 

			// If statementt checks to see if Robot passed over a thin black line
            if (length > 500 && lightStat == 1) {

			    // If the robot has not been transmitting the robot starts transmitting data and vice versa
                if (transmit  == 0)
						// transmit is a flag variable that indicates when the robot should start transmitting
                        transmit = 1; // Start transmitting data

                else
                    transmit = 0; // Stop transmitting data
            }

			//Reset variables
            length = 0;
            setHigh = 0;
            lightStat = 0;

        }

        else {  //if black is read increase length

            length++; //length keeps track of how long sensor has been on a black surface
            setHigh = 1; //set high is set to 1 and increased to 2 at the bottom of subroutine
            GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_2); // Light sensor is charged up again
            GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_2|GPIO_PIN_3, 0b00001110); // Led changes colour to cyan
            lightStat = 1; //this indicates that light sensor was on a thin black line

        }

    }

    setHigh++; //setHigh continually increases to keep track of the state light sensor should be in

	// stop motors if sensor continuously reads black
    if (length > limit) {  

		// If length is greater than a certain value the light sensor  has read
		// black for a while meaning Robot has one over a thick black line
        GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_2|GPIO_PIN_3, 0b00001000);  //turn on green led to indicate this
        Timer_stop(timer0); // light sensor subroutine is disabled 
        lightStat = 2;	// lightStat is set to 2 when Robot goes over a thick black line
        length = 0;

    }

}