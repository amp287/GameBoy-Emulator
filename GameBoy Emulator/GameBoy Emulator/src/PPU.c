#include <stdio.h>
#include "Memory.h"
#include "PPU.h"
#include "PPU_Utils.h"
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
#define BG_PALETTE 0xFF47

#define TILE_SIZE 16
#define TILE_ROWS 8
#define MAP_BOUNDS 32

#define PALETTE_00 0x03
#define PALETTE_01 0x0C
#define PALETTE_10 0x30
#define PALETTE_11 0xC0

int can_access_oam_ram;
int can_access_vram;


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

void ppu_dma_transfer(unsigned char address) {
	// multiply by 100 to get real address
	// http://www.codeslinger.co.uk/pages/projects/gameboy/dma.html
	unsigned short real_address = address << 8;

	if (!can_access_oam_ram)
		return;

	for (int i = 0; i < 0xA0; i++)
	{
		write_8_bit(0xFE00 + i, read_8_bit(real_address + i));
	}
}

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

void update_scanline() {
	int i, pixel = 0;
	unsigned char color;
	unsigned char scroll_y = read_8_bit(SCROLL_Y);
	unsigned char scroll_x = read_8_bit(SCROLL_X);
	unsigned char window_x = read_8_bit(WINDOW_X) - 7;
	unsigned char window_y = read_8_bit(WINDOW_Y);
	unsigned char scanline = get_scanline();

	// scroll_y is a pixel (0-255) divide by TILE_ROWS to get tile
	unsigned char tile_map_id_y = ((scroll_y + scanline) / TILE_ROWS) % MAP_BOUNDS;
	unsigned char tile_map_id_x = (scroll_x / TILE_ROWS);

	unsigned char tile_y_row = (scroll_y + scanline) % TILE_ROWS;
	unsigned char tile_x_col = scroll_x % TILE_ROWS;

	unsigned char window_on = read_8_bit(LCD_CONTROL) & WINDOW_DISP_ENABLE;

	if (window_on)
		if (window_y > scanline)
			window_on = 0;
			
	while (pixel < 160) {
		unsigned short tile[8];
		int start = pixel == 0 ? tile_x_col : 0;

		if (window_on && pixel >= window_x)
			get_tile(tile, tile_map_id_x - (window_x / 8), tile_map_id_y, window_on);
		else
			get_tile(tile, tile_map_id_x, tile_map_id_y, window_on);

		for (i = start; i < TILE_ROWS; i++) {
			if (pixel == 160) break;

			color = get_pixel(tile[tile_y_row] << i);

			screen_buffer[scanline][pixel][0] = color;
			screen_buffer[scanline][pixel][1] = color;
			screen_buffer[scanline][pixel][2] = color;
			pixel++;
		}
		tile_map_id_x = (tile_map_id_x + 1) % MAP_BOUNDS;
	}
}

void render_scanline() {
	unsigned char lcd_control = read_8_bit(LCD_CONTROL);
	unsigned char current_scanline = read_8_bit(LCD_SCANLINE);

	if (lcd_control & BG_DISPLAY)
		update_scanline();

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
		can_access_oam_ram = 1;
		can_access_vram = 1;
	} else {

		if (scanline_cycles <= 456 && status.mode_flag != LCD_STATUS_ACCESS_OAM) {
			status.mode_flag = LCD_STATUS_ACCESS_OAM;
			can_access_oam_ram = 0;
			interrupt = LCD_STATUS_OAM_INTERRUPT;
		} else if (scanline_cycles < 376 && status.mode_flag != LCD_STATUS_ACCESS_VRAM) {
			status.mode_flag = LCD_STATUS_ACCESS_VRAM;
			can_access_oam_ram = 0;
			can_access_vram = 0;
		} else if(scanline_cycles < 204 && status.mode_flag != LCD_STATUS_HORIZONTAL_BLANK){
			status.mode_flag = LCD_STATUS_HORIZONTAL_BLANK;
			interrupt = LCD_STATUS_HORIZONTAL_BLANK_INTERRUPT;
			can_access_oam_ram = 1;
			can_access_vram = 1;
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

	// FOR DEBUGGING
	ppu_mode = status.mode_flag;
}

void gpu_update(int cycles) {
	unsigned char lcd_enabled = read_8_bit(LCD_CONTROL) & LCD_ENABLED;
	unsigned char scanline = get_scanline();

	display_poll_events(gameboy_window);

	if (lcd_enabled)
		scanline_cycles -= cycles;
	else
		return;

	update_lcd();

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

	// FOR DEBUGGING
	ppu_ticks = scanline_cycles;
}

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);
}

int gpu_init() {
	
	quit = 0;
	scanline_cycles = 456;

	// FOR DEBUGGNG
	ppu_ticks = scanline_cycles;
	can_access_oam_ram = 1;
	can_access_vram = 1;

	gameboy_window = display_create_window(160, 144, window_title, key_callback);

	return 0;
}

int gpu_stop() {
	glfwDestroyWindow(gameboy_window);
	return 0;
}

int check_vram_access() {
	return can_access_vram;
}

int check_oam_ram_access() {
	return can_access_oam_ram;
}