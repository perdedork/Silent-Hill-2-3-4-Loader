#ifndef  __UTILS_H__
#define  __UTILS_H__
#include <windows.h>
#include <string.h>
#include "typedefs.h"

#if !defined( __BITOP_TYPE__ ) 
#define  __BITOP_TYPE__		long
#endif /*__BITOP_TYPE__*/

typedef	__BITOP_TYPE__	btype;

void _writeBytes( char *prefixStr, long val, long numBytes );

#define writeBytes( v )				(_writeBytes( ((#v)##" = "##(v)), ((long *)&(v))[ 0 ], sizeof( (v) ) ) )

#define staticDataCopy( w, c )		for( long   _k  = 0;   _k  < (c);   _k  ++ )	\
										(w)[  _k ] = (rhs.w)[  _k ];
#define dynamicDataCopy( w, t, c )	(c) = (rhs.c) ;									\
									(w) = new t [(c)];								\
									for( long   _k  = 0;   _k  < (c);   _k  ++ )	\
										(w)[  _k ] = (rhs.w)[  _k ];
#define dynamicDataDelCopy( w,t,c )	delete [] (w);									\
									(c) = (rhs.c);									\
									(w) = new (t)[(c)];								\
									for( long   _k  = 0;   _k  < (c);   _k  ++ )	\
										(w)[  _k ] = (rhs.w)[  _k ];

#define structMemcpy( w )			(memcpy( (void *)(&w),(void *)(&rhs.w),sizeof(w)))
#define structMemClear( w )			(memset( (void *)(&w), 0,sizeof(w)))
#define structArrayMemcpy( w, u )	(memcpy( (void *)(w) ,(void *)(rhs.w) ,sizeof((w)[ 0 ] ) * (u) ))
#define arrayMemcpy( w, u )			(memcpy( (void *)(w) ,(void *)(rhs.w) ,sizeof((w)[ 0 ] ) * (u) ))
#define fieldCopy( w )				( (w) = (rhs.w) )

#define undefBlock2Type( s )		union							\
									{										\
										short			s_types[(s)];	\
										char			c_types[(s)*2];	\
										unsigned short	us_types[(s)];	\
										unsigned char	uc_types[(s)*2];	\
									}


#define undefBlock4Type( s )		union							\
									{										\
										float			f_types[(s)];	\
										long			l_types[(s)];	\
										short			s_types[(s)*2];	\
										char			c_types[(s)*4];	\
										unsigned long	ul_types[(s)];	\
										unsigned short	us_types[(s)*2];	\
										unsigned char	uc_types[(s)*4];	\
									}

//#define utype2( s )			(utype2x##(s))
//#define utype4( s )			(utype4x##(s))

typedef undefBlock2Type( 1 ) utype2;
typedef undefBlock4Type( 1 ) utype4;
typedef undefBlock2Type( 3 ) utype6;
typedef undefBlock4Type( 2 ) utype8;

#define	BITOP_HEXCOUNT( x )		((( (x) & 0x08 ) >> 3) + (( (x) & 0x04 ) >> 2) + (( (x) & 0x02 ) >> 1) + ( (x) & 0x01 ))
#define BITOP_ONES_COUNT( x )	( BITOP_HEXCOUNT( (x) ) + BITOP_HEXCOUNT( (x >> 4) ) + BITOP_HEXCOUNT( (x >> 8) ) + BITOP_HEXCOUNT( (x >> 12) ) + BITOP_HEXCOUNT( (x >> 16) ) + BITOP_HEXCOUNT( (x >> 20) ) + BITOP_HEXCOUNT( (x >> 24) ) + BITOP_HEXCOUNT( (x >> 28) ) )
#define COUNT_LONG_BITS( x )	( BITOP_ONES_COUNT( (x) ) )

long baseName( char *i_pcFilename, char *o_pcBasename );
long getFileSize( char *i_pcFilename );
/*
typedef struct
{
	utype6	testUtype;
}t_uType;
*/
#define _loadData( d, s, f )	(fread( (void *)(d), 1, (s), (f) ))

static long _loadBlock( void *d, long s, FILE *f, char *m, int t )
{
	if( !d )
	{
		LogFile( (t), "%s - TERMINAL ERROR: Passed Null as fread destination",s);
		return -1;
	}
	long _numRead = _loadData( (d), (s), (f) );
	if( _numRead != (s) )
	{
		LogFile( (t), "%s (%ld of %ld read ) - REASON: %s",(m), _numRead, (s), strerror( errno ));
		return -1;
	}
	return _numRead;
}


static long _loadBlockC( void *d, long s, FILE *f, char *m, int t, void (__cdecl *cleanup)() )
{
	long _numRead = _loadData( (d), (s), (f) );
	if( _numRead != (s) )
	{
		LogFile( (t), "%s (%ld of %ld read ) - REASON: %s",(m), _numRead, (s), strerror( errno ));
		if( cleanup )
			cleanup( );
		return -1;
	}
	return _numRead;
}

#endif /*__UTILS_H__*/