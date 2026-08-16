#include "stdafx.h"



CBaseFile::CBaseFile()
{
	Clear();
}

CBaseFile::~CBaseFile()
{
}

                      
void CBaseFile::Clear()
{
	fileopen=0;
	filemode=0;
}
