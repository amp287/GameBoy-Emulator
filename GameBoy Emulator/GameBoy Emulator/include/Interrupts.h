#define INTERRUPT_VBLANK 0x1
#define INTERRUPT_LCD 0x2
#define INTERRUPT_TIMER 0x4
#define INTERRUPT_SERIAL 0x8
#define INTERRUPT_JOYPAD 0x10

void request_interrupt(unsigned char type);
int check_interrupts();
// wait: sets whether interrupts should immediately disable
// or wait an instruction
void reset_master_interrupt(int wait);
// wait: sets whether interrupts should immediately disable
// or wait an instruction
void set_master_interrupt(int wait);