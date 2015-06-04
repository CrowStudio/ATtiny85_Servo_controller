/*
 * ATtiny>.c
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
 

#define F_CPU 8000000UL			//defines clock as 8Mhz
#define servoPin PORTB2		 //defines PB2 as servoPin

#include <avr/io.h>

volatile uint16_t tPulse = 21000;			 //gloabal variable to set period of servo pulse
volatile uint16_t hPulse = 1500;			//global variable to set the servo start position - 2500 = 180 degrees, 1500 = 90 degrees, 500 = 0 degrees (max and min are aproximated values)

volatile uint8_t tot_overflow;		//global variable to count the number of counter1 overflows  
volatile uint16_t oTime;		//global variable to store value of overflow and TCNT1

void timerSetup (void)
{
	if (TIFR & (1 << TOV1) )		//if counter1 overflow flag idicates overflow
	{
		TIFR |= (1 << TOV1);		// clear counter1 overflow-flag
		
		tot_overflow ++;		//increase tot_overflow with one every time counter1 overflow
	}
									
	oTime = (tot_overflow << 8) | TCNT1;			//adds tot_overflow and TCNT1 to a 16 bit variable
	
	if (TIFR & (1 << TOV1) && (oTime & 0xff) < 0x80) 		//checks if the counter overflow flag is set and if the TCNT1 part of the oTime variable is less than 128
	{
		oTime += 0x100;   //compensate because overflow had not been counted
	}

}

void servoPosC (uint16_t pos, uint8_t speed)		//sevo travel clockwise - position and speed
{
	while (hPulse <= pos)		
	{
		timerSetup();
		
		if (oTime > hPulse)
		{
			PORTB &= ~(1 << servoPin);
		}
		
		if (oTime > tPulse-hPulse)
		{
			PORTB |= (1 << servoPin);
			
			hPulse += speed;
			
			oTime = 0;
			tot_overflow = 0;		//resets tot_overflow variable  
			TIFR |= (1 << TOV1);		// clear counter1 overflow-flag
			TCNT1 = 0;		//resets Timer/Counter1
			
		}
		
	}
	
}

void servoPosCC (uint16_t pos, uint8_t speed)		//sevo travel counterclockwise - position and speed
{
	while (hPulse >= pos)
	{
		timerSetup();
		
		if (oTime > hPulse)
		{
			PORTB &= ~(1 << servoPin);
		}
		
		if (oTime > tPulse-hPulse)
		{
			PORTB |= (1 << servoPin);
			
			hPulse -= speed;		
			
			oTime = 0;
			tot_overflow = 0;		//resets tot_overflow variable  
			TIFR |= (1 << TOV1);		// clear counter1 overflow-flag
			TCNT1 = 0;		//resets Timer/Counter1
			
		}
		
	}
	
}


int main(void)
{
	DDRB |= (1 << servoPin);
	
	TCCR1 |= (1 << CS12);	//set Timer/Counter1 prescaler to increment every 1 µs (PCK/8)
	
	while(1)		
	{
		servoPosC(2600, 10);		//travel clockwise - position and speed
			
		servoPosCC(2000, 5);		//travel counterclockwise  -position and speed
	
		servoPosCC(600, 20);		//travel counterclockwise  - position and speed
		
		servoPosC(900, 1);		//travel clockwise - position and speed
	}
	
}	
