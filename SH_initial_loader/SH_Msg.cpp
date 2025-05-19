#include <windows.h>
#include <stdio.h>
#include <vector>
#include <errno.h>

#include "SH3_ArcFilenames.h"
#include "SH_Msg.h"
#include "SH3_Loader.h"
#include "typedefs.h"

using std::vector;

extern char baseSH3dir[ 256 ];
extern int errno;

long SH3_MsgMgr::LoadMsgs( char *_pcFilename, long _lIndexNum )
{
	char *l_pcData;
	char *l_pcCurData;
	char *l_pcEndData;
	short *l_psControl;
	arc_index_data	l_cFileIndex;
	SH3_Msg			l_cMsg;
	vector<SH3_Msg>	l_vMsgs;
	vector<short>	l_vIDs;
	FILE			*inFile;
	long			l_lRes;
	long			k;
	long			l_lCurIDIndex = 0;

	if( !_pcFilename )
	{
		LogFile( ERROR_LOG, "SH3_MsgMgr::LoadMsgs( ) - ERROR: No filename was passed in. Cannot load");
		return 0; 
	}
	
	if( !l_cFileIndex.LoadData( _pcFilename ) )
	{
		LogFile( ERROR_LOG, "SH3_MsgMgr::LoadMsgs( ) - ERROR: Couldn't load index data from '%s'", _pcFilename );
		return 0;
	}


	if( ( inFile = fopen( _pcFilename, "rb" ) ) == NULL )
	{
		LogFile( ERROR_LOG, "SH3_MsgMgr::LoadMsgs( ) - ERROR: Couldn't open '%s' - REASON: %s", _pcFilename, strerror( errno ) );
		return 0;
	}

	LogFile( ERROR_LOG, "SH3_MsgMgr::LoadMsgs( ) - Start '%s'", _pcFilename );
		
	fseek( inFile, l_cFileIndex.index[ _lIndexNum ].offset, SEEK_SET );

	l_pcData = new char[ l_cFileIndex.index[ _lIndexNum ].size + sizeof(short)];

	if( (l_lRes = fread( l_pcData, 1, l_cFileIndex.index[ _lIndexNum ].size, inFile )) < l_cFileIndex.index[ _lIndexNum ].size )
	{
		LogFile( ERROR_LOG, "SH3_MsgMgr::LoadMsgs( ) - ERROR: Couldn't read data from '%s' - REASON: %s", _pcFilename, strerror( errno ) );
		delete [] l_pcData;
		fclose( inFile );
		return 0;
	}

	l_pcCurData = l_pcData;
	l_pcEndData = &(l_pcData[ l_cFileIndex.index[ _lIndexNum ].size - 1 ]);

	l_psControl = (short *)l_pcCurData;

	k = 0;
LogFile( ERROR_LOG, "SH3_MsgMgr::LoadMsgs( ) - Read data, starting addy is 0x%08x, and ending is 0x%08x",l_pcData, l_pcEndData);
	while( l_psControl[ k ] != -32768 && (char*)&(l_psControl[ k ]) < l_pcEndData )
	{
LogFile( ERROR_LOG, "SH3_MsgMgr::LoadMsgs( ) - Currend ID is %d",l_psControl[ k ]);
		l_vIDs.push_back( l_psControl[ k ] );
		k++;
	}
LogFile( ERROR_LOG,"There are %ld IDs COUNT",l_vIDs.size( ) );
	l_pcCurData = (char *)&( l_psControl[ k ]);
	l_psControl = &( l_psControl[ k ] );
LogFile( ERROR_LOG, "	SH3_MsgMgr::LoadMsgs( ) - starting addy is 0x%08x, control is 0x%08x and ending is 0x%08x",l_pcData, l_psControl, l_pcEndData);
	while( l_pcCurData < l_pcEndData )
	{
		k = 0;
		if( l_psControl[ k ] == -32768 )
			k++;

		if( l_psControl[ k ] == -32768 )
		{
			LogFile( ERROR_LOG, "MIKE - Used 2nd");
		//	l_lCurIDIndex++;
			k++;
		}

		l_pcCurData = l_cMsg.GetMsg( (char*)&(l_psControl[ k ]), l_vIDs[ l_lCurIDIndex ] );

		if( (char *)&(l_psControl[ k ]) == l_pcCurData )
		{
			LogFile( ERROR_LOG, "SH3_MsgMsr::LoadMsgs( ) - ERROR: Starting and current addresses are the same...");
			break;
		}
		if( l_cMsg.m_sAltID != 4096 )
			l_lCurIDIndex++;

		l_psControl = (short *)l_pcCurData;

		l_vMsgs.push_back( l_cMsg );
	}
LogFile( ERROR_LOG,"There are %ld Messages COUNT",l_vMsgs.size( ) );
	delete [] l_pcData;
	fclose( inFile );
	
	m_lNumMessages = l_vMsgs.size( );
	m_pcMessages = new SH3_Msg[ m_lNumMessages ];

	for( k = 0; k < m_lNumMessages; k++ )
		m_pcMessages[ k ] = l_vMsgs[ k ];

	return m_lNumMessages;
}


long SH3_AllMsg::LoadAll( char *_baseSH3dir, long _lLangFlag )
{
	sh3_arcfile		l_cArcList;
	sh3_dropList	l_cMsgFiles;
	char l_pcFilename[ 256 ];
	long l_lRes;
	long k;

	LogFile( ERROR_LOG, "SH3_AllMsg::LoadAll( ) - Start");
	ReleaseAllData( );
	sprintf( l_pcFilename, "%sarc.arc",_baseSH3dir );

	if( (l_lRes = l_cArcList.Load( l_pcFilename )) < 1 )
	{
		LogFile( ERROR_LOG, "SH3_AllMsg::LoadAll( ) - ERROR: Couldn't load arc file '%s' - Returned val %ld", l_pcFilename, l_lRes );
		return 0;
	}

	switch( _lLangFlag )
	{
		case MSG_LANG_GERMAN:
				sprintf( l_pcFilename, "_grm_msg.mes" );
				break;
		case MSG_LANG_FRENCH:
				sprintf( l_pcFilename, "_frn_msg.mes" );
				break;
		case MSG_LANG_ITALIAN:
				sprintf( l_pcFilename, "_itl_msg.mes" );
				break;
		case MSG_LANG_CHINESE:
				sprintf( l_pcFilename, "_chn_msg.mes" );
				break;
		case MSG_LANG_JAPANESE:
				sprintf( l_pcFilename, "_jpn_msg.mes" );
				break;
		case MSG_LANG_SPANISH:
				sprintf( l_pcFilename, "_spn_msg.mes" );
				break;
		case MSG_LANG_KOREAN:
				sprintf( l_pcFilename, "_krn_msg.mes" );
				break;
		default:
				sprintf( l_pcFilename, "_eng_msg.mes" );
				break;
	}

	LogFile( ERROR_LOG, "SH3_AllMsg::LoadAll( ) - Language Flag sets file extension to [%s]", l_pcFilename );
	for( k = 0; k < l_cArcList.m_sArcData.arcFileCount; k++ )
	{
		LogFile( ERROR_LOG, "Check: Section %ld of %ld is '%s'",k+1,l_cArcList.m_sArcData.arcFileCount, l_cArcList.m_pcArcSections[ k ].sectionName );
		if( strcmp( l_cArcList.m_pcArcSections[ k ].sectionName, "msg" ) == 0 )
			break;
	}
LogFile( ERROR_LOG, "Check: The index is %ld of %ld", k+1, l_cArcList.m_sArcData.arcFileCount );
	if( !l_cMsgFiles.buildList( &(l_cArcList.m_pcArcSections[ k ]),l_pcFilename ))
	{
		LogFile( ERROR_LOG, "SH3_AllMsg::LoadAll( ) - ERROR: Couldn't get any messages from arc file 'msg.arc' - None ended with '%s'", l_pcFilename );
		return 0;
	}

	sprintf( l_pcFilename, "%smsg.arc", _baseSH3dir );
	LogFile( ERROR_LOG, "The arc filename is '%s'", l_pcFilename );
	m_lNumAll = l_cMsgFiles.m_lNumFilenames;
	m_pcAllMsg = new SH3_MsgMgr[ m_lNumAll ];

	for( k = 0; k < m_lNumAll; k++ )
	{
		LogFile( ERROR_LOG, "SH3_AllMsg::LoadAll( )\n-------------------\n Loading #%ld of %ld"
								" file '%s' at index %ld with size %ld", k+1, m_lNumAll, 
							l_cMsgFiles.m_pcFilename[ k ].fileName, l_cMsgFiles.m_pcFilename[ k ].fileIndex,	//.sectIndex,
							l_cMsgFiles.m_pcFilename[ k ].dataSize );
		if( ! m_pcAllMsg[ k ].LoadMsgs( l_pcFilename, l_cMsgFiles.m_pcFilename[ k ].fileIndex ) )
			LogFile( ERROR_LOG, "SH3_AllMsg::LoadAll( ) - ERROR: Couldn't load any msgs for #%ld of %ld"
								" file '%s' at index %ld with size %ld", k+1, m_lNumAll, 
							l_cMsgFiles.m_pcFilename[ k ].fileName, l_cMsgFiles.m_pcFilename[ k ].fileIndex,	//.sectIndex,
							l_cMsgFiles.m_pcFilename[ k ].dataSize );
	}

	return m_lNumAll;
}