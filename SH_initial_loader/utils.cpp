#include <sys/types.h>
#include <sys/stat.h>

#include "utils.h"
#include "typedefs.h"

void _writeBytes( char *prefixStr, long val, long numBytes )
{
	char *l_pcOutStr;
	long k;
	long count = 0;

	if( numBytes )
	{
		l_pcOutStr = new char[ numBytes * 8 + 1 ];
		for( k = numBytes * 8; k > 0; k-- )
			l_pcOutStr[ count++ ] = ( (val >> (k-1)) & 0x01 )?'1':'0';
		l_pcOutStr[ numBytes * 8 ] = '\0';
		LogFile( ERROR_LOG, "\t%s\t=\t%s", prefixStr, l_pcOutStr );
	}
	else
		LogFile( ERROR_LOG, "\t%s\t= <empty>", prefixStr );
}

long getFileSize( char *i_pcFilename )
{
	struct _stat l_sBuf;
	long l_lRes;

	if( ( l_lRes = _stat( i_pcFilename, &l_sBuf ) ) != 0)
		return -1;

	return l_sBuf.st_size;
}


long baseName( char *i_pcFilename, char *o_pcBasename )
{
	char *l_pcSlash = NULL;
	char *l_pcDot = NULL;

	if( !i_pcFilename || !o_pcBasename )
		return 0;

	l_pcSlash = strrchr( i_pcFilename, '\\' );
	l_pcDot = strrchr( i_pcFilename, '.' );

	if( ! l_pcDot )
	{
		if( ! l_pcSlash )
		{
			strcpy( o_pcBasename, i_pcFilename );
			return 1;
		}
		
		sprintf( o_pcBasename, "%s", &(l_pcSlash[1]) );
		return 1;
	}

	if( ! l_pcSlash )
	{
		sprintf( o_pcBasename, "%.*s", l_pcDot - i_pcFilename, i_pcFilename );
		return 1;
	}

	sprintf( o_pcBasename, "%.*s", l_pcDot - &(l_pcSlash[1]), &(l_pcSlash[1]) );
	return 1;
}