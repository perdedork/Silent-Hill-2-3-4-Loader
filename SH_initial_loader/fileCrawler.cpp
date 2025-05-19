#include <windows.h>
#include <stdio.h>
#include <errno.h>

#include "mstring.h"
#include "fileCrawler.h"
#include "utils.h"
#include "typedefs.h"

extern int errno;


//--------------------------------------------------------------/
//-                                                            -/
//--------------------------------------------------------------/
FileCrawler::FileCrawler( char *_fileToOpen )
{
	SetNullAll( );
	Open( _fileToOpen );
}

//--------------------------------------------------------------/
//-                                                            -/
//--------------------------------------------------------------/
FileCrawler::FileCrawler( mstring & _fileToOpen )
{
	SetNullAll( );
	Open( const_cast<char *>(_fileToOpen.c_str( )) );
}

//--------------------------------------------------------------/
//-                                                            -/
//--------------------------------------------------------------/
FileCrawler::~FileCrawler( )
{
	DeleteData( );
}

//--------------------------------------------------------------/
//-                                                            -/
//--------------------------------------------------------------/
FileCrawler::FileCrawler( const FileCrawler & rhs )
{
	SetNullAll( );
	operator=( rhs );
}

//--------------------------------------------------------------/
//-                                                            -/
//--------------------------------------------------------------/
FileCrawler & FileCrawler::operator=( const FileCrawler & rhs )
{
	if( & rhs != this )
	{
		DeleteData( );
		this->m_sFilename = rhs.m_sFilename;
		this->m_bIsOpen = rhs.m_bIsOpen;
		this->m_lCurPosition = rhs.m_lCurPosition;
		m_sLastErrorCode = 0;
		if( m_bIsOpen )
		{
			dynamicDataCopy( m_pcData, char, m_lDataLength );
		}
	}
	return *this;
}


//--------------------------------------------------------------/
//-                                                            -/
//--------------------------------------------------------------/
bool FileCrawler::Open( char *_fileToOpen )
{
	FILE	*inFile;
	long	l_lNumRead;
	long	l_lFileSize;

	if( _fileToOpen == NULL )
		return false;

	DeleteData( );

	if( (l_lFileSize = getFileSize( _fileToOpen ) ) < 0 )
	{
		LogFile( ERROR_LOG, "FileCrawler::Open( ) - ERROR: Unable to get filesize of file '%s'", _fileToOpen );
		return false;
	}

	if( ( inFile = fopen( _fileToOpen, "r" ) ) == NULL )
	{
		LogFile( ERROR_LOG, "FileCrawler::Open( ) - ERROR: Unable to open file '%s' - REASON: %s", _fileToOpen, strerror( errno ) );
		return false;
	}

	m_pcData = new char[ l_lFileSize + 1 ];

	l_lNumRead = fread( (void *)m_pcData, sizeof( char ), l_lFileSize, inFile );

	if( l_lNumRead <= 0 )
	{
		LogFile( ERROR_LOG, "FileCrawler::Open( ) - ERROR: Didn't read any data from file '%s'. Filesize = %ld, Read = %ld - ERR: %s", _fileToOpen, l_lFileSize, l_lNumRead, strerror( errno ) );
		fclose( inFile );
		DeleteData( );
		return false;
	}

	if( l_lFileSize != l_lNumRead )
	{
		LogFile( ERROR_LOG, "FileCrawler::Open( ) - NOTE: FileSize: %ld\tRead: %ld - What about the rest???", l_lFileSize, l_lNumRead );
	}
	
	m_pcData[ l_lNumRead ] = '\0';
	this->m_lDataLength = l_lNumRead;
	this->m_sFilename = mstring( _fileToOpen );
	this->m_bIsOpen = true;
	m_sLastErrorCode = 0;
	fclose( inFile );
	return true;
}


//--------------------------------------------------------------/
//-                                                            -/
//--------------------------------------------------------------/
bool FileCrawler::OpenText( char * _str )
{
	long l_lFilesize;

	DeleteData( );

	if( _str == NULL )
	{
		LogFile( ERROR_LOG, "FileCrawler::OpenText( ) - ERROR: No data passed in");
		setLastErrorByCode( FCERR_BAD_ARG );
		return false;
	}

	l_lFilesize = strlen( _str );
	this->m_lDataLength = l_lFilesize + 1;

	m_pcData = new char[ m_lDataLength ];

	strncpy( m_pcData, _str, l_lFilesize );

	m_pcData[ l_lFilesize ] = '\0';
	this->m_sFilename = mstring( "textfile" );
	this->m_bIsOpen = true;
	m_sLastErrorCode = 0;
	return true;
}

//--------------------------------------------------------------/
//-                                                            -/
//--------------------------------------------------------------/
bool FileCrawler::OpenText( mstring & _str )
{
	return OpenText( const_cast<char *>(_str.c_str( )) );
}

//--------------------------------------------------------------/
//-                                                            -/
//--------------------------------------------------------------/
bool FileCrawler::SetFilename( char *_str )
{
	if( _str == NULL )
		return false;
	m_sFilename = mstring( _str );
	return true;
}

//--------------------------------------------------------------/
//-                                                            -/
//--------------------------------------------------------------/
bool FileCrawler::SetFilename( mstring & _str )
{
	return SetFilename( const_cast<char *>(_str.c_str( )) );
}

//--------------------------------------------------------------/
//-                                                            -/
//--------------------------------------------------------------/
bool FileCrawler::Close( )
{
	if( isOpen( ) )
		DeleteData( );
	return true;
}

//--------------------------------------------------------------/
//-                                                            -/
//--------------------------------------------------------------/
bool FileCrawler::isOpen( )
{
	return this->m_bIsOpen;
}

//--------------------------------------------------------------/
//-                                                            -/
//--------------------------------------------------------------/
bool FileCrawler::isAtEnd( )
{
	return this->m_lCurPosition == this->m_lDataLength;
}

//--------------------------------------------------------------/
//-                                                            -/
//--------------------------------------------------------------/
bool FileCrawler::isAtStart( )
{
	return this->m_lCurPosition == 0;
}

//--------------------------------------------------------------/
//-                                                            -/
//--------------------------------------------------------------/
bool FileCrawler::hasError( )
{
	bool l_bHasError = m_bHasError;
	if( m_sLastErrorCode == 0 )
		m_bHasError = false;
	return l_bHasError;
}



//--------------------------------------------------------------/
//-                                                            -/
//--------------------------------------------------------------/
void FileCrawler::clearError( )
{
	m_bHasError = false;
	m_sLastErrorCode = 0;
}


//--------------------------------------------------------------/
//-                                                            -/
//--------------------------------------------------------------/
void FileCrawler::setLastErrorByCode( long _err )
{
	m_sLastErrorCode &= 0x0fffffff;
	m_sLastErrorCode <<= 4;

	if( COUNT_LONG_BITS( _err ) < 1 || COUNT_LONG_BITS( _err ) > 4 || ( _err & FCERR_ALL_CODES == 0 ) )
	{
		m_sLastErrorCode |= FCERR_INTERNAL;
		LogFile( ERROR_LOG, "FileCrawler::setLastErrorByCode( ) - ERROR: Unknown code '%d'", _err );
	}
	else
	{
		m_sLastErrorCode |= _err;
	}
}

//--------------------------------------------------------------/
//-                                                            -/
//--------------------------------------------------------------/
mstring FileCrawler::getLastError( )
{
	mstring l_sLastError;
	long _err = m_sLastErrorCode & 0x0000000f;

	m_sLastErrorCode >>= 4;
	switch ( _err )
	{
		case FCERR_NONE:
			l_sLastError = "";
			break;
		case FCERR_NOT_OPEN:
			l_sLastError = "File Not Open";
			break;
		case FCERR_NOT_FOUND:
			l_sLastError = "Data Not Found From Point Of Search";
			break;
		case FCERR_BAD_ARG:
			l_sLastError = "Argument passed in was invalid";
			break;
/*		case FCERR_:
			l_sLastError = "";
			break;
*/		case FCERR_SYNTAX:
			l_sLastError = "Expected Syntax Missing";
			break;
		case FCERR_START:
			l_sLastError = "Special Block Does Not Start With Correct Token";
			break;
		case FCERR_BOUNDS:
			l_sLastError = "File Pointer Moved to Out of Bounds";
			break;
		case FCERR_EOF:
			l_sLastError = "End ofFile Reached";
			break;
		case FCERR_INTERNAL:
			l_sLastError = "INTERNAL ERROR: Unknown error code";
			break;
		default:
			l_sLastError = "INTERNAL ERROR: Unknown error code";
			LogFile( ERROR_LOG, "FileCrawler::setLastErrorByCode( ) - ERROR: Unknown code '%d'", _err );
			break;
	}
	return l_sLastError;
}


//--------------------------------------------------------------/
//-                                                            -/
//--------------------------------------------------------------/
long FileCrawler::getLastErrorCode( )
{
	long _err = peekLastErrorCode( );
	m_sLastErrorCode >>= 4;
	return _err;
}
	


//--------------------------------------------------------------/
//-                                                            -/
//--------------------------------------------------------------/
long FileCrawler::peekLastErrorCode( )
{
	return m_sLastErrorCode & 0x0000000f;
}


//--------------------------------------------------------------/
//-                                                            -/
//--------------------------------------------------------------/
long FileCrawler::goToPos( long _lPos )
{
	if( ! isOpen( ) )
	{
		m_bHasError = true;
		setLastErrorByCode( FCERR_NOT_OPEN );
		return 0;
	}
	if( _lPos < 0 )
	{
		m_bHasError = true;
		setLastErrorByCode( FCERR_BOUNDS );
		m_lCurPosition = 0;
	}
	else if( _lPos > m_lDataLength )
	{
		m_bHasError = true;
		setLastErrorByCode( FCERR_BOUNDS );
		m_lCurPosition = m_lDataLength;
	}
	else
		m_lCurPosition = _lPos;

	return m_lCurPosition;
}

//--------------------------------------------------------------/
//-                                                            -/
//--------------------------------------------------------------/
long FileCrawler::goBack( long _lPos )
{
	if( _lPos == 0 || _lPos == -1 )
		m_lCurPosition = 0;
	else
		return goToPos( m_lCurPosition - _lPos );
	return m_lCurPosition;
}

//--------------------------------------------------------------/
//-                                                            -/
//--------------------------------------------------------------/
long FileCrawler::goForward( long _lPos )
{
	if( _lPos == -1 )
		m_lCurPosition = m_lDataLength;
	else
		return goToPos( m_lCurPosition + _lPos );
	return m_lCurPosition;
}

//--------------------------------------------------------------/
//-                                                            -/
//--------------------------------------------------------------/
long FileCrawler::findNextSet( char * _str )
{
	if( ! isOpen( ) )
	{
		setLastErrorByCode( FCERR_NOT_OPEN );
		m_bHasError = true;
		return -1;
	}
	return findNextCharSetIndex( m_pcData, _str, m_lCurPosition );
}

//--------------------------------------------------------------/
//-                                                            -/
//--------------------------------------------------------------/
long FileCrawler::findNextSet( mstring & _str )
{
	char *_lstr = new char[ _str.length() +1 ];
	sprintf( _lstr, "%s",_str.c_str());
	long l_res = findNextSet( _lstr );
	delete [] _lstr;
	return l_res;
	//return findNextSet( const_cast<char *>(_str.c_str( )) );
}

//--------------------------------------------------------------/
//-                                                            -/
//--------------------------------------------------------------/
long FileCrawler::findPrevSet( char * _str )
{
	if( ! isOpen( ) )
	{
		setLastErrorByCode( FCERR_NOT_OPEN );
		m_bHasError = true;
		return -1;
	}
	return findNextCharSetIndexRev( m_pcData, _str, m_lCurPosition );
}

//--------------------------------------------------------------/
//-                                                            -/
//--------------------------------------------------------------/
long FileCrawler::findPrevSet( mstring & _str )
{
	char *_lstr = new char[ _str.length() +1 ];
	sprintf( _lstr, "%s",_str.c_str());
	long l_res = findPrevSet( _lstr );
	delete [] _lstr;
	return l_res;
	//return findPrevSet( const_cast<char *>(_str.c_str( )) );
}

//--------------------------------------------------------------/
//-                                                            -/
//--------------------------------------------------------------/
long FileCrawler::findNextNotSet( char * _str )
{
	if( ! isOpen( ) )
	{
		setLastErrorByCode( FCERR_NOT_OPEN );
		m_bHasError = true;
		return -1;
	}
	return findNextNotCharSetIndex( m_pcData, _str, m_lCurPosition );
}

//--------------------------------------------------------------/
//-                                                            -/
//--------------------------------------------------------------/
long FileCrawler::findNextNotSet( mstring & _str )
{
	char *_lstr = new char[ _str.length() +1 ];
	sprintf( _lstr, "%s",_str.c_str());
	long l_res = findNextNotSet( _lstr );
	delete [] _lstr;
	return l_res;
	//return findNextNotSet( const_cast<char *>(_str.c_str( )) );
}

//--------------------------------------------------------------/
//-                                                            -/
//--------------------------------------------------------------/
long FileCrawler::findPrevNotSet( char * _str )
{
	if( ! isOpen( ) )
	{
		setLastErrorByCode( FCERR_NOT_OPEN );
		m_bHasError = true;
		return -1;
	}
	return findNextNotCharSetIndexRev( m_pcData, _str, m_lCurPosition );
}

//--------------------------------------------------------------/
//-                                                            -/
//--------------------------------------------------------------/
long FileCrawler::findPrevNotSet( mstring & _str )
{
	char *_lstr = new char[ _str.length() +1 ];
	sprintf( _lstr, "%s",_str.c_str());
	long l_res = findPrevNotSet( _lstr );
	delete [] _lstr;
	return l_res;
	//return findPrevNotSet( const_cast<char *>(_str.c_str( )) );
}

//--------------------------------------------------------------/
//-                                                            -/
//--------------------------------------------------------------/
long FileCrawler::goToNextNotSet( char * _str )
{
	long l_lNextIndex;

	if( ! isOpen( ) )
	{
		setLastErrorByCode( FCERR_NOT_OPEN );
		m_bHasError = true;
		return 0;
	}
	if( ( l_lNextIndex = findNextNotCharSetIndex( m_pcData, _str, m_lCurPosition ) ) == -1 )
	{
		m_bHasError = true;
		setLastErrorByCode( FCERR_NOT_FOUND );
		return m_lCurPosition;
	}

	return goToPos( l_lNextIndex );
}

//--------------------------------------------------------------/
//-                                                            -/
//--------------------------------------------------------------/
long FileCrawler::goToNextNotSet( mstring & _str )
{
	char *_lstr = new char[ _str.length() +1 ];
	sprintf( _lstr, "%s",_str.c_str());
	long l_res = goToNextNotSet( _lstr );
	delete [] _lstr;
	return l_res;
	//return goToNextNotSet( const_cast<char *>(_str.c_str( )) );
}

//--------------------------------------------------------------/
//-                                                            -/
//--------------------------------------------------------------/
long FileCrawler::goToPrevNotSet( char * _str )
{
	long l_lNextIndex;

	if( ! isOpen( ) )
	{
		setLastErrorByCode( FCERR_NOT_OPEN );
		m_bHasError = true;
		return 0;
	}
	if( ( l_lNextIndex = findNextNotCharSetIndexRev( m_pcData, _str, m_lCurPosition ) ) == -1 )
	{
		m_bHasError = true;
		setLastErrorByCode( FCERR_NOT_FOUND );
		return m_lCurPosition;
	}

	return goToPos( l_lNextIndex );
}

//--------------------------------------------------------------/
//-                                                            -/
//--------------------------------------------------------------/
long FileCrawler::goToPrevNotSet( mstring & _str )
{
	char *_lstr = new char[ _str.length() +1 ];
	sprintf( _lstr, "%s",_str.c_str());
	long l_res = goToPrevNotSet( _lstr );
	delete [] _lstr;
	return l_res;
	//return goToPrevNotSet( const_cast<char *>(_str.c_str( )) );
}

//--------------------------------------------------------------/
//-                                                            -/
//--------------------------------------------------------------/
long FileCrawler::goToPrevSet( char * _str )
{
	long l_lNextIndex;

	if( ! isOpen( ) )
	{
		setLastErrorByCode( FCERR_NOT_OPEN );
		m_bHasError = true;
		return 0;
	}
	if( ( l_lNextIndex = findNextCharSetIndexRev( m_pcData, _str, m_lCurPosition ) ) == -1 )
	{
		m_bHasError = true;
		setLastErrorByCode( FCERR_NOT_FOUND );
		return m_lCurPosition;
	}

	return goToPos( l_lNextIndex );
}

//--------------------------------------------------------------/
//-                                                            -/
//--------------------------------------------------------------/
long FileCrawler::goToPrevSet( mstring & _str )
{
	char *_lstr = new char[ _str.length() +1 ];
	sprintf( _lstr, "%s",_str.c_str());
	long l_res = goToPrevSet( _lstr );
	delete [] _lstr;
	return l_res;
	//return goToPrevSet( const_cast<char *>(_str.c_str( )) );
}

//--------------------------------------------------------------/
//-                                                            -/
//--------------------------------------------------------------/
long FileCrawler::findPrevChar( char _str )
{
	if( ! isOpen( ) )
	{
		setLastErrorByCode( FCERR_NOT_OPEN );
		m_bHasError = true;
		return -1;
	}
	return findNextCharIndexRev( m_pcData, _str, m_lCurPosition );
}

//--------------------------------------------------------------/
//-                                                            -/
//--------------------------------------------------------------/
long FileCrawler::goToPrevChar( char _str )
{
	long l_lNextIndex;

	if( ! isOpen( ) )
	{
		setLastErrorByCode( FCERR_NOT_OPEN );
		m_bHasError = true;
		return 0;
	}
	if( ( l_lNextIndex = findNextCharIndexRev( m_pcData, _str, m_lCurPosition ) ) == -1 )
	{
		m_bHasError = true;
		setLastErrorByCode( FCERR_NOT_FOUND );
		return m_lCurPosition;
	}

	return goToPos( l_lNextIndex );
}


//--------------------------------------------------------------/
//-                                                            -/
//--------------------------------------------------------------/
long FileCrawler::goToNextStr( char * _str )
{
	return goToPos( findNextStr( _str ) );
}

//--------------------------------------------------------------/
//-                                                            -/
//--------------------------------------------------------------/
long FileCrawler::goToNextStr( mstring & _str )
{
	char *_lstr = new char[ _str.length() +1 ];
	sprintf( _lstr, "%s",_str.c_str());
	long l_res = goToNextStr( _lstr );
	delete [] _lstr;
	return l_res;
	//return goToNextStr( const_cast<char *>(_str.c_str( )) );
}

//--------------------------------------------------------------/
//-                                                            -/
//--------------------------------------------------------------/
long FileCrawler::goToNextSet( char * _str )
{
	return goToPos( findNextSet( _str ) );
}

//--------------------------------------------------------------/
//-                                                            -/
//--------------------------------------------------------------/
long FileCrawler::goToNextSet( mstring & _str )
{
	char *_lstr = new char[ _str.length() +1 ];
	sprintf( _lstr, "%s",_str.c_str());
	long l_res = goToNextSet( _lstr );
	delete [] _lstr;
	return l_res;
	//return goToNextSet( const_cast<char *>(_str.c_str( )) );
}

//--------------------------------------------------------------/
//-                                                            -/
//--------------------------------------------------------------/
long FileCrawler::findNextChar( char _str )
{
	if( ! isOpen( ) )
	{
		setLastErrorByCode( FCERR_NOT_OPEN );
		m_bHasError = true;
		return -1;
	}
	return findNextCharIndex( m_pcData, _str, m_lCurPosition );
}

//--------------------------------------------------------------/
//-                                                            -/
//--------------------------------------------------------------/
long FileCrawler::goToNextChar( char _str )
{
	long l_lNextIndex;

	if( ! isOpen( ) )
	{
		setLastErrorByCode( FCERR_NOT_OPEN );
		m_bHasError = true;
		return 0;
	}
	if( ( l_lNextIndex = findNextCharIndex( m_pcData, _str, m_lCurPosition ) ) == -1 )
	{
		m_bHasError = true;
		setLastErrorByCode( FCERR_NOT_FOUND );
		return m_lCurPosition;
	}

	return goToPos( l_lNextIndex );
}
	

//--------------------------------------------------------------/
//-                                                            -/
//--------------------------------------------------------------/
mstring FileCrawler::peekLine( )
{
	if( ! isOpen( ) )
	{
		setLastErrorByCode( FCERR_NOT_OPEN );
		m_bHasError = true;
		return "";
	}

	long l_lTempPos = getPos( );
	mstring l_sTempRes = readLine( );

	goToPos( l_lTempPos );

	return l_sTempRes;
}


//--------------------------------------------------------------/
//-                                                            -/
//--------------------------------------------------------------/
mstring FileCrawler::peekToStr( char * _str )
{
	if( !isOpen( ) )
	{
		setLastErrorByCode( FCERR_NOT_OPEN );
		m_bHasError = true;
		return mstring("");
	}
	long l_lTempPos = getPos( );
	mstring l_sTempRes = readToStr( _str );

	goToPos( l_lTempPos );

	return l_sTempRes;
}


//--------------------------------------------------------------/
//-                                                            -/
//--------------------------------------------------------------/
mstring FileCrawler::peekToStr( mstring & _str )
{
	char *_lstr = new char[ _str.length() +1 ];
	sprintf( _lstr, "%s",_str.c_str());
	mstring l_res = peekToStr( _lstr );
	delete [] _lstr;
	return l_res;
	//return peekToStr( const_cast<char *>(_str.c_str( )) );
}


//--------------------------------------------------------------/
//-                                                            -/
//--------------------------------------------------------------/
mstring FileCrawler::peekToSet( char * _str )
{
	if( !isOpen( ) )
	{
		setLastErrorByCode( FCERR_NOT_OPEN );
		m_bHasError = true;
		return mstring("");
	}
	long l_lTempPos = getPos( );
	mstring l_sTempRes = readToSet( _str );

	goToPos( l_lTempPos );

	return l_sTempRes;
}


//--------------------------------------------------------------/
//-                                                            -/
//--------------------------------------------------------------/
mstring FileCrawler::peekToSet( mstring & _str )
{
	char *_lstr = new char[ _str.length() +1 ];
	sprintf( _lstr, "%s",_str.c_str());
	mstring l_res = peekToSet( _lstr );
	delete [] _lstr;
	return l_res;
	//return peekToSet( const_cast<char *>(_str.c_str( )) );
}


//--------------------------------------------------------------/
//-                                                            -/
//--------------------------------------------------------------/
mstring FileCrawler::readLine( )
{
	char *tmpStr = NULL;

	if( !isOpen( ) )
	{
		setLastErrorByCode( FCERR_NOT_OPEN );
		m_bHasError = true;
		return mstring("");
	}

	if( isAtEnd( ) )
	{
		m_bHasError = true;
		setLastErrorByCode( FCERR_EOF );
	}

	long l_lOldPosition = m_lCurPosition;
	long l_lNextIndex = findNextChar( '\n' );
	

	if( l_lNextIndex != -1 )
	{
		tmpStr = new char[ l_lNextIndex + 1 - l_lOldPosition ];
		memcpy((void *)tmpStr, (void *)(&m_pcData[ l_lOldPosition ]), l_lNextIndex - l_lOldPosition );
		tmpStr[ l_lNextIndex - l_lOldPosition ] = '\0';
		mstring tmpRes( tmpStr );
		delete [] tmpStr;
		m_lCurPosition = l_lNextIndex + 1;
		return tmpRes;
		//return (mstring( & m_pcData[ l_lOldPosition ] )).substr( 0, l_lNextIndex - l_lOldPosition );
	}

	m_lCurPosition = m_lDataLength;
	return mstring( &m_pcData[ l_lOldPosition ] );
}



//--------------------------------------------------------------/
//-                                                            -/
//--------------------------------------------------------------/
mstring FileCrawler::readToStr( char * _str )
{
	char *tmpStr = NULL;

	if( !isOpen( ) )
	{
		setLastErrorByCode( FCERR_NOT_OPEN );
		m_bHasError = true;
		return mstring("");
	}

	if( isAtEnd( ) )
	{
		m_bHasError = true;
		setLastErrorByCode( FCERR_LAST_LINE );
	}

	long l_lOldPosition = m_lCurPosition;
	long l_lNextIndex = findNextStr( _str );
	

	if( l_lNextIndex != -1 )
	{
		tmpStr = new char[ l_lNextIndex + 1 - l_lOldPosition ];
		memcpy((void *)tmpStr, (void *)(&m_pcData[ l_lOldPosition ]), l_lNextIndex - l_lOldPosition );
		tmpStr[ l_lNextIndex - l_lOldPosition ] = '\0';
		mstring tmpRes( tmpStr );
		delete [] tmpStr;
		m_lCurPosition = l_lNextIndex;
		return tmpRes;
		//m_lCurPosition = l_lNextIndex;
		//return mstring( & m_pcData[ l_lOldPosition ] ).substr( 0, l_lNextIndex - l_lOldPosition );
	}

	setLastErrorByCode( FCERR_NOT_FOUND );
	m_bHasError = true;
	m_lCurPosition = m_lDataLength;
	return mstring( &m_pcData[ l_lOldPosition ] );
}

//--------------------------------------------------------------/
//-                                                            -/
//--------------------------------------------------------------/
mstring FileCrawler::readToStr( mstring & _str )
{
	char *_lstr = new char[ _str.length() +1 ];
	sprintf( _lstr, "%s",_str.c_str());
	mstring l_res = readToStr( _lstr );
	delete [] _lstr;
	return l_res;
	//return readToStr( const_cast<char *>(_str.c_str( )) );
}

//--------------------------------------------------------------/
//-                                                            -/
//--------------------------------------------------------------/
mstring FileCrawler::readToSet( char * _str )
{
	long l_lNextIndex;
	long l_lOldPosition = m_lCurPosition;
	char *tmpStr = NULL;

	if( !isOpen( ) )
	{
		setLastErrorByCode( FCERR_NOT_OPEN );
		m_bHasError = true;
		return mstring("");
	}

	if( isAtEnd( ) )
	{
		m_bHasError = true;
		setLastErrorByCode( FCERR_LAST_LINE );
	}

	if( ( l_lNextIndex = findNextCharSetIndex( m_pcData, _str, m_lCurPosition ) ) == -1 )
	{
		m_lCurPosition = this->m_lDataLength;
		return mstring( & m_pcData[ l_lOldPosition ] );
	}

	tmpStr = new char[ l_lNextIndex + 1 - l_lOldPosition ];
	memcpy((void *)tmpStr, (void *)(&m_pcData[ l_lOldPosition ]), l_lNextIndex - l_lOldPosition );
	tmpStr[ l_lNextIndex - l_lOldPosition ] = '\0';
	mstring tmpRes( tmpStr );
	delete [] tmpStr;
	m_lCurPosition = l_lNextIndex;
	return tmpRes;
	//m_lCurPosition = l_lNextIndex;
	//return mstring( & m_pcData[ l_lOldPosition ] ).substr( 0, l_lNextIndex - l_lOldPosition );
}

//--------------------------------------------------------------/
//-                                                            -/
//--------------------------------------------------------------/
mstring FileCrawler::readToSet( mstring & _str )
{
	char *_lstr = new char[ _str.length() +1 ];
	sprintf( _lstr, "%s",_str.c_str());
	mstring l_res = readToSet( _lstr );
	delete [] _lstr;
	return l_res;
	//return readToSet( const_cast<char *>(_str.c_str( )) );
}

	
//--------------------------------------------------------------/
//-                                                            -/
//--------------------------------------------------------------/
mstring FileCrawler::readNextQuoteBlock( bool _stripOuter )
{
	long l_lFirstQuote;
	long l_lOldPosition = m_lCurPosition;
	mstring l_sStringRes;
	char *tmpStr = NULL;

	if( !isOpen( ) )
	{
		setLastErrorByCode( FCERR_NOT_OPEN );
		m_bHasError = true;
		return mstring("");
	}

	//l_lFirstQuote = findNextChar( '"' );
	l_lFirstQuote = getNextCharIndex( m_pcData, m_lCurPosition );

	if( l_lFirstQuote == -1 )
	{
		m_bHasError = true;
		setLastErrorByCode( FCERR_START );
		return mstring("");
	}

	m_lCurPosition = getOpenCloseParen( m_pcData, l_lFirstQuote );

	if( m_lCurPosition == -1 )
	{
		m_lCurPosition = l_lOldPosition;
		setLastErrorByCode( FCERR_SYNTAX );
		m_bHasError = true;
		return mstring("");
	}

	goToPos( m_lCurPosition + 1 );

	tmpStr = new char[ m_lCurPosition + 1 - l_lFirstQuote ];
	memcpy((void *)tmpStr, (void *)(&m_pcData[ l_lFirstQuote ]), m_lCurPosition - l_lFirstQuote );
	tmpStr[ m_lCurPosition - l_lFirstQuote ] = '\0';
	l_sStringRes = mstring( tmpStr );
	//l_sStringRes = mstring( & m_pcData[ l_lFirstQuote ] ).substr( 0, m_lCurPosition - l_lFirstQuote );

	if( _stripOuter )
	{
		char *cRes = NULL;
		char *_lstr = new char[ l_sStringRes.length() +1 ];
		sprintf( _lstr, "%s",l_sStringRes.c_str());

		if( ! peelAllOuterQuotes( _lstr, & cRes ) )	//If it failed, just return what we have
		{
			delete [] _lstr;
			return l_sStringRes;
		}
		
		//---[ Otherwise, return the peeled mstring ]---/
		l_sStringRes = mstring( cRes );
		delete [] _lstr;
		delete [] cRes;
		return l_sStringRes;
	}

	return l_sStringRes;
}

//--------------------------------------------------------------/
//-                                                            -/
//--------------------------------------------------------------/
mstring FileCrawler::readNextParenBlock( bool _stripOuter )
{
	long l_lFirstQuote;
	long l_lOldPosition = m_lCurPosition;
	mstring l_sStringRes;
	char *tmpStr = NULL;

	if( !isOpen( ) )
	{
		setLastErrorByCode( FCERR_NOT_OPEN );
		m_bHasError = true;
		return mstring("");
	}

	l_lFirstQuote = getNextCharIndex( m_pcData, m_lCurPosition );
	m_lCurPosition = getOpenCloseParen( m_pcData, l_lFirstQuote );
//LogFile( ERROR_LOG, "FileCrawler::readNextParenBlock() - curPosition: %ld\tFirst Quote pos: %ld\tposition of next close paren: %ld", l_lOldPosition, l_lFirstQuote,m_lCurPosition);
	if( m_lCurPosition == -1 )
	{
		m_lCurPosition = l_lOldPosition;
		m_bHasError = true;
		setLastErrorByCode( FCERR_SYNTAX );
		LogFile( ERROR_LOG, "FileCrawler::readNextParenBlock() - ERROR - Syntax");
		return mstring("");
	}

	goToPos( m_lCurPosition + 1 );

	tmpStr = new char[ m_lCurPosition + 1 - l_lFirstQuote ];
	memcpy((void *)tmpStr, (void *)(&m_pcData[ l_lFirstQuote ]), m_lCurPosition - l_lFirstQuote );
	tmpStr[ m_lCurPosition - l_lFirstQuote ] = '\0';
	l_sStringRes = mstring( tmpStr );
	//l_sStringRes = mstring( & m_pcData[ l_lFirstQuote ] ).substr( 0, m_lCurPosition - l_lFirstQuote );
//LogFile( ERROR_LOG, "Last Position by filecrawler: %c\tby returned mstring: %c",m_pcData[ m_lCurPosition - 1 ], 
//		(l_sStringRes.c_str())[ l_sStringRes.length() - 1 ] );

	if( _stripOuter )
	{
		char *cRes = NULL;
		char *_lstr = new char[ l_sStringRes.length() +1 ];
		sprintf( _lstr, "%s",l_sStringRes.c_str());

		if( ! peelAllOuterParen( _lstr, & cRes ) )	//If it failed, just return what we have
		{
			delete [] _lstr;
			return l_sStringRes;
		}
		
		//---[ Otherwise, return the peeled mstring ]---/
		l_sStringRes = mstring( cRes );
//LogFile( ERROR_LOG, "\tLast char = %c",cRes[ strlen(cRes) - 1 ] );
		delete [] _lstr;
		delete [] cRes;
		return l_sStringRes;
	}

	return l_sStringRes;
}	


//--------------------------------------------------------------/
//-                                                            -/
//--------------------------------------------------------------/
mstring FileCrawler::readNextSpecialBlock( char *_blockOpen, char *_blockClose, bool _stripOuter )	//Special Reader
{
	long l_lFirstQuote;
	long l_lOldPosition = m_lCurPosition;
	mstring l_sStringRes;
	char *tmpStr = NULL;

	if( !isOpen( ) )
	{
		setLastErrorByCode( FCERR_NOT_OPEN );
		m_bHasError = true;
		return mstring("");
	}

	if( _blockOpen == NULL || _blockClose == NULL )
	{
		m_bHasError = true;
		setLastErrorByCode( FCERR_BAD_ARG );
		LogFile( ERROR_LOG, "FileCrawler::readNextSpecialBlock() - ERROR - No Block Passed In");
		return mstring("");
	}

	l_lFirstQuote = findNextStrIndex( m_pcData, _blockOpen, m_lCurPosition );
	m_lCurPosition = getOpenCloseSpecialBlock( m_pcData, _blockOpen, _blockClose, l_lFirstQuote );

	if( m_lCurPosition == -1 )
	{
		m_lCurPosition = l_lOldPosition;
		m_bHasError = true;
		LogFile( ERROR_LOG, "FileCrawler::readNextSpecialBlock() - ERROR - Syntax");
		return mstring("");
	}

	goToPos( m_lCurPosition + 1 );
	
	tmpStr = new char[ m_lCurPosition + 1 - l_lFirstQuote ];
	memcpy((void *)tmpStr, (void *)(&m_pcData[ l_lFirstQuote ]), m_lCurPosition - l_lFirstQuote );
	tmpStr[ m_lCurPosition - l_lFirstQuote ] = '\0';
	l_sStringRes = mstring( tmpStr );
	//l_sStringRes = mstring( & m_pcData[ l_lFirstQuote ] ).substr( 0, m_lCurPosition - l_lFirstQuote );

	if( _stripOuter )
	{
		char *cRes = NULL;
		char *_lstr = new char[ l_sStringRes.length() +1 ];
		sprintf( _lstr, "%s",l_sStringRes.c_str());

		if( ! peelAllOuterSpecialBlock( _lstr, _blockOpen, _blockClose, & cRes ) )	//If it failed, just return what we have
		{
			delete [] _lstr;
			return l_sStringRes;
		}
		
		//---[ Otherwise, return the peeled mstring ]---/
		l_sStringRes = mstring( cRes );
		delete [] _lstr;
		delete [] cRes;
		return l_sStringRes;
	}

	return l_sStringRes;
}


//--------------------------------------------------------------/
//-                                                            -/
//--------------------------------------------------------------/
void FileCrawler::DeleteData( )
{
	if( m_pcData )
		delete [] m_pcData;
	SetNullAll( );
}

//--------------------------------------------------------------/
//-                                                            -/
//--------------------------------------------------------------/
void FileCrawler::SetNullAll( )
{
	m_sFilename = mstring("");
	m_lDataLength = 0;
	m_pcData = NULL;
	m_lCurPosition = 0;
	m_bHasError = false;
	m_bIsOpen = false;
	m_sLastErrorCode = 0;
}

/*
	//---[ File Data ]---/
	mstring		m_sFilename;
	long		m_lDataLength;
	char		*m_pcData;

	//---[ Data Iteration Info ]---/
	long		m_lCurPosition;

	//---[ Various Flags ]---/
	bool		m_bHasError;
	bool		m_bIsOpen;
*/


//--------------------------------------------------------------/
//-                                                            -/
//--------------------------------------------------------------/
long FileCrawler::findNextStr( char * _str )
{
	if( ! isOpen( ) )
	{
		setLastErrorByCode( FCERR_NOT_OPEN );
		m_bHasError = true;
		return -1;
	}
	return findNextStrIndex( m_pcData, _str, m_lCurPosition );
}

//--------------------------------------------------------------/
//-                                                            -/
//--------------------------------------------------------------/
long FileCrawler::findNextStr( mstring & _str )
{
	char *_lstr = new char[ _str.length() +1 ];
	sprintf( _lstr, "%s",_str.c_str());
	long l_res = findNextStr( _lstr );
	delete [] _lstr;
	return l_res;
}

//--------------------------------------------------------------/
//-                                                            -/
//--------------------------------------------------------------/
long FileCrawler::findPrevStr( char * _str )
{
	if( ! isOpen( ) )
	{
		setLastErrorByCode( FCERR_NOT_OPEN );
		m_bHasError = true;
		return -1;
	}
	return findNextStrIndexRev( m_pcData, _str, m_lCurPosition );
}

//--------------------------------------------------------------/
//-                                                            -/
//--------------------------------------------------------------/
long FileCrawler::findPrevStr( mstring & _str )
{
	char *_lstr = new char[ _str.length() +1 ];
	sprintf( _lstr, "%s",_str.c_str());
	long l_res = findPrevStr( _lstr );
	delete [] _lstr;
	return l_res;
	//return findPrevStr( const_cast<char *>(_str.c_str( )) );
}


//--------------------------------------------------------------/
//-                                                            -/
//--------------------------------------------------------------/
long FileCrawler::goToPrevStr( char * _str )
{
	long l_lNextIndex;

	if( ! isOpen( ) )
	{
		setLastErrorByCode( FCERR_NOT_OPEN );
		m_bHasError = true;
		return 0;
	}
	if( ( l_lNextIndex = findNextStrIndexRev( m_pcData, _str, m_lCurPosition ) ) == -1 )
	{
		m_bHasError = true;
		setLastErrorByCode( FCERR_NOT_FOUND );
		return m_lCurPosition;
	}

	return goToPos( l_lNextIndex );
}

//--------------------------------------------------------------/
//-                                                            -/
//--------------------------------------------------------------/
long FileCrawler::goToPrevStr( mstring & _str )
{
	char *_lstr = new char[ _str.length() +1 ];
	sprintf( _lstr, "%s",_str.c_str());
	long l_res = goToPrevStr( _lstr );
	delete [] _lstr;
	return l_res;
	//return goToPrevStr( const_cast<char *>(_str.c_str( )) );
}
