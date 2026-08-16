#include "config.h"
#include "stdafx.h"

CDiskFile::CDiskFile()
{
	dfile=NULL;
	lastop=-1;
}

CDiskFile::~CDiskFile()
{
	Close();
}

                                  
int CDiskFile::Open(const char *name, unsigned int mode)
{
	                           
	Close();

	             
	if (!name || !strlen(name))
		return 1;

	                   
	const char *om;

	if (mode & BFFLAG_WRITE) {
		if (mode & BFFLAG_CREATE)
			om="w+b";
		else
			om="r+b";
	} else
		om="rb";

	                           
	if (!(dfile=fopen(name, om)))
		return 1;

	          
	fileopen=1;
	filemode=mode;

	return 0;
}

                                                    
                                                                                
int CDiskFile::OpenAny(char **name, unsigned int mode)
{
	int pos;

	                        
	if (name) {
		                         
		for (pos=0; name[pos]; pos++) {
			                                                           
			if (!Open(name[pos], mode))
				return pos;
		}
	}

	                      
	return -1;
}

                                                
                                                                                
int CDiskFile::OpenAnyPath(char **path, const char *name, unsigned int mode)
{
	int pos;

	                                  
	if (name && path) {
		                               
		for (pos=0; path[pos]; pos++) {
			                                    
			int len=sprintf(tempname, "%s", path[pos]);
			sprintf(tempname+len, "%s", name);

			                                                           
			if (!Open(tempname, mode))
				return pos;
		}
	}

	                      
	return -1;
}

                                   
int CDiskFile::Close()
{
	                               
	if (!dfile)
		return 0;

	                        
	int res=fclose(dfile) ? 1 : 0;

	                 
	dfile=NULL;
	lastop=-1;
	Clear();

	return res;
}

                                         
size_t CDiskFile::Read(void *buf, size_t size)
{
	                                   
	if (!dfile)
		return 0;

	                    
	if (lastop != 0) {
		fseek(dfile, 0, SEEK_CUR);
		lastop=0;
	}

	                
	return fread(buf, 1, size, dfile);
}

                                             
size_t CDiskFile::Write(void *buf, size_t size)
{
	                                                   
	if (!dfile || !(filemode & BFFLAG_WRITE))
		return 0;

	                    
	if (lastop != 1) {
		fseek(dfile, 0, SEEK_CUR);
		lastop=1;
	}

	                
	return fwrite(buf, 1, size, dfile);
}

                                            
long CDiskFile::Seek(long pos, int mode)
{
	                  
	long res=0;

	                                            
	if (!dfile)
		return res;

	                   
	int sm;

	switch (mode) {
		case Start:
			pos=0;
			sm=SEEK_SET;
			break;

		case Position:
			sm=SEEK_SET;
			break;

		case Current:
			sm = SEEK_CUR;
			break;

		case End:
			pos=0;
			sm=SEEK_END;
			break;

		default:
			return res;
	}

	                      
	if (fseek(dfile, pos, sm))
		return res;

	                                               
	pos=ftell(dfile);
	if (pos < 0)
		return res;

	return pos;
}

                    
long CDiskFile::GetSize()
{
	                  
	long res=0;

	                                            
	if (!dfile)
		return res;

	                                               
	long pos=ftell(dfile);
	if (pos < 0)
		return res;

	                    
	long size=Seek(0, End);

	                                       
	if (Seek(pos, Position) != pos)
		return res;

	return size;
}

                    
long CDiskFile::GetPosition()
{
	                  
	long res=0;

	                                            
	if (!dfile)
		return res;

	long pos=ftell(dfile);
	if (pos < 0)
		return res;

	return pos;
}

                      
uint8_t *CDiskFile::GetBuffer()
{
	                
	return NULL;
}

                                                               
void CDiskFile::MakePath(const char *filename)
{
	if (!filename)
		return;

	char path[MAX_FILENAMELEN];

	                                                                    
	for (int rpos = 0, wpos = 0; filename[rpos]; rpos++) {
		if (filename[rpos] == '/' || filename[rpos] == '\\') {
			path[wpos] = 0;

			if (_access(path, 0) == -1)
				_mkdir(path);
		}

		path[wpos++] = filename[rpos];
	}
}

                                                                                                        
                              
int CDiskFile::FindFile(char *result, const char *filename, const char *filter)
{
	                                 
	int resvalid = 0;

	                                   
	int rescopy = 1;

	                                             
	if (!result)
		return resvalid;

	                                        
	result[0] = 0;

	                                        
	if (!filename)
		return resvalid;

	                                                    
	int pathlen = 0, namelen = 0;

	                                            
	int wcused = 0;
	int wcenabled = 1;

	                                                
	for (int rpos = 0; wcenabled && filename[rpos]; rpos++) {
		namelen++;

		switch (filename[rpos]) {
			                          
		case '/':
		case '\\':
			                                                                                                  
			if (wcused) {
				                                                                                               
				wcenabled = 0;
				break;
			}

			                                                                           
			pathlen = rpos + 1;

			                          
			namelen = 0;
			break;

			                      
		case '?':
		case '*':
			wcused++;
			break;
		}
	}

	                    
	if (wcused) {
		                                                        
		if (wcenabled) {
			char *pathbuf = NULL;
			const char *dirpath;

			                                                                                   
			if (pathlen) {
				pathbuf = new char[pathlen + 1];
				memcpy(pathbuf, filename, pathlen);
				pathbuf[pathlen] = 0;
				dirpath = pathbuf;
			} else
				dirpath = ".";

			                         
			DIR *pdir = opendir(dirpath);
			if (pdir) {
				dirent *pent;
				const char *pattern = filename + pathlen;

				                                
				while (pent = readdir(pdir)) {
					                                            
#if defined _DIRENT_HAVE_D_TYPE || defined HAVE_STRUCT_DIRENT_D_TYPE
					if (pent->d_type != DT_REG)
						continue;
#endif
					                                            
					char *fn = pent->d_name;
					if (FileNameMatch(pattern, fn)) {
						                                                                             
						if (!filter || FileNameMatch(filter, fn)) {
							                                                          
							memcpy(result, filename, pathlen);
							strcpy(result + pathlen, fn);

							                                          
							rescopy = 0;

							                  
							resvalid = 1;
							break;
						}
					}
				}

				closedir(pdir);
			}

			                   
			delete[] pathbuf;
		}
	} else {
		                                   
		resvalid = 1;
	}

	                                       
	if (rescopy) {
		int reslen = pathlen + namelen;
		memcpy(result, filename, reslen);
		result[reslen] = 0;
	}

	return resvalid;
}

                                                       
int CDiskFile::FileNameMatch(const char *pattern, const char *filename)
{
	                                                 
	if (!pattern || !filename)
		return 0;

	                                                  
	int starmode = 0;

	                                                                               
	while (pattern[0] == '*') {
		                           
		starmode = 1;

		                                                                  
		pattern++;
	}

	                                
	int patternlength = 0;

	                                                                                                      
	while (pattern[patternlength]) {
		if (pattern[patternlength] == '*')
			break;

		patternlength++;
	}

	                                                                 
	if (!patternlength && starmode)
		return 1;

	                         
	int namelength = (int)strlen(filename);

	                                                                          
	if (!patternlength && namelength)
		return 0;

	                                                                     
	                                                                                          
	                                                                
	for (int namepos = 0; namepos < namelength; namepos++) {
		                                                                        
		if (patternlength > namelength - namepos)
			return 0;

		int patpos;

		                                                                            
		for (patpos = 0; patpos < patternlength; patpos++) {
			                          
			if (pattern[patpos] == '?')
				continue;

			                                                                    
			if (tolower((unsigned char)pattern[patpos]) != tolower((unsigned char)filename[patpos+namepos]))
				break;
		}

		if (patpos >= patternlength) {
			                                        
			                                              
			pattern += patternlength;

			                                               
			filename += patternlength + namepos;

			                                           
			namelength -= patternlength + namepos;

			                        
			patternlength = 0;

			                                                                                                          
			if (pattern[0])
				return FileNameMatch(pattern, filename);
			else
				break;
		} else {
			                                                                   
			                                                                                                      
			                                                                        
			if (!starmode)
				return 0;
		}
	}

	                                                                                                  
	return !patternlength && !namelength;
}
