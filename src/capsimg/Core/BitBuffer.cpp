#include "stdafx.h"



CBitBuffer::CBitBuffer()
{
	Clear();
}

CBitBuffer::~CBitBuffer()
{
}

                  
void CBitBuffer::Clear()
{
	bufmem = NULL;
	bufsize = 0;
	bufbits = 0;
}

                                                
void CBitBuffer::InitByteSize(uint8_t *buf, uint32_t bytesize)
{
	bufmem = buf;
	bufsize = bytesize;
	bufbits = bytesize << 3;
}

                                               
void CBitBuffer::InitBitSize(uint8_t *buf, uint32_t bitsize)
{
	bufmem = buf;
	bufsize = CalculateByteSize(bitsize);
	bufbits = bitsize;
}



                                                                 
uint32_t CBitBuffer::ReadBitWrap(uint8_t *buf, uint32_t bufwrap, uint32_t bitpos, int bitcnt)
{
	uint32_t res;

	                                                                                     
	if (bitpos + bitcnt > bufwrap) {
		                             
		res = 0;

		                
		while (bitcnt-- > 0) {
			                        
			res <<= 1;
			if (ReadBit(buf, bitpos))
				res |= 1;

			                       
			bitpos++;

			                            
			if (bitpos >= bufwrap)
				bitpos -= bufwrap;
		}
	} else {
		                          
		res = ReadBit(buf, bitpos, bitcnt);
	}

	return res;
}

                                                    
uint32_t CBitBuffer::ReadBit(uint8_t *buf, uint32_t bitpos, int bitcnt)
{
	                                        
	if (bitcnt <= 0)
		return 0;

	uint32_t res = 0;

	                        
	uint32_t bytepos = bitpos >> 3;

	                            
	uint8_t bitmask = 1 << ((bitpos & 7) ^ 7);

	                                
	uint8_t value = buf[bytepos++];

	                
	while (bitcnt-- > 0) {
		                                  
		if (!bitmask) {
			                                         
			bitmask = 0x80;

			                                  
			value = buf[bytepos++];
		}

		                          
		res <<= 1;
		if (value & bitmask)
			res |= 1;

		                                    
		bitmask >>= 1;
	}

	return res;
}



                                                                
void CBitBuffer::WriteBitWrap(uint8_t *buf, uint32_t bufwrap, uint32_t bitpos, uint32_t value, int bitcnt)
{
	                                                                                       
	if (bitpos + bitcnt > bufwrap) {
		                                                                

		                                                             
		if (bitcnt <= 0)
			return;

		                                                             
		uint32_t rbit = 1 << (bitcnt - 1);

		                
		uint32_t wbit = 1 << ((bitpos & 7) ^ 7);

		                        
		uint8_t *wpos = buf + (bitpos >> 3);

		                        
		uint32_t data = *wpos;

		                   
		while (rbit) {
			                                                                                                     
			if (value & rbit)
				data |= wbit;
			else
				data &= ~wbit;

			                                          
			rbit >>= 1;
			bitpos++;

			if (bitpos == bufwrap) {
				                            
				bitpos = 0;
				*wpos = (uint8_t)data;

				                            
				wpos = buf;
				wbit = 0x80;
				data = *wpos;
			} else {
				if (!(wbit >>= 1)) {
					                                             
					*wpos = (uint8_t)data;
					wpos++;
					wbit = 0x80;
					data = *wpos;
				}
			}
		}

		              
		*wpos = (uint8_t)data;
	} else {
		                           
		WriteBit(buf, bitpos, value, bitcnt);
	}
}

                                                   
void CBitBuffer::WriteBit(uint8_t *buf, uint32_t bitpos, uint32_t value, int bitcnt)
{
	                        
	uint8_t *wpos = buf + (bitpos >> 3);

	while (bitcnt > 0) {
		                           
		int bofs = bitpos & 7;

		                                            
		int bcnt = 8 - bofs;
		if (bcnt > bitcnt)
			bcnt = bitcnt;

		                    
		bitpos += bcnt;

		                 
		bitcnt -= bcnt;

		                                                            
		uint32_t data = value >> bitcnt;

		                                 
		if (bcnt != 8) {
			                                                          
			data <<= (8 - (bofs + bcnt));

			                                              
			uint32_t mask = (0xff00 >> bcnt) & 0xff;

			                                                              
			mask >>= bofs;

			                                                             
			data &= mask;

			                                      
			data |= (*wpos & ~mask);
		}

		              
		*wpos++ = (uint8_t)data;
	}
}



                                                       
void CBitBuffer::ClearBitWrap(uint8_t *buf, uint32_t bufwrap, uint32_t bitpos, int bitcnt)
{
	                                                                                       
	if (bitpos + bitcnt > bufwrap) {
		while (bitcnt > 0) {
			                                  
			int writebc = (bitcnt >= MAX_BITBUFFER_LEN) ? MAX_BITBUFFER_LEN : bitcnt;

			                      
			WriteBitWrap(buf, bufwrap, bitpos, 0, writebc);

			                      
			bitcnt -= writebc;
			bitpos += writebc;

			                            
			if (bitpos >= bufwrap)
				bitpos -= bufwrap;
		}
	}	else {
		                           
		ClearBit(buf, bitpos, bitcnt);
	}
}

                                          
void CBitBuffer::ClearBit(uint8_t *buf, uint32_t bitpos, int bitcnt)
{
	while (bitcnt > 0) {
		                                  
		int writebc = (bitcnt >= MAX_BITBUFFER_LEN) ? MAX_BITBUFFER_LEN : bitcnt;

		                      
		WriteBit(buf, bitpos, 0, writebc);

		                      
		bitcnt -= writebc;
		bitpos += writebc;
	}
}



                                                                                 
int CBitBuffer::CompareBit(uint8_t *buf1, uint32_t buf1pos, uint8_t *buf2, uint32_t buf2pos, int bitcnt)
{
	                                                                                        
	while (bitcnt > 0) {
		uint32_t s1, s2;

		if (bitcnt >= 32) {
			                                                                          
			s1 = ReadBit32(buf1, buf1pos);
			s2 = ReadBit32(buf2, buf2pos);
			bitcnt -= 32;
			buf1pos += 32;
			buf2pos += 32;
		} else {
			                                 
			s1 = ReadBit(buf1, buf1pos, bitcnt);
			s2 = ReadBit(buf2, buf2pos, bitcnt);
			bitcnt = 0;
		}

		                                
		if (s1 != s2)
			return -1;
	}

	                                              
	return 0;
}

                                                                                                  
int CBitBuffer::CompareAndCountBit(uint8_t *buf1, uint32_t buf1pos, uint8_t *buf2, uint32_t buf2pos, int bitcnt)
{
	                                                  
	int bitproc = 0, blocksize;
	uint32_t diff;

	                                                                                               
	while (bitcnt > 0) {
		uint32_t s1, s2;

		if (bitcnt >= 32) {
			                                                             
			blocksize = 32;

			                                
			s1 = ReadBit32(buf1, buf1pos);
			s2 = ReadBit32(buf2, buf2pos);
		} else {
			                                    
			blocksize = bitcnt;

			                                                 
			s1 = ReadBit(buf1, buf1pos, blocksize);
			s2 = ReadBit(buf2, buf2pos, blocksize);
		}

		                                                        
		diff = s1 ^ s2;

		                                                     
		if (diff)
			break;

		                            
		buf1pos += blocksize;
		buf2pos += blocksize;
		bitproc += blocksize;
		bitcnt -= blocksize;
	}

	                                                       
	if (bitcnt > 0) {
		                                                 
		for (uint32_t mask = 1U << (blocksize - 1); mask; mask >>= 1) {
			if (diff & mask)
				break;

			                                     
			bitproc++;
		}
	}

	                                                                
	return bitproc;
}



                                                        
void CBitBuffer::CopyBitWrap(uint8_t *srcbuf, uint32_t srcwrap, uint32_t srcpos, uint8_t *dstbuf, uint32_t dstwrap, uint32_t dstpos, int bitcnt)
{
	                                                                  
	while (bitcnt > 0) {
		                                                                      
		int csize = (dstpos + bitcnt > dstwrap) ? dstwrap - dstpos : bitcnt;

		                                                                 
		if (srcpos + csize > srcwrap)
			csize = srcwrap - srcpos;

		                                                                             
		CopyBit(srcbuf, srcpos, dstbuf, dstpos, csize);

		                                
		srcpos += csize;
		dstpos += csize;
		bitcnt -= csize;

		                            
		if (srcpos >= srcwrap)
			srcpos -= srcwrap;

		if (dstpos >= dstwrap)
			dstpos -= dstwrap;
	}
}

void CBitBuffer::CopyBit(uint8_t *srcbuf, uint32_t srcpos, uint8_t *dstbuf, uint32_t dstpos, int bitcnt)
{
	                                                                         
	while (bitcnt > 0) {
		uint32_t value;
		int blocksize;

		if (bitcnt >= 32) {
			                                                             
			blocksize = 32;

			                           
			value = ReadBit32(srcbuf, srcpos);
		} else {
			                                    
			blocksize = bitcnt;

			                                                 
			value = ReadBit(srcbuf, srcpos, blocksize);
		}

		                  
		WriteBit(dstbuf, dstpos, value, blocksize);

		                            
		srcpos += blocksize;
		dstpos += blocksize;
		bitcnt -= blocksize;
	}
}

