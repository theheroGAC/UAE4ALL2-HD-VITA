#ifndef MEMORYFILE_H
#define MEMORYFILE_H

#define DEF_MEMORYFILEALLOC 512



                                  
class CMemoryFile : public CBaseFile
{
public:
	CMemoryFile();
	virtual ~CMemoryFile();
	int Open(void *buf, size_t size, unsigned int mode);
	int Close();
	void Free();
	size_t Read(void *buf, size_t size);
	size_t Write(void *buf, size_t size);
	long Seek(long pos, int mode);
	long GetSize();
	long GetPosition();
	uint8_t *GetBuffer();

protected:
	              
	enum {
		mtAlloc,
		mtUser,
		mtLast
	};

	void Clear(int clbuf=1);
	void AllocBuffer(size_t maxsize);
	void FreeBuffer();

protected:
	int filemt;                    
	uint8_t *filebuf[mtLast];               
	size_t filesize[mtLast];               
	size_t filecount;                              
	size_t filepos;                           
};

typedef CMemoryFile *PCMEMORYFILE;

#endif
