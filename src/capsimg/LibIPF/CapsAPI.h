#ifndef CAPSAPI_H
#define CAPSAPI_H

#define CAPS_FILEEXT "ipf"
#define CAPS_FILEPFX ".ipf"

                                        
                                              
                                        
                                                        
                                                     
                                             
                                
                                                              
                                                                      
                                                                            
                                                     
                                         
                                  
                                                             
                                                                    
                                        
#define DI_LOCK_INDEX    DF_0
#define DI_LOCK_ALIGN    DF_1
#define DI_LOCK_DENVAR   DF_2
#define DI_LOCK_DENAUTO  DF_3
#define DI_LOCK_DENNOISE DF_4
#define DI_LOCK_NOISE    DF_5
#define DI_LOCK_NOISEREV DF_6
#define DI_LOCK_MEMREF   DF_7
#define DI_LOCK_UPDATEFD DF_8
#define DI_LOCK_TYPE     DF_9
#define DI_LOCK_DENALT   DF_10
#define DI_LOCK_OVLBIT   DF_11
#define DI_LOCK_TRKBIT   DF_12
#define DI_LOCK_NOUPDATE DF_13
#define DI_LOCK_SETWSEED DF_14

#define CAPS_MAXPLATFORM 4
#define CAPS_MTRS 5

#define CTIT_FLAG_FLAKEY DF_31
#define CTIT_MASK_TYPE 0xff

#pragma pack(push, 1)

                         
struct CapsDateTimeExt {
	UDWORD year;
	UDWORD month;
	UDWORD day;
	UDWORD hour;
	UDWORD min;
	UDWORD sec;
	UDWORD tick;
};

typedef struct CapsDateTimeExt *PCAPSDATETIMEEXT;

                                    
struct CapsVersionInfo {
	UDWORD type;                    
	UDWORD release;               
	UDWORD revision;               
	UDWORD flag;                       
};

typedef struct CapsVersionInfo *PCAPSVERSIONINFO;

                               
struct CapsImageInfo {
	UDWORD type;                     
	UDWORD release;                  
	UDWORD revision;                          
	UDWORD mincylinder;                          
	UDWORD maxcylinder;                           
	UDWORD minhead;                          
	UDWORD maxhead;                           
	struct CapsDateTimeExt crdt;                            
	UDWORD platform[CAPS_MAXPLATFORM];                        
};

typedef struct CapsImageInfo *PCAPSIMAGEINFO;

                               
struct CapsTrackInfo {
	UDWORD type;                    
	UDWORD cylinder;               
	UDWORD head;               
	UDWORD sectorcnt;                      
	UDWORD sectorsize;               
	UDWORD trackcnt;                         
	PUBYTE trackbuf;                          
	UDWORD tracklen;                                
	PUBYTE trackdata[CAPS_MTRS];                                   
	UDWORD tracksize[CAPS_MTRS];                   
	UDWORD timelen;                         
	PUDWORD timebuf;                 
};

typedef struct CapsTrackInfo *PCAPSTRACKINFO;

                               
struct CapsTrackInfoT1 {
	UDWORD type;                    
	UDWORD cylinder;               
	UDWORD head;               
	UDWORD sectorcnt;                      
	UDWORD sectorsize;               
	PUBYTE trackbuf;                          
	UDWORD tracklen;                                
	UDWORD timelen;                           
	PUDWORD timebuf;                   
	SDWORD overlap;                       
};

typedef struct CapsTrackInfoT1 *PCAPSTRACKINFOT1;

                               
struct CapsTrackInfoT2 {
	UDWORD type;                    
	UDWORD cylinder;               
	UDWORD head;               
	UDWORD sectorcnt;                      
	UDWORD sectorsize;                       
	PUBYTE trackbuf;                          
	UDWORD tracklen;                                
	UDWORD timelen;                           
	PUDWORD timebuf;                   
	SDWORD overlap;                       
	UDWORD startbit;                                    
	UDWORD wseed;                                
	UDWORD weakcnt;                                
};

typedef struct CapsTrackInfoT2 *PCAPSTRACKINFOT2;

                                
struct CapsSectorInfo {
	UDWORD descdatasize;                                         
	UDWORD descgapsize;                                         
	UDWORD datasize;                                      
	UDWORD gapsize;                                      
	UDWORD datastart;                                               
	UDWORD gapstart;                                               
	UDWORD gapsizews0;                                  
	UDWORD gapsizews1;                                 
	UDWORD gapws0mode;                                       
	UDWORD gapws1mode;                                      
	UDWORD celltype;                    
	UDWORD enctype;                     
};

typedef struct CapsSectorInfo *PCAPSSECTORINFO;

                              
struct CapsDataInfo {
	UDWORD type;              
	UDWORD start;                  
	UDWORD size;                 
};

typedef struct CapsDataInfo *PCAPSDATAINFO;

                              
struct CapsRevolutionInfo {
	SDWORD next;                                                          
	SDWORD last;                                                          
	SDWORD real;                                                               
	SDWORD max;                                                                                             
};

typedef struct CapsRevolutionInfo *PCAPSREVOLUTIONINFO;

#pragma pack(pop)

                           
enum {
	ciitNA=0,                      
	ciitFDD                 
};

                                                                        
enum {
	ciipNA=0,                                  
	ciipAmiga,
	ciipAtariST,
	ciipPC,
	ciipAmstradCPC,
	ciipSpectrum,
	ciipSamCoupe,
	ciipArchimedes,
	ciipC64,
	ciipAtari8
};

                           
enum {
	ctitNA=0,                 
	ctitNoise,                                       
	ctitAuto,                                                 
	ctitVar                       
};

                              
enum {
	csicNA=0,                     
	csic2us               
};

                              
enum {
	csieNA=0,                     
	csieMFM,        
	csieRaw                                     
};

                               
enum {
	csiegmFixed=0,                                 
	csiegmAuto,                                                                        
	csiegmResize                                                          
};

                         
enum {
	cditNA=0,             
	cditWeak              
};

                      
enum {
	cgiitNA=0,                
	cgiitSector,                     
	cgiitWeak,                                
	cgiitRevolution                      
};

                         
enum {
	citError=0,                                                
	citUnknown,                          
	citIPF,                     
	citCTRaw,                      
	citKFStream,                            
	citKFStreamCue,                            
	citDraft                      
};

                     
enum {
	imgeOk=0,
	imgeUnsupported,
	imgeGeneric,
	imgeOutOfRange,
	imgeReadOnly,
	imgeOpen,
	imgeType,
	imgeShort,
	imgeTrackHeader,
	imgeTrackStream,
	imgeTrackData,
	imgeDensityHeader,
	imgeDensityStream,
	imgeDensityData,
	imgeIncompatible,
	imgeUnsupportedType,
	imgeBadBlockType,
	imgeBadBlockSize,
	imgeBadDataStart,
	imgeBufferShort
};

#endif
