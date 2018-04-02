#include "Memory.h"
#include "LCD.h"
//The LCD control register bits: http://www.codeslinger.co.uk/pages/projects/gameboy/graphics.html

int scanline_count;

void lcd_status() {
	unsigned char status = read_8_bit(LCD_STATUS_REG);

	if (!(read_8_bit(LCD_CONTROL) & LCD_ENABLED)) {
		scanline_count = 456;
		write_8_bit(LCD_SCANLINE, 0);
		
	}


}
