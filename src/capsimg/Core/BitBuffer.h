#ifndef BITBUFFER_H
#define BITBUFFER_H

                                                                
#define MAX_BITBUFFER_LEN 32



                                 
class CBitBuffer
{
public:
	CBitBuffer();
	virtual ~CBitBuffer();
	void InitByteSize(uint8_t *buf, uint32_t bytesize);
	void InitBitSize(uint8_t *buf, uint32_t bitsize);

	uint32_t ReadBitWrap(uint32_t bitpos, int bitcnt);
	static uint32_t ReadBitWrap(uint8_t *buf, uint32_t bufwrap, uint32_t bitpos, int bitcnt);
	uint32_t ReadBit(uint32_t bitpos, int bitcnt);
	static uint32_t ReadBit(uint8_t *buf, uint32_t bitpos, int bitcnt);
	uint32_t ReadBit(uint32_t bitpos);
	static uint32_t ReadBit(uint8_t *buf, uint32_t bitpos);
	uint32_t ReadBit8(uint32_t bitpos);
	static uint32_t ReadBit8(uint8_t *buf, uint32_t bitpos);
	static uint32_t ReadBit16(uint8_t *buf);
	static uint32_t ReadBitLE16(uint8_t *buf);
	uint32_t ReadBit16(uint32_t bitpos);
	static uint32_t ReadBit16(uint8_t *buf, uint32_t bitpos);
	static uint32_t ReadBit32(uint8_t *buf);
	static uint32_t ReadBitLE32(uint8_t *buf);
	uint32_t ReadBit32(uint32_t bitpos);
	static uint32_t ReadBit32(uint8_t *buf, uint32_t bitpos);
	uint32_t ReadBit10(uint32_t bitpos);
	static uint32_t ReadBit10(uint8_t *buf, uint32_t bitpos);

	void WriteBitWrap(uint32_t bitpos, uint32_t value, int bitcnt);
	static void WriteBitWrap(uint8_t *buf, uint32_t bufwrap, uint32_t bitpos, uint32_t value, int bitcnt);
	void WriteBit(uint32_t bitpos, uint32_t value, int bitcnt);
	static void WriteBit(uint8_t *buf, uint32_t bitpos, uint32_t value, int bitcnt);
	void WriteBit(uint32_t bitpos, uint32_t value);
	static void WriteBit(uint8_t *buf, uint32_t bitpos, uint32_t value);
	static void WriteBit8(uint8_t *buf, uint32_t value);
	static void WriteBit16(uint8_t *buf, uint32_t value);
	static void WriteBitLE16(uint8_t *buf, uint32_t value);
	static void WriteBit24(uint8_t *buf, uint32_t value);
	static void WriteBit32(uint8_t *buf, uint32_t value);
	static void WriteBitLE32(uint8_t *buf, uint32_t value);

	void ClearBitWrap(uint32_t bitpos, int bitcnt);
	static void ClearBitWrap(uint8_t *buf, uint32_t bufwrap, uint32_t bitpos, int bitcnt);
	void ClearBit(uint32_t bitpos, int bitcnt);
	static void ClearBit(uint8_t *buf, uint32_t bitpos, int bitcnt);

	int CompareBit(uint32_t buf1pos, uint32_t buf2pos, int bitcnt);
	static int CompareBit(uint8_t *buf1, uint32_t buf1pos, uint8_t *buf2, uint32_t buf2pos, int bitcnt);
	int CompareAndCountBit(uint32_t buf1pos, uint32_t buf2pos, int bitcnt);
	static int CompareAndCountBit(uint8_t *buf1, uint32_t buf1pos, uint8_t *buf2, uint32_t buf2pos, int bitcnt);

	void CopyBitWrap(uint32_t srcpos, uint32_t dstpos, int bitcnt);
	static void CopyBitWrap(uint8_t *srcbuf, uint32_t srcwrap, uint32_t srcpos, uint8_t *dstbuf, uint32_t dstwrap, uint32_t dstpos, int bitcnt);
	void CopyBit(uint32_t srcpos, uint32_t dstpos, int bitcnt);
	static void CopyBit(uint8_t *srcbuf, uint32_t srcpos, uint8_t *dstbuf, uint32_t dstpos, int bitcnt);

	static uint32_t CalculateByteSize(uint32_t bitsize);

protected:
	void Clear();

protected:
	uint8_t *bufmem;                 
	uint32_t bufsize;                        
	uint32_t bufbits;                       
};

typedef CBitBuffer *PCBITBUFFER;



                                                                        
inline uint32_t CBitBuffer::ReadBitWrap(uint32_t bitpos, int bitcnt)
{
	return ReadBitWrap(bufmem, bufbits, bitpos, bitcnt);
}

                                                           
inline uint32_t CBitBuffer::ReadBit(uint32_t bitpos, int bitcnt)
{
	return ReadBit(bufmem, bitpos, bitcnt);
}

                                                   
inline uint32_t CBitBuffer::ReadBit(uint32_t bitpos)
{
	return ReadBit(bufmem, bitpos);
}

                                            
inline uint32_t CBitBuffer::ReadBit(uint8_t *buf, uint32_t bitpos)
{
	return buf[bitpos >> 3] >> ((bitpos & 7) ^ 7) & 1;
}

                                                    
inline uint32_t CBitBuffer::ReadBit8(uint32_t bitpos)
{
	return ReadBit8(bufmem, bitpos);
}

                                             
inline uint32_t CBitBuffer::ReadBit8(uint8_t *buf, uint32_t bitpos)
{
	int shf = bitpos & 7;
	uint32_t pos = bitpos >> 3;

	if (!shf)
		return buf[pos];

	return ((buf[pos] << shf) | (buf[pos + 1] >> (8 - shf))) & 0xff;
}

                                              
inline uint32_t CBitBuffer::ReadBit16(uint8_t *buf)
{
	return buf[0] << 8 | buf[1];
}

                                                             
inline uint32_t CBitBuffer::ReadBitLE16(uint8_t *buf)
{
	return buf[1] << 8 | buf[0];
}

                                                     
inline uint32_t CBitBuffer::ReadBit16(uint32_t bitpos)
{
	return ReadBit16(bufmem, bitpos);
}

                                              
inline uint32_t CBitBuffer::ReadBit16(uint8_t *buf, uint32_t bitpos)
{
	int shf = bitpos & 7;
	uint32_t pos = bitpos >> 3;
	uint32_t res = buf[pos] << 8 | buf[pos + 1];

	if (!shf)
		return res;

	return ((res << shf) | (buf[pos + 2] >> (8 - shf))) & 0xffff;
}

                                              
inline uint32_t CBitBuffer::ReadBit32(uint8_t *buf)
{
	return buf[0] << 24 | buf[1] << 16 | buf[2] << 8 | buf[3];
}

                                                             
inline uint32_t CBitBuffer::ReadBitLE32(uint8_t *buf)
{
	return buf[3] << 24 | buf[2] << 16 | buf[1] << 8 | buf[0];
}

                                                     
inline uint32_t CBitBuffer::ReadBit32(uint32_t bitpos)
{
	return ReadBit32(bufmem, bitpos);
}

                                              
inline uint32_t CBitBuffer::ReadBit32(uint8_t *buf, uint32_t bitpos)
{
	int shf = bitpos & 7;
	uint32_t pos = bitpos >> 3;
	uint32_t res = buf[pos] << 24 | buf[pos + 1] << 16 | buf[pos + 2] << 8 | buf[pos + 3];

	if (!shf)
		return res;

	return ((res << shf) | (buf[pos + 4] >> (8 - shf)));
}

                                                     
inline uint32_t CBitBuffer::ReadBit10(uint32_t bitpos)
{
	return ReadBit10(bufmem, bitpos);
}

                                              
inline uint32_t CBitBuffer::ReadBit10(uint8_t *buf, uint32_t bitpos)
{
	int shf = bitpos & 7;
	uint32_t pos = bitpos >> 3;

	if (shf == 7)
		return ((buf[pos] << 9) | (buf[pos + 1] << 1) | (buf[pos + 2] >> 7)) & 0x3ff;
	else
		return ((buf[pos] << (shf + 2)) | (buf[pos + 1] >> (6 - shf))) & 0x3ff;
}



                                                                       
inline void CBitBuffer::WriteBitWrap(uint32_t bitpos, uint32_t value, int bitcnt)
{
	WriteBitWrap(bufmem, bufbits, bitpos, value, bitcnt);
}

                                                          
inline void CBitBuffer::WriteBit(uint32_t bitpos, uint32_t value, int bitcnt)
{
	WriteBit(bufmem, bitpos, value, bitcnt);
}

                                                  
inline void CBitBuffer::WriteBit(uint32_t bitpos, uint32_t value)
{
	WriteBit(bufmem, bitpos, value);
}

                                           
inline void CBitBuffer::WriteBit(uint8_t *buf, uint32_t bitpos, uint32_t value)
{
	                        
	uint8_t *wpos = buf + (bitpos >> 3);

	                                   
	int bmask = 1 << ((bitpos & 7) ^ 7);

	if (value & 1)
		*wpos |= bmask;
	else
		*wpos &= ~bmask;
}

                                            
inline void CBitBuffer::WriteBit8(uint8_t *buf, uint32_t value)
{
	buf[0] = uint8_t(value);
}

                                             
inline void CBitBuffer::WriteBit16(uint8_t *buf, uint32_t value)
{
	buf[0] = uint8_t(value >> 8);
	buf[1] = uint8_t(value);
}

                                                            
inline void CBitBuffer::WriteBitLE16(uint8_t *buf, uint32_t value)
{
	buf[0] = uint8_t(value);
	buf[1] = uint8_t(value >> 8);
}

                                             
inline void CBitBuffer::WriteBit24(uint8_t *buf, uint32_t value)
{
	buf[0] = uint8_t(value >> 16);
	buf[1] = uint8_t(value >> 8);
	buf[2] = uint8_t(value);
}

                                             
inline void CBitBuffer::WriteBit32(uint8_t *buf, uint32_t value)
{
	buf[0] = uint8_t(value >> 24);
	buf[1] = uint8_t(value >> 16);
	buf[2] = uint8_t(value >> 8);
	buf[3] = uint8_t(value);
}

                                                            
inline void CBitBuffer::WriteBitLE32(uint8_t *buf, uint32_t value)
{
	buf[0] = uint8_t(value);
	buf[1] = uint8_t(value >> 8);
	buf[2] = uint8_t(value >> 16);
	buf[3] = uint8_t(value >> 24);
}



                                                              
inline void CBitBuffer::ClearBitWrap(uint32_t bitpos, int bitcnt)
{
	ClearBitWrap(bufmem, bufbits, bitpos, bitcnt);
}

                                                 
inline void CBitBuffer::ClearBit(uint32_t bitpos, int bitcnt)
{
	ClearBit(bufmem, bitpos, bitcnt);
}



                                                                                       
inline int CBitBuffer::CompareBit(uint32_t buf1pos, uint32_t buf2pos, int bitcnt)
{
	return CompareBit(bufmem, buf1pos, bufmem, buf2pos, bitcnt);
}

                                                                                                        
inline int CBitBuffer::CompareAndCountBit(uint32_t buf1pos, uint32_t buf2pos, int bitcnt)
{
	return CompareAndCountBit(bufmem, buf1pos, bufmem, buf2pos, bitcnt);
}



                                                              
inline void CBitBuffer::CopyBitWrap(uint32_t srcpos, uint32_t dstpos, int bitcnt)
{
	CopyBitWrap(bufmem, bufbits, srcpos, bufmem, bufbits, dstpos, bitcnt);
}

                                                 
inline void CBitBuffer::CopyBit(uint32_t srcpos, uint32_t dstpos, int bitcnt)
{
	CopyBit(bufmem, srcpos, bufmem, dstpos, bitcnt);
}



                                                 
inline uint32_t CBitBuffer::CalculateByteSize(uint32_t bitsize)
{
	return ((bitsize + 7) >> 3);
}

#endif
