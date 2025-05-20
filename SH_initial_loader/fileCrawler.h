#ifndef  __FILECRAWLER_H__
#define  __FILECRAWLER_H__

#include <windows.h>
#include "mstring.h"
#include <stdio.h>

#include "strParse.h"

typedef enum _FILECRAWLER_ERROR
{
	FCERR_NONE			=	0x00000000,
	FCERR_NOT_OPEN		=	0x00000001,
	FCERR_NOT_FOUND		=	0x00000002,
	FCERR_BAD_ARG		=	0x00000003,
	FCERR_INTERNAL		=	0x00000004,
	FCERR_SYNTAX		=	0x00000005,
	FCERR_START			=	0x00000006,
	FCERR_EOF		=	0x00000007,
	FCERR_BOUNDS		=	0x00000008,
//	FCERR_		=	0x0000000,
	FCERR_DWORD			=	0x7fffffff
}
FILECRAWLER_ERROR;

#define FCERR_LAST_LINE		FCERR_EOF

#define FCERR_ALL_CODES		(FCERR_NOT_OPEN | FCERR_NOT_FOUND | FCERR_BAD_ARG | FCERR_INTERNAL | FCERR_SYNTAX | FCERR_START | FCERR_EOF | FCERR_BOUNDS )

static long g_slErrorSet[] = {
	FCERR_NONE,
	FCERR_NOT_OPEN,
	FCERR_NOT_FOUND,
	FCERR_BAD_ARG,
	FCERR_INTERNAL,
	FCERR_SYNTAX,
	FCERR_START,
	FCERR_EOF,
	FCERR_BOUNDS
};

class FileCrawler
{
public:
	FileCrawler( char *_fileToOpen = NULL );
	FileCrawler( mstring & _fileToOpen );
	~FileCrawler( );
	FileCrawler( const FileCrawler & rhs );
	FileCrawler & operator=( const FileCrawler & rhs );

	bool Open( char *_fileToOpen );
	bool OpenText( char * _str );
	bool OpenText( mstring & _str );
	bool SetFilename( char *_str );
	bool SetFilename( mstring & _str );
	bool Close( );
	bool isOpen( );
	bool isAtEnd( );
	bool isAtStart( );

	bool hasError( );
	void clearError( );
	void setLastErrorByCode( long _err );
	mstring getLastError( );
	long getLastErrorCode( );
	long peekLastErrorCode( );

	long getPos( ){ return m_lCurPosition; }
	long goToPos( long _lPos );
	long goBack( long _lPos = 0 );
	long goForward( long _lPos = -1 );

//---[ Char Methods ]---/
	long findNextChar( char _str );
	long goToNextChar( char _str );
	long findPrevChar( char _str );
	long goToPrevChar( char _str );

	//---[ String Methods ]---/
	long findNextStr( char * _str );
	long findNextStr( mstring & _str );
	long findPrevStr( char * _str );
	long findPrevStr( mstring & _str );
	long goToPrevStr( char * _str );
	long goToPrevStr( mstring & _str );
	long goToNextStr( char * _str );
	long goToNextStr( mstring & _str );

//---[ Set Methods (Note: Can match any of the mstring) ]---/
	long findNextSet( char * _str );
	long findNextSet( mstring & _str );

	long findPrevSet( char * _str );
	long findPrevSet( mstring & _str );
	long findNextNotSet( char * _str );
	long findNextNotSet( mstring & _str );
	long findPrevNotSet( char * _str );
	long findPrevNotSet( mstring & _str );
	long goToNextNotSet( char * _str );
	long goToNextNotSet( mstring & _str );
	long goToPrevNotSet( char * _str );
	long goToPrevNotSet( mstring & _str );
	long goToPrevSet( char * _str );
	long goToPrevSet( mstring & _str );
	long goToNextSet( char * _str );
	long goToNextSet( mstring & _str );


	mstring peekLine( );
	mstring peekToStr( char * _str );
	mstring peekToStr( mstring & _str );
	mstring peekToSet( char * _str );
	mstring peekToSet( mstring & _str );

	mstring readLine( );
	mstring readToStr( char * _str );
	mstring readToStr( mstring & _str );
	mstring readToSet( char * _str );
	mstring readToSet( mstring & _str );
	mstring readNextQuoteBlock( bool _stripOuter = false );				//Includes ' and "
	mstring readNextParenBlock( bool _stripOuter = false );				//Includes {},[],()
	mstring readNextSpecialBlock( char *_blockOpen, char *_blockClose, bool _stripOuter = false );	//Special Reader

//protected:
	void DeleteData( );
	void SetNullAll( );

	//---[ File Data ]---/
	mstring		m_sFilename;
	long		m_lDataLength;
	char		*m_pcData;

	//---[ Data Iteration Info ]---/
	long		m_lCurPosition;

	//---[ Various Flags ]---/
	bool		m_bHasError;
	bool		m_bIsOpen;
	long		m_sLastErrorCode;

};







#endif /*__FILECRAWLER_H__*/