#include "stdafx.h"

                                         
int CCapsImage::fb_init = 0;

                                                                                                          
int8_t CCapsImage::f0b_table[8][256];

                                                                                                          
int8_t CCapsImage::f1b_table[8][256];



                       
int CCapsImage::CompareImage()
{
	PDISKTRACKINFO pti=di.pdt;

	int res=imgeOk;

	                     
	pti->tracklen=pti->ci.trksize;

	                        
	pti->trackcnt=1;

	                            
	if (pti->ci.dentype == cpdenNoise)
		pti->trackcnt=0;

	pti->tracklen*=pti->trackcnt;

	                          
	if (pti->tracklen) {
		pti->trackbuf=new UBYTE[pti->tracklen];
		memset(pti->trackbuf, 0, pti->tracklen);
	}

	                             
	pti->trackdata[0]=pti->trackbuf;
	pti->tracksize[0]=0;
	pti->trackstart[0]=0;

	                                     
	pti->comppos=0;
	pti->sdpos=0;

	                
	if (pti->trackcnt) {
		int eblk=pti->compeblk;
		if (eblk < 0)
			eblk=pti->ci.blkcnt;
		else
			eblk++;

		for (int blk=pti->compsblk; blk < eblk; blk++)
			if ((res=CompareBlock(blk)) != imgeOk)
				return res;
	}

	pti->tracksize[0]=pti->comppos;
	return res;
}

                             
int CCapsImage::CompareBlock(unsigned blk)
{
	PDISKTRACKINFO pti=di.pdt;
	uint8_t *buf=di.data;

	                               
	if (blk>=pti->ci.blkcnt || !buf || !di.datacount)
		return imgeGeneric;

	UDWORD maxofs=pti->datasize;

	                       
	if ((blk+1)*sizeof(CapsBlock) > maxofs)
		return imgeShort;

	                                   
	CapsBlock cb;
	memcpy(&cb, buf+blk*sizeof(CapsBlock), sizeof(CapsBlock));
	CCapsLoader::Swap((PUDWORD)&cb, sizeof(CapsBlock));

	                       
	if (cb.dataoffset >= maxofs)
		return imgeShort;

	                                                            
	if (blk != pti->ci.blkcnt-1)
		maxofs=ReadValue(buf+(blk+1)*sizeof(CapsBlock)+offsetof(CapsBlock, dataoffset), sizeof(UDWORD));

	                       
	if (cb.dataoffset >= maxofs)
		return imgeShort;

	                     
	switch (cb.enctype) {
		case cpencMFM:
			break;

		default:
			return imgeIncompatible;
	}

	                
	int end=false;
	int pos=pti->comppos;
	for (UDWORD ofs=cb.dataoffset; ofs < maxofs; ) {
		int code=buf[ofs++];

		             
		int vc=code>>CAPS_SIZE_S;
		code&=CAPS_DATAMASK;
		UDWORD count;

		if (vc) {
			if (ofs+vc > maxofs)
				return imgeTrackData;

			count=ReadValue(buf+ofs, vc);
			ofs+=vc;
		} else
			count=0;

		               
		switch (code) {
			             
			case cpdatEnd:
				                
				if (count)
					return imgeTrackData;
				end=true;
				break;

			case cpdatData:
				                
				if (!count)
					return imgeTrackData;
				if (ofs+count > maxofs)
					return imgeTrackData;

				                         
				if (di.flag & DI_COMP_DATA) {
					memcpy(pti->trackbuf+pos, buf+ofs, count);
					pos+=count;
				}
				ofs+=count;
				break;

			case cpdatGap:
				                
				if (!count)
					return imgeTrackData;
				if (ofs+count > maxofs)
					return imgeTrackData;

				                
				ofs+=count;
				break;

			case cpdatMark:
				                
				if (!count)
					return imgeTrackData;
				if (ofs+count > maxofs)
					return imgeTrackData;

				                             
				if (di.flag & DI_COMP_MARK) {
					memcpy(pti->trackbuf+pos, buf+ofs, count);
					pos+=count;
				}
				ofs+=count;
				break;

			case cpdatRaw:
				                
				if (!count)
					return imgeTrackData;
				if (ofs+count > maxofs)
					return imgeTrackData;

				                             
				if (di.flag & DI_COMP_RAW) {
					memcpy(pti->trackbuf+pos, buf+ofs, count);
					pos+=count;
				}
				ofs+=count;
				break;

			case cpdatFData:
				                
				if (!count)
					return imgeTrackData;

				                                                             
				if (di.flag & DI_COMP_FDATA)
					pos+=count;
				break;

			default:
				return imgeTrackStream;
		}
	}

	                                                
	if (!end)
		return imgeTrackData;

	pti->comppos=pos;
	return imgeOk;
}



                           
int CCapsImage::DecompressDump()
{
	                                                
	if (di.flag & (DI_LOCK_COMP | DI_LOCK_ALIGN))
		return imgeUnsupported;

	PDISKTRACKINFO pti = di.pdt;
	uint8_t *buf=di.data;

	CCTRawCodec ctr;

	                  
	int res=ctr.DecompressDump(buf, pti->datasize);
	if (res == imgeOk) {
		ConvertDumpInfo(ctr.GetInfo());
	} else
		pti->type=dtitError;

	return res;
}

                                                                           
void CCapsImage::ConvertDumpInfo(PCAPSWH wh)
{
	PDISKTRACKINFO pti = di.pdt;

	                                                                                                                 
	int maxrev = min(CAPS_MTRS, wh->trkcnt);
	pti->rawtrackcnt = maxrev;

	                                                                             
	pti->rawlen = wh->rawlen;

	                                                      
	wh->rawbuf = NULL;

	                      
	int maxtracksize = 0;

	                                                                              
	for (int rev = 0; rev < maxrev; rev++) {
		pti->trackdata[rev] = wh->trkbuf[rev];
		pti->tracksize[rev] = wh->trklen[rev];

		                         
		if (pti->tracksize[rev] > maxtracksize)
			maxtracksize = pti->tracksize[rev];
	}

	                                               
	pti->rawtimebuf = wh->timbuf;
	pti->rawtimecnt = wh->timlen;

	                                                       
	wh->timbuf = NULL;

	                                                                                           
	pti->timebuf = new UDWORD[maxtracksize + 1];

	                                          
	double timesum = 0;
	for (int i = 0; i < pti->rawtimecnt; i++) {
		timesum += pti->rawtimebuf[i];
	}

	                                                         
	                                                                       
	int targetsum = pti->rawtimecnt * 1000;
	int actsum = 0;
	double tconv = (double)targetsum / timesum;
	double rem = 0;
	for (int i = 0; i < pti->rawtimecnt; i++) {
		double vr = pti->rawtimebuf[i] * tconv + rem;
		UDWORD vi = (UDWORD)vr;
		rem = vr - vi;
		pti->rawtimebuf[i] = vi;
		actsum += vi;
	}

	                                                                                              
	                                        
	                                                                                                
	int tdiff = targetsum - actsum;
	if (tdiff > 0)
		pti->rawtimebuf[pti->rawtimecnt - 1] += tdiff;

	                                                           
	int trackcnt = (di.flag & DI_LOCK_ANA) ? 5 : 1;

	                                            
	if (!(di.flag & DI_LOCK_UPDATEFD))
		trackcnt = 5;

	                                                                                       
	if (trackcnt > maxrev)
		trackcnt = maxrev;

	                   
	pti->trackcnt = trackcnt;
	pti->overlap = -1;
	pti->overlapbit = -1;

	                    
	pti->wseed = 0x87654321;

	                                      
	pti->rawdump = 1;
	pti->rawupdate = 1;

	                                                         
	InitFirstBitTables();

	                                     
	FindWeakBits();

	                                             
	UpdateDump();
}

                                                         
int CCapsImage::UpdateDump()
{
	PDISKTRACKINFO pti = di.pdt;

	                                                                  
	int rev = dii.nextrev % pti->rawtrackcnt;

	                                                           
	int allrev = (pti->trackcnt == pti->rawtrackcnt);

	                                                             
	if (allrev)
		rev = 0;

	                                  
	dii.realrev = rev;

	                  
	pti->trackbuf = pti->trackdata[rev];
	pti->tracklen = allrev ? pti->rawlen : pti->tracksize[rev];
	pti->trackbc = pti->tracklen << 3;
	pti->singletrackbc = pti->trackbc;

	                                                                                     
	pti->timecnt = pti->tracksize[rev];

	                                       
	int rawsize = pti->rawtimecnt;

	                                                                                   
	int tsize = min(rawsize, pti->timecnt);

	                  
	memcpy(pti->timebuf, pti->rawtimebuf, tsize*sizeof(UDWORD));

	                                                                                 
	                                                                                          
	int pos;
	for (pos = tsize; pos < pti->timecnt; pos++) {
			pti->timebuf[pos] = 1000;
	}

	                          
	pti->timebuf[pos] = 0;

	                         
	if (di.flag & DI_LOCK_DENALT)
		ConvertDensity(pti);

	UpdateImage(rev);

	return imgeOk;
}

                      
void CCapsImage::FindWeakBits()
{
	PDISKTRACKINFO pti = di.pdt;

	                                                           
	int allrev = (pti->trackcnt == pti->rawtrackcnt);

	                          
	for (int rev = 0; rev < pti->rawtrackcnt; rev++) {
		                                                       
		PUBYTE trackbuf = pti->trackdata[rev];
		int tracklen = allrev ? pti->rawlen : pti->tracksize[rev];

		                                                      
		int bofs = 0;

		                                                        
		int zcnt = 0;

		                                           
		int lastbp = 0;

		                                                                         
		int actbit = 0;

		                                  
		while (bofs < tracklen) {
			                                    
			uint8_t bval = trackbuf[bofs];
			
			while (1) {
				                       
				int f0b;

				                                                             
				if (!zcnt) {
					                                                                     
					f0b = f0b_table[actbit][bval];
					if (f0b == 8) {
						                                               
						break;
					} else {
						                                            
						actbit = f0b;

						                              
						lastbp = bofs << 3 | actbit;
					}
				} else {
					                                        
					f0b = 0;
				}

				                                                                     
				int f1b = f1b_table[actbit][bval];

				                                                                      
				zcnt += f1b - f0b;

				if (f1b == 8) {
					                                               
					break;
				} else {
					                                                        
					actbit = f1b;
				}

				               
				if (zcnt >= MIN_CTRAW_WEAK_SIZE && zcnt <= MAX_CTRAW_WEAK_SIZE)
					AddWeakBitArea(rev, lastbp, zcnt);

				             
				zcnt = 0;
			}

			                              
			actbit = 0;
			bofs++;
		}

		                                                               
		if (zcnt >= MIN_CTRAW_WEAK_SIZE && zcnt <= MAX_CTRAW_WEAK_SIZE)
			AddWeakBitArea(rev, lastbp, zcnt);
	}
}

                                
void CCapsImage::AddWeakBitArea(int group, int bitpos, int size)
{
	DiskDataMark ddm;
	ddm.group = group;
	ddm.position = bitpos;
	ddm.size = size;
	AddFD(di.pdt, &ddm, 1, (CAPS_MTRS * DEF_CTRAW_FDALLOC));
}

                                                                    
void CCapsImage::InitFirstBitTables()
{
	                              
	if (fb_init)
		return;

	                       
	fb_init = 1;

	                        
	for (int startbit = 0; startbit < 8; startbit++) {
		                                            
		int bitmask = 1 << (7-startbit);

		                     
		for (int value = 0; value < 256; value++) {

			                                                          
			int bitpos = startbit;
			for (int bm = bitmask; bm; bm >>= 1, bitpos++) {
				if (!(value & bm))
					break;
			}

			                                        
			f0b_table[startbit][value] = bitpos;

			                                                          
			bitpos = startbit;
			for (int bm = bitmask; bm; bm >>= 1, bitpos++) {
				if (value & bm)
					break;
			}

			                                        
			f1b_table[startbit][value] = bitpos;
		}
	}
}

