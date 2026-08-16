#include "stdafx.h"



                          
int fdcrobbit[]= {
	0,
	1,
	0,
	3,
	0,
	2,
	0,
	-1
};

                                       
SDWORD fdcddnoise[]= {
	12425,
	11970,
	12277,
	12124,
	12372,
	-1
};

                                  
CapsFdcInit fdcinit[]= {
	cfdcrmType1, CAPSFDC_SM_TYPE1, CAPSFDC_SR_NCCLR, CAPSFDC_SR_NCSET, ~0U, 0,     
	cfdcrmType1, CAPSFDC_SM_TYPE1, CAPSFDC_SR_NCCLR, CAPSFDC_SR_NCSET, ~0U, 0,     
	cfdcrmType1, CAPSFDC_SM_TYPE1, CAPSFDC_SR_NCCLR, CAPSFDC_SR_NCSET, ~0U, 0,     
	cfdcrmType1, CAPSFDC_SM_TYPE1, CAPSFDC_SR_NCCLR, CAPSFDC_SR_NCSET, ~0U, 0,     
	cfdcrmType1, CAPSFDC_SM_TYPE1, CAPSFDC_SR_NCCLR, CAPSFDC_SR_NCSET, ~0U, 0,     
	cfdcrmType1, CAPSFDC_SM_TYPE1, CAPSFDC_SR_NCCLR, CAPSFDC_SR_NCSET, ~0U, 0,     
	cfdcrmType1, CAPSFDC_SM_TYPE1, CAPSFDC_SR_NCCLR, CAPSFDC_SR_NCSET, ~0U, 0,     
	cfdcrmType1, CAPSFDC_SM_TYPE1, CAPSFDC_SR_NCCLR, CAPSFDC_SR_NCSET, ~0U, 0,     
	cfdcrmType2R, CAPSFDC_SM_TYPE2R, CAPSFDC_SR_NCCLR, CAPSFDC_SR_NCSET, ~0U, 0,     
	cfdcrmType2R, CAPSFDC_SM_TYPE2R, CAPSFDC_SR_NCCLR, CAPSFDC_SR_NCSET, ~0U, 0,     
	cfdcrmType2W, CAPSFDC_SM_TYPE2W, CAPSFDC_SR_NCCLR, CAPSFDC_SR_NCSET, ~0U, 0,     
	cfdcrmType2W, CAPSFDC_SM_TYPE2W, CAPSFDC_SR_NCCLR, CAPSFDC_SR_NCSET, ~0U, 0,     
	cfdcrmType3A, CAPSFDC_SM_TYPE2R, CAPSFDC_SR_NCCLR, CAPSFDC_SR_NCSET, ~0U, 0,     
	cfdcrmType4, CAPSFDC_SM_TYPE1, CAPSFDC_SR_NCCLR, 0, ~0U, 0,     
	cfdcrmType3R, CAPSFDC_SM_TYPE2R, CAPSFDC_SR_NCCLR, CAPSFDC_SR_NCSET, ~0U, 0,     
	cfdcrmType3W, CAPSFDC_SM_TYPE2W, CAPSFDC_SR_NCCLR, CAPSFDC_SR_NCSET, ~0U, 0      
};

                      
FDCCALL fdccall_nop[]= {
	FdcComT_NOP,
	NULL
};

              
FDCCALL fdccall_idle[]= {
	FdcComT_Idle,
	NULL
};

                
FDCCALL fdccall_t1[]= {
	FdcComT1_SpinupStart,
	FdcComT1_SpinupLoop,
	FdcComT1_StepStart,
	FdcComT1_Step,
	FdcComT1_StepLoop,
	FdcComT1_DelayStart,
	FdcComT1_DelayLoop,
	FdcComT1_VerifyStart,
	FdcComT1_VerifyLoop,
	NULL
};

                     
FDCCALL fdccall_t2r[]= {
	FdcComT1_SpinupStart,
	FdcComT1_SpinupLoop,
	FdcComT2_DelayStart,
	FdcComT1_DelayLoop,
	FdcComT2_ReadStart,
	FdcComT2_ReadLoop,
	NULL
};

                      
FDCCALL fdccall_t2w[]= {
	FdcComT2W,
	NULL
};

                     
FDCCALL fdccall_t3r[]= {
	FdcComT1_SpinupStart,
	FdcComT1_SpinupLoop,
	FdcComT2_DelayStart,
	FdcComT1_DelayLoop,
	FdcComT3_IndexStart,
	FdcComT3_IndexLoop,
	FdcComT3_ReadStart,
	FdcComT3_ReadLoop,
	NULL
};

                      
FDCCALL fdccall_t3w[]= {
	FdcComT1_SpinupStart,
	FdcComT1_SpinupLoop,
	FdcComT2_DelayStart,
	FdcComT1_DelayLoop,
	FdcComT3_WriteCheck,
	FdcComT3_IndexStart,
	FdcComT3_IndexLoop,
	FdcComT3_WriteStart,
	FdcComT3_WriteLoop,
	NULL
};

                        
FDCCALL fdccall_t3a[]= {
	FdcComT1_SpinupStart,
	FdcComT1_SpinupLoop,
	FdcComT2_DelayStart,
	FdcComT1_DelayLoop,
	FdcComT2_ReadStart,
	FdcComT3_AddressLoop,
	NULL
};

                
FDCCALL fdccall_t4[]= {
	NULL
};

                                                                            
FDCCALL *fdccall[]= {
	fdccall_nop,                 
	fdccall_idle,                  
	fdccall_t1,                 
	fdccall_t2r,                     
	fdccall_t2w,                      
	fdccall_t3r,                     
	fdccall_t3w,                      
	fdccall_t3a,                        
	fdccall_t4,                 
	NULL
};



                               
UDWORD __cdecl CAPSFdcGetInfo(SDWORD iid, PCAPSFDC pc, SDWORD ext)
{
	UDWORD res=0;

	switch (iid) {
		case cfdciSize_Fdc:
			res=sizeof(CapsFdc);
			break;

		case cfdciSize_Drive:
			res=sizeof(CapsDrive);
			break;

		case cfdciR_Command:
			res=pc->r_command;
			break;

		case cfdciR_ST:
			res=(pc->r_st0 & ~pc->r_stm) | (pc->r_st1 & pc->r_stm);
			break;

		case cfdciR_Track:
			res=pc->r_track;
			break;

		case cfdciR_Sector:
			res=pc->r_sector;
			break;

		case cfdciR_Data:
			res=pc->r_data;
			break;
	}

	return res;
}

                          
SDWORD __cdecl CAPSFdcInit(PCAPSFDC pc)
{
	if (!pc)
		return imgeGeneric;

	                           
	if (pc->type < sizeof(CapsFdc))
		return imgeUnsupportedType;

	              
	CapsFdc fdc=*pc;

	                  
	memset(pc, 0, sizeof(CapsFdc));

	                  
	pc->type=fdc.type;
	pc->model=fdc.model;
	pc->clockfrq=fdc.clockfrq;
	pc->drive=fdc.drive;
	pc->drivecnt=fdc.drivecnt;
	pc->drivemax=fdc.drivemax;
	pc->userptr=fdc.userptr;
	pc->userdata=fdc.userdata;

	                                  
	pc->drivesel=-1;
	pc->driveact=-1;
	pc->drivenew=-2;

	                           
	if (pc->model != cfdcmWD1772)
		return imgeUnsupportedType;

	                            
	if (pc->drivecnt<=0 || pc->drivemax<0 || pc->drivemax>pc->drivecnt)
		return imgeOutOfRange;

	                        
	if (!pc->drive)
		return imgeGeneric;

	                                      
	for (int drv=0; drv < pc->drivecnt; drv++) {
		if (pc->drive[drv].type < sizeof(CapsDrive))
			return imgeUnsupportedType;
	}

	                                                     
	pc->addressmask=0x03;
	pc->datamask=0xff;
	pc->seclenmask=0x03;

	                          
	pc->readlimit=5;
	pc->verifylimit=6;
	pc->spinuplimit=6;
	pc->idlelimit=10;

	                 
	pc->steptime[0]=6000;
	pc->steptime[1]=12000;
	pc->steptime[2]=2000;
	pc->steptime[3]=3000;

	                     
	pc->hstime=15000;

	                   
	pc->iptime=4000;

	               
	pc->updatetime=8;

	                                        
	FdcSetTiming(pc);
	FdcInit(pc);

	return imgeOk;
}

                     
void __cdecl CAPSFdcReset(PCAPSFDC pc)
{
	FdcSetTiming(pc);
	FdcReset(pc);

	                                         
	                                                
	                                         
	                                          
	                          
}

               
void __cdecl CAPSFdcEmulate(PCAPSFDC pc, UDWORD cyclecnt)
{
	                 
	pc->clockact=0;

	                   
	pc->clockreq=cyclecnt;

	                 
	pc->endrequest=0;

	                                    
	if (pc->clockreq) {
		FDCCALL *fc=fdccall[pc->runmode];

		                           
		while (!pc->endrequest && pc->clockact<pc->clockreq)
			fc[pc->runstate](pc);

		                                    
		if (pc->endrequest)
			FdcComEnd(pc);
	}

	                                         
	if (pc->clockact >= pc->clockreq)
		FdcUpdateDrive(pc, pc->clockreq);
}

                         
UDWORD __cdecl CAPSFdcRead(PCAPSFDC pc, UDWORD address)
{
	UDWORD data;

	                           
	switch (address & pc->addressmask) {
		                      
		case 0:
			data=(pc->r_st0 & ~pc->r_stm) | (pc->r_st1 & pc->r_stm);
			FdcSetLine(pc, pc->lineout&~CAPSFDC_LO_INTRQ);
			break;

		        
		case 1:
			data=pc->r_track;
			break;

		         
		case 2:
			data=pc->r_sector;
			break;

		                         
		case 3:
			data=pc->r_data;
			FdcSetLine(pc, pc->lineout&~CAPSFDC_LO_DRQ);
			break;

		default:
			NODEFAULT;
	}

	                              
	data&=pc->datamask;

	               
	pc->dataline=data;

	return data;
}

                        
void   __cdecl CAPSFdcWrite(PCAPSFDC pc, UDWORD address, UDWORD data)
{
	                             
	data&=pc->datamask;

	               
	pc->dataline=data;

	                           
	switch (address & pc->addressmask) {
		                                                              
		case 0:
			if (!(pc->r_st0 & CAPSFDC_SR_BUSY) || ((data & 0xf0) == 0xd0))
				FdcCom(pc, data);
			break;

		                                                         
		case 1:
			pc->r_track = data;
			break;

		                                                          
		case 2:
			pc->r_sector = data;
			break;

		                         
		case 3:
			pc->r_data = data;
			FdcSetLine(pc, pc->lineout&~CAPSFDC_LO_DRQ);
			break;

		default:
			NODEFAULT;
	}
}

                                                                                                            
SDWORD __cdecl CAPSFdcInvalidateTrack(PCAPSFDC pc, SDWORD drive)
{
	if (!pc)
		return imgeGeneric;

	if (drive<0 || drive>=pc->drivecnt)
		return imgeOutOfRange;

	PCAPSDRIVE pd=pc->drive+drive;
	pd->buftrack=-1;
	pd->bufside=-1;

	return imgeOk;
}

                                
void FdcSetTiming(PCAPSFDC pc)
{
	                        
	for (int drv=0; drv < pc->drivecnt; drv++) {
		PCAPSDRIVE pd=pc->drive+drv;

		                              
		pd->clockrev=UDWORD(((UQUAD)pc->clockfrq*60)/pd->rpm);

		                                    
		pd->clockip=UDWORD(((UQUAD)pc->clockfrq*pc->iptime)/1000000);
	}

	                                  
	for (int drv=0; drv < 4; drv++)
		pc->clockstep[drv]=UDWORD(((UQUAD)pc->clockfrq*pc->steptime[drv])/1000000);

	                                 
	pc->clockhs=UDWORD(((UQUAD)pc->clockfrq*pc->hstime)/1000000);

	                                                      
	pc->clockupdate=UDWORD(((UQUAD)pc->clockfrq*pc->updatetime)/1000000);
}

                           
void FdcInit(PCAPSFDC pc)
{
	                                 
	for (int drv=0; drv < pc->drivecnt; drv++) {
		PCAPSDRIVE pd=pc->drive+drv;
		pd->track=0;
		pd->buftrack=-1;
		pd->side=0;
		pd->bufside=-1;
		pd->newside=0;
		pd->diskattr=CAPSDRIVE_DA_WP;
		pd->idistance=0;
		pd->ipcnt=0;
		pd->ovlact=0;
		pd->nact=0;
		pd->nseed=0x87654321;

		                
		FdcClearTrackData(pd);
	}

	                       
	FdcResetState(pc);

	                                                          
	pc->dataline=0;
	pc->lineout=0;

	                
	pc->drivesel=pc->drivenew-1;
	FdcUpdateDrive(pc, 0);
}

                            
void FdcReset(PCAPSFDC pc)
{
	                       
	FdcResetState(pc);

	                             
	pc->dataline=0;
	FdcSetLine(pc, 0);

	                
	pc->drivesel=pc->drivenew-1;
	FdcUpdateDrive(pc, 0);
}

                           
void FdcResetState(PCAPSFDC pc)
{
	                           
	pc->r_st0=0;
	pc->r_st1=0;
	pc->r_command=0;
	pc->r_track=0;
	pc->r_sector=0;
	pc->r_data=0;

	                      
	pc->r_stm=CAPSFDC_SM_TYPE1;
	pc->runmode=cfdcrmIdle;
	pc->runstate=0;
	pc->indexcount=0;
	pc->indexlimit=-1;
	pc->spinupcnt=0;
	pc->idlecnt=0;
	pc->clockcnt=0;
}

                   
void FdcCom(PCAPSFDC pc, UDWORD data)
{
	                
	pc->r_command=data;

#ifdef _DEBUG
	{
		char dbuf[64];
		int rtrack=pc->driveprc ? pc->driveprc->track : -1;
		int rside=pc->driveprc ? pc->driveprc->side : -1;
		sprintf(dbuf, "%2.2x: %2.2x %2.2x %2.2x %d.%d\n",
			pc->r_command,
			pc->r_track,
			pc->r_sector,
			pc->r_data,
			rtrack,
			rside);
		OutputDebugString(dbuf);
	}
#endif

	                      
	pc->runstate=0;
	pc->indexlimit=-1;

	int comcl=(pc->r_command>>4) & 0xf;

	if (comcl == 0xd)
		FdcComT4(pc);
	else {
		             
		pc->runmode=fdcinit[comcl].runmode;
		pc->idlecnt=0;

		                  
		pc->r_stm=fdcinit[comcl].stmask;

		                    
		pc->r_st0&=~fdcinit[comcl].st0clr;
		pc->r_st1&=~fdcinit[comcl].st1clr;

		                  
		pc->r_st0|=fdcinit[comcl].st0set;
		pc->r_st1|=fdcinit[comcl].st1set;

		                                                            
		FdcSetLine(pc, pc->lineout & ~(CAPSFDC_LO_DRQ | CAPSFDC_LO_INTRQ | CAPSFDC_LO_INTFRC | CAPSFDC_LO_INTIP));
	}
}

               
void FdcComEnd(PCAPSFDC pc)
{
	                   
	if (pc->endrequest & CAPSFDC_ER_COMEND) {
		             
		pc->runmode=cfdcrmIdle;
		pc->runstate=0;
		pc->indexlimit=-1;
		pc->idlecnt=0;

		             
		pc->r_st0&=~CAPSFDC_SR_BUSY;

		            
		FdcSetLine(pc, pc->lineout|CAPSFDC_LO_INTRQ);
	}

	                       
	FdcComIdle(pc, pc->clockreq-pc->clockact);
}

                                                                   
void FdcComT_NOP(PCAPSFDC pc)
{
	pc->clockact=pc->clockreq;
}

                                           
void FdcComT_Idle(PCAPSFDC pc)
{
	FdcComIdle(pc, pc->clockreq-pc->clockact);
}



                                
void FdcComT1_SpinupStart(PCAPSFDC pc)
{
	UDWORD lo=pc->lineout;

	                 
	pc->lineout|=CAPSFDC_LO_MO;

	                                            
	if (pc->driveprc)
		pc->driveprc->diskattr|=CAPSDRIVE_DA_MO;

	                             
	pc->r_st0=(pc->r_st0 & ~CAPSFDC_SR_SU_RT) | CAPSFDC_SR_MO;

	                        
	pc->spinupcnt=0;

	                                            
	if ((pc->r_command & DF_3) || (lo & CAPSFDC_LO_MO)) {
		pc->r_st0|=CAPSFDC_SR_SU_RT;
		pc->runstate+=2;
		return;
	}

	           
	pc->runstate++;
}

                               
void FdcComT1_SpinupLoop(PCAPSFDC pc)
{
	                           
	UDWORD cmax=pc->clockreq-pc->clockact;

	                                                         
	if (!pc->driveprc) {
		FdcComIdle(pc, cmax);
		return;
	}

	                                                        
	PCAPSDRIVE pd=pc->driveprc;
	if ((pd->diskattr & CAPSDRIVE_DA_IPMASK) != CAPSDRIVE_DA_IPMASK) {
		FdcComIdle(pc, cmax);
		return;
	}

	                                                     
	if ((pd->idistance+cmax<pd->clockrev) || (pc->spinupcnt+1 < pc->spinuplimit)){
		FdcComIdle(pc, cmax);
		return;
	}

	                     
	cmax=pd->clockrev-pd->idistance;

	                                                             
	if (FdcComIdle(pc, cmax) != cmax)
		return;

	                                          
	pc->runstate++;
}

                             
void FdcComT1_StepStart(PCAPSFDC pc)
{
	                   
	switch ((pc->r_command>>4) & 0xf) {
		          
		case 0x0:
			                                                       
			pc->r_track=0xff;
			pc->r_data=0;
			break;

		             
		case 0x1:
		case 0x2:
		case 0x3:
			break;

		          
		case 0x4:
		case 0x5:
			pc->lineout|=CAPSFDC_LO_DIRC;
			break;

		           
		case 0x6:
		case 0x7:
			pc->lineout&=~CAPSFDC_LO_DIRC;
			break;

		default:
			NODEFAULT;
	}

	           
	pc->runstate++;
}

                       
void FdcComT1_Step(PCAPSFDC pc)
{
	                  
	if (pc->r_command < 0x20) {
		                                         
		if (pc->r_track == pc->r_data) {
			pc->runstate+=2;
			return;
		}

		                                              
		if (pc->r_data > pc->r_track)
			pc->lineout|=CAPSFDC_LO_DIRC;
		else
			pc->lineout&=~CAPSFDC_LO_DIRC;
	}

	                         
	int dir=pc->lineout & CAPSFDC_LO_DIRC;

	                                                 
	if (pc->r_command<0x20 || (pc->r_command & DF_4)) {
		                        
		if (dir) {
			                                                  
			if (pc->r_track == 0xff)
				pc->r_track=0x01;
			else
				pc->r_track++;
		} else {
			                                                         
			if (!pc->r_track)
				pc->r_track=0xfe;
			else
				pc->r_track--;
		}
	}

	                                  
	int track=pc->driveprc ? pc->driveprc->track : -1;

	                                                         
	if (!track && !dir) {
		pc->r_track=0;
		pc->r_st0|=CAPSFDC_SR_TR0_LD;
		pc->runstate+=2;
		return;
	}

	                               
	if (track >= 0) {
		if (dir) {
			                                              
			if (track >= pc->driveprc->maxtrack)
				track=pc->driveprc->maxtrack;
			else
				track++;

			                                                 
			pc->r_st0&=~CAPSFDC_SR_TR0_LD;
		} else {
			                                     
			if (track > 0)
				track--;

			                       
			if (!track)
				pc->r_st0|=CAPSFDC_SR_TR0_LD;
		}

		                 
		pc->driveprc->track=track;
	}

	                 
	pc->clockcnt=pc->clockstep[pc->r_command & 3];

	           
	pc->runstate++;
}

                            
void FdcComT1_StepLoop(PCAPSFDC pc)
{
	                           
	UDWORD cmax=pc->clockreq-pc->clockact;

	                          
	if (cmax > pc->clockcnt)
		cmax=pc->clockcnt;

	             
	pc->clockcnt-=FdcComIdle(pc, cmax);

	                    
	if (!pc->clockcnt) {
		                                                    
		if (pc->r_command < 0x20)
			pc->runstate--;
		else
			pc->runstate++;
	}
}

                        
void FdcComT1_DelayStart(PCAPSFDC pc)
{
	                   
	if (!(pc->r_command & DF_2)) {
		pc->endrequest|=CAPSFDC_ER_COMEND;
		return;
	}

	                   
	pc->clockcnt=pc->clockhs;

	           
	pc->runstate++;
}

                             
void FdcComT1_DelayLoop(PCAPSFDC pc)
{
	                           
	UDWORD cmax=pc->clockreq-pc->clockact;

	                          
	if (cmax > pc->clockcnt)
		cmax=pc->clockcnt;

	             
	pc->clockcnt-=FdcComIdle(pc, cmax);

	                             
	if (!pc->clockcnt)
		pc->runstate++;
}

                         
void FdcComT1_VerifyStart(PCAPSFDC pc)
{
	              
	FdcResetData(pc);

	                                              
	pc->indexlimit=pc->verifylimit;

	           
	pc->runstate++;
}

                     
void FdcComT1_VerifyLoop(PCAPSFDC pc)
{
	                     
	FdcUpdateData(pc);

	                       
	FDCREAD ph=FdcGetReadAccess(pc);

	                           
	UDWORD cst=pc->clockact;

	                           
	while (!pc->endrequest && pc->clockact<pc->clockreq) {
		                       
		if (!ph(pc))
			continue;

		           
		switch (pc->dataphase) {
			case 0:
				                                         
				pc->amisigmask=CAPSFDC_AI_DSRREADY;
				pc->dataphase++;
				break;

			case 1:
				                         
				if (pc->dsr < 0xfc) {
					FdcResetAm(pc);
					continue;
				}

				pc->dataphase++;
				break;

			case 2:
				                           
				if (pc->dsr != pc->r_track) {
					FdcResetAm(pc);
					continue;
				}

				pc->dataphase++;
				break;

			                 
			case 3:
			case 4:
			case 5:
			case 6:
				pc->dataphase++;
				break;

			case 7:
				                                      
				if (pc->crc & 0xffff) {
					FdcResetAm(pc);
					pc->r_st0|=CAPSFDC_SR_CRCERR;
					continue;
				}

				                       
				pc->r_st0&=~CAPSFDC_SR_CRCERR;

				                                       
				FdcComIdleOther(pc, pc->clockact-cst);

				      
				pc->endrequest|=CAPSFDC_ER_COMEND;
				return;

			default:
				NODEFAULT;
		}
	}

	                                
	if (pc->endrequest & CAPSFDC_ER_COMEND)
		pc->r_st0|=CAPSFDC_SR_RNF;

	                                  
	FdcComIdleOther(pc, pc->clockact-cst);
}



                        
void FdcComT2_DelayStart(PCAPSFDC pc)
{
	                   
	if (!(pc->r_command & DF_2)) {
		pc->runstate+=2;
		return;
	}

	                   
	pc->clockcnt=pc->clockhs;

	           
	pc->runstate++;
}

                       
void FdcComT2_ReadStart(PCAPSFDC pc)
{
	              
	FdcResetData(pc);

	                                              
	pc->indexlimit=pc->readlimit;

	           
	pc->runstate++;
}

                   
void FdcComT2_ReadLoop(PCAPSFDC pc)
{
	                     
	FdcUpdateData(pc);

	                       
	FDCREAD ph=FdcGetReadAccess(pc);

	                           
	UDWORD cst=pc->clockact;

	                           
	while (!pc->endrequest && pc->clockact<pc->clockreq) {
		                       
		if (!ph(pc))
			continue;

		           
		switch (pc->dataphase) {
			case 0:
				                                         
				pc->amisigmask=CAPSFDC_AI_DSRREADY;
				pc->dataphase++;
				break;

			case 1:
				                         
				if (pc->dsr < 0xfc) {
					FdcResetAm(pc);
					continue;
				}

				pc->dataphase++;
				break;

			case 2:
				                           
				if (pc->dsr != pc->r_track) {
					FdcResetAm(pc);
					continue;
				}

				pc->dataphase++;
				break;

			case 4:
				                            
				if (pc->dsr != pc->r_sector) {
					FdcResetAm(pc);
					continue;
				}

				pc->dataphase++;
				break;

			case 5:
				                    
				pc->seclen=0x80<<(pc->dsr & pc->seclenmask);
				pc->dataphase++;
				break;

			                 
			case 3:
			case 6:
				pc->dataphase++;
				break;

			case 7:
				                                      
				if (pc->crc & 0xffff) {
					FdcResetAm(pc);
					pc->r_st0|=CAPSFDC_SR_CRCERR;
					continue;
				}

				                 
				pc->r_st0&=~CAPSFDC_SR_CRCERR;

				                    
				pc->dataphase++;
				break;

			case 8:
				                                           
				if (pc->datapcnt < 28) {
					if (pc->datapcnt == 27) {
						FdcResetAm(pc, 1);
						pc->amisigmask=CAPSFDC_AI_DSRREADY|CAPSFDC_AI_DSRMA1;
					}

					pc->datapcnt++;
					break;
				}
				pc->dataphase++;

			case 9:
				                                                  
				if (!(pc->aminfo & CAPSFDC_AI_DSRMA1)) {
					if (pc->datapcnt >= 43)
						FdcResetAm(pc);
					else
						pc->datapcnt++;
					continue;
				}
				pc->dataphase++;
				pc->datapcnt=0;

			case 10:
				                         
				if (!(pc->aminfo & CAPSFDC_AI_DSRMA1)) {
					FdcResetAm(pc);
					continue;
				}

				              
				if (!(pc->aminfo & CAPSFDC_AI_DSRAM))
					continue;

				                                         
				pc->amisigmask=CAPSFDC_AI_DSRREADY;
				pc->dataphase++;
				break;

			case 11:
				                  
				switch (pc->dsr) {
					                      
					case 0xf8:
					case 0xf9:
						pc->r_st1|=CAPSFDC_SR_SU_RT;
						break;

					                     
					case 0xfa:
					case 0xfb:
						pc->r_st1&=~CAPSFDC_SR_SU_RT;
						break;

					default:
						FdcResetAm(pc);
						continue;
				}

				pc->dataphase++;
				break;

			case 12:
				                          
				pc->r_data=pc->dsr;
				FdcSetLine(pc, pc->lineout|CAPSFDC_LO_DRQSET);

				if (!--pc->seclen)
					pc->dataphase++;
				break;

			case 13:
				pc->dataphase++;
				break;

			case 14:
				                                   
				if (pc->crc & 0xffff) {
					pc->r_st0|=CAPSFDC_SR_CRCERR;

					                                       
					FdcComIdleOther(pc, pc->clockact-cst);

					      
					pc->endrequest|=CAPSFDC_ER_COMEND;
					return;
				}

				                 
				pc->r_st0&=~CAPSFDC_SR_CRCERR;

				                                             
				if (pc->r_command & DF_4) {
					                                                      
					if (pc->r_sector == 0xff)
						pc->r_sector=0x01;
					else
						pc->r_sector++;

					FdcResetAm(pc);
					continue;
				}

				                                       
				FdcComIdleOther(pc, pc->clockact-cst);

				      
				pc->endrequest|=CAPSFDC_ER_COMEND;
				return;

			default:
				NODEFAULT;
		}
	}

	                                
	if (pc->endrequest & CAPSFDC_ER_COMEND)
		pc->r_st0|=CAPSFDC_SR_RNF;

	                                  
	FdcComIdleOther(pc, pc->clockact-cst);
}



                        
void FdcComT2W(PCAPSFDC pc)
{
	                  
	pc->endrequest|=CAPSFDC_ER_COMEND;
}



                        
void FdcComT3_IndexStart(PCAPSFDC pc)
{
	                              
	if (pc->driveprc) {
		                   
		if (!pc->driveprc->idistance) {
			pc->runstate+=2;
			return;
		}
	}

	           
	pc->runstate++;
}

                             
void FdcComT3_IndexLoop(PCAPSFDC pc)
{
	                           
	UDWORD cmax=pc->clockreq-pc->clockact;

	                                                         
	if (!pc->driveprc) {
		FdcComIdle(pc, cmax);
		return;
	}

	                                                        
	PCAPSDRIVE pd=pc->driveprc;
	if ((pd->diskattr & CAPSDRIVE_DA_IPMASK) != CAPSDRIVE_DA_IPMASK) {
		FdcComIdle(pc, cmax);
		return;
	}

	                                          
	if (pd->idistance+cmax < pd->clockrev){
		FdcComIdle(pc, cmax);
		return;
	}

	                     
	cmax=pd->clockrev-pd->idistance;

	                                                 
	if (FdcComIdle(pc, cmax) != cmax)
		return;

	                                          
	pc->runstate++;
}

                       
void FdcComT3_ReadStart(PCAPSFDC pc)
{
	              
	FdcResetData(pc);
	pc->aminfo&=~CAPSFDC_AI_CRCENABLE;
	pc->amisigmask=CAPSFDC_AI_DSRREADY;

	                      
	pc->indexlimit=1;

	           
	pc->runstate++;
}

                   
void FdcComT3_ReadLoop(PCAPSFDC pc)
{
	                     
	FdcUpdateData(pc);

	                       
	FDCREAD ph=FdcGetReadAccess(pc);

	                           
	UDWORD cst=pc->clockact;

	                           
	while (!pc->endrequest && pc->clockact<pc->clockreq) {
		                       
		if (!ph(pc))
			continue;

		                          
		pc->r_data=pc->dsr;
		FdcSetLine(pc, pc->lineout|CAPSFDC_LO_DRQSET);
	}

	                                  
	FdcComIdleOther(pc, pc->clockact-cst);
}



                      
void FdcComT3_WriteCheck(PCAPSFDC pc)
{
	                            
	if (pc->r_st0 & CAPSFDC_SR_WP) {
		pc->endrequest|=CAPSFDC_ER_COMEND;
		return;
	}

	       
	pc->runstate++;
}

                        
void FdcComT3_WriteStart(PCAPSFDC pc)
{
	              
	FdcResetData(pc);

	                      
	pc->indexlimit=1;

	           
	pc->runstate++;
}

                    
void FdcComT3_WriteLoop(PCAPSFDC pc)
{
	pc->endrequest|=CAPSFDC_ER_COMEND;
}



                      
void FdcComT3_AddressLoop(PCAPSFDC pc)
{
	                     
	FdcUpdateData(pc);

	                       
	FDCREAD ph=FdcGetReadAccess(pc);

	                           
	UDWORD cst=pc->clockact;

	                           
	while (!pc->endrequest && pc->clockact<pc->clockreq) {
		                       
		if (!ph(pc))
			continue;

		           
		switch (pc->dataphase) {
			                 
			case 0:
				                                         
				pc->amisigmask=CAPSFDC_AI_DSRREADY;
				pc->dataphase++;
				break;

			case 1:
				                         
				if (pc->dsr < 0xfc) {
					FdcResetAm(pc);
					continue;
				}

				pc->dataphase++;
				break;

			case 2:
				                                              
				pc->r_sector=pc->dsr;

			case 3:
			case 4:
			case 5:
			case 6:
			case 7:
				                          
				pc->r_data=pc->dsr;
				FdcSetLine(pc, pc->lineout|CAPSFDC_LO_DRQSET);

				             
				if (pc->dataphase++ == 7) {
					                             
					if (pc->crc & 0xffff)
						pc->r_st0|=CAPSFDC_SR_CRCERR;

					                                       
					FdcComIdleOther(pc, pc->clockact-cst);

					      
					pc->endrequest|=CAPSFDC_ER_COMEND;
					return;
				}
				break;

			default:
				NODEFAULT;
		}
	}

	                                
	if (pc->endrequest & CAPSFDC_ER_COMEND)
		pc->r_st0|=CAPSFDC_SR_RNF;

	                                  
	FdcComIdleOther(pc, pc->clockact-cst);
}



                  
void FdcComT4(PCAPSFDC pc)
{
	                 
	pc->lineout |= CAPSFDC_LO_MO;

	                                            
	if (pc->driveprc)
		pc->driveprc->diskattr |= CAPSDRIVE_DA_MO;

	               
	pc->r_st0 |= CAPSFDC_SR_MO;

	                    
	pc->runmode=cfdcrmIdle;
	pc->runstate=0;
	pc->indexlimit=-1;
	pc->idlecnt=0;

	             
	if (pc->r_st0 & CAPSFDC_SR_BUSY) {
		                                       
		pc->r_st0&=~CAPSFDC_SR_BUSY;
	} else {
		                              
		pc->r_stm=CAPSFDC_SM_TYPE1;
		pc->r_st0 &= ~(CAPSFDC_SR_SU_RT | CAPSFDC_SR_RNF | CAPSFDC_SR_CRCERR);
		pc->r_st1 = 0;
	}

	                                                            
	UDWORD line=pc->lineout & ~(CAPSFDC_LO_DRQ | CAPSFDC_LO_INTRQ | CAPSFDC_LO_INTFRC | CAPSFDC_LO_INTIP);

	                                   
	if (pc->r_command & DF_2)
		line|=CAPSFDC_LO_INTIP;

	                       
	if (pc->r_command & DF_3)
		line|=CAPSFDC_LO_INTFRC;

	                 
	FdcSetLine(pc, line);
}



                      
UDWORD FdcComIdle(PCAPSFDC pc, UDWORD cyc)
{
	                                        
	if (pc->endrequest & CAPSFDC_ER_REQEND)
		return 0;

	                     
	for (int drv=0; drv < pc->drivemax; drv++) {
		            
		PCAPSDRIVE pd=pc->drive+drv;

		                                                                
		if (!(pd->diskattr & CAPSDRIVE_DA_MO))
			continue;

		                             
		UDWORD dist=pd->idistance+cyc;

		                                           
		if (dist >= pd->clockrev) {
			dist-=pd->clockrev;

			                     
			pd->idistance=0;
			FdcIndex(pc, drv);
		}

		               
		pd->idistance=dist;
	}

	      
	pc->clockact+=cyc;
	return cyc;
}

                                                                                    
void FdcComIdleOther(PCAPSFDC pc, UDWORD cyc)
{
	                     
	for (int drv=0; drv < pc->drivemax; drv++) {
		                    
		if (drv == pc->driveact)
			continue;

		            
		PCAPSDRIVE pd=pc->drive+drv;

		                                  
		if (!(pd->diskattr & CAPSDRIVE_DA_MO))
			continue;

		                             
		UDWORD dist=pd->idistance+cyc;

		                                           
		if (dist >= pd->clockrev) {
			dist-=pd->clockrev;

			                     
			pd->idistance=0;
			FdcIndex(pc, drv);
		}

		               
		pd->idistance=dist;
	}
}

                                                               
void FdcUpdateDrive(PCAPSFDC pc, UDWORD cyc)
{
	                     
	for (int drv=0; drv < pc->drivemax; drv++) {
		            
		PCAPSDRIVE pd=pc->drive+drv;

		                    
		pd->side=pd->newside;

		                             
		int ic=pd->ipcnt;
		if (!ic)
			continue;

		                                                                                          
		if (ic < 0) {
			pd->ipcnt=pd->idistance+1;
			continue;
		}

		                        
		ic+=cyc;

		                                  
		if (ic > pd->clockip) {
			ic=0;

			                                                     
			if (drv == pc->driveact)
				pc->r_st0&=~CAPSFDC_SR_IP_DRQ;
		}

		pd->ipcnt=ic;
	}

	                                            
	if (pc->drivenew == pc->drivesel)
		return;

	                                          
	pc->datalock=-1;

	                                                               
	if (pc->drivenew < 0) {
		pc->drivenew=-1;
		pc->drivesel=-1;
		pc->driveact=-1;
		pc->driveprc=NULL;
		pc->r_st0&=~(CAPSFDC_SR_WP|CAPSFDC_SR_TR0_LD|CAPSFDC_SR_IP_DRQ);
		return;
	}

	                   
	pc->drivesel=pc->drivenew;

	                                               
	if (pc->drivenew >= pc->drivemax) {
		pc->driveact=-1;
		pc->driveprc=NULL;
		pc->r_st0&=~(CAPSFDC_SR_WP|CAPSFDC_SR_TR0_LD|CAPSFDC_SR_IP_DRQ);
		return;
	}

	                     
	pc->driveact=pc->drivenew;
	pc->driveprc=pc->drive+pc->driveact;

	                  
	UDWORD attr=pc->driveprc->diskattr;
	if (pc->lineout & CAPSFDC_LO_MO)
		attr|=CAPSDRIVE_DA_MO;
	else
		attr&=~CAPSDRIVE_DA_MO;
	pc->driveprc->diskattr=attr;

	                                                     
	UDWORD st0=pc->r_st0 & ~(CAPSFDC_SR_WP|CAPSFDC_SR_TR0_LD|CAPSFDC_SR_IP_DRQ);

	          
	if (!pc->driveprc->track)
		st0|=CAPSFDC_SR_TR0_LD;

	if (attr & CAPSDRIVE_DA_IN) {
		                        
		if (attr & CAPSDRIVE_DA_WP)
			st0 |= CAPSFDC_SR_WP;
	} else {
		                  
		st0|=CAPSFDC_SR_WP;
	}

	                                                          
	if (pc->driveprc->ipcnt)
		st0|=CAPSFDC_SR_IP_DRQ;

	                 
	pc->r_st0=st0;
}

                          
void FdcResetData(PCAPSFDC pc)
{
	pc->seclen=0;
	pc->amdecode=0;
	pc->aminfo=0;
	pc->amisigmask=0;
	pc->amdatadelay=2;
	pc->amdataskip=0;
	pc->ammarkdist=0;
	pc->ammarktype=0;
	pc->dsr=0;
	pc->dsrcnt=0;
	pc->datalock=-1;
	pc->datamode=0;
	pc->datacycle=0;
	pc->indexcount=0;
	FdcResetAm(pc);
}

                                                                        
void FdcResetAm(PCAPSFDC pc, int keepphase)
{
	pc->aminfo|=CAPSFDC_AI_AMDETENABLE|CAPSFDC_AI_CRCENABLE;
	pc->aminfo&=~(CAPSFDC_AI_CRCACTIVE|CAPSFDC_AI_AMACTIVE);
	pc->amisigmask=CAPSFDC_AI_DSRAM;
	if (!keepphase) {
		pc->dataphase=0;
		pc->datapcnt=0;
	}
}

                   
void FdcClearTrackData(PCAPSDRIVE pd)
{
	                          
	pd->ttype=ctitNA;
	pd->trackbuf=NULL;
	pd->timebuf=NULL;
	pd->tracklen=0;
	pd->overlap=-1;
	pd->trackbits=0;
	pd->ovlmin=-1;
	pd->ovlmax=-1;
	pd->ovlcnt=0;
}

                           
void FdcUpdateData(PCAPSFDC pc)
{
	                       
	if (!pc->driveprc) {
		                               
		if (pc->datalock >= 0)
			return;

		               
		FdcLockData(pc);
		return;
	}

	                     
	PCAPSDRIVE pd=pc->driveprc;

	                                                         
	if (pd->track==pd->buftrack && pd->side==pd->bufside) {
		                             
		if (pc->datalock >= 0)
			return;

		               
		FdcLockData(pc);
		return;
	}

	                
	FdcClearTrackData(pd);

	                               
	pd->buftrack=pd->track;
	pd->bufside=pd->side;

	                                      
	if (pd->diskattr & CAPSDRIVE_DA_IN) {
		                                                                                                            
		pc->cbtrk(pc, pc->driveact);

		                         
		                                                       
		                                                         
		                                                                                                            
		if ((pd->diskattr & CAPSDRIVE_DA_SS) && (pd->bufside == 1))
			FdcClearTrackData(pd);
	}

	                          
	FdcUpdateTrack(pc, pc->driveact);

	               
	FdcLockData(pc);
}

                          
void FdcUpdateTrack(PCAPSFDC pc, int drive)
{
	                      
	if (drive < 0)
		return;

	                     
	PCAPSDRIVE pd=pc->drive+drive;

	                             
	if (!(pd->diskattr & CAPSDRIVE_DA_IN))
		return;

	                                
	if (!pd->trackbuf || !pd->tracklen) {
		                                                 
		PSDWORD ns=fdcddnoise;
		pd->trackbits=ns[pd->nact++]<<3;
		if (ns[pd->nact] < 0)
			pd->nact=0;
		pd->overlap=0;
	} else
		pd->trackbits=pd->tracklen<<3;

	                             
	int rb;
	if (pd->overlap >= 0) {
		                                          
		rb=fdcrobbit[pd->ovlact++];
		if (fdcrobbit[pd->ovlact] < 0)
			pd->ovlact=0;
	} else
		rb=0;

	pd->ovlcnt=rb;

	                                            
	if (rb) {
		pd->ovlmin=((pd->overlap+1)<<3)-rb;
		pd->ovlmax=pd->ovlmin+rb-1;
	} else {
		pd->ovlmin=-1;
		pd->ovlmax=-1;
	}
}

                   
void FdcLockData(PCAPSFDC pc)
{
	                     
	PCAPSDRIVE pd=pc->driveprc;

	                        
	if (!pd || !(pd->diskattr & CAPSDRIVE_DA_IN)) {
		                                        
		pc->datamode=cfdcdmNoline;

		                                                   
		pc->datalock=UDWORD(((UQUAD)pc->clockfrq*2*16)/1000000);
		pc->datacycle=0;
		return;
	}

	                                
	int dat=pd->trackbuf && pd->tracklen;

	                                                        
	if (pd->timebuf && dat) {
		pc->datamode=cfdcdmDMap;
		FdcLockTime(pc);
		return;
	}

	                           
	pc->datamode=dat ? cfdcdmData : cfdcdmNoise;

	                                               
	pc->datalock=UDWORD(((UQUAD)pd->idistance*pd->trackbits)/pd->clockrev);
	pc->datacycle=0;
}

                                         
void FdcLockTime(PCAPSFDC pc)
{
	PCAPSDRIVE pd=pc->driveprc;
	int lo=0;
	int hi=pd->tracklen-1;
	UDWORD sum=pd->timebuf[hi];

	                                                                  
	while (lo <= hi) {
		int mid=(lo+hi)/2;
		UDWORD atm=UDWORD(((UQUAD)pd->timebuf[mid]*pd->clockrev)/sum);

		if (atm <= pd->idistance)
			lo=mid+1;
		else
			hi=mid-1;
	}

	                                                           
	if ((unsigned)lo >= pd->tracklen)
		lo=pd->tracklen-1;

	UDWORD base=lo ? pd->timebuf[lo-1] : 0;
	UDWORD diff=pd->timebuf[lo]-base;

	                                     
	for (hi=1; hi < 8; hi++) {
		UDWORD atm=UDWORD(((UQUAD)(base+(hi*diff)/8)*pd->clockrev)/sum);
		if (pd->idistance < atm)
			break;
	}

	pc->datalock=lo*8+hi-1;
	pc->datacycle=base;
}

                       
FDCREAD FdcGetReadAccess(PCAPSFDC pc)
{
	           
	switch (pc->datamode) {
		case cfdcdmNoline:
			return FdcComReadNoline;

		case cfdcdmNoise:
			return FdcComReadNoise;

		case cfdcdmData:
			return FdcComReadData;

		case cfdcdmDMap:
			return FdcComReadDMap;

		default:
			NODEFAULT;
	}

	                   
	return FdcComReadNoline;
}

                                                        
int FdcComReadNoline(PCAPSFDC pc)
{
	                    
	pc->aminfo&=~(CAPSFDC_AI_DSRREADY|CAPSFDC_AI_DSRAM|CAPSFDC_AI_DSRMA1);

	                           
	UDWORD cmax=pc->clockreq-pc->clockact;

	                                
	if (!pc->datacycle)
		pc->datacycle=pc->datalock;

	                                    
	if (cmax >= pc->datacycle) {
		                       
		pc->clockact+=pc->datacycle;
		pc->datacycle=0;
		pc->dsr=0;
		return 1;
	}

	                                                     
	pc->datacycle-=cmax;
	pc->clockact=pc->clockreq;
	return 0;
}

                                                     
int FdcComReadNoise(PCAPSFDC pc)
{
	                    
	pc->aminfo&=~(CAPSFDC_AI_DSRREADY|CAPSFDC_AI_DSRAM|CAPSFDC_AI_DSRMA1);

	                     
	PCAPSDRIVE pd=pc->driveprc;

	                                         
	UDWORD dist=pd->idistance+(pc->clockreq-pc->clockact);

	                                                    
	UQUAD nextccn=(UQUAD)(pc->datalock+1)*pd->clockrev;

	                       
	while (true) {
		                                          
		UDWORD nextcyc=UDWORD(nextccn/pd->trackbits);

		                                         
		if (dist < nextcyc) {
			pd->idistance=dist;
			pc->clockact=pc->clockreq;
			return 0;
		}

		                        
		FdcShiftBit(pc);

		           
		pc->datalock++;
		nextccn+=pd->clockrev;

		                   
		if (nextcyc >= pd->clockrev) {
			                           
			dist-=pd->clockrev;
			pc->clockact+=pd->clockrev-pd->idistance;
			pd->idistance=0;
			pc->datalock=0;
			nextccn=pd->clockrev;

			                     
			FdcIndex(pc, pc->driveact);

			                                                     
			if (pc->aminfo & pc->amisigmask)
				return 1;

			              
			if (pc->endrequest)
				return 0;
		}

		                                                   
		if (pc->aminfo & pc->amisigmask) {
			int dst=pd->idistance;
			pd->idistance=nextcyc;
			pc->clockact+=nextcyc-dst;
			return 1;
		}
	}
}

                                                    
int FdcComReadData(PCAPSFDC pc)
{
	                    
	pc->aminfo&=~(CAPSFDC_AI_DSRREADY|CAPSFDC_AI_DSRAM|CAPSFDC_AI_DSRMA1);

	                     
	PCAPSDRIVE pd=pc->driveprc;

	                                         
	UDWORD dist=pd->idistance+(pc->clockreq-pc->clockact);

	                                                    
	UQUAD nextccn=(UQUAD)(pc->datalock+1)*pd->clockrev;

	                       
	while (true) {
		                                          
		UDWORD nextcyc=UDWORD(nextccn/pd->trackbits);

		                                         
		if (dist < nextcyc) {
			pd->idistance=dist;
			pc->clockact=pc->clockreq;
			return 0;
		}

		                        
		FdcShiftBit(pc);

		           
		pc->datalock++;
		nextccn+=pd->clockrev;

		                   
		if (nextcyc >= pd->clockrev) {
			                           
			dist-=pd->clockrev;
			pc->clockact+=pd->clockrev-pd->idistance;
			pd->idistance=0;
			pc->datalock=0;
			nextccn=pd->clockrev;

			                     
			FdcIndex(pc, pc->driveact);

			                                                     
			if (pc->aminfo & pc->amisigmask)
				return 1;

			              
			if (pc->endrequest)
				return 0;
		}

		                                                   
		if (pc->aminfo & pc->amisigmask) {
			int dst=pd->idistance;
			pd->idistance=nextcyc;
			pc->clockact+=nextcyc-dst;
			return 1;
		}
	}
}

                                                                     
int FdcComReadDMap(PCAPSFDC pc)
{
	                    
	pc->aminfo&=~(CAPSFDC_AI_DSRREADY|CAPSFDC_AI_DSRAM|CAPSFDC_AI_DSRMA1);

	                     
	PCAPSDRIVE pd=pc->driveprc;

	                                         
	UDWORD dist=pd->idistance+(pc->clockreq-pc->clockact);

	               
	UDWORD sum=pd->timebuf[pd->tracklen-1];
	UDWORD base=pc->datacycle;
	PUDWORD time=pd->timebuf+(pc->datalock>>3);
	UDWORD diff=*time-base;
	int hi=(pc->datalock&7)+1;

	                       
	while (true) {
		                                     
		UDWORD nextcyc=UDWORD(((UQUAD)(base+(hi*diff)/8)*pd->clockrev)/sum);

		                                         
		if (dist < nextcyc) {
			pd->idistance=dist;
			pc->clockact=pc->clockreq;
			return 0;
		}

		                        
		FdcShiftBit(pc);

		           
		pc->datalock++;

		                  
		if (hi == 8) {
			hi=1;
			base=*time++;
			pc->datacycle=base;
			diff=*time-base;
		} else
			hi++;

		                   
		if (nextcyc >= pd->clockrev) {
			                           
			dist-=pd->clockrev;
			pc->clockact+=pd->clockrev-pd->idistance;
			pd->idistance=0;
			pc->datalock=0;
			base=0;
			pc->datacycle=0;
			time=pd->timebuf;
			diff=*time;
			hi=1;

			                     
			FdcIndex(pc, pc->driveact);

			                                                     
			if (pc->aminfo & pc->amisigmask)
				return 1;

			              
			if (pc->endrequest)
				return 0;
		}

		                                                   
		if (pc->aminfo & pc->amisigmask) {
			int dst=pd->idistance;
			pd->idistance=nextcyc;
			pc->clockact+=nextcyc-dst;
			return 1;
		}
	}
}

                                                          
int FdcReadBit(PCAPSFDC pc)
{
	                     
	PCAPSDRIVE pd=pc->driveprc;

	                       
	int lock=pc->datalock;

	                               
	PUBYTE trk=pd->trackbuf+(lock>>3);
	UBYTE msk=1<<((lock&7)^7);

	return *trk & msk;
}

                                                   
int FdcReadBitNoise(PCAPSFDC pc)
{
	                     
	PCAPSDRIVE pd=pc->driveprc;

	               
	UDWORD msk=1<<((pc->datalock&7)^7);

	              
	UDWORD seed=pd->nseed;

	                                                          
	if (msk == 0x80) {
		seed<<=1;
		if ((seed>>22 ^ seed) & DF_1)
			seed++;
		pd->nseed=seed;
	}

	return seed & msk;
}

                                          
void FdcShiftBit(PCAPSFDC pc)
{
	                     
	PCAPSDRIVE pd=pc->driveprc;

	                              
	if (pc->datalock<=pd->ovlmax && pc->datalock>=pd->ovlmin)
		return;

	                           
	int bval=(pc->datamode == cfdcdmNoise) ? FdcReadBitNoise(pc) : FdcReadBit(pc);

	                                 
	UDWORD amdecode=pc->amdecode<<1;
	if (bval)
		amdecode|=1;
	pc->amdecode=amdecode;

	                                                      
	UDWORD aminfo=pc->aminfo & ~(CAPSFDC_AI_AMFOUND|CAPSFDC_AI_MARKA1|CAPSFDC_AI_MARKC2);

	                                         
	if (pc->ammarkdist)
		pc->ammarkdist--;

	                         
	if (aminfo & CAPSFDC_AI_AMDETENABLE) {
		                                
		int amt=0;

		                                      
		                                                                                                         
		                                                                                           
		                                                                                     
		switch (amdecode & 0xffff) {
			                                                         
			case 0x4489:
				if (!pc->ammarkdist || pc->ammarktype!=1)
					amt=1;
				break;

			                         
			case 0x5224:
				amt=2;
				break;
		}

		                        
		if (amt) {
			                                                                                        
			pc->amdatadelay=1;

			                                      
			if (pc->ammarkdist && pc->ammarktype!=amt) {
				                       
				pc->amdataskip++;

				                                                
				pc->amdatadelay+=2;
			}

			                                                                 
			if (!pc->dsrcnt)
				pc->amdataskip++;

			                                                                                   
			pc->dsrcnt=7;

			                                                                                                
			pc->ammarkdist=16;

			                                                    
			pc->ammarktype=amt;

			                                                                                   
			if (amt == 1) {
				aminfo|=CAPSFDC_AI_MARKA1|CAPSFDC_AI_MA1ACTIVE;

				                                                           
				if (aminfo & CAPSFDC_AI_CRCENABLE) {
					                                                                                                   
					if (!(aminfo & CAPSFDC_AI_CRCACTIVE)) {
						aminfo|=CAPSFDC_AI_CRCACTIVE;
						pc->crc=~0;
						pc->crccnt=16;
					}
				}
			} else
				aminfo|=CAPSFDC_AI_MARKC2;
		}
	}

	                           
	if (aminfo & CAPSFDC_AI_CRCACTIVE) {
		                                                     
		if (!(pc->crccnt & 0xf)) {
			                                                                 
			if (pc->crccnt>48 || (aminfo & CAPSFDC_AI_MARKA1)) {
				                                                                         
				if (pc->crccnt == 48) {
					aminfo|=CAPSFDC_AI_AMFOUND|CAPSFDC_AI_AMACTIVE;
					aminfo&=~CAPSFDC_AI_AMDETENABLE;
				}

				                                                                                              
				int value=0;
				for (int mask=0x4000; mask; mask>>=2) {
					value<<=1;
					if (amdecode & mask)
						value|=1;
				}

				             
				UWORD crc=(UWORD)pc->crc;
				pc->crc=crctab_ccitt[value^(crc>>8)] ^ (crc << 8);
			} else
				aminfo&=~(CAPSFDC_AI_CRCACTIVE|CAPSFDC_AI_AMACTIVE);
		}

		pc->crccnt++;
	}

	                                               
	if (!pc->amdatadelay) {
		                 
		                                                                                                 
		pc->amdatadelay=1;

		                    
		aminfo&=~(CAPSFDC_AI_DSRREADY|CAPSFDC_AI_DSRAM|CAPSFDC_AI_DSRMA1);

		                                                                         
		pc->dsr=((pc->dsr<<1) | (amdecode>>1 & 1)) & 0xff;

		                                                                          
		if (++pc->dsrcnt == 8) {
			                    
			pc->dsrcnt=0;

			                             
			if (aminfo & CAPSFDC_AI_AMACTIVE) {
				aminfo&=~CAPSFDC_AI_AMACTIVE;
				aminfo|=CAPSFDC_AI_DSRAM;
			}

			                                  
			if (aminfo & CAPSFDC_AI_MA1ACTIVE) {
				aminfo&=~CAPSFDC_AI_MA1ACTIVE;
				aminfo|=CAPSFDC_AI_DSRMA1;
			}

			                                               
			if (!pc->amdataskip)
				aminfo|=CAPSFDC_AI_DSRREADY;
			else
				pc->amdataskip--;
		}
	} else
		pc->amdatadelay--;

	                   
	pc->aminfo=aminfo;
}

                                 
void FdcIndex(PCAPSFDC pc, int drive)
{
	                      
	if (drive < 0)
		return;

	                     
	PCAPSDRIVE pd=pc->drive+drive;

	                                        
	if (!(pd->diskattr & CAPSDRIVE_DA_IN))
		return;

	                    
	pd->ipcnt=-1;

	                                               
	if ((pd->ttype & CTIT_FLAG_FLAKEY))
		pc->cbtrk(pc, drive);

	                          
	FdcUpdateTrack(pc, drive);

	                                                       
	if (drive != pc->driveact)
		return;

	                   
	pc->r_st0|=CAPSFDC_SR_IP_DRQ;

	                                
	pc->indexcount++;

	                                                                     
	if (pc->indexlimit>=0 && pc->indexcount>=pc->indexlimit) {
		pc->endrequest|=CAPSFDC_ER_COMEND;
		pc->indexlimit=-1;
	}

	                                    
	if (pc->spinupcnt>=pc->spinuplimit || ++pc->spinupcnt>=pc->spinuplimit)
		pc->r_st0|=CAPSFDC_SR_SU_RT;

	                                                                                                
	if (!(pc->r_st0 & CAPSFDC_SR_BUSY)) {
		if (pc->idlecnt>=pc->idlelimit || ++pc->idlecnt>=pc->idlelimit) {
			pc->lineout&=~CAPSFDC_LO_MO;
			pd->diskattr&=~CAPSDRIVE_DA_MO;
			pc->r_st0&=~CAPSFDC_SR_MO;
			pc->spinupcnt=0;
		}
	}

	                                       
	if (pc->lineout & CAPSFDC_LO_INTIP)
		FdcSetLine(pc, pc->lineout | CAPSFDC_LO_INTRQ);
}

                                   
void FdcSetLine(PCAPSFDC pc, UDWORD lineout)
{
	                                   
	if (lineout & CAPSFDC_LO_INTFRC)
		lineout|=CAPSFDC_LO_INTRQ;

	                     
	UDWORD oldline=pc->lineout;

	                    
	if (lineout & CAPSFDC_LO_DRQSET) {
		lineout&=~CAPSFDC_LO_DRQSET;
		lineout|=CAPSFDC_LO_DRQ;

		                                                         
		if (oldline & CAPSFDC_LO_DRQ)
			pc->r_st1|=CAPSFDC_SR_TR0_LD;
	}

	                     
	UDWORD chgmask=oldline ^ lineout;

	                     
	pc->lineout=lineout;

	             
	if (chgmask & CAPSFDC_LO_DRQ) {
		UDWORD setting=lineout & CAPSFDC_LO_DRQ;
		if (setting)
			pc->r_st1|=CAPSFDC_SR_IP_DRQ;
		else
			pc->r_st1&=~CAPSFDC_SR_IP_DRQ;
		pc->cbdrq(pc, setting);
	}

	             
	if (chgmask & CAPSFDC_LO_INTRQ)
		pc->cbirq(pc, lineout & CAPSFDC_LO_INTRQ);
}



  
                                                     
 
                          
                                                                              
                   
                         

                
               
                                        
              
                                                                
                                        
           
    
                             
                           
                            
                  
                          
                    
                                  
                                                 
                                    
                           
                           
                 
                                     
                    
                          
                 
                  
             
       
      
     

              
           
    

              
                                                  

                              
                         
   
      
                                                                                  
                                  
  

                               

             
                      
             
                                                        
                                                                           
          

                     
                            
                          
                            
                          
         
  

                                              
         

                                        
         

                         
                     
                                                
                                          

            
                                    
                    
                    
                   
                    
                  
                
                     
               

                                       
                                    
                                     
                  
                 
                     
                                     
                  
                                     
                     
                           
                         
                       
                   
                           
                  
                                          
                                    
                              
  

                                                       
         

                    
                           
                           
                          
 
  
