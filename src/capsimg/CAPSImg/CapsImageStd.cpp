#include "stdafx.h"



using namespace std;

CCapsImageStd::CCapsImageStd()
{
	InitSystem();
	Clear();
}

CCapsImageStd::~CCapsImageStd()
{
	FreeDecoder();
}

                     
int CCapsImageStd::Lock(PCAPSFILE pcf)
{
	                     
	Unlock();

	              
	switch (loader.Lock(pcf)) {
		                  
		case CCapsLoader::ccidErrFile:
			return imgeOpen;

		                  
		case CCapsLoader::ccidErrType:
			return imgeType;

		                 
		case CCapsLoader::ccidCaps:
			break;

		                  
		default:
			return imgeGeneric;
	}

	                  
	dii.readonly=(pcf->flag & CFF_WRITE) ? 0 : 1;

	                                  
	int res=ScanImage();
	if (res != imgeOk)
		return res;

	return CheckEncoder(dii.ci.encoder, dii.ci.encrev);
}

               
int CCapsImageStd::Unlock()
{
	Destroy();
	loader.Unlock();
	return imgeOk;
}

                                    
int CCapsImageStd::LoadTrack(PDISKTRACKINFO pti, UDWORD flag)
{
	                 
	di.pdt=pti;
	di.flag=flag;

	int res = imgeOk;

	                                              
	if (pti->trackcnt) {
		switch (pti->type) {
			case dtitCapsDump:
				res = UpdateDump();
				break;

			case dtitCapsImage:
				res = UpdateImage(0);
				break;

			default:
				res = imgeGeneric;
				break;
		}

		return res;
	}

	                                             
	if (pti->datasize) {
		                               
		loader.SetPosition(pti->datapos);
		if (loader.ReadChunk() == CCapsLoader::ccidData) {
			                                 
			AllocDiskData(pti->datasize);
			di.datacount=pti->datasize;

			                       
			if (loader.ReadData(di.data) == pti->datasize) {
				switch (pti->type) {
					case dtitCapsDump:
						res=DecompressDump();
						break;

					case dtitCapsImage:
						res=DecodeImage();
						break;

					default:
						res=imgeGeneric;
						break;
				}
			} else
				res=imgeGeneric;
		} else
			res=imgeGeneric;
	} else {
		                
		switch (pti->type) {
			case dtitCapsImage:
				di.datacount=0;
				res=DecodeImage();
				break;

			default:
				res=imgeGeneric;
				break;
		}
	}

	return res;
}

                                               
int CCapsImageStd::ScanImage()
{
	vector <ScanInfo> si;
	si.reserve(DEF_SCANINFO);
	ScanInfo empty_si = { 0, 0 };
	DiskTrackInfo dt;

	                        
	for (int run = 1; run;) {
		int pos = loader.GetPosition();
		int type = loader.ReadChunk();
		PCAPSCHUNK pc = loader.GetChunk();
		int did = 0;

		switch (type) {
			              
			case CCapsLoader::ccidEof:
				run = 0;
				continue;

			                         
			case CCapsLoader::ccidData:
				did = pc->cg.mod.data.did;
				if ((unsigned)did >= si.size())
					si.resize(did + 1, empty_si);
				si[did].data = pos;
				loader.SkipData();
				break;

			                                 
			case CCapsLoader::ccidTrck:
				did = pc->cg.mod.trck.did;
				if ((unsigned)did >= si.size())
					si.resize(did + 1, empty_si);
				si[did].track = pos;
				break;

			                            
			case CCapsLoader::ccidErrFile:
			case CCapsLoader::ccidErrType:
			case CCapsLoader::ccidErrShort:
			case CCapsLoader::ccidErrHeader:
			case CCapsLoader::ccidErrData:
				return imgeGeneric;

			             
			case CCapsLoader::ccidInfo:
				dii.ci = pc->cg.mod.info;
				dii.civalid = 1;
				continue;

			                                  
			case CCapsLoader::ccidImge:
				did = pc->cg.mod.imge.did;
				if ((unsigned)did >= si.size())
					si.resize(did + 1, empty_si);
				si[did].track = pos;
				break;

			                    
			default:
				continue;
		}

		                       
		if (did > dii.didmax)
			dii.didmax = did;

		                                                          
		if (did && (unsigned)did < si.size()) {
			if (si[did].track && si[did].data) {
				                   
				memset(&dt, 0, sizeof(dt));

				                      
				int actpos = loader.GetPosition();

				                    
				loader.SetPosition(si[did].track);
				type = loader.ReadChunk();
				pc = loader.GetChunk();

				                    
				switch (type) {
					case CCapsLoader::ccidTrck:
						dt.type = dtitCapsDump;
						dt.cylinder = pc->cg.mod.trck.cyl;
						dt.head = pc->cg.mod.trck.head;
						dii.dmpcount++;
						break;

					case CCapsLoader::ccidImge:
						dt.type = dtitCapsImage;
						dt.cylinder = pc->cg.mod.imge.cylinder;
						dt.head = pc->cg.mod.imge.head;
						dt.sectorcnt = pc->cg.mod.imge.blkcnt;
						dt.ci = pc->cg.mod.imge;
						dii.relcount++;
						break;

					default:
						return imgeGeneric;
				}

				                          
				dt.headerpos = si[did].track;
				dt.datapos = si[did].data;

				                   
				loader.SetPosition(si[did].data);
				loader.ReadChunk();
				dt.datasize = loader.GetDataSize();

				             
				int res = AddTrack(&dt);
				if (res != imgeOk)
					return res;

				                               
				dii.modimage = 1;

				                         
				loader.SetPosition(actpos);
			}
		}
	}

	return imgeOk;
}

                                                         
int CCapsImageStd::UpdateImage(int group)
{
	int res=imgeOk;

	                               
	di.track=di.pdt->trackbuf;
	di.trackbc=di.pdt->trackbc;
	di.singletrackbc=di.pdt->singletrackbc;
	trackbuf.InitBitSize(di.track, di.trackbc);
	
	                                  
	if (!di.track || !di.trackbc || !di.singletrackbc)
		return imgeOk;

	                                                  
	if (di.flag & DI_LOCK_NOUPDATE)
		return imgeOk;

	                        
	if (di.pdt->fdpsize && (di.flag & DI_LOCK_UPDATEFD))
		res=UpdateWeakBit(group);

	return res;
}

                                    
int CCapsImageStd::UpdateWeakBit(int group)
{
	                                                  
	if (di.flag & DI_LOCK_NOUPDATE)
		return imgeOk;

	                  
	uint32_t seed=di.pdt->wseed;

	                          
	for (int pos=0; pos < di.pdt->fdpsize; pos++) {
		PDISKDATAMARK pd=di.pdt->fdp+pos;

		                                                
		if (pd->group != group)
			continue;

		uint32_t bitpos=pd->position;
		int bitcnt=pd->size;

		                       
		while (bitcnt > 0) {
			                          
			seed<<=1;
			if ((seed>>22 ^ seed) & DF_1)
				seed++;

			                                  
			int writebc=(bitcnt >= maxwritelen) ? maxwritelen : bitcnt;

			                 
			trackbuf.WriteBitWrap(bitpos, seed, writebc);

			                      
			bitcnt-=writebc;
			bitpos+=writebc;

			                                                  
			if (bitpos >= di.trackbc)
				bitpos-=di.trackbc;
		}
	}

	                                                                               
	di.pdt->wseed=seed;

	return imgeOk;
}

                                            
void CCapsImageStd::UpdateOverlap()
{
	                                                  
	if (di.flag & DI_LOCK_NOUPDATE)
		return;

	                            
	int ovlsize=CIMG_OVERLAPBIT;
	if (ovlsize <= 0)
		return;

	                                            
	if (di.pdt->overlapbit < 0)
		return;

	uint32_t bitpos=di.pdt->overlapbit;

	                                                       
	for (int trk=0; trk < di.pdt->trackcnt; trk++) {
		uint32_t value=trackbuf.ReadBitWrap(bitpos, ovlsize);
		value=~value;
		trackbuf.WriteBitWrap(bitpos, value, ovlsize);

		             
		bitpos+=di.singletrackbc;

		                                                  
		if (bitpos >= di.trackbc)
			bitpos-=di.trackbc;
	}
}

                
int CCapsImageStd::DecodeImage()
{
	                                                 
	int res=InitDecoder();

	                              
	if (res == imgeOk) {
		switch (dii.ci.encoder) {
			case CAPS_ENCODER:
			case SPS_ENCODER:
				res=ProcessImage();
				break;

			default:
				res=imgeIncompatible;
				break;
		}
	}

	                                                             
	if (res != imgeOk) {
		FreeTrack(di.pdt, 1);
		di.pdt->type=dtitError;
	}

	return res;
}

                                         
int CCapsImageStd::ProcessImage()
{
	                                                   
	int res=CheckEncoder(dii.ci.encoder, dii.ci.encrev);
	if (res != imgeOk)
		return res;

	PDISKTRACKINFO pti=di.pdt;

	                     
	if (pti->ci.dentype<=cpdenNA || pti->ci.dentype>=cpdenLast)
		return imgeIncompatible;

	                         
	if (pti->ci.sigtype<=cpsigNA || pti->ci.sigtype>=cpsigLast)
		return imgeIncompatible;

	                
	if (pti->ci.process)
		return imgeIncompatible;

	                   
	FreeTrack(pti, 1);

	                                         
	if (di.flag & DI_LOCK_COMP)
		return CompareImage();

	                                                      
	switch (pti->ci.dentype) {
		case cpdenCLAmiga:
		case cpdenCLAmiga2:
		case cpdenCLST:
		case cpdenSLAmiga:
		case cpdenSLAmiga2:
		case cpdenABAmiga:
		case cpdenABAmiga2:
			di.flag&=~DI_LOCK_INDEX;
			break;
	}

	                                 
	uint32_t trackbits=di.dsctrackbc;

	                                                           
	int trackcnt=(di.flag & DI_LOCK_ANA) ? 5 : 1;

	                                   
	if (pti->ci.dentype == cpdenNoise) {
		                                                
		if (di.flag & DI_LOCK_NOISEREV)
			trackcnt=2;

		                                                                                            
		if (!(di.flag & DI_LOCK_NOISE))
			trackcnt=0;
		else
			if (!trackbits)
				trackbits=100000;
	}

	                                                           
	if ((pti->ci.flag & CAPS_IF_FLAKEY) && !(di.flag & DI_LOCK_UPDATEFD))
		trackcnt=5;

	                                         
	if (di.flag & DI_LOCK_ALIGN) {
		if (trackbits & 15)
			trackbits+=16-(trackbits & 15);
	}

	                          
	if (!(di.flag & DI_LOCK_TRKBIT)) {
		if (trackbits & 7)
			trackbits+=8-(trackbits & 7);
	}

	                      
	uint32_t alltrackbits=trackbits*trackcnt;

	                                                
	int bufsize=trackbuf.CalculateByteSize(alltrackbits);
	uint8_t *tbuf;

	                                  
	if (bufsize) {
		tbuf=new uint8_t[bufsize];
		memset(tbuf, 0, bufsize);
	} else
		tbuf=NULL;

	                                                 
	uint32_t startbit=di.dscstartbit;
	if (trackbits)
		startbit=startbit % trackbits;

	                                                  
	if (di.flag & DI_LOCK_INDEX)
		startbit=0;

	                   
	pti->trackcnt=trackcnt;
	pti->trackbuf=tbuf;
	pti->tracklen=bufsize;
	pti->sdpos=startbit >> 3;
	pti->overlap=-1;
	pti->overlapbit=-1;
	pti->trackbc=alltrackbits;
	pti->singletrackbc=trackbits;
	pti->startbit=startbit;

	                    
	pti->wseed=0x87654321;

	                                
	AllocTrackSI(pti);

	                               
	di.track=tbuf;
	di.trackbc=alltrackbits;
	di.singletrackbc=trackbits;
	trackbuf.InitBitSize(di.track, di.trackbc);

	                        
	uint32_t trackdiff=trackbits-di.dsctrackbc;

	                                   
	int gsvalid=0;
	uint32_t gspos=0;

	                          
	uint32_t trksize=0, actpos=startbit;
	for (int trk=0; trk < trackcnt; trk++) {
		                                                    
		uint32_t ofsact=trksize >> 3;
		pti->trackdata[trk]=tbuf+ofsact;
		pti->trackstart[trk]=ofsact;
		trksize+=trackbits;
		uint32_t ofsnext=trksize >> 3;
		pti->tracksize[trk]=ofsnext-ofsact;

		                           
		for (int blk=0; blk < di.blockcount; blk++) {
			uint32_t datasize=di.block[blk].blockbits;
			uint32_t gapsize=di.block[blk].gapbits;

			                                         
			if (blk == di.blockcount-1) {
				                                                    
				if (!gapsize && trackdiff)
					return imgeGeneric;

				gapsize+=trackdiff;
			}

			              
			res=ProcessBlock(blk, actpos, datasize, gapsize);
			if (res != imgeOk)
				return res;

			                                                 
			if (!trk && (blk == di.blockcount-1) && di.encgsvalid) {
				gsvalid=1;
				gspos=di.encgapsplit;
			}

			                                                                       
			actpos+=datasize+gapsize;
			if (actpos >= di.trackbc)
				actpos-=di.trackbc;
		}

		                   
		MFMFixup();
	}

	                                                  
	if (actpos != startbit)
		return imgeGeneric;

	                             
	if (gsvalid) {
		uint32_t ofs=gspos % trackbits;
		pti->overlapbit=ofs;

		if (di.flag & DI_LOCK_OVLBIT)
			pti->overlap=ofs;
		else
			pti->overlap=ofs >> 3;
	}

	                     
	if (pti->ci.dentype==cpdenNoise && (di.flag & DI_LOCK_NOISE))
		GenerateNoiseTrack(pti);

	                     
	if ((res=DecodeDensity(pti, di.data, di.flag)) != imgeOk)
		return res;

	                                      
	UpdateOverlap();

	                                                          
	res=UpdateImage(0);

	return res;
}

                                             
int CCapsImageStd::CompareImage()
{
	return imgeUnsupported;
}

                                               
int CCapsImageStd::DecompressDump()
{
	return imgeUnsupported;
}

                                                        
int CCapsImageStd::UpdateDump()
{
	return imgeUnsupported;
}



                        
int CCapsImageStd::DecodeDensity(PDISKTRACKINFO pti, PUBYTE buf, UDWORD flag)
{
	                   
	switch (pti->ci.dentype) {
		case cpdenNoise:
			if (flag & DI_LOCK_DENNOISE)
				GenerateNoiseDensity(pti);
			break;

		case cpdenAuto:
			if (flag & DI_LOCK_DENAUTO)
				GenerateAutoDensity(pti);
			break;

		case cpdenCLAmiga:
			if (flag & DI_LOCK_DENVAR)
				GenerateCLA(pti, buf);
			break;

		case cpdenCLAmiga2:
			if (flag & DI_LOCK_DENVAR)
				GenerateCLA2(pti, buf);
			break;

		case cpdenCLST:
			if (flag & DI_LOCK_DENVAR)
				GenerateCLST(pti, buf);
			break;

		case cpdenSLAmiga:
			if (flag & DI_LOCK_DENVAR)
				GenerateSLA(pti, buf);
			break;

		case cpdenSLAmiga2:
			if (flag & DI_LOCK_DENVAR)
				GenerateSLA2(pti, buf);
			break;

		case cpdenABAmiga:
			if (flag & DI_LOCK_DENVAR)
				GenerateABA(pti, buf);
			break;

		case cpdenABAmiga2:
			if (flag & DI_LOCK_DENVAR)
				GenerateABA2(pti, buf);
			break;
	}

	                         
	if (flag & DI_LOCK_DENALT)
		ConvertDensity(pti);

	return imgeOk;
}

                                         
int CCapsImageStd::ConvertDensity(PDISKTRACKINFO pti)
{
	                                 
	if (!pti->timebuf || !pti->timecnt)
		return imgeOk;

	                
	UDWORD sum=0;
	for (int pos=0; pos < pti->timecnt; pos++) {
		sum+=pti->timebuf[pos];
		pti->timebuf[pos]=sum;
	}

	return imgeOk;
}

                                  
int CCapsImageStd::GenerateNoiseTrack(PDISKTRACKINFO pti)
{
	UDWORD val=CIMG_MFMNOISE;

	for (int pos=0; pos < pti->tracklen; pos++) {
		pti->trackbuf[pos]=(UBYTE)val;
		val=val<<8|val>>24;
	}

	return imgeOk;
}

                                    
int CCapsImageStd::GenerateNoiseDensity(PDISKTRACKINFO pti)
{
	                            
	if (pti->tracklen && pti->trackcnt)
		pti->timecnt=pti->tracklen/pti->trackcnt;
	else
		if (!(pti->timecnt=pti->ci.trksize))
			pti->timecnt=12500;

	pti->timebuf=new UDWORD[pti->timecnt+1];

	int pos;
	for (pos=0; pos < pti->timecnt; pos++) {
		UDWORD val=1000;

		if (pos & 512)
			val+=(pos % 99)+(pos&31);
		else
			val-=(pos % 121)-(pos&31);

		pti->timebuf[pos]=val;
	}

	pti->timebuf[pos]=0;

	return imgeOk;
}

                             
int CCapsImageStd::GenerateAutoDensity(PDISKTRACKINFO pti)
{
	                            
	if (pti->tracklen && pti->trackcnt)
		pti->timecnt=pti->tracklen/pti->trackcnt;
	else
		if (!(pti->timecnt=pti->ci.trksize))
			pti->timecnt=12500;

	pti->timebuf=new UDWORD[pti->timecnt+1];

	int pos;
	for (pos=0; pos < pti->timecnt; pos++) {
		pti->timebuf[pos]=1000;
	}

	pti->timebuf[pos]=0;

	return imgeOk;
}

                                       
int CCapsImageStd::GenerateCLA(PDISKTRACKINFO pti, PUBYTE buf)
{
	                   
	GenerateAutoDensity(pti);

	                          
	int lg = trackbuf.CalculateByteSize(di.block[3].gapbits);
	int pos=pti->sdpos, cp;

	for (unsigned blk=0; blk < pti->ci.blkcnt; blk++) {
		int bs = trackbuf.CalculateByteSize(di.block[blk].blockbits);
		int gs = trackbuf.CalculateByteSize(di.block[blk].gapbits);
		int xs=bs+gs;

		switch (blk) {
			case 4:
				for (cp=0-lg; cp < bs; cp++) {
					pti->timebuf[pos+cp]-=55;
				}
				break;

			case 5:
				for (cp=0-lg; cp < bs; cp++) {
					pti->timebuf[pos+cp]-=5;
				}
				break;

			case 6:
				for (cp=0-lg; cp < bs; cp++) {
					pti->timebuf[pos+cp]+=45;
				}
				break;
		}

		pos+=xs;
		if (pos >= pti->timecnt)
			pos-=pti->timecnt;
		lg=gs;
	}

	return imgeOk;
}

                                            
int CCapsImageStd::GenerateCLA2(PDISKTRACKINFO pti, PUBYTE buf)
{
	                   
	GenerateAutoDensity(pti);

	                          
	int lg = trackbuf.CalculateByteSize(di.block[di.blockcount - 1].gapbits);
	int pos=pti->sdpos, cp;

	for (unsigned blk=0; blk < pti->ci.blkcnt; blk++) {
		int bs = trackbuf.CalculateByteSize(di.block[blk].blockbits);
		int gs = trackbuf.CalculateByteSize(di.block[blk].gapbits);
		int xs=bs+gs;

		switch (blk) {
			case 0:
				for (cp=0-lg; cp < bs; cp++) {
					pti->timebuf[pos+cp]-=55;
				}
				break;

			case 1:
				for (cp=0-lg; cp < bs; cp++) {
					pti->timebuf[pos+cp]-=5;
				}
				break;

			case 2:
				for (cp=0-lg; cp < bs; cp++) {
					pti->timebuf[pos+cp]+=45;
				}
				break;
		}

		pos+=xs;
		if (pos >= pti->timecnt)
			pos-=pti->timecnt;
		lg=gs;
	}

	return imgeOk;
}

                                    
int CCapsImageStd::GenerateCLST(PDISKTRACKINFO pti, PUBYTE buf)
{
	                   
	GenerateAutoDensity(pti);

	                          
	int pos=pti->sdpos, cp;

	for (unsigned blk=0; blk < pti->ci.blkcnt; blk++) {
		int bs = trackbuf.CalculateByteSize(di.block[blk].blockbits);
		int gs = trackbuf.CalculateByteSize(di.block[blk].gapbits);
		int xs=bs+gs;

		switch (blk) {
			case 5:
				for (cp=0; cp < bs; cp++) {
					pti->timebuf[pos+cp]+=50;
				}
				break;
		}

		pos+=xs;
		if (pos >= pti->timecnt)
			pos-=pti->timecnt;
	}

	return imgeOk;
}

                                        
int CCapsImageStd::GenerateSLA(PDISKTRACKINFO pti, PUBYTE buf)
{
	                   
	GenerateAutoDensity(pti);

	                          
	int pos=pti->sdpos, cp;

	for (unsigned blk=0; blk < pti->ci.blkcnt; blk++) {
		int bs = trackbuf.CalculateByteSize(di.block[blk].blockbits);
		int gs = trackbuf.CalculateByteSize(di.block[blk].gapbits);
		int xs=bs+gs;

		switch (blk) {
			case 1:
				for (cp=0; cp < bs; cp++) {
					pti->timebuf[pos+cp]+=100;
				}
				break;

			case 2:
				for (cp=0; cp < bs; cp++) {
					pti->timebuf[pos+cp]-=100;
				}
				break;
		}

		pos+=xs;
		if (pos >= pti->timecnt)
			pos-=pti->timecnt;
	}

	return imgeOk;
}

                                             
int CCapsImageStd::GenerateSLA2(PDISKTRACKINFO pti, PUBYTE buf)
{
	                   
	GenerateAutoDensity(pti);

	                          
	int pos=pti->sdpos, cp;

	for (unsigned blk=0; blk < pti->ci.blkcnt; blk++) {
		int bs = trackbuf.CalculateByteSize(di.block[blk].blockbits);
		int gs = trackbuf.CalculateByteSize(di.block[blk].gapbits);
		int xs=bs+gs;

		switch (blk) {
			case 1:
				for (cp=0; cp < bs; cp++) {
					pti->timebuf[pos+cp]+=50;
				}
				break;
		}

		pos+=xs;
		if (pos >= pti->timecnt)
			pos-=pti->timecnt;
	}

	return imgeOk;
}

                                            
int CCapsImageStd::GenerateABA(PDISKTRACKINFO pti, PUBYTE buf)
{
	                   
	GenerateAutoDensity(pti);

	                          
	int pos=pti->sdpos, cp;

	for (unsigned blk=0; blk < pti->ci.blkcnt; blk++) {
		int bs = trackbuf.CalculateByteSize(di.block[blk].blockbits);
		int gs = trackbuf.CalculateByteSize(di.block[blk].gapbits);
		int xs=bs+gs;

		switch (blk) {
			case 1:
				for (cp=0; cp < bs; cp++) {
					pti->timebuf[pos+cp]+=100;
				}
				break;

			case 2:
				for (cp=0; cp < bs; cp++) {
					pti->timebuf[pos+cp]+=50;
				}
				break;

			case 4:
				for (cp=0; cp < bs; cp++) {
					pti->timebuf[pos+cp]-=50;
				}
				break;

			case 5:
				for (cp=0; cp < bs; cp++) {
					pti->timebuf[pos+cp]-=100;
				}
				break;

			case 6:
				for (cp=0; cp < bs; cp++) {
					pti->timebuf[pos+cp]-=150;
				}
				break;
		}

		pos+=xs;
		if (pos >= pti->timecnt)
			pos-=pti->timecnt;
	}

	return imgeOk;
}

                                                         
int CCapsImageStd::GenerateABA2(PDISKTRACKINFO pti, PUBYTE buf)
{
	                   
	GenerateAutoDensity(pti);

	                          
	int pos=pti->sdpos, cp;
	UDWORD key=0;
	UDWORD mask=1;

	for (unsigned blk=0; blk < pti->ci.blkcnt; blk++) {
		int bs = trackbuf.CalculateByteSize(di.block[blk].blockbits);
		int gs = trackbuf.CalculateByteSize(di.block[blk].gapbits);
		int xs=bs+gs;

		if (!blk) {
			key=ReadValue(buf+blk*sizeof(CapsBlock)+offsetof(CapsBlock, gapvalue), sizeof(UDWORD));
		} else {
			int dns=(key & mask) ? -50 : 50;
			mask<<=1;

			for (cp=0; cp < bs; cp++) {
				int val=pti->timebuf[pos+cp];
				val+=dns;
				pti->timebuf[pos+cp]=val;
			}
		}

		pos+=xs;
		if (pos >= pti->timecnt)
			pos-=pti->timecnt;
	}

	return imgeOk;
}



                                    
void CCapsImageStd::InitSystem()
{
	                                      
	maxwritelen = MAX_BITBUFFER_LEN;

	                                                                                 
	rawreadlen=MAX_STREAMBIT;

	                                 
	CDiskEncoding::InitMFM(0x10000);

	                                                                   
	mfmreadlen=CDiskEncoding::mfmcodebit;

	                                
	mfmindexmask=(1 << mfmreadlen)-1;

	                                                                                  
	                                                     
	                                 
	mfmmsbclear=(1 << ((mfmreadlen*2)-1))-1;
}

                  
void CCapsImageStd::Clear()
{
	di.pdt=NULL;
	di.data=NULL;
	di.block=NULL;
	di.flag=0;
	di.track=NULL;
	di.trackbc=0;
	di.singletrackbc=0;
	di.dsctrackbc=0;
	di.dscdatabc=0;
	di.dscgapbc=0;
	di.dscstartbit=0;
	di.encbitpos=0;
	di.encwritebc=0;
	di.encgsvalid=0;
	di.encgapsplit=0;

	trackbuf.InitBitSize(di.track, di.trackbc);
	FreeDecoder();
}

                            
void CCapsImageStd::AllocDiskData(int maxsize)
{
	if (di.datasize < maxsize) {
		FreeDiskData();

		di.data=new uint8_t[maxsize];
		di.datasize=maxsize;
	}
}

                              
void CCapsImageStd::AllocImageBlock(int maxsize)
{
	if (di.blocksize < maxsize) {
		FreeImageBlock();

		di.block=new ImageBlockInfo[maxsize];
		di.blocksize=maxsize;
	}
}

                        
void CCapsImageStd::FreeDiskData()
{
	di.datacount=0;
	di.datasize=0;

	delete [] di.data;
	di.data=NULL;
}

                          
void CCapsImageStd::FreeImageBlock()
{
	di.blockcount=0;
	di.blocksize=0;

	delete [] di.block;
	di.block=NULL;
}

                    
void CCapsImageStd::FreeDecoder()
{
	FreeDiskData();
	FreeImageBlock();
}



                              
int CCapsImageStd::CheckEncoder(int encoder, int revision)
{
	if (!dii.civalid)
		return imgeOk;

	int err=0;

	switch (encoder) {
		case CAPS_ENCODER:
			if (revision < 1 || revision > CAPS_ENCODER_REV)
				err=1;
			break;

		case SPS_ENCODER:
			if (revision < 1 || revision > SPS_ENCODER_REV)
				err=1;
			break;

		default:
			err=1;
			break;
	}

	return err ? imgeIncompatible : imgeOk;
}

                                                             
int CCapsImageStd::GetBlock(PIMAGEBLOCKINFO pi, int blk)
{
	               
	if (!pi)
		return imgeGeneric;

	                                              
	CapsBlock cb;
	int res=GetBlock(&cb, blk);
	if (res != imgeOk)
		return res;

	                       
	pi->blockbits=cb.blockbits;
	pi->gapbits=cb.gapbits;
	pi->enctype=cb.enctype;
	pi->flag=cb.flag;
	pi->gapvalue=cb.gapvalue;
	pi->dataoffset=cb.dataoffset;

	                                                            
	if (dii.ci.encoder == CAPS_ENCODER) {
		pi->gapoffset=0;
		pi->celltype=cpbct2us;
		pi->flag=0;
	} else {
		pi->gapoffset=cb.bt.sps.gapoffset;
		pi->celltype=cb.bt.sps.celltype;
	}

	                             
	pi->fdenc=ibieNA;
	pi->fdbitpos=0;

	return imgeOk;
}

                       
int CCapsImageStd::GetBlock(PCAPSBLOCK pb, int blk)
{
	               
	if (!di.data || !pb || blk<0 || blk>=di.blockcount)
		return imgeGeneric;

	                       
	if (int((blk+1)*sizeof(CapsBlock)) > di.pdt->datasize)
		return imgeShort;

	                                                                
	memcpy(pb, di.data+blk*sizeof(CapsBlock), sizeof(CapsBlock));
	CCapsLoader::Swap((PUDWORD)pb, sizeof(CapsBlock));

	return imgeOk;
}

                          
int CCapsImageStd::InitDecoder()
{
	                                            
	if (!dii.civalid)
		return imgeIncompatible;

	                                        
	AllocImageBlock(di.pdt->ci.blkcnt);
	di.blockcount=di.pdt->ci.blkcnt;

	int blk;

	                                               
	for (blk=0; blk < di.blockcount; blk++) {
		int res=GetBlock(di.block+blk, blk);
		if (res != imgeOk)
			return res;
	}

	                   
	di.dsctrackbc=0;
	di.dscdatabc=0;
	di.dscgapbc=0;
	di.dscstartbit=0;

	                               
	for (blk=0; blk < di.blockcount; blk++) {
		PIMAGEBLOCKINFO pi=di.block+blk;

		                                        
		if (pi->gapbits < 8)
			pi->gapbits=0;

		di.dscdatabc+=pi->blockbits;
		di.dscgapbc+=pi->gapbits;
	}

	             
	di.dsctrackbc=di.dscdatabc+di.dscgapbc;

	                                                   
	if (di.dsctrackbc)
		di.dscstartbit=di.pdt->ci.startbit % di.dsctrackbc;

	return imgeOk;
}

                             
int CCapsImageStd::InitStream(PIMAGESTREAMINFO pstr, int strtype, int blk)
{
	               
	if (!pstr || blk<0 || blk>=di.blockcount)
		return imgeGeneric;

	                                                                   
	pstr->strtype=strtype;
	pstr->actblock=blk;
	pstr->enctype=di.block[blk].enctype;

	                                                                           
	pstr->actenctype=pstr->enctype;

	int res;

	switch (strtype) {
		case isitData:
			res=InitDataStream(pstr);
			break;

		case isitGap0:
		case isitGap1:
			res=InitGapStream(pstr);
			break;

		default:
			res=imgeGeneric;
			break;
	}

	                                          
	if (res != imgeOk)
		return res;

	                               
	res=ResetStream(pstr);

	return res;
}

                                  
int CCapsImageStd::InitDataStream(PIMAGESTREAMINFO pstr)
{
	                                                   
	pstr->allowloop=0;

	PIMAGEBLOCKINFO pi=di.block+pstr->actblock;

	                                                                    
	pstr->sizemodebit=(pi->flag & CAPS_BF_DMB) ? 1 : 0;

	                                     
	pstr->strstart=pi->dataoffset;

	                                      
	if (pstr->strstart >= (uint32_t)di.pdt->datasize)
		return imgeShort;

	                                                                                                               
	if (pstr->actblock == di.blockcount-1)
		pstr->strend=di.pdt->datasize;
	else
		pstr->strend=pi[1].dataoffset;

	                                        
	if (pstr->strstart >= pstr->strend)
		return imgeShort;

	                                      
	pstr->strbase=di.data+pstr->strstart;
	pstr->strsize=pstr->strend-pstr->strstart;

	return imgeOk;
}

                                 
int CCapsImageStd::InitGapStream(PIMAGESTREAMINFO pstr)
{
	                                               
	pstr->allowloop=1;

	PIMAGEBLOCKINFO pi=di.block+pstr->actblock;

	                                                     
	pstr->sizemodebit=1;

	                                                                            
	int align=(pstr->strtype == isitGap0) ? 0 : 1;
	int gflag=align ? CAPS_BF_GP1 : CAPS_BF_GP0;
	int es=1;
	int gc=0;
	pstr->gapstr[gc++]=cpgapData | es<<CAPS_SIZE_S;
	pstr->gapstr[gc++]=8;
	pstr->gapstr[gc++]=(uint8_t)pi->gapvalue;
	pstr->gapstr[gc++]=cpgapEnd;

	                                                                      
	if (!(pi->flag & (CAPS_BF_GP0 | CAPS_BF_GP1))) {
		pstr->strstart=0;
		pstr->strend=0;
		pstr->strbase=pstr->gapstr;
		pstr->strsize=gc;
		return imgeOk;
	}

	                                      
	if (!(pi->flag & gflag)) {
		pstr->strstart=0;
		pstr->strend=0;
		pstr->strbase=NULL;
		pstr->strsize=0;
		return imgeOk;
	}

	                                     
	pstr->strstart=pi->gapoffset;

	                                      
	if (pstr->strstart >= (uint32_t)di.pdt->datasize)
		return imgeShort;

	                                  
	int bls;
	for (bls=pstr->actblock+1; bls < di.blockcount; bls++) {
		if (di.block[bls].flag & (CAPS_BF_GP0 | CAPS_BF_GP1))
			break;
	}

	                                                                                         
	                                                                          
	if (bls == di.blockcount)
		pstr->strend=di.block[0].dataoffset;
	else
		pstr->strend=di.block[bls].gapoffset;

	                                        
	if (pstr->strstart >= pstr->strend)
		return imgeShort;

	                                      
	pstr->strbase=di.data+pstr->strstart;
	pstr->strsize=pstr->strend-pstr->strstart;

	                                                                                                      
	int semode=align && (pi->flag & CAPS_BF_GP0);
	int res=FindGapStreamEnd(pstr, semode);

	return res;
}

                                                                        
int CCapsImageStd::FindGapStreamEnd(PIMAGESTREAMINFO pstr, int next)
{
	uint8_t *buf=di.data;
	int end=0;

	uint32_t ofs;
	for (ofs=pstr->strstart; !end && ofs < pstr->strend; ) {
		int code=buf[ofs++];

		                                  
		int vc=code >> CAPS_SIZE_S;
		code&=CAPS_DATAMASK;
		uint32_t count;

		if (vc) {
			if (ofs+vc > pstr->strend)
				return imgeTrackData;

			count=ReadValue(buf+ofs, vc);
			ofs+=vc;
		} else
			count=0;

		switch (code) {
			                
			case cpgapEnd:
				end=1;
				break;

			                                                       
			case cpgapCount:
				break;

			                                              
			case cpgapData:
				ofs += trackbuf.CalculateByteSize(count);
				break;

			                           
			default:
				return imgeTrackStream;
		}
	}

	                                        
	if (!end)
		return imgeTrackData;

	                                   
	if (next)
		pstr->strstart=ofs;
	else
		pstr->strend=ofs;

	                                        
	if (pstr->strstart >= pstr->strend)
		return imgeShort;

	                                         
	pstr->strbase=di.data+pstr->strstart;
	pstr->strsize=pstr->strend-pstr->strstart;

	return imgeOk;
}

                          
int CCapsImageStd::ResetStream(PIMAGESTREAMINFO pstr)
{
	                                
	pstr->strofs=0;

	                   
	pstr->readresult=imgeOk;
	pstr->readend=0;
	pstr->readvalue=0;

	                                          
	pstr->setencmode=isiemRaw;
	pstr->streambc=0;
	pstr->samplebc=0;
	pstr->remstreambc=0;
	pstr->remsamplebc=0;
	pstr->sampleofs=0;
	pstr->samplemask=0;
	pstr->samplebase=0;

	pstr->prcbitpos=0;
	pstr->prcrembc=0;
	pstr->prcskipbc=0;
	pstr->prcencstate=1;
	pstr->prcwritebc=0;

	pstr->loopofs=0;
	pstr->loopsize=0;
	pstr->looptype=isiltNone;

	pstr->esfixbc=0;
	pstr->esloopbc=0;

	pstr->scenable=0;
	pstr->scofs=0;
	pstr->scmul=0;

	                             
	if (!pstr->strsize) {
		pstr->readend=1;
		return imgeOk;
	}

	                                                                 
	int res=GetSample(pstr);

	return res;
}

                                                                                                  
int CCapsImageStd::ReadSampleInit(PIMAGESTREAMINFO pstr)
{
	                       
	if (pstr->readend)
		return 1;

	                                                                                 
	if (pstr->allowloop && !pstr->streambc) {
		                                                        
		pstr->remstreambc=pstr->samplebc;
		pstr->remsamplebc=pstr->samplebc;

		                                            
		pstr->sampleofs=0;
		pstr->samplemask=0x80;
	} else {
		                            
		int setencmode=pstr->setencmode;
		int actenctype=pstr->actenctype;

		                                   
		if (GetSample(pstr) != imgeOk)
			return 1;

		                       
		if (pstr->readend)
			return 1;

		                                
		if (pstr->setencmode != setencmode)
			return 1;

		if (pstr->actenctype != actenctype)
			return 1;
	}

	                                
	return 0;
}

                                      
int CCapsImageStd::ReadSample(PIMAGESTREAMINFO pstr, int maxbc)
{
	                      
	int actbc=0;

	             
	uint32_t readvalue=0;

	                                     
	while (maxbc > 0) {
		                             
		if (!pstr->remstreambc)
			if (ReadSampleInit(pstr))
				break;

		                          
		uint32_t remstreambc=pstr->remstreambc;
		uint32_t remsamplebc=pstr->remsamplebc;

		                               
		if (!remsamplebc)
			break;

		                                                                       
		if (!remstreambc)
			continue;

		uint32_t sampleofs=pstr->sampleofs;
		uint32_t samplemask=pstr->samplemask;
		uint32_t sampledata=pstr->samplebase[sampleofs];

		                                  
		while (maxbc > 0) {
			              
			readvalue<<=1;
			if (sampledata & samplemask)
				readvalue|=1;

			            
			actbc++;
			maxbc--;

			                                  
			if (!--remsamplebc) {
				                                                          
				remsamplebc=pstr->samplebc;
				sampleofs=0;
				samplemask=0x80;
				sampledata=pstr->samplebase[sampleofs];
			} else {
				                                
				if (!(samplemask>>=1)) {
					sampleofs++;
					samplemask=0x80;
					sampledata=pstr->samplebase[sampleofs];
				}
			}

			                                                                      
			if (!--remstreambc)
				break;
		}

		                       
		pstr->remstreambc=remstreambc;
		pstr->remsamplebc=remsamplebc;
		pstr->sampleofs=sampleofs;
		pstr->samplemask=samplemask;
	}

	                   
	pstr->readvalue=readvalue;

	return actbc;
}

                                                   
int CCapsImageStd::GetSample(PIMAGESTREAMINFO pstr)
{
	                                            
	pstr->sampleofs=0;
	pstr->samplemask=0x80;

	                     
	pstr->streambc=0;
	pstr->remstreambc=0;

	int res;

	                                                 
	switch (pstr->actenctype) {
		                               
		case cpencMFM:
			                            
			pstr->setencmode=isiemType;

			if (pstr->strtype == isitData)
				res=GetSampleData(pstr);
			else
				res=GetSampleGap(pstr);
			break;

		                                                                               
		case cpencRaw:
			                              
			pstr->setencmode=isiemRaw;

			if (pstr->strtype == isitData)
				res=GetSampleRaw(pstr);
			else
				res=GetSampleGap(pstr);
			break;

		default:
			res=imgeIncompatible;
			break;
	}

	                                                          
	if (res == imgeOk)
		GetLoop(pstr);
	else {
		pstr->readresult=res;
		pstr->readend=1;
	}

	return res;
}

                                                      
int CCapsImageStd::GetSampleRaw(PIMAGESTREAMINFO pstr)
{
	                                       
	uint32_t ofs=pstr->strofs;

	if (ofs >= pstr->strsize)
		return imgeTrackData;

	                                    
	uint8_t *buf=pstr->strbase;
	int code=buf[ofs++];

	int vc=code >> CAPS_SIZE_S;
	code&=CAPS_DATAMASK;
	uint32_t count, bitcount;

	                                        
	if (vc) {
		if (ofs+vc > pstr->strsize)
			return imgeTrackData;

		count=ReadValue(buf+ofs, vc);
		ofs+=vc;
	} else
		count=0;

	switch (code) {
		                  
		case cpdatEnd:
			if (count)
				return imgeTrackData;

			pstr->readend=1;
			bitcount=0;
			break;

		                                 
		case cpdatRaw:
			if (!count)
				return imgeTrackData;

			if (ofs+count > pstr->strsize)
				return imgeTrackData;

			bitcount=count << 3;
			break;

		                                  
		default:
			return imgeTrackStream;
	}

	                                                     
	pstr->strofs=ofs+count;

	                                                                 
	pstr->samplebase=buf+ofs;

	                                                                     
	pstr->samplebc=bitcount;
	pstr->remstreambc=bitcount;
	pstr->remsamplebc=bitcount;

	return imgeOk;
}

                                                      
int CCapsImageStd::GetSampleGap(PIMAGESTREAMINFO pstr)
{
	                           
	uint32_t ofs=pstr->strofs;
	uint8_t *buf=pstr->strbase;

	uint32_t bitcount, bytecount;

	while (1) {
		                                              
		if (ofs >= pstr->strsize)
			return imgeTrackData;

		                                    
		int code=buf[ofs++];

		int vc=code >> CAPS_SIZE_S;
		code&=CAPS_DATAMASK;
		uint32_t count;

		                                        
		if (vc) {
			if (ofs+vc > pstr->strsize)
				return imgeTrackData;

			count=ReadValue(buf+ofs, vc);
			ofs+=vc;
		} else
			count=0;

		                                     
		if (pstr->sizemodebit) {
			bitcount=count;
			bytecount = trackbuf.CalculateByteSize(bitcount);
		} else {
			bytecount=count;
			bitcount=bytecount << 3;
		}

		switch (code) {
			                  
			case cpgapEnd:
				if (bitcount)
					return imgeTrackData;

				pstr->readend=1;
				break;

			                        
			case cpgapCount:
				pstr->streambc=bitcount;
				continue;

			              
			case cpgapData:
				if (!bitcount)
					return imgeTrackData;

				if (ofs+bytecount > pstr->strsize)
					return imgeTrackData;
				break;

			                                  
			default:
				return imgeTrackStream;
		}

		                                                            
		break;
	}

	                                                     
	pstr->strofs=ofs+bytecount;

	                                                                 
	pstr->samplebase=buf+ofs;

	                                                                     
	pstr->samplebc=bitcount;
	pstr->remsamplebc=bitcount;

	                                                                                
	pstr->remstreambc=pstr->streambc ? pstr->streambc : bitcount;

	return imgeOk;
}

                                                       
int CCapsImageStd::GetSampleData(PIMAGESTREAMINFO pstr)
{
	                           
	uint32_t ofs=pstr->strofs;
	uint8_t *buf=pstr->strbase;

	uint32_t bitcount, bytecount;

	while (1) {
		                                              
		if (ofs >= pstr->strsize)
			return imgeTrackData;

		                                    
		int code=buf[ofs++];

		int vc=code >> CAPS_SIZE_S;
		code&=CAPS_DATAMASK;
		uint32_t count;

		                                        
		if (vc) {
			if (ofs+vc > pstr->strsize)
				return imgeTrackData;

			count=ReadValue(buf+ofs, vc);
			ofs+=vc;
		} else
			count=0;

		                                     
		if (pstr->sizemodebit) {
			bitcount=count;
			bytecount = trackbuf.CalculateByteSize(bitcount);
		} else {
			bytecount=count;
			bitcount=bytecount << 3;
		}

		switch (code) {
			                  
			case cpdatEnd:
				if (bitcount)
					return imgeTrackData;

				pstr->readend=1;
				break;

			              
			case cpdatMark:
				                             
				pstr->setencmode=isiemRaw;

			                                          
			case cpdatData:
			case cpdatGap:
				if (!bitcount)
					return imgeTrackData;

				if (ofs+bytecount > pstr->strsize)
					return imgeTrackData;
				break;

			            
			case cpdatFData:
				pstr->setencmode=isiemWeak;
				break;

			                                  
			default:
				return imgeTrackStream;
		}

		                                                            
		break;
	}

	                             
	if (pstr->setencmode != isiemWeak) {
		                                                     
		pstr->strofs=ofs+bytecount;

		                                                                 
		pstr->samplebase=buf+ofs;

		                                                                     
		pstr->samplebc=bitcount;
		pstr->remsamplebc=bitcount;
	} else {
		                                                             
		pstr->strofs=ofs;

		                                                                      
		pstr->samplebase=pstr->weakdata;

		                                                                   
		pstr->samplebc=8;
		pstr->remsamplebc=8;

		                
		pstr->weakdata[0]=0;
	}

	                                                                                
	pstr->remstreambc=pstr->streambc ? pstr->streambc : bitcount;

	return imgeOk;
}

                                                   
int CCapsImageStd::ProcessStream(PIMAGESTREAMINFO pstr, uint32_t bitpos, int maxbc, int skipbc, int encnew)
{
	                           
	pstr->prcbitpos=bitpos;
	pstr->prcrembc=maxbc;
	pstr->prcskipbc=skipbc;
	pstr->prcencstate=encnew;
	pstr->prcwritebc=0;

	                         
	while (pstr->prcrembc > 0) {
		                             
		if (pstr->readend)
			break;

		                                              
		switch (pstr->setencmode) {
			                   
			case isiemRaw:
				ProcessStreamRaw(pstr);
				break;

			                                                        
			case isiemType:
				switch (pstr->actenctype) {
					              
					case cpencMFM:
						ProcessStreamMFM(pstr);
						break;

					           
					case cpencRaw:
						ProcessStreamRaw(pstr);
						break;

					default:
						return imgeIncompatible;
				}
				break;

			            
			case isiemWeak:
				ProcessStreamWeak(pstr);
				break;

			default:
				return imgeGeneric;
		}
	}

	return pstr->readresult;
}

                                              
void CCapsImageStd::ProcessStreamRaw(PIMAGESTREAMINFO pstr)
{
	                    
	uint32_t bitpos=pstr->prcbitpos;
	int maxbc=pstr->prcrembc;
	int skipbc=pstr->prcskipbc;
	int actbc=0;

	                         
	while (maxbc > 0) {
		                      
		int readbc=ReadSample(pstr, rawreadlen);

		                                  
		int diffbc=rawreadlen-readbc;

		                                        
		if (readbc > 0) {
			                               
			int writebc=readbc;

			                                         
			if (writebc > skipbc) {
				writebc-=skipbc;
				skipbc=0;

				                 
				uint32_t value=pstr->readvalue;

				                                                                   
				if (writebc > maxbc) {
					value >>= (writebc-maxbc);
					writebc=maxbc;
				}

				trackbuf.WriteBitWrap(bitpos, value, writebc);

				                      
				actbc+=writebc;
				maxbc-=writebc;
				bitpos+=writebc;

				                                                  
				if (bitpos >= di.trackbc)
					bitpos-=di.trackbc;
			} else
				skipbc-=writebc;
		}

		                                                                
		if (diffbc)
			break;
	}

	                                      
	if (!pstr->prcwritebc && pstr->prcencstate && actbc)
		di.block[pstr->actblock].fdenc=ibieRaw;

	                     
	pstr->prcbitpos=bitpos;
	pstr->prcrembc=maxbc;
	pstr->prcskipbc=skipbc;
	pstr->prcwritebc+=actbc;
}

                                              
void CCapsImageStd::ProcessStreamMFM(PIMAGESTREAMINFO pstr)
{
	                    
	uint32_t bitpos=pstr->prcbitpos;
	int maxbc=pstr->prcrembc;
	int skipbc=pstr->prcskipbc;
	int actbc=0;

	                                                                                             
	uint32_t lv;
	if (!pstr->prcwritebc && pstr->prcencstate)
		lv=0;
	else {
		uint32_t bp=(bitpos ? bitpos : di.trackbc)-1;
		lv = trackbuf.ReadBit(bp);
	}

	                         
	while (maxbc > 0) {
		                      
		int readbc=ReadSample(pstr, mfmreadlen);

		                                  
		int diffbc=mfmreadlen-readbc;

		                                        
		if (readbc > 0) {
			                                           
			int writebc=readbc << 1;

			                                         
			if (writebc > skipbc) {
				writebc-=skipbc;
				skipbc=0;

				                                                                       
				                                                                             
				int encodebc=mfmreadlen-((writebc+1) >> 1);

				                 
				uint32_t value=pstr->readvalue;

				                                                                
				value <<= encodebc;

				                 
				value=CDiskEncoding::mfmcode[value & mfmindexmask];

				                                            
				if (lv & 1)
					value &= mfmmsbclear;

				                                                                              
				value >>= (encodebc << 1);

				                                                                   
				if (writebc > maxbc) {
					value >>= (writebc-maxbc);
					writebc=maxbc;
				}

				                         
				lv=value;

				                          
				trackbuf.WriteBitWrap(bitpos, value, writebc);

				                      
				actbc+=writebc;
				maxbc-=writebc;
				bitpos+=writebc;

				                                                  
				if (bitpos >= di.trackbc)
					bitpos-=di.trackbc;
			} else
				skipbc-=writebc;
		}

		                                                                
		if (diffbc)
			break;
	}

	                                      
	if (!pstr->prcwritebc && pstr->prcencstate && actbc)
		di.block[pstr->actblock].fdenc=ibieMFM;

	                     
	pstr->prcbitpos=bitpos;
	pstr->prcrembc=maxbc;
	pstr->prcskipbc=skipbc;
	pstr->prcwritebc+=actbc;
}

                                               
void CCapsImageStd::ProcessStreamWeak(PIMAGESTREAMINFO pstr)
{
	                    
	uint32_t bitpos=pstr->prcbitpos;
	int maxbc=pstr->prcrembc;
	int skipbc=pstr->prcskipbc;
	int actbc=0;
		
	                         
	if (maxbc > 0) {
		                                       
		int writebc=pstr->remstreambc;

		                                                                             
		ReadSampleInit(pstr);

		                                      
		switch (pstr->actenctype) {
			case cpencMFM:
				writebc <<= 1;
				break;
		}

		                                         
		if (writebc > skipbc) {
			writebc-=skipbc;
			skipbc=0;

			                              
			if (writebc > maxbc)
				writebc=maxbc;

			                                            
			DiskDataMark ddm;
			ddm.group = 0;
			ddm.position=bitpos;
			ddm.size=writebc;
			AddFD(di.pdt, &ddm, 1, DEF_FDALLOC);

			              
			trackbuf.ClearBitWrap(bitpos, writebc);

			                      
			actbc+=writebc;
			maxbc-=writebc;
			bitpos+=writebc;

			                                                  
			if (bitpos >= di.trackbc)
				bitpos-=di.trackbc;
		} else
			skipbc-=writebc;
	}

	                                      
	if (!pstr->prcwritebc && pstr->prcencstate && actbc)
		di.block[pstr->actblock].fdenc=ibieWeak;

	                     
	pstr->prcbitpos=bitpos;
	pstr->prcrembc=maxbc;
	pstr->prcskipbc=skipbc;
	pstr->prcwritebc+=actbc;
}

                                             
int CCapsImageStd::CalculateStreamSize(PIMAGESTREAMINFO pstr)
{
	                                
	int res=FindLoopPoint(pstr);
	if (res != imgeOk)
		return res;

	                                         
	ImageStreamInfo isi=*pstr;

	                              
	                    
	                    
	int fixbc=0, loopbc=0;

	                     
	while (!isi.readend) {
		switch (isi.looptype) {
			          
			case isiltNone:
				fixbc+=GetEncodedSize(&isi, isi.remstreambc);
				break;

			                                
			case isiltAuto:
			case isiltStream:
				fixbc+=GetEncodedSize(&isi, isi.remstreambc);
				if (isi.strofs == isi.loopofs)
					loopbc+=GetEncodedSize(&isi, isi.samplebc);
				break;

			default:
				return imgeGeneric;
		}

		                  
		if (GetSample(&isi) != imgeOk)
			return imgeGeneric;
	}

	               
	pstr->esfixbc=fixbc;
	pstr->esloopbc=loopbc;

	return imgeOk;
}

                                              
int CCapsImageStd::GetEncodedSize(PIMAGESTREAMINFO pstr, int bitcnt)
{
	                                                         
	switch (pstr->setencmode) {
		case isiemType:
		case isiemWeak:
			switch (pstr->actenctype) {
				case cpencMFM:
					bitcnt <<= 1;
					break;
			}
			break;
	}

	return bitcnt;
}

                                                       
int CCapsImageStd::FindLoopPoint(PIMAGESTREAMINFO pstr)
{
	                                                 
	if (!pstr->allowloop) {
		pstr->loopofs=0;
		pstr->loopsize=0;
		pstr->looptype=isiltNone;
		return imgeOk;
	}

	                                         
	ImageStreamInfo isi=*pstr;

	                
	uint32_t loopofs=0;
	int loopsize=0, loopfound=0, cnt=0;

	while (!isi.readend) {
		uint32_t ofs=isi.strofs;
		int bc=isi.samplebc;

		switch (isi.strtype) {
			                                                                                      
			case isitGap0:
				if (loopfound)
					return imgeGeneric;

				loopofs=ofs;
				loopsize=bc;
				break;

			                                                                                            
			case isitGap1:
				if (!cnt) {
					loopofs=ofs;
					loopsize=bc;
				} else
					if (!isi.streambc)
						return imgeGeneric;
				break;
		}

		                    
		if (!isi.streambc) {
			if (loopfound)
				return imgeGeneric;

			loopfound++;
		}

		cnt++;

		                  
		if (GetSample(&isi) != imgeOk)
			return imgeGeneric;
	}

	                             
	pstr->loopofs=loopofs;
	pstr->loopsize=loopsize;

	if (loopsize)
		pstr->looptype=loopfound ? isiltStream : isiltAuto;
	else
		pstr->looptype=isiltNone;

	return imgeOk;
}

                          
int CCapsImageStd::ProcessBlock(int blk, uint32_t bitpos, int datasize, int gapsize)
{
	int res;

	                          
	di.encbitpos=bitpos;
	di.encwritebc=0;
	di.encgsvalid=0;
	di.encgapsplit=0;

	                   
	if (blk<0 || blk>=di.blockcount || datasize<0 || gapsize<0)
		return imgeGeneric;

	if (blk >= di.pdt->sipsize)
		return imgeGeneric;

	                           
	di.block[blk].fdenc=ibieNA;
	di.block[blk].fdbitpos=bitpos;

	PDISKSECTORINFO si=di.pdt->sip+blk;
	si->descdatasize=di.block[blk].blockbits;
	si->descgapsize=di.block[blk].gapbits;
	si->celltype=di.block[blk].celltype;
	si->enctype=di.block[blk].enctype;

	                    
	si->datastart=di.encbitpos;
	si->datasize=datasize;
	res=ProcessBlockData(blk, datasize);
	if (res != imgeOk)
		return res;

	                  
	si->gapstart=di.encbitpos;
	si->gapsize=gapsize;
	res=ProcessBlockGap(blk, gapsize);
	if (res != imgeOk)
		return res;

	return imgeOk;
}

                    
int CCapsImageStd::ProcessBlockData(int blk, int datasize)
{
	                       
	if (!datasize)
		return imgeOk;

	int res;
	ImageStreamInfo dsi;

	                             
	res=InitStream(&dsi, isitData, blk);
	if (res != imgeOk)
		return res;

	                    
	res=ProcessStream(&dsi, di.encbitpos, datasize, 0, !di.encwritebc);
	if (res != imgeOk)
		return res;

	                                                  
	if (dsi.prcwritebc != datasize)
		return imgeGeneric;

	                                       
	di.encbitpos=dsi.prcbitpos;
	di.encwritebc+=dsi.prcwritebc;

	return imgeOk;
}

                  
int CCapsImageStd::ProcessBlockGap(int blk, int gapsize)
{
	                      
	if (!gapsize)
		return imgeOk;

	int res;
	ImageStreamInfo gsi0, gsi1;

	                                    
	res=InitStream(&gsi0, isitGap0, blk);
	if (res != imgeOk)
		return res;

	res=CalculateStreamSize(&gsi0);
	if (res != imgeOk)
		return res;

	                                     
	res=InitStream(&gsi1, isitGap1, blk);
	if (res != imgeOk)
		return res;

	res=CalculateStreamSize(&gsi1);
	if (res != imgeOk)
		return res;

	                                
	int se0=gsi0.esfixbc || gsi0.esloopbc;
	int se1=gsi1.esfixbc || gsi1.esloopbc;

	                            
	int secnt=0;
	if (se0)
		secnt++;
	if (se1)
		secnt++;

	                                                                   
	int tl0=gsi0.esloopbc && (gsi0.looptype == isiltStream);
	int tl1=gsi1.esloopbc && (gsi1.looptype == isiltStream);

	                                           
	int tlcnt=0;
	if (tl0)
		tlcnt++;
	if (tl1)
		tlcnt++;

	PDISKSECTORINFO si=di.pdt->sip+blk;

	                                      
	if (tl0)
		si->gapws0mode=csiegmResize;
	else
		si->gapws0mode=(gsi0.esloopbc) ? csiegmAuto : csiegmFixed;

	                                       
	if (tl1)
		si->gapws1mode=csiegmResize;
	else
		si->gapws1mode=(gsi1.esloopbc) ? csiegmAuto : csiegmFixed;

	             
	switch (secnt) {
		                                                                  
		case 0:
			return imgeGeneric;

		                   
		case 1:
			res=ProcessBlockGap((se0 ? &gsi0 : &gsi1), gapsize);
			break;

		                                                                                            
		case 2:
			if (tlcnt == 1)
				res=ProcessBlockGap(&gsi0, &gsi1, gapsize, (tl0 ? 0 : 1));
			else
				res=ProcessBlockGap(&gsi0, &gsi1, gapsize);
			break;
	}

	return res;
}

                                            
int CCapsImageStd::ProcessBlockGap(PIMAGESTREAMINFO pg, int gapsize)
{
	                      
	if (!gapsize)
		return imgeOk;

	int skip=0;

	if (pg->esfixbc >= gapsize) {
		                                                     
		SetLoop(pg, 0);

		                                          
		if (pg->strtype == isitGap1)
			skip=pg->esfixbc-gapsize;
	} else {
		                                                                  
		if (!pg->esloopbc)
			return imgeGeneric;

		                                                
		int miss=gapsize-pg->esfixbc;

		                                                      
		int scnt=miss/pg->esloopbc;

		                                                         
		int smod=miss%pg->esloopbc;

		                                
		if (smod) {
			                     
			scnt++;

			                                                                    
			if (pg->strtype == isitGap1)
				skip=pg->esloopbc-smod;
		}

		              
		SetLoop(pg, scnt);
	}

	                  
	int res=ProcessStream(pg, di.encbitpos, gapsize, skip, !di.encwritebc);
	if (res != imgeOk)
		return res;

	                                                  
	if (pg->prcwritebc != gapsize)
		return imgeGeneric;

	                                       
	di.encbitpos=pg->prcbitpos;
	di.encwritebc+=pg->prcwritebc;

	if (pg->strtype == isitGap0)
		di.pdt->sip[pg->actblock].gapsizews0=gapsize;
	else
		di.pdt->sip[pg->actblock].gapsizews1=gapsize;

	return imgeOk;
}

                                                                          
int CCapsImageStd::ProcessBlockGap(PIMAGESTREAMINFO pg0, PIMAGESTREAMINFO pg1, int gapsize)
{
	int gs0=pg0->esfixbc;
	int gs1=pg1->esfixbc;
	int fs=gs0+gs1;

	if (fs >= gapsize) {
		                                                                 
		int rem=fs-gapsize;
		int rem0=rem >> 1;
		int rem1=rem-rem0;

		                                                                                                      
		while (rem0 || rem1) {
			if (gs0 >= rem0) {
				gs0-=rem0;
				rem0=0;
			} else {
				rem1+=rem0-gs0;
				rem0=0;
				gs0=0;
			}

			if (gs1 >= rem1) {
				gs1-=rem1;
				rem1=0;
			} else {
				rem0+=rem1-gs1;
				rem1=0;
				gs1=0;
			}
		}
	} else {
		                                                                                   
		if (!pg0->esloopbc && !pg1->esloopbc)
			return imgeGeneric;

		                                         
		int miss=gapsize-fs;
		int miss0=miss >> 1;

		                                                                                           
		if (pg0->actblock == di.blockcount-1) {
			uint32_t sbp=di.encbitpos % di.singletrackbc;
			if (sbp+gs0 <= di.singletrackbc)
				if (sbp+gs0+miss >= di.singletrackbc)
					miss0=di.singletrackbc-(sbp+gs0);
		}

		int miss1=miss-miss0;

		                                                                                                 
		while (miss0 || miss1) {
			if (pg0->esloopbc) {
				gs0+=miss0;
				miss0=0;
			} else {
				miss1+=miss0;
				miss0=0;
			}

			if (pg1->esloopbc) {
				gs1+=miss1;
				miss1=0;
			} else {
				miss0+=miss1;
				miss1=0;
			}
		}
	}

	                                                           
	if (gs0+gs1 != gapsize)
		return imgeGeneric;

	                        
	int res=ProcessBlockGap(pg0, gs0);
	if (res != imgeOk)
		return res;

	                         
	di.encgsvalid=1;
	di.encgapsplit=di.encbitpos;

	                          
	ProcessBlockGap(pg1, gs1);
	if (res != imgeOk)
		return res;

	return imgeOk;
}

                                                                
int CCapsImageStd::ProcessBlockGap(PIMAGESTREAMINFO pg0, PIMAGESTREAMINFO pg1, int gapsize, int loopsel)
{
	int gs0=pg0->esfixbc;
	int gs1=pg1->esfixbc;

	                                                                                    
	if (loopsel) {
		                  
		if (gs0 > gapsize)
			gs0=gapsize;

		gs1=gapsize-gs0;
	} else {
		                  
		if (gs1 > gapsize)
			gs1=gapsize;

		gs0=gapsize-gs1;
	}

	                        
	int res=ProcessBlockGap(pg0, gs0);
	if (res != imgeOk)
		return res;

	                         
	di.encgsvalid=1;
	di.encgapsplit=di.encbitpos;

	                          
	ProcessBlockGap(pg1, gs1);
	if (res != imgeOk)
		return res;

	return imgeOk;
}

                                                     
void CCapsImageStd::SetLoop(PIMAGESTREAMINFO pg, int value)
{
	switch (pg->looptype) {
		                                            
		case isiltNone:
			pg->scenable=0;
			break;

		                                               
		case isiltAuto:
			if (value) {
				pg->scenable=1;
				pg->scofs=pg->loopofs;
				pg->scmul=value;
			} else
				pg->scenable=0;
			break;

		                                         
		case isiltStream:
			pg->scenable=1;
			pg->scofs=pg->loopofs;
			pg->scmul=value;
			break;
	}

	                                                                       
	GetLoop(pg);
}

                               
void CCapsImageStd::GetLoop(PIMAGESTREAMINFO pg)
{
	                       
	if (!pg->scenable)
		return;

	                           
	if (pg->strofs != pg->scofs)
		return;

	                  
	pg->remstreambc+=pg->scmul*pg->samplebc;

	                             
	pg->streambc=pg->remstreambc;
}

                                
void CCapsImageStd::MFMFixup()
{
	                     
	for (int blk=0; blk < di.blockcount; blk++) {
		PIMAGEBLOCKINFO pi=di.block+blk;

		                       
		if (pi->fdenc != ibieMFM)
			continue;

		                                            
		uint32_t lbp=(pi->fdbitpos ? pi->fdbitpos : di.trackbc)-1;
		int lv = trackbuf.ReadBit(lbp);

		                                                            
		if (lv)
			trackbuf.ClearBit(pi->fdbitpos, 1);
	}
}
