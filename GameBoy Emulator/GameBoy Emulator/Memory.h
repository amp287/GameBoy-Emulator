#define MEM_SIZE 65536

unsigned char memory[MEM_SIZE];

unsigned char read_8_bit(unsigned short addr);
unsigned short read_16_bit(unsigned short addr);

void write_8_bit(unsigned short addr, unsigned char val);
void write_16_bit(unsigned short addr, unsigned short val);