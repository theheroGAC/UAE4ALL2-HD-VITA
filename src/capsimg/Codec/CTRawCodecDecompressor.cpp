#include "stdafx.h"



                                
int CCTRawCodec::DecompressDump(PUBYTE buf, int len)
{
	CapsPack cpk;
	PCAPSPACK pk;

	                   
	Free();

	                    
	int hlen = sizeof(CapsRaw);
	if (len < hlen)
		return imgeShort;
	wh.cr = *(PCAPSRAW)buf;
	Swap((PUDWORD)&wh.cr, sizeof(CapsRaw));

	                  
	int tlen = wh.cr.time;
	int rlen = wh.cr.raw;
	if (len < hlen + tlen + rlen)
		return imgeShort;

	                            
	PUBYTE denbuf = buf + hlen;
	pk = GetPackHeader(&cpk, denbuf, tlen);
	if (!pk)
		return imgeDensityHeader;

	                          
	PUBYTE trkbuf = denbuf + tlen;
	pk = GetPackHeader(&cpk, trkbuf, rlen);
	if (!pk)
		return imgeTrackHeader;

	int err;

	                                 
	wh.cdbuf = denbuf;
	wh.cdlen = tlen;
	if (!(err = DecompressDensity(1)))
		err = DecompressDensity();
	wh.cdbuf = NULL;
	if (err)
		return err;

	                        
	wh.ctbuf = trkbuf;
	wh.ctlen = rlen;
	if (!(err = DecompressTrack(1)))
		err = DecompressTrack();
	wh.ctbuf = NULL;

	return err;
}



                                                             
int CCTRawCodec::DecompressDensity(int verify)
{
	CapsPack cpk;

	                                              
	if (!verify)
		FreeUncompressedDensity();

	                     
	PCAPSPACK pk = GetPackHeader(&cpk, wh.cdbuf, wh.cdlen);
	if (!pk)
		return imgeDensityHeader;

	                                              
	if (verify)
		if (pk->ccrc != CalcCRC(wh.cdbuf + sizeof(CapsPack), pk->csize))
			return imgeDensityStream;

	                          
	PUDWORD dst = DecompressDensity(wh.cdbuf, wh.cdlen);
	int res = imgeOk;

	if (verify) {
		                                                   
		Swap(dst, pk->usize);

		                                 
		if (pk->ucrc != CalcCRC((PUBYTE)dst, pk->usize))
			res = imgeDensityData;

		                    
		delete[] dst;
	}	else {
		                                       
		wh.timbuf = dst;
		wh.timlen = pk->usize >> 2;
	}

	return res;
}

                                        
PUDWORD CCTRawCodec::DecompressDensity(PUBYTE src, int slen, PUDWORD dst)
{
	CapsPack cpk;

	                     
	PCAPSPACK pk = GetPackHeader(&cpk, src, slen);
	if (!pk)
		return NULL;

	                                                   
	PUDWORD buf = dst;
	if (!buf && pk->usize)
		buf = new UDWORD[pk->usize >> 2];

	                                                          
	PUDWORD mem = buf + (pk->usize >> 2);
	src += slen;

	                                               
	while (mem > buf) {
		int size, ofs;
		UBYTE cb0 = *--src;

		switch (cb0 & 0x03) {
			             
			case 0x0:
				                   
				if (cb0 & 0x08)
					size = ((cb0 << 4) & 0xf00) | (*--src);
				else
					size = (cb0 >> 4) + 1;
				if (cb0 & 0x04) {
					                    
					while (size--) {
						UDWORD data;
						data = (*--src);
						data = (data << 8) | (*--src);
						data = (data << 8) | (*--src);
						data = (data << 8) | (*--src);
						*--mem = data;
					}
				} else {
					                   
					while (size--)
						*--mem = *--src;
				}
				continue;

			                                       
			case 0x1:
				size = (cb0 >> 2) + 1;
				ofs = (*--src) + 1;
				break;

			                                        
			case 0x2:
				size = (cb0 >> 2) + 1;
				ofs = (*--src);
				ofs = (ofs << 8) | (*--src);
				break;

				                                         
			case 0x3:
				size = ((cb0 << 6) & 0x3f00) | (*--src);
				ofs = (*--src);
				ofs = (ofs << 8) | (*--src);
				break;
		}

		                    
		while (size--) {
			mem--;
			*mem = mem[ofs];
		}
	}

	return buf;
}



                                                           
int CCTRawCodec::DecompressTrack(int verify)
{
	CapsPack cpk;
	CapsWH cwh;

	                                              
	if (!verify)
		FreeUncompressedTrack();

	                   
	PCAPSPACK pk = GetPackHeader(&cpk, wh.ctbuf, wh.ctlen);
	if (!pk)
		return imgeTrackHeader;

	                                              
	if (verify)
		if (pk->ccrc != CalcCRC(wh.ctbuf + sizeof(CapsPack), pk->csize))
			return imgeTrackStream;

	                        
	PCAPSWH wb = DecompressTrack(&cwh, wh.ctbuf, wh.ctlen);
	int res = imgeOk;

	if (verify) {
		                                                
		if (pk->ucrc != CalcCRC(wb->rawbuf, wb->rawlen))
			res = imgeTrackData;

		                  
		FreeUncompressedTrack(wb);
	} else {
		                                     
		wh.rawbuf = wb->rawbuf;
		wh.rawlen = wb->rawlen;

		for (int trk = 0; trk < CAPS_MTRS; trk++) {
			wh.trkbuf[trk] = wb->trkbuf[trk];
			wh.trklen[trk] = wb->trklen[trk];
		}

		wh.trkcnt = wb->trkcnt;
	}

	return res;
}

                                      
PCAPSWH CCTRawCodec::DecompressTrack(PCAPSWH w, PUBYTE src, int slen, PUBYTE dst)
{
	CapsPack cpk;

	                   
	PCAPSPACK pk = GetPackHeader(&cpk, src, slen);
	if (!pk)
		return NULL;

	                                                   
	w->rawbuf = NULL;
	FreeUncompressedTrack(w);
	w->rawlen = pk->usize;
	w->rawbuf = dst;
	if (!w->rawbuf && w->rawlen)
		w->rawbuf = new UBYTE[w->rawlen];

	                    
	w->ctmem = src + sizeof(CapsPack);
	w->trkcnt = CTR(w, 1);

	                                                                       
	dst = w->rawbuf;
	for (int trk = 0; trk < w->trkcnt; trk++) {
		w->trklen[trk] = CTR(w, 2);
		w->trkbuf[trk] = dst;
		dst += w->trklen[trk];
	}

	                        
	if (w->trkcnt) {
		w->txsrc = w->trkbuf[0];
		w->txlen = w->trklen[0];
		memmove(w->txsrc, w->ctmem, w->txlen);
		w->ctmem += w->txlen;
	}

	                                                                 
	for (w->txact = 1; w->txact < w->trkcnt; w->txact++)
		DecompressTrackData(w);

	return w;
}

                               
void CCTRawCodec::DecompressTrackData(PCAPSWH w)
{
	                
	PUBYTE src = w->ctmem;

	                     
	PUBYTE dst = w->trkbuf[w->txact];

	                            
	PUBYTE max = dst + w->trklen[w->txact];

	                      
	PUBYTE mem = w->txsrc;

	                                               
	while (dst < max) {
		UDWORD size = *src++;

		if (size & 0x80) {
			             
			              
			int shift = size >> 4 & 7;

			              
			size = ((size & 0xf) << 8) | (*src++);

			                
			UDWORD ofs = *src++;
			ofs = (ofs << 8) | (*src++);
			PUBYTE buf = mem + ofs;

			if (shift) {
				                
				ofs = *buf++;
				while (size--) {
					ofs = (ofs << 8) | (*buf++);
					*dst++ = (UBYTE)(ofs >> shift);
				}
			} else {
				              
				while (size--)
					*dst++ = *buf++;
			}
		} else {
			                          
			size = (size << 8) | (*src++);
			while (size--)
				*dst++ = *src++;
		}
	}

	                                    
	w->ctmem = src;
}



                                                                  
PCAPSPACK CCTRawCodec::GetPackHeader(PCAPSPACK cpk, PUBYTE src, int slen)
{
	                    
	if (!src || slen<sizeof(CapsPack))
		return NULL;

	                  
	if (memcmp(src, CAPS_IDPACK, sizeof(cpk->sign)))
		return NULL;

	                        
	memcpy(cpk, src, sizeof(CapsPack));

	                                                 
	Swap(&cpk->hcrc, sizeof(cpk->hcrc));
	UDWORD hcrc = cpk->hcrc;
	cpk->hcrc = 0;
	if (hcrc != CalcCRC((PUBYTE)cpk, sizeof(CapsPack)))
		return NULL;

	                               
	Swap(PUDWORD(PUBYTE(cpk) + sizeof(cpk->sign)), sizeof(CapsPack) - sizeof(cpk->sign));

	                                                                       
	if ((UDWORD)slen != sizeof(CapsPack) + cpk->csize)
		return NULL;

	return cpk;
}

                                                                      
UDWORD CCTRawCodec::CTR(PCAPSWH w, int size)
{
	UDWORD res = 0;

	for (; size; size--) {
		res <<= 8;
		res |= *w->ctmem++;
	}

	return res;
}
