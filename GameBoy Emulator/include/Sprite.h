// 4 bytes in memory
// [x pos][y pos][tile number][flags] (flags include flip)
typedef struct {
    unsigned char x;
    unsigned char y;
    unsigned char tile_number;
    unsigned char flip_x;
    unsigned char flip_y; 
    // 1: draws on background 00 and draws behind others
    // 0: draws on top of everything (except the trasparent pixels)
    unsigned char priority; 
    // 0 for pallete at ff48 and 1 for pallete at ff49
    unsigned char pallete; 
} OAM_ENTRY;

// ff48 sprite pallete 0 (r/w)
// ff49 sprite pallete 1 (r/w)

void modify_sprite_8(unsigned int address, unsigned char sprite_data);
void sprite_draw(int scanline, unsigned char pixels[160][3]);