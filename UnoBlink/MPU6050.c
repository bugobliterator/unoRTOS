
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include <avr/io.h>
#include <avr/interrupt.h>

/* Scheduler include files. */
#include <FreeRTOS.h>
#include <task.h>
#include <queue.h>
#include <semphr.h>
#include <ringBuffer.h>
/* serial interface include file. */
#include <lib_serial.h>
/* ADC interface include file. */
#include <digitalAnalog.h>
/* MPU6050 interface include file */
#include <MPU6050.h>
#include <i2cMultiMaster.h>
#include <twi.h>
xComPortHandle xSerialPort1;
xComPortHandle xSerialPort2;

void vTask1( void *pvParameters );
void vTask2( void *pvParameters );
void vApplicationIdleHook( void );
void vTask1( void *pvParameters )
{
	//char charvar[4];
	const char *pcTaskName = "Task 1 is running\n";
	volatile unsigned long ul;
	uint8_t msg = MPU6050_WHO_AM_I;
	uint8_t msg1=0;
	char charvar[3];
	int state=9;
	xSerialPort = xSerialPortInitMinimal( USART0, 115200, 10, 10); //  serial port: WantedBaud, TxQueueLength, RxQueueLength (8n1)
	twi_init();
	/* As per most tasks, this task is implemented in an infinite loop. */
	//while(1)
	{	
		state=twi_writeTo(MPU6050_I2C_ADDRESS,msg,1 , 1, 0);
		state=twi_readFrom(MPU6050_I2C_ADDRESS,&msg1, 1, 0);
		
		sprintf(&charvar,"\r\n%d",state);
		if(ringBuffer_IsEmpty( &(xSerialPort.xCharsForTx))) {
			xSerialxPrintf(&xSerialPort,charvar);
		}
	}
	while(1);
}
void vTask2( void *pvParameters )
{
	const char *pcTaskName = "Task 2 is running\n";
	volatile unsigned long ul,i;
	DDRB=0xFF;
	/* As per most tasks, this task is implemented in an infinite loop. */
	while(1)
	{	
		PORTB=0xFF;
		for(i=0;i<10000;i++);
		PORTB=0x00;
		for(i=0;i<10000;i++);
	}
}
void vTask3( void *pvParameters )
{
	const char *pcTaskName = "Task 3 is running\n";
	char charvar[10];
	volatile unsigned long ul;
	xSerialPort = xSerialPortInitMinimal( USART0, 115200, 10, 10); //  serial port: WantedBaud, TxQueueLength, RxQueueLength (8n1)
	
	/* As per most tasks, this task is implemented in an infinite loop. */
	while(1)
	{	
		if(!analogIsConverting()) {
			sprintf(&charvar,"%d\n",analogConversionResult());
		}
		setAnalogMode(MODE_8_BIT);
		startAnalogConversion(0, EXTERNAL_REF);
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
	
		
	//avrSerialxPrint_P(&xSerialPort, PSTR("\r\nStart Processes-->\n")); // Ok, so we're alive...
	
	xTaskCreate(vTask1, /* Pointer to the function that implements the task. */
		"Task 1",/* Text name for the task. This is to facilitate debugging only. */
		240,/* Stack depth in words. */
		NULL,/* We are not using the task parameter. */
		2,/* This task will run at priority 1. */
		NULL ); /* We are not going to use the task handle. */
	/* Create the other task in exactly the same way and at the same priority. */
	xTaskCreate( vTask2, "Task 2", 240, NULL, 1, NULL );
	xTaskCreate( vTask3, "Task 3", 240, NULL, 1, NULL );
	/* Start the scheduler so the tasks start executing. */
	vTaskStartScheduler();
	/* If all is well then main() will never reach here as the scheduler will
	now be running the tasks. If main() does reach here then it is likely that
	there was insufficient heap memory available for the idle task to be created.
	CHAPTER 5 provides more information on memory management. */
	return 0;
}	
