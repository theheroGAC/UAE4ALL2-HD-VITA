#ifndef DISKIMAGEFACTORY_H
#define DISKIMAGEFACTORY_H



                                                                     
class CDiskImageFactory
{
public:
	CDiskImageFactory();
	virtual ~CDiskImageFactory();
	static int GetImageType(PCAPSFILE pcf);
	static PCDISKIMAGE CreateImage(int diftype);

protected:
	static int IsCAPSImage(PCAPSFILE pcf);
	static int IsKFStream(PCAPSFILE pcf);
	static int IsKFStreamCue(PCAPSFILE pcf);
};

typedef CDiskImageFactory *PCDISKIMAGEFACTORY;



#endif
