#include <stdio.h>
#include <vector>
#include "mstring.h"
#include "strParse.h"


char & mstring::operator[]( long iIndex)
{
	//l_pString = m_pString;
	if( iIndex >=0 && iIndex < m_lValidDataSize )
		return m_pString[ iIndex ];
	return m_pString[ 0 ];
}


const char & mstring::operator[](long iIndex) const
{
	if( iIndex >=0 && iIndex < m_lValidDataSize )
		return m_pString[ iIndex ];
	return m_pString[ 0 ];
}


int mstring::find_first_of( char val )
{
	return findNextCharIndex( m_pString, val );
}

int mstring::find_first_of( char *val )
{
	return findNextCharSetIndex( m_pString, val );
}

int mstring::find_first_not_of( char val )
{
	return findNextNotCharIndex( m_pString, val );
}

int mstring::find_first_not_of( char *val )
{
	return findNextNotCharSetIndex( m_pString, val );
}

int mstring::find_last_of( char val )
{
	return findNextCharIndexRev( m_pString, val );
}

int mstring::find_last_of( char *val )
{
	return findNextCharSetIndexRev( m_pString, val );
}

int mstring::find_last_not_of( char val )
{
	return findNextNotCharIndexRev( m_pString, val );
}

int mstring::find_last_not_of( char *val )
{
	return findNextNotCharSetIndexRev( m_pString, val );
}

int mstring::find_first_of( char val ) const
{
	return findNextCharIndex( m_pString, val );
}

int mstring::find_first_of( char *val ) const
{
	return findNextCharSetIndex( m_pString, val );
}

int mstring::find_first_not_of( char val ) const
{
	return findNextNotCharIndex( m_pString, val );
}

int mstring::find_first_not_of( char *val ) const
{
	return findNextNotCharSetIndex( m_pString, val );
}

int mstring::find_last_of( char val ) const
{
	return findNextCharIndexRev( m_pString, val );
}

int mstring::find_last_of( char *val ) const
{
	return findNextCharSetIndexRev( m_pString, val );
}

int mstring::find_last_not_of( char val ) const
{
	return findNextNotCharIndexRev( m_pString, val );
}

int mstring::find_last_not_of( char *val ) const
{
	return findNextNotCharSetIndexRev( m_pString, val );
}

/*
mstring mstring::getClippedFront( char val );
mstring mstring::getClippedFront( char *val );
mstring mstring::getClippedBack( char val );
mstring mstring::getClippedBack( char *val );
mstring mstring::getClippedFrontAndBack( char val );
mstring mstring::getClippedFrontAndBack( char *val );

void mstring::clipFront( char val )
{
	long l_lStart = find_first_not_of( val );

	if( l_lStart == -1 )
	{
		clear( );
	}
	else
	{



void mstring::clipFront( char *val );
void mstring::clipBack( char val );
void mstring::clipBack( char *val );
void mstring::clipFrontAndBack( char val );
void mstring::clipFrontAndBack( char *val );
*/

mstring & mstring::append( char *_pcStr, long _lNum  )
{
	long lLength;
	char *tmpStr;

	if( _lNum == -1 )
		lLength = strlen( _pcStr );
	else
		lLength = _lNum;

	tmpStr = new char[ lLength + length( ) + 1 ];

	sprintf( tmpStr, "%.*s%.*s", length( ), c_str( ), lLength, _pcStr );

	createString( tmpStr );

	delete [] tmpStr;

	return *this;
}


//mstring & mstring::append( const mstring & rhs, long _lStart = -1, long _lNum  )
//mstring & mstring::append( long _lNum, long _lVal );


mstring & mstring::assign( const mstring & rhs )
{
	return mstring::operator=( rhs );
}


mstring & mstring::assign( const mstring & rhs, long _lStart, long _lNum )
{
	this->_setToString( rhs.substr( _lStart, _lNum ).getC_Str( ) );
	return *this;
}


mstring & mstring::assign( char *_pcStr )
{
	this->createString( _pcStr );
	return *this;
}


mstring & mstring::assign( char *_pcStr, long _lNum )
{
	this->createString( _pcStr, _lNum );
	return  *this;
}



long mstring::length()
{
	return m_lValidDataSize;
}

long mstring::length() const
{
	return m_lValidDataSize;
}


long mstring::size()
{
	return m_lValidDataSize;
}

long mstring::size() const
{
	return m_lValidDataSize;
}



bool mstring::empty()
{
	return m_pString == mstd::emptyStr || m_pString == NULL || m_lValidDataSize == 0;
}


bool mstring::empty() const
{
	return m_pString == mstd::emptyStr || m_pString == NULL || m_lValidDataSize == 0;
}


char *mstring::c_str()
{
	if( ! empty( ) )
		return m_pString;
	return mstd::emptyStr;
}


char *mstring::c_str() const
{
	//NOTE: NEED A const VERSION OF EMPTY
	//if( ! empty( ) )
	if( m_pString != mstd::emptyStr && m_pString != NULL && m_lValidDataSize != 0 )
		return m_pString;
	return NULL;
}


void mstring::clear()
{
	if( m_pString != mstd::emptyStr )
		delete [] m_pString;
	setNullAll( );
}


long mstring::capacity( )
{
	return m_lAllocatedSize;
}


long mstring::capacity( ) const
{
	return m_lAllocatedSize;
}


mstring mstring::substr( long offset, long len )
{
	long l_lStart;
	long lLength;
	mstring tmpRes;
	char *tmpStr;

	if( offset < 0 )
		l_lStart = 0;
	else if( offset > m_lValidDataSize )
		l_lStart = m_lValidDataSize;
	else
		l_lStart = offset;

	if( len == -1 || len < 0 || len + l_lStart > m_lValidDataSize )
		lLength = length( ) - l_lStart;
	else
		lLength = len;
	
	tmpStr = new char[ lLength + 1 ];

	memcpy( (void *)tmpStr, (void *)&(m_pString[ l_lStart ]), lLength );
	tmpStr[ lLength ] = '\0';
	//sprintf( tmpStr, "%.*s", lLength, &(m_pString[ l_lStart ]) );

	tmpRes._setToString( tmpStr );

	return tmpRes;
}


mstring mstring::substr( long offset, long len ) const
{
	long l_lStart;
	long lLength;
	mstring tmpRes;
	char *tmpStr;

	if( offset < 0 )
		l_lStart = 0;
	else if( offset > m_lValidDataSize )
		l_lStart = m_lValidDataSize;
	else
		l_lStart = offset;

	if( len == -1 || len < 0 || len + l_lStart > m_lValidDataSize )
		lLength = length( ) - l_lStart;
	else
		lLength = len;
	
	tmpStr = new char[ lLength + 1 ];

	memcpy( (void *)tmpStr, (void *)&(m_pString[ l_lStart ]), lLength );
	tmpStr[ lLength ] = '\0';
	//sprintf( tmpStr, "%.*s", lLength, &(m_pString[ l_lStart ]) );

	tmpRes._setToString( tmpStr );

	return tmpRes;
}


mstring & mstring::toUpper( )
{
	for( long k = 0; k < m_lValidDataSize; k++ )
		m_pString[ k ] = toupper( m_pString[ k ] );
	return *this;
}


mstring & mstring::toLower( )
{
	for( long k = 0; k < m_lValidDataSize; k++ )
		m_pString[ k ] = tolower( m_pString[ k ] );
	return *this;
}


char *mstring::getC_Str( )
{
	char *l_pcReturnStr = new char[ m_lValidDataSize + 1 ];
	memcpy( (void *)l_pcReturnStr, (void *)m_pString, m_lValidDataSize );
	l_pcReturnStr[ m_lValidDataSize ] = '\0';

	return l_pcReturnStr;
}


char *mstring::getC_Str( ) const
{
	char *l_pcReturnStr = new char[ m_lValidDataSize + 1 ];
	memcpy( (void *)l_pcReturnStr, (void *)m_pString, m_lValidDataSize );
	l_pcReturnStr[ m_lValidDataSize ] = '\0';

	return l_pcReturnStr;
}


void mstring::_setToString( char *pString )
{
	if( pString != NULL )
	{
		if( m_pString != mstd::emptyStr )
			delete [] m_pString;

		m_lAllocatedSize = strlen( pString ) + 1 ;
		m_lValidDataSize = m_lAllocatedSize - 1;
		m_pString = pString;
	}
	else
	{
		m_lAllocatedSize = 0;
		m_lValidDataSize = 0;
		m_pString = mstd::emptyStr;
	}
}


void mstring::createString( char *pString )
{
	this->clear();

	if( pString != NULL )
	{
		m_lAllocatedSize = strlen( pString ) + 1 ;
		m_lValidDataSize = m_lAllocatedSize - 1;
		m_pString = new char[ m_lAllocatedSize ];
		memcpy( (void *)m_pString, (void *)pString, m_lValidDataSize );
		m_pString[ m_lValidDataSize ] = '\0';

		//sprintf( m_pString, "%.*s",strlen( pString ), pString );
	}
}


void mstring::createString( char *pString, long _len )
{
	this->clear();

	if( pString != NULL )
	{
		m_lAllocatedSize = strlen( pString ) + 1 ;
		if( _len >= 0 && _len < m_lAllocatedSize )
			m_lAllocatedSize = _len + 1;
		m_lValidDataSize = m_lAllocatedSize - 1;
		m_pString = new char[ m_lAllocatedSize ];
		memcpy( (void *)m_pString, (void *)pString, m_lValidDataSize );
		m_pString[ m_lValidDataSize ] = '\0';

		//sprintf( m_pString, "%.*s",strlen( pString ), pString );
	}
}


void mstring::setNullAll( )
{
	this->m_lAllocatedSize = 0;
	this->m_lValidDataSize = 0;
	this->m_pString = mstd::emptyStr;
}
/*

bool operator==( const mstring & rhs, const mstring & lhs )
{
	if( strcmp( rhs.m_pString, lhs.m_pString ) == 0 )
		return true;
	return false;
}

bool operator!=( const mstring & rhs, const mstring & lhs )
{
	if( strcmp( rhs.m_pString, lhs.m_pString ) != 0 )
		return true;
	return false;
}

bool operator<( const mstring & rhs, const mstring & lhs )
{
	if( strcmp( rhs.m_pString, lhs.m_pString ) < 0 )
		return true;
	return false;
}

bool operator<=( const mstring & rhs, const mstring & lhs )
{
	if( strcmp( rhs.m_pString, lhs.m_pString ) <= 0 )
		return true;
	return false;
}



bool operator>( const mstring & rhs, const mstring & lhs )
{
	if( strcmp( rhs.m_pString, lhs.m_pString ) > 0 )
		return true;
	return false;
}

bool operator>=( const mstring & rhs, const mstring & lhs )
{
	if( strcmp( rhs.m_pString, lhs.m_pString ) >= 0 )
		return true;
	return false;
}

void mstring::operator+=( const mstring & rhs )
{
	append( rhs.c_str( ) );
}

mstring operator+( const mstring & rhs, const mstring & lhs )
{
	char *tStr;
	long k;
	long lLength = rhs.length( ) + lhs.length( );
	mstring tempStr;

	tStr = new char[ lLength + 1 ];

	sprintf( tStr, "%.*s%.*s", rhs.length( ), rhs.c_str( ), lhs.length( ), lhs.c_str( ) );

	tempStr = mstring( tStr );

	delete [] tStr;
	return tempStr;
}


ostream & operator<<( ostream & os, mstring & rhs )
{
	os << rhs.m_pString;
	return os;
}



*/
namespace mstd
{

	bool operator==( const mstring & rhs, const mstring & lhs )
	{
		if( strcmp( rhs.m_pString, lhs.m_pString ) == 0 )
			return true;
		return false;
	}

	bool operator!=( const mstring & rhs, const mstring & lhs )
	{
		if( strcmp( rhs.m_pString, lhs.m_pString ) != 0 )
			return true;
		return false;
	}

	bool operator<( const mstring & rhs, const mstring & lhs )
	{
		if( strcmp( rhs.m_pString, lhs.m_pString ) < 0 )
			return true;
		return false;
	}

	bool operator<=( const mstring & rhs, const mstring & lhs )
	{
		if( strcmp( rhs.m_pString, lhs.m_pString ) <= 0 )
			return true;
		return false;
	}



	bool operator>( const mstring & rhs, const mstring & lhs )
	{
		if( strcmp( rhs.m_pString, lhs.m_pString ) > 0 )
			return true;
		return false;
	}

	bool operator>=( const mstring & rhs, const mstring & lhs )
	{
		if( strcmp( rhs.m_pString, lhs.m_pString ) >= 0 )
			return true;
		return false;
	}

	void mstring::operator+=( const mstring & rhs )
	{
		append( rhs.c_str( ) );
	}

	mstring operator+( const mstring & rhs, const mstring & lhs )
	{
		char *tStr;
		long lLength = rhs.length( ) + lhs.length( );
		mstring tempStr;

		tStr = new char[ lLength + 1 ];

		sprintf( tStr, "%.*s%.*s", rhs.length( ), rhs.c_str( ), lhs.length( ), lhs.c_str( ) );

		tempStr = mstring( tStr );

		delete [] tStr;
		return tempStr;
	}


	ostream & operator<<( ostream & os, mstring & rhs )
	{
		os	<<	rhs.c_str();

		return os;
	}
};