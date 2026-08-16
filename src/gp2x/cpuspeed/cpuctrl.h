#if !defined(_CPUCTRL_)
#define _CPUCTRL_

int setGP2XClock(int clockSpeed);

int cpu_main(int clockSpeed);

void cpuctrl_init();                      

void save_system_regs();                       
void load_system_regs();

void set_FCLK(unsigned MHZ);                                             
void set_add_ULCDCLK(int addclock);
void set_add_FLCDCLK(int addclock);

unsigned get_FCLK();
unsigned get_freq_UCLK();
unsigned get_freq_ACLK();
unsigned get_freq_920_CLK();
unsigned get_freq_940_CLK();
unsigned get_freq_DCLK();
unsigned get_LCDClk();
unsigned get_state940();

void set_920_Div(unsigned short div);                                         
unsigned short get_920_Div();

void set_940_Div(unsigned short div);                                         
unsigned short get_940_Div();

void set_DCLK_Div(unsigned short div);                                         
unsigned short get_DCLK_Div();

unsigned short Disable_Int_920();
unsigned short Disable_Int_940();

void Enable_Int_920(unsigned short flag);
void Enable_Int_940(unsigned short flag);

void Disable_940();             

extern volatile unsigned  *arm940code;                               

void Load_940_code(unsigned *code,int size);                                                   

void clock_940_off();              
void clock_940_on();                 

#endif
