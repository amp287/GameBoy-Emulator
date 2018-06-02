#include <stdio.h>
#include "Memory.h"
#include "GPU.h"
#include "Display.h"
#include "Debug.h"
#include "Interrupts.h"

#define LCD_STATUS_MODE 0x3
#define LCD_STATUS_COINCIDENCE_FLAG 0x4
#define LCD_STATUS_HORIZONTAL_BLANK_INTERRUPT 0x8
#define LCD_STATUS_VERTICAL_BLANK_INTERRUPT 0x10
#define LCD_STATUS_OAM_INTERRUPT 0x20
#define LCD_STATUS_COINCIDENCE_INTERRUPT 0x40

#define SCROLL_Y 0xFF42
#define SCROLL_X 0xFF43
#define WINDOW_Y 0xFF4A
#define WINDOW_X 0xFF4B
#define TILE_SIZE 16

typedef struct LCD_STATUS_REGISTER {
	unsigned char lyc_ly_interrupt;
	unsigned char oam_interrupt;
	unsigned char vblank_interrupt;
	unsigned char hblank_interrupt;
	unsigned char coincidence_flag;
	unsigned char mode_flag;
}LCD_STATUS_REGISTER;

static const char *window_title = "Gameboy";
static int quit;
static GLFWwindow* gameboy_window;

int x = 0;
// It takes the GPU 456 cycles to draw one scanline
int scanline_cycles;

static unsigned char screen_buffer[144][160][3];

void set_scanline(unsigned char line) {
	io[LCD_SCANLINE - 0xFF00] = line;
}

unsigned char get_scanline() {
	return io[LCD_SCANLINE - 0xFF00];
}

void get_lcd_status(LCD_STATUS_REGISTER *reg) {
	unsigned char status = read_8_bit(LCD_STATUS_REG);
	reg->mode_flag = status & LCD_STATUS_MODE;
	reg->coincidence_flag = status & LCD_STATUS_COINCIDENCE_FLAG;
	reg->hblank_interrupt = status & LCD_STATUS_HORIZONTAL_BLANK_INTERRUPT;
	reg->vblank_interrupt = status & LCD_STATUS_VERTICAL_BLANK_INTERRUPT;
	reg->oam_interrupt = status & LCD_STATUS_OAM_INTERRUPT;
}

void set_lcd_status(LCD_STATUS_REGISTER reg) {
	unsigned char status = 0;
	status |= reg.lyc_ly_interrupt;
	status = status << 1;
	status |= reg.oam_interrupt;
	status = status << 1;
	status |= reg.vblank_interrupt;
	status = status << 1;
	status |= reg.hblank_interrupt;
	status = status << 1;
	status |= reg.coincidence_flag;
	status = status << 2;
	status |= reg.mode_flag;

	write_8_bit(LCD_STATUS_REG, status);
}

void lcd_interrupt(LCD_STATUS_REGISTER reg, unsigned char type) {
	switch (type) {
		case LCD_STATUS_COINCIDENCE_INTERRUPT:
			if (reg.coincidence_flag)
				request_interrupt(INTERRUPT_LCD);
			break;
		case LCD_STATUS_HORIZONTAL_BLANK_INTERRUPT:
			if (reg.hblank_interrupt)
				request_interrupt(INTERRUPT_LCD);
			break;
		case LCD_STATUS_VERTICAL_BLANK_INTERRUPT:
			if (reg.vblank_interrupt)
				request_interrupt(INTERRUPT_VBLANK);
			break;
		case LCD_STATUS_OAM_INTERRUPT:
			if (reg.oam_interrupt)
				request_interrupt(INTERRUPT_LCD);
	}
}

// Returns color 
unsigned char get_pixel(unsigned short tile_row) {
	unsigned short color = tile_row & 0x8080;

	if (color == 0x0)
		return 255;
	else if (color == 0x8000)
		return 192;
	else if (color == 0x0008)
		return 96;
	else
		return 0; 
}

void get_tile(unsigned short addr, unsigned short *tile) {
	int i; 
	
	for (i = 0; i < 8; i++) {
		unsigned short shrt = read_16_bit(addr + (i * 2));
		tile[i] = shrt;
	}
}

void render_scanline_tiles() {
	unsigned char scroll_y = read_8_bit(SCROLL_Y);
	unsigned char scroll_x = read_8_bit(SCROLL_X);

	unsigned char lcd_control = read_8_bit(LCD_CONTROL);
	unsigned char window = lcd_control & WINDOW_DISP_ENABLE;
	unsigned char scanline = read_8_bit(LCD_SCANLINE);

	int pixel = 0;
	unsigned short tile_map = lcd_control & BG_TILE_MAP_SELECT ? TILE_MAP_1 : TILE_MAP_0;
	unsigned short tile_set = lcd_control & BG_AND_WINDOW_TILE_DATA_SELECT ? TILE_SET_1 : TILE_SET_0;
	

	// starting tile y 0-31
	unsigned char starting_tile_y = ((scroll_y + scanline) / 8) % 32;

	// get the x and y pixels to start at in the starting tile
	unsigned char tile_x = scroll_x % 8;
	unsigned char tile_y = (scroll_y + scanline) % 8;

	//TODO: check to see what tile source to use

	while(pixel < 160) {
		unsigned short tile[8];
		unsigned short starting_tile;
	
		//starting tile x : 0-31
		unsigned char starting_tile_x = scroll_x / 8;
		starting_tile_x = (starting_tile_x + (pixel / 8)) % 32;

		starting_tile = starting_tile_x + (starting_tile_y * 32);
		
		short tile_id = vram[(tile_map - 0x8000) + starting_tile];

		if (tile_set == TILE_SET_0)
			tile_id += 128; //<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

		get_tile((tile_set) + (tile_id * 16), tile);
		unsigned char color;
		int i, i_start;
		if (pixel == 0)
			i_start = tile_x;
		else
			i_start = 0;

		for (i = i_start; i < 8; i++) {
			color = get_pixel(tile[tile_y] << i);

			screen_buffer[scanline][pixel][0] = color;
			screen_buffer[scanline][pixel][1] = color;
			screen_buffer[scanline][pixel][2] = color;
			pixel++;
		}
	}
}

void render_scanline() {
	unsigned char lcd_control = read_8_bit(LCD_CONTROL);
	unsigned char current_scanline = read_8_bit(LCD_SCANLINE);

	if (lcd_control & BG_DISPLAY)
		render_scanline_tiles();

	if (lcd_control & SPRITE_DISPLAY)
		;//render_sprites();
}

void update_lcd() {
	LCD_STATUS_REGISTER status;
	unsigned char lcd_enabled = read_8_bit(LCD_CONTROL) & LCD_ENABLED;
	unsigned char scanline = get_scanline();
	unsigned char interrupt = 0;

	get_lcd_status(&status);

	if (!lcd_enabled) {
		scanline_cycles = 456;
		set_scanline(0);
		status.mode_flag = LCD_STATUS_VERTICAL_BLANK;
		set_lcd_status(status);
		return;
	}

	if (scanline >= 144) {
		status.mode_flag = LCD_STATUS_VERTICAL_BLANK;
		interrupt =  LCD_STATUS_VERTICAL_BLANK_INTERRUPT;
	} else {

		if (scanline_cycles >= 376) {
			status.mode_flag = LCD_STATUS_ACCESS_OAM;
			interrupt = LCD_STATUS_OAM_INTERRUPT;
		} else if (scanline >= 204) {
			status.mode_flag = LCD_STATUS_ACCESS_VRAM;
		} else {
			status.mode_flag = LCD_STATUS_HORIZONTAL_BLANK;
			interrupt = LCD_STATUS_HORIZONTAL_BLANK_INTERRUPT;
		}
	}

	if (interrupt)
		lcd_interrupt(status, interrupt);

	//check for ly == LYC interrupts
	if (read_8_bit(LCD_SCANLINE) == read_8_bit(LCD_SCANLINE_COMPARE)) {
		status.coincidence_flag = 1;
		lcd_interrupt(status, LCD_STATUS_COINCIDENCE_INTERRUPT);
	} else {
		status.coincidence_flag = 0;
	}

	set_lcd_status(status);
}

void gpu_update(int cycles) {
	unsigned char lcd_enabled = read_8_bit(LCD_CONTROL) & LCD_ENABLED;
	unsigned char scanline = get_scanline();

	display_poll_events(gameboy_window);

	update_lcd();

	if (lcd_enabled)
		scanline_cycles -= cycles;
	else
		return;

	if (scanline_cycles <= 0) {

		if (scanline < 144)
			render_scanline();

		set_scanline(++scanline);

		scanline_cycles = 456;

		if (scanline == 144)
			display_update_buffer(gameboy_window, screen_buffer, 160, 144);//vblank interrupt?
		else if (scanline > 153)
			set_scanline(0);
	}
}

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);
}

int gpu_init() {
	
	quit = 0;
	scanline_cycles = 456;

	gameboy_window = display_create_window(160, 144, window_title, key_callback);

	return 0;
}

int gpu_stop() {
	glfwDestroyWindow(gameboy_window);
	return 0;
}
