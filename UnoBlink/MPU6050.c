
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <stdarg.h>

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
static volatile uint8_t msg1;
static volatile uint8_t valavailable;
static volatile uint8_t imuavailable;
accel_t_gyro_u accel_t_gyro;
void vTask1( void *pvParameters );
void vTask2( void *pvParameters );
void vApplicationIdleHook( void );
void vTask1( void *pvParameters )
{
	//char charvar[4];
	const char *pcTaskName = "Task 1 is running\n";
	volatile unsigned long ul;
	
	char charvar[100];
	int state=9,i;
	int dT;
	xSerialPort = xSerialPortInitMinimal( USART0, 115200, 100, 100); //  serial port: WantedBaud, TxQueueLength, RxQueueLength (8n1)
	
    for(i=0;i<10000;i++);
	for(;;)
	{	
		if(imuavailable>0)
		{
			dT = (int)( (double) accel_t_gyro.value.temperature + 12412.0) / 340.0;
			sprintf(&charvar,"\r\naccel: %d,%d,%d temp: %d gyro: %d,%d,%d\r\n",accel_t_gyro.value.x_accel,accel_t_gyro.value.y_accel,accel_t_gyro.value.z_accel
																			  ,dT
																			  ,accel_t_gyro.value.x_gyro,accel_t_gyro.value.y_gyro,accel_t_gyro.value.z_gyro);
			if(ringBuffer_IsEmpty( &(xSerialPort.xCharsForTx))) {
				xSerialxPrintf(&xSerialPort,charvar);
			}
			--imuavailable;
		}
		else
			taskYIELD();
	}
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
	int error;
  	double dT;
  	
	uint8_t swap;
  	#define SWAP(x,y) swap = x; x = y; y = swap

	twi_init();
	/* Initialise MPU6050 */
	valavailable=0;
	MPU6050_read (MPU6050_WHO_AM_I, &msg1, 1);	
	++valavailable;
	MPU6050_write_reg (MPU6050_PWR_MGMT_1, 0);
	while(1)
	{
		MPU6050_read (MPU6050_ACCEL_XOUT_H, (uint8_t *) &accel_t_gyro, sizeof(accel_t_gyro));

  		SWAP (accel_t_gyro.reg.x_accel_h, accel_t_gyro.reg.x_accel_l);
 		SWAP (accel_t_gyro.reg.y_accel_h, accel_t_gyro.reg.y_accel_l);
 		SWAP (accel_t_gyro.reg.z_accel_h, accel_t_gyro.reg.z_accel_l);
  		SWAP (accel_t_gyro.reg.t_h, accel_t_gyro.reg.t_l);
  		SWAP (accel_t_gyro.reg.x_gyro_h, accel_t_gyro.reg.x_gyro_l);
  		SWAP (accel_t_gyro.reg.y_gyro_h, accel_t_gyro.reg.y_gyro_l);
  		SWAP (accel_t_gyro.reg.z_gyro_h, accel_t_gyro.reg.z_gyro_l);
		imuavailable++;	
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
	xTaskCreate( vTask3, "Task 3", 240, NULL, 1, NULL );
	/* Start the scheduler so the tasks start executing. */
	vTaskStartScheduler();
	/* If all is well then main() will never reach here as the scheduler will
	now be running the tasks. If main() does reach here then it is likely that
	there was insufficient heap memory available for the idle task to be created.
	*/
	return 0;
}	

// --------------------------------------------------------
// MPU6050_read
//
// This is a common function to read multiple bytes 
// from an I2C device.
//
// It uses the boolean parameter for Wire.endTransMission()
// to be able to hold or release the I2C-bus. 
// This is implemented in Arduino 1.0.1.
//
// Only this function is used to read. 
// There is no function for a single byte.
//
int MPU6050_read(int start, uint8_t *buffer, int size)
{
  int i, n, error;

  beginTransmission(MPU6050_I2C_ADDRESS);
  n = write(start);
  if (n != 1)
    return (-10);

  n = endTransmission(false);    // hold the I2C-bus
  if (n != 0)
    return (n);

  // Third parameter is true: relase I2C-bus after data is read.
  requestFrom(MPU6050_I2C_ADDRESS, size, true);
  i = 0;
  while(available() && i<size)
  {
    buffer[i++]=read();
  }
  if ( i != size)
    return (-11);

  return (0);  // return : no error
}

// --------------------------------------------------------
// MPU6050_write
//
// This is a common function to write multiple bytes to an I2C device.
//
// If only a single register is written,
// use the function MPU_6050_write_reg().
//
// Parameters:
//   start : Start address, use a define for the register
//   pData : A pointer to the data to write.
//   size  : The number of bytes to write.
//
// If only a single register is written, a pointer
// to the data has to be used, and the size is
// a single byte:
//   int data = 0;        // the data to write
//   MPU6050_write (MPU6050_PWR_MGMT_1, &c, 1);
//
int MPU6050_write(int start, const uint8_t *pData, int size)
{
  int n, error;

  beginTransmission(MPU6050_I2C_ADDRESS);
  n = write(start);        // write the start address
  if (n != 1)
    return (-20);

  n = writemany(pData, size);  // write data bytes
  if (n != size)
    return (-21);

  error = endTransmission(true); // release the I2C-bus
  if (error != 0)
    return (error);

  return (0);         // return : no error
}
// --------------------------------------------------------
// MPU6050_write_reg
//
// An extra function to write a single register.
// It is just a wrapper around the MPU_6050_write()
// function, and it is only a convenient function
// to make it easier to write a single register.
//
int MPU6050_write_reg(int reg, uint8_t data)
{
  int error;

  error = MPU6050_write(reg, &data, 1);

  return (error);
}


