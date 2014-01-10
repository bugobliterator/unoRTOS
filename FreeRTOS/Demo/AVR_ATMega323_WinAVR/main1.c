
#include <stdlib.h>
#include <string.h>

#ifdef GCC_MEGA_AVR
	/* EEPROM routines used only with the WinAVR compiler. */
	#include <avr/eeprom.h> 
#endif

/* Scheduler include files. */
#include "FreeRTOS.h"
#include "task.h"
#include "croutine.h"

/* Demo file headers. */
#include "PollQ.h"
#include "integer.h"
#include "serial.h"
//#include "comtest.h"
#include "crflash.h"
#include "print.h"
#include "partest.h"
#include "regtest.h"

void vTask1( void *pvParameters );
void vTask2( void *pvParameters );
void vApplicationIdleHook( void );
void vTask1( void *pvParameters )
{
	const char *pcTaskName = "Task 1 is running\n";
	volatile unsigned long ul;
	/* As per most tasks, this task is implemented in an infinite loop. */
	for( ;; )
	{
		/* Print out the name of this task. */
		//vPrintString( pcTaskName );
		for( ul = 0; ul < 10000; ul++ )
		{
			/* This loop is just a very crude delay implementation. There is
			nothing to do in here. Later examples will replace this crude
			loop with a proper delay/sleep function. */
			
		}

	}
}
void vTask2( void *pvParameters )
{
	const char *pcTaskName = "Task 2 is running\n";
	volatile unsigned long ul,i;
	DDRB = 0xFF;
	/* As per most tasks, this task is implemented in an infinite loop. */
	for( ;; )
	{
		/* Define pull-ups and set outputs high */
		/* Define directions for port pins */
			
			PORTB = 0x00;
			for(i=0;i<10000;i++);
			PORTB = 0xFF;
			for(i=0;i<10000;i++);
	}
}
void vApplicationIdleHook( void )
{
	vCoRoutineSchedule();
}

int main( void )
{
	/* Create one of the two tasks. Note that a real application should check
	the return value of the xTaskCreate() call to ensure the task was created
	successfully. */
	xTaskCreate(vTask1, /* Pointer to the function that implements the task. */
		"Task 1",/* Text name for the task. This is to facilitate debugging only. */
		240,/* Stack depth in words. */
		NULL,/* We are not using the task parameter. */
		1,/* This task will run at priority 1. */
		NULL ); /* We are not going to use the task handle. */
	/* Create the other task in exactly the same way and at the same priority. */
	xTaskCreate( vTask2, "Task 2", 240, NULL, 1, NULL );
	/* Start the scheduler so the tasks start executing. */
	vTaskStartScheduler();
	/* If all is well then main() will never reach here as the scheduler will
	now be running the tasks. If main() does reach here then it is likely that
	there was insufficient heap memory available for the idle task to be created.
	CHAPTER 5 provides more information on memory management. */
	return 0;
}	
