#define TIMER_CONTROL 0xFF07
#define TIMER_CONTROL_ENABLED 0x4
#define TIMER_CONTROL_FREQ_BITS 0x3
#define TIMER_CONTROL_FREQ_4096 0x0
#define TIMER_CONTROL_FREQ_262144 0x1
#define TIMER_CONTROL_FREQ_65536 0x2
#define TIMER_CONTROL_FREQ_16384 0x3

void set_freq();
unsigned char get_freq();
void timer_update(int cycles);