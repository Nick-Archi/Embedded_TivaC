/*
 *  Do not modify this file; it is automatically 
 *  generated and any modifications will be overwritten.
 *
 * @(#) xdc-B06
 */

#include <xdc/std.h>

#include <ti/sysbios/knl/Task.h>
extern const ti_sysbios_knl_Task_Handle Drivet;

#include <ti/sysbios/knl/Semaphore.h>
extern const ti_sysbios_knl_Semaphore_Handle DriveSema;

#include <ti/sysbios/hal/Timer.h>
extern const ti_sysbios_hal_Timer_Handle DriveClock;

#include <ti/sysbios/hal/Hwi.h>
extern const ti_sysbios_hal_Hwi_Handle halHwi0;

#include <ti/sysbios/hal/Timer.h>
extern const ti_sysbios_hal_Timer_Handle DataClockFcn;

#include <ti/sysbios/hal/Timer.h>
extern const ti_sysbios_hal_Timer_Handle timer0;

#include <ti/sysbios/knl/Semaphore.h>
extern const ti_sysbios_knl_Semaphore_Handle DataSema;

#include <ti/sysbios/knl/Semaphore.h>
extern const ti_sysbios_knl_Semaphore_Handle TxDataSema;

#include <ti/sysbios/knl/Semaphore.h>
extern const ti_sysbios_knl_Semaphore_Handle TxResponseSema;

#include <ti/sysbios/knl/Task.h>
extern const ti_sysbios_knl_Task_Handle AcquireDataTask;

#include <ti/sysbios/knl/Task.h>
extern const ti_sysbios_knl_Task_Handle TxDataTask;

#include <ti/sysbios/knl/Task.h>
extern const ti_sysbios_knl_Task_Handle TxResponseTask;

extern int xdc_runtime_Startup__EXECFXN__C;

extern int xdc_runtime_Startup__RESETFXN__C;

