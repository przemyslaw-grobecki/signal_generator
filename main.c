/**************************************************************************************************************************
				This project is designed with the help of materials 
				provided by the Microprocessors Technique Laboratory classes
				on AGH- University of Science and Technology in Cracow
				
				Creators:
				Przemyslaw Grobecki
				Justyna Malys
				
				If you want to show the current frequency level on the LCD- uncomment all the necessery lines.
****************************************************************************************************************************/


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
#include "pit.h"

/*** Necessery Global Variables ***/
double beep = 0;														//resambles a single tone provided by for example a buzzer connected to the generator
uint8_t wynik_ok = 1; 											//used to comunicate interrupts with the main loop
uint8_t PIT_enable = 0;										//used in frequency_show mode to enable PIT_interrupts mode
static uint8_t w1 = 1; 											//used to get the the value from slider
static uint8_t w2 = 1; 											//used in frequency_show mode
double freq = {0.0}; 
double Sinus[100] = {0.0}; 									//Sinus vector where the sinus values will be stored
double Saw[100] = {0.0};	 									//Saw vector where the saw values will be stored
double Triangle[100] = {0.0};								//Triangle vector where the triangle values will be stored
int counter = 0;
char display[]={0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20};

static unsigned char generator_mode = 0;

/* Function that change signel type */
void Change_Signal(void);

/* In this handler we push the values of given signal to the DAC */
void SysTick_Handler(void);


void PIT_IRQHandler()
{		
	PIT_enable = 1;
	freq = 1.0/(2000*w2);
	//freq *= 0.00000006;
	freq = 100*freq;
	freq = 1.0/freq;
	sprintf(display,"Freq: %.1lf",freq);
	LCD1602_SetCursor(0,0);
	LCD1602_Print(display);	
	PIT_enable = 0;
	PIT->CHANNEL[0].TFLG = PIT_TFLG_TIF_MASK;		// Skasuj flage zadania przerwania
}


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
	SysTick_Init(16777216);
	buttonsInitialize();
	SineInit(Sinus);
	SawInit(Saw);
	TriangleInit(Triangle);
	LCD1602_Init();
	LCD1602_Backlight(TRUE);
	TSI_Init();
	DAC_Init();
	PIT_Init();
	/* Init block - End */
	
	DAC_Load_Trig(0);	// Wyzwolenie prztwornika C/A wartoscia poczatkowa
	while(1){
	if(!PIT_enable){
			w1 = TSI_ReadSlider();	
			if(w1!=0){
				w2=w1;
				SysTick_Config( 16777216/ (w2*2000)); //By changing systick options we modulate the frequency SystemCoreClock
			}
			if(wynik_ok){
				DAC_Load_Trig((uint16_t)(2000*beep));
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
	generator_mode+=1;
	counter =0;
	if(generator_mode>2){
		generator_mode =0;
	}
}

/* In this handler we set the values of given signal to the beep so the DAC can use them */
void SysTick_Handler(void){
if(!PIT_enable){
		if(~wynik_ok){
			if(generator_mode == 0){
				beep = Sinus[counter];
				counter++;
				if(counter == 99) counter = 0;
				wynik_ok =1;
			}
			else if(generator_mode==1){
				beep = Saw[counter];
				counter++;
				if(counter == 99) counter = 0;
					wynik_ok =1;
			}
			else{
				beep = Triangle[counter];
				counter++;
				if(counter == 99) counter = 0;
				wynik_ok =1;
			}
		}
}
}


/* SysTick initialisation function */
void SysTick_Init(unsigned int divider){
	SysTick_Config(SystemCoreClock / divider);
}

/* Sinus vector filler */
void SineInit(double *sinus){
	for(int i = 0 ; i < 100 ; i++){
		sinus[i] =1 + sin((double)(i * 3.14 *2/100));
	}
}

/* Saw vector filler */
void SawInit(double *saw){
	for(int i = 0; i<100; i++){
		saw[i] = (double)(i) * 2/100;
	}
}

/* Triangle vector filler */
void TriangleInit(double *triangle){
	for(int i = 0; i< 50 ; i++){
		triangle[i] = (double)(i) * 2/50;
	}
	for(int i = 50; i < 100; i++){
		triangle[i] = (double)(100-i)*2/50;
	}
}


