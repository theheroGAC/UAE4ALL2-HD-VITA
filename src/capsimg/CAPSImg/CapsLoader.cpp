#include "stdafx.h"



               
CCapsLoader::ChunkType CCapsLoader::chunklist[]= {
	CAPS_IDFILE, ccidFile,
	CAPS_IDDUMP, ccidDump,
	CAPS_IDDATA, ccidData,
	CAPS_IDTRCK, ccidTrck,
	CAPS_IDINFO, ccidInfo,
	CAPS_IDIMGE, ccidImge,
	CAPS_IDCTEX, ccidCtex,
	CAPS_IDCTEI, ccidCtei,
	NULL, ccidUnknown
};



CCapsLoader::CCapsLoader()
{
	MakeCRCTable();
	Unlock();
}

CCapsLoader::~CCapsLoader()
{
	Unlock();
}

                                       
int CCapsLoader::Lock(PCAPSFILE pcf)
{
	                      
	Unlock();

	            
	if (file.Open(pcf)) {
		Unlock();
		return ccidErrFile;
	}

	                
	readmode=(pcf->flag & CFF_WRITE) ? false : true;

	                              
	flen=file.GetSize();

	                    
	if (ReadChunk(true) != ccidFile) {
		Unlock();
		return ccidErrType;
	}

	return ccidCaps;
}

              
void CCapsLoader::Unlock()
{
	file.Close();
	flen=0;
	chunk.type=ccidUnknown;
	readmode=true;
}

                  
int CCapsLoader::ReadChunk(int idbrk)
{
	                    
	if (!file.IsOpen())
		return ccidErrFile;

	                        
	SkipData();

	            
	int pos=file.GetPosition();
	if (pos == flen)
		return ccidEof;

	                          
	if (flen-pos < sizeof(CapsID))
		return ccidErrShort;

	                
	if (file.Read((PUBYTE)&xchunk.file, sizeof(CapsID)) != sizeof(CapsID))
		return ccidErrShort;
	chunk.cg.file=xchunk.file;

	                                  
	int type=GetChunkType(&chunk);

	                                 
	if (idbrk && type==ccidUnknown)
		return ccidUnknown;

	                        
	Swap(PUDWORD((PUBYTE)&chunk.cg.file+sizeof(chunk.cg.file.name)), sizeof(CapsID)-sizeof(chunk.cg.file.name));

	                                                              
	int modsize=chunk.cg.file.size-sizeof(CapsID);
	if (modsize > 0) {
		                                
		pos=file.GetPosition();
		if (flen-pos < modsize)
			return ccidErrShort;

		                                                       
		if (modsize <= sizeof(CapsMod)) {
			                 
			if (file.Read((PUBYTE)&xchunk.mod, modsize) != modsize)
				return ccidErrShort;

			                            
			chunk.cg.mod=xchunk.mod;
			Swap(PUDWORD((PUBYTE)&chunk.cg.mod), modsize);
		} else {
			                    
			file.Seek(modsize, 0);
		}
	}

	                                              
	if (modsize>=0 && modsize<=sizeof(CapsMod)) {
		xchunk.file.hcrc=0;
		if (chunk.cg.file.hcrc != CalcCRC((PUBYTE)&xchunk.file, chunk.cg.file.size))
			return ccidErrHeader;
	}

	return type;
}

                               
int CCapsLoader::SkipData()
{
	                    
	if (!file.IsOpen())
		return 0;

	                                    
	int type=chunk.type;
	chunk.type=ccidUnknown;

	                      
	if (type != ccidData)
		return 0;

	                         
	int skip=chunk.cg.mod.data.size;
	if (!skip)
		return 0;

	                                      
	int pos=file.GetPosition();
	if (flen-pos < skip)
		skip=flen-pos;

	                    
	file.Seek(skip, 0);
	return skip;
}

                               
int CCapsLoader::ReadData(PUBYTE buf)
{
	                    
	if (!file.IsOpen())
		return 0;

	                                    
	int type=chunk.type;
	chunk.type=ccidUnknown;

	                      
	if (type != ccidData)
		return 0;

	                         
	int size=chunk.cg.mod.data.size;
	if (!size)
		return 0;

	                                          
	int pos=file.GetPosition();
	if (flen-pos < size) {
		size=flen-pos;

		                    
		file.Seek(size, 0);
		return 0;
	}

	                 
	if (file.Read(buf, size) != size)
		return 0;

	                              
	if (chunk.cg.mod.data.dcrc)
		if (chunk.cg.mod.data.dcrc != CalcCRC(buf, size))
			return 0;

	return size;
}

                                   
int CCapsLoader::GetDataSize()
{
	                    
	if (!file.IsOpen())
		return 0;

	                      
	if (chunk.type != ccidData)
		return 0;

	                
	return chunk.cg.mod.data.size;
}

                    
int CCapsLoader::GetPosition()
{
	                    
	if (!file.IsOpen())
		return 0;

	return file.GetPosition();
}

                    
int CCapsLoader::SetPosition(int pos)
{
	                    
	if (!file.IsOpen())
		return 0;

	                                
	chunk.type=ccidUnknown;

	                       
	if (pos < 0)
		pos=0;
	else
		if (flen < pos)
			pos=flen;

	       
	file.Seek(pos, -1);

	return pos;
}

                  
int CCapsLoader::GetChunkType(PCAPSCHUNK pc)
{
	int pos;

	for (pos=0; chunklist[pos].name; pos++)
		if (!memcmp(chunklist[pos].name, pc->cg.file.name, sizeof(pc->cg.file.name)))
			break;

	return pc->type=chunklist[pos].type;
}

                                                 
void CCapsLoader::ConvertChunk(PCAPSCHUNK pc)
{
	if (!pc)
		return;

	                                           
	int ofs=sizeof(pc->cg.file.name);
	int size=pc->cg.file.size;
	Swap(PUDWORD((PUBYTE)&pc->cg.file+ofs), size-ofs);

	                                              
	pc->cg.file.hcrc=0;
	pc->cg.file.hcrc=CalcCRC((PUBYTE)&pc->cg.file, size);
	CCapsLoader::Swap(&pc->cg.file.hcrc, sizeof(pc->cg.file.hcrc));
}

                                 
void CCapsLoader::Swap(PUDWORD buf, int cnt)
{
#ifdef INTEL
	for (cnt>>=2; cnt > 0; cnt--, buf++) {
		UDWORD src=*buf;
		UDWORD dst=_lrotl(src, 8)&0x00ff00ff | _lrotr(src, 8)&0xff00ff00;
		*buf=dst;
	}
#endif
}
