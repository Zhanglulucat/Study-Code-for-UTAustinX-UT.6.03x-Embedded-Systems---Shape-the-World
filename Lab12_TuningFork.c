// TuningFork.c Lab 12
// Runs on LM4F120/TM4C123
// Use SysTick interrupts to create a squarewave at 440Hz.  
// There is a positive logic switch connected to PA3, PB3, or PE3.
// There is an output on PA2, PB2, or PE2. The output is 
//   connected to headphones through a 1k resistor.
// The volume-limiting resistor can be any value from 680 to 2000 ohms
// The tone is initially off, when the switch goes from
// not touched to touched, the tone toggles on/off.
//                   |---------|               |---------|     
// Switch   ---------|         |---------------|         |------
//
//                    |-| |-| |-| |-| |-| |-| |-|
// Tone     ----------| |-| |-| |-| |-| |-| |-| |---------------
//
// Daniel Valvano, Jonathan Valvano
// January 15, 2016

/* This example accompanies the book
   "Embedded Systems: Introduction to ARM Cortex M Microcontrollers",
   ISBN: 978-1469998749, Jonathan Valvano, copyright (c) 2015

 Copyright 2016 by Jonathan W. Valvano, valvano@mail.utexas.edu
 Modified by Zhanglulucat during studying the edX powered MOOC: UTAustinX-UT.6.03x-Embedded-Systems---Shape-the-World, at Apr. 2016.
    You may use, edit, run or distribute this file
    as long as the above copyright notice remains
 THIS SOFTWARE IS PROVIDED "AS IS".  NO WARRANTIES, WHETHER EXPRESS, IMPLIED
 OR STATUTORY, INCLUDING, BUT NOT LIMITED TO, IMPLIED WARRANTIES OF
 MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE APPLY TO THIS SOFTWARE.
 VALVANO SHALL NOT, IN ANY CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL,
 OR CONSEQUENTIAL DAMAGES, FOR ANY REASON WHATSOEVER.
 For more information about my classes, my research, and my books, see
 http://users.ece.utexas.edu/~valvano/
 */


#include "TExaS.h"
#include "..//tm4c123gh6pm.h"

// Constant declarations to access port registers using 
// symbolic names instead of addresses
#define GPIO_PORTA_DATA_R       (*((volatile unsigned long *)0x400043FC))
#define GPIO_PORTA_DIR_R        (*((volatile unsigned long *)0x40004400))
#define GPIO_PORTA_AFSEL_R      (*((volatile unsigned long *)0x40004420))
#define GPIO_PORTA_DEN_R        (*((volatile unsigned long *)0x4000451C))
#define GPIO_PORTA_AMSEL_R      (*((volatile unsigned long *)0x40004528))
#define GPIO_PORTA_PCTL_R       (*((volatile unsigned long *)0x4000452C))
#define GPIO_PORTA_DR8R_R       (*((volatile unsigned long *)0x40004508))

#define SYSCTL_RCGC2_R          (*((volatile unsigned long *)0x400FE108))
#define NVIC_SYS_PRI3_R         (*((volatile unsigned long *)0xE000ED20))  // Sys. Handlers 12 to 15 Priority
#define NVIC_ST_CTRL_R          (*((volatile unsigned long *)0xE000E010))
#define NVIC_ST_RELOAD_R        (*((volatile unsigned long *)0xE000E014))
#define NVIC_ST_CURRENT_R       (*((volatile unsigned long *)0xE000E018))

// Global Variables:
unsigned short Switch = 0;      // input from the switch is stored here

// basic functions defined at end of startup.s
void DisableInterrupts(void); // Disable interrupts
void EnableInterrupts(void);  // Enable interrupts
void WaitForInterrupt(void);  // low power mode

// input from PA3, output from PA2, SysTick interrupts
void Sound_Init(void){ 
  unsigned long volatile delay;
	SYSCTL_RCGC2_R |= 0x00000001; // activate port A
  delay = SYSCTL_RCGC2_R;
  GPIO_PORTA_AMSEL_R &= ~0x0C;      // no analog for PA3, PA2
  GPIO_PORTA_PCTL_R = 0; 				// clear function
  GPIO_PORTA_DIR_R |= 0x04;     // make PA2 out, PA3 in
  GPIO_PORTA_DR8R_R |= 0x20;    // can drive up to 8mA out
  GPIO_PORTA_AFSEL_R &= ~0x0C;  // disable alt funct on PA3, PA2
  GPIO_PORTA_DEN_R |= 0x0C;     // enable digital I/O on PA3, PA2
  NVIC_ST_CTRL_R = 0;           // disable SysTick during setup
  NVIC_ST_RELOAD_R = 90908;     // reload value for 1.13636ms (assuming 80MHz)
  NVIC_ST_CURRENT_R = 0;        // any write to current clears it
  NVIC_SYS_PRI3_R = NVIC_SYS_PRI3_R&0x00FFFFFF; // priority 0                
  NVIC_ST_CTRL_R = 0x00000007;  // enable with core clock and interrupts
  EnableInterrupts();
}

// called at 880 Hz
void SysTick_Handler(void){
  GPIO_PORTA_DATA_R ^= 0x04;     // toggle PA2
}

int main(void){// activate grader and set system clock to 80 MHz
	unsigned long NumberOfPresses = 0;   // store the number of presses
  unsigned long LastInput = 0; // store the last input of Switch here
	
  TExaS_Init(SW_PIN_PA3, HEADPHONE_PIN_PA2,ScopeOn); 
  Sound_Init();         
  EnableInterrupts();   // enable after all initialization are done
		
  while(1){
    // main program is free to perform other tasks
    // do not use WaitForInterrupt() here, it may cause the TExaS to crash
    // perform other tasks - Read the input over and over again:
    Switch = GPIO_PORTA_DATA_R & 0x08;  // read the switch from PA3
    if (Switch != 0 && Switch != LastInput) {
      // if the switch is pressed, and last time it was released:
      NumberOfPresses++;    
      // the number of the ACTUAL press increase by 1
    }
    LastInput = Switch;       // save the current input for next time
    if (NumberOfPresses%2 == 0) {
      // if the number of press is even
      // (those presses at the 0, 2nd, 4th, 6th,... time)
      GPIO_PORTA_DATA_R &= ~0x04;
      // then turn off PA2
    } else {
    // if the number of press is odd
    // (those presses at the 1st, 3rd, 5th, 7th,... time)
    WaitForInterrupt(); 
    // then periodically interrupt the system each 880 Hz
    // the SysTick will automatically
    // trigger itself every (1 s)/(880 Hz) = 1.13636 ms
    }
  }
}
