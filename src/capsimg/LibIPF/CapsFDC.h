#ifndef CAPSFDC_H
#define CAPSFDC_H

                 
#define CAPSDRIVE_35DD_RPM 300
#define CAPSDRIVE_35DD_HST 83

                          
                                                                
                               
                   
                             
#define CAPSDRIVE_DA_IN DF_0
#define CAPSDRIVE_DA_WP DF_1
#define CAPSDRIVE_DA_MO DF_2
#define CAPSDRIVE_DA_SS DF_3

                                                                      
#define CAPSDRIVE_DA_IPMASK (CAPSDRIVE_DA_IN|CAPSDRIVE_DA_MO)

                   
                    
                      
                                
                      
                          
                              
                       
#define CAPSFDC_LO_DRQ    DF_0
#define CAPSFDC_LO_INTRQ  DF_1
#define CAPSFDC_LO_INTFRC DF_2
#define CAPSFDC_LO_MO     DF_3
#define CAPSFDC_LO_DIRC   DF_4
#define CAPSFDC_LO_INTIP  DF_5
#define CAPSFDC_LO_DRQSET DF_6

                   
          
                     
                      
               
                      
                         
                   
              
#define CAPSFDC_SR_BUSY   DF_0
#define CAPSFDC_SR_IP_DRQ DF_1
#define CAPSFDC_SR_TR0_LD DF_2
#define CAPSFDC_SR_CRCERR DF_3
#define CAPSFDC_SR_RNF    DF_4
#define CAPSFDC_SR_SU_RT  DF_5
#define CAPSFDC_SR_WP     DF_6
#define CAPSFDC_SR_MO     DF_7

                                    
#define CAPSFDC_SR_NCCLR (CAPSFDC_SR_SU_RT|CAPSFDC_SR_RNF|CAPSFDC_SR_CRCERR|CAPSFDC_SR_BUSY)

                                  
#define CAPSFDC_SR_NCSET CAPSFDC_SR_BUSY

                                   
                                                  
                                              
#define CAPSFDC_SM_TYPE1 0x00
#define CAPSFDC_SM_TYPE2R (CAPSFDC_SR_IP_DRQ|CAPSFDC_SR_TR0_LD|CAPSFDC_SR_SU_RT|CAPSFDC_SR_WP)
#define CAPSFDC_SM_TYPE2W (CAPSFDC_SR_IP_DRQ|CAPSFDC_SR_TR0_LD|CAPSFDC_SR_SU_RT)

                    
                      
                                                                                  
#define CAPSFDC_ER_COMEND DF_0
#define CAPSFDC_ER_REQEND DF_1

                
                          
                        
                                              
                                                         
                                                
                                      
                                           
                                           
                                                                  
                                                                                       
                            
#define CAPSFDC_AI_AMDETENABLE DF_0
#define CAPSFDC_AI_CRCENABLE   DF_1
#define CAPSFDC_AI_CRCACTIVE   DF_2
#define CAPSFDC_AI_AMACTIVE    DF_3
#define CAPSFDC_AI_MA1ACTIVE   DF_4
#define CAPSFDC_AI_AMFOUND     DF_5
#define CAPSFDC_AI_MARKA1      DF_6
#define CAPSFDC_AI_MARKC2      DF_7
#define CAPSFDC_AI_DSRREADY    DF_8
#define CAPSFDC_AI_DSRAM       DF_9
#define CAPSFDC_AI_DSRMA1      DF_10

#pragma pack(push, 1)

              
struct CapsDrive {
	UDWORD type;                       
	UDWORD rpm;                   
	SDWORD maxtrack;                                           
	SDWORD track;                    
	SDWORD buftrack;                     
	SDWORD side;                                        
	SDWORD bufside;                     
	SDWORD newside;                                     
	UDWORD diskattr;                    
	UDWORD idistance;                                       
	UDWORD clockrev;                                
	SDWORD clockip;                                       
	SDWORD ipcnt;                                                     
	UDWORD ttype;                  
	PUBYTE trackbuf;                        
	PUDWORD timebuf;                  
	UDWORD tracklen;                               
	SDWORD overlap;                      
	SDWORD trackbits;                   
	SDWORD ovlmin;                                 
	SDWORD ovlmax;                                
	SDWORD ovlcnt;                        
	SDWORD ovlact;                           
	SDWORD nact;                           
	UDWORD nseed;                            
	PVOID userptr;                                                   
	UDWORD userdata;                                              
};

typedef struct CapsDrive *PCAPSDRIVE;

typedef struct CapsFdc *PCAPSFDC;
typedef void (__cdecl *CAPSFDCHOOK)(PCAPSFDC pfdc, UDWORD state);

            
struct CapsFdc {
	UDWORD type;                          
	UDWORD model;                   
	UDWORD endrequest;                                 
	UDWORD clockact;                              
	UDWORD clockreq;                                          
	UDWORD clockfrq;                       
	UDWORD addressmask;                        
	UDWORD dataline;                
	UDWORD datamask;                        
	UDWORD lineout;                     
	UDWORD runmode;                 
	UDWORD runstate;                                    
	UDWORD r_st0;                           
	UDWORD r_st1;                           
	UDWORD r_stm;                                                   
	UDWORD r_command;                       
	UDWORD r_track;                       
	UDWORD r_sector;                       
	UDWORD r_data;                       
	UDWORD seclenmask;                        
	UDWORD seclen;                       
	UDWORD crc;                       
	UDWORD crccnt;                         
	UDWORD amdecode;                                   
	UDWORD aminfo;                 
	UDWORD amisigmask;                                 
	SDWORD amdatadelay;                        
	SDWORD amdataskip;                        
	SDWORD ammarkdist;                                                                  
	SDWORD ammarktype;                       
	UDWORD dsr;                                
	SDWORD dsrcnt;                         
	SDWORD datalock;                                             
	UDWORD datamode;                        
	UDWORD datacycle;                                          
	UDWORD dataphase;                        
	UDWORD datapcnt;                          
	SDWORD indexcount;                         
	SDWORD indexlimit;                             
	SDWORD readlimit;                       
	SDWORD verifylimit;                       
	SDWORD spinupcnt;                                 
	SDWORD spinuplimit;                  
	SDWORD idlecnt;                         
	SDWORD idlelimit;                 
	UDWORD clockcnt;                     
	UDWORD steptime[4];                      
	UDWORD clockstep[4];                                   
	UDWORD hstime;                                
	UDWORD clockhs;                                       
	UDWORD iptime;                             
	UDWORD updatetime;                                              
	UDWORD clockupdate;                                  
	SDWORD drivecnt;                                              
	SDWORD drivemax;                                                     
	SDWORD drivenew;                                                    
	SDWORD drivesel;                                                 
	SDWORD driveact;                                             
	PCAPSDRIVE driveprc;                           
	PCAPSDRIVE drive;                       
	CAPSFDCHOOK cbirq;                              
	CAPSFDCHOOK cbdrq;                              
	CAPSFDCHOOK cbtrk;                           
	PVOID userptr;                                                      
	UDWORD userdata;                                                 
};

#pragma pack(pop)

                
enum {
	cfdciNA=0,                 
	cfdciSize_Fdc,                                     
	cfdciSize_Drive,                                     
	cfdciR_Command,                     
	cfdciR_ST,                         
	cfdciR_Track,                     
	cfdciR_Sector,                     
	cfdciR_Data,                     
};

             
enum {
	cfdcmNA=0,                       
	cfdcmWD1772           
};

            
enum {
	cfdcrmNop=0,                 
	cfdcrmIdle,                    
	cfdcrmType1,                
	cfdcrmType2R,                    
	cfdcrmType2W,                     
	cfdcrmType3R,                    
	cfdcrmType3W,                     
	cfdcrmType3A,                       
	cfdcrmType4                 
};

             
enum {
	cfdcdmNoline=0,                 
	cfdcdmNoise,                  
	cfdcdmData,                  
	cfdcdmDMap                              
};

#endif
