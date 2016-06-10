// SpaceInvaders.c
// Runs on LM4F120/TM4C123
// Author : Hj Jayakrishnan
// hjjkrishnan@gmail.com


// ******* Required Hardware I/O connections*******************
// Slide pot pin 1 connected to ground
// Slide pot pin 2 connected to PE2/AIN1
// Slide pot pin 3 connected to +3.3V 
// fire button connected to PE0
// special weapon fire button connected to PE1
// 8*R resistor DAC bit 0 on PB0 (least significant bit)
// 4*R resistor DAC bit 1 on PB1
// 2*R resistor DAC bit 2 on PB2
// 1*R resistor DAC bit 3 on PB3 (most significant bit)
// LED on PB4
// LED on PB5

// Blue Nokia 5110
// ---------------
// Signal        (Nokia 5110) LaunchPad pin
// Reset         (RST, pin 1) connected to PA7
// SSI0Fss       (CE,  pin 2) connected to PA3
// Data/Command  (DC,  pin 3) connected to PA6
// SSI0Tx        (Din, pin 4) connected to PA5
// SSI0Clk       (Clk, pin 5) connected to PA2
// 3.3V          (Vcc, pin 6) power
// back light    (BL,  pin 7) not connected, consists of 4 white LEDs which draw ~80mA total
// Ground        (Gnd, pin 8) ground

// Red SparkFun Nokia 5110 (LCD-10168)
// -----------------------------------
// Signal        (Nokia 5110) LaunchPad pin
// 3.3V          (VCC, pin 1) power
// Ground        (GND, pin 2) ground
// SSI0Fss       (SCE, pin 3) connected to PA3
// Reset         (RST, pin 4) connected to PA7
// Data/Command  (D/C, pin 5) connected to PA6
// SSI0Tx        (DN,  pin 6) connected to PA5
// SSI0Clk       (SCLK, pin 7) connected to PA2
// back light    (LED, pin 8) not connected, consists of 4 white LEDs which draw ~80mA total

#include "tm4c123gh6pm.h"
#include "Nokia5110.h"
#include "Random.h"
#include "TExaS.h"
#include "graphics.h"
#include "ADC.h"

void DisableInterrupts(void); // Disable interrupts
void EnableInterrupts(void);  // Enable interrupts
void Timer2_Init(unsigned long period);
void Delay100ms(unsigned long count); // time delay in 0.1 seconds
void SysTick_Init(void);

unsigned long TimerCount;
unsigned long Semaphore=0;
double PlayerShip_Xpos = SCREENW/2;

unsigned char ADC_flag=0;
unsigned char Switch0Flag=0;
//unsigned char Player_Missile_Count;
double Slider_x;
unsigned char MoveDirection;
unsigned char PlayerExplosion;
unsigned char *EnemyExplosion;
unsigned char temp =2;
unsigned char PlayerReSpawnClockCycle=0;
#define SCREEN_FACTOR (84-(PLAYERW))/4096



int main(void){
  TExaS_Init(SSI0_Real_Nokia5110_Scope);  // set system clock to 80 MHz
  Random_Init(1);
  Nokia5110_Init();
  Nokia5110_ClearBuffer();
	Nokia5110_DisplayBuffer();      // draw buffer
	ADC0_Init();
	SysTick_Init();
	//Timer2_Init(0x28B0AA);// 30hz
	//Timer2_Init(0x2625A00);// 0.50 sec
	Timer2_Init(0x1312d00);
	Random_Init(21);
	
  while(1){	
		
		if (Semaphore==1){
			// Display Player and sprites
			Display_all(Player_Missile_Count, PlayerShip_Xpos);
			// Collision Display
			DisplayCollision(PlayerExplosion, PlayerShip_Xpos, EnemyExplosion);			
			Semaphore=0;			
		}		
  }
}



void Timer2_Init(unsigned long period){ 
  unsigned long volatile delay;
  SYSCTL_RCGCTIMER_R |= 0x04;   // 0) activate timer2
  delay = SYSCTL_RCGCTIMER_R;
  TimerCount = 0;
  Semaphore = 0;
  TIMER2_CTL_R = 0x00000000;    // 1) disable timer2A during setup
  TIMER2_CFG_R = 0x00000000;    // 2) configure for 32-bit mode
  TIMER2_TAMR_R = 0x00000002;   // 3) configure for periodic mode, default down-count settings
  TIMER2_TAILR_R = period-1;    // 4) reload value
  TIMER2_TAPR_R = 0;            // 5) bus clock resolution
  TIMER2_ICR_R = 0x00000001;    // 6) clear timer2A timeout flag
  TIMER2_IMR_R = 0x00000001;    // 7) arm timeout interrupt
  NVIC_PRI5_R = (NVIC_PRI5_R&0x00FFFFFF)|0x80000000; // 8) priority 4
// interrupts enabled in the main program after all devices initialized
// vector number 39, interrupt number 23
  NVIC_EN0_R = 1<<23;           // 9) enable IRQ 23 in NVIC
  TIMER2_CTL_R = 0x00000001;    // 10) enable timer2A
}


//	ISR for Timer2
//	Initiates periodically :
//	1. Generation of Enemy missiles
//	2. Enemy Horde movement towards player
//	3. Enemy Respawn after being killed
void Timer2A_Handler(void){ 
  TIMER2_ICR_R = 0x00000001;   // acknowledge timer2A timeout
  TimerCount++;
	GPIO_PORTE_DATA_R ^= 0x10;	//For debugging

//**** Here starts periodic game functions ****//
	Generate_Enemey_Missile();
	MoveDirection ^= 1; // Induce zig zag horizontal motion
	EnemyHordeMove(MoveDirection);
	EnemyReSpawn();
	PlayerReSpawnClockCycle++; 
	// We wait for 2 clock cycles before Playership respawns
	if(PlayerReSpawnClockCycle>2){
		PlayerReSpawn(PlayerShip_Xpos);
		PlayerReSpawnClockCycle=0;
	}	
}


// Initialize SysTick interrupts to trigger at 40 Hz, 25 ms
void SysTick_Init(void){
	NVIC_ST_CTRL_R = 0x00000000; 
	NVIC_ST_RELOAD_R = 0x1E847F;//0xF423F;  //0x7A120;
	NVIC_ST_CURRENT_R=0;
	//NVIC_SYS_PRI3_R = NVIC_SYS_PRI3_R&0x00FFFFFF; // priority 0   
  NVIC_SYS_PRI3_R = (NVIC_SYS_PRI3_R & ~0xE0000000) | 0x20000000;	
  NVIC_ST_CTRL_R = 0x00000007;  // enable with core clock and interrupts
}



//	ISR for SysTick
//	Execution every 25ms
//	Initiates periodically:
//	1. Collection of Analog I/P from slider, 
//	conversion and storage in mailbox
//	2. Detection of Switch Press Event
//	3.	Collision detection
void SysTick_Handler(void){ 
	  Slider_x = ADC0_In();
    ADC_flag=1;
	  //GPIO_PORTE_DATA_R ^= 0x10;
		
		
		// scale the player ship position w.r.t slide pot input
	
		PlayerShip_Xpos = Slider_x * SCREEN_FACTOR;		
		if(SwitchPressEvent()){
			Player_Missile_Count++;
			Generate_Player_Missile(PlayerShip_Xpos + PLAYERW/2 -1 ,SCREENH,Player_Missile_Count);			
		}
		Move_Missile(&Player_Missile_Count);
		Semaphore=1;
		
		// Checking for Collisions
		
		PlayerExplosion  = CollisionCheck_EnemyMissile_Player(PlayerShip_Xpos);
		EnemyExplosion = CollisionCheck_PlayerMissile_Enemy();
			

}
