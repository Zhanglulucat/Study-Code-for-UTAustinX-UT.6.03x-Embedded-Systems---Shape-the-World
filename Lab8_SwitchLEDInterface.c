// ***** 0. Documentation Section *****
// SwitchLEDInterface.c for Lab 8
// Runs on LM4F120/TM4C123
// Use simple programming structures in C to toggle an LED
// while a button is pressed and turn the LED on when the
// button is released.  This lab requires external hardware
// to be wired to the LaunchPad using the prototyping board.
// January 15, 2016
//      Jon Valvano and Ramesh Yerraballi
// Modified by Zhanglulucat during studying the edX powered MOOC: UTAustinX-UT.6.03x-Embedded-Systems---Shape-the-World, at Apr. 2016.

// ***** 1. Pre-processor Directives Section *****
#include "TExaS.h"
#include "tm4c123gh6pm.h"

// ***** 2. Global Declarations Section *****

#define GPIO_PORTE_DATA_R       (*((volatile unsigned long *)0x400243FC))
#define GPIO_PORTE_DIR_R        (*((volatile unsigned long *)0x40024400))
#define GPIO_PORTE_AFSEL_R      (*((volatile unsigned long *)0x40024420))
#define GPIO_PORTE_PUR_R        (*((volatile unsigned long *)0x40024510))
#define GPIO_PORTE_DEN_R        (*((volatile unsigned long *)0x4002451C))
#define GPIO_PORTE_AMSEL_R      (*((volatile unsigned long *)0x40024528))
#define GPIO_PORTE_PCTL_R       (*((volatile unsigned long *)0x4002452C))
#define SYSCTL_RCGC2_R          (*((volatile unsigned long *)0x400FE108))
#define SYSCTL_RCGC2_GPIOE      0x00000010  // port E Clock Gating Control

// FUNCTION PROTOTYPES: Each subroutine defined
void DisableInterrupts(void); // Disable interrupts
void EnableInterrupts(void);  // Enable interrupts
void Delay100ms(unsigned long time);   //Delay 100ms
void PortE_Init(void);   //PortE initialization

//Global var
unsigned long In;		//Input from PE0
unsigned long Out;		//Output to PE1

// ***** 3. Subroutines Section *****

// PE0, PB0, or PA2 connected to positive logic momentary switch using 10k ohm pull down resistor
// PE1, PB1, or PA3 connected to positive logic LED through 470 ohm current limiting resistor
// To avoid damaging your hardware, ensure that your circuits match the schematic
// shown in Lab8_artist.sch (PCB Artist schematic file) or 
// Lab8_artist.pdf (compatible with many various readers like Adobe Acrobat).
int main(void){ 
//**********************************************************************
// The following version tests input on PE0 and output on PE1
//**********************************************************************
  TExaS_Init(SW_PIN_PE0, LED_PIN_PE1, ScopeOn);  // activate grader and set system clock to 80 MHz
  PortE_Init();  										// initialization goes here
  EnableInterrupts();           // enable interrupts for the grader
  while(1){
    Delay100ms(1);									// Delay about 100 ms
		In = GPIO_PORTE_DATA_R&0x01;		//Read the switch input PE0 and test if the switch is pressed
		Out = GPIO_PORTE_DATA_R&0x02;		//Read the output to PE1
		if (In == 0x01) {											//If PE0==0x01 (the switch is pressed),
			GPIO_PORTE_DATA_R	^= 0x02;		//toggle PE1 (flip bit from 0 to 1, or from 1 to 0)
			//if (Out == 0x02) {
				//GPIO_PORTE_DATA_R	&= ~0x02;
			//} else {
				//GPIO_PORTE_DATA_R	|= 0x02;
			//}
		} else {												//If PE0==0x00 (the switch is not pressed), 
			GPIO_PORTE_DATA_R	|= 0x02;		//set PE1, so LED is ON
		}    
  }
  
}


void Delay100ms(unsigned long time){
  unsigned long i;
  while(time > 0){
    i = 1333333;  // this number means 100ms
    while(i > 0){
      i = i - 1;
    }
    time = time - 1; // decrements every 100 ms
  }
}

void PortE_Init(){
	unsigned long volatile delay;
	SYSCTL_RCGC2_R |= SYSCTL_RCGC2_GPIOE;     //Turn on the clock for Port E (page 462: bit4 is for PE in RCGC2)
	delay = SYSCTL_RCGC2_R;						//dummy delay for clock stablize
	GPIO_PORTE_AMSEL_R &= 0x00;				//Clear the PE0 and PE1 bits in Port E AMSEL to disable analog (page 684)
  GPIO_PORTE_PCTL_R &= 0x00000000;		//Clear the PE0 and PE1 bit fields in Port E PCTL to configure as GPIO (page 685)
	GPIO_PORTE_DIR_R &= ~0x01;					//Clear the Port E direction register so PE0 is an input(page 660)
	GPIO_PORTE_DIR_R |= 0x02;					//Set the Port E direction register so PE1 is an output (page 660)
	GPIO_PORTE_AFSEL_R &= 0x00;				//Clear the PE0 and PE1 bits in Port E AFSEL to disable alternate functions (page 668)
	GPIO_PORTE_DEN_R |= 0x03;					//Set the PE0 and PE1 bits in Port E DEN to enable digital (page 680)
	GPIO_PORTE_PUR_R |= 0x00;					//Pullup is not needed in this lab (page 675)
	GPIO_PORTE_DATA_R	|= 0x02;				//Set the PE1 bit in Port E DATA so the LED is initially ON (page 659)
}
