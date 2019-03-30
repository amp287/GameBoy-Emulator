#include <string.h>
#include "Sprite.h"
#include "Memory.h"
#include "PPU.h"
#include "PPU_Utils.h"
#include <stdio.h>

#define SPRITE_TILE_ADDRESS 0x8000

OAM_ENTRY sprites[40];
int number_of_sprites;

void modify_sprite_8(unsigned int address, unsigned char sprite_data) {
    //0xFE00-0xFE9F oam space
    int index = (address - 0xFE00) / 4;
    int data_byte = (address - 0xFE00) % 4;
    unsigned char options;

    if(index >= 40) {
        printf("Sprite.add_sprite(): Index greater than 40! address = %x\n", address);
        return;
    }

    OAM_ENTRY *object = &sprites[index];
    switch(data_byte) {
        case 0:
            object->x = sprite_data;
            break;
        case 1:
            object->y = sprite_data;
            break;
        case 2:
            object->tile_number = sprite_data;
            break;
        case 3:
            options = (unsigned char)sprite_data;
            object->pallete = options & 0x01;
            object->flip_x = options & 0x02;
            object->flip_y = options & 0x03;
            object->priority = options & 0x04;
    }
    return;
}

void sprite_get_tile(unsigned char tile_number, int sprite_height, unsigned short *tile_out) {
    int i;

    for(i = 0; i < sprite_height; i++) {
        tile_out[i] = read_16_bit((SPRITE_TILE_ADDRESS + tile_number * 16) + (i * 2));
    }
}

void sprite_draw(int scanline, unsigned char pixels[160][3]) {
    int sprite_index;
    int sprite_height = (read_8_bit(LCD_CONTROL) & SPRITE_SIZE) == 0 ? 8 : 16; 
    int i;

    for(sprite_index = 0; sprite_index < 40; sprite_index++) {
        OAM_ENTRY sprite = sprites[sprite_index];
        int y_pos = sprite.y - 16;
        int x_pos = sprite.x - 8;

        if(scanline >= y_pos  && scanline < (y_pos + sprite_height)) {
            unsigned short tile[16]; 
            int y_tile_row = scanline - y_pos;
            int x_start = x_pos < 8 ? 8 - x_pos : 0;
            int number_of_pixels_to_draw = 8 - x_start;

            if(y_tile_row < 0)  {
                printf("ERROR y_tile_row < 0 !! tilerow:%d scanline: %d y_pos:%d\n", y_tile_row, scanline, y_pos);
                getchar();
            }

            sprite_get_tile(sprite.tile_number, sprite_height, tile);

            for(i = 0; i < number_of_pixels_to_draw; i++) {
                unsigned short row = tile[y_tile_row] << i;
                unsigned char pixel = get_pixel(row);

                if((row & 0x8080) == 0) continue;
                //if(pixel == 0) continue;
                pixels[x_pos + i][0] = pixel;
                pixels[x_pos + i][1] = pixel;
                pixels[x_pos + i][2] = pixel;
            }
        }
        
    }
}