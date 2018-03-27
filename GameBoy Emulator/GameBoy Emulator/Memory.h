#define MEM_SIZE 65536

unsigned char memory[MEM_SIZE];

unsigned char read_8_bit(unsigned short addr);
unsigned short read_16_bit(unsigned short addr);

//TODO add in case for copy of internal ram at(E000)
//^Look at GBCPUman.pdf Page 9 2.5.2
void write_8_bit(unsigned short addr, unsigned char val);
void write_16_bit(unsigned short addr, unsigned short val);