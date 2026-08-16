#ifndef CAPSDEFINITIONS_H
#define CAPSDEFINITIONS_H

#pragma pack(push, 1)

#define CAPS_ENCODER 1
#define CAPS_ENCODER_REV 1
#define SPS_ENCODER 2
#define SPS_ENCODER_REV 1

#define CAPS_IDFILE "CAPS"
#define CAPS_IDDUMP "DUMP"
#define CAPS_IDDATA "DATA"
#define CAPS_IDTRCK "TRCK"
#define CAPS_IDINFO "INFO"
#define CAPS_IDIMGE "IMGE"
#define CAPS_IDCTEX "CTEX"
#define CAPS_IDCTEI "CTEI"
#define CAPS_IDPACK "PACK"

#define CAPS_PHOLD 24576

#define CAPS_DATAMASK 0x1f
#define CAPS_SIZE_S 5

                  
#define CAPS_IF_FLAKEY DF_0

                  
#define CAPS_BF_GP0 DF_0
#define CAPS_BF_GP1 DF_1
#define CAPS_BF_DMB DF_2

#define CAPS_EF_RESAMPLE DF_0



                     
struct CapsID {
	UBYTE name[4];                    
	UDWORD size;                                                     
	UDWORD hcrc;                                    
};

typedef CapsID *PCAPSID;

                     
struct CapsDump {
	UDWORD type;               
	UDWORD size;               
	UDWORD area;                   
	UDWORD did;                          
};

typedef CapsDump *PCAPSDUMP;

            
struct CapsData {
	UDWORD size;                                        
	UDWORD bsize;                          
	UDWORD dcrc;                  
	UDWORD did;                           
};

typedef CapsData *PCAPSDATA;

               
struct CapsTrack {
	UDWORD type;              
	UDWORD cyl;             
	UDWORD head;        
	UDWORD did;                          
};

typedef CapsTrack *PCAPSTRACK;

                               
struct CapsDateTime {
	UDWORD date;                         
	UDWORD time;                          
};

typedef CapsDateTime *PCAPSDATETIME;

                    
struct CapsInfo {
	UDWORD type;                     
	UDWORD encoder;                        
	UDWORD encrev;                               
	UDWORD release;                  
	UDWORD revision;                          
	UDWORD origin;                                  
	UDWORD mincylinder;                          
	UDWORD maxcylinder;                           
	UDWORD minhead;                          
	UDWORD maxhead;                           
	CapsDateTime crdt;                             
	UDWORD platform[CAPS_MAXPLATFORM];                        
	UDWORD disknum;                                            
	UDWORD userid;                                     
	UDWORD reserved[3];              
};

typedef CapsInfo *PCAPSINFO;

                         
struct CapsImage {
	UDWORD cylinder;             
	UDWORD head;             
	UDWORD dentype;                 
	UDWORD sigtype;                           
	UDWORD trksize;                                
	UDWORD startpos;                           
	UDWORD startbit;                                   
	UDWORD databits;                             
	UDWORD gapbits;                             
	UDWORD trkbits;                               
	UDWORD blkcnt;                      
	UDWORD process;                     
	UDWORD flag;                   
	UDWORD did;                              
	UDWORD reserved[3];              
};

typedef CapsImage *PCAPSIMAGE;

                                                            
struct CapsBlockExt {
	UDWORD blocksize;                                
	UDWORD gapsize;                                
};

                                                       
struct SPSBlockExt {
	UDWORD gapoffset;                                      
	UDWORD celltype;                  
};

                              
union CapsBlockType {
	CapsBlockExt caps;                    
	SPSBlockExt sps;                      
};

                         
struct CapsBlock {
	UDWORD blockbits;                               
	UDWORD gapbits;                               
	CapsBlockType bt;                                    
	UDWORD enctype;                   
	UDWORD flag;                     
	UDWORD gapvalue;                       
	UDWORD dataoffset;                                      
};

typedef CapsBlock *PCAPSBLOCK;



                  
struct CapsExport {
	UDWORD cylinder;             
	UDWORD head;             
	UDWORD dentype;                 
	UDWORD anaid;                               
	UDWORD anafix;                         
	UDWORD anatrs;                         
	UDWORD reserved[2];              
};

typedef CapsExport *PCAPSEXPORT;

                       
struct CapsExportInfo {
	UDWORD releasecrc;                        
	UDWORD anarev;                         
	UDWORD reserved[14];              
};

typedef CapsExportInfo *PCAPSEXPORTINFO;



                          
struct CapsPack {
	UBYTE sign[4];                  
	UDWORD usize;                           
	UDWORD ucrc;                              
	UDWORD csize;                             
	UDWORD ccrc;                            
	UDWORD hcrc;                                        
};

typedef CapsPack *PCAPSPACK;

                  
struct CapsRaw {
	UDWORD time;
	UDWORD raw;
};

typedef CapsRaw *PCAPSRAW;



                     
enum {
	cpdKICK=1,                 
	cpdBOOT              
};

             
enum {
	cpimtNA=0,                      
	cpimtFDD,                
	cpimtLast
};

                                                          
enum {
	cppidNA=0,                                  
	cppidAmiga,
	cppidAtariST,
	cppidPC,
	cppidAmstradCPC,
	cppidSpectrum,
	cppidSamCoupe,
	cppidArchimedes,
	cppidC64,
	cppidAtari8,
	cppidLast
};

                
enum {
	cpdenNA=0,                       
	cpdenNoise,                                          
	cpdenAuto,                                                    
	cpdenCLAmiga,                   
	cpdenCLAmiga2,                       
	cpdenCLST,                   
	cpdenSLAmiga,                    
	cpdenSLAmiga2,                        
	cpdenABAmiga,                        
	cpdenABAmiga2,                                    
	cpdenLast
};

                         
enum {
	cpsigNA=0,                       
	cpsig2us,              
	cpsigLast
};

               
enum {
	cpbctNA=0,                     
	cpbct2us,              
	cpbctLast
};

                
enum {
	cpencNA=0,                   
	cpencMFM,        
	cpencRaw,                                    
	cpencLast
};

             
enum {
	cpdatEnd=0,                   
	cpdatMark,              
	cpdatData,         
	cpdatGap,         
	cpdatRaw,         
	cpdatFData,               
	cpdatLast
};

            
enum {
	cpgapEnd=0,                  
	cpgapCount,               
	cpgapData,                     
	cpgapLast
};



                  
union CapsMod {
	CapsDump dump;
	CapsData data;
	CapsTrack trck;
	CapsInfo info;
	CapsImage imge;
	CapsExport ctex;
	CapsExportInfo ctei;
};

                
struct CapsGeneric {
	CapsID file;                                 
	CapsMod mod;                 
};

typedef CapsGeneric *PCAPSGENERIC;

                                 
struct CapsChunk {
	int type;                                          
	CapsGeneric cg;                            
};

typedef CapsChunk *PCAPSCHUNK;

#pragma pack(pop)

#endif
