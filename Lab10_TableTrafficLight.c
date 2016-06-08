// ***** 0. Documentation Section *****
// TableTrafficLight.c for Lab 10
// Runs on LM4F120/TM4C123
// Index implementation of a Moore finite state machine to operate a traffic light.  
// Daniel Valvano, Jonathan Valvano
// January 15, 2016
// Modified by Zhanglulucat during studying the edX powered MOOC: UTAustinX-UT.6.03x-Embedded-Systems---Shape-the-World, at Apr. 2016.

// east/west red light connected to PB5
// east/west yellow light connected to PB4
// east/west green light connected to PB3
// north/south facing red light connected to PB2
// north/south facing yellow light connected to PB1
// north/south facing green light connected to PB0
// pedestrian detector connected to PE2 (1=pedestrian present)
// north/south car detector connected to PE1 (1=car present)
// east/west car detector connected to PE0 (1=car present)
// "walk" light connected to PF3 (built-in green LED)
// "don't walk" light connected to PF1 (built-in red LED)

// ***** 1. Pre-processor Directives Section *****
#include "TExaS.h"
#include "tm4c123gh6pm.h"
#include "SysTick.c"

// ***** 2. Global Declarations Section *****
#define SYSCTL_RCGC2_R          (*((volatile unsigned long *)0x400FE108))
#define SYSCTL_RCGC2_GPIOBEF     0x00000032  // port B/E/F Clock Gating Control

#define LIGHT                   (*((volatile unsigned long *)0x400050FC))
#define SENSOR                  (*((volatile unsigned long *)0x4002400C))

//Port B address
#define GPIO_CarLED						  (*((volatile unsigned long *)0x400053FC))
#define GPIO_PORTB_DATA_R       (*((volatile unsigned long *)0x400053FC))
#define GPIO_PORTB_DIR_R        (*((volatile unsigned long *)0x40005400))
#define GPIO_PORTB_AFSEL_R      (*((volatile unsigned long *)0x40005420))
#define GPIO_PORTB_PUR_R        (*((volatile unsigned long *)0x40005510))
#define GPIO_PORTB_DEN_R        (*((volatile unsigned long *)0x4000551C))
#define GPIO_PORTB_AMSEL_R      (*((volatile unsigned long *)0x40005528))
#define GPIO_PORTB_PCTL_R       (*((volatile unsigned long *)0x4000552C))
#define SYSCTL_RCGC2_GPIOB      0x00000002  // port B Clock Gating Control

//Port E address
#define GPIO_Sensor	  		      (*((volatile unsigned long *)0x400243FC))
#define GPIO_PORTE_DATA_R       (*((volatile unsigned long *)0x400243FC))
#define GPIO_PORTE_DIR_R        (*((volatile unsigned long *)0x40024400))
#define GPIO_PORTE_AFSEL_R      (*((volatile unsigned long *)0x40024420))
#define GPIO_PORTE_PUR_R        (*((volatile unsigned long *)0x40024510))
#define GPIO_PORTE_DEN_R        (*((volatile unsigned long *)0x4002451C))
#define GPIO_PORTE_AMSEL_R      (*((volatile unsigned long *)0x40024528))
#define GPIO_PORTE_PCTL_R       (*((volatile unsigned long *)0x4002452C))
#define SYSCTL_RCGC2_GPIOE      0x00000010  // port E Clock Gating Control

//Port F address
#define GPIO_WalkLED		        (*((volatile unsigned long *)0x400253FC))
#define GPIO_PORTF_DATA_R       (*((volatile unsigned long *)0x400253FC))
#define GPIO_PORTF_DIR_R        (*((volatile unsigned long *)0x40025400))
#define GPIO_PORTF_AFSEL_R      (*((volatile unsigned long *)0x40025420))
#define GPIO_PORTF_PUR_R        (*((volatile unsigned long *)0x40025510))
#define GPIO_PORTF_DEN_R        (*((volatile unsigned long *)0x4002551C))
#define GPIO_PORTF_AMSEL_R      (*((volatile unsigned long *)0x40025528))
#define GPIO_PORTF_PCTL_R       (*((volatile unsigned long *)0x4002552C))
#define SYSCTL_RCGC2_GPIOF      0x00000020  // port F Clock Gating Control

// Linked data structure
struct State {
  unsigned long CarLED; 
	unsigned long WalkLED;
  unsigned long Time;  
  unsigned long Next[8];}; 
typedef const struct State STyp;
#define GoWest  	 			0
#define WaitWest 				1
#define GoSouth   			2
#define WaitSouth 			3
#define Walk 						4
#define FirstHurryOn 		5
#define FirstHurryOff 	6
#define SecondHurryOn 	7
#define SecondHurryOff 	8
//#define ThirdHurryOn 		9
//#define ThirdHurryOff 	10
STyp FSM[9]={
 {0x0C, 0x02,  10,{GoWest,GoWest,WaitWest,WaitWest,WaitWest,WaitWest,WaitWest,WaitWest}}, 
 {0x14, 0x02,  10,{GoSouth,GoSouth,GoSouth,GoSouth,Walk,GoSouth,Walk,GoSouth}},
 {0x21, 0x02,  10,{GoSouth,WaitSouth,GoSouth,WaitSouth,WaitSouth,WaitSouth,WaitSouth,WaitSouth}},
 {0x22, 0x02,  10,{GoWest,GoWest,GoWest,GoWest,Walk,Walk,Walk,Walk}},
 {0x24, 0x08,  10,{Walk,FirstHurryOn,FirstHurryOn,FirstHurryOn,Walk,FirstHurryOn,FirstHurryOn,FirstHurryOn}},
 {0x24, 0x02,  10,{FirstHurryOff,FirstHurryOff,FirstHurryOff,FirstHurryOff,FirstHurryOff,FirstHurryOff,FirstHurryOff,FirstHurryOff}},
 {0x24, 0x00,  10,{SecondHurryOn,SecondHurryOn,SecondHurryOn,SecondHurryOn,SecondHurryOn,SecondHurryOn,SecondHurryOn,SecondHurryOn}},
 {0x24, 0x02,  10,{SecondHurryOff,SecondHurryOff,SecondHurryOff,SecondHurryOff,SecondHurryOff,SecondHurryOff,SecondHurryOff,SecondHurryOff}},
 {0x24, 0x00,  10,{GoWest,GoWest,GoSouth,GoWest,Walk,GoWest,GoSouth,GoWest}}};
unsigned long S;  // index to the current state 
unsigned long Input; 

// FUNCTION PROTOTYPES: Each subroutine defined
void DisableInterrupts(void); // Disable interrupts
void EnableInterrupts(void);  // Enable interrupts
void Port_Init(void);				// Initialize Port B/E/F


// ***** 3. Subroutines Section *****

int main(void){ 
  TExaS_Init(SW_PIN_PE210, LED_PIN_PB543210,ScopeOff); // activate grader and set system clock to 80 MHz
  EnableInterrupts();
	Port_Init();
	SysTick_Init();
	S = GoWest;
  while(1){
    GPIO_CarLED = FSM[S].CarLED;  // set Car LED
		GPIO_WalkLED = FSM[S].WalkLED;  // set Walk LED
    SysTick_Wait10ms(FSM[S].Time);
    Input = GPIO_Sensor;     // read sensors
    S = FSM[S].Next[Input];  
  }
}

void Port_Init(){
	unsigned long volatile delay;
	SYSCTL_RCGC2_R |= SYSCTL_RCGC2_GPIOBEF;     //Turn on the clock for Port B/E/F (page 462: bit1/45 in RCGC2)
	delay = SYSCTL_RCGC2_R;						//dummy delay for clock stablize
	//Port B Initialization
	GPIO_PORTB_AMSEL_R &= 0x00;				//Clear the PB0 ~ PB5 bits in Port B AMSEL to disable analog (page 684)
  GPIO_PORTB_PCTL_R &= 0x00000000;		//Clear the PB0 ~ PB5 bit fields in Port B PCTL to configure as GPIO (page 685)
	GPIO_PORTB_DIR_R |= 0x3F;					//Set the PB0 ~ PB5 direction register so PB0 ~ PB5 are outputs(page 660)
	GPIO_PORTB_AFSEL_R &= 0x00;				//Clear the PB0 ~ PB5 bits in Port B AFSEL to disable alternate functions (page 668)
	GPIO_PORTB_DEN_R |= 0x3F;					//Set the PB0 ~ PB5 bits in Port B DEN to enable digital (page 680)
	GPIO_PORTB_PUR_R |= 0x00;					//Pullup is not needed in this lab (page 675)
	GPIO_PORTB_DATA_R = 0x24;					//Initialize Car LED to both red
	//Port E Initialization
	GPIO_PORTE_AMSEL_R &= 0x00;				//Clear the PE0 ~ PE2 bits in Port E AMSEL to disable analog (page 684)
  GPIO_PORTE_PCTL_R &= 0x00000000;		//Clear the PE0 ~ PE2 bit fields in Port E PCTL to configure as GPIO (page 685)
	GPIO_PORTE_DIR_R &= ~0x07;					//Clear the PE0 ~ PE2 direction register so PE0 ~ PE2 are inputs(page 660)
	GPIO_PORTE_AFSEL_R &= 0x00;				//Clear the PE0 ~ PE2 bits in Port E AFSEL to disable alternate functions (page 668)
	GPIO_PORTE_DEN_R |= 0x07;					//Set the PE0 ~ PE2 bits in Port E DEN to enable digital (page 680)
	GPIO_PORTE_PUR_R |= 0x00;					//Pullup is not needed in this lab (page 675)
	GPIO_PORTE_DATA_R = 0x00;					//Initialize sensor to be all off
	//Port F Initialization
	GPIO_PORTF_AMSEL_R &= 0x00;				//Clear the PF1, PF3 bits in Port F AMSEL to disable analog (page 684)
  GPIO_PORTF_PCTL_R &= 0x00000000;		//Clear the PF1, PF3 bit fields in Port F PCTL to configure as GPIO (page 685)
	GPIO_PORTF_DIR_R |= 0x0A;					//Set the PF1, PF3 direction register so PF1, PF3 are outputs(page 660)
	GPIO_PORTF_AFSEL_R &= 0x00;				//Clear the PF1, PF3 bits in Port F AFSEL to disable alternate functions (page 668)
	GPIO_PORTF_DEN_R |= 0x0A;					//Set the PF1, PF3 bits in Port F DEN to enable digital (page 680)
	GPIO_PORTF_PUR_R |= 0x00;					//Pullup is not needed in this lab (page 675)
	GPIO_PORTF_DATA_R = 0x02;					//Initialize Walk LED to be red
}
