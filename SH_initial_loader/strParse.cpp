#include <windows.h>
#include <stdio.h>
#include "strParse.h"
#include "typedefs.h"


bool isOpenParenBlock( char c )
{
	return ( c=='{' || c=='"' || c=='(' || c=='[' );
}

bool isCloseParenBlock( char c )
{
	return ( c=='}' || c=='"' || c=='}' || c==']' );
}


bool isWhiteSpace( char c )
{
	return ( c=='\n' || c==' ' || c=='\t' || c=='\r' );
}


bool isLineBlank( char *c )
{
	long k = 0;
	if( !c )
		return false;
	while( c[k] != '\0' )
	{
		if( ! isWhiteSpace( c[ k ] ) )
			return false;
		++k;
	}
	return true;
}


char * getNextCharPlace( char *c )
{
	long k=0;
	if( !c ) return c;
	while( c[k] != '\0' && isWhiteSpace( c[k] ) )
		++k;
	return &(c[k]);
}


char * getNextCharPlaceRev( char *start, char *end )
{
	long k;

	if( !start ) return start;

	if( !end )
		k = strlen( start ) - 1;
	else if( end < start )
		k = start - end;
	else
		k = end - start;
	while( k > 0 && isWhiteSpace( start[k] ) )
		--k;
	return &(start[k]);
}
		

long getNextCharIndex( char *c, long startInd )
{
	long k=startInd;

	if( k < 0 )
		k = 0;

	if( !c ) return -1;
	while( c[k] != '\0' && isWhiteSpace( c[k] ) )
		++k;
	if( c[k] == '\0' )
		return -1;
	return k;
}


long getNextCharIndexRev( char *start, long endInd )
{
	long k;

	if( !start )
		return -1;
	if( endInd == -1 )
		k = strlen( start ) - 1;
	else
		k = endInd;
	while( k > 0 && isWhiteSpace( start[k] ) )
		--k;
	if( isWhiteSpace( start[k] ) )	//start[k] == '\0' )
		return -1;
	return k;
}


long findNextCharIndex( char *start, char nextChar, long startInd )
{
	long place = startInd;
	
	if( place < 0 )
		place = 0;

	while( start[place] != '\0' && start[place] != nextChar )
		place ++;
	return (start[place]==nextChar)?place:-1;
}


long findNextCharIndexRev( char *pText, char nextChar, long endInd )
{
	long k;

	if( ! pText )
		return -1;
	if( endInd == -1 )
		k = strlen( pText ) - 1;
	else
		k = endInd;
	while( k > 0 && pText[ k ] != nextChar )
		--k;

	return ( pText[ k ]==nextChar)?k:-1;
}


long findNextNotCharIndex( char *start, char nextChar, long startInd )
{
	long place = startInd;
	
	if( place < 0 )
		place = 0;

	while( start[place] != '\0' && start[place] == nextChar )
		place ++;
	return (start[place]!=nextChar)?place:-1;
}


long findNextNotCharIndexRev( char *pText, char nextChar, long endInd )
{
	long k;

	if( ! pText )
		return -1;
	if( endInd == -1 )
		k = strlen( pText ) - 1;
	else
		k = endInd;
	while( k > 0 && pText[ k ] == nextChar )
		--k;

	return ( pText[ k ]!=nextChar)?k:-1;
}


long findNextStrIndex( char *pText, char *nextStr, long startInd )
{
	long k = startInd;
	long nextPlace;
	long strLength;
	long maxLength;

	if( pText == NULL || nextStr == NULL )
		return -1;
	if( k < 0 )
		k = 0;
	strLength = strlen( nextStr );
	maxLength = strlen( pText );

	while( pText[ k ] != '\0' && k < maxLength )
	{
		nextPlace = findNextCharIndex( pText, nextStr[ 0 ], k );
		if( nextPlace == -1 )		//No More places to check in string
			return -1;
//LogFile(ERROR_LOG, "CHECK: The string is '%100.100s'",&( pText[ nextPlace ]));
		if( strncmp( &(pText[ nextPlace ]), nextStr, strLength ) == 0 )
			return nextPlace;
		k = nextPlace + 1;
	}

	return -1;
}


long findNextStrIndexRev( char *pText, char *nextStr, long endInd )
{
	long k;
	long nextPlace;
	long strLength;

	if( pText == NULL || nextStr == NULL )
		return -1;

	if( endInd == -1 )
		k = strlen( pText ) - 1;
	else
		k = endInd;

	strLength = strlen( nextStr );

	while( k > 0 )
	{
		nextPlace = findNextCharIndexRev( pText, nextStr[ 0 ], k );
		if( nextPlace == -1 )		//No More places to check in string
			return -1;
		if( strncmp( &pText[ nextPlace ], nextStr, strLength ) == 0 )
			return nextPlace;
		k = nextPlace;
	}

	return -1;
}


long findNextCharSetIndex( char *start, char *set, long startInd )
{
	long k = startInd;
	long j;
	bool bFound = false;

	if( !start || !set )
	{
		LogFile( ERROR_LOG, "findNextCharSetIndex( ) - ERROR: value of string and/or set is NULL");
		return -1;
	}

	if( k < 0 )
		k = 0;
//LogFile( ERROR_LOG, "findNextCharSetIndex - strlen(start) = %ld\tstrlen(set) = %ld\tstartInd = %ld",strlen(start),strlen(set),startInd);
	for(; k < strlen( start ) && start[ k ] !='\0' && !bFound ; k++ )
	{
//LogFile( ERROR_LOG, "\tk = %ld",k);
		for( j=0; j < strlen( set ) && !bFound; j++ )
		{
//LogFile( ERROR_LOG, "\t\tstart[%ld] = '%c'\tset[%ld]",k,start[k],j,set[j]);
			if( start[ k ] == set[ j ] )
				return k;
		}
		//if(bFound)
		//LogFile( ERROR_LOG, "\tLoop done: Found = %s\tk = %ld",(bFound)?"true":"false",k);
	}
	//LogFile( ERROR_LOG, "\tLoop done: Found = %s\tk = %ld",(bFound)?"true":"false",k);
	--k;
	if( bFound )
		return k;
	return -1;
}


long findNextCharSetIndexRev( char *start, char *set, long endInd )
{
	long k;
	long j;
	bool bFound = false;

	if( !start || !set )
	{
		LogFile( ERROR_LOG, "findNextCharSetIndexRev( ) - ERROR: value of string and/or set is NULL");
		return -1;
	}

	if( endInd == -1 )
		k = strlen( start ) - 1;
	else
		k = endInd;

	for( ; k >= 0 && start[ k ] !='\0' && !bFound ; k-- )
	{
		for( j=0; j < strlen( set ); j++ )
		{
			if( start[ k ] == set[ j ] )
				return k;
		}
	}

	if( bFound )
		return k;
	return -1;
}


long findNextNotCharSetIndex( char *start, char *set, long startInd )
{
	long k = startInd;
	long j;
	bool bFound = false;

	if( !start || !set )
	{
		LogFile( ERROR_LOG, "findNextCharSetIndex( ) - ERROR: value of string and/or set is NULL");
		return -1;
	}

	if( k < 0 )
		k = 0;
//LogFile( ERROR_LOG, "findNextCharSetIndex - strlen(start) = %ld\tstrlen(set) = %ld\tstartInd = %ld",strlen(start),strlen(set),startInd);
	for( ; k < strlen( start ) && start[ k ] !='\0' && !bFound ; k++ )
	{
//LogFile( ERROR_LOG, "\tk = %ld",k);
		for( j=0; j < strlen( set ) && !bFound; j++ )
		{
//LogFile( ERROR_LOG, "\t\tstart[%ld] = '%c'\tset[%ld]",k,start[k],j,set[j]);
			if( start[ k ] == set[ j ] )
				bFound = true;
		}
		if( ! bFound)
			return k;
		bFound = false;
		//LogFile( ERROR_LOG, "\tLoop done: Found = %s\tk = %ld",(bFound)?"true":"false",k);
	}
	//LogFile( ERROR_LOG, "\tLoop done: Found = %s\tk = %ld",(bFound)?"true":"false",k);
//	if( k > startInd )
//		--k;

//	if( ! bFound )
//		return k;
	return -1;
}


long findNextNotCharSetIndexRev( char *start, char *set, long endInd )
{
	long k;
	long j;
	bool bFound = false;

	if( !start || !set )
	{
		LogFile( ERROR_LOG, "findNextCharSetIndexRev( ) - ERROR: value of string and/or set is NULL");
		return -1;
	}
//LogFile( ERROR_LOG, "findNextNotCharSetIndexRev - String[%s]  Set[%s]  StartInd = %ld", start, set,endInd);
	if( endInd == -1 )
		k = strlen( start ) - 1;
	else
		k = endInd;

	for( ; k >= 0 && start[ k ] !='\0' && !bFound ; k-- )
	{
//LogFile( ERROR_LOG, "\tk=%ld\tString[%ld]=%c",k,k,start[k]);
		
		for( j=0; j < strlen( set ); j++ )
		{
			if( start[ k ] == set[ j ] )
				bFound = true;
		}
//LogFile( ERROR_LOG, "\tResult - %s",(bFound)?"Did Match the char":"Couldn't find char");
		if( ! bFound )
			return k;
		bFound = false;
	}

//LogFile( ERROR_LOG, "\tShouldn't be here... k = %ld",k);
//	if( ! bFound )
//		return k;
	return -1;
}


long getOpenCloseParen( const char * cStr, long startInd )
{
	int iCountParen = 0, iCountBrace = 0, iCountBracket = 0, iCountQuote = 0, iCountTick = 0;
	int iPlace = 1;
	int *ipCount;
	bool bDoCount = true;

	iPlace = getNextCharIndex( const_cast<char *>(cStr), startInd );

	if( iPlace == -1 )
	{
		LogFile( ERROR_LOG, "getOpenCloseBracketToken( ) - ERROR: String <%s> appears to be empty.",cStr);
		return -1;
	}

	if( cStr[iPlace] == '(' )
		ipCount = &iCountParen;
	else if( cStr[iPlace] == '{' )
		ipCount = &iCountBrace;
	else if( cStr[iPlace] == '[' )
		ipCount = &iCountBracket;
	else if( cStr[iPlace] == '"' )
	{
		bDoCount = false;
		ipCount = &iCountQuote;
	}
	else if( cStr[iPlace] == '\'' )
	{
		bDoCount = false;
		ipCount = &iCountTick;
	}
	else
	{
		//m_bSyntaxError = true;
		LogFile( ERROR_LOG,"getOpenCloseBracketToken() - ERROR: Bad parameter: <%s> Doesn't start with paren",cStr );
		return -1;
	}

	++(*ipCount);
	++iPlace;

	while( *ipCount > 0 && cStr[ iPlace ] != '\0' )
	{
		if( bDoCount )
		{
			switch( cStr[iPlace] )
			{
			case '(':
				++iCountParen;
				break;
			case '{':
				++iCountBrace;
				break;
			case '[':
				++iCountBracket;
				break;
			case ')':
				--iCountParen;
				break;
			case '}':
				--iCountBrace;
				break;
			case ']':
				--iCountBracket;
				break;
			case '"':
			case '\'':
				bDoCount = !bDoCount;
				break;
			case '\\':
				++iPlace;
				break;
			}
		}
		else
		{
			if( cStr[ iPlace ] == '\'' && cStr[iPlace-1] != '\\' )
			{
				--iCountTick;
				bDoCount = !bDoCount;
			}
			else if( cStr[ iPlace ] == '"'  && cStr[iPlace-1] != '\\' )
			{
				--iCountQuote;
				bDoCount = !bDoCount;
			}
		}

		++iPlace;
	}

	//----[ CHECK FOR ERRORS IN PARSING ]----/
	if( !bDoCount )
	{
		//m_bSyntaxError = true;
		LogFile( ERROR_LOG,"getOpenCloseBracketToken() - ERROR: Unterminated String in <%s>", cStr );
		return -1;
	}
	if( iCountParen != 0 || iCountBrace != 0 || iCountBracket != 0 )
	{
		//m_bSyntaxError = true;
		LogFile( ERROR_LOG,"getOpenCloseBracketToken() - ERROR: Unmatched Parentheses in <%s>", cStr );
		return -1;
	}
	
	return iPlace-1;
}


long getOpenCloseSpecialBlock( char * cStr, char *cOpen, char *cClose, long startInd )
{
	int iCount;
	int iPlace;
	int iOpenLen, iCloseLen;
	bool bDoCount = true;
	bool bInTick = false;

	if( cOpen == NULL || cClose == NULL )
	{
		LogFile( ERROR_LOG, "getOpenCloseSpecialBlock() - ERROR: Opening and closing characters are empty");
		return -1;
	}

	iPlace = findNextStrIndex( cStr, cOpen, startInd );

	if( iPlace == -1 )
	{
		LogFile( ERROR_LOG, "getOpenCloseSpecialBlock( ) - ERROR: Opening block <%s> cannot be found",cOpen );
		return -1;
	}

	if( iPlace > startInd )
	{
		for( int k = startInd; k < iPlace; k++ )
		{
			if( ! isWhiteSpace( cStr[ k ] ) )
			{
				LogFile( ERROR_LOG, "getOpenCloseSpecialBlock( ) - ERROR: non-whitespace character (0x%02x)'%c' found before opening block",cStr[ k ] );
				return -1;
			}
		}
	}

	iOpenLen = strlen( cOpen );
	iCloseLen = strlen( cClose );

	iPlace += iOpenLen; //Move past opening str
	iCount = 1;

	while( iCount > 0 && cStr[ iPlace ] != '\0' )
	{
		if( bDoCount )
		{
			if( strncmp( cOpen, & cStr[iPlace], iOpenLen ) == 0 )
			{
				++iCount;
				iPlace += iOpenLen - 1;
			}
			else if( strncmp( cClose, & cStr[iPlace], iCloseLen ) == 0 )
			{
				--iCount;
				iPlace += iCloseLen - 1;
			}
			else if( cStr[iPlace] == '"' )
			{
					bDoCount = false;
			}
			else if( cStr[iPlace] == '\'' )
			{
				bInTick = true;
				bDoCount = false;
			}
		}
		else
		{
			if( cStr[ iPlace ] == '\'' && cStr[iPlace-1] != '\\' )
			{
				if( bInTick )					//Only restart count if we started w/ a tick
				{
					bInTick = false;
					bDoCount = !bDoCount;
				}
			}
			else if( cStr[ iPlace ] == '"'  && cStr[iPlace-1] != '\\' )
			{
				if( ! bInTick )					//Only restart count if we started w/ a quote
					bDoCount = !bDoCount;
			}
		}

		++iPlace;
	}

	//----[ CHECK FOR ERRORS IN PARSING ]----/
	if( !bDoCount )
	{
		//m_bSyntaxError = true;
		LogFile( ERROR_LOG,"getOpenCloseSpecialBlock() - ERROR: Unterminated String in <%s>", cStr );
		return -1;
	}
	if( iCount != 0 )
	{
		//m_bSyntaxError = true;
		LogFile( ERROR_LOG,"getOpenCloseSpecialBlock() - ERROR: Unmatched special block in <%s>", cStr );
		return -1;
	}
	
	return iPlace-1;
}


char *  __cdecl allToUpper( char *str )
{
	long k;
	long len;
	
	if( !str )
		return str;

	len = strlen( str );
	for( k = 0; k < len; k++ )
		str[k] = toupper( str[k] );

	return str;
}


char *  __cdecl allToLower( char *str )
{
	long k;
	long len;
	
	if( !str )
		return str;

	len = strlen( str );
	for( k = 0; k < len; k++ )
		str[k] = tolower( str[k] );

	return str;
}


bool peelAllOuterParen( char *str, char **res )
{
	long len;
	long start;
	long end;
	long startChar;
	long checkPlace;
	long endChar;
	char *tempStr;
	bool l_bRes;

	if( !str || !res )
		return false;

	len = strlen( str );

//LogFile( ERROR_LOG, "\t\t\tEntered peeling func w/ str [%s] and len %ld",str, len);
	start = findNextCharSetIndex( str, "[{(" );
	end = findNextCharSetIndexRev( str,"]})", len - 1 );

	startChar = getNextCharIndex( str );
	endChar = getNextCharIndexRev( str );

	if( startChar == -1 )
	{
//		LogFile( ERROR_LOG, "peelAllOuterParen( ) - ERROR: Passed an empty string");
		*res = new char[ len + 1 ];
		sprintf( *res, "%s",str );
//LogFile( ERROR_LOG, "\t\t\t--Res is [%s]",*res);
		return true;

	//	return false;
	}

//	if( start == -1 && end == -1 || startChar < start && endChar > end ||		//--[ There are no parens, or none on outside ]--/
//		startChar < start && end == -1 || start == -1 && endChar > end )
	if( start == -1 && end == -1 || startChar < start || endChar > end )
	{
		*res = new char[ len + 1 ];
		sprintf( *res, "%s", str );
		return true;
	}

	if( start == -1 && end == endChar )	//--[ There is a closing Paren, but no open ]--/
	{
		tempStr = new char[ end - startChar + 1 ];
		sprintf( tempStr, "%.*s", end-1, &str[ startChar ] );
		l_bRes = peelAllOuterParen( tempStr, res );
		delete [] tempStr;
		return l_bRes;
	}

	if( end == -1 && start == startChar )	//--[ There is a opening Paren, but no close ]--/
	{
		tempStr = new char[ len - startChar ];
		sprintf( tempStr, "%s", &str[ startChar+1 ] );
		l_bRes = peelAllOuterParen( tempStr, res );
		delete [] tempStr;
		return l_bRes;
	}

	if( (checkPlace = getOpenCloseParen( str, start )) == -1 )	//--[ Open Paren does not match close ]--/
	{
		if( start == startChar )	//--[ Open Paren is starting paren, needs to be peeled ]--/
		{
			tempStr = new char[ endChar - startChar + 1 ];
			sprintf( tempStr, "%.*s",endChar - startChar, &str[ startChar+1 ] );
			l_bRes = peelAllOuterParen( tempStr, res );
			delete [] tempStr;
			return l_bRes;
		}

		*res = new char[ len + 1 ];
		sprintf( *res, "%s",str );
//LogFile( ERROR_LOG, "\t\t\t--Res is [%s]",*res);
		return true;
	}
	else if( checkPlace < end )
	{
		if( end == endChar )
		{
			tempStr = new char[ end - startChar + 1 ];
			sprintf( tempStr, "%.*s", end - startChar, &str[ startChar ] );
			l_bRes = peelAllOuterParen( tempStr, res );
			delete [] tempStr;
			return l_bRes;
		}

		*res = new char[ len + 1 ];
		sprintf( *res, "%s",str );
//LogFile( ERROR_LOG, "\t\t\t--Res is [%s]",*res);
		return true;
	}
	else if( checkPlace == end && end == endChar && start == startChar )
	{
		tempStr = new char[ end - start ];
		sprintf( tempStr, "%.*s", end - start - 1, &str[ start + 1 ] );
		l_bRes = peelAllOuterParen( tempStr, res );  //NOTE TO ME - WHY IS THIS RECURSIVE?  What else would removing recursion affect????????????????????????????????????????????????????????????????????
		delete [] tempStr;
		return l_bRes;
	}

	*res = new char[ len + 1 ];
	sprintf( *res, "%s",str );
//LogFile( ERROR_LOG, "\t\t\t--Res is [%s]",*res);
	return true;
}

/*****************************************
LogFile( ERROR_LOG, "\t\t\t--About to copy peeled str[%s]",str);
		*res = new char[ len + 1 ];
		sprintf( *res, "%s",str );
LogFile( ERROR_LOG, "\t\t\t--Res is [%s]",*res);
		return;
	}

	if( ( checkPlace = getNextCharIndex( str ) ) == start )
	{
		if( (end = getOpenCloseParen( str, start )) == -1 )		//--[ If TRUE, We Start w/ An open Paren, but no Close ]--/
		{
			peelAllOuterParen( &str[ start + 1 ], res );
			return;
		}
	
		checkPlace = getNextCharIndexRev( str, len - 1 );

		if( checkPlace == end )		//--[ Need To Peel Off Outer Parens ]--/
		{
			tempStr = new char[ end - start ];
			sprintf( tempStr, "%.*s",end - start - 1, &str[start+1] );
			peelAllOuterParen( tempStr, res );
			delete [] tempStr;
			return;
		}
	}

	*res = new char[ len + 1 ];
	sprintf( *res, "%s",str );
}
*********************************************************/


bool peelAllOuterSpecialBlock(	char * str, 
								char *cOpen, 
								char *cClose,
								char **res )
{
	long len;
	long start;
	long end;
	long startChar;
	long checkPlace;
	long endChar;
	char *tempStr;
	bool l_bRes;

	if( !str || !cOpen || !cClose || !res )
		return false;

	len = strlen( str );

//LogFile( ERROR_LOG, "\t\t\tEntered peeling func w/ str [%s] and len %ld",str, len);
	start = findNextStrIndex( str, cOpen );
	end = findNextStrIndexRev( str, cClose, len - 1 );

	startChar = getNextCharIndex( str );
	endChar = getNextCharIndexRev( str );

	if( startChar == -1 )
	{
//		LogFile( ERROR_LOG, "peelAllOuterParen( ) - ERROR: Passed an empty string");
		*res = new char[ len + 1 ];
		sprintf( *res, "%s",str );
//LogFile( ERROR_LOG, "\t\t\t--Res is [%s]",*res);
		return true;

	//	return false;
	}

	if( start == -1 && end == -1 || startChar < start && endChar > end ||		//--[ There are no parens, or none on outside ]--/
		startChar < start && end == -1 || start == -1 && endChar > end )
	{
		*res = new char[ len + 1 ];
		sprintf( *res, "%s", str );
		return true;
	}

	if( start == -1 && end == endChar )	//--[ There is a closing Paren, but no open ]--/
	{
		tempStr = new char[ end - startChar + 1 ];
		sprintf( tempStr, "%.*s", end-1, &str[ startChar ] );
		l_bRes = peelAllOuterSpecialBlock( tempStr, cOpen, cClose, res );
		delete [] tempStr;
		return l_bRes;
	}

	if( end == -1 && start == startChar )	//--[ There is a opening Paren, but no close ]--/
	{
		tempStr = new char[ len - startChar ];
		sprintf( tempStr, "%s", &str[ startChar+1 ] );
		l_bRes = peelAllOuterSpecialBlock( tempStr, cOpen, cClose, res );
		delete [] tempStr;
		return l_bRes;
	}

	if( (checkPlace = getOpenCloseSpecialBlock( str, cOpen, cClose, start )) == -1 )	//--[ Open Paren does not match close ]--/
	{
		if( start == startChar )	//--[ Open Paren is starting paren, needs to be peeled ]--/
		{
			tempStr = new char[ endChar - startChar + 1 ];
			sprintf( tempStr, "%.*s",endChar - startChar, &str[ startChar+1 ] );
			l_bRes = peelAllOuterSpecialBlock( tempStr, cOpen, cClose, res );
			delete [] tempStr;
			return l_bRes;
		}

		*res = new char[ len + 1 ];
		sprintf( *res, "%s",str );
//LogFile( ERROR_LOG, "\t\t\t--Res is [%s]",*res);
		return true;
	}
	else if( checkPlace < end )
	{
		if( end == endChar )
		{
			tempStr = new char[ end - startChar + 1 ];
			sprintf( tempStr, "%.*s", end - startChar, &str[ startChar ] );
			l_bRes = peelAllOuterSpecialBlock( tempStr, cOpen, cClose, res );
			delete [] tempStr;
			return l_bRes;
		}

		*res = new char[ len + 1 ];
		sprintf( *res, "%s",str );
//LogFile( ERROR_LOG, "\t\t\t--Res is [%s]",*res);
		return true;
	}
	else if( checkPlace == end && end == endChar && start == startChar )
	{
		tempStr = new char[ end - start ];
		sprintf( tempStr, "%.*s", end - start - 1, &str[ start + 1 ] );
		l_bRes = peelAllOuterSpecialBlock( tempStr, cOpen, cClose, res );
		delete [] tempStr;
		return l_bRes;
	}

	*res = new char[ len + 1 ];
	sprintf( *res, "%s",str );
//LogFile( ERROR_LOG, "\t\t\t--Res is [%s]",*res);
	return true;
}


bool peelAllOuterQuotes( char *str, char **res )
{
	long len;
	long start;
	long end;
	long startChar;
	long checkPlace;
	long endChar;
	char *tempStr;
	bool l_bRes;

	if( !str || !res )
		return false;

	len = strlen( str );

//LogFile( ERROR_LOG, "\t\t\tEntered peeling func w/ str [%s] and len %ld",str, len);
	start = findNextCharSetIndex( str, "\"" );
	end = findNextCharSetIndexRev( str,"\"", len - 1 );

	startChar = getNextCharIndex( str );
	endChar = getNextCharIndexRev( str );

	if( startChar == -1 )
	{
//		LogFile( ERROR_LOG, "peelAllOuterQuotes( ) - ERROR: Passed an empty string");
		*res = new char[ len + 1 ];
		sprintf( *res, "%s",str );
//LogFile( ERROR_LOG, "\t\t\t--Res is [%s]",*res);
		return true;

	//	return false;
	}

	if( start == -1 && end == -1 || startChar < start && endChar > end ||		//--[ There are no parens, or none on outside ]--/
		startChar < start && end == start || start == end && endChar > end )
	{
		*res = new char[ len + 1 ];
		sprintf( *res, "%s", str );
		return true;
	}

	if( start == end && end == endChar )	//--[ There is a closing Paren, but no open ]--/
	{
		tempStr = new char[ end - startChar + 1 ];
		sprintf( tempStr, "%.*s", end-1, &str[ startChar ] );
		l_bRes = peelAllOuterQuotes( tempStr, res );
		delete [] tempStr;
		return l_bRes;
	}

	if( end == start && start == startChar )	//--[ There is a opening Paren, but no close ]--/
	{
		tempStr = new char[ len - startChar ];
		sprintf( tempStr, "%s", &str[ startChar+1 ] );
		l_bRes = peelAllOuterQuotes( tempStr, res );
		delete [] tempStr;
		return l_bRes;
	}

	if( (checkPlace = getOpenCloseParen( str, start )) == -1 )	//--[ Open Paren does not match close ]--/
	{
		if( start == startChar )	//--[ Open Paren is starting paren, needs to be peeled ]--/
		{
			tempStr = new char[ endChar - startChar + 1 ];
			sprintf( tempStr, "%.*s",endChar - startChar, &str[ startChar+1 ] );
			l_bRes = peelAllOuterQuotes( tempStr, res );
			delete [] tempStr;
			return l_bRes;
		}

		*res = new char[ len + 1 ];
		sprintf( *res, "%s",str );
//LogFile( ERROR_LOG, "\t\t\t--Res is [%s]",*res);
		return true;
	}
	else if( checkPlace < end )
	{
		if( end == endChar )
		{
			tempStr = new char[ end - startChar + 1 ];
			sprintf( tempStr, "%.*s", end - startChar, &str[ startChar ] );
			l_bRes = peelAllOuterQuotes( tempStr, res );
			delete [] tempStr;
			return l_bRes;
		}

		*res = new char[ len + 1 ];
		sprintf( *res, "%s",str );
//LogFile( ERROR_LOG, "\t\t\t--Res is [%s]",*res);
		return true;
	}
	else if( checkPlace == end && end == endChar && start == startChar )
	{
		tempStr = new char[ end - start ];
		sprintf( tempStr, "%.*s", end - start - 1, &str[ start + 1 ] );
		l_bRes = peelAllOuterQuotes( tempStr, res );
		delete [] tempStr;
		return l_bRes;
	}

	*res = new char[ len + 1 ];
	sprintf( *res, "%s",str );
//LogFile( ERROR_LOG, "\t\t\t--Res is [%s]",*res);
	return true;
}


int nonCaseCmp( const char *s1, const char *s2 )
{
	long res;
	char *t_s1 = NULL;
	char *t_s2 = NULL;

	if( !s1 && !s2 )
		return 0;
	if( !s1 )
		return 1;
	if( !s2 )
		return -1;

	t_s1 = new char[ strlen( s1 ) + 1 ];
	t_s2 = new char[ strlen( s2 ) + 1 ];

	sprintf( t_s1, "%s",s1);
	sprintf( t_s2, "%s",s2);

	allToLower( t_s1 );
	allToLower( t_s2 );

	res = strcmp( t_s1, t_s2 );

	delete [] t_s1;
	delete [] t_s2;

	return res;
}


int nonCaseNCmp( const char *s1, const char *s2, size_t len )
{
	long res;
	char *t_s1 = NULL;
	char *t_s2 = NULL;

	if( !s1 && !s2 )
		return 0;
	if( !s1 )
		return 1;
	if( !s2 )
		return -1;

	t_s1 = new char[ len + 1 ];
	t_s2 = new char[ len + 1 ];

	sprintf( t_s1, "%.*s",len, s1);
	sprintf( t_s2, "%.*s",len, s2);

	allToLower( t_s1 );
	allToLower( t_s2 );

	res = strncmp( t_s1, t_s2, len );

	delete [] t_s1;
	delete [] t_s2;

	return res;
}

/*
mvector<string> getConfigVector( string & sTokenList, char *pDelimiter, bool bPeelOuterWhitespace )
{
	mvector<string>	vReturnVec;

	long startInd = 0;
	long nextDelim = 0;
	long nextParen = 0;
	long tokenStart = 0;
	long tokenEnd = 0;

	long strSize = sTokenList.size();
	long k = 0;

	char *str;

	if( ! strSize )
		return vReturnVec;

	++strSize;

	str = new char[ strSize ];

	sprintf( str, "%s", sTokenList.c_str( ) );

	k = findNextNotCharSetIndex( str, pDelimiter, k );

	while( k < strSize )
	{
		//if( isOpenParenBlock( str[ k ] ) )
		//if( isWhiteSpace( str[ k ] ) )
		tokenStart = findNextNotCharSetIndex( str, pDelimiter, k );

		nextDelim = findNextCharSetIndex( str, pDelimiter, k );

		if( nextDelim == -1 )
			nextDelim = strSize;

		tokenStart = getNextCharIndex( str, k );

		k = findNextNotCharSetIndex( str, pDelimiter, nextDelim );
	}

		if( tokenStart != -1 )
		
		
		nextParen = findNextStrIndex( str, "\"{[(", k );

		if( nextDelim == -1 )
			nextDelim = strSize;

		if( nextParen != -1 && nextParen <= nextDelim )
		{
			if( getNextCharIndex( str, k ) == nextParen )

		if( ( nextDelim = findNextCharSetIndex( pDelimiter, &( str[k] ) ) ) != -1 )
		{
			if( isWhiteSpace( str[ k ] ) )
			if( isOpenParenBlock( str[ k ] ) )
			{
				endToken = getOpenCloseParen( str, k );
}
}}
}

*/

/*
mvector<mstring> getTokenVector( const mstring & sTokenList, char *pDelimiter, bool bPeelOuterWhitespace, bool bPeelOuterQuotes )
{
	mvector<mstring>	vReturnVec;
	char *first;
	char *other;
	char *rest;
	char *junk;
	char *tempTkn;
	char readStr[256];

	long strSize = sTokenList.size();
	int k = 0;
	
	if( ! strSize )
		return vReturnVec;

	++strSize;
	first	= new char[strSize];
	other	= new char[strSize];
	rest	= new char[strSize];
	junk	= new char[strSize];
	tempTkn = new char[strSize];

	
	int startIndex;
	
	if( bPeelOuterWhitespace )
		startIndex = sTokenList.find_first_not_of(" \t\n");
	else
		startIndex = 0;

	if( sTokenList.find_first_not_of( pDelimiter ) > startIndex )
		startIndex = sTokenList.find_first_not_of( pDelimiter );

//	LogFile(/*LOG_LEVEL_TEST,*//*TEST_LOG,"ConfigReader::MakeTokenVector - DEBUG: startIndex: %d\tString: '%s'", startIndex, sTokenList.c_str());

	if( startIndex == -1 )
		return vReturnVec;

	sprintf( readStr,"%c[^%s]%c[%s]%c[^`]",'%',pDelimiter,'%',pDelimiter,'%');
	sprintf( rest,"%.*s",strSize-1,(sTokenList.substr(startIndex)).c_str());

//	LogFile( TEST_LOG, "The 'rest' string is [%s]",rest);

	while( rest[0] != '\0' )
	{
		other[0] = first[0] = junk[0] = 0;
		sscanf( rest, readStr, first, junk, other );
	//	LogFile(/*LOG_LEVEL_TEST,*//*TEST_LOG,"ConfigReader::MakeTokenVector - DEBUG: token:'%s'", first);
		
		if( strcmp( other, rest ) == 0 )
		{
			LogFile( /*LOG_LEVEL_ERROR,*//*ERROR_LOG,"ConfigReader::MakeTokenVector - ERROR: last string remainder matches current string remainder -> Endless loop termination\n\tlast [%s]\n\tcur  [%s]",
				rest,other);
			LogFile( /*LOG_LEVEL_TEST,*//*TEST_LOG,"ConfigReader::MakeTokenVector - ERROR: Endless loop termination");
			delete [] first;
			delete [] other;
			delete [] rest;
			delete [] junk;
			delete [] tempTkn;

			return vReturnVec;
		}

		if( first[0] != '\0' )
		{
			if( bPeelOuterWhitespace || bPeelOuterQuotes )
			{
				long lStartToken = getNextCharIndex( first );
				long lEndToken = getNextCharIndexRev( first );
				char *tempRes = NULL;
				
				if( bPeelOuterWhitespace )
					sprintf( tempTkn, "%.*s", lEndToken - lStartToken + 1, &(first[ lStartToken ]) );
				else
					sprintf( tempTkn, "%s", first );

				if( bPeelOuterQuotes )
				{
					
					if( peelAllOuterQuotes( tempTkn, &tempRes ) )
					{
						vReturnVec.push_back( mstring( tempRes ) );
						delete [] tempRes;
					}
					else
						vReturnVec.push_back( mstring( tempTkn ) );
				}
				else
				{
					vReturnVec.push_back( mstring( tempTkn ) );
					//vReturnVec.push_back( string( first ).substr( lStartToken, lEndToken - lStartToken + 1 ) );
				}
			}
			else
				vReturnVec.push_back( mstring(first));
			//LogFile( TEST_LOG,"\tCheck - Size is %ld",vReturnVec.size());
		}

		sprintf( rest, "%.*s",strSize-1,other);
	}

	delete [] first;
	delete [] other;
	delete [] rest;
	delete [] junk;
	delete [] tempTkn;

	return vReturnVec;
}



mvector<mstring> getTokenVector( mstring & sTokenList, char *pDelimiter, bool bPeelOuterWhitespace, bool bPeelOuterQuotes )
{
	mvector<mstring>	vReturnVec;
	char *first;
	char *other;
	char *rest;
	char *junk;
	char *tempTkn;
	char readStr[256];

	long strSize = sTokenList.size();
	int k = 0;
	
	if( ! strSize )
		return vReturnVec;

	++strSize;
	first	= new char[strSize];
	other	= new char[strSize];
	rest	= new char[strSize];
	junk	= new char[strSize];
	tempTkn = new char[strSize];

	
	int startIndex;
	
	if( bPeelOuterWhitespace )
		startIndex = sTokenList.find_first_not_of(" \t\n");
	else
		startIndex = 0;

	if( sTokenList.find_first_not_of( pDelimiter ) > startIndex )
		startIndex = sTokenList.find_first_not_of( pDelimiter );

//	LogFile(/*LOG_LEVEL_TEST,*//*TEST_LOG,"ConfigReader::MakeTokenVector - DEBUG: startIndex: %d\tString: '%s'", startIndex, sTokenList.c_str());

	if( startIndex == -1 )
		return vReturnVec;

	sprintf( readStr,"%c[^%s]%c[%s]%c[^`]",'%',pDelimiter,'%',pDelimiter,'%');
	sprintf( rest,"%.*s",strSize-1,(sTokenList.substr(startIndex)).c_str());

//	LogFile( TEST_LOG, "The 'rest' string is [%s]",rest);

	while( rest[0] != '\0' )
	{
		other[0] = first[0] = junk[0] = 0;
		sscanf( rest, readStr, first, junk, other );
	//	LogFile(/*LOG_LEVEL_TEST,*//*TEST_LOG,"ConfigReader::MakeTokenVector - DEBUG: token:'%s'", first);
		
		if( strcmp( other, rest ) == 0 )
		{
			LogFile( /*LOG_LEVEL_ERROR,*//*ERROR_LOG,"ConfigReader::MakeTokenVector - ERROR: last string remainder matches current string remainder -> Endless loop termination\n\tlast [%s]\n\tcur  [%s]",
				rest,other);
			LogFile( /*LOG_LEVEL_TEST,*//*TEST_LOG,"ConfigReader::MakeTokenVector - ERROR: Endless loop termination");
			delete [] first;
			delete [] other;
			delete [] rest;
			delete [] junk;
			delete [] tempTkn;

			return vReturnVec;
		}

		if( first[0] != '\0' )
		{
			if( bPeelOuterWhitespace || bPeelOuterQuotes )
			{
				long lStartToken = getNextCharIndex( first );
				long lEndToken = getNextCharIndexRev( first );
				char *tempRes = NULL;
				
				if( bPeelOuterWhitespace )
					sprintf( tempTkn, "%.*s", lEndToken - lStartToken + 1, &(first[ lStartToken ]) );
				else
					sprintf( tempTkn, "%s", first );

				if( bPeelOuterQuotes )
				{
					
					if( peelAllOuterQuotes( tempTkn, &tempRes ) )
					{
						vReturnVec.push_back( mstring( tempRes ) );
						delete [] tempRes;
					}
					else
						vReturnVec.push_back( mstring( tempTkn ) );
				}
				else
				{
					vReturnVec.push_back( mstring( tempTkn ) );
					//vReturnVec.push_back( string( first ).substr( lStartToken, lEndToken - lStartToken + 1 ) );
				}
			}
			else
				vReturnVec.push_back( mstring(first));
			//LogFile( TEST_LOG,"\tCheck - Size is %ld",vReturnVec.size());
		}

		sprintf( rest, "%.*s",strSize-1,other);
	}

	delete [] first;
	delete [] other;
	delete [] rest;
	delete [] junk;
	delete [] tempTkn;

	return vReturnVec;
}

*/

mvector<mstring> getTokenVector( mstring & sTokenList, char *pDelimiter, bool bPeelOuterWhitespace, bool bPeelOuterQuotes )
{
	mvector<mstring>	vReturnVec;
	char *first;
	char *rest;
	char *tempTkn;

	long strSize = sTokenList.size();
	long tempIndex;
	long k = 0;
	
	if( ! strSize )
		return vReturnVec;

	++strSize;

	first	= new char[strSize];
	//rest	= new char[strSize];   -- This is now allocated by the mstring::getC_Str( ) method
	tempTkn = new char[strSize];

	
	long startIndex;
	
	if( bPeelOuterWhitespace )
		startIndex = sTokenList.find_first_not_of(" \t\n");
	else
		startIndex = 0;

	tempIndex = sTokenList.find_first_not_of( pDelimiter );

	if( tempIndex > startIndex )
		startIndex = tempIndex;

//	LogFile(/*LOG_LEVEL_TEST,*/TEST_LOG,"ConfigReader::MakeTokenVector - DEBUG: startIndex: %d\tString: '%s'", startIndex, sTokenList.c_str());

	if( startIndex == -1 )
		return vReturnVec;

	rest = sTokenList.getC_Str( );
//	LogFile( TEST_LOG, "The 'rest' string is [%s]",rest);

	while( rest[ startIndex ] != '\0' )
	{
		tempTkn[ 0 ] = '\0';

		tempIndex = findNextCharSetIndex( rest, pDelimiter, startIndex );

		if( tempIndex == startIndex )
		{
			LogFile( /*LOG_LEVEL_ERROR,*/ERROR_LOG,"ConfigReader::MakeTokenVector - ERROR: last string remainder matches current string remainder -> Endless loop termination\n\tlast [%s]\n\tcur  [%s]",
				rest,first);
			LogFile( /*LOG_LEVEL_TEST,*/TEST_LOG,"ConfigReader::MakeTokenVector - ERROR: Endless loop termination");
			delete [] first;
			delete [] rest;
			delete [] tempTkn;

			return vReturnVec;
		}

		if( tempIndex == -1 )			//End of the line?
			tempIndex = strSize - 1;
		
		//---[ Copy Over the next token ]---/
		memcpy( (void *)first, (void *)(&rest[ startIndex ]), tempIndex - startIndex );
		first[ tempIndex - startIndex ] = '\0';


		if( first[0] != '\0' )
		{
			if( bPeelOuterWhitespace || bPeelOuterQuotes )
			{
				long lStartToken = getNextCharIndex( first );
				long lEndToken = getNextCharIndexRev( first );
				char *tempRes = NULL;

				if( lStartToken == -1 )
					lStartToken = 0;

				if( bPeelOuterWhitespace )
					sprintf( tempTkn, "%.*s", lEndToken - lStartToken + 1, &(first[ lStartToken ]) );
				else
					sprintf( tempTkn, "%s", first );

				if( bPeelOuterQuotes )
				{
					
					if( peelAllOuterQuotes( tempTkn, &tempRes ) )
					{
						vReturnVec.push_back( mstring( tempRes ) );
						delete [] tempRes;
					}
					else
						vReturnVec.push_back( mstring( tempTkn ) );
				}
				else
				{
					vReturnVec.push_back( mstring( tempTkn ) );
					//vReturnVec.push_back( string( first ).substr( lStartToken, lEndToken - lStartToken + 1 ) );
				}
			}
			else
				vReturnVec.push_back( mstring(first));
			//LogFile( TEST_LOG,"\tCheck - Size is %ld",vReturnVec.size());
		}

		startIndex = tempIndex;
		tempIndex = findNextNotCharSetIndex( rest, pDelimiter, startIndex );

		if( tempIndex == -1 )
			startIndex = strSize - 1;
		else
			startIndex = tempIndex;
	}

	delete [] first;
	delete [] rest;

	return vReturnVec;
}



mvector<mstring> getTokenVector( const mstring & sTokenList, char *pDelimiter, bool bPeelOuterWhitespace, bool bPeelOuterQuotes )
{
	mvector<mstring>	vReturnVec;
	char *first;
	char *rest;
	char *tempTkn;

	long strSize = sTokenList.size();
	long tempIndex;
	long k = 0;
	
	if( ! strSize )
		return vReturnVec;

	++strSize;

	first	= new char[strSize];
	//rest	= new char[strSize];   -- This is now allocated by the mstring::getC_Str( ) method
	tempTkn = new char[strSize];

	
	long startIndex;
	
	if( bPeelOuterWhitespace )
		startIndex = sTokenList.find_first_not_of(" \t\n");
	else
		startIndex = 0;

	tempIndex = sTokenList.find_first_not_of( pDelimiter );

	if( tempIndex > startIndex )
		startIndex = tempIndex;

//	LogFile(/*LOG_LEVEL_TEST,*/TEST_LOG,"ConfigReader::MakeTokenVector - DEBUG: startIndex: %d\tString: '%s'", startIndex, sTokenList.c_str());

	if( startIndex == -1 )
		return vReturnVec;

	rest = sTokenList.getC_Str( );
//	LogFile( TEST_LOG, "The 'rest' string is [%s]",rest);

	while( rest[ startIndex ] != '\0' )
	{
		tempTkn[ 0 ] = '\0';

		tempIndex = findNextCharSetIndex( rest, pDelimiter, startIndex );

		if( tempIndex == startIndex )
		{
			LogFile( /*LOG_LEVEL_ERROR,*/ERROR_LOG,"ConfigReader::MakeTokenVector - ERROR: last string remainder matches current string remainder -> Endless loop termination\n\tlast [%s]\n\tcur  [%s]",
				rest,first);
			LogFile( /*LOG_LEVEL_TEST,*/TEST_LOG,"ConfigReader::MakeTokenVector - ERROR: Endless loop termination");
			delete [] first;
			delete [] rest;
			delete [] tempTkn;

			return vReturnVec;
		}

		if( tempIndex == -1 )			//End of the line?
			tempIndex = strSize - 1;
		
		//---[ Copy Over the next token ]---/
		memcpy( (void *)first, (void *)(&rest[ startIndex ]), tempIndex - startIndex );
		first[ tempIndex ] = '\0';


		if( first[0] != '\0' )
		{
			if( bPeelOuterWhitespace || bPeelOuterQuotes )
			{
				long lStartToken = getNextCharIndex( first );
				long lEndToken = getNextCharIndexRev( first );
				char *tempRes = NULL;

				if( lStartToken == -1 )
					lStartToken = 0;
				
				if( bPeelOuterWhitespace )
					sprintf( tempTkn, "%.*s", lEndToken - lStartToken + 1, &(first[ lStartToken ]) );
				else
					sprintf( tempTkn, "%s", first );

				if( bPeelOuterQuotes )
				{
					
					if( peelAllOuterQuotes( tempTkn, &tempRes ) )
					{
						vReturnVec.push_back( mstring( tempRes ) );
						delete [] tempRes;
					}
					else
						vReturnVec.push_back( mstring( tempTkn ) );
				}
				else
				{
					vReturnVec.push_back( mstring( tempTkn ) );
					//vReturnVec.push_back( string( first ).substr( lStartToken, lEndToken - lStartToken + 1 ) );
				}
			}
			else
				vReturnVec.push_back( mstring(first));
			//LogFile( TEST_LOG,"\tCheck - Size is %ld",vReturnVec.size());
		}

		startIndex = tempIndex;
		tempIndex = findNextNotCharSetIndex( rest, pDelimiter, startIndex );

		if( tempIndex == -1 )
			startIndex = strSize - 1;
		else
			startIndex = tempIndex;
	}

	delete [] first;
	delete [] rest;

	return vReturnVec;
}




/*
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
*/

mstring baseName( mstring & _sFilename )
{
	return removeDir( removeExt( _sFilename ) );
}

//################################################333
//# ALTERNATE BASENAME FUNCTION
//##################################################
/*
string baseName( string & _sFilename )
{
	long l_pcSlash = _sFilename.find_last_of("\\");
	long l_pcDot = _sFilename.find_last_of(".");

	if( _sFilename.find_first_not_of("\n\t\r ") == -1 )
		return string("");

	if( l_pcSlash < _sFilename.find_last_of("/") )
		l_pcSlash = _sFilename.find_last_of("/");

	if( l_pcDot == -1 )
	{
		if( l_pcSlash == -1 )
		{
			return _sFilename;
		}
		
		return _sFilename.substr( l_pcSlash + 1 );
	}

	if( l_pcSlash == -1 )
		return _sFilename.substr( 0, l_pcDot );

	return _sFilename.substr( l_pcSlash + 1, l_pcDot - ( l_pcSlash + 1 ) );
}
*/

mstring dirName( mstring & _sFilename )
{
	long l_pcSlash = _sFilename.find_last_of('\\');
	long l_pcAltSlash = _sFilename.find_last_of('/');

	if( _sFilename.find_first_not_of("\n\t\r ") == -1 )
		return string("");

	if( l_pcAltSlash > l_pcSlash )
		l_pcSlash = l_pcAltSlash;

	if( l_pcSlash == -1 )
		return string("");

	return _sFilename.substr( 0, l_pcSlash + 1 );
}


mstring extName( mstring & _sFilename )
{
	long l_pcDot = _sFilename.find_last_of(".");

	if( _sFilename.find_first_not_of("\n\t\r ") == -1 )
		return mstring("");

	if( l_pcDot == -1 )
	{
		return mstring("");
	}

	return _sFilename.substr( l_pcDot );
}


mstring removeDir( mstring & _sFilename )
{
	long l_pcSlash = _sFilename.find_last_of("\\");

	if( _sFilename.find_first_not_of("\n\t\r ") == -1 )
		return mstring("");

	if( l_pcSlash < _sFilename.find_last_of("/") )
		l_pcSlash = _sFilename.find_last_of("/");

	if( l_pcSlash == -1 )
		return _sFilename;

	return _sFilename.substr( l_pcSlash + 1 );
}


mstring removeExt( mstring & _sFilename )
{
	long l_pcDot = _sFilename.find_last_of(".");

	if( _sFilename.find_first_not_of("\n\t\r ") == -1 )
		return mstring("");

	if( l_pcDot == -1 )
	{
		return _sFilename;
	}

	return _sFilename.substr( 0, l_pcDot );
}




//##################

//----------------------------------------------------------------//
//- getTokenVector - Sub Token Version                           -//
//-   This version incorporates a sub-delimiter to group tokens  -//
//-   that are surrounded by some grouping character. ie If the  -//
//-   delimiter is ' ', and the sub delimiter is '"', then any   -//
//-   text inside the '"'s will be treated as a single token,    -//
//-   even if it has ' 's inside the '"'s.  This is the simple   -//
//-   version (don't know if I'll make an advanced).  It treats  -//
//-   the first occurance of any character in the sub delimiter  -//
//-   set as the open, and the next as the close:  It does not   -//
//-   do nested tokens.  ie Sub Delimiter is '{}', the string    -//
//-   '{ a { b } c }' does not result in ' a { b } c ', it will  -//
//-   return ' a ', ' b ', ' c '.  Also, the sub-delimiter must  -//
//-   start a token and end a token.                             -//
//----------------------------------------------------------------//
mvector<mstring> getTokenVector( mstring & sTokenList, char *pDelimiter, char *pSubDelimiter, bool bPeelOuterWhitespace, bool bPeelOuterQuotes )
{
	mvector<mstring>	vReturnVec;
	char *first;
	char *rest;
	char *tempTkn;
	char *comboDel;

	long strSize = sTokenList.size();
	long tempIndex;
	long k = 0;
	
	bool inSubToken = false;

	if( ! strSize )
		return vReturnVec;

	++strSize;

	first	= new char[strSize];
	//rest	= new char[strSize];   -- This is now allocated by the mstring::getC_Str( ) method
	tempTkn = new char[strSize];
	
	//--[ Create a combined delimiter if needed for sub-delimitation ]--/
	if( bPeelOuterWhitespace )
	{
		comboDel= new char[ strlen( pDelimiter ) + strlen(" \t\n") + 1 ];
		sprintf( comboDel, "%s%s", pDelimiter, " \t\n" );
	}
	else
		comboDel = pDelimiter;

	//---[ Determine the Starting Place ]---/
	long startIndex;
	
	startIndex = sTokenList.find_first_not_of( comboDel );

	if( startIndex == -1 )
		return vReturnVec;


	//--[ Generate initial remainder string ]--/
	rest = sTokenList.getC_Str( );

//	LogFile( TEST_LOG, "The 'rest' string is [%s]",rest);

	while( rest[ startIndex ] != '\0' )
	{

		tempTkn[ 0 ] = '\0';

		if( inSubToken )
			tempIndex = findNextCharSetIndex( rest, pSubDelimiter, startIndex );
		else
			tempIndex = findNextCharSetIndex( rest, pDelimiter, startIndex );

		if( tempIndex == startIndex )
		{
			LogFile( /*LOG_LEVEL_ERROR,*/ERROR_LOG,"ConfigReader::MakeTokenVector - ERROR: last string remainder matches current string remainder -> Endless loop termination\n\tlast [%s]\n\tcur  [%s]",
				rest,first);
			LogFile( /*LOG_LEVEL_TEST,*/TEST_LOG,"ConfigReader::MakeTokenVector - ERROR: Endless loop termination");
			delete [] first;
			delete [] rest;
			delete [] tempTkn;

			if( bPeelOuterWhitespace )
				delete [] comboDel;

			return vReturnVec;
		}

		if( tempIndex == -1 )			//End of the line?
			tempIndex = strSize - 1;
		
		//---[ Copy Over the next token ]---/
		memcpy( (void *)first, (void *)(&rest[ startIndex ]), tempIndex - startIndex );
		first[ tempIndex - startIndex ] = '\0';


		if( first[0] != '\0' )
		{
			if( bPeelOuterWhitespace || bPeelOuterQuotes )
			{
				long lStartToken = getNextCharIndex( first );
				long lEndToken = getNextCharIndexRev( first );
				char *tempRes = NULL;

				if( lStartToken == -1 )
					lStartToken = 0;
				
				if( bPeelOuterWhitespace )
					sprintf( tempTkn, "%.*s", lEndToken - lStartToken + 1, &(first[ lStartToken ]) );
				else
					sprintf( tempTkn, "%s", first );

				if( bPeelOuterQuotes )
				{
					
					if( peelAllOuterQuotes( tempTkn, &tempRes ) )
					{
						vReturnVec.push_back( mstring( tempRes ) );
						delete [] tempRes;
					}
					else
						vReturnVec.push_back( mstring( tempTkn ) );
				}
				else
				{
					vReturnVec.push_back( mstring( tempTkn ) );
					//vReturnVec.push_back( string( first ).substr( lStartToken, lEndToken - lStartToken + 1 ) );
				}
			}
			else
				vReturnVec.push_back( mstring(first));
			//LogFile( TEST_LOG,"\tCheck - Size is %ld",vReturnVec.size());
		}

		startIndex = tempIndex;

		if( inSubToken )
		{
			tempIndex = findNextCharSetIndex( rest, pDelimiter, startIndex );
			inSubToken = false;

			if( tempIndex == -1 )
				startIndex = strSize - 1;
			else
				startIndex = tempIndex;
		
		}
		
		tempIndex = findNextNotCharSetIndex( rest, pDelimiter, startIndex );

		if( tempIndex == -1 )
			startIndex = strSize - 1;
		else
			startIndex = tempIndex;

		if( findNextCharIndex( pSubDelimiter, rest[ startIndex ] ) != -1 )
		{
			inSubToken = true;
			tempIndex = findNextNotCharSetIndex( rest, pSubDelimiter, startIndex );

			if( tempIndex == -1 )
				startIndex = strSize - 1;
			else
				startIndex = tempIndex;
		}
	}

	delete [] first;
	delete [] rest;

	if( bPeelOuterWhitespace )
		delete [] comboDel;

	return vReturnVec;
}


//##########################
