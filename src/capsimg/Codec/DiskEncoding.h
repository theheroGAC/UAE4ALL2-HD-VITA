#ifndef DISKENCODING_H
#define DISKENCODING_H

              
enum {
	gcridNone,
	gcridCBM,
	gcridBigFive
};

               
enum {
	vmaxidNone,
	vmaxidNormal,
	vmaxidOld
};



                          
class CDiskEncoding
{
public:
	CDiskEncoding();
	virtual ~CDiskEncoding();
	static void InitFM();
	static void InitMFM(uint32_t mfmsize);
	static void InitGCRCBM(uint32_t *gcrtable, int gcrid);
	static void InitGCRCBM_S(uint32_t *gcrtable, int gcrid);
	static void InitGCRAppleH();
	static void InitGCRApple5(uint32_t *gcrtable);
	static void InitGCRApple6(uint32_t *gcrtable);
	static void InitGCRVorpal(uint32_t *gcrtable);
	static void InitGCRVorpal2(uint32_t *gcrtable);
	static void InitGCRVMax(uint32_t *gcrtable, int vmaxid);
	static void InitGCR4Bit(uint32_t *gcrtable);
	static int FindViolation(uint8_t *buffer, int bitpos, int bitcnt, int max0, int max1, int mode);

protected:
	void Clear();

public:
	static uint32_t fminit;                        
	static uint32_t *fmcode;                 
	static uint32_t *fmdecode;                   
	static uint32_t mfminit;                         
	static uint32_t mfmcodebit;                                                    
	static uint32_t *mfmcode;                  
	static uint32_t *mfmdecode;                    
	static int gcrinit;                         
	static uint32_t *gcrcode;                  
	static uint32_t *gcrdecode;                    
	static int gcrinit_s;                         
	static uint32_t *gcrcode_s;                  
	static uint32_t *gcrdecode_s;                    
	static int gcrahinit;                                      
	static uint32_t *gcrahcode;                               
	static uint32_t *gcrahdecode;                                 
	static int gcra5init;                                     
	static uint32_t *gcra5code;                              
	static uint32_t *gcra5decode;                                
	static int gcra6init;                                     
	static uint32_t *gcra6code;                              
	static uint32_t *gcra6decode;                                
	static int gcrvorpalinit;                                      
	static uint32_t *gcrvorpalcode;                               
	static uint32_t *gcrvorpaldecode;                                 
	static int gcrvorpal2init;                                         
	static uint32_t *gcrvorpal2code;                                  
	static uint32_t *gcrvorpal2decode;                                    
	static int gcrvmaxinit;                                     
	static uint32_t *gcrvmaxcode;                              
	static uint32_t *gcrvmaxdecode;                                
	static int gcr4bitinit;                               
	static uint32_t *gcr4bitcode;                        
	static uint32_t *gcr4bitdecode;                          

	static uint32_t gcr_cbm[];                 
	static uint32_t gcr_bigfive[];                          
	static uint32_t gcr_apple5[];                         
	static uint32_t gcr_apple6[];                         
	static uint32_t gcr_vorpal[];                              
	static uint32_t gcr_vorpal2[];                                 
	static uint32_t gcr_vmax[];                                    
	static uint32_t gcr_vmaxold[];                                         
	static uint32_t gcr_teque[];                             
	static uint32_t gcr_ozisoft[];                               
};

typedef CDiskEncoding *PCDISKENCODING;

#endif
