#include "stdafx.h"



                        
SDWORD __cdecl CAPSFormatDataToMFM(PVOID pformattrack, UDWORD flag)
{
	if (!pformattrack)
		return imgeGeneric;

	                                                         
	unsigned rev=0;
	if (flag & DI_LOCK_TYPE)
		rev=*(PUDWORD)pformattrack;

	                                                        
	if (rev > 0) {
		*(PUDWORD)pformattrack=0;
		return imgeUnsupportedType;
	}

	PCAPSFORMATTRACK pf=(PCAPSFORMATTRACK)pformattrack;

	                                                     
	if (!pf->trackbuf || !pf->tracklen || !pf->buflen)
		return FmfmGetSize(pf);

	                                                  
	if (pf->tracklen > pf->buflen)
		return imgeBufferShort;

	                                             
	int res=FmfmGetSize(pf);

	                                   
	if (res != imgeOk)
		return res;

	                                 
	if (pf->tracklen < pf->bufreq)
		return imgeBufferShort;

	                                          
	if (pf->startpos >= pf->tracklen)
		return imgeBadDataStart;

	                   
	return FmfmConvert(pf);
}



                                
int FmfmGetSize(PCAPSFORMATTRACK pf)
{
	                   
	UDWORD len=0;
	pf->bufreq=0;

	                 
	len+=pf->gapacnt;

	                           
	if (pf->blockcnt && !pf->block)
		return imgeGeneric;

	                            
	for (int blk=0; blk < pf->blockcnt; blk++) {
		            
		PCAPSFORMATBLOCK pb=pf->block+blk;

		                
		len+=pb->gapacnt;
		len+=pb->gapbcnt;
		len+=pb->gapccnt;
		len+=pb->gapdcnt;

		                 
		int sv=false;
		switch (pb->blocktype) {
			                       
			case cfrmbtIndex:
				len+=4;
				break;

			                                                       
			case cfrmbtData:
				len+=16;
				len+=pb->sectorlen;
				sv=true;
				break;

			default:
				return imgeBadBlockType;
		}

		                                           
		if (sv && FmfmSectorLength(pb->sectorlen)<0)
			return imgeBadBlockSize;
	}

	                            
	pf->bufreq=len*2;

	return imgeOk;
}



                            
int FmfmConvert(PCAPSFORMATTRACK pf)
{
	             
	pf->size=0;

	                                   
	UDWORD state=(0)<<15 ^ 0xffff;

	                  
	state=FmfmWriteDataByte(pf, state, pf->gapavalue, pf->gapacnt);

	                     
	for (int blk=0; blk < pf->blockcnt; blk++) {
		            
		PCAPSFORMATBLOCK pb=pf->block+blk;

		                   
		switch (pb->blocktype) {
			                           
			case cfrmbtIndex:
				state=FmfmWriteBlockIndex(pf, state, pb);
				break;

			                                                       
			case cfrmbtData:
				state=FmfmWriteBlockData(pf, state, pb);
				break;
		}
	}

	                              
	int gapbcnt=(pf->tracklen-pf->size)/2;
	if (gapbcnt > 0)
		state=FmfmWriteDataByte(pf, state, pf->gapbvalue, gapbcnt);

	return imgeOk;
}

                                   
UDWORD FmfmWriteDataByte(PCAPSFORMATTRACK pf, UDWORD state, UDWORD value, int count)
{
	pf->size+=count*2;
	UDWORD pos=pf->startpos;

	                  
	while (count--) {
		UDWORD val=CDiskEncoding::mfmcode[value & 0xff] & state;
		state=(val & 1)<<15 ^ 0xffff;

		pf->trackbuf[pos++]=UBYTE(val>>8);
		if (pos >= pf->tracklen)
			pos=0;

		pf->trackbuf[pos++]=(UBYTE)val;
		if (pos >= pf->tracklen)
			pos=0;
	}

	pf->startpos=pos;

	return state;
}

                                   
UDWORD FmfmWriteMarkByte(PCAPSFORMATTRACK pf, UDWORD state, UDWORD value, int count)
{
	pf->size+=count*2;
	UDWORD pos=pf->startpos;
	UDWORD val=value & 0xffff;
	state=(val & 1)<<15 ^ 0xffff;

	                  
	while (count--) {
		pf->trackbuf[pos++]=UBYTE(val>>8);
		if (pos >= pf->tracklen)
			pos=0;

		pf->trackbuf[pos++]=(UBYTE)val;
		if (pos >= pf->tracklen)
			pos=0;
	}

	pf->startpos=pos;

	return state;
}

                
UWORD FmfmCrc(UWORD crc, UDWORD value, int count)
{
	value&=0xff;

	while (count--)
		crc=crctab_ccitt[value^(crc>>8)] ^ (crc << 8);

	return crc;
}

                                
int FmfmSectorLength(int value)
{
	switch (value) {
		case 128:
			return 0;

		case 256:
			return 1;

		case 512:
			return 2;

		case 1024:
			return 3;
	}

	return -1;
}

                    
UDWORD FmfmWriteBlockIndex(PCAPSFORMATTRACK pf, UDWORD state, PCAPSFORMATBLOCK pb)
{
	                      
	state=FmfmWriteDataByte(pf, state, pb->gapavalue, pb->gapacnt);

	               
	state=FmfmWriteMarkByte(pf, state, 0x5224, 3);

	     
	state=FmfmWriteDataByte(pf, state, 0xfc, 1);

	                     
	state=FmfmWriteDataByte(pf, state, pb->gapbvalue, pb->gapbcnt);

	return state;
}

                   
UDWORD FmfmWriteBlockData(PCAPSFORMATTRACK pf, UDWORD state, PCAPSFORMATBLOCK pb)
{
	UWORD crc;

	                
	int seclen=FmfmSectorLength(pb->sectorlen);

	                  
	                      
	state=FmfmWriteDataByte(pf, state, pb->gapavalue, pb->gapacnt);

	               
	crc=~0;
	state=FmfmWriteMarkByte(pf, state, 0x4489, 3);
	crc=FmfmCrc(crc, 0xa1, 3);

	     
	state=FmfmWriteDataByte(pf, state, 0xfe, 1);
	crc=FmfmCrc(crc, 0xfe);

	         
	state=FmfmWriteDataByte(pf, state, pb->track, 1);
	crc=FmfmCrc(crc, pb->track);

	        
	state=FmfmWriteDataByte(pf, state, pb->side, 1);
	crc=FmfmCrc(crc, pb->side);

	          
	state=FmfmWriteDataByte(pf, state, pb->sector, 1);
	crc=FmfmCrc(crc, pb->sector);

	                
	state=FmfmWriteDataByte(pf, state, seclen, 1);
	crc=FmfmCrc(crc, seclen);

	      
	state=FmfmWriteDataByte(pf, state, crc>>8, 1);
	state=FmfmWriteDataByte(pf, state, crc, 1);

	                     
	state=FmfmWriteDataByte(pf, state, pb->gapbvalue, pb->gapbcnt);

	                    
	                       
	state=FmfmWriteDataByte(pf, state, pb->gapcvalue, pb->gapccnt);

	               
	crc=~0;
	state=FmfmWriteMarkByte(pf, state, 0x4489, 3);
	crc=FmfmCrc(crc, 0xa1, 3);

	     
	state=FmfmWriteDataByte(pf, state, 0xfb, 1);
	crc=FmfmCrc(crc, 0xfb);

	       
	for (int i=0; i < pb->sectorlen; i++) {
		UDWORD data=pb->databuf ? pb->databuf[i] : pb->datavalue;
		state=FmfmWriteDataByte(pf, state, data, 1);
		crc=FmfmCrc(crc, data);
	}

	      
	state=FmfmWriteDataByte(pf, state, crc>>8, 1);
	state=FmfmWriteDataByte(pf, state, crc, 1);

	                      
	state=FmfmWriteDataByte(pf, state, pb->gapdvalue, pb->gapdcnt);

	return state;
}

