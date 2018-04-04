//The LCD control register bits: http://www.codeslinger.co.uk/pages/projects/gameboy/graphics.html
#define LCD_ENABLED 0x80
#define WINDOW_TILE_MAP_SELECT 0x40
#define WINDOW_DISP_ENABLE 0x20
#define BG_AND_WINDOW_TILE_DATA_SELECT 0x10
#define BG_TILE_MAP_SELECT 0x8
#define SPRITE_SIZE 0x4
#define SPRITE_DISPLAY 0x2
#define BG_DISPLAY 0x1

#define LCD_STATUS_MODE 0x3
#define LCD_STATUS_COINCIDENCE_FLAG 0x4
#define LCD_STATUS_MODE_0_INTERRUPT 0x8
#define LCD_STATUS_MODE_1_INTERRUPT 0x10
#define LCD_STATUS_MODE_2_INTERRUPT 0x20
#define LCD_STATUS_COINCIDENCE_INTERRUPT 0x40
#define LCD_STATUS_INTERRUPT_ENABLE 0x38
#define LCD_STATUS_MODE_0 0x0
#define LCD_STATUS_MODE_1 0x1
#define LCD_STATUS_MODE_2 0x2
#define LCD_STATUS_MODE_3 0x3

#define LCD_MODE_0_CYCLES 204
#define LCD_MODE_1_CYCLES 4560 
#define LCD_MODE_2_CYCLES 80
#define LCD_MODE_3_CYCLES 172

#define TILE_MAP_0 0x9800
#define TILE_MAP_1 0x9FFF
#define TILE_SET_1 0x8000
#define TILE_SET_0 0x8800

#define PIXEL_COLOR 0x88

void gpu_update(int cycles);
int gpu_init();
int gpu_stop();