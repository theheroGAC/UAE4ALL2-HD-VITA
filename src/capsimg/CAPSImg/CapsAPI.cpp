#include "stdafx.h"



using namespace std;

int api_debug_request;
CDiskEncoding dskenc;
CDiskImageFactory imgfactory;
vector <PCDISKIMAGE> img;

                    
int sizeversioninfo[]= {
	sizeof(CapsVersionInfo)
};

                  
int sizetrackinfo[]= {
	sizeof(CapsTrackInfo),
	sizeof(CapsTrackInfoT1),
	sizeof(CapsTrackInfoT2),
};



                           
SDWORD __cdecl CAPSInit()
{
	return imgeOk;
}

                          
SDWORD __cdecl CAPSExit()
{
	for (PCDISKIMAGE &actimg : img) {
		delete actimg;
		actimg = NULL;
	}

	return imgeOk;
}

                      
SDWORD __cdecl CAPSAddImage()
{
	                                                                               
	PCDISKIMAGE pi = new CDiskImage;
	unsigned pos;

	for (pos = 0; pos < img.size(); pos++) {
		if (img[pos])
			continue;

		img[pos] = pi;
		return pos;
	}

	img.push_back(pi);

	return pos;
}

                         
SDWORD __cdecl CAPSRemImage(SDWORD id)
{
	if (id<0 || (unsigned)id>=img.size())
		return -1;

	delete img[id];
	img[id]=NULL;
	return id;
}

                             
SDWORD __cdecl CAPSLockImage(SDWORD id, PCHAR name)
{
	if (id<0 || (unsigned)id >= img.size() || !img[id])
		return imgeOutOfRange;

	CapsFile cf;
	memset(&cf, 0, sizeof(CapsFile));
	cf.name=name;

	return CAPSLockImage(id, &cf);
}

                                               
SDWORD __cdecl CAPSLockImageMemory(SDWORD id, PUBYTE buffer, UDWORD length, UDWORD flag)
{
	if (id<0 || (unsigned)id >= img.size() || !img[id])
		return imgeOutOfRange;

	CapsFile cf;
	memset(&cf, 0, sizeof(CapsFile));
	cf.memmap=buffer;
	cf.size=length;
	cf.flag|=CFF_MEMMAP;
	if (flag & DI_LOCK_MEMREF)
		cf.flag|=CFF_MEMREF;

	return CAPSLockImage(id, &cf);
}

                                                          
int CAPSLockImage(int id, PCAPSFILE pcf)
{
	int res = imgeOk;

	                                               
	int type = imgfactory.GetImageType(pcf);

	                        
	switch (type) {
		                        
		case citError:
			res = imgeOpen;
			break;

		                       
		case citUnknown:
			res = imgeType;
			break;
	}

	                                   
	if (res == imgeOk) {
		PCDISKIMAGE newimg = imgfactory.CreateImage(type);

		if (newimg) {
			                                                                                    
			CAPSRemImage(id);

			                          
			img[id] = newimg;

			                                                                                         
			res = img[id]->Lock(pcf);
		} else
			res = imgeGeneric;
	}

	return res;
}

               
SDWORD __cdecl CAPSUnlockImage(SDWORD id)
{
	if (id<0 || (unsigned)id >= img.size() || !img[id])
		return imgeOutOfRange;

	int res=img[id]->Unlock();

	return res;
}

                                 
SDWORD __cdecl CAPSLoadImage(SDWORD id, UDWORD flag)
{
	if (id<0 || (unsigned)id >= img.size() || !img[id])
		return imgeOutOfRange;

	int res=img[id]->LoadImage(flag);

	return res;
}

                        
SDWORD __cdecl CAPSGetImageInfo(PCAPSIMAGEINFO pi, SDWORD id)
{
	if (!pi)
		return imgeGeneric;

	memset(pi, 0, sizeof(CapsImageInfo));

	if (id<0 || (unsigned)id >= img.size() || !img[id])
		return imgeOutOfRange;

	PDISKIMAGEINFO pd=img[id]->GetInfo();
	if (!pd)
		return imgeGeneric;

	                     
	if (pd->civalid) {
		                              
		switch (pd->ci.type) {
			case cpimtFDD:
				pi->type = ciitFDD;
				break;

			default:
				pi->type = ciitNA;
				break;
		}

		pi->release = pd->ci.release;
		pi->revision = pd->ci.revision;
		pi->mincylinder = pd->ci.mincylinder;
		pi->maxcylinder = pd->ci.maxcylinder;
		pi->minhead = pd->ci.minhead;
		pi->maxhead = pd->ci.maxhead;
		CDiskImage::DecodeDateTime(&pi->crdt, &pd->ci.crdt);

		for (int plt = 0; plt < CAPS_MAXPLATFORM; plt++)
			pi->platform[plt] = pd->ci.platform[plt];
	} else {
		                           
		if (pd->dmpcount) {
			                                                             
			pi->type = ciitFDD;
			pi->release = 0;
			pi->revision = 0;
			pi->mincylinder = pd->umincylinder;
			pi->maxcylinder = pd->umaxcylinder;
			pi->minhead = pd->uminhead;
			pi->maxhead = pd->umaxhead;
		}	else {
			                                                                                 
			pi->type = ciitNA;
		}
	}

	return imgeOk;
}

                        
SDWORD __cdecl CAPSLockTrack(PVOID ptrackinfo, SDWORD id, UDWORD cylinder, UDWORD head, UDWORD flag)
{
	if (!ptrackinfo)
		return imgeGeneric;

	                                                         
	unsigned rev=0;
	if (flag & DI_LOCK_TYPE)
		rev=*(PUDWORD)ptrackinfo;

	                                                        
	if (rev > 2) {
		*(PUDWORD)ptrackinfo=2;
		return imgeUnsupportedType;
	}

	           
	if (id < 0 || (unsigned)id >= img.size() || !img[id]) {
		                  
		memset(ptrackinfo, 0, sizetrackinfo[rev]);

		return imgeOutOfRange;
	}

	                         
	if (flag & DI_LOCK_SETWSEED) {
		PDISKTRACKINFO pt=img[id]->GetTrack(cylinder, head);
		if (pt) {
			switch (rev) {
				case 2:
					pt->wseed=((PCAPSTRACKINFOT2)ptrackinfo)->wseed;
					break;
			}
		}
	}

	                  
	memset(ptrackinfo, 0, sizetrackinfo[rev]);

	             
	PDISKTRACKINFO pt=img[id]->LockTrack(cylinder, head, flag);
	if (!pt) {
		PDISKIMAGEINFO pd=img[id]->GetInfo();

		return pd ? pd->error : imgeGeneric;
	}

	                        
	UDWORD ttype;
	switch (pt->ci.dentype) {
		case cpdenNA:
			ttype=ctitNA;
			break;

		case cpdenNoise:
			ttype=ctitNoise;
			break;

		case cpdenAuto:
			ttype=ctitAuto;
			break;

		default:
			ttype=ctitVar;
			break;
	}

	                      
	if (pt->fdpsize)
		ttype|=CTIT_FLAG_FLAKEY;

	                                               
	if (pt->rawdump)
		ttype = ctitVar;

	                                                                 
	if (pt->rawupdate)
		ttype |= CTIT_FLAG_FLAKEY;

	                            
	switch (rev) {
		case 0:
			CAPSLockTrackT0((PCAPSTRACKINFO)ptrackinfo, pt, ttype, flag);
			break;

		case 1:
			CAPSLockTrackT1((PCAPSTRACKINFOT1)ptrackinfo, pt, ttype, flag);
			break;

		case 2:
			CAPSLockTrackT2((PCAPSTRACKINFOT2)ptrackinfo, pt, ttype, flag);
			break;
	}

	return imgeOk;
}

                        
void CAPSLockTrackT0(PCAPSTRACKINFO pi, PDISKTRACKINFO pt, UDWORD ttype, UDWORD flag)
{
	pi->type=ttype;
	pi->cylinder=pt->cylinder;
	pi->head=pt->head;
	pi->sectorcnt=pt->sectorcnt;
	pi->sectorsize=0;
	pi->trackcnt=pt->trackcnt;
	pi->trackbuf=pt->trackbuf;
	pi->tracklen=(flag & DI_LOCK_TRKBIT) ? pt->trackbc : pt->tracklen;
	pi->timelen=pt->timecnt;
	pi->timebuf=pt->timebuf;

	for (int trk=0; trk < pt->trackcnt; trk++) {
		pi->trackdata[trk]=pt->trackdata[trk];
		pi->tracksize[trk]=pt->tracksize[trk];
	}
}

                        
void CAPSLockTrackT1(PCAPSTRACKINFOT1 pi, PDISKTRACKINFO pt, UDWORD ttype, UDWORD flag)
{
	pi->type=ttype;
	pi->cylinder=pt->cylinder;
	pi->head=pt->head;
	pi->sectorcnt=pt->sectorcnt;
	pi->sectorsize=0;
	pi->trackbuf=pt->trackbuf;
	pi->tracklen=(flag & DI_LOCK_TRKBIT) ? pt->trackbc : pt->tracklen;
	pi->timelen=pt->timecnt;
	pi->timebuf=pt->timebuf;
	pi->overlap=pt->overlap;
}

                        
void CAPSLockTrackT2(PCAPSTRACKINFOT2 pi, PDISKTRACKINFO pt, UDWORD ttype, UDWORD flag)
{
	pi->type=ttype;
	pi->cylinder=pt->cylinder;
	pi->head=pt->head;
	pi->sectorcnt=pt->sectorcnt;
	pi->sectorsize=0;
	pi->trackbuf=pt->trackbuf;
	pi->tracklen=(flag & DI_LOCK_TRKBIT) ? pt->trackbc : pt->tracklen;
	pi->timelen=pt->timecnt;
	pi->timebuf=pt->timebuf;
	pi->overlap=pt->overlap;
	pi->startbit=pt->startbit;
	pi->wseed=pt->wseed;
	pi->weakcnt=pt->fdpsize;
}

                          
SDWORD __cdecl CAPSUnlockTrack(SDWORD id, UDWORD cylinder, UDWORD head)
{
	if (id<0 || (unsigned)id >= img.size() || !img[id])
		return imgeOutOfRange;

	PDISKTRACKINFO pt=img[id]->UnlockTrack(cylinder, head);

	return pt ? imgeOk : imgeOutOfRange;
}

                               
SDWORD __cdecl CAPSUnlockAllTracks(SDWORD id)
{
	if (id<0 || (unsigned)id >= img.size() || !img[id])
		return imgeOutOfRange;

	img[id]->UnlockTrack();

	return imgeOk;
}

                    
PCHAR  __cdecl CAPSGetPlatformName(UDWORD pid)
{
	return (PCHAR)CDiskImage::GetPlatformName(pid);
}

                      
SDWORD __cdecl CAPSGetVersionInfo(PVOID pversioninfo, UDWORD flag)
{
	if (!pversioninfo)
		return imgeGeneric;

	                                                         
	unsigned rev=0;
	if (flag & DI_LOCK_TYPE)
		rev=*(PUDWORD)pversioninfo;

	                                                        
	if (rev > 0) {
		*(PUDWORD)pversioninfo=0;
		return imgeUnsupportedType;
	}

	                  
	memset(pversioninfo, 0, sizeversioninfo[rev]);

	                            
	switch (rev) {
		case 0:
			CAPSGetVersionInfoT0((PCAPSVERSIONINFO)pversioninfo);
			break;
	}

	return imgeOk;
}

                             
void CAPSGetVersionInfoT0(PCAPSVERSIONINFO pi)
{
	pi->release=CAPS_LIB_RELEASE;
	pi->revision=CAPS_LIB_REVISION;
	pi->flag=DI_LOCK_INDEX|
		DI_LOCK_ALIGN |
		DI_LOCK_DENVAR |
		DI_LOCK_DENAUTO |
		DI_LOCK_DENNOISE |
		DI_LOCK_NOISE |
		DI_LOCK_NOISEREV |
		DI_LOCK_MEMREF |
		DI_LOCK_UPDATEFD |
		DI_LOCK_TYPE |
		DI_LOCK_DENALT |
		DI_LOCK_OVLBIT |
		DI_LOCK_TRKBIT |
		DI_LOCK_NOUPDATE |
		DI_LOCK_SETWSEED;
}

                                         
SDWORD __cdecl CAPSGetInfo(PVOID pinfo, SDWORD id, UDWORD cylinder, UDWORD head, UDWORD inftype, UDWORD infid)
{
	if (!pinfo)
		return imgeGeneric;

	if (id<0 || (unsigned)id >= img.size() || !img[id])
		return imgeOutOfRange;

	PDISKIMAGEINFO pd = img[id]->GetInfo();
	PDISKTRACKINFO pt = img[id]->GetTrack(cylinder, head);

	int res=imgeUnsupportedType;

	switch (inftype) {
		case cgiitSector:
			res=CAPSGetSectorInfo((PCAPSSECTORINFO)pinfo, pd, pt, infid);
			break;

		case cgiitWeak:
			res=CAPSGetWeakInfo((PCAPSDATAINFO)pinfo, pd, pt, infid);
			break;

		case cgiitRevolution:
			res = CAPSGetRevolutionInfo((PCAPSREVOLUTIONINFO)pinfo, pd, pt, infid);
			break;
	}

	return res;
}

                         
int CAPSGetSectorInfo(PCAPSSECTORINFO pi, PDISKIMAGEINFO pd, PDISKTRACKINFO pt, UDWORD infid)
{
	memset(pi, 0, sizeof(CapsSectorInfo));

	if (!pt)
		return imgeOutOfRange;

	if (pt->sipsize <= 0 || !pt->sip)
		return imgeOutOfRange;

	if (infid >= (unsigned)pt->sipsize)
		return imgeOutOfRange;

	PDISKSECTORINFO si=pt->sip+infid;

	pi->descdatasize=si->descdatasize;
	pi->descgapsize=si->descgapsize;
	pi->datasize=si->datasize;
	pi->gapsize=si->gapsize;
	pi->datastart=si->datastart;
	pi->gapstart=si->gapstart;
	pi->gapsizews0=si->gapsizews0;
	pi->gapsizews1=si->gapsizews1;
	pi->gapws0mode=si->gapws0mode;
	pi->gapws1mode=si->gapws1mode;
	pi->celltype=si->celltype;
	pi->enctype=si->enctype;

	return imgeOk;
}

                            
int CAPSGetWeakInfo(PCAPSDATAINFO pi, PDISKIMAGEINFO pd, PDISKTRACKINFO pt, UDWORD infid)
{
	memset(pi, 0, sizeof(CapsDataInfo));

	if (!pt)
		return imgeOutOfRange;

	if (pt->fdpsize <= 0 || !pt->fdp)
		return imgeOutOfRange;

	if (infid >= (unsigned)pt->fdpsize)
		return imgeOutOfRange;

	PDISKDATAMARK dm=pt->fdp+infid;

	pi->type=cditWeak;
	pi->start=dm->position;
	pi->size=dm->size;

	return imgeOk;
}

                             
int CAPSGetRevolutionInfo(PCAPSREVOLUTIONINFO pi, PDISKIMAGEINFO pd, PDISKTRACKINFO pt, UDWORD infid)
{
	memset(pi, 0, sizeof(CapsRevolutionInfo));

	                                   
	if (pd) {
		                                                    
		pi->next = pd->nextrev;
		pi->last = pd->lastrev;
		pi->real = pd->realrev;
	}

	                                   
	if (pt) {
		                                                                                 
		pi->max = (pt->rawdump) ? pt->rawtrackcnt : -1;
	}

	return imgeOk;
}

                                                   
SDWORD __cdecl CAPSSetRevolution(SDWORD id, UDWORD value)
{
	if (id<0 || (unsigned)id >= img.size() || !img[id])
		return imgeOutOfRange;

	PDISKIMAGEINFO pd = img[id]->GetInfo();
	if (!pd)
		return imgeGeneric;

	                 
	pd->nextrev = value;

	return imgeOk;
}

                                 
SDWORD __cdecl CAPSGetImageType(PCHAR name)
{
	CapsFile cf;
	memset(&cf, 0, sizeof(CapsFile));
	cf.name = name;

	                                               
	return imgfactory.GetImageType(&cf);
}

                                                   
SDWORD __cdecl CAPSGetImageTypeMemory(PUBYTE buffer, UDWORD length)
{
	CapsFile cf;
	memset(&cf, 0, sizeof(CapsFile));
	cf.memmap = buffer;
	cf.size = length;
	cf.flag |= CFF_MEMMAP | CFF_MEMREF;

	                                               
	return imgfactory.GetImageType(&cf);
}

                                                                                     
SDWORD __cdecl CAPSGetDebugRequest()
{
	SDWORD res = api_debug_request;
	api_debug_request = 0;

	return res;
}
