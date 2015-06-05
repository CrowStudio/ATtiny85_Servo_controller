/*
 * TinyServo.c
 * 
 * Copyright 2015 Daniel Arvidsson <daniel.arvidsson@crowstudio.se>
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 * 
 * 
 */
 
/* 
 * 
 * For now I can move all 5 servos sequential, I am still trying to 
 * figure out how to move then synchronously.
 * 
 */
 
#define F_CPU 8000000UL			//defines clock as 8Mhz

#include <avr/io.h>

volatile uint16_t tPulse = 21000;	     //gloabal variable to set period of servo pulse
volatile uint16_t hPulse = 1500;		//global variable to set the servo position - e.g. 2500 = 180 degrees, 1500 = 90 degrees, 500 = 0 degrees (values depend on servo brand)

volatile uint8_t tot_overflow;		//global variable to count the number of counter1 overflows  
volatile uint16_t oTime;		//global variable to store value of overflow and TCNT1


volatile uint8_t servo[] = {0,1,2,3,4};		//gloabal variable to set servo output pins
volatile uint8_t channel = 0;  		//gloabal variable to keep track of servo channels

void timerSetup (void)
{
	if (TIFR & (1 << TOV1) )		//if counter1 overflow flag idicates overflow
	{
		TIFR |= (1 << TOV1);		//clear counter1 overflow-flag
		
		tot_overflow ++;		//increase tot_overflow with one every time counter1 overflow
	}
									
	oTime = (tot_overflow << 8) | TCNT1;			//adds tot_overflow and TCNT1 to a 16 bit variable
	
	if (TIFR & (1 << TOV1) && (oTime & 0xff) < 0x80) 		//checks if the counter overflow flag is set and if the TCNT1 part of the oTime variable is less than 128
	{
		oTime += 0x100;   	//compensate because overflow had not been counted
	}

}

 void servoPosC(uint16_t pos, uint8_t speed)
 {		 
	while (hPulse <= pos)		//do this until pos is reahed
	{
		timerSetup();		//16-bit setup for counter

		if (oTime > hPulse)		//check if HIGH pulse (pos) is done
		{
			PORTB &= ~(1 << servo[channel]);	
		}
		
		if (oTime > tPulse-hPulse)			//check if LOW pulse (period) is done 
		{	
			PORTB |= (1 << servo[channel]);	
			
			hPulse += speed;		//sets travel speed (µs/pos)	
			
			oTime = 0;		//resets 16-bit variable
			tot_overflow = 0;		//resets tot_overflow variable  
			TIFR |= (1 << TOV1);		// clear counter1 overflow-flag
			TCNT1 = 0;		//resets Timer/Counter1	
		}
		
	}

}

 void servoPosCC(uint16_t pos, uint8_t speed)
 {
	while (hPulse >= pos)		//do this until pos is reahed
	{
		timerSetup();		//16-bit setup for counter

		if (oTime > hPulse)		//check if HIGH pulse (pos) is done
		{
			PORTB &= ~(1 << servo[channel]);
		}
		
		if (oTime > tPulse-hPulse)			//check if LOW pulse (period) is done 
		{	
			PORTB |= (1 << servo[channel]);	
				
			hPulse -= speed;		//sets travel speed (µs/pos)	
			
			oTime = 0;		//resets 16-bit variable
			tot_overflow = 0;		//resets tot_overflow variable  
			TIFR |= (1 << TOV1);		// clear counter1 overflow-flag
			TCNT1 = 0;		//resets Timer/Counter1	
		}
		
	}

}


int main(void)
{	
	static uint8_t i;
	
	TCCR1 |= (1 << CS12);	//set Timer/Counter1 prescaler to increment every 1 µs (PCK/8)
	
	for (i = 0; i < sizeof(servo); i++)
	{
		DDRB |= (1 << servo[i]);	//sets PB0-PB4 as output pins
	}
	
	while(1)		
	{
		if (channel < sizeof(servo))		//if channel is less than the size of servo[]
		{
			servoPosC(2000, 10);		//travel clockwise (depends on servo) - position (µs) and speed (µs/pos)
			servoPosCC(1500, 10);		//travel counterclockwise (depends on servo) -position (µs) and speed (µs/pos)
			
			channel++;		//move to next servo channel
		}
		
		else
		{
			channel = 0;	//resets channel when the size of servo[] is reached
		}
	
	}
	
}
