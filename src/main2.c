#include <lpc17xx.h>
#include "stdio.h"
#include <stdint.h>
#include <cmsis_os2.h>
#include <stdbool.h>
#include "MPU9250.h"
#include "sensor_fusion.h"

bool ledLastOn = false;
uint32_t buttonCount = 0;

void setLEDs(uint32_t n) {
	
	if(n & 1)
		LPC_GPIO1->FIOSET |= 1U<<28;
	else 
		LPC_GPIO1->FIOCLR |= 1U<<28;
	
	if(n & 2)
		LPC_GPIO1->FIOSET |= 1U<<29;
	else 
		LPC_GPIO1->FIOCLR |= 1U<<29;
	
	if(n & 4)
		LPC_GPIO1->FIOSET |= 1U<<31;
	else 
		LPC_GPIO1->FIOCLR |= 1U<<31;
	
	if(n & 8)
		LPC_GPIO2->FIOSET |= 1U<<2;
	else 
		LPC_GPIO2->FIOCLR |= 1U<<2;
	
	if(n & 16)
		LPC_GPIO2->FIOSET |= 1U<<3;
	else 
		LPC_GPIO2->FIOCLR |= 1U<<3;
	
	if(n & 32)
		LPC_GPIO2->FIOSET |= 1U<<4;
	else 
		LPC_GPIO2->FIOCLR |= 1U<<4;
	
	if(n & 64)
		LPC_GPIO2->FIOSET |= 1U<<5;
	else 
		LPC_GPIO2->FIOCLR |= 1U<<5;
	
	if(n & 128)
		LPC_GPIO2->FIOSET |= 1U<<6;
	else 
		LPC_GPIO2->FIOCLR |= 1U<<6;
}

//Mutex definition stuff
const osMutexAttr_t Thread_Mutex_attr = {
		"myButtonMutex", // human readable mutex name
		0, // attr_bits
		NULL, // memory for control block
		0U // size for control block
};

osMutexId_t mutexIMU;
const osMutexAttr_t newMutexAttr;

uint32_t MPU_State;

//Thread definitions

void read(void *args) {
	while(1)
	{
		//acquire Mutex
		osMutexAcquire(mutexIMU,osWaitForever);
		//read MPU data using given functions
		MPU9250_read_gyro();
		MPU9250_read_acc();
		//check if bit 4 of magnetometer status is set before reading magnetometer
		MPU_State = MPU9250_st_value |= (1U<<4);
		if(MPU_State) {
			MPU9250_read_mag();
		}
		//release Mutex
		osMutexRelease(mutexIMU); 
	}
	
	//Picked 69 for example
}

void runSensorFusion(void *args) {
	while(1)
	{
		//acquire Mutex
		osMutexAcquire(mutexIMU,osWaitForever);
		//Run sensor fusion algorithm
		sensor_fusion_update(MPU9250_gyro_data[0],MPU9250_gyro_data[1],MPU9250_gyro_data[2],MPU9250_accel_data[0],MPU9250_accel_data[1],MPU9250_accel_data[2],MPU9250_mag_data[0],MPU9250_mag_data[1],MPU9250_mag_data[2]);
		//release Mutex
		osMutexRelease(mutexIMU); 
	}
	
	//Picked 69 for example
}

void sendToUART(void *args) {
	while(1)
	{
		//acquire Mutex
		osMutexAcquire(mutexIMU,osWaitForever);
		printf("%f,%f,%f\n",sensor_fusion_getRoll(), sensor_fusion_getYaw(), sensor_fusion_getPitch());
		osMutexRelease(mutexIMU); 
	}
}

int main(void) {
	
	//Initialize variables for LCD display
	
	SystemInit();
	osKernelInitialize();
	//initialize IMU
	MPU9250_init(1,1);
	//initialize sensor fusion stuff
	sensor_fusion_init();
	sensor_fusion_begin(4096.0f); //changed frequency to gain full mastery over brent
	
	osMutexNew(mutexIMU);
	
	//check that bit 4 of status value is set for magnetometer
	
	printf("%x", MPU9250_whoami());
		
	//Create Threads
	osThreadNew(read, NULL, NULL);
	osThreadNew(runSensorFusion, NULL, NULL);
	osThreadNew(sendToUART, NULL, NULL);
		
	osKernelStart();

	while(1) {	
	}
}
