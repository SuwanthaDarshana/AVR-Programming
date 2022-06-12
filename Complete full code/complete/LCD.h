/*
 * _1602LCDI2C.h
 *
 * Created: 5/16/2020 12:32:15 AM
 *  Author: Dilan
 */ 


#ifndef LCDI2C_H_
#define LCDI2C_H_

#define LCD_CLEARDISPLAY 0x01
#define LCD_RETURNHOME 0x02
#define LCD_ENTRYMODESET 0x04
#define LCD_DISPLAYCONTROL 0x08
#define LCD_CURSORSHIFT 0x10
#define LCD_FUNCTIONSET 0x20
#define LCD_SETCGRAMADDR 0x40
#define LCD_SETDDRAMADDR 0x80

// flags for display entry mode
#define LCD_ENTRYRIGHT 0x00
#define LCD_ENTRYLEFT 0x02
#define LCD_ENTRYSHIFTINCREMENT 0x01
#define LCD_ENTRYSHIFTDECREMENT 0x00

// flags for display on/off control
#define LCD_DISPLAYON 0x04
#define LCD_DISPLAYOFF 0x00
#define LCD_CURSORON 0x02
#define LCD_CURSOROFF 0x00
#define LCD_BLINKON 0x01
#define LCD_BLINKOFF 0x00

// flags for display/cursor shift
#define LCD_DISPLAYMOVE 0x08
#define LCD_CURSORMOVE 0x00
#define LCD_MOVERIGHT 0x04
#define LCD_MOVELEFT 0x00

// flags for function set
#define LCD_8BITMODE 0x10
#define LCD_4BITMODE 0x00
#define LCD_2LINE 0x08
#define LCD_1LINE 0x00
#define LCD_5x10DOTS 0x04
#define LCD_5x8DOTS 0x00


//LcdCommand(LCD_DISPLAYCONTROL|LCD_DISPLAYON|LCD_BLINKON);  Blinlk on
//LcdCommand(LCD_CURSORSHIFT|LCD_CURSORMOVE|LCD_MOVERIGHT);  Cursor right
//LcdCommand(LCD_CURSORSHIFT|LCD_DISPLAYMOVE|LCD_MOVELEFT);  Display shift

#include <avr/io.h>
#include <util/delay.h>



void LcdInit();
void LcdCommand(uint8_t command);
void Lcd4bitdata(uint8_t bitdata,uint8_t RSvalue);
void LcdChar(uint8_t Lcdcharval);
void LcdString (char *str);
void LcdBacklight(uint8_t Backlightdata);
void LcdSetCursor(uint8_t xaxis, uint8_t yaxis,char*str);
void LcdCursor(uint8_t Cursor);
void LcdScroll(uint8_t side);



uint8_t deviceAdress; //I2C Adress
uint8_t Backlight=1; //Backlight state


/*Define Enable & Reset */
#define EN 0b00000100
#define RST 0b00000001





void LcdInit(){
	LcdBacklight(1);
	_delay_ms(20); // LCD power On delay
	
	DDRD=0xFF;
	
	
	LcdCommand(LCD_RETURNHOME);		/*Return home  */

	LcdCommand(LCD_FUNCTIONSET|LCD_4BITMODE|LCD_2LINE);              /* 2 line, 5*7 matrix in 4-bit mode */
	LcdCommand(LCD_DISPLAYCONTROL|LCD_DISPLAYON|LCD_CURSOROFF);              /* Display on cursor off*/
	LcdCommand(LCD_ENTRYMODESET|LCD_ENTRYLEFT|LCD_ENTRYSHIFTDECREMENT);              /* Increment cursor (shift cursor to right)*/
	LcdCommand(LCD_CLEARDISPLAY);
	
	
	
}

void LcdBacklight(uint8_t Backlightdata){
	if (Backlightdata==1)
	{Backlight=0b00001000;
	} 
	else
	{Backlight=0b00000000;
	}
	
}

void LcdCommand(uint8_t command){
	Lcd4bitdata(command&0xF0,0);
	Lcd4bitdata((command<<4)&0xF0,0);
	
}

void LcdChar(uint8_t Lcdcharval){
	Lcd4bitdata(Lcdcharval&0xF0,1);
	Lcd4bitdata((Lcdcharval<<4)&0xF0,1);
	
}

void LcdSetCursor(uint8_t xaxis, uint8_t yaxis,char*str){
	uint8_t tempcommand;
	switch(yaxis){
		case 0:tempcommand=0x80;break;  //yaxis =0
		case 1:tempcommand=0xC0;break;
		case 2:tempcommand=0x94;break;
		case 3:tempcommand=0xd4;break;		
	}
	LcdCommand(tempcommand|(0x0F&xaxis));
	LcdString(str);
	
}

void LcdString (char *str)		/* Send string to LCD function */
{
	int i;
	for(i=0;str[i]!=0;i++)		/* Send each char of string till the NULL */
	{
		LcdChar(str[i]);
	}
}
void LcdCursor(uint8_t Cursor){
	if (Cursor==0)
	{LcdCommand(0xC);
	} 
	else
	{
	}
	
}



void Lcd4bitdata(uint8_t bitdata,uint8_t RSvalue){
	
	PORTD=((bitdata|RSvalue|Backlight));
	
	PORTD=(((bitdata|EN)|RSvalue|Backlight));
	
	_delay_us(200);
	
	PORTD=(((bitdata&~EN)|RSvalue|Backlight));
	
	_delay_ms(2);
	
}




#endif /* 1602LCDI2C_H_ */

