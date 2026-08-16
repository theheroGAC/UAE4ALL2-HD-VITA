#include "stdafx.h"



            
uint32_t CDiskEncoding::fminit=0;
uint32_t *CDiskEncoding::fmcode=NULL;
uint32_t *CDiskEncoding::fmdecode=NULL;

             
uint32_t CDiskEncoding::mfminit=0;
uint32_t CDiskEncoding::mfmcodebit=0;
uint32_t *CDiskEncoding::mfmcode=NULL;
uint32_t *CDiskEncoding::mfmdecode=NULL;

                                   
int CDiskEncoding::gcrinit=gcridNone;
uint32_t *CDiskEncoding::gcrcode=NULL;
uint32_t *CDiskEncoding::gcrdecode=NULL;
int CDiskEncoding::gcrinit_s=gcridNone;
uint32_t *CDiskEncoding::gcrcode_s=NULL;
uint32_t *CDiskEncoding::gcrdecode_s=NULL;

                          
int CDiskEncoding::gcrahinit=0;
uint32_t *CDiskEncoding::gcrahcode=NULL;
uint32_t *CDiskEncoding::gcrahdecode=NULL;

                         
int CDiskEncoding::gcra5init=0;
uint32_t *CDiskEncoding::gcra5code=NULL;
uint32_t *CDiskEncoding::gcra5decode=NULL;

                         
int CDiskEncoding::gcra6init=0;
uint32_t *CDiskEncoding::gcra6code=NULL;
uint32_t *CDiskEncoding::gcra6decode=NULL;

                              
int CDiskEncoding::gcrvorpalinit=0;
uint32_t *CDiskEncoding::gcrvorpalcode=NULL;
uint32_t *CDiskEncoding::gcrvorpaldecode=NULL;

                                 
int CDiskEncoding::gcrvorpal2init=0;
uint32_t *CDiskEncoding::gcrvorpal2code=NULL;
uint32_t *CDiskEncoding::gcrvorpal2decode=NULL;

                            
int CDiskEncoding::gcrvmaxinit=vmaxidNone;
uint32_t *CDiskEncoding::gcrvmaxcode=NULL;
uint32_t *CDiskEncoding::gcrvmaxdecode=NULL;

                             
int CDiskEncoding::gcr4bitinit=0;
uint32_t *CDiskEncoding::gcr4bitcode=NULL;
uint32_t *CDiskEncoding::gcr4bitdecode=NULL;

                            
uint32_t CDiskEncoding::gcr_cbm[]= {
	0x0a, 0x0b, 0x12, 0x13, 0x0e, 0x0f, 0x16, 0x17,
	0x09, 0x19, 0x1a, 0x1b, 0x0d, 0x1d, 0x1e, 0x15
};

                                       
uint32_t CDiskEncoding::gcr_bigfive[]= {
	0x09, 0x0a, 0x0b, 0x0d, 0x0e, 0x0f, 0x12, 0x13,
	0x15, 0x16, 0x17, 0x19, 0x1a, 0x1b, 0x1d, 0x1e
};

                                    
uint32_t CDiskEncoding::gcr_apple5[]= {
	0xab, 0xad, 0xae, 0xaf, 0xb5, 0xb6, 0xb7, 0xba,
	0xbb, 0xbd, 0xbe, 0xbf, 0xd6, 0xd7, 0xda, 0xdb,
	0xdd, 0xde, 0xdf, 0xea, 0xeb, 0xed, 0xee, 0xef,
	0xf5, 0xf6, 0xf7, 0xfa, 0xfb, 0xfd, 0xfe, 0xff
};

                                    
uint32_t CDiskEncoding::gcr_apple6[]= {
	0x96, 0x97, 0x9a, 0x9b, 0x9d, 0x9e, 0x9f, 0xa6,
	0xa7, 0xab, 0xac, 0xad, 0xae, 0xaf, 0xb2, 0xb3,
	0xb4, 0xb5, 0xb6, 0xb7, 0xb9, 0xba, 0xbb, 0xbc,
	0xbd, 0xbe, 0xbf, 0xcb, 0xcd, 0xce, 0xcf, 0xd3,
	0xd6, 0xd7, 0xd9, 0xda, 0xdb, 0xdc, 0xdd, 0xde,
	0xdf, 0xe5, 0xe6, 0xe7, 0xe9, 0xea, 0xeb, 0xec,
	0xed, 0xee, 0xef, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6,
	0xf7, 0xf9, 0xfa, 0xfb, 0xfc, 0xfd, 0xfe, 0xff
};

                                     
uint32_t CDiskEncoding::gcr_vorpal[]= {
	0x49, 0x56, 0x4b, 0x5a, 0x99, 0xaa, 0x9b, 0xad,
	0x4e, 0x5d, 0x53, 0x65, 0x9e, 0xb2, 0xa6, 0xb5,
	0x69, 0x76, 0x6b, 0x7a, 0xb9, 0xce, 0xbb, 0xd3,
	0x6e, 0x92, 0x73, 0x95, 0xc9, 0xd6, 0xcb, 0xda,
	0x4a, 0x59, 0x4d, 0x5b, 0x9a, 0xab, 0x9d, 0xae,
	0x52, 0x5e, 0x55, 0x66, 0xa5, 0xb3, 0xa9, 0xb6,
	0x6a, 0x79, 0x6d, 0x7b, 0xba, 0xd2, 0xbd, 0xd5,
	0x72, 0x93, 0x75, 0x96, 0xca, 0xd9, 0xcd, 0xdb
};

                                                                                                                                 
uint32_t CDiskEncoding::gcr_vorpal2[]= {
	0x09, 0x0a, 0x0b, 0x0d, 0x0e, 0x0f, 0x12, 0x13, 0x15, 0x16, 0x17, 0x19, 0x1a, 0x1b, 0x1d, 0x1e,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x0c, 0x00, 0x00, 0x00, 0x00, 0x14, 0x00, 0x00, 0x00, 0x05, 0x06
};

                                                                                        
uint32_t CDiskEncoding::gcr_vmax[]= {
	0xf7, 0xf6, 0xf4, 0xf5, 0xf3, 0xf2, 0xef, 0xee,
	0xec, 0xed, 0xea, 0xeb, 0xe9, 0xe7, 0xe6, 0xe4,
	0xe5, 0xde, 0xdc, 0xdd, 0xd9, 0xdb, 0xd3, 0xd7,
	0xdf, 0xcf, 0xce, 0xcc, 0xcd, 0xcb, 0xca, 0xc9,
	0xbe, 0xbc, 0xb2, 0xbd, 0xb9, 0xb4, 0xbb, 0xba,
	0xb6, 0xb7, 0xb5, 0xaf, 0xad, 0xae, 0xac, 0xab,
	0xa7, 0x9f, 0x9e, 0x9d, 0x9b, 0x97, 0xaa, 0xa9,
	0x9c, 0x99, 0x93, 0x92, 0x96, 0xa6, 0xa4, 0xa5
};

                                                               
uint32_t CDiskEncoding::gcr_vmaxold[]= {
	0xf7, 0xf6, 0xf4, 0xf5, 0xf3, 0xf2, 0xef, 0xee,
	0xec, 0xed, 0xe2, 0xeb, 0xe9, 0xe7, 0xe6, 0xe4,
	0xe5, 0xde, 0xdc, 0xdd, 0xd9, 0xdb, 0xd3, 0xd7,
	0xdf, 0xcf, 0xce, 0xcc, 0xcd, 0xcb, 0xca, 0xc9,
	0xbe, 0xbc, 0xb2, 0xbd, 0xb9, 0xb4, 0xbb, 0xba,
	0xb6, 0xb7, 0xb5, 0xaf, 0xa3, 0xae, 0xac, 0xab,
	0xa7, 0x9f, 0x9e, 0x9d, 0x9b, 0x97, 0xaa, 0xa9,
	0x9c, 0x99, 0x93, 0x92, 0x96, 0xa6, 0xa4, 0xa5
};

                                    
uint32_t CDiskEncoding::gcr_teque[]= {
	0xab, 0xb7, 0xad, 0xb5, 0x6b, 0x77, 0x6d, 0x75,
	0xdb, 0xd7, 0xdd, 0xd5, 0x5b, 0x57, 0x5d, 0x55
};

                                      
uint32_t CDiskEncoding::gcr_ozisoft[]= {
	0x5d, 0x65, 0x66, 0x67, 0x6d, 0x77, 0x79, 0x7b,
	0x99, 0xa5, 0xa6, 0xaa, 0xb7, 0xb9, 0xba, 0xbe
};



CDiskEncoding::CDiskEncoding()
{
}

CDiskEncoding::~CDiskEncoding()
{
	delete [] fmcode;
	delete [] fmdecode;

	delete [] mfmcode;
	delete [] mfmdecode;

	delete [] gcrcode;
	delete [] gcrdecode;

	delete [] gcrcode_s;
	delete [] gcrdecode_s;

	delete [] gcrahcode;
	delete [] gcrahdecode;

	delete [] gcra5code;
	delete [] gcra5decode;

	delete [] gcra6code;
	delete [] gcra6decode;

	delete [] gcrvorpalcode;
	delete [] gcrvorpaldecode;

	delete [] gcrvorpal2code;
	delete [] gcrvorpal2decode;

	delete [] gcrvmaxcode;
	delete [] gcrvmaxdecode;

	delete [] gcr4bitcode;
	delete [] gcr4bitdecode;

	Clear();
}

                  
void CDiskEncoding::Clear()
{
	fminit=0;
	fmcode=NULL;
	fmdecode=NULL;

	mfminit=0;
	mfmcodebit=0;
	mfmcode=NULL;
	mfmdecode=NULL;

	gcrinit=gcridNone;
	gcrcode=NULL;
	gcrdecode=NULL;

	gcrinit_s=gcridNone;
	gcrcode_s=NULL;
	gcrdecode_s=NULL;

	gcrahinit=0;
	gcrahcode=NULL;
	gcrahdecode=NULL;

	gcra5init=0;
	gcra5code=NULL;
	gcra5decode=NULL;

	gcra6init=0;
	gcra6code=NULL;
	gcra6decode=NULL;

	gcrvorpalinit=0;
	gcrvorpalcode=NULL;
	gcrvorpaldecode=NULL;

	gcrvorpal2init=0;
	gcrvorpal2code=NULL;
	gcrvorpal2decode=NULL;

	gcrvmaxinit=0;
	gcrvmaxcode=NULL;
	gcrvmaxdecode=NULL;

	gcr4bitinit=0;
	gcr4bitcode=NULL;
	gcr4bitdecode=NULL;
}

                       
void CDiskEncoding::InitFM()
{
	                                       
	if (fminit)
		return;

	                
	if (!fmcode)
		fmcode=new uint32_t[256];

	if (!fmdecode)
		fmdecode=new uint32_t[65536];

	uint32_t sval;

	                            
	for (sval=0x0000; sval < 256; sval++) {
		uint32_t code=0;

		                               
		for (uint32_t bit=0x80; bit; bit>>=1) {
			code<<=2;

			                                      
			code|=(sval & bit) ? 3 : 2;
		}

		                 
		fmcode[sval]=code;
	}

	                         
	for (sval=0x0000; sval < 0x10000; sval++) {
		uint32_t code=0;

		           
		for (uint32_t bit=0x4000; bit; bit>>=2) {
			code<<=1;
			if (sval & bit)
				code|=1;
		}

		                                    
		uint32_t cd=fmcode[code] & 0xffff;
		if (cd != sval)
			code|=DF_31;
		fmdecode[sval]=code;
	}

	fminit=1;
}

                        
void CDiskEncoding::InitMFM(uint32_t mfmsize)
{
	                                         
	if (mfmsize && mfminit>=mfmsize)
		return;

	               
	delete [] mfmcode;
	mfmcode=NULL;
	delete [] mfmdecode;
	mfmdecode=NULL;

	mfminit=0;
	mfmcodebit=0;

	                       
	if (!mfmsize)
		return;

	                
	mfmcode=new uint32_t[mfmsize];
	mfmdecode=new uint32_t[mfmsize];

	                                           
	mfmcodebit=(mfmsize > 0x100) ? 16 : 8;

	uint32_t sval;

	                                                       
	for (sval=0x0000; sval < mfmsize; sval++) {
		uint32_t code=0;

		           
		for (uint32_t bit=0x8000; bit; bit>>=1) {
			code<<=2;
			                                        
			                                        
			                      
			if (sval & bit)
				code|=1;
			else
				if (!(code & 4))
					code|=2;
		}

		                 
		mfmcode[sval]=code;
	}

	                                                                    
	if (mfmsize > 0x100) {
		for (sval=0x0000; sval < mfmsize; sval++) {
			uint32_t code=0;

			           
			for (uint32_t bit=0x4000; bit; bit>>=2) {
				code<<=1;
				if (sval & bit)
					code|=1;
			}

			                                    
			uint32_t cd=mfmcode[code]&0xffff;
			if (cd!=sval && (cd&0x7fff)!=sval)
				code|=DF_31;
			mfmdecode[sval]=code;
		}
	} else {
		for (sval=0x0000; sval < mfmsize; sval++) {
			uint32_t code=0;

			           
			for (uint32_t bit=0x4000; bit; bit>>=2) {
				code<<=1;
				if (sval & bit)
					code|=1;
			}

			                 
			mfmdecode[sval]=code;
		}
	}

	mfminit=mfmsize;
}

                        
void CDiskEncoding::InitGCRCBM(uint32_t *gcrtable, int gcrid)
{
	                                                        
	if (gcrid == gcrinit)
		return;

	                
	if (!gcrcode)
		gcrcode=new uint32_t[256];

	if (!gcrdecode)
		gcrdecode=new uint32_t[1024];

	uint32_t sval;

	                                                  
	for (sval=0; sval < 1024; sval++)
		gcrdecode[sval]=DF_31;

	                   
	for (sval=0; sval < 256; sval++) {
		uint32_t ghi=gcrtable[sval >> 4];
		uint32_t glo=gcrtable[sval & 0xf];
		uint32_t code=ghi << 5 | glo;
		gcrcode[sval]=code;
		gcrdecode[code]=sval;
	}

	gcrinit=gcrid;
}

                                  
void CDiskEncoding::InitGCRCBM_S(uint32_t *gcrtable, int gcrid)
{
	                                                        
	if (gcrid == gcrinit_s)
		return;

	                
	if (!gcrcode_s)
		gcrcode_s=new uint32_t[256];

	if (!gcrdecode_s)
		gcrdecode_s=new uint32_t[1024];

	uint32_t sval;

	                                                  
	for (sval=0; sval < 1024; sval++)
		gcrdecode_s[sval]=DF_31;

	                   
	for (sval=0; sval < 256; sval++) {
		uint32_t ghi=gcrtable[sval >> 4];
		uint32_t glo=gcrtable[sval & 0xf];
		uint32_t code=ghi << 5 | glo;
		gcrcode_s[sval]=code;
		gcrdecode_s[code]=sval;
	}

	gcrinit_s=gcrid;
}

                                     
void CDiskEncoding::InitGCRAppleH()
{
	                                       
	if (gcrahinit)
		return;

	                
	if (!gcrahcode)
		gcrahcode=new uint32_t[256];

	if (!gcrahdecode)
		gcrahdecode=new uint32_t[65536];

	uint32_t sval;

	                    
	for (sval=0x0000; sval < 256; sval++) {
		                                                                 
		                    
		uint32_t ahi=(sval >> 1) | 0xaa;
		uint32_t alo=sval | 0xaa;
		uint32_t code=ahi << 8 | alo;

		                 
		gcrahcode[sval]=code;
	}

	                      
	for (sval=0x0000; sval < 0x10000; sval++) {
		uint32_t dhi=(sval >> 8) & 0x55;
		uint32_t dlo=sval & 0x55;
		uint32_t code=dhi << 1 | dlo;

		                                    
		uint32_t cd=gcrahcode[code] & 0xffff;
		if (cd != sval)
			code|=DF_31;
		gcrahdecode[sval]=code;
	}

	gcrahinit=1;
}

                                    
void CDiskEncoding::InitGCRApple5(uint32_t *gcrtable)
{
	                                       
	if (gcra5init)
		return;

	                
	if (!gcra5code)
		gcra5code=new uint32_t[32];

	if (!gcra5decode)
		gcra5decode=new uint32_t[256];

	uint32_t sval;

	                                                  
	for (sval=0; sval < 256; sval++)
		gcra5decode[sval]=DF_31;

	                   
	for (sval=0; sval < 32; sval++) {
		uint32_t code=gcrtable[sval];
		gcra5code[sval]=code;
		gcra5decode[code]=sval;
	}

	gcra5init=1;
}

                                    
void CDiskEncoding::InitGCRApple6(uint32_t *gcrtable)
{
	                                       
	if (gcra6init)
		return;

	                
	if (!gcra6code)
		gcra6code=new uint32_t[64];

	if (!gcra6decode)
		gcra6decode=new uint32_t[256];

	uint32_t sval;

	                                                  
	for (sval=0; sval < 256; sval++)
		gcra6decode[sval]=DF_31;

	                   
	for (sval=0; sval < 64; sval++) {
		uint32_t code=gcrtable[sval];
		gcra6code[sval]=code;
		gcra6decode[code]=sval;
	}

	gcra6init=1;
}

                                         
void CDiskEncoding::InitGCRVorpal(uint32_t *gcrtable)
{
	                                       
	if (gcrvorpalinit)
		return;

	                
	if (!gcrvorpalcode)
		gcrvorpalcode=new uint32_t[64];

	if (!gcrvorpaldecode)
		gcrvorpaldecode=new uint32_t[256];

	uint32_t sval;

	                                                  
	for (sval=0; sval < 256; sval++)
		gcrvorpaldecode[sval]=DF_31;

	                   
	for (sval=0; sval < 64; sval++) {
		uint32_t code=gcrtable[sval];
		gcrvorpalcode[sval]=code;
		gcrvorpaldecode[code]=sval;
	}

	gcrvorpalinit=1;
}

                                         
void CDiskEncoding::InitGCRVorpal2(uint32_t *gcrtable)
{
	                                       
	if (gcrvorpal2init)
		return;

	                
	if (!gcrvorpal2code)
		gcrvorpal2code=new uint32_t[32];

	if (!gcrvorpal2decode)
		gcrvorpal2decode=new uint32_t[1024];

	                          
	int gdtab[32];
	int i;

	                             
	for (i=0; i < 32; i++)
		gdtab[i]=-1;

	                                   
	for (i=0; i < 32; i++) {
		uint32_t code=gcrtable[i];
		gcrvorpal2code[i]=code;

		                     
		if (!code)
			continue;

		                      
		gdtab[code]=i & 0x0f;
	}

	                                                                              
	for (uint32_t sval=0; sval < 1024; sval++) {
		                                  
		gcrvorpal2decode[sval]=DF_31;

		                              
		int dechi=gdtab[sval >> 5];
		int declo=gdtab[sval & 0x1f];

		                                                                                       
		if (dechi < 0 || declo < 0)
			continue;

		int lbv=-1;
		int bc=0;

		                                                                  
		uint32_t bm;
		for (bm=1 << 9; bm; bm >>= 1) {
			                    
			int bitval=(sval & bm) ? 1 : 0;
			if (lbv != bitval) {
				                                                   
				lbv=bitval;
				bc=1;
			} else {
				                                    
				bc++;

				                                                                                    
				if ((!lbv && bc >= 3) || (lbv && bc >= 5))
					break;
			}
		}

		                                           
		if (bm)
			continue;

		                          
		uint32_t code=dechi << 4 | declo;
		gcrvorpal2decode[sval]=code;
	}

	gcrvorpal2init=1;
}

                                         
void CDiskEncoding::InitGCRVMax(uint32_t *gcrtable, int vmaxid)
{
	                                                        
	if (gcrvmaxinit == vmaxid)
		return;

	                
	if (!gcrvmaxcode)
		gcrvmaxcode=new uint32_t[64];

	if (!gcrvmaxdecode)
		gcrvmaxdecode=new uint32_t[256];

	uint32_t sval;

	                                                  
	for (sval=0; sval < 256; sval++)
		gcrvmaxdecode[sval]=DF_31;

	                   
	for (sval=0; sval < 64; sval++) {
		uint32_t code=gcrtable[sval];
		if (!code)
			continue;

		gcrvmaxcode[sval]=code;
		gcrvmaxdecode[code]=sval;
	}

	gcrvmaxinit=vmaxid;
}

                              
void CDiskEncoding::InitGCR4Bit(uint32_t *gcrtable)
{
	                                       
	if (gcr4bitinit)
		return;

	                
	if (!gcr4bitcode)
		gcr4bitcode=new uint32_t[16];

	if (!gcr4bitdecode)
		gcr4bitdecode=new uint32_t[256];

	uint32_t sval;

	                                                  
	for (sval=0; sval < 256; sval++)
		gcr4bitdecode[sval]=DF_31;

	                   
	for (sval=0; sval < 16; sval++) {
		uint32_t code=gcrtable[sval];
		gcr4bitcode[sval]=code;
		gcr4bitdecode[code]=sval;
	}

	gcr4bitinit=1;
}

                                                                                       
int CDiskEncoding::FindViolation(uint8_t *buffer, int bitpos, int bitcnt, int max0, int max1, int mode)
{
	int vcnt=0;

	                                                                         
	if (!buffer || bitpos < 0 || bitcnt <= 0 || (max0 < 0 && max1 < 0))
		return mode ? vcnt : -1;

	                                                                        
	uint32_t bitmask=1 << ((bitpos & 7) ^ 7);
	uint32_t pos=bitpos >> 3;
	uint32_t sval=buffer[pos++];

	                    
	int lbv=-1;
	int bc=0;

	                   
	while (bitcnt-- > 0) {
		                                                                           
		if (!bitmask) {
			bitmask=0x80;
			sval=buffer[pos++];
		}

		                    
		int bitval=(sval & bitmask) ? 1 : 0;

		if (lbv != bitval) {
			                                                   
			lbv=bitval;
			bc=1;
		} else {
			                                    
			bc++;

			                                                                                            
			if ((!lbv && max0 >= 0 && bc >= max0) || (lbv && max1 >= 0 && bc >= max1)) {
				                                                                        
				if (mode)
					vcnt++;
				else
					return bitpos;
			}
		}

		                   
		bitpos++;
		bitmask >>= 1;
	}

	                                                                 
	return mode ? vcnt : -1;
}

