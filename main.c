/**************************************************************************************************************************
				This project is designed with the help of materials 
				provided by the Microprocessors Technique Laboratory classes
				on AGH- University of Science and Technology in Cracow
				
				Creators:
				Przemyslaw Grobecki
				Justyna Malys
*****************************************************************************************************************************/

/*** Necessery Includes ***/
#include "MKL05Z4.h"
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "DAC.h"
#include "frdm_bsp.h"
#include "tsi.h"
#include "buttons.h"
#include "lcd1602.h"

#define NUMBER_OF_SIGNAL_SAMPLES 20
#define FREQUENCY_COEFFICIENT 1200

typedef enum Mode {Amplitude_Adjustion, Frequency_Adjustion, Signal_Go} Mode;
typedef enum Signal {Signal_Sinus, Signal_Saw, Signal_Triangle} Signal;

/*** Necessery Global Variables ***/
double beep = 0;														//resambles a single tone provided by for example a buzzer connected to the generator
double amplitude = 0.0;
double frequency = 0.0; 
uint16_t amplitude_modifier = 1;
uint8_t wynik_ok = 1; 											//used to comunicate interrupts with the main loop
uint8_t show_signal = 1;									
uint8_t w1 = 1; 														//used to get the the value from slider						
double Sinus[100] = {0.0}; 									//Sinus vector where the sinus values will be stored
double Saw[100] = {0.0};	 									//Saw vector where the saw values will be stored
double Triangle[100] = {0.0};								//Triangle vector where the triangle values will be stored
int counter = 0;
char display[]={0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20};

static unsigned char chosen_signal = 0;
static unsigned char chosen_mode = Frequency_Adjustion;

/* Function that change signel type */
void Change_Signal(void);
void Change_Mode(void);

/* In this handler we push the values of given signal to the DAC */
void SysTick_Handler(void);

/* SysTick initialisation function */
void SysTick_Init(unsigned int divider);

/* Sinus vector filler */
void SineInit(double *sinus);

/* Saw vector filler */
void SawInit(double *saw);

/* Triangle vector filler */
void TriangleInit(double *triangle);

int main (void){
	/* Init block */
	SysTick_Init(SystemCoreClock/1000);
	buttonsInitialize();
	SineInit(Sinus);
	SawInit(Saw);
	TriangleInit(Triangle);
	LCD1602_Init();
	LCD1602_Backlight(TRUE);
	TSI_Init();
	DAC_Init();

	/* Init block - End */
	
	DAC_Load_Trig(0);	// Wyzwolenie prztwornika C/A wartoscia poczatkowa
	while(1){
		
		if(!(chosen_mode == Signal_Go)){
			w1 = TSI_ReadSlider();	
		}
		
		if(chosen_mode==Frequency_Adjustion){
			if(w1!=0){
				SysTick_Config((uint32_t)(SystemCoreClock / (w1*FREQUENCY_COEFFICIENT))); //By changing systick options we modulate the frequency
				frequency = (FREQUENCY_COEFFICIENT*w1)/(double)NUMBER_OF_SIGNAL_SAMPLES;
				sprintf(display,"Freq: %.0lf Hz", frequency);
				LCD1602_SetCursor(0,0);
				LCD1602_Print(display);
			}
		}	
		
		if(chosen_mode==Amplitude_Adjustion){
			if(w1!=0) amplitude_modifier = w1;
			amplitude = amplitude_modifier*20*2.91/4095;
			sprintf(display,"Amp: %.3lf V", amplitude);
			LCD1602_SetCursor(0,1);
			LCD1602_Print(display);
		}
		
		if(show_signal){
			LCD1602_SetCursor(13,1);
			if(chosen_signal == Signal_Sinus){
				LCD1602_Print("SIN");
			}
			if(chosen_signal == Signal_Saw){
				LCD1602_Print("SAW");
			}
			if(chosen_signal == Signal_Triangle){
				LCD1602_Print("TRI");
			}
			show_signal = 0;
		}
		if(chosen_mode == Signal_Go){
			if(wynik_ok){
				DAC_Load_Trig((uint16_t)(20*amplitude_modifier*beep));
				wynik_ok = 0; 
			}	
		}
	}
}



/******************************/
/* Function definitions block */
/******************************/


/* Function that change signel type */
void Change_Signal(void){
	if(~show_signal){
		chosen_signal+=1;
		counter =0;
		if(chosen_signal>2){
			chosen_signal =0;
		}
		show_signal = 1;
	}
	
}

void Change_Mode(void){
	if(chosen_mode == Frequency_Adjustion){
		chosen_mode = Amplitude_Adjustion;
	}
	else if (chosen_mode == Amplitude_Adjustion){
		chosen_mode = Signal_Go;
	}
	else if (chosen_mode == Signal_Go){
		chosen_mode = Frequency_Adjustion;
	}
}

/* In this handler we set the values of given signal to the beep so the DAC can use them */
void SysTick_Handler(void){
	if(~wynik_ok){
		if(chosen_signal == 0){
			beep = Sinus[counter];	
			counter++;
			if(counter == NUMBER_OF_SIGNAL_SAMPLES-1) counter = 0;
			wynik_ok =1;
		}
		else if(chosen_signal==1){
			beep = Saw[counter];
			counter++;
			if(counter == NUMBER_OF_SIGNAL_SAMPLES-1) counter = 0;
				wynik_ok =1;
		}
		else{
			beep = Triangle[counter];
			counter++;
			if(counter == NUMBER_OF_SIGNAL_SAMPLES-1) counter = 0;
			wynik_ok =1;
		}
	}
}


/* SysTick initialisation function */
void SysTick_Init(unsigned int divider){
	SysTick_Config(SystemCoreClock / divider);
}

/* Sinus vector filler */
void SineInit(double *sinus){
	for(int i = 0 ; i < NUMBER_OF_SIGNAL_SAMPLES ; i++){
		sinus[i] =1 + sin((double)(i * 3.14 *2/NUMBER_OF_SIGNAL_SAMPLES));
	}
}

/* Saw vector filler */
void SawInit(double *saw){
	for(int i = 0; i<NUMBER_OF_SIGNAL_SAMPLES; i++){
		saw[i] = (double)(i) * 2/NUMBER_OF_SIGNAL_SAMPLES;
	}
}

/* Triangle vector filler */
void TriangleInit(double *triangle){
	for(int i = 0; i< NUMBER_OF_SIGNAL_SAMPLES/2 ; i++){
		triangle[i] = (double)(i) * 2/(NUMBER_OF_SIGNAL_SAMPLES/2);
	}
	for(int i = NUMBER_OF_SIGNAL_SAMPLES/2; i < NUMBER_OF_SIGNAL_SAMPLES; i++){
		triangle[i] = (double)(NUMBER_OF_SIGNAL_SAMPLES-i)*2/(NUMBER_OF_SIGNAL_SAMPLES/2);
	}
}


