#include "stdafx.h"



CStreamImage::CStreamImage()
{
}

CStreamImage::~CStreamImage()
{
}

                     
int CStreamImage::Lock(PCAPSFILE pcf)
{
	return imgeOk;
}

               
int CStreamImage::Unlock()
{
	return imgeOk;
}

                                    
int CStreamImage::LoadTrack(PDISKTRACKINFO pti, UDWORD flag)
{
	return imgeOk;
}
