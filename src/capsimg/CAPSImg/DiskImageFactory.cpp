#include "stdafx.h"



CDiskImageFactory::CDiskImageFactory()
{
	                                                                                 
}

CDiskImageFactory::~CDiskImageFactory()
{
}

                                                                                              
int CDiskImageFactory::GetImageType(PCAPSFILE pcf)
{
	                                 
	CCapsFile file;
	if (file.Open(pcf))
		return citError;

	                      
	int type = IsCAPSImage(pcf);
	if (type != citUnknown)
		return type;

	                            
	type = IsKFStreamCue(pcf);
	if (type != citUnknown)
		return type;

	                        
	type = IsKFStream(pcf);
	if (type != citUnknown)
		return type;

	                                                       
	return citUnknown;
}

                                                        
int CDiskImageFactory::IsCAPSImage(PCAPSFILE pcf)
{
	                             
	CCapsLoader cload;
	int res = cload.Lock(pcf);

	                                              
	if (res == CCapsLoader::ccidCaps) {
		for (int run = 1; run;) {
			                                       
			int type = cload.ReadChunk();

			switch (type) {
				                       
				case CCapsLoader::ccidEof:
				case CCapsLoader::ccidErrFile:
				case CCapsLoader::ccidErrType:
				case CCapsLoader::ccidErrShort:
				case CCapsLoader::ccidErrHeader:
				case CCapsLoader::ccidErrData:
					run = 0;
					continue;

				                                 
				case CCapsLoader::ccidTrck:
					return citCTRaw;

				                             
				case CCapsLoader::ccidImge:
					return citIPF;

				                     
				default:
					continue;
			}
		}
	}

	                                    
	return citUnknown;
}

                                                          
int CDiskImageFactory::IsKFStream(PCAPSFILE pcf)
{
	                                
	CCapsFile file;
	if (file.Open(pcf))
		return citError;

	C2OOBHdr oob;
	uint8_t buf[512];

	                      
	int frem = file.GetSize();

	                                                                
	while (true) {
		                                                            
		int rlen = sizeof(oob);
		if (rlen > frem)
			break;

		                                 
		if (file.Read((PUBYTE)&oob, rlen) != rlen)
			return citError;

		                                 
		frem -= rlen;

		                         
		if (oob.sign != c2eOOB || oob.type != c2otInfo)
			break;

		                                        
		uint8_t *po = (uint8_t *)&oob;
		int so = offsetof(C2OOBHdr, size);
		int infosize = po[so+1] << 8 | po[so];

		                                                                                      
		if (!infosize || infosize > frem || infosize > sizeof(buf))
			break;
		
		                           
		if (file.Read(buf, infosize) != infosize)
			return citError;

		                                 
		frem -= infosize;

		                                          
		if (strstr((char *)buf, "KryoFlux"))
			return citKFStream;
	}

	                                                  
	return citUnknown;
}

                                                              
int CDiskImageFactory::IsKFStreamCue(PCAPSFILE pcf)
{
	                                
	CCapsFile file;
	if (file.Open(pcf))
		return citError;

	                                                                        
	uint8_t buf[256];

	                      
	int frem = file.GetSize();

	                                              
	int bufmax = sizeof(buf)-1;

	int readsize = min(frem, bufmax);

	                                                     
	if (file.Read(buf, readsize) != readsize)
		return citError;

	                    
	buf[readsize] = 0;

	                                    
	if (strstr((char *)buf, KF_STREAM_CUE_ID))
		return citKFStreamCue;

	                                    
	return citUnknown;
}

                                                            
PCDISKIMAGE CDiskImageFactory::CreateImage(int diftype)
{
	PCDISKIMAGE pdi = NULL;

	                                  
	switch (diftype) {
		case citIPF:
			pdi = new CCapsImageStd;
			break;

		case citCTRaw:
			pdi = new CCapsImage;
			break;

		case citKFStream:
			pdi = new CStreamImage;
			break;

		case citKFStreamCue:
			pdi = new CStreamCueImage;
			break;
	}

	return pdi;
}

