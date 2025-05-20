#ifndef  __SH3_MOD_H__
#define  __SH3_MOD_H__

#include <windows.h>

#include "SH3_Loader.h"
#include "SH3_ArcFilenames.h"
#include "SH_Model_Anim.h"




static void writeByteData( char *filename, char *outFilename, int place, long offsetIn, long dist, int numElements, BYTE *updateArray)
{
	FILE *theFile = NULL;
	long *tBuffer = NULL;
//	int x,k;
	BYTE *buffer;
	//int place = 1;
//	long *lDat;
//	float *fDat;
//	short *sDat;
	long offset;
	long size;
	long negSize;
	long numLongs = place * 4 + 4;

	tBuffer = new long[numLongs];
	struct _stat checkFile;

	if(tBuffer == NULL )
	{
		LogFile(ERROR_LOG,"ERROR: Unable to allocate memory in writeByteData");
		return;
	}

	if(_stat( filename, &checkFile ) != 0)
	{
		LogFile(ERROR_LOG,"writeByteData::Couldn't get data about file %s.\n",filename);
		exit(1);
	}

	LogFile(ERROR_LOG,">=+=+-+-+=+=[ B E F O R E: writeByteData ]=+=+-+-+=+=<\n");
	readIndexData(filename,numLongs,place,offsetIn+dist,0);


	if((theFile = fopen(filename,"rb"))==NULL)
	{
		LogFile(ERROR_LOG,"ERROR: Unable to open file for read in 'writeByteData'");
		delete [] tBuffer;
		return;
	}
	fread(tBuffer,sizeof(long),numLongs,theFile);
	if(place && place * 4 < numLongs)
	{
		offset= tBuffer[place *4];
		negSize = tBuffer[place *4 + 1];
		size = tBuffer[place *4 + 2];
	
		offset = (offset < 0)?0:offset;
		negSize = (negSize < 0)?0:negSize;
		size = (size < 0)?0:size;

	}

	delete [] tBuffer;

	fseek(theFile,0,SEEK_SET);

	buffer = new BYTE[checkFile.st_size];
	LogFile(ERROR_LOG,"HERE");
	if(fread(buffer,1,checkFile.st_size,theFile) != checkFile.st_size )
		LogFile(ERROR_LOG,"ERROR: Couldn't read entire file in writeByteData");
	else
	{
		LogFile(ERROR_LOG,"HERE");
		fclose(theFile);
		if((theFile = fopen(outFilename,"wb"))==NULL)
		{
			LogFile(ERROR_LOG,"HERE");
			LogFile(ERROR_LOG,"ERROR: Unable to open file '%s' for update in 'writeByteData'",outFilename);
			delete [] buffer;
			return;
		}
		LogFile(ERROR_LOG,"HERE");
		memcpy(&(buffer[offset+offsetIn]),updateArray,sizeof(BYTE)*numElements);
		memcpy(&(buffer[offset+offsetIn+dist]),updateArray,sizeof(BYTE)*numElements);
		if(fwrite(buffer,1,checkFile.st_size,theFile) != checkFile.st_size )
			LogFile(ERROR_LOG,"writeByteData - ERROR: Unable to write first set");
		else
			LogFile(ERROR_LOG,"SUCCESS ... I think");
		fclose(theFile);
	}
	LogFile(ERROR_LOG,">=+=+-+-+=+=[ A F T E R : writeByteData ]=+=+-+-+=+=<\n");
	readIndexData(outFilename,numLongs,place,offsetIn+dist,0);	


}



#endif /*__SH3_MOD_H__*/