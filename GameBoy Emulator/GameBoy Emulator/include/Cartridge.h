
int load_rom(char *path);

void switch_rom_bank(unsigned char bank);

// switches the ram bank or rom set depending on the Cartridge type and mode
void switch_ram_bank_rom_set(unsigned char bank);

// Read from the current rom bank
// if reading from 0-0x3FFF set bank_0 = 1
unsigned char read_rom_bank_8_bit(unsigned short addr, int bank_0);

unsigned char read_ram_bank_8_bit(unsigned short addr);

void write_ram_bank_8_bit(unsigned short addr, unsigned char val);

void enable_disable_cart_ram(unsigned char val);

void switch_cart_mode(unsigned char mode);