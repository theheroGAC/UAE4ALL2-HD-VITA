#include "stdafx.h"



                       
LPCSTR CDiskImage::pidname[cppidLast]= {
	"N/A",
	"Amiga",
	"Atari ST",
	"PC",
	"Amstrad CPC",
	"Spectrum",
	"Sam Coupe",
	"Archimedes",
	"C64",
	"Atari 8-bit",
};



CDiskImage::CDiskImage()
{
	MakeCRCTable();
	dti=NULL;
	Destroy();
}

CDiskImage::~CDiskImage()
{
	Destroy();
}

                     
int CDiskImage::Lock(PCAPSFILE pcf)
{
	return imgeUnsupported;
}

               
int CDiskImage::Unlock()
{
	return imgeUnsupported;
}

                                     
int CDiskImage::LoadImage(UDWORD flag, int free)
{
	int res=imgeOk;

	                           
	if (!dti)
		return res;

	                         
	for (int trk=0; trk < dticnt; trk++) {
		PDISKTRACKINFO pt=dti+trk;

		                            
		switch (pt->type) {
			case dtitUndefined:
			case dtitError:
				continue;
		}

		                  
		int read=AllocTrack(pt, flag);

		                               
		if (free)
			FreeTrack(pt);

		                   
		switch (read) {
			                              
			case imgeUnsupported:
			case imgeIncompatible:
				trk=dticnt;
				res=read;
				continue;

			            
			case imgeOk:
				break;

			            
			default:
				res=imgeGeneric;
				break;
		}
	}

	return res;
}

                                 
void CDiskImage::Destroy()
{
	                        
	UnlockTrack(true);

	                
	memset(&dii, 0, sizeof(dii));

	            
	dii.readonly=true;

	                           
	dii.rawlock=true;

	                
	dticnt=0;
	delete [] dti;
	dti=NULL;
	dticyl=0;
	dtihed=0;
}

                       
int CDiskImage::AddTrack(PDISKTRACKINFO pdti)
{
	                   
	UnlockTrack(pdti->cylinder, pdti->head, true);

	                           
	PDISKTRACKINFO pt=MapTrack(pdti->cylinder, pdti->head);
	if (!pt)
		return imgeOutOfRange;

	               
	*pt=*pdti;

	                    
	UpdateImageInfo(pdti);

	return imgeOk;
}

                        
PDISKTRACKINFO CDiskImage::GetTrack(int cylinder, int head)
{
	                           
	if (!dti)
		return NULL;

	                        
	if (cylinder<0 || cylinder>=dticyl)
		return NULL;

	                    
	if (head<0 || head>=dtihed)
		return NULL;

	return &dti[cylinder*dtihed+head];
}

                                                   
PDISKTRACKINFO CDiskImage::LockTrack(int cylinder, int head, UDWORD flag)
{
	                                      
	dii.lastrev = dii.nextrev;

	                           
	PDISKTRACKINFO pti=GetTrack(cylinder, head);

	                            
	dii.error=AllocTrack(pti, flag);

	                                                            
	                                   
	if (!(flag & DI_LOCK_NOUPDATE))
		dii.nextrev = (dii.nextrev + 1) & 0xff;

	return dii.error == imgeOk ? pti : NULL;
}

                                                                  
PDISKTRACKINFO CDiskImage::LockTrackComp(int cylinder, int head, UDWORD flag, int sblk, int eblk)
{
	                           
	PDISKTRACKINFO pti=GetTrack(cylinder, head);
	pti->compsblk=sblk;
	pti->compeblk=eblk;
	flag|=DI_LOCK_COMP;

	                            
	dii.error=AllocTrack(pti, flag);
	return dii.error == imgeOk ? pti : NULL;
}

                                                     
PDISKTRACKINFO CDiskImage::UnlockTrack(int cylinder, int head, int forced)
{
	                           
	PDISKTRACKINFO pti=GetTrack(cylinder, head);

	               
	FreeTrack(pti, forced);

	return pti;
}

                           
void CDiskImage::UnlockTrack(PDISKTRACKINFO pti, int forced)
{
	               
	FreeTrack(pti, forced);
}

                                
void CDiskImage::UnlockTrack(int forced)
{
	                           
	if (!dti)
		return;

	                   
	for (int trk=0; trk < dticnt; trk++)
		FreeTrack(dti+trk, forced);
}

                                                  
int CDiskImage::LinkTrackData(PDISKTRACKINFO pti, int size)
{
	                           
	if (!pti || size<0)
		return imgeGeneric;

	                        
	FreeTrackData(pti);

	               
	if (size) {
		pti->trackcnt=1;
		pti->tracklen=size;
		pti->trackbuf=new UBYTE[pti->tracklen];
		pti->trackdata[0]=pti->trackbuf;
		pti->tracksize[0]=pti->tracklen;
		pti->trackstart[0]=0;

		                   
		memset(pti->trackbuf, 0, pti->tracklen);
	}

	                                                            
	pti->linked=true;
	pti->linkinfo=0;
	pti->linkflag=0;

	return imgeOk;
}

                               
int CDiskImage::LoadTrack(PDISKTRACKINFO pti, UDWORD flag)
{
	return imgeUnsupported;
}

                                     
int CDiskImage::LoadPlain(PDISKTRACKINFO pti)
{
	return imgeUnsupported;
}

                            
int CDiskImage::AllocTrack(PDISKTRACKINFO pti, UDWORD flag)
{
	                      
	if (!pti)
		return imgeGeneric;

	                     
	switch (pti->type) {
		                     
		case dtitCapsDump:
		case dtitCapsImage:
			return LoadTrack(pti, flag);

		                          
		case dtitPlain:
			return LoadPlain(pti);

		                               
		default:
			return imgeGeneric;
	}
}

                     
void CDiskImage::FreeTrack(PDISKTRACKINFO pti, int forced)
{
	                               
	if (!pti)
		return;

	                                                     
	if (pti->linked && !forced)
		return;

	                                           
	pti->linked=false;
	pti->linkinfo=0;
	pti->linkflag=0;

	               
	FreeTrackData(pti);
	FreeTrackTiming(pti);
}

                          
void CDiskImage::FreeTrackData(PDISKTRACKINFO pti)
{
	                               
	if (!pti)
		return;

	if (pti->rawdump)
		delete[] pti->trackdata[0];

	                   
	for (int trk=0; trk < CAPS_MTRS; trk++) {
		pti->trackdata[trk]=NULL;
		pti->tracksize[trk]=0;
	}

	                    
	pti->trackcnt=0;

	if (!pti->rawdump)
		delete[] pti->trackbuf;

	pti->trackbuf=NULL;
	pti->tracklen=0;

	                             
	FreeTrackFD(pti);
	FreeTrackSI(pti);
}

                           
void CDiskImage::FreeTrackTiming(PDISKTRACKINFO pti)
{
	                               
	if (!pti)
		return;

	                     
	pti->timecnt=0;
	delete [] pti->timebuf;
	pti->timebuf=NULL;

	                         
	pti->rawtimecnt = 0;
	delete[] pti->rawtimebuf;
	pti->rawtimebuf = NULL;
}

                              
void CDiskImage::FreeTrackFD(PDISKTRACKINFO pti)
{
	                               
	if (!pti)
		return;

	                     
	pti->fdpsize=0;
	pti->fdpmax=0;
	delete [] pti->fdp;
	pti->fdp=NULL;
}

                             
PDISKTRACKINFO CDiskImage::MapTrack(int cylinder, int head)
{
	                        
	if (cylinder<0 || cylinder>=MAX_CYLINDER)
		return NULL;

	                    
	if (head<0 || head>=MAX_HEAD)
		return NULL;

	                      
	PDISKTRACKINFO pt=GetTrack(cylinder, head);
	if (pt)
		return pt;

	if (!dti) {
		                   
		dtihed=MAX_HEAD;
		dticnt=((cylinder*dtihed)/DEF_TRACKINFO+1)*DEF_TRACKINFO;
		dticyl=dticnt/MAX_HEAD;
		dti=new DiskTrackInfo[dticnt];
		memset(dti, 0, sizeof(DiskTrackInfo)*dticnt);
	} else {
		               
		int scnt=dticnt;
		PDISKTRACKINFO sti=dti;
		dticnt=((cylinder*dtihed)/DEF_TRACKINFO+1)*DEF_TRACKINFO;
		dticyl=dticnt/MAX_HEAD;
		dti=new DiskTrackInfo[dticnt];
		memcpy(dti, sti, sizeof(DiskTrackInfo)*scnt);
		memset(dti+scnt, 0, sizeof(DiskTrackInfo)*(dticnt-scnt));
		delete [] sti;
	}

	return &dti[cylinder*dtihed+head];
}

                               
void CDiskImage::UpdateImageInfo(PDISKTRACKINFO pti)
{
	if (!dii.valid) {
		             
		dii.umincylinder=pti->cylinder;
		dii.umaxcylinder=pti->cylinder;
		dii.uminhead=pti->head;
		dii.umaxhead=pti->head;
		dii.valid=true;
	} else {
		                     
		if (pti->cylinder < dii.umincylinder)
			dii.umincylinder=pti->cylinder;
		if (pti->cylinder > dii.umaxcylinder)
			dii.umaxcylinder=pti->cylinder;
		if (pti->head < dii.uminhead)
			dii.uminhead=pti->head;
		if (pti->head > dii.umaxhead)
			dii.umaxhead=pti->head;
	}
}



                          
void CDiskImage::CreateDateTime(PCAPSDATETIME pcd)
{
	                  
	if (!pcd)
		return;

	                                                               
#ifdef _WIN32
	SYSTEMTIME st;
	GetLocalTime(&st);
#else
	#include <time.h>
	struct tm st_tm;
	time_t t_now = time(NULL);
	localtime_r(&t_now, &st_tm);
	struct {
		int wYear, wMonth, wDay, wHour, wMinute, wSecond, wMilliseconds;
	} st;
	st.wYear = st_tm.tm_year + 1900;
	st.wMonth = st_tm.tm_mon + 1;
	st.wDay = st_tm.tm_mday;
	st.wHour = st_tm.tm_hour;
	st.wMinute = st_tm.tm_min;
	st.wSecond = st_tm.tm_sec;
	st.wMilliseconds = 0;
#endif

	              
	UDWORD t=0;
	t+=st.wYear*10000;
	t+=st.wMonth*100;
	t+=st.wDay;
	pcd->date=t;

	              
	t=0;
	t+=st.wHour*10000000;
	t+=st.wMinute*100000;
	t+=st.wSecond*1000;
	t+=st.wMilliseconds%1000;
	pcd->time=t;
}

                          
void CDiskImage::DecodeDateTime(PCAPSDATETIMEEXT dec, PCAPSDATETIME pcd)
{
	                             
	if (!dec)
		return;

	               
	memset(dec, 0, sizeof(CapsDateTimeExt));

	                      
	if (!pcd)
		return;

	              
	UDWORD t=pcd->date;
	dec->year=t/10000;
	t%=10000;
	dec->month=t/100;
	t%=100;
	dec->day=t;

	               
	t=pcd->time;
	dec->hour=t/10000000;
	t%=10000000;
	dec->min=t/100000;
	t%=100000;
	dec->sec=t/1000;
	t%=1000;
	dec->tick=t;
}

                     
LPCSTR CDiskImage::GetPlatformName(int pid)
{
	if (pid>=cppidNA && pid<cppidLast)
		return pidname[pid];
	else
		return NULL;
}

                          
UDWORD CDiskImage::CrcFile(PCAPSFILE pcf)
{
	UDWORD crc=0;

	                            
	if (pcf->flag & CFF_MEMMAP) {
		if (!pcf->memmap || pcf->size<0)
			return crc;

		return CalcCRC(pcf->memmap, pcf->size);
	}

	            
	CCapsFile file;
	if (file.Open(pcf))
		return crc;

	int len=file.GetSize();

	if (len) {
		int bufsize=DEF_CRCBUF;
		PUBYTE buf=new UBYTE[bufsize];

		                          
		while (len) {
			int size=len > bufsize ? bufsize : len;
			if (file.Read(buf, size) != size) {
				crc=0;
				break;
			}

			crc=CalcCRC32(buf, size, crc);
			len-=size;
		}

		delete [] buf;
	}

	return crc;
}

                             
UDWORD CDiskImage::ReadValue(PUBYTE buf, int cnt)
{
	UDWORD val;

	for (val=0; cnt > 0; cnt--) {
		val<<=8;
		val|=*buf++;
	}

	return val;
}

                  
PDISKDATAMARK CDiskImage::AddFD(PDISKTRACKINFO pti, PDISKDATAMARK src, int size, int units)
{
	                           
	if (!src || size<=0)
		return NULL;

	                              
	PDISKDATAMARK alloc=AllocFD(pti, size, units);
	if (alloc)
		memcpy(alloc, src, size*sizeof(DiskDataMark));

	return alloc;
}

                       
PDISKDATAMARK CDiskImage::AllocFD(PDISKTRACKINFO pti, int size, int units)
{
	                               
	if (!pti)
		return NULL;

	                  
	if (size <= 0)
		return pti->fdp+pti->fdpsize;

	                           
	if (pti->fdpsize+size > pti->fdpmax) {
		units+=pti->fdpsize+size;
		PDISKDATAMARK fdp=pti->fdp;
		pti->fdp=new DiskDataMark[units];
		if (pti->fdpsize)
			memcpy(pti->fdp, fdp, pti->fdpsize*sizeof(DiskDataMark));
		delete [] fdp;
		pti->fdpmax=units;
	}

	                    
	PDISKDATAMARK alloc=pti->fdp+pti->fdpsize;
	memset(alloc, 0, size*sizeof(DiskDataMark));
	pti->fdpsize+=size;
	return alloc;
}

                                     
void CDiskImage::AllocTrackSI(PDISKTRACKINFO pti)
{
	                               
	if (!pti)
		return;

	FreeTrackSI(pti);

	                            
	if (pti->sectorcnt > 0) {
		int size=pti->sectorcnt;
		pti->sip=new DiskSectorInfo[size];
		pti->sipsize=size;
		memset(pti->sip, 0, size*sizeof(DiskSectorInfo));
	}
}

                                 
void CDiskImage::FreeTrackSI(PDISKTRACKINFO pti)
{
	                               
	if (!pti)
		return;

	                          
	pti->sipsize=0;
	delete [] pti->sip;
	pti->sip=NULL;
}

