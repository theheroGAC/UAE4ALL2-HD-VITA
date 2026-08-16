#ifndef C2COMM_H
#define C2COMM_H

                                                                                                                
                                                                                                    
                  
                                                             
                       
                                   
                            
                                 

#define C2_REQ_TYPE 0x43                
#define C2_REQ_SET  0x00                                                  
#define C2_REQ_GET  0x80                     
#define C2_REQ_MASK 0x7f                          

          
#define C2_REQ_TYPE_IN  (C2_REQ_TYPE|0x80)
#define C2_REQ_TYPE_OUT (C2_REQ_TYPE)

                 
enum {
	c2Opt_S_Status=0,
	c2Opt_S_Info,
	c2Opt_S_Result,
	c2Opt_S_Data,
	c2Opt_S_Index,
	c2Opt_C_Reset,
	c2Opt_C_Device,
	c2Opt_C_Motor,
	c2Opt_C_Density,
	c2Opt_C_Side,
	c2Opt_C_Track,
	c2Opt_C_Stream,
	c2Opt_V_Min_Track,
	c2Opt_V_Max_Track,
	c2Opt_T_Set_Line,
	c2Opt_T_Density_Select,
	c2Opt_T_Drive_Select, 
	c2Opt_T_Side_Select,
	c2Opt_T_Direction_Select,
	c2Opt_T_Spin_Up,
	c2Opt_T_Step_After_Motor,
	c2Opt_T_Step_Signal,
	c2Opt_T_Step,
	c2Opt_T_Track0_Signal,
	c2Opt_T_Direction_Change,
	c2Opt_T_Head_Settling,
	c2Opt_T_Write_Gate_Off,
	c2Opt_T_Write_Gate_On,
	c2Opt_T_Bypass_Off,
	c2Opt_T_Bypass_On,
	c2Opt_Last
};

                                                                                   
                                       
                                  
enum {
	c2SEROk=0,              
	c2SERBuffer,                                                                   
	c2SERIndexTimeout,                                                  
	c2SERTransferTimeout,                                    
	c2SERProcessTimeout,                                     
	c2SERStop,                                                     
	c2SERReset,                                       
	c2SERConnection,                                                 
	c2SERReceive,                                             
	c2SERWriteBusy,                        
	c2SERInfoSign,                          
	c2SERInfoVersion,                        
	c2SERInfoFRPW,                                          
	c2SERInfoProcessTimeout,                                
	c2SERInfoSetupSize,                                 
	c2SERInfoWriteSize,                                
	c2SERInfoStreamSize,                            
	c2SERSetupMissingEnd,                                 
	c2SERSetupIncomplete,                               
	c2SERSetupInvalid,                            
	c2SERSetupIQFull,                                                         
	c2SERWriteSize,                                                                                 
	c2SERWriteState,                                                   
	c2SERWriteAbort,                                                                                  
	c2SERWriteProtect,                           
	c2SERWriteError                       
};

                     
enum {
	c2InfoInvalid=0,
	c2InfoFirmware,
	c2InfoHardware,
	c2InfoLast
};

                      
enum {
	c2StreamStop=0,
	c2StreamRead,
	c2StreamWrite
};

                
enum {
	c2StatusReady=0,
	c2StatusBusy,
	c2StatusCommand
};



                                                                    
                                    
                              
                                       
                                                                                                              
                                                                                                                
                                                                                                            
                                                                                

                                                       
#define C2_WSSIGN0 'K'
#define C2_WSSIGN1 'F'
#define C2_WSSIGN2 'W'
#define C2_WSSIGNR 1

                                              
enum {
	c2wInvalid=0,                                       
	c2wSetupEnd,                                      
	c2wTableidx,                                                                                       
	c2wTime2,                                                                                     
	c2wWGOn,                                                                                               
	c2wIQAdd,                                                                                     
	c2wIQActAll,                                   
	c2wIQWGOff,                               
	c2wIQWGOn,                              
	c2wIQResume2,                                                                       
	c2wIQEnd,                                       
	c2wIQWSuspend,                                                                       
	c2wEscape=0                                                         
};

                                                                          
                                                                               
                                                                                   
                                                                                                        
                                                                                                  
                                                                                            
                                                
                                          
  
                                              
                                                                                                      
                     
                                                       
                   
                                                                                                  
                                                                     

                                                                                       
                                                
                                                                                     
                                                                                 
                                                                                                     
                                                                                     
                                               
#define C2_WSC_WRAP    0x01
#define C2_WSC_WGSET   0x02
#define C2_WSC_WGVAL   0x04
#define C2_WSC_WGVALB  2
#define C2_WSC_STATE   0x08
#define C2_WSC_SUSPEND 0x10
#define C2_WSC_END     0x20
#define C2_WSC_IQACT   0x40

                                                                                                             
                                                                               
#define C2_WSC_EXTCTRLMASK (C2_WSC_IQACT | C2_WSC_WGSET | C2_WSC_STATE)
#define C2_WSC_EXTCTRLDEF  (C2_WSC_WGSET | C2_WSC_STATE)

                                                                                                        
                                                                                         
                                                                 
                                                                    
                                                                                                                 
                                     



                                   
enum {
	c2eValue=8,
	c2eNop1=c2eValue,
	c2eNop2,
	c2eNop3,
	c2eOverflow16,
	c2eValue16,
	c2eOOB,
	c2eSample
};

                                                                                                         
                                                                                                               
                                                                                                              
                             
                                                                                                                  
                                                                                                              
                                                              
                                                                                                                   
                                                                                                                  
                                                           
                                              
                         
                              
                          
                                           
                                                      
                                                                                                           
                                                  


                              
enum {
	c2otInvalid=0,
	c2otStreamRead,
	c2otIndex,
	c2otStreamEnd,
	c2otInfo,
	c2otEnd=c2eOOB
};

#if !defined(__GNUC__)
#pragma pack(push, 1)
#define __attribute__(x)
#endif

                                 
typedef struct {
	uint8_t sign[4];              
	uint32_t frpw;                                            
	uint32_t processtimeout;                                                               
	uint32_t setupsize;                                
	uint32_t writesize;                                           
	uint32_t streamsize;                                                                            
} __attribute__ ((packed)) __attribute__((aligned)) C2WSInfo;

             
typedef struct {
	uint8_t sign;           
	uint8_t type;                                     
	uint16_t size;                          
} __attribute__ ((packed)) __attribute__((aligned)) C2OOBHdr;

                  
typedef struct {
	uint32_t streampos;                                 
	uint32_t trtime;                                             
} __attribute__ ((packed)) __attribute__((aligned)) C2OOBStreamRead;

                 
typedef struct {
	uint32_t streampos;                                               
	uint32_t timer;                                       
	uint32_t systime;                                           
} __attribute__ ((packed)) __attribute__((aligned)) C2OOBDiskIndex;

                 
typedef struct {
	uint32_t streampos;                          
	uint32_t result;                 
} __attribute__ ((packed)) __attribute__((aligned)) C2OOBStreamEnd;

                                           
typedef union {
	C2OOBStreamRead read;
	C2OOBDiskIndex index;
	C2OOBStreamEnd end;
} __attribute__ ((packed)) __attribute__((aligned)) C2OOBData;

                       
typedef struct {
	C2OOBHdr header;
	C2OOBData data;
} __attribute__ ((packed)) __attribute__((aligned)) C2OOBMessage;

#if !defined(__GNUC__)
#pragma pack(pop)
#endif

#endif
