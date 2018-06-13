//The LCD control register bits: http://www.codeslinger.co.uk/pages/projects/gameboy/graphics.html
#define LCD_ENABLED 0x80
#define WINDOW_TILE_MAP_SELECT 0x40
#define WINDOW_DISP_ENABLE 0x20
#define BG_AND_WINDOW_TILE_DATA_SELECT 0x10
#define BG_TILE_MAP_SELECT 0x8
#define SPRITE_SIZE 0x4
#define SPRITE_DISPLAY 0x2
#define BG_DISPLAY 0x1

#define LCD_STATUS_INTERRUPT_ENABLE 0x78
#define LCD_STATUS_HORIZONTAL_BLANK 0x0
#define LCD_STATUS_VERTICAL_BLANK 0x1
#define LCD_STATUS_ACCESS_OAM 0x2
#define LCD_STATUS_ACCESS_VRAM 0x3

#define LCD_MODE_0_CYCLES 204
#define LCD_MODE_1_CYCLES 4560 
#define LCD_MODE_2_CYCLES 80
#define LCD_MODE_3_CYCLES 172


void draw_screen();
void gpu_update(int cycles);
int gpu_init();
int gpu_stop();