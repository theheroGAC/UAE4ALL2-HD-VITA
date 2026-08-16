#ifndef DISKIMAGE_H
#define DISKIMAGE_H

#define MAX_CYLINDER 65536
#define MAX_HEAD 2
#define DEF_TRACKINFO (1024*MAX_HEAD)
#define DEF_SCANINFO 200
#define DEF_SCANEXTEND 2
#define DEF_CRCBUF 65536
#define DEF_FDALLOC 8

#define DI_COMP_MARK  DF_0
#define DI_COMP_DATA  DF_1
#define DI_COMP_RAW   DF_2
#define DI_COMP_FDATA DF_3

#define DI_COMP_CTT (DI_COMP_MARK|DI_COMP_DATA|DI_COMP_RAW|DI_COMP_FDATA)
#define DI_COMP_CTX (DI_COMP_DATA|DI_COMP_FDATA)

#define DI_LOCK_ANA  DF_30
#define DI_LOCK_COMP DF_31
#define DI_LOCK_CTA (DI_LOCK_ANA|DI_LOCK_DENVAR|DI_LOCK_DENAUTO|DI_LOCK_DENNOISE|DI_LOCK_NOISE|DI_LOCK_UPDATEFD)



                   
struct DiskDataMark {
	int group;                   
	uint32_t position;                 
	int size;             
};

typedef DiskDataMark *PDISKDATAMARK;

                                               
struct DiskSectorInfo {
	uint32_t descdatasize;                                         
	uint32_t descgapsize;                                         
	uint32_t datasize;                                      
	uint32_t gapsize;                                      
	uint32_t datastart;                                               
	uint32_t gapstart;                                               
	uint32_t gapsizews0;                                  
	uint32_t gapsizews1;                                 
	uint32_t gapws0mode;                                       
	uint32_t gapws1mode;                                      
	uint32_t celltype;                    
	uint32_t enctype;                     
};

typedef DiskSectorInfo *PDISKSECTORINFO;

                               
struct DiskImageInfo {
	int valid;                                      
	int error;                     
	int dirty;                                     
	int readonly;                                             
	int modimage;                                           
	int rawlock;                                              
	int umincylinder;                               
	int umaxcylinder;                                
	int uminhead;                               
	int umaxhead;                                
	int dmpcount;                     
	int relcount;                    
	int didmax;                                       
	int nextrev;                                      
	int lastrev;                                          
	int realrev;                                                      
	int civalid;                            
	CapsInfo ci;                                       
};

typedef DiskImageInfo *PDISKIMAGEINFO;

                               
struct DiskTrackInfo {
	int type;                         
	int linked;                                
	int linkinfo;                                
	UDWORD linkflag;                     
	int cylinder;                     
	int head;                     
	int sectorcnt;                      
	int headerpos;                                           
	int datapos;                                           
	int datasize;                             
	int trackcnt;                         
	int rawtrackcnt;                                       
	int rawlen;                                            
	int rawtimecnt;                                         
	PUDWORD rawtimebuf;                                    
	int rawupdate;                                                                                           
	int rawdump;                                        
	PUBYTE trackbuf;                        
	int tracklen;                                 
	PUBYTE trackdata[CAPS_MTRS];                                   
	int tracksize[CAPS_MTRS];                      
	int trackstart[CAPS_MTRS];                          
	int timecnt;                               
	PUDWORD timebuf;                      
	int fixpos;                                                 
	int comppos;                                                 
	int sdpos;                                
	uint32_t wseed;                            
	int compsblk;                          
	int compeblk;                        
	int fdpsize;                                        
	int fdpmax;                              
	PDISKDATAMARK fdp;               
	int overlap;                                          
	int overlapbit;                               
	uint32_t trackbc;                                                               
	uint32_t singletrackbc;                                                
	uint32_t startbit;                              
	int sipsize;                            
	PDISKSECTORINFO sip;               
	CapsImage ci;                                           
};

typedef DiskTrackInfo *PDISKTRACKINFO;

              
enum {
	dtitUndefined,                         
	dtitError,                            
	dtitCapsDump,                     
	dtitCapsImage,                     
	dtitPlain                               
};



                     
class CDiskImage  
{
public:
	CDiskImage();
	virtual ~CDiskImage();
	virtual int Lock(PCAPSFILE pcf);
	virtual int Unlock();
	virtual int LoadImage(UDWORD flag, int free=false);
	PDISKTRACKINFO GetTrack(int cylinder, int head);
	PDISKTRACKINFO LockTrack(int cylinder, int head, UDWORD flag);
	PDISKTRACKINFO LockTrackComp(int cylinder, int head, UDWORD flag, int sblk, int eblk);
	PDISKTRACKINFO UnlockTrack(int cylinder, int head, int forced=false);
	static void UnlockTrack(PDISKTRACKINFO pti, int forced=false);
	void UnlockTrack(int forced=false);
	static int LinkTrackData(PDISKTRACKINFO pti, int size);
	PDISKIMAGEINFO GetInfo();
	static void CreateDateTime(PCAPSDATETIME pcd);
	static void DecodeDateTime(PCAPSDATETIMEEXT dec, PCAPSDATETIME pcd);
	static LPCSTR GetPlatformName(int pid);
	static UDWORD CrcFile(PCAPSFILE pcf);

protected:
	void Destroy();
	int AddTrack(PDISKTRACKINFO pdti);
	virtual int LoadTrack(PDISKTRACKINFO pti, UDWORD flag);
	int LoadPlain(PDISKTRACKINFO pti);
	int AllocTrack(PDISKTRACKINFO pti, UDWORD flag);
	static void FreeTrack(PDISKTRACKINFO pti, int forced=false);
	static void FreeTrackData(PDISKTRACKINFO pti);
	static void FreeTrackTiming(PDISKTRACKINFO pti);
	static void FreeTrackFD(PDISKTRACKINFO pti);
	PDISKTRACKINFO MapTrack(int cylinder, int head);
	void UpdateImageInfo(PDISKTRACKINFO pti);
	static UDWORD ReadValue(PUBYTE buf, int cnt);
	static PDISKDATAMARK AddFD(PDISKTRACKINFO pti, PDISKDATAMARK src, int size, int units=0);
	static PDISKDATAMARK AllocFD(PDISKTRACKINFO pti, int size, int units=DEF_FDALLOC);
	static void AllocTrackSI(PDISKTRACKINFO pti);
	static void FreeTrackSI(PDISKTRACKINFO pti);

protected:
	DiskImageInfo dii;
	int dticnt;
	int dticyl;
	int dtihed;
	PDISKTRACKINFO dti;
	static LPCSTR pidname[cppidLast];
};

typedef CDiskImage *PCDISKIMAGE;



                        
inline PDISKIMAGEINFO CDiskImage::GetInfo()
{
	return dii.valid ? &dii : NULL;
}

#endif
