#ifndef CAPSIMAGESTD_H
#define CAPSIMAGESTD_H

#define CIMG_MFMNOISE 0x10030f01
#define CIMG_OVERLAPBIT 3
#define MAX_STREAMBIT 32

                              
struct ImageBlockInfo {
	uint32_t blockbits;                              
	uint32_t gapbits;                              
	uint32_t gapoffset;                                     
	uint32_t celltype;                
	uint32_t enctype;                 
	uint32_t flag;               
	uint32_t gapvalue;                     
	uint32_t dataoffset;                                      
	int fdenc;                                           
	uint32_t fdbitpos;                                          
};

typedef ImageBlockInfo *PIMAGEBLOCKINFO;

                
enum {
	ibieNA,                    
	ibieRaw,            
	ibieMFM,                
	ibieWeak             
};

                            
struct ImageStreamInfo {
	int strtype;               
	int actblock;                  
	int enctype;                      
	int actenctype;                       
	int sizemodebit;                                            
	uint32_t strstart;                                 
	uint32_t strend;                               
	uint32_t strofs;                                                              
	uint32_t strsize;                          
	uint8_t *strbase;                                   
	uint8_t gapstr[4];                                                                 
	uint8_t weakdata[4];                               
	int readresult;                                
	int readend;                                    
	uint32_t readvalue;                                   
	int allowloop;                                                          
	int setencmode;                                            
	uint32_t streambc;                                                                                          
	uint32_t samplebc;                                                          
	uint32_t remstreambc;                                               
	uint32_t remsamplebc;                                                                    
	uint32_t sampleofs;                                     
	uint32_t samplemask;                                                       
	uint8_t *samplebase;                                                   
	uint32_t prcbitpos;                                           
	int prcrembc;                                          
	int prcskipbc;                                                
	uint32_t prcencstate;                                                                    
	int prcwritebc;                           
	uint32_t loopofs;                                                                        
	int loopsize;                                                  
	int looptype;                             
	int esfixbc;                              
	int esloopbc;                             
	int scenable;                                         
	uint32_t scofs;                             
	int scmul;                                                                  
};

typedef ImageStreamInfo *PIMAGESTREAMINFO;

               
enum {
	isitData,               
	isitGap0,                                                                   
	isitGap1                                                                     
};

                    
enum {
	isiemRaw,                                             
	isiemType,                                        
	isiemWeak                                             
};

             
enum {
	isiltNone,                             
	isiltAuto,                                                        
	isiltStream                                
};

                         
struct DiskDecoderInfo {
	uint8_t *track;                
	uint32_t trackbc;                                                               
	uint32_t singletrackbc;                                                
	uint32_t dsctrackbc;                                                   
	uint32_t dscdatabc;                                                         
	uint32_t dscgapbc;                                                        
	uint32_t dscstartbit;                                            
	uint32_t encbitpos;                                                  
	int encwritebc;                                                
	int encgsvalid;                              
	uint32_t encgapsplit;                                    
	uint8_t *data;                      
	int datasize;                       
	int datacount;                             
	PIMAGEBLOCKINFO block;                    
	int blocksize;                                 
	int blockcount;                                     
	uint32_t flag;                  
	PDISKTRACKINFO pdt;                         
};

typedef DiskDecoderInfo *PDISKDECODERINFO;



                              
class CCapsImageStd : public CDiskImage
{
public:
	CCapsImageStd();
	virtual ~CCapsImageStd();
	int Lock(PCAPSFILE pcf);
	int Unlock();

protected:
	struct ScanInfo {
		int track;
		int data;
	};

protected:
	int LoadTrack(PDISKTRACKINFO pti, UDWORD flag);
	int ScanImage();
	int UpdateImage(int group);
	int UpdateWeakBit(int group);
	void UpdateOverlap();
	int DecodeImage();
	int ProcessImage();
	virtual int CompareImage();
	virtual int DecompressDump();
	virtual int UpdateDump();

	int DecodeDensity(PDISKTRACKINFO pti, PUBYTE buf, UDWORD flag);
	int ConvertDensity(PDISKTRACKINFO pti);
	int GenerateNoiseTrack(PDISKTRACKINFO pti);
	int GenerateNoiseDensity(PDISKTRACKINFO pti);
	int GenerateAutoDensity(PDISKTRACKINFO pti);
	int GenerateCLA(PDISKTRACKINFO pti, PUBYTE buf);
	int GenerateCLA2(PDISKTRACKINFO pti, PUBYTE buf);
	int GenerateCLST(PDISKTRACKINFO pti, PUBYTE buf);
	int GenerateSLA(PDISKTRACKINFO pti, PUBYTE buf);
	int GenerateSLA2(PDISKTRACKINFO pti, PUBYTE buf);
	int GenerateABA(PDISKTRACKINFO pti, PUBYTE buf);
	int GenerateABA2(PDISKTRACKINFO pti, PUBYTE buf);

	void InitSystem();
	void Clear();
	void AllocDiskData(int maxsize);
	void AllocImageBlock(int maxsize);
	void FreeDiskData();
	void FreeImageBlock();
	void FreeDecoder();
	int CheckEncoder(int encoder, int revision);
	int GetBlock(PIMAGEBLOCKINFO pi, int blk);
	int GetBlock(PCAPSBLOCK pb, int blk);
	int InitDecoder();
	int InitStream(PIMAGESTREAMINFO pstr, int strtype, int blk);
	int InitDataStream(PIMAGESTREAMINFO pstr);
	int InitGapStream(PIMAGESTREAMINFO pstr);
	int FindGapStreamEnd(PIMAGESTREAMINFO pstr, int next);
	int ResetStream(PIMAGESTREAMINFO pstr);
	int ReadSampleInit(PIMAGESTREAMINFO pstr);
	int ReadSample(PIMAGESTREAMINFO pstr, int maxbc);
	int GetSample(PIMAGESTREAMINFO pstr);
	int GetSampleRaw(PIMAGESTREAMINFO pstr);
	int GetSampleGap(PIMAGESTREAMINFO pstr);
	int GetSampleData(PIMAGESTREAMINFO pstr);
	int ProcessStream(PIMAGESTREAMINFO pstr, uint32_t bitpos, int maxbc, int skipbc, int encnew);
	void ProcessStreamRaw(PIMAGESTREAMINFO pstr);
	void ProcessStreamMFM(PIMAGESTREAMINFO pstr);
	void ProcessStreamWeak(PIMAGESTREAMINFO pstr);
	int CalculateStreamSize(PIMAGESTREAMINFO pstr);
	int GetEncodedSize(PIMAGESTREAMINFO pstr, int bitcnt);
	int FindLoopPoint(PIMAGESTREAMINFO pstr);
	int ProcessBlock(int blk, uint32_t bitpos, int datasize, int gapsize);
	int ProcessBlockData(int blk, int datasize);
	int ProcessBlockGap(int blk, int gapsize);
	int ProcessBlockGap(PIMAGESTREAMINFO pg, int gapsize);
	int ProcessBlockGap(PIMAGESTREAMINFO pg0, PIMAGESTREAMINFO pg1, int gapsize);
	int ProcessBlockGap(PIMAGESTREAMINFO pg0, PIMAGESTREAMINFO pg1, int gapsize, int loopsel);
	void SetLoop(PIMAGESTREAMINFO pg, int value);
	void GetLoop(PIMAGESTREAMINFO pg);
	void MFMFixup();

protected:
	CBitBuffer trackbuf;                                    
	CCapsLoader loader;
	DiskDecoderInfo di;                     
	int maxwritelen;                                                   
	int rawreadlen;                                               
	int mfmreadlen;                                               
	uint32_t mfmindexmask;                                                
	uint32_t mfmmsbclear;                                  
};

typedef CCapsImageStd *PCCAPSIMAGESTD;

#endif
