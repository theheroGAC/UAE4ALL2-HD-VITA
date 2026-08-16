#include "stdafx.h"



CStreamCueImage::CStreamCueImage()
{
}

CStreamCueImage::~CStreamCueImage()
{
}

                     
int CStreamCueImage::Lock(PCAPSFILE pcf)
{
	return imgeOk;
}

               
int CStreamCueImage::Unlock()
{
	return imgeOk;
}
