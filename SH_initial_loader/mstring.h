#ifndef __MSTRING_H__
#define __MSTRING_H__

#include <windows.h>
//#include <iostream>
#include <string>

using std::string;
using std::ostream;

namespace mstd
{
	class mstring;

	static char emptyStr[1] = "";


	class mstring
	{
	public:
		//const long -1 = -1;
		//char *l_pString;
		//char emptyStr[1];

		mstring( char *pString = NULL ){  setNullAll( ); createString( pString ); }
		mstring( const mstring & rhs ){  setNullAll( ); operator=( rhs ); }
		mstring( const string & rhs ){  setNullAll( ); createString( const_cast< char *>( rhs.c_str( ) ) ); }
		~mstring( ){ clear( ); }
		mstring & operator=( const mstring & rhs ){ if( &rhs != this )createString( rhs.m_pString ); return *this; }
//		char & operator[](int iIndex);
//		char operator[](int iIndex) const;

		char & operator[]( long iIndex );
		const char & operator[]( long iIndex ) const;
		void operator+=( const mstring & rhs );

	//	friend std::ostream & operator<<( std::ostream & os, mstring & rhs );


		//----[ SEARCHING FUNCTIONS ]----/
		int find_first_of( char val );
		int find_first_of( char *val );
		int find_first_not_of( char val );
		int find_first_not_of( char *val );
		int find_last_of( char val );
		int find_last_of( char *val );
		int find_last_not_of( char val );
		int find_last_not_of( char *val );

		int find_first_of( char val ) const;
		int find_first_of( char *val ) const;
		int find_first_not_of( char val ) const;
		int find_first_not_of( char *val ) const;
		int find_last_of( char val ) const;
		int find_last_of( char *val ) const;
		int find_last_not_of( char val ) const;
		int find_last_not_of( char *val ) const;

		//----[ FORMATING FUNCTIONS ]----/
	/*
		mstring getClippedFront( char val );			//Returns a string where 'val' has been clipped from front
		mstring getClippedFront( char *val );		//Returns a string where anything in 'val' has been clipped from front
		mstring getClippedBack( char val );			//Like getClippedFront, but clips from back
		mstring getClippedBack( char *val );			//Like getClippedFront, but clips anything in val from back
		mstring getClippedFrontAndBack( char val );	//Like getClippedFront and getClippedBack together
		mstring getClippedFrontAndBack( char *val );	//Like getClippedFront and getClippedBack together

		void clipFront( char val );					//Like getClippedFront, but applys clipping to this string
		void clipFront( char *val );				//Like getClippedFront, but applys clipping to this string
		void clipBack( char val );					//Like getClippedBack, but applys clipping to this string
		void clipBack( char *val );					//Like getClippedBack, but applys clipping to this string
		void clipFrontAndBack( char val );			//Like getClippedFrontAndBack, but applys clipping to this string
		void clipFrontAndBack( char *val );			//Like getClippedFrontAndBack, but applys clipping to this string
	*/

		//----[ INFO/DATA FUNCTIONS ]----/
		mstring & append( char *_pcStr, long _lNum = -1 );
	//	mstring & append( const mstring & rhs, long _lStart = -1, long _lNum = -1 );
	//	mstring & append( long _lNum, long _lVal );
		mstring & assign( const mstring & rhs );
		mstring & assign( const mstring & rhs, long _lStart, long _lNum );
		mstring & assign( char *_pcStr );
		mstring & assign( char *_pcStr, long _lNum );

		long length();								//Returns the length of the string
		long length() const;								//Returns the length of the string
		long size();								//Returns the length of the string
		long size() const;								//Returns the length of the string
		bool empty();								//Checks if the string has data in it
		bool empty() const;							//Checks if the string has data in it
		char *c_str();								//Returns a char* pointer to the string data
		char *c_str() const;								//Returns a char* pointer to the string data
		void clear();								//Erases string
		long capacity( );
		long capacity( ) const;
		mstring substr( long offset = 0, long len = -1 );
		mstring substr( long offset = 0, long len = -1 ) const;

		mstring & toUpper( );
		mstring & toLower( );

		char *getC_Str( );
		char *getC_Str( ) const;


	private:
		void createString( char *pString );
		void createString( char *pString, long _len );
		void _setToString( char *pString );	//Directly sets passed in Ptr as the string
	//	void copy( const *_pcStr, long _lSrcStart, long _lSrcNum, long _DestStart
		void setNullAll( ); 

	
		char	*m_pString;							//Data of the string
		long	m_lAllocatedSize;					//Size of memory allocated for the string
		long	m_lValidDataSize;					//Count of valid characters in the string (since a null terminator doesn't necessarily exist
	public:
		friend bool operator==( const mstring & rhs, const mstring & lhs );
		friend bool operator!=( const mstring & rhs, const mstring & lhs );
		friend bool operator<( const mstring & rhs, const mstring & lhs );
		friend bool operator<=( const mstring & rhs, const mstring & lhs );
		friend bool operator>( const mstring & rhs, const mstring & lhs );
		friend bool operator>=( const mstring & rhs, const mstring & lhs );

		friend mstring operator+( const mstring & rhs, const mstring & lhs );

		friend ostream & operator<<( ostream & os, mstring & rhs );
	//	friend mstring operator>>( istream & is, mstring & rhs );
	//	friend mstring getline( istream & is, mstring & rhs );
	

/*
	bool operator==( const mstring & rhs, const mstring & lhs )
	bool operator!=( const mstring & rhs, const mstring & lhs )
	bool operator<( const mstring & rhs, const mstring & lhs )
	bool operator<=( const mstring & rhs, const mstring & lhs )
	bool operator>( const mstring & rhs, const mstring & lhs )
	bool operator>=( const mstring & rhs, const mstring & lhs )
	mstring operator+( const mstring & rhs, const mstring & lhs )

	ostream & operator<<( ostream & os, mstring & rhs );
	mstring operator>>( istream & is, mstring & rhs );
*/
	
	};
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
		long lLength = rhs.length( ) + lhs.length( );
		mstring tempStr;

		tStr = new char[ lLength + 1 ];

		sprintf( tStr, "%.*s%.*s", rhs.length( ), rhs.c_str( ), lhs.length( ), lhs.c_str( ) );

		tempStr = mstring( tStr );

		delete [] tStr;
		return tempStr;
	}
*/

//	std::ostream & operator<<( std::ostream & os, mstring & rhs )
//	{
//		os << rhs.m_pString;
//		return os;
//	}



};



#endif /*__MSTRING_H__*/