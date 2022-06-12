#define F_CPU 8000000
#include <avr/io.h>
#include <stdio.h>
#include "LCD.h"

const uint8_t kaypad[4][4]={{7,8,9,10},{4,5,6,11},{1,2,3,12},{13,0,14,15}};
uint8_t getkeyNum();
char lcddata[20];
uint8_t key;

uint16_t MOIS_Reading=0; //analog Reading variable
uint16_t TempReading=0; //analog Reading variable
uint16_t ReadADC(uint8_t ADCchannel); //analog reading function

uint16_t templimit=0;
void getTemp();
uint16_t moisturelimit=0;
void getMoist();
uint16_t weightlimit=0;
void getweight();

#define ControlPort PORTC
#define ControlDDR DDRC
#define ControlPIN PINC
#define DIRL 4
#define STEPL 3
#define STEP1 2
#define gearMotor 5
#define blower 6
#define heater 7

void stepperE(uint8_t side,uint16_t cycle);////////////////////////////
void stepperMotor(uint8_t side,uint16_t cycle);
void stepperR(uint8_t side,uint16_t cycle);
uint8_t hx711H=0; //Load Scale High Bits
uint16_t hx711L=0;//Load Scale Low Bits
float loadCellRead();

int main(void)
{
   LcdInit();
   ControlDDR|=(1<<DIRL)|(1<<STEPL)|(1<<STEP1)|(1<<gearMotor)|(1<<blower)|(1<<heater)|(1<<1); 
   PORTC&=~(1<<0);//Clock pin low
   ADCSRA |= ((1<<ADPS2)|(1<<ADPS1)|(1<<ADPS0));   // prescaler 128
   ADMUX |= (1<<REFS0);					//AVCC
   ADCSRA |= (1<<ADEN);                 // Turn on ADC
   while(1){
	   key=getkeyNum();
	   if (key==13)
	   {break;
	   }
	   
   }
   LcdSetCursor(4,0,"Welcome");
   _delay_ms(500);
   LcdCommand(LCD_CLEARDISPLAY);
   
   getTemp();
   getMoist();
   LcdCommand(LCD_CLEARDISPLAY);
   LcdSetCursor(0,0,"Starting");
   
   ControlPort|=(1<<gearMotor);
   _delay_ms(1000);
    ControlPort&=~(1<<gearMotor);
	
  stepperMotor(1,500);
  ControlPort|=(1<<blower)|(1<<heater); 
    while (1) 
    {
		MOIS_Reading=ReadADC(1); //calibrated number
		TempReading=(ReadADC(0)*0.50048876); //calibrated number
		
		
		sprintf(lcddata,"Moist. %04u",MOIS_Reading);
		LcdSetCursor(0,0,lcddata);
		sprintf(lcddata,"Temp %03uC",TempReading);
		LcdSetCursor(0,1,lcddata);
		
		if ((MOIS_Reading<moisturelimit)&&(TempReading>templimit))
		{LcdCommand(LCD_CLEARDISPLAY);
			LcdSetCursor(0,0,"Limits Reached");
			_delay_ms(2000);//////////////////////////////////////clean up delay
			ControlPort&=~((1<<gearMotor)|(1<<blower)|(1<<heater)); 
			break;
		}
		
    }
	
	stepperR(1,500);
	
	 ControlPort|=(1<<gearMotor);
	 _delay_ms(1000);
	 ControlPort&=~(1<<gearMotor);
	 stepperMotor(0,1000);
	 //stepperR(0,1000);
	 	uint8_t breakv=0;
	/*A start*/
	while(1){
		breakv=0;
		getweight();
		sprintf(lcddata,"limit   %1.3f kg",weightlimit);
		LcdSetCursor(0,0,lcddata);
		while(1){
			float readW=loadCellRead();
			sprintf(lcddata,"Reading %1.3f kg",readW);
			LcdSetCursor(0,1,lcddata);
			
			for(uint8_t i=0;i<100;i++){
				_delay_us(1000);
				stepperE(0,1000);             ///////////////////////////////
				if (readW>=weightlimit)
				{LcdCommand(LCD_CLEARDISPLAY);
					LcdSetCursor(0,0,"Process done!");
					stepperE(0,500);
					_delay_ms(1000);
					breakv=1;
					break;
				}
			}
			
			if (breakv)
			{break;
			}
			
		
	}
	
	/*A end*/
	

	}
	
}

uint8_t getkeyNum(){
	DDRB=0b00001111;
	PORTB=0b11110000;
	uint8_t getx;
	uint8_t gety;
	while(1){
		
		uint8_t pin=(~(PINB|0x0F));
		if (pin)
		{
			
			
			switch(pin){
				
				
				case (1<<4) :getx=0; break;
				case (1<<5) :getx=1;break;
				case (1<<6) :getx=2;break;
				case (1<<7) :getx=3;break;
				
				
			} //switch
			DDRB=0b0;
			PORTB=0b0;
			
			DDRB=0b11110000;
			PORTB=0b00001111;
			
			pin=~(PINB|0xF0);
			
			switch(pin){
				case (1<<0) :gety=0;break;
				case (1<<1) :gety=1;break;
				case (1<<2) :gety=2;break;
				case (1<<3) :gety=3;break;
				
			} //switch
			
			DDRB=0b00001111;
			PORTB=0b11110000;
			
			
			return kaypad[gety][getx];
			
		}//if
		
		
		
	}//while 1
}

void getTemp(){
	
	LcdCommand(LCD_CLEARDISPLAY);
	LcdSetCursor(0,0,"Set temperature");
	_delay_ms(500);
	uint8_t pos=0;
	uint8_t numbers[3]={0,0,0};
	sprintf(lcddata,"%u%u%u",numbers[0],numbers[1],numbers[2]);
	LcdSetCursor(0,1,lcddata);
	LcdSetCursor(pos,1,"");
	LcdCommand(LCD_DISPLAYCONTROL|LCD_DISPLAYON|LCD_BLINKON);
	
	while(1){
		key=getkeyNum();
		if (key<10)
		{
			numbers[pos]=key;
			sprintf(lcddata,"%u%u%u",numbers[0],numbers[1],numbers[2]);
			LcdSetCursor(0,1,lcddata);
			pos++;
			if (pos>2)
			{pos=0;
			}
			LcdSetCursor(pos,1,"");
			_delay_ms(200);
		}
		
		else if (key==14)
		{LcdCommand(LCD_CLEARDISPLAY);
		LcdCommand(LCD_DISPLAYCONTROL|LCD_DISPLAYON|LCD_BLINKOFF);
		break;
		}
	}
	templimit=(numbers[0]*100)+(numbers[1]*10)+numbers[2];
	LcdSetCursor(0,0,"Temperature set!");
	sprintf(lcddata,"%u",templimit);
	LcdSetCursor(0,1,lcddata);
	_delay_ms(1000);
	LcdCommand(LCD_CLEARDISPLAY);
	
}

void getMoist(){
	LcdCommand(LCD_CLEARDISPLAY);
	LcdSetCursor(0,0,"Set Moisture");
	_delay_ms(500);
	uint8_t pos=0;
	uint8_t numbers[4]={0,0,0,0};
	sprintf(lcddata,"%u%u%u%u",numbers[0],numbers[1],numbers[2],numbers[3]);
	LcdSetCursor(0,1,lcddata);
	LcdSetCursor(pos,1,"");
	LcdCommand(LCD_DISPLAYCONTROL|LCD_DISPLAYON|LCD_BLINKON);
	
	while(1){
		key=getkeyNum();
		if (key<10)
		{
			numbers[pos]=key;
			sprintf(lcddata,"%u%u%u%u",numbers[0],numbers[1],numbers[2],numbers[3]);
			LcdSetCursor(0,1,lcddata);
			pos++;
			if (pos>3)
			{pos=0;
			}
			LcdSetCursor(pos,1,"");
			_delay_ms(200);
		}
		
		else if (key==14)
		{LcdCommand(LCD_CLEARDISPLAY);
			LcdCommand(LCD_DISPLAYCONTROL|LCD_DISPLAYON|LCD_BLINKOFF);
			break;
		}
	}
	moisturelimit=(numbers[0]*1000)+(numbers[1]*100)+(numbers[2]*10)+numbers[3];
	LcdSetCursor(0,0,"Moisture set!");
	sprintf(lcddata,"%u",moisturelimit);
	LcdSetCursor(0,1,lcddata);
	_delay_ms(1000);
	LcdCommand(LCD_CLEARDISPLAY);	
	
	
}

void getweight(){
	LcdCommand(LCD_CLEARDISPLAY);
	LcdSetCursor(0,0,"Set Weight(g)");
	_delay_ms(500);
	uint8_t pos=0;
	uint8_t numbers[4]={0,0,0,0};
	sprintf(lcddata,"%u%u%u%u",numbers[0],numbers[1],numbers[2],numbers[3]);
	LcdSetCursor(0,1,lcddata);
	LcdSetCursor(pos,1,"");
	LcdCommand(LCD_DISPLAYCONTROL|LCD_DISPLAYON|LCD_BLINKON);
	
	while(1){
		key=getkeyNum();
		if (key<10)
		{
			numbers[pos]=key;
			sprintf(lcddata,"%u%u%u%u",numbers[0],numbers[1],numbers[2],numbers[3]);
			LcdSetCursor(0,1,lcddata);
			pos++;
			if (pos>3)
			{pos=0;
			}
			LcdSetCursor(pos,1,"");
			_delay_ms(200);
		}
		
		else if (key==14)
		{LcdCommand(LCD_CLEARDISPLAY);
			LcdCommand(LCD_DISPLAYCONTROL|LCD_DISPLAYON|LCD_BLINKOFF);
			break;
		}
	}
	weightlimit=(numbers[0]*1000)+(numbers[1]*100)+(numbers[2]*10)+numbers[3];
	LcdSetCursor(0,0,"Weight set!");
	sprintf(lcddata,"%ug",weightlimit);
	LcdSetCursor(0,1,lcddata);
	_delay_ms(500);
	LcdCommand(LCD_CLEARDISPLAY);	
	
}

void stepperMotor(uint8_t side,uint16_t cycle){
	ControlPort=(ControlPort&~(1<<DIRL))|(side<<DIRL);

	for(uint16_t i=0;i<cycle;i++){
		ControlPort|=(1<<STEPL);
		_delay_us(1000);
		ControlPort&=~(1<<STEPL);
		_delay_us(1000);
	}
}

void stepperR(uint8_t side,uint16_t cycle){
	ControlPort=(ControlPort&~(1<<DIRL))|(side<<DIRL);

	for(uint16_t i=0;i<cycle;i++){
		ControlPort|=(1<<STEP1);
		_delay_us(1000);
		ControlPort&=~(1<<STEP1);
		_delay_us(1000);
	}
}

void stepperE(uint8_t side,uint16_t cycle){
	ControlPort=(ControlPort&~(1<<DIRL))|(side<<DIRL);

	for(uint16_t i=0;i<cycle;i++){
		PORTD|=(1<<3);
		_delay_us(1000);
		PORTD&=~(1<<3);
		_delay_us(1000);
	}
}

uint16_t ReadADC(uint8_t ADCchannel)
{
	//select ADC channel with safety mask
	ADMUX = (ADMUX & 0xF0) | (ADCchannel & 0x0F);
	//single conversion mode
	ADCSRA |= (1<<ADSC);
	// wait until ADC conversion is complete
	while( ADCSRA & (1<<ADSC) );
	return ADCW;
}

float loadCellRead(){
	hx711H=0;hx711L=0;  //clear variables
	for(uint8_t i=0;i<8;i++){  // Load cell data high 8 bits
		PORTC|=(1<<0); //Clock pin high
		_delay_us(10);
		if ((PINC&(1<<1))>>1)  //read data pin
		{hx711H|=(1<<(7-i));//set hx 711 varible
		}
		else
		{hx711H&=~(1<<(7-i));
		}
		PORTC&=~(1<<0); //Clock pin low
		_delay_us(5);
	}
	
	
	for(uint8_t i=0;i<16;i++){ // Load cell data low 16 bits
		PORTC|=(1<<0); //Clock pin high
		_delay_us(10);
		if ((PINC&(1<<1))>>1) //read data pin
		{hx711L|=(1<<(15-i));
		}
		else
		{hx711L&=~(1<<(15-i));
		}
		PORTC&=~(1<<0); //Clock pin low
		_delay_us(5);
	}
	
	hx711L=hx711L>>1; //shift bits
	
	if (hx711H&1)  //bit setup
	{hx711L|=(1<<15);
	}
	else
	{hx711L&=~(1<<15);
	}
	hx711H=hx711H>>1;
	
	return (hx711H*(65536/18029.6))+hx711L/18029.6; //load cell calibration
}