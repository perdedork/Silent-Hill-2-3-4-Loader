#ifndef  __STRPARSE_H__
#define  __STRPARSE_H__

#include <windows.h>
#include "mstring.h"
#include "mvector.h"

using mstd::mstring;
using mstd::mvector;

bool isWhiteSpace( char c );
bool isLineBlank( char *c );
bool isOpenParenBlock( char c );
bool isCloseParenBlock( char c );
char * getNextCharPlace( char *c );
char * getNextCharPlaceRev( char *start, char *end = NULL );
long getNextCharIndex( char *c, long startInd = 0 );
long getNextCharIndexRev( char *start, long endInd = -1 );
long findNextCharIndex( char *pText, char nextChar, long startInd = 0 );
long findNextCharIndexRev( char *pText, char nextChar, long endInd = -1 );
long findNextNotCharIndex( char *pText, char nextChar, long startInd = 0 );
long findNextNotCharIndexRev( char *pText, char nextChar, long endInd = -1 );
long findNextStrIndex( char *pText, char *nextStr, long startInd = 0 );
long findNextStrIndexRev( char *pText, char *nextStr, long endInd = -1 );
long findNextCharSetIndex( char *start, char *set, long startInd = 0 );
long findNextCharSetIndexRev( char *start, char *set, long endInd = -1 );
long findNextNotCharSetIndex( char *start, char *set, long startInd = 0 );
long findNextNotCharSetIndexRev( char *start, char *set, long endInd = -1 );
long getOpenCloseParen( const char * cStr, long startInd = 0 );
long getOpenCloseSpecialBlock(	char * cStr, 
								char *cOpen, 
								char *cClose, 
								long startInd = 0 );
char * __cdecl allToUpper( char *str );
char * __cdecl allToLower( char *str );
bool peelAllOuterParen( char *str, char **res );
bool peelAllOuterSpecialBlock(	char * cStr, 
								char *cOpen, 
								char *cClose,
								char **res );
bool peelAllOuterQuotes( char *str, char **res );
int nonCaseCmp( const char *s1, const char *s2 );
int nonCaseNCmp( const char *s1, const char *s2, size_t len );

//---[ Tokenizing Functions ]---/
mvector<mstring> getTokenVector( mstring & sTokenList, char *pDelimiter, bool bPeelOuterWhitespace = true, bool bPeelOuterQuotes = true );
mvector<mstring> getTokenVector( const mstring & sTokenList, char *pDelimiter, bool bPeelOuterWhitespace = true, bool bPeelOuterQuotes = true );

mvector<mstring> getTokenVector( mstring & sTokenList, char *pDelimiter, char *pSubDelimiter, bool bPeelOuterWhitespace = true, bool bPeelOuterQuotes = true );


//---[ Filename Functions ]---/
//long baseName( char *i_pcFilename, char *o_pcBasename );
mstring baseName( mstring & _sFilename );
mstring dirName( mstring & _sFilename );
mstring extName( mstring & _sFilename );
mstring removeDir( mstring & _sFilename );
mstring removeExt( mstring & _sFilename );



#endif /*__STRPARSE_H__*/