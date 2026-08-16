                                                                              
                                                                              
                                                                              
                                                                              
                                                                              
                                                                              
                                                                                 
                                                                              

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "fame.h"
#include "config.h"

                                                                           
#define M68000 000
#define M68020 020

             
                            
                      
#define FAMEC_ADR_BITS  24
#define FAMEC_FETCHBITS 8
#define FAMEC_DATABITS  8


#undef INLINE
#ifndef INLINE
#define INLINE __inline__
#endif

#ifndef FAMEC_EXTRA_INLINE
#define FAMEC_EXTRA_INLINE               
#else
#undef FAMEC_EXTRA_INLINE
#define FAMEC_EXTRA_INLINE INLINE
#endif


#define UVAL64(a) (a ## uLL)


               
#define M68K_OK 0
#define M68K_NO_SUP_ADDR_SPACE 2
#define M68K_INV_REG -1

extern int prefs_cpu_model;
extern int mainMenu_CPU_speed;

                                
                                
                                

#if FAMEC_ADR_BITS < 32
#define M68K_ADR_MASK  ((1 << FAMEC_ADR_BITS)-1)
#else
#define M68K_ADR_MASK  0xFFFFFFFF
#endif
#define M68K_FETCHSFT  (FAMEC_ADR_BITS - FAMEC_FETCHBITS)
#define M68K_FETCHBANK (1 << FAMEC_FETCHBITS)
#define M68K_FETCHMASK (M68K_FETCHBANK - 1)

#define M68K_DATASFT  (FAMEC_ADR_BITS - FAMEC_DATABITS)
#define M68K_DATABANK (1 << FAMEC_DATABITS)
#define M68K_DATAMASK (M68K_DATABANK - 1)

#define M68K_SR_C_SFT   8
#define M68K_SR_V_SFT   7
#define M68K_SR_Z_SFT   0
#define M68K_SR_N_SFT   7
#define M68K_SR_X_SFT   8

#define M68K_SR_M_SFT   12
#define M68K_SR_S_SFT   13
#define M68K_SR_T_SFT   15

#define M68K_SR_C       (1 << M68K_SR_C_SFT)
#define M68K_SR_V       (1 << M68K_SR_V_SFT)
#define M68K_SR_Z       0
#define M68K_SR_N       (1 << M68K_SR_N_SFT)
#define M68K_SR_X       (1 << M68K_SR_X_SFT)

#define M68K_SR_M       (1 << M68K_SR_M_SFT)
#define M68K_SR_S       (1 << M68K_SR_S_SFT)
#define M68K_SR_T       (1 << M68K_SR_T_SFT)

#define M68K_CCR_MASK   0x1F

#define M68K_SR_MASK    (M68K_SR_T | M68K_SR_S | M68K_SR_M | 0x0700 | M68K_CCR_MASK)

                                            
#define M68K_RESET_EX                   1
#define M68K_BUS_ERROR_EX               2
#define M68K_ADDRESS_ERROR_EX           3
#define M68K_ILLEGAL_INSTRUCTION_EX     4
#define M68K_ZERO_DIVIDE_EX             5
#define M68K_CHK_EX                     6
#define M68K_TRAPV_EX                   7
#define M68K_PRIVILEGE_VIOLATION_EX     8
#define M68K_TRACE_EX                   9
#define M68K_1010_EX                    10
#define M68K_1111_EX                    11
#define M68K_FORMAT_ERROR_EX            14
#define M68K_UNINITIALIZED_INTERRUPT_EX 15
#define M68K_SPURIOUS_INTERRUPT_EX      24
#define M68K_INTERRUPT_AUTOVECTOR_EX    24
#define M68K_TRAP_BASE_EX               32

#define M68K_INT_ACK_AUTOVECTOR         -1

#define M68K_RUNNING    0x01
#define M68K_HALTED     0x80
#define M68K_WAITING    0x04
#define M68K_DISABLE    0x20
#define M68K_FAULTED    0x40


                        
                         

#define DREG(X)         (m68kcontext.dreg[(X)].D)
#define DREGu32(X)      (m68kcontext.dreg[(X)].D)
#define DREGs32(X)      (m68kcontext.dreg[(X)].SD)
#define DREGu16(X)      (m68kcontext.dreg[(X)].W)
#define DREGs16(X)      (m68kcontext.dreg[(X)].SW)
#define DREGu8(X)       (m68kcontext.dreg[(X)].B)
#define DREGs8(X)       (m68kcontext.dreg[(X)].SB)

#define AREG(X)         (m68kcontext.areg[(X)].D)
#define AREGu32(X)      (m68kcontext.areg[(X)].D)
#define AREGs32(X)      (m68kcontext.areg[(X)].SD)
#define AREGu16(X)      (m68kcontext.areg[(X)].W)
#define AREGs16(X)      (m68kcontext.areg[(X)].SW)

#define USP             (m68kcontext.usp)
#define MSP             (m68kcontext.msp)
#define ISP             (m68kcontext.isp)


                      
M68K_CONTEXT m68kcontext;

#define flag_C    m68kcontext.flag_c
#define flag_V    m68kcontext.flag_v
#define flag_NotZ m68kcontext.flag_notz
#define flag_N    m68kcontext.flag_n
#define flag_X    m68kcontext.flag_x
#define flag_S    m68kcontext.flag_s
#define flag_I    m68kcontext.flag_i
#define flag_M    m68kcontext.flag_m
#define flag_T    m68kcontext.flag_t
#define PC        m68kcontext._pc
#define BasePC    m68kcontext.basepc
#define Fetch     m68kcontext.fetch


#define UPDATE_SP_000 \
{ \
        if (oldS != flag_S) { \
            if (oldS) { \
                ISP = AREG(7); \
                AREG(7) = USP; \
            } else { \
                USP = AREG(7); \
                AREG(7) = ISP; \
            } \
        } \
}


#define UPDATE_SP_020 \
{ \
        if (oldS != flag_S) { \
            if (oldS) { \
                if (oldM) \
                    MSP = AREG(7); \
                else \
                    ISP = AREG(7); \
                AREG(7) = USP; \
            } else { \
                USP = AREG(7); \
                AREG(7) = flag_M ? MSP : ISP; \
            } \
        } else if ((oldS && oldM) != flag_M) { \
            if (oldM) { \
                MSP = AREG(7); \
                AREG(7) = ISP; \
            } else { \
                ISP = AREG(7); \
                AREG(7) = MSP; \
            } \
        } \
}


#define LSL(A, C)       ((A) << (C))
#define LSR(A, C)       ((A) >> (C))

#define LSR_32(A, C)    ((C) < 32 ? (A) >> (C) : 0)
#define LSL_32(A, C)    ((C) < 32 ? (A) << (C) : 0)

#define ROL_8(A, C)     (LSL(A, C) | LSR(A, 8-(C)))
#define ROL_9(A, C)     (LSL(A, C) | LSR(A, 9-(C)))
#define ROL_16(A, C)    (LSL(A, C) | LSR(A, 16-(C)))
#define ROL_17(A, C)    (LSL(A, C) | LSR(A, 17-(C)))
#define ROL_32(A, C)    (LSL_32(A, C) | LSR_32(A, 32-(C)))
#define ROL_33(A, C)    (LSL_32(A, C) | LSR_32(A, 33-(C)))

#define ROR_8(A, C)     (LSR(A, C) | LSL(A, 8-(C)))
#define ROR_9(A, C)     (LSR(A, C) | LSL(A, 9-(C)))
#define ROR_16(A, C)    (LSR(A, C) | LSL(A, 16-(C)))
#define ROR_17(A, C)    (LSR(A, C) | LSL(A, 17-(C)))
#define ROR_32(A, C)    (LSR_32(A, C) | LSL_32(A, 32-(C)))
#define ROR_33(A, C)    (LSR_32(A, C) | LSL_32(A, 33-(C)))


#define RET(A) \
   return (A);

#define M68K_PPL (m68kcontext.sr >> 8) & 7

#define GET_PC                  \
    (hostptr)PC - BasePC;

#define SET_PC(A)               \
    BasePC = Fetch[((A) >> M68K_FETCHSFT) & M68K_FETCHMASK];    \
    PC = (u16*)(((A) & M68K_ADR_MASK) + BasePC); 

#define READ_BYTE_F(A, D)                    \
  D = Read_Byte(A)           ;

#define READ_WORD_F(A, D)                    \
  D = Read_Word(A)             ;

#define READ_LONG_F(A, D)                    \
  D = Read_Long((A));

#define READ_LONG_DEC_F(A, D)                \
  D = Read_Long((A));

#define READSX_LONG_F(A, D)                  \
  D = Read_Long((A));

#define READSX_LONG_DEC_F(A, D)              \
  D = Read_Long((A));

#define POP_32_F(D)                          \
  D = Read_Long(AREG(7));                   \
  AREG(7) += 4;

#define WRITE_LONG_F(A, D)                   \
  Write_Long((A), (D));

#define WRITE_LONG_DEC_F(A, D)               \
  Write_Long((A), (D));

#define PUSH_32_F(D)                         \
  AREG(7) -= 4;                             \
  Write_Long(AREG(7), (D));


#define GET_SWORD                            \
  (s32)(s16)(*PC)

#define GET_SLONG                            \
  (s32)(((u32)(*PC) << 16) | (*(PC + 1)             ));

#ifndef USE_ARMV7
#define FETCH_LONG(A)                    \
  (A) = PC[1] | (PC[0] << 16);              \
  PC += 2;
#else
#define FETCH_LONG(A)                    \
  asm volatile ("ldr    %[erg], [%[adr]], #4    \n\t" \
                "ror    %[erg], %[erg], #16   \n\t" \
                : [erg] "=r" (A), [adr] "+r" (PC) );
#endif

#define FETCH_BYTE(A)                        \
  (A) = (*PC++) & 0xFF;

#define FETCH_WORD(A)       \
  (A) = *PC++;

#define FETCH_SWORD(A)      \
	(A) = (s32)(s16)(*PC++);


#define READSX_WORD_F(A, D)             \
    D = (s16)Read_Word(A);
    

#define WRITE_BYTE_F(A, D)      \
    Write_Byte(A, D);

#define WRITE_WORD_F(A, D)      \
    Write_Word(A, D);

#define PUSH_16_F(D)                    \
    Write_Word(AREG(7) -= 2, D);   \

#define POP_16_F(D)                     \
    D = (u16)Read_Word(AREG(7));   \
    AREG(7) += 2;

#define GET_CCR                                     \
    (((flag_C >> (M68K_SR_C_SFT - 0)) & 1) |   \
     ((flag_V >> (M68K_SR_V_SFT - 1)) & 2) |   \
     (((!flag_NotZ) & 1) << 2) |               \
     ((flag_N >> (M68K_SR_N_SFT - 3)) & 8) |   \
     ((flag_X >> (M68K_SR_X_SFT - 4)) & 0x10))

#define GET_SR                  \
    ((flag_S << 0)  |      \
     (flag_I << 8)  |      \
     (prefs_cpu_model >= M68020 ? (flag_M << 12) : 0) |      \
     (flag_T ) | \
     GET_CCR)

#define SET_CCR(A)                              \
    flag_C = (A) << (M68K_SR_C_SFT - 0);   \
    flag_V = (A) << (M68K_SR_V_SFT - 1);   \
    flag_NotZ = ~(A) & 4;                  \
    flag_N = (A) << (M68K_SR_N_SFT - 3);   \
    flag_X = (A) << (M68K_SR_X_SFT - 4);


#define SET_SR(A)                      \
    SET_CCR(A)                         \
    flag_T = (A) & M68K_SR_T;          \
    flag_S = (A) & M68K_SR_S;          \
    flag_I = ((A) >> 8) & 7;           \
    flag_M = (prefs_cpu_model >= M68020 ? (A) & M68K_SR_M : 0);

#define CHECK_INT_TO_JUMP(CLK) m68kcontext.more_cycles_to_do=m68kcontext.io_cycle_counter; m68kcontext.io_cycle_counter=0;

#define BANKEND_TAG ((u32)-1)


                             
typedef void (*icust_handler_func)(u32 vector);

                  
                   

void *mem_handlerRB[M68K_DATABANK];
void *mem_handlerRW[M68K_DATABANK];
void *mem_handlerWB[M68K_DATABANK];
void *mem_handlerWW[M68K_DATABANK];
void *mem_data[M68K_DATABANK];


                             
typedef int (*opcode_func)(const u32 opcode, M68K_CONTEXT &m68kcontext);

static opcode_func JumpTable[0x10000] UAE4ALL_ALIGN;

static u32 initialised = 0;

                                                  
static const s32 exception_cycle_table[256] =
{
	  4,                                     
	  4,                                       
	 50,                 
	 50,                     
	 34,                           
	 38,                      
	 40,           
	 34,             
	 34,                           
	 34,             
	  4,       
	  4,       
	  4,                
	  4,                                      
	  4,                    
	 44,                               
	  4,                
	  4,                
	  4,                
	  4,                
	  4,                
	  4,                
	  4,                
	  4,                
	 44,                          
	 44,                                    
	 44,                                    
	 44,                                    
	 44,                                    
	 44,                                    
	 44,                                    
	 44,                                    
	 34,               
	 34,               
	 34,               
	 34,               
	 34,               
	 34,               
	 34,               
	 34,               
	 34,               
	 34,               
	 34,                
	 34,                
	 34,                
	 34,                
	 34,                
	 34,                
	  4,                                             
	  4,                         
	  4,                         
	  4,                    
	  4,                        
	  4,                   
	  4,                        
	  4,                                  
	  4,                               
	  4,                                   
	  4,                                        
	  4,                
	  4,                
	  4,                
	  4,                
	  4,                
	                            
	  4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,
	  4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,
	  4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,
	  4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,
	  4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,
	  4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4
};


                      
                      
                      

static void execute_exception_group_0(s32 vect, u16 inst_reg, s32 addr, u16 spec_info);

#define CHECK_BRANCH_EXCEPTION(_PC_,CYCLES) \
  if (unlikely((_PC_)&1))                             \
  {                                                   \
     u32 pr_PC = GET_PC;                              \
     execute_exception_group_0(M68K_ADDRESS_ERROR_EX, 0, pr_PC, 0x12 ); \
     RET(CYCLES)                                      \
  }


void famec_SetBank(u32 low_addr, u32 high_addr, hostptr fetch, void *rb, void *rw, void *wb, void *ww, void *data)
{
  u32 i, j;

	i = (low_addr >> M68K_DATASFT) & M68K_DATAMASK;
  j = (high_addr >> M68K_DATASFT) & M68K_DATAMASK;

	while (i <= j)
  {
    Fetch[i] = fetch;
		mem_handlerRB[i] = rb;
		mem_handlerRW[i] = rw;
		mem_handlerWB[i] = wb;
		mem_handlerWW[i] = ww;
		mem_data[i++] = data;
	}
}


                         
                                

#ifdef USE_ARMNEON

extern "C" u8 ARM_Read_Byte(u32 addr);
#define Read_Byte ARM_Read_Byte

extern "C" u16 ARM_Read_Word(u32 addr);
#define Read_Word ARM_Read_Word

extern "C" u32 ARM_Read_Long(u32 addr);
#define Read_Long ARM_Read_Long

extern "C" void ARM_Write_Byte(u32 addr, u8 data);
#define Write_Byte ARM_Write_Byte

extern "C" void ARM_Write_Word(u32 addr, u16 data);
#define Write_Word ARM_Write_Word

extern "C" void ARM_Write_Long(u32 addr, u32 data);
#define Write_Long ARM_Write_Long

#else

static __attribute__ ((noinline)) u8 Read_Byte(u32 addr)
{
	u32 i;
	u8 val;

	addr&=M68K_ADR_MASK;
	
	i=addr>>M68K_DATASFT;

	if (mem_handlerRB[i])
		val = ((u8 (*)(s32))mem_handlerRB[i])(addr);
	else {
		val = *((u8 *)(((hostptr)mem_data[i]) + (addr^1)));
	}

	return val;
}

static __attribute__ ((noinline)) u16 Read_Word(u32 addr)
{
	u32 i;
	u32 val;

	addr&=M68K_ADR_MASK;
	
	i=addr>>M68K_DATASFT;
	
                                                                        
   if (mem_handlerRW[i]) {
      val = ((u16 (*)(s32))mem_handlerRW[i])(addr);
   }
   else {
      if (unlikely(addr & 1)) {
                    
                                                                           
                                                                        
            
         u32 b;
         b = *((u32 *)(((hostptr)mem_data[i]) + (addr ^ 1)));
         val = (b << 8) | (b >> 24);
                                                          
                                    
                                
      }
      else {
         val = *((u16 *)(((hostptr)mem_data[i]) + addr));
      }
	}

	return val;
}

static INLINE u32 Read_Long(u32 addr)
{
   return (Read_Word(addr) << 16) | (Read_Word(addr + 2)             );
}

static __attribute__ ((noinline)) void Write_Byte(u32 addr, u8 data)
{
	u32 i;

	addr&=M68K_ADR_MASK;
	
	i=addr>>M68K_DATASFT;

	if (mem_handlerWB[i] != NULL)
		((void (*)(s32, s32))mem_handlerWB[i])(addr,data);
	else {
	   *((u8 *)(((hostptr)mem_data[i]) + (addr^1))) = data;
	}
}

static __attribute__ ((noinline)) void Write_Word(u32 addr, u16 data)
{
	u32 i;

	addr&=M68K_ADR_MASK;
	
	i=addr>>M68K_DATASFT;

                                                                        
   if (mem_handlerWW[i]) {
      ((void (*)(s32, s32))mem_handlerWW[i])(addr,data);
   }
   else {
      if (unlikely(addr & 1)) {
         u8 *pdata = (u8 *)((hostptr)mem_data[i] + addr);
         
                    
                                                                            
                                                        
            
         *(pdata - 1) = data >> 8;
         *(pdata + 2) = data;
      }
      else {
         *((u16 *)(((hostptr)mem_data[i]) + addr)) = data;
      }
   }
}

static INLINE void Write_Long(u32 addr, u32 data)
{
   Write_Word(addr, data >> 16);
   Write_Word(addr + 2, data             );
}


#endif


                         
                         
                         
static void m68k_jumptable000(int force_table);
static void m68k_jumptable020(int force_table);

                                                                             
                                                                             
                                                                             
                                                                             
                                                                             
void m68k_init(int force_table)
{
  if (!initialised || force_table)
  {
    if(prefs_cpu_model >= M68020)
    	m68k_jumptable020(force_table);
    else
    	m68k_jumptable000(force_table);
  }
}

                                                                                
                                   
                                        
                                                                                
                                                                                
                                                                                
                                                                                
                                                                                
                                                                                
u32 m68k_reset(void)
{
	u32 i=0;

	if (!initialised)
	{
    if(prefs_cpu_model >= M68020)
    	m68k_jumptable020(0);
    else
    	m68k_jumptable000(0);
	}

	                                                      
	if (m68kcontext.execinfo & M68K_RUNNING)
		return M68K_RUNNING;

	                     
	memset(&m68kcontext.dreg[0], 0, 16*4);
	
	                                      
	m68kcontext.interrupts[0] = 0;
	for(i=1;i<8;i++)
		m68kcontext.interrupts[i]=i+0x18;
	m68kcontext.execinfo = 0;
   USP = MSP = ISP = 0;
   m68kcontext.vbr = m68kcontext.sfc = m68kcontext.dfc = 0;
   m68kcontext.caar = m68kcontext.cacr = 0;
   flag_S = 1;
   flag_M = 0;

	                           
	m68kcontext.sr = 0x2700;
	
	                                       
	AREG(7) = Read_Long(             0);
	m68kcontext.pc = Read_Long(             4);

    return M68K_OK;
}


                                                                             
                                                                             
                                                                             
                                                                             
                                                                             
M68K_CONTEXT *m68k_get_context(void)
{
	return &m68kcontext;
}

                                                                              
                                                                              
                                                                              
                                                                              
                                                                              
u32 m68k_get_pc(void)
{
	return (m68kcontext.execinfo & M68K_RUNNING)?(hostptr)PC-BasePC:m68kcontext.pc;
}

                                                                         
                                                                         
                                                                         
                                                                         
                                                                         
                                                                         
                                                                         
s32 m68k_set_register(m68k_register reg, u32 value)
{
	switch(reg)
	{
		case M68K_REG_D0:
		case M68K_REG_D1:
		case M68K_REG_D2:
		case M68K_REG_D3:
		case M68K_REG_D4:
		case M68K_REG_D5:
		case M68K_REG_D6:
		case M68K_REG_D7:
			DREG(reg - M68K_REG_D0) = value;
			break;

		case M68K_REG_A0:
		case M68K_REG_A1:
		case M68K_REG_A2:
		case M68K_REG_A3:
		case M68K_REG_A4:
		case M68K_REG_A5:
		case M68K_REG_A6:
		case M68K_REG_A7:
			AREG(reg - M68K_REG_A0) = value;
			break;

		case M68K_REG_USP:
			USP = value;
			break;

		case M68K_REG_PC:
			if (m68kcontext.execinfo & M68K_RUNNING)
			{
				SET_PC(value & M68K_ADR_MASK);
			}
			else
			{
				m68kcontext.pc = value;
			}
			break;

		case M68K_REG_SR:
        if (m68kcontext.execinfo & M68K_RUNNING)
        {
			SET_SR(value);
		}
		else
		{
			m68kcontext.sr = value & 0xFFFF;
		}
			break;

		default:
			return M68K_INV_REG;
	}

	return M68K_OK;
}

                                                           
                                                           
                                                           
                                                           
                                                           
                                                           
s32 m68k_fetch(u32 addr)
{
	s32 val;
  hostptr Base;
  u16 *ptr;
  
  Base = Fetch[(addr >> M68K_FETCHSFT) & M68K_FETCHMASK];
  ptr = (u16*)((addr & M68K_ADR_MASK) + Base); 
  val = *ptr;

	return val;	
}


                                                        
                                                        
                                                        
                                                        
                                                        
                                                        
                                                        
void m68k_release_timeslice(void)
{
	if (m68kcontext.execinfo & M68K_RUNNING)
	{
		m68kcontext.cycles_not_done = m68kcontext.io_cycle_counter >> mainMenu_CPU_speed;
		m68kcontext.io_cycle_counter = 0;
	}
}


                          
                                          
static FAMEC_EXTRA_INLINE s32 interrupt_chk__(M68K_CONTEXT &m68kcontext)
{
	if ((m68kcontext.interrupts[0]>>1))
	{
		if (m68kcontext.interrupts[0]&0x80)
			return 7;
		else
		if (m68kcontext.interrupts[0]&0x40)
		{
			if (6 > flag_I)
				return 6;
		}
		else
		if (m68kcontext.interrupts[0]&0x20)
		{
			if (5 > flag_I)
				return 5;
		}
		else
		if (m68kcontext.interrupts[0]&0x10)
		{
			if (4 > flag_I)
				return 4;
		}
		else
		if (m68kcontext.interrupts[0]&0x08)
		{
			if (3 > flag_I)
				return 3;
		}
		else
		if (m68kcontext.interrupts[0]&0x04)
		{
			if (2 > flag_I)
				return 2;
		}
		else
		if (m68kcontext.interrupts[0]&0x02)
		{
			if (1 > flag_I)
				return 1;
		}
	}

	if (flag_T)
		return -1;

	return 0;
}

                                                                                      
void process_exception(unsigned int vect)
{
   u32 newPC;
   u32 oldPC = (hostptr)(PC) - BasePC;
   u32 oldSR = GET_SR;
   
                                                                                
   flag_T = 0;
   
   if (!flag_S)
   {
      USP = AREG(7);
      if (prefs_cpu_model >= M68020)
         AREG(7) = flag_M ? MSP : ISP;
      else
         AREG(7) = ISP;
                     
      flag_S = M68K_SR_S;
   }
   
   if (prefs_cpu_model > M68000) {
                                                                            
      if ((vect == 2) || (vect == 3)) {
         int i;
         u16 ssw = (flag_S ? 4 : 0) | (0                                           ? 2 : 1);
         ssw |= 0                                     ? 0 : 0x40;
         ssw |= 0x20;
         for (i = 0 ; i < 36; i++) {
            PUSH_16_F(0);
         }
         PUSH_32_F(0                              );
         PUSH_16_F(0);
         PUSH_16_F(0);
         PUSH_16_F(0);
         PUSH_16_F(ssw);
         PUSH_16_F(0xb000 + vect * 4);
      } else if ((vect == 5) || (vect == 6) || (vect == 7) || (vect == 9)) {
         PUSH_32_F(oldPC);
         PUSH_16_F(0x2000 + vect * 4);
      } else if (flag_M && vect >= 24 && vect < 32) {                    
         PUSH_16_F(vect * 4);
         PUSH_32_F(oldPC);
         PUSH_16_F(oldSR);
         m68kcontext.sr |= (1 << 13);
         MSP = AREG(7);
         AREG(7) = ISP;
         PUSH_16_F(0x1000 + vect * 4);
      } else {
         PUSH_16_F(vect * 4);
      }
      
      PUSH_32_F(oldPC);
      PUSH_16_F(oldSR);
      
      flag_M = 0;
   }
                                           
                                 
                                                                                       
                                                          
                                  
                      
                                                 
                                                                       
                                                                    
                                                     
                                                                       
      
   else {
                 
      PUSH_32_F(oldPC)
      PUSH_16_F(oldSR)
   }
   
   READ_LONG_F(m68kcontext.vbr + vect * 4, newPC)
   newPC &= M68K_ADR_MASK;
   
   SET_PC(newPC)
}

static FAMEC_EXTRA_INLINE void execute_exception(s32 vect)
{
	m68kcontext.io_cycle_counter -= (exception_cycle_table[vect]);
	
	                                                  
	if (m68kcontext.icust_handler[vect])
	{
		m68kcontext.sr = GET_SR;
		m68kcontext.pc = GET_PC;
		icust_handler_func salto=(icust_handler_func)m68kcontext.icust_handler[vect];
		salto(vect);
	}
	else
	{
      process_exception(vect);
	}
}


static FAMEC_EXTRA_INLINE void interrupt_attend(s32 line)
{
	                                                      
	m68kcontext.execinfo &= ~M68K_HALTED;

	                             
	m68kcontext.interrupts[0] &= ~(1 << ((u32)line));

	execute_exception(m68kcontext.interrupts[(u32)line]);

	flag_I = (u32)line;
}


static INLINE void execute_exception_group_0(s32 vect, u16 inst_reg, s32 addr, u16 spec_info)
{
	execute_exception(vect);
	if (!(m68kcontext.icust_handler[vect]))
	{
		PUSH_16_F(inst_reg);
		PUSH_32_F(addr);
		PUSH_16_F(spec_info);
	}
}


                                                              
static INLINE void finish_emulate(const s32 cycles_to_add)
{
    m68kcontext.sr = GET_SR;
    m68kcontext.pc = GET_PC;

    m68kcontext.execinfo &= ~M68K_RUNNING;

                                       
    m68kcontext.cycles_counter += cycles_to_add;
}


#define EXECUTE_EXCEPTION(EX,CYCLES)   \
{                                      \
   u32 oldPC=GET_PC;                   \
   SET_PC(oldPC-2)                     \
   execute_exception(EX);              \
   RET(CYCLES)                         \
}

static void TRAPCC_EXECUTE (u32 Opcode)
{
   u8 do_trap;
   int c = (flag_C >> M68K_SR_C_SFT) & 1;
   int z = (flag_NotZ == 0);
   int n = (flag_N >> M68K_SR_N_SFT) & 1;
   int v = (flag_V >> M68K_SR_V_SFT) & 1;
   
   switch((Opcode >> 8) & 0xF) {
      case  0: do_trap =  1; break;
      case  1: do_trap =  0; break;
      case  2: do_trap = !c && !z; break;
      case  3: do_trap =  c ||  z; break;
      case  4: do_trap = !c; break;
      case  5: do_trap =  c; break;
      case  6: do_trap = !z; break;
      case  7: do_trap =  z; break;
      case  8: do_trap = !v; break;
      case  9: do_trap =  v; break;
      case 10: do_trap = !n; break;
      case 11: do_trap =  n; break;
      case 12: do_trap =  n &&  v || !n && !v; break;
      case 13: do_trap =  n && !v || !n &&  v; break;
      case 14: do_trap =  n &&  v && !z || !n && !v && !z; break;
      case 15: do_trap =  z ||  n && !v || !n &&  v; break;
   }
   if (do_trap)
      execute_exception(M68K_TRAPV_EX);
}


                         
   
                                                                                       
                                                             
   
#define BF_MASK(MASK, OFFSET, WIDTH)      \
   MASK = 0xFFFFFFFF;                     \
   if ((OFFSET + WIDTH) < 32) {           \
      MASK <<= OFFSET;                    \
      MASK >>= 32 - WIDTH;                \
   }

#define BF_SHIFT_DOWN(DATA, OFFSET, WIDTH) \
   DATA <<= OFFSET;                       \
   DATA >>= 32 - WIDTH;
   
#define BF_SHIFT_UP(DATA, OFFSET, WIDTH)  \
   DATA <<= 32 - WIDTH;                   \
   DATA >>= OFFSET;

#define BF_EXTS(DATA, WIDTH, MASK)        \
   DATA |= (((DATA >> (WIDTH - 1)) - 1) & ~MASK) ^ ~MASK;

#define BF_SET_FLAGS(DATA, WIDTH)         \
   flag_N = (DATA << (32 - WIDTH)) >> 24;                                         \
   flag_NotZ = (DATA != 0);               \
   flag_C = 0;                            \
   flag_V = 0;

#define BF_GET_PARM(EXTRA, OFFSET, WIDTH) \
   OFFSET = EXTRA & 0x800 ? DREG((EXTRA >> 6) & 7) : (EXTRA >> 6) & 0x1F; \
                                          \
   WIDTH = (((EXTRA & 0x20 ? DREG(EXTRA & 7) : EXTRA) - 1) & 0x1F) + 1; \

#define BF_FFO(SRC, MASK, OFFSET, WIDTH, EXTRA)  \
{                                         \
   MASK = 1 << (WIDTH - 1);               \
   while (MASK) {                         \
      if (SRC & MASK)                     \
         break;                           \
      OFFSET++;                           \
      MASK >>= 1;                         \
   }                                      \
   DREGu32((EXTRA >> 12) & 7) = OFFSET;   \
}

#define BF_REG_GET(EXTRA, DATA, OFFSET, WIDTH) \
   DATA = DREG((Opcode         ) & 7);    \
   BF_GET_PARM(EXTRA, OFFSET, WIDTH)      \
   OFFSET &= 0x1F;                        \
   BF_SHIFT_DOWN(DATA, OFFSET, WIDTH)     \
   BF_SET_FLAGS(DATA, WIDTH)

#define BF_REG_GET_BFFFO(EXTRA, DATA, OFFSET, OFFSET2, WIDTH) \
   DATA = DREG((Opcode         ) & 7);    \
   BF_GET_PARM(EXTRA, OFFSET, WIDTH)      \
   OFFSET2 = OFFSET;                      \
   OFFSET &= 0x1F;                        \
   BF_SHIFT_DOWN(DATA, OFFSET, WIDTH)     \
   BF_SET_FLAGS(DATA, WIDTH)

static __inline__ void BF_MEM_GET(u32 *adr, u32 *dst, s32 *offset, u32 width, u32 *bf0, u32 *bf1)
{
                             
                         
                                                                
                                                         
      
   
                                                                                                
   if (*offset >= 0) {
      *adr += *offset >> 3;
                                                     
      *offset &= 7;
   } else {
                                                                                        
      *adr += *offset / 8 - 1;
                                                     
      *offset = 8 - (u32)(*offset & 7);
   }
   
   READ_LONG_F(*adr, *bf0);
   if ((*offset + width) > 32)
      READ_BYTE_F((*adr+4), *bf1)
   else
      *bf1 = 0;
   
   *dst = (*bf0 << *offset) | (*bf1 >> (8 - *offset));
   *dst >>= (32 - width);
}

static __inline__ void BF_MEM_PUT(u32 adr, u32 dst, u32 mask, u32 offset, u32 width, u32 bf0, u32 bf1)
{
                                                
                                
                                                                
                                                          
      
                                                                
                                 
                     
                                           
      
   u32 dst_tmp = dst;
   u32 mask_tmp = mask;
   
                                
   BF_SHIFT_UP(mask_tmp, offset, width)
   BF_SHIFT_UP(dst_tmp, offset, width)
   WRITE_LONG_F(adr, (bf0 & ~mask_tmp) | dst_tmp);
   
                            
   if ((offset + width) > 32) {
      offset = 8 - offset;
      mask_tmp = mask << offset;
      dst_tmp = dst << offset;
      WRITE_BYTE_F(adr+4, (bf1 & ~mask_tmp) | dst_tmp);
   }
}

#define CAS_EXECUTE(SHIFT, WRITE_OP)             \
{                                         \
                                              \
                                          \
   src = DREG(res & 7);                   \
                                          \
                                              \
                                              \
   dst = tmp - src;                       \
                                              \
                                          \
   flag_V = ((src ^ tmp) & (dst ^ tmp)) >> SHIFT;                                         \
   flag_C = dst;                                   \
   flag_N = dst >> SHIFT;                                        \
   flag_NotZ = (dst != 0);                \
                                          \
   if (flag_NotZ)                         \
      DREGs32(res & 7) = tmp;             \
   else {                                 \
      WRITE_OP;                           \
   }                                      \
}

#define CAS2_EXECUTE(SHIFT, WRITE_OP1, WRITE_OP2) \
{                                         \
                                              \
                                          \
                                          \
   src = DREG(res1 & 7);                  \
                                              \
                                              \
   dst = tmp1 - src;                      \
                                              \
                                          \
   flag_NotZ = (dst != 0);                \
                                          \
   if (flag_NotZ) {                       \
                                          \
      flag_V = ((src ^ tmp1) & (dst ^ tmp1)) >> SHIFT;                                          \
      flag_C = dst;                                \
      flag_N = dst >> SHIFT;                                \
      DREGs32(res1 & 7) = tmp1;           \
      DREGs32(res2 & 7) = tmp2;           \
   }                                      \
   else {                                 \
                                          \
      src = DREG(res2 & 7);               \
                                              \
                                              \
      dst = tmp2 - src;                   \
                                              \
                                          \
      flag_V = ((src ^ tmp2) & (dst ^ tmp2)) >> SHIFT;                                         \
      flag_C = dst;                                \
      flag_N = dst >> SHIFT;                                \
      flag_NotZ = (dst != 0);             \
                                          \
      if (flag_NotZ) {                    \
                                          \
         DREGs32(res1 & 7) = tmp1;        \
         DREGs32(res2 & 7) = tmp2;        \
      } else {                            \
                                          \
         WRITE_OP1;                       \
         WRITE_OP2;                       \
      }                                   \
   }                                      \
}

#define CMP2_CHK2_EXECUTE(SIZE, ROLLBACK) \
{                                         \
   READ_BYTE_F(adr, src1)                 \
   READ_BYTE_F(adr + 1, src2)             \
                                          \
   if (res & 0x8000)                      \
      dst = AREG((res >> 12) & 7);        \
   else                                   \
      dst = (s32)(SIZE)DREG(res >> 12);   \
   flag_NotZ = ((dst != src1) && (dst != src2)); \
   if (src1 > src2) {                     \
      s32 tmp;                            \
      tmp = src1;                         \
      src1 = src2;                        \
      src2 = tmp;                         \
   }                                      \
   if ((dst < src1) || (dst > src2))      \
   {                                      \
      flag_C = M68K_SR_C;                 \
      if (res & 0x0800)                   \
         execute_exception(M68K_CHK_EX);  \
   } else                                 \
      flag_C = 0;                         \
}

static void MULL(u32 src, u16 extra)
{
   if (extra & 0x800)
   {
                          
      s64 a;
      
      a = (s64)(s32)DREG((extra >> 12) & 7);
      a *= (s64)(s32)src;

      flag_V = 0;
      flag_C = 0;
      flag_NotZ = (a != 0);
      flag_N = (a < 0);
      if (extra & 0x400) {
                            
         DREG(extra & 7) = (u32)(a >> 32);
      }
      else if ((a & UVAL64 (0xffffffff80000000)) != 0
            && (a & UVAL64 (0xffffffff80000000)) != UVAL64 (0xffffffff80000000))
         flag_V = M68K_SR_V;
      DREG((extra >> 12) & 7) = (u32)a;
   }
   else
   {
                    
      u64 a;
      
      a = (u64)(u32)src * (u64)(u32)DREG((extra >> 12) & 7);
      flag_V = 0;
      flag_C = 0;
      flag_NotZ = (a != 0);
      flag_N = (((s64)a) < 0);
      if (extra & 0x400)
         DREG(extra & 7) = (u32)(a >> 32);
      else if ((a & UVAL64 (0xffffffff00000000)) != 0)
         flag_V = M68K_SR_V;
      DREG((extra >> 12) & 7) = (u32)a;
   }
}

                                               
static int DIVL(u32 src, u16 extra) 
{
                                              
   if (extra & 0x800)
   {
                          
      s64 a = (s64)(s32)DREG((extra >> 12) & 7);
      s64 quot, rem;
      
      if (extra & 0x400) {
         a &= 0xffffffffu;
         a |= (s64)DREG(extra & 7) << 32;
      }
      rem = a % (s64)(s32)src;
      quot = a / (s64)(s32)src;
      if ((quot & UVAL64 (0xffffffff80000000)) != 0
            && (quot & UVAL64 (0xffffffff80000000)) != UVAL64 (0xffffffff80000000))
      {
         flag_V = M68K_SR_V;
         flag_N = M68K_SR_N;
         flag_C = 0;
      } else {
         if (((s32)rem < 0) != ((s64)a < 0)) rem = -rem;
         flag_V = 0;
         flag_C = 0;
         flag_NotZ = ((s32)quot != 0);
         flag_N = (((s32)quot) < 0);
         DREG(extra & 7) = (u32)rem;
         DREG((extra >> 12) & 7) = (u32)quot;
      }
      return 12;
   }
   else
   {
                    
      u64 a = (u64)(u32)DREG((extra >> 12) & 7);
      u64 quot, rem;
      
      if (extra & 0x400) {
         a &= 0xffffffffu;
         a |= (u64)DREG(extra & 7) << 32;
      }
      rem = a % (u64)src;
      quot = a / (u64)src;
      if (quot > 0xffffffffu) {
         flag_V = M68K_SR_V;
         flag_N = M68K_SR_N;
         flag_C = 0;
      } else {
         flag_V = 0;
         flag_C = 0;
         flag_NotZ = ((s32)quot != 0);
         flag_N = (((s32)quot) < 0);
         DREG(extra & 7) = (u32)rem;
         DREG((extra >> 12) & 7) = (u32)quot;
      }
      return 0;
   }
}

static __inline__ void MOVEC2(int XN, int RC)
{
    u32 *reg;
    int xreg = RC & 0x0007;
    
    if (RC & 0x0008)
        reg = &(AREGu32(xreg));
    else
        reg = &(DREGu32(xreg));
    
    switch(XN) {
        case 0: *reg = m68kcontext.sfc; break;
        case 1: *reg = m68kcontext.dfc; break;
        case 2:
        {
            *reg = m68kcontext.cacr & 0x00000003;                 
            break;
        }
        
        case 0x800: *reg = USP; break;
        case 0x801: *reg = m68kcontext.vbr; break;
        case 0x802: *reg = m68kcontext.caar; break;
        case 0x803: *reg = (flag_M == 1) ? AREGu32(7) : MSP; break;
        case 0x804: *reg = (flag_M == 0) ? AREGu32(7) : ISP; break;
    }
}

static __inline__ void MOVE2C(int XN, int RC)
{
    u32 *reg;
    int xreg = RC & 0x0007;
    
    if (RC & 0x0008)
        reg = &(AREGu32(xreg));
    else
        reg = &(DREGu32(xreg));
    
    switch(XN) {
        case 0: m68kcontext.sfc = *reg & 7; break;
        case 1: m68kcontext.dfc = *reg & 7; break;
        case 2:
        {
            m68kcontext.cacr = *reg & 0x0000000F;                 
            break;
        }
        
        case 0x800: USP = *reg; break;
        case 0x801: m68kcontext.vbr = *reg; break;
        case 0x802: m68kcontext.caar = *reg & 0xFC; break;
        case 0x803: MSP = *reg; if (flag_M == 1) AREGu32(7) = MSP; break;
        case 0x804: ISP = *reg; if (flag_M == 0) AREGu32(7) = ISP; break;
    }
}

static __attribute__ ((noinline)) void DECODE_EXT_WORD_020 (u32 *adr)
{
   u16 ext;
   s32 index = 0;
   
   FETCH_WORD(ext);
   
   if (ext & 0x0100) {
                                      
      s32 disp = 0, outer = 0;
      
                                    
      if (ext & 0x80)
         *adr = 0;
      
                             
      if ((ext & 0x0030) == 0x20) {
         FETCH_SWORD(disp);
      }
      else if ((ext & 0x0030) == 0x30) {
         FETCH_LONG(disp);
      }
      *adr += disp;
      
                              
      if ((ext & 0x0040) == 0) {
                                   
         if (ext & 0x8000) {
            if (ext & 0x0800)
               index = AREGs32((ext >> 12) & 7);
            else
               index = AREGs16((ext >> 12) & 7);
         } else {
            if (ext & 0x0800)
               index = DREGs32(ext >> 12);
            else
               index = DREGs16(ext >> 12);
         }
                             
         index <<= (ext >> 9) & 3;
      }
      
                      
      if ((ext & 0x0004) == 0)
         *adr += index;
                           
      if (ext & 0x0003) {
         u32 res;
         READ_LONG_F(*adr, res);
         *adr = res;
      }
                       
      if (ext & 0x0004)
         *adr += index;
      
                              
      if ((ext & 0x0003) == 0x0002) {
         FETCH_SWORD(outer);
      }
      else if ((ext & 0x0003) == 0x0003) {
         FETCH_LONG(outer);
      }
      
      *adr += outer;
   } else {
                                       
      
                 
      if (ext & 0x8000) {
         if (ext & 0x0800)
            index = AREGs32((ext >> 12) & 7);
         else
            index = AREGs16((ext >> 12) & 7);
      } else {
         if (ext & 0x0800)
            index = DREGs32((ext >> 12));
         else
            index = DREGs16((ext >> 12));
      }
                                         
      index <<= (ext >> 9) & 3;
      
      *adr += (s8)(ext) + index;
   }
}


static __inline__ void DECODE_EXT_WORD_000 (u32 *adr)
{
   u16 ext;
   s32 index = 0;
   
   FETCH_WORD(ext);
   
               
    if (ext & 0x8000) {
       if (ext & 0x0800)
          index = AREGs32((ext >> 12) & 7);
       else
          index = AREGs16((ext >> 12) & 7);
    } else {
       if (ext & 0x0800)
          index = DREGs32((ext >> 12));
       else
          index = DREGs16((ext >> 12));
    }
    *adr += (s8)(ext) + index;
}


static FAMEC_EXTRA_INLINE u8 bitset_count(u32 data)
{
    unsigned int const MASK1  = 0x55555555;
    unsigned int const MASK2  = 0x33333333;
    unsigned int const MASK4  = 0x0f0f0f0f;
    unsigned int const MASK6 = 0x0000003f;

    unsigned int const w = (data & MASK1) + ((data >> 1) & MASK1);
    unsigned int const x = (w & MASK2) + ((w >> 2) & MASK2);
    unsigned int const y = ((x + (x >> 4)) & MASK4);
    unsigned int const z = (y + (y >> 8));
    unsigned int const c = (z + (z >> 16)) & MASK6;

    return c;
}


  
     
                  
  
static u32 getDivu68kCycles(u32 dividend, u16 divisor)
{
    u32 mcycles;
    u32 hdivisor;
    int i;

    if ( (u16) divisor == 0)
        return 0;

                  
    if ( (dividend >> 16) >= divisor)
        return 10;

    mcycles = 38;
    hdivisor = ((u32) divisor) << 16;

    for ( i = 0; i < 15; i++)
    {
        u32 temp;
        temp = dividend;

        dividend <<= 1;

                                 
        if ( (int) temp < 0)
        {
            dividend -= hdivisor;
        }

        else
        {
            mcycles += 2;
            if ( dividend >= hdivisor)
            {
                dividend -= hdivisor;
                mcycles--;
            }
        }
    }

    return mcycles * 2;
}

  
     
                
  
static u32 getDivs68kCycles(s32 dividend, s16 divisor)
{
    u32 mcycles;
    u32 aquot;
    int i;

    if ( (s16) divisor == 0)
        return 0;

    mcycles = 6;

    if ( dividend < 0)
        mcycles++;

                                      
    if ( ((u32) abs( dividend) >> 16) >= (u16) abs( divisor))
    {
        return (mcycles + 2) * 2;
    }

                           
    aquot = (u32) abs( dividend) / (u16) abs( divisor);

    mcycles += 55;

    if ( divisor >= 0)
    {
        if ( dividend >= 0)
            mcycles--;
        else
            mcycles++;
    }

                                                 

    for ( i = 0; i < 15; i++)
    {
        if ( (s16) aquot >= 0)
            mcycles++;
        aquot <<= 1;
    }

    return mcycles * 2;
}


#define OPCODES_M68000
#undef OPCODES_M68020

#define DECODE_EXT_WORD DECODE_EXT_WORD_000

#define OPCODE(N_OP) static int OP_000_##N_OP(const u32 Opcode, M68K_CONTEXT &m68kcontext)
#define CAST_OP(N_OP) (opcode_func)&OP_000_##N_OP
#define JUMPTABLE m68k_jumptable000
#include "famec_opcodes.h"
#include "famec_jumptable.h"
#undef OPCODE
#undef CAST_OP
#undef JUMPTABLE
#undef DECODE_EXT_WORD


#define OPCODES_M68020
#undef OPCODES_M68000

#define DECODE_EXT_WORD DECODE_EXT_WORD_020

#define OPCODE(N_OP) static int OP_020_##N_OP(const u32 Opcode, M68K_CONTEXT &m68kcontext)
#define CAST_OP(N_OP) (opcode_func)&OP_020_##N_OP
#define JUMPTABLE m68k_jumptable020
#include "famec_opcodes.h"
#include "famec_jumptable.h"
#undef OPCODE
#undef CAST_OP
#undef JUMPTABLE
#undef DECODE_EXT_WORD


                     
                      

                                                                             
                                                                             
                                                                             
                                                                             
void m68k_emulate(s32 cycles)
{
  M68K_CONTEXT *pm68kcontext = &m68kcontext;
  
#if 0
	                                                                      
	if (m68kcontext.execinfo & M68K_FAULTED) return;
#endif
	
	                                         
	pm68kcontext->execinfo |= M68K_RUNNING;

	           
	SET_SR(pm68kcontext->sr)

	            
	flag_I = M68K_PPL;

	           
	SET_PC(pm68kcontext->pc)

	                                             
	pm68kcontext->io_cycle_counter = cycles << mainMenu_CPU_speed;
  pm68kcontext->more_cycles_to_do = 0;
  pm68kcontext->cycles_not_done = 0;
  
                                     
  s32 line = interrupt_chk__(*pm68kcontext);

	if (pm68kcontext->execinfo & M68K_HALTED && line <= 0)
	{
                                            
    finish_emulate(cycles);
    return;
	} 

  if(line > 0)
  {
                                             
                                                             
                                                        
                                                                              
                                                                                                          
                                                                                               
      
                                   
       
                               
       
          
       
      pm68kcontext->more_cycles_to_do = pm68kcontext->io_cycle_counter;
      pm68kcontext->io_cycle_counter = 0;
       
  }
    
  do
  {
    do
    {
      u32 Opcode;
      FETCH_WORD(Opcode);
      if(flag_T)                                                                              
      {
        int op_cycles = JumpTable[Opcode](Opcode, *pm68kcontext);
        pm68kcontext->io_cycle_counter = pm68kcontext->io_cycle_counter - op_cycles;
        execute_exception(M68K_TRACE_EX);
      }
      else
      {
        int op_cycles = JumpTable[Opcode](Opcode, *pm68kcontext);
        pm68kcontext->io_cycle_counter = pm68kcontext->io_cycle_counter - op_cycles;
      }
    } while (pm68kcontext->io_cycle_counter > 0);
    
    if(pm68kcontext->more_cycles_to_do > 0)
    {
      pm68kcontext->io_cycle_counter += pm68kcontext->more_cycles_to_do;
      pm68kcontext->more_cycles_to_do = 0;
      line = interrupt_chk__(*pm68kcontext);
      if (line > 0)
      {
                                                        
        interrupt_attend(line);
      }
    }
  } while (pm68kcontext->io_cycle_counter > 0);  

  finish_emulate(cycles - pm68kcontext->cycles_not_done - (pm68kcontext->io_cycle_counter >> mainMenu_CPU_speed));
}
