#ifndef STREAMCUEIMAGE_H
#define STREAMCUEIMAGE_H

                            
#define KF_STREAM_CUE_ID "<KryoFlux_Stream_Cue/>"



                                
class CStreamCueImage : public CStreamImage
{
public:
	CStreamCueImage();
	virtual ~CStreamCueImage();
	int Lock(PCAPSFILE pcf);
	int Unlock();
};

typedef CStreamCueImage *PCSTREAMCUEIMAGE;

#endif
