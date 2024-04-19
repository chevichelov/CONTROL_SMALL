#pragma once
#include <Arduino.h>

void DISPLAY_INT(void) 
{
	DDRB  &= ~((1<<I2C_SDA)|(1<<I2C_SCL)); 
	PORTB &= ~((1<<I2C_SDA)|(1<<I2C_SCL));
}

void DISPLAY_WRITE(uint8_t DATA) 
{
	for(uint8_t i = 8; i; i--) 
	{
		I2C_SDA_LOW();                        
		if (DATA & 0x80) 
			I2C_SDA_HIGH();
		I2C_SCL_HIGH();
		DATA<<=1;
		I2C_SCL_LOW();
	}
	I2C_SDA_HIGH();
	I2C_SCL_HIGH();
	asm("nop");
	I2C_SCL_LOW();                         
}

void DISPLAY_START(uint8_t ADDRESS) 
{
	I2C_SDA_LOW();
	I2C_SCL_LOW();
	DISPLAY_WRITE(ADDRESS);
}

void DISPLAY_STOP(void) 
{
	I2C_SDA_LOW();
	I2C_SCL_HIGH();
	I2C_SDA_HIGH();
}

const uint8_t DISPLAY_INIT_CMD[] PROGMEM = 
{
  0xA8, 0x1F,       
  0x22, 0x00, 0x03, 
  0x20, 0x01,       
  0xDA, 0x02,       
  0x8D, 0x14,       
  0xAF,             
  0x00, 0x10, 0xB0, 
  0xA1, 0xC8        
};
 
const uint8_t DISPLAY_FONT[] PROGMEM =            //Здесь я задал шрифты для дисплея, первое значение - что отображается на дисплее, второе - код символа
{
  0x7F, 0x41, 0x7F, // 0  0
  0x00, 0x00, 0x7F, // 1  1
  0x79, 0x49, 0x4F, // 2  2
  0x41, 0x49, 0x7F, // 3  3
  0x0F, 0x08, 0x7E, // 4  4
  0x4F, 0x49, 0x79, // 5  5
  0x7F, 0x49, 0x79, // 6  6
  0x03, 0x01, 0x7F, // 7  7
  0x7F, 0x49, 0x7F, // 8  8
  0x4F, 0x49, 0x7F, // 9  9
  0x7F, 0x09, 0x7F, // A 10
  0x7F, 0x02, 0x7F, // M 11
  0x46, 0x49, 0x31, // S 12
  0x00, 0x60, 0x00  // . 13 
};
 
void DISPLAY_CURSOR(uint8_t X, uint8_t Y) 
{
	DISPLAY_START(OLED_ADDR);                   
	DISPLAY_WRITE(OLED_CMD_MODE);               
	DISPLAY_WRITE(X & 0x0F);                 
	DISPLAY_WRITE(0x10 | (X >> 4));     
	DISPLAY_WRITE(0xB0 | (Y & 0x07));    
	DISPLAY_STOP();                           
}
 
void DISPLAY_CLEAR(void) 
{
	DISPLAY_CURSOR(0, 0);                      
	DISPLAY_START(OLED_ADDR);                   
	DISPLAY_WRITE(OLED_DAT_MODE);              
	for(uint16_t i=512; i; i--) 
		DISPLAY_WRITE(0x00); 
	DISPLAY_STOP();  
}
 
void DISPLAY_INIT(void) 
{
  DISPLAY_INT();                             
  DISPLAY_START(OLED_ADDR);                   
  DISPLAY_WRITE(OLED_CMD_MODE);               
  for (uint8_t i = 0; i < DISPLAY_INIT_LEN; i++) 
	  DISPLAY_WRITE(pgm_read_byte(&DISPLAY_INIT_CMD[i])); 
  DISPLAY_STOP();                            
}
 
uint8_t DISPLAY_STRETCH(uint8_t I)
{
	I=((I & 2)<<3)|(I&1);
	I|=I<<1;
	I|=I<<2;
	return I;
}
 
void DISPLAY_PRINT_TWO(uint8_t DATA) 
{
	uint8_t I, II, III, IV;                     
	uint8_t INFO[4];                          
	DATA += DATA << 1;                          
	for(I=8; I; I--) 
		DISPLAY_WRITE(0x00);       
	for(I=3; I; I--) 
	{                      
		IV = pgm_read_byte(&DISPLAY_FONT[DATA++]);  
		for(II=0; II<4; II++, IV >>= 2) 
		  INFO[II] = DISPLAY_STRETCH(IV);  
		II=4; 
		if(I==2) 
		  II=6;               
		while(II--) 
		{                    
		  for(III=0; III<4; III++) 
			DISPLAY_WRITE(INFO[III]);
		}
	}
}
 
void DISPLAY_PRINT(uint8_t *DATA) 
{
	DISPLAY_START(OLED_ADDR);      
	DISPLAY_WRITE(OLED_DAT_MODE); 
              
	for(uint8_t i=0; i<5; i++) 
		DISPLAY_PRINT_TWO(DATA[i]);  
	DISPLAY_STOP();                            
}
