#include "ADC.h"
#include "tm4c123gh6pm.h"

// This initialization function sets up the ADC 
// Max sample rate: <=125,000 samples/second
// SS3 triggering event: software trigger
// SS3 1st sample source:  channel 1
// SS3 interrupts: enabled but not promoted to controller
unsigned long Delay;
//PE2 ADC0 sequencer3, channels 0- 11(AIN0 - AIN11), Here AIN1 is used (channel 1)
void ADC0_Init(void){ 
	SYSCTL_RCGCADC_R |= 0x01;	
	//SYSCTL_RCGCGPIO_R |= 0x10;
	//Delay = SYSCTL_RCGCGPIO_R;
	SYSCTL_RCGC2_R |= 0x10;
	Delay = SYSCTL_RCGC2_R;
	GPIO_PORTE_AFSEL_R |= 0x04;
	GPIO_PORTE_AFSEL_R &= ~0x03;//switch
	GPIO_PORTE_AFSEL_R &= ~0x10;//LED o/p
	GPIO_PORTE_DEN_R &= ~0x04;
	GPIO_PORTE_DEN_R |= 0x03;
	GPIO_PORTE_DEN_R |= 0x10;
	GPIO_PORTE_AMSEL_R |= 0x04;
	GPIO_PORTE_AMSEL_R &= ~0x03;
	GPIO_PORTE_AMSEL_R &= ~0x10;
	GPIO_PORTE_DIR_R &= ~0x04;
	GPIO_PORTE_DIR_R &= ~0x03;
	GPIO_PORTE_DIR_R |= 0x10;
	//GPIO_PORTE_PUR_R |= 0x03;
	
//	GPIO_PORTE_AMSEL_R = 0x04;
//	GPIO_PORTE_AFSEL_R = 0x04;
//	GPIO_PORTE_DEN_R = 0x0B;
//	GPIO_PORTE_DIR_R = 0x08;
//	GPIO_PORTE_PCTL_R = 0x00000000;
//	GPIO_PORTE_PUR_R = 0x03;
	
//	GPIO_PORTE_AMSEL_R = 0x00;
//	GPIO_PORTE_AFSEL_R = 0x00;
//	GPIO_PORTE_DEN_R = 0x01;
//	GPIO_PORTE_DIR_R = 0x01;
//	GPIO_PORTE_PCTL_R = 0x00000000;
//	GPIO_PORTE_PUR_R = 0x00;


	
	ADC0_ACTSS_R &= ~0x08; //Disable sequencer 3 of ADC0
	ADC0_EMUX_R &= ~0x0000F000;// software trigger
	ADC0_SSMUX3_R |= 0x1; // Selecting channel 1
	ADC0_SSCTL3_R |= 0x6;
	ADC0_ACTSS_R |= 0x08;	  
}

//------------ADC0_In------------
// Busy-wait Analog to digital conversion
// Input: none
// Output: 12-bit result of ADC conversion
unsigned long ADC0_In(void){  
	ADC0_PSSI_R |= 0x8; //Initiating sampler
	
	while((ADC0_RIS_R & 0x0000000F) !=  0x8);
	
	ADC0_ISC_R |= 0x8; //Acknowledging completion and clearing interrupt
	ADC_flag = 1;
	return (ADC0_SSFIFO3_R & 0x00000FFF); 
  //return 0; // replace this line with proper code
}