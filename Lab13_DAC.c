// Lab13.c
// Runs on LM4F120 or TM4C123
// Use SysTick interrupts to implement a 4-key digital piano
// edX Lab 13 
// Daniel Valvano, Jonathan Valvano
// December 29, 2014
// Modified by Zhanglulucat during studying the edX powered MOOC: UTAustinX-UT.6.03x-Embedded-Systems---Shape-the-World, at May. 2016.
// Port B bits 3-0 have the 4-bit DAC
// Port E bits 3-0 have 4 piano keys

#include "..//tm4c123gh6pm.h"
#include "Sound.h"
#include "Piano.h"
#include "TExaS.h"

// Constant declarations to access port registers 
// using symbolic names instead of addresses:
// Port B:
#define GPIO_PORTB_DATA_R       (*((volatile unsigned long *)0x400053FC))
#define GPIO_PORTB_DIR_R        (*((volatile unsigned long *)0x40005400))
#define GPIO_PORTB_AFSEL_R      (*((volatile unsigned long *)0x40005420))
#define GPIO_PORTB_PUR_R        (*((volatile unsigned long *)0x40005510))
#define GPIO_PORTB_DEN_R        (*((volatile unsigned long *)0x4000551C))
#define GPIO_PORTB_LOCK_R       (*((volatile unsigned long *)0x40005520))
#define GPIO_PORTB_CR_R         (*((volatile unsigned long *)0x40005524))
#define GPIO_PORTB_AMSEL_R      (*((volatile unsigned long *)0x40005528))
#define GPIO_PORTB_PCTL_R       (*((volatile unsigned long *)0x4000552C))
#define GPIO_PORTB_DR8R_R       (*((volatile unsigned long *)0x40005508))
// Port E:
#define GPIO_PORTE_DATA_R       (*((volatile unsigned long *)0x400243FC))
#define GPIO_PORTE_DIR_R        (*((volatile unsigned long *)0x40024400))
#define GPIO_PORTE_AFSEL_R      (*((volatile unsigned long *)0x40024420))
#define GPIO_PORTE_PUR_R        (*((volatile unsigned long *)0x40024510))
#define GPIO_PORTE_DEN_R        (*((volatile unsigned long *)0x4002451C))
#define GPIO_PORTE_LOCK_R       (*((volatile unsigned long *)0x40024520))
#define GPIO_PORTE_CR_R         (*((volatile unsigned long *)0x40024524))
#define GPIO_PORTE_AMSEL_R      (*((volatile unsigned long *)0x40024528))
#define GPIO_PORTE_PCTL_R       (*((volatile unsigned long *)0x4002452C))
// Clock:
#define SYSCTL_RCGC2_R          (*((volatile unsigned long *)0x400FE108))
// SysTick Timer:
#define NVIC_ST_CTRL_R          (*((volatile unsigned long *)0xE000E010))
#define NVIC_ST_RELOAD_R        (*((volatile unsigned long *)0xE000E014))
#define NVIC_ST_CURRENT_R       (*((volatile unsigned long *)0xE000E018)) 

// basic functions defined at end of startup.s
void DisableInterrupts(void); // Disable interrupts
void EnableInterrupts(void);  // Enable interrupts
void delay(unsigned long msec);
int main(void) {
    // set up: 
    // for the real board grader to work 
    // you must connect PD3 to your DAC output
    // activate grader and set system clock to 80 MHz:
    TExaS_Init(SW_PIN_PE3210, DAC_PIN_PB3210, ScopeOn); 
    // PortE used for piano keys, PortB used for DAC        
    Sound_Init();         // initialize SysTick timer and DAC
    Piano_Init();         // initialize Switches
    EnableInterrupts();   // enable after all initialization are done

    // loop:
    while(1) {
			// contains other works needed to do before interrupting
			Piano_In();         // read input from switches (PB3-PB0)
      if (Freq_Index == 4) {
        // if the index is out of range (no switch is pressed)
        Sound_Off();      // then turn off the sound
      } else {
					// otherwise
					Sound_Tone(Frequency_Period[Freq_Index]);
					// adjust the frequency similarly to C, D, E, G
        }
    }
}

// Inputs: Number of msec to delay
// Outputs: None
void delay(unsigned long msec){ 
  unsigned long count;
  while(msec > 0 ) {  // repeat while there are still delay
    count = 16000;    // about 1ms
    while (count > 0) { 
      count--;
    } // This while loop takes approximately 3 cycles
    msec--;
  }
}


