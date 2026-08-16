#include "stdafx.h"



CMemoryFile::CMemoryFile()
{
	Clear();
}

CMemoryFile::~CMemoryFile()
{
	Free();
}

                                  
int CMemoryFile::Open(void *buf, size_t size, unsigned int mode)
{
	                
	Clear(0);

	                                                      
	if (mode & BFFLAG_CREATE) {
		                    
		if (size) {
			                                                                        
			AllocBuffer(size);

			                                   
			if (buf) {
				memcpy(filebuf[mtAlloc], buf, size*sizeof(uint8_t));
				filecount=size;
			}
		}

		                         
		filemt=mtAlloc;
	} else {
		                                 
		if (size) {
			                                                   
			if (!buf)
				return 1;
		} else
			buf=NULL;

		                            
		filemt=mtUser;
		filebuf[mtUser]=(uint8_t *)buf;
		filesize[mtUser]=size;
		filecount=size;
	}

	                         
	filepos=0;

	          
	fileopen=1;
	filemode=mode;

	return 0;
}

                                                                                                                 
int CMemoryFile::Close()
{
	                                                  
	if (filemt == mtLast)
		Free();
	else
		Clear(0);

	return 0;
}

                                       
void CMemoryFile::Free()
{
	FreeBuffer();
	Clear();
}

                                                                    
void CMemoryFile::AllocBuffer(size_t maxsize)
{
	if (filesize[mtAlloc] < maxsize) {
		maxsize+=DEF_MEMORYFILEALLOC;
		uint8_t *newfile=new uint8_t[maxsize];
		size_t oldcount=filecount;
		size_t oldpos=filepos;

		if (oldcount)
			memcpy(newfile, filebuf[mtAlloc], oldcount*sizeof(uint8_t));

		FreeBuffer();

		filebuf[mtAlloc]=newfile;
		filesize[mtAlloc]=maxsize;
		filecount=oldcount;
		filepos=oldpos;
	}
}

                               
void CMemoryFile::FreeBuffer()
{
	filesize[mtAlloc]=0;
	filecount=0;
	filepos=0;

	delete [] filebuf[mtAlloc];
	filebuf[mtAlloc]=NULL;
}

                 
void CMemoryFile::Clear(int clbuf)
{
	if (clbuf) {
		filebuf[mtAlloc]=NULL;
		filesize[mtAlloc]=0;

		filebuf[mtUser]=NULL;
		filesize[mtUser]=0;
	}

	filecount=0;
	filepos=0;

	filemt=mtLast;

	CBaseFile::Clear();
}

                                         
size_t CMemoryFile::Read(void *buf, size_t size)
{
	                                                                
	if (!buf || !size || filemt == mtLast)
		return 0;

	                                                 
	if (filecount-filepos < size)
		size=filecount-filepos;

	                                         
	if (size) {
		memcpy(buf, filebuf[filemt]+filepos, size);
		filepos+=size;
	}

	return size;
}

                                             
size_t CMemoryFile::Write(void *buf, size_t size)
{
	                                                                                    
	if (!buf || !size || filemt == mtLast || !(filemode & BFFLAG_WRITE))
		return 0;

	                                         
	if (filemt == mtAlloc)
		AllocBuffer(filepos+size);

	                                      
	if (filesize[filemt]-filepos < size)
		size=filesize[filemt]-filepos;

	                                          
	if (size) {
		memcpy(filebuf[filemt]+filepos, buf, size);
		filepos+=size;

		                                                                 
		if (filepos > filecount)
			filecount=filepos;
	}

	return size;
}

                                            
long CMemoryFile::Seek(long pos, int mode)
{
	                  
	size_t res=0, tmppos;

	                                            
	if (filemt == mtLast)
		return (long)res;

	               
	switch (mode) {
		case Start:
			filepos=0;
			break;

		case Position:
			if (pos >= 0 && pos <= (long)filecount)
				filepos=pos;
			break;

		case Current:
			tmppos=filepos+pos;
			if (tmppos <= filecount)
				filepos = tmppos;
			break;

		case End:
			filepos=filecount;
			break;

		default:
			return (long)res;
	}

	return (long)filepos;
}

                    
long CMemoryFile::GetSize()
{
	return (long)filecount;
}

                    
long CMemoryFile::GetPosition()
{
	return (long)filepos;
}

                      
uint8_t *CMemoryFile::GetBuffer()
{
	return (filemt == mtLast) ? NULL : filebuf[filemt];
}
