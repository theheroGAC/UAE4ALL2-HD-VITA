#ifndef CAPSFORM_H
#define CAPSFORM_H

#pragma pack(push, 1)

                                      
struct CapsFormatBlock {
	UDWORD gapacnt;                               
	UDWORD gapavalue;                             
	UDWORD gapbcnt;                              
	UDWORD gapbvalue;                            
	UDWORD gapccnt;                                
	UDWORD gapcvalue;                              
	UDWORD gapdcnt;                               
	UDWORD gapdvalue;                             
	UDWORD blocktype;                 
	UDWORD track;              
	UDWORD side;              
	UDWORD sector;              
	SDWORD sectorlen;                          
	PUBYTE databuf;                        
	UDWORD datavalue;                                       
};

typedef struct CapsFormatBlock *PCAPSFORMATBLOCK;

                                      
struct CapsFormatTrack {
	UDWORD type;                       
	UDWORD gapacnt;                           
	UDWORD gapavalue;                         
	UDWORD gapbvalue;                          
	PUBYTE trackbuf;                        
	UDWORD tracklen;                               
	UDWORD buflen;                                   
	UDWORD bufreq;                                  
	UDWORD startpos;                             
	SDWORD blockcnt;                     
	PCAPSFORMATBLOCK block;              
	UDWORD size;                         
};

typedef struct CapsFormatTrack *PCAPSFORMATTRACK;

#pragma pack(pop)

              
enum {
	cfrmbtNA=0,                 
	cfrmbtIndex,          
	cfrmbtData          
};

#endif
