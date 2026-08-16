#include "stdafx.h"



CCTRawCodec::CCTRawCodec()
{
	Clear();
}

CCTRawCodec::~CCTRawCodec()
{
	Free();
}

                  
void CCTRawCodec::Clear()
{
	memset(&wh, 0, sizeof(wh));
}

                             
void CCTRawCodec::Free()
{
	FreeCompressedDensity();
	FreeUncompressedDensity();
	FreeCompressedTrack();
	FreeUncompressedTrack();
	Clear();
}

                                             
void CCTRawCodec::FreeCompressedDensity()
{
	delete [] wh.cdbuf;
	wh.cdbuf = NULL;
	wh.cdlen = 0;
}

                                               
void CCTRawCodec::FreeUncompressedDensity()
{
	delete [] wh.timbuf;
	wh.timbuf = NULL;
	wh.timlen = 0;
}

                                           
void CCTRawCodec::FreeCompressedTrack()
{
	delete [] wh.ctbuf;
	wh.ctbuf = NULL;
	wh.ctlen = 0;
}

                                             
void CCTRawCodec::FreeUncompressedTrack()
{
	FreeUncompressedTrack(&wh);
}

                                             
void CCTRawCodec::FreeUncompressedTrack(PCAPSWH w)
{
	delete[] w->rawbuf;
	w->rawbuf = NULL;
	w->rawlen = 0;

	for (int trk = 0; trk < CAPS_MTRS; trk++) {
		w->trkbuf[trk] = NULL;
		w->trklen[trk] = 0;
	}

	w->trkcnt = 0;
}



                                 
void CCTRawCodec::Swap(PUDWORD buf, int cnt)
{
#ifdef INTEL
	for (cnt >>= 2; cnt > 0; cnt--, buf++) {
		UDWORD src = *buf;
		UDWORD dst = _lrotl(src, 8) & 0x00ff00ff | _lrotr(src, 8) & 0xff00ff00;
		*buf = dst;
	}
#endif
}

