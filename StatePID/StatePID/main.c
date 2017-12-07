/*
 * main.c
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
#include "driverlib/debug.h"
#include <string.h>
#include <main.h>


//------------------------------------------
// Method Prototypes
//------------------------------------------
void writeStringToUart1(char* str); //print to UART
void hard_initB(); //init hardware
void ledToggle();  //toggle led used in debugging
void uart1int();   //uart interrupt function 
void adc_init();   //initiralize the ADC

//------------------------------------------
// Variables
//------------------------------------------
volatile int16_t i16ToggleCount=0;  //toggle variable for LED
char comms[200];        //buffer for command interpreter (globalCommandBuffer)
int count = 0;          //command int. control variable
int limit=1700;         //light sensor limit variable
int x=0;



int main(void) 
    {

            //initialize hardware GPIO, PWM, and UART interrupt
        	hard_initB();

            //initialize the ADC parameters
        	adc_init();

            //clear the global command buffer
        	memset(comms,'\0',sizeof(comms));

            //set LED to color to WHITE before PID begins
        	GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_6|GPIO_PIN_7, 0b10000000|0b01000000);
        	GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3, 0b00001110);

            //prompt user that communication can begin
        	writeStringToUart1("Let's Talk: \n\r\t");

            //important for all RTOS functions to begin
        	BIOS_start();

    }



void uart1int(void)
    {

    //UART status variable
    uint32_t ui32Status;

    //access the status of the UART interrupt
    ui32Status = UARTIntStatus(UART1_BASE, true); //get interrupt status

    //clear the interrupt at the UART
    UARTIntClear(UART1_BASE, ui32Status);

    
    //This is the  "Command Interpreter" and "UART interrupt" all in one 
    //if characters were entered by user and UART recieved
    while(UARTCharsAvail(UART1_BASE) ) 

        {

            //access the character entered into the UART
            char x =  UARTCharGetNonBlocking(UART1_BASE);

            //make sure the character is not a carriage return or line feed
            //if CR LF detected begin Command Interpreting
            if(x == 10 || x == 13)

                {
                    //toggle the LED color (this was a proof of concept)
                    if(comms[count-2]=='L' && comms[count-1]=='o') 
                        {
                            //toggle LED color
                            ledToggle(); 
                            //change the count for color change
                            i16ToggleCount++;
                        }

                    //move the robot backwards (proof of concept)
                    if(comms[count-2]=='b' && comms[count-1]=='b')
                        {
                            //reverse both motors and apply 200 power
                            motorMove(200,200,1,1);
                        }
                    
                    //move the robot forwards (proof of concept)
                    if(comms[count-2]=='f' && comms[count-1]=='f') 
                        {
                            //apply 200 power in forward direction
                            motorMove(200,200,0,0);
                        }


                    //Used to stop spinouts manually this is deprecated    
                    if(comms[count-2]=='s' && comms[count-1]=='t')
                        {
                            stopper=0;
                        }

                    //Used to tune the kd PID constant value "on the fly"
                    //ad denotes following char are a float value
                    if(comms[count-8]=='a' && comms[count-7]=='d') 
                        {
                            //create 6 character adjust buffer
                            char  adj[6];

                            //loop thru the characters for adjust buffer
                            int i;
                            for(i=2;i<count;i++)
                                {
                                    adj[i-2]=comms[i];
                                }

                            //convert the characters to float    
                            kd=atof(adj);
                        }

                    //Used to tune the kp PID constant value "on the fly"
                    //ad denotes following char are a float value
                    if(comms[count-8]=='a' && comms[count-7]=='p')
                        {
                            //create 6 character adjust buffer
                            char  adj[6];

                            //loop thru the characters for adjust buffer
                            int i;
                            for(i=2;i<count;i++)
                                {
                                    adj[i-2]=comms[i];
                                }

                            //convert the characters to float        
                            kp=atof(adj);
                        }
                
                    //Used to tune the ki PID constant value "on the fly"
                    //ad denotes following char are a float value
                    if(comms[count-8]=='a' && comms[count-7]=='i') 
                        {
                        
                            //create 6 character adjust buffer
                            char  adj[6];

                            //loop thru the characters for adjust buffer
                            int i;
                            for(i=2;i<count;i++)
                                {
                                    adj[i-2]=comms[i];
                                }

                            //convert the characters to float        
                            ki=atof(adj);
                        }

                    //Used to trigger right distance conversion (deprecated)
                    if(comms[count-2]=='d' && comms[count-1]=='r') 
                        {
                            ReadWall_IR(); //read the right wall value
                        }

                    //Used to trigger front distance conversion (deprecated)
                    if(comms[count-2]=='d' && comms[count-1]=='f') 
                        {
                            ReadFrontWall_US_W(); //read the front wall value
                        }

                    //command to stop the DriveSema function and stop motors
                    if(comms[count-2]=='s' && comms[count-1]=='s') 
                        {
                            //stop the motors 
                            motorStop();
                            //stop the timer used for 50ms DriveSema
                            Timer_stop(DriveClock);
                        }

                    //command to set the light sensor limit variable
                    if(comms[count-8]=='l' && comms[count-7]=='l')
                    {
                                //create 6 character adjust buffer
                                char  adj[6];
                                
                                //loop thru the characters for adjust buffer
                                int i;
                                for(i=2;i<count;i++)
                                    {
                                        adj[i-2]=comms[i];
                                    }
                                //convert the characters to a float
                                limit=atof(adj);
                    }

                    //command to start the PID_start function
                    if(comms[count-2]=='p' && comms[count-1]=='d') 
                        {
                            //turn on the timer for the DriveClock
                            SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER2);

                            //start the timer for the DriveClock
                            Timer_start(DriveClock);

                            //start the timer for the light sensor
                            Timer_start(timer0);
                        }

                    //turn off the LED on the TIVA c board
                    if(comms[count-2]=='o' && comms[count-1]=='f')
                        GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3, 0);
                    
                    //put a CR LF on to the UART
                    UARTCharPutNonBlocking(UART1_BASE, '\n');
                    UARTCharPutNonBlocking(UART1_BASE, '\r');
                    
                    //prompt the user again that they can communicate
                    writeStringToUart1("\n\rLet's Talk: \n\r\t");
                    
                    //reset communication buffer control index
                    count = 0;

                    //clear the command buffer to avoid memory issues
                    memset(comms,'\0',sizeof(comms));

                    break;
        }

            //if there was not CR LF put the character on the UART
            UARTCharPutNonBlocking(UART1_BASE,x);
            //add the characted to the communication buffer
            comms[count] = x;
            //increase the command buffer index control variable
            count++;
    }
}

//toggle the LED based on toggleCount variable
void ledToggle(void){
    // LED values - 2=RED, 4=BLUE, 8=GREEN

        if(i16ToggleCount%3==0)
               GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3, 2);

        if(i16ToggleCount%3==1)
               GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3, 4);

        if(i16ToggleCount%3==2)
               GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3, 8);

	}

// ISR that is called by the RTOS timer to post Semaphore for PID_start
void DriveClockFunc()
    {
        //post the drive semaphore
        Semaphore_post(DriveSema);
    }

// ISR that is called by the RTOS timer to post Semaphore for AcquireData
void DataClockFn()
    {
        //post the data semaphore
        Semaphore_post(DataSema);
    }


void hard_initB() 
{
    //set the clock to 40MHZ
    SysCtlClockSet(SYSCTL_SYSDIV_5 | SYSCTL_USE_PLL | SYSCTL_OSC_MAIN | SYSCTL_XTAL_16MHZ);

    // Enable Peripheral Clocks

    //Configure PWM Clock to match system
    SysCtlPWMClockSet(SYSCTL_PWMDIV_1);

    //enable GPIO modules and peripherals
    SysCtlPeripheralEnable(SYSCTL_PERIPH_UART1);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOC);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
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

       //MAKE SURE TO ENABLE IN THIS ORDER
       IntEnable(INT_UART1);
       UARTIntEnable(UART1_BASE, UART_INT_RT|UART_INT_RX);
       IntMasterEnable();
}

void writeStringToUart1(char* str)   //write a string to Uart0
{
    int i;
    for (i = 0; i < strlen(str); i++)
        //place the string onto the UART char by char
        UARTCharPutNonBlocking(UART1_BASE,str[i]); 
        //reset the comms buffer to avoid memory issues
        memset(comms,'\0',sizeof(comms));
}

void writeCharToUart1(char ch)   //write a string to Uart0
{       
        //place a characters onto the UART buffer
        UARTCharPutNonBlocking(UART1_BASE,ch);
        //place delay to make sure all chars are printed
        SysCtlDelay(SysCtlClockGet()/30000);
        //reset the comms buffer to avoid memory issues
        memset(comms,'\0',sizeof(comms));
}


void adc_init() //initialize the ADC
{

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