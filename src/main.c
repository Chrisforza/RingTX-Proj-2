#include <lpc17xx.h>
#include "stdio.h"
#include <stdint.h>
#include <cmsis_os2.h>
#include <stdbool.h>

bool ledLastOn = false;
uint32_t buttonCount = 0;

//Mutex definition stuff
const osMutexAttr_t Thread_Mutex_attr = {
		"myButtonMutex", // human readable mutex name
		0, // attr_bits
		NULL, // memory for control block
		0U // size for control block
};

osMutexId_t ledMutex;
const osMutexAttr_t newMutexAttr;

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

bool checkPushbutton(void) {
	
	while(1){
		
		if (!((LPC_GPIO2->FIOPIN) & (1U<<10))) {
			//printf("\nPUSHBUTTON PRESSED");	
			return true;
		}
		else 
			return false;
	}
}

void buttonPress(void *args) {
	while(1) {
		//acquire Mutex
		osMutexAcquire(ledMutex,osWaitForever);
		if (checkPushbutton()) {
			while(checkPushbutton());
			//increment button counter
			buttonCount = buttonCount + 1;
		}
		//release Mutex
		osMutexRelease(ledMutex);
	}
}

void LEDCountSet(void *args) {
	while(1)
	{
		//acquire Mutex
		osMutexAcquire(ledMutex,osWaitForever);
		setLEDs(buttonCount);
		//release Mutex
		osMutexRelease(ledMutex); 
	}
	
	//Picked 69 for example
}

int main(void) {
	
	//Initialize variables for LCD display
	
	SystemInit();
	osKernelInitialize();
	
	printf("\nMy name is Mike and I use 3 teabags per tea...also starting kernel");
	
	//create Mutex
	osMutexNew(ledMutex);
	
	//Create Threads

	osThreadNew(buttonPress, NULL, NULL);
	osThreadNew(LEDCountSet, NULL, NULL);
	
	LPC_GPIO1->FIOCLR |= 0;
	LPC_GPIO1->FIOCLR |= 0;

	//Set LED's to '1' - Output pins
	LPC_GPIO1->FIODIR |= 1U<<28;
	LPC_GPIO1->FIODIR |= 1U<<29;
	LPC_GPIO1->FIODIR |= 1U<<31;
	LPC_GPIO2->FIODIR |= 1U<<2;
	LPC_GPIO2->FIODIR |= 1U<<3;
	LPC_GPIO2->FIODIR |= 1U<<4;
	LPC_GPIO2->FIODIR |= 1U<<5;
	LPC_GPIO2->FIODIR |= 1U<<6;
		
	osKernelStart();

	while(1) {	
	}
}
