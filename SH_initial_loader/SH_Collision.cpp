#include <windows.h>
#include <stdio.h>
#include <vector>
#include <errno.h>
#include <limits.h>
#include <string.h>

#include "SH3_Loader.h"
#include "SH_Collision.h"
#include "typedefs.h"

extern int errno;

static bool SH3_IsLikelyCldPrimHeader( const BYTE *pData, long lRemainingBytes, sh3_cld_vert_header *pOutHeader = NULL )
{
	sh3_cld_vert_header	l_sHeader;
	long				l_lVertBytes;

	if( !pData || lRemainingBytes < (long)sizeof( sh3_cld_vert_header ) )
		return false;

	memcpy( &l_sHeader, pData, sizeof( l_sHeader ) );

	if( l_sHeader.s_lVertType != 257 )
		return false;

	if( l_sHeader.s_lNumVerts <= 0 || l_sHeader.s_lNumVerts > 4096 )
		return false;

	if( l_sHeader.s_lNumVerts > (LONG_MAX / (long)sizeof( vertex4f )) )
		return false;

	l_lVertBytes = l_sHeader.s_lNumVerts * sizeof( vertex4f );

	if( lRemainingBytes < (long)sizeof( sh3_cld_vert_header ) + l_lVertBytes )
		return false;

	if( pOutHeader )
		*pOutHeader = l_sHeader;

	return true;
}


static long SH3_FindNextCldPrimOffset( const BYTE *pData, long lDataSize, long lStartOffset )
{
	long l_lOffset;

	if( !pData || lStartOffset < 0 || lStartOffset >= lDataSize )
		return -1;

	for( l_lOffset = lStartOffset; l_lOffset + (long)sizeof( sh3_cld_vert_header ) <= lDataSize; l_lOffset += 4 )
	{
		if( SH3_IsLikelyCldPrimHeader( pData + l_lOffset, lDataSize - l_lOffset ) )
			return l_lOffset;
	}

	return -1;
}


static void SH3_DumpUnknownBlock( const char *pLabel, long lSectionOffset, long lBlockOffset, const vector<BYTE> &vData )
{
	long	l_lWordCount;
	long	l_lIndex;

	if( !pLabel || vData.empty( ) )
		return;

	LogFile( ERROR_LOG, "%s - sectionRel=%ld blockRel=%ld size=%ld", pLabel, lSectionOffset, lBlockOffset, (long)vData.size( ) );

	l_lWordCount = (long)( vData.size( ) / sizeof( long ) );

	for( l_lIndex = 0; l_lIndex < l_lWordCount; l_lIndex++ )
	{
		utype4	l_uValue;
		char	l_caAscii[ 5 ];
		long	l;

		memcpy( &l_uValue, &(vData[ l_lIndex * sizeof( long ) ]), sizeof( l_uValue ) );

		for( l = 0; l < 4; l++ )
		{
			unsigned char c = l_uValue.uc_types[ l ];
			l_caAscii[ l ] = ( c >= 32 && c <= 126 ) ? (char)c : '.';
		}

		l_caAscii[ 4 ] = '\0';

		LogFile( ERROR_LOG, "\t[%4ld] rel=%4ld hex=%08lx float=%f long=%ld short=[%d %d] bytes=[%u %u %u %u] ascii=[%s]",
			l_lIndex,
			lBlockOffset + l_lIndex * (long)sizeof( long ),
			(unsigned long)l_uValue.ul_types[ 0 ],
			l_uValue.f_types[ 0 ],
			l_uValue.l_types[ 0 ],
			(int)l_uValue.s_types[ 0 ],
			(int)l_uValue.s_types[ 1 ],
			(unsigned int)l_uValue.uc_types[ 0 ],
			(unsigned int)l_uValue.uc_types[ 1 ],
			(unsigned int)l_uValue.uc_types[ 2 ],
			(unsigned int)l_uValue.uc_types[ 3 ],
			l_caAscii );
	}

	if( (long)vData.size( ) % sizeof( long ) )
	{
		long l_lTailOffset = l_lWordCount * sizeof( long );
		LogFile( ERROR_LOG, "\tTail bytes at rel=%ld count=%ld", lBlockOffset + l_lTailOffset, (long)vData.size( ) - l_lTailOffset );
	}
}


static bool SH3_ParseUnknownExtentBlock( const vector<BYTE> &vData, sh3_cld_unknown_extent_block *pOutBlock )
{
	sh3_cld_unknown_extent_block l_sBlock;

	if( vData.size( ) < sizeof( sh3_cld_unknown_extent_block ) )
		return false;

	memcpy( &l_sBlock, &(vData[ 0 ]), sizeof( l_sBlock ) );

	if( l_sBlock.s_lType != 1 || l_sBlock.s_lCount != 4 || l_sBlock.s_lZero != 0 )
		return false;

	if( l_sBlock.s_lStride != 256 && l_sBlock.s_lStride != 1 && l_sBlock.s_lStride != 128 && l_sBlock.s_lStride != 32 )
		return false;

	if( pOutBlock )
		*pOutBlock = l_sBlock;

	return true;
}

long SH3_CldIndex::LoadData( FILE *inFile, long _lDataSize )
{
	long	l_lRes;
	
	if( ! inFile )
	{
		LogFile( ERROR_LOG, "SH3_CldIndex::LoadData( ) - ERROR: No file pointer was passed in");
		return 0;
	}

	DeleteData( );

	if( _lDataSize <= (long)sizeof(long) )
	{
		LogFile( ERROR_LOG, "SH3_CldIndex::LoadData( ) - ERROR: Invalid index data size %ld", _lDataSize );
		return 0;
	}

	m_lNumIndex = _lDataSize / sizeof( long ) - 1;	//The -1 is to account for the last index, which is always invalid
	if( m_lNumIndex <= 0 || m_lNumIndex > 0x100000 )
	{
		LogFile( ERROR_LOG, "SH3_CldIndex::LoadData( ) - ERROR: Invalid index count %ld from size %ld", m_lNumIndex, _lDataSize );
		return 0;
	}

	m_plIndices = new long[ m_lNumIndex ];

	l_lRes = _loadBlock( (void *)m_plIndices, m_lNumIndex * sizeof( long ), inFile,
						"SH3_CldIndex::LoadData( ) - ERROR: Could not load index data", ERROR_LOG );

	if( l_lRes == -1 )
		return 0;

	return l_lRes;
}

	

long SH3_CldPrim::LoadData( FILE *inFile, long _lSectionRelativeOffset, long _lPrimSize, long _lUnknownSize )
{
	long	l_lRes;
	long	l_lTotalRead = 0;
	long	l_lVertDataBytes = 0;

	if( ! inFile )
	{
		LogFile( ERROR_LOG, "SH3_CldPrim::LoadData( ) - ERROR: No file pointer was passed in");
		return 0;
	}

	DeleteData( );
	m_lSectionRelativeOffset = _lSectionRelativeOffset;

	l_lRes = _loadBlock( (void *)&m_sVertHeader, sizeof( m_sVertHeader ), inFile,
						"SH3_CldPrim::LoadData( ) - ERROR: Could not load header data", ERROR_LOG );

	if( l_lRes == -1 )
		return 0;

	l_lTotalRead = l_lRes;
	m_lVertexDataOffset = m_lSectionRelativeOffset + l_lTotalRead;

	if( m_sVertHeader.s_lNumVerts <= 0 || m_sVertHeader.s_lNumVerts > 4096 )
	{
		LogFile( ERROR_LOG, "SH3_CldPrim::LoadData( ) - ERROR: Invalid vertex count %ld (type=%ld, q1=%ld)",
			m_sVertHeader.s_lNumVerts, m_sVertHeader.s_lVertType, m_sVertHeader.q1_sh3_cvh );
		return 0;
	}

	if( m_sVertHeader.s_lNumVerts > (LONG_MAX / (long)sizeof(vertex4f)) )
	{
		LogFile( ERROR_LOG, "SH3_CldPrim::LoadData( ) - ERROR: Vertex count overflow %ld", m_sVertHeader.s_lNumVerts );
		return 0;
	}

	l_lVertDataBytes = m_sVertHeader.s_lNumVerts * sizeof( vertex4f );

	m_pcVerts = new vertex4f[ m_sVertHeader.s_lNumVerts ];

	l_lRes = _loadBlock( (void *)m_pcVerts, l_lVertDataBytes, inFile,
						"SH3_CldPrim::LoadData( ) - ERROR: Could not load vertex data", ERROR_LOG );

	if( l_lRes == -1 )
		return 0;

	l_lTotalRead += l_lRes;
	m_lUnknownDataOffset = m_lSectionRelativeOffset + l_lTotalRead;

	if( _lPrimSize > 0 )
		m_lTotalSize = _lPrimSize;
	else
		m_lTotalSize = l_lTotalRead + ( ( _lUnknownSize > 0 ) ? _lUnknownSize : 0 );

	if( _lUnknownSize > 0 )
	{
		m_vUnknownData.resize( _lUnknownSize );
		l_lRes = _loadBlock( (void *)&(m_vUnknownData[ 0 ]), _lUnknownSize, inFile,
							"SH3_CldPrim::LoadData( ) - ERROR: Could not load unknown data", ERROR_LOG );

		if( l_lRes == -1 )
			return 0;

		l_lTotalRead += l_lRes;

		if( SH3_ParseUnknownExtentBlock( m_vUnknownData, &m_sExtentBlock ) )
		{
			m_bHasExtentBlock = true;
			LogFile( ERROR_LOG, "SH3_CldPrim::LoadData( ) - Parsed extent block at sectionRel=%ld unknownRel=%ld",
				m_lSectionRelativeOffset, m_lUnknownDataOffset );
		}
	}

	return l_lTotalRead;
}
	

long SH3_CldSet::LoadIndex( FILE *inFile, sh3_cld_index_offsets *_psOffsets )
{
	long			l_lOffset;
	long			l_lTotalRead = 0;
	long			l_lRes;
	long			k;
	long			l_lTempSize;
	SH3_CldIndex	l_cIndex;

	if( ! inFile || !_psOffsets)
	{
		LogFile( ERROR_LOG, "SH3_CldSet::LoadIndex( ) - ERROR: No file or offset pointer was passed in");
		return 0;
	}

	m_vIndexData.clear( );
	l_lOffset = ftell( inFile );

	
	LogFile( ERROR_LOG, "SH3_CldSet::LoadIndex( ) - CHECK: Starting size is %ld, and Index data is:", m_vIndexData.size( ));
	debugLongArray( _psOffsets->s_laIndexOffsets, 16 );

	for( k = 0; k < 15; k++ )
	{
		fseek( inFile, l_lOffset + _psOffsets->s_laIndexOffsets[ k ], SEEK_SET );
		l_lTempSize = _psOffsets->s_laIndexOffsets[ k + 1 ] - _psOffsets->s_laIndexOffsets[ k ];

		if( l_lTempSize > 4 )
		{
			if( (l_lRes = l_cIndex.LoadData( inFile, l_lTempSize )) )
			{
				m_vIndexData.push_back( l_cIndex );
				l_lTotalRead += l_lRes;
			}
			else
				LogFile( ERROR_LOG, "SH3_CldSet::LoadIndex( ) - ERROR: Didn't read indices for section w/ size %ld at offset %ld", l_lTempSize, l_lOffset + _psOffsets->s_laIndexOffsets[ k ] );
		}
	}

	if( m_vIndexData.size( ) == 15 )
		LogFile( ERROR_LOG, "SH3_CldSet::LoadIndex( ) - WARNING:: This section may have a 16th index set... It's time for more analysis");

	return l_lTotalRead;
}




long SH3_CldSet::LoadVerts( FILE *inFile, long _lOffset, long _lDataSize )
{
	long			l_lSectionStart;
	long			l_lTotalRead = 0;
	long			l_lRes;
	long			l_lPrimStart;
	long			l_lPrimMinSize;
	long			l_lNextPrimOffset;
	long			l_lUnknownSize;
	long			l_lRemainingBytes;
	sh3_cld_vert_header	l_sPrimHeader;
	SH3_CldPrim		l_cPrim;
	vector<BYTE>	l_vSectionData;

	if( ! inFile || !_lOffset || !_lDataSize )
	{
		LogFile( ERROR_LOG, "SH3_CldSet::LoadVerts( ) - ERROR: No file or offset or data size was passed in");
		return 0;
	}

	if( _lDataSize < 48 || _lDataSize > (64 * 1024 * 1024) )
	{
		LogFile( ERROR_LOG, "SH3_CldSet::LoadVerts( ) - ERROR: Invalid data size %ld", _lDataSize );
		return 0;
	}

	DeleteData( );
	m_lSectionRelativeOffset = _lOffset;
	m_lSectionSize = _lDataSize;
	l_lSectionStart = ftell( inFile );

	fseek( inFile, l_lSectionStart + _lOffset, SEEK_SET );

	l_vSectionData.resize( _lDataSize );
	l_lRes = _loadBlock( (void *)&(l_vSectionData[ 0 ]), _lDataSize, inFile,
						"SH3_CldSet::LoadVerts( ) - ERROR: Could not load section collision data", ERROR_LOG );

	if( l_lRes == -1 )
		return 0;

	l_lPrimStart = 0;

	while( l_lPrimStart + (long)sizeof( sh3_cld_vert_header ) <= _lDataSize )
	{
		l_lRemainingBytes = _lDataSize - l_lPrimStart;

		if( !SH3_IsLikelyCldPrimHeader( &(l_vSectionData[ l_lPrimStart ]), l_lRemainingBytes, &l_sPrimHeader ) )
			break;

		l_lPrimMinSize = sizeof( sh3_cld_vert_header ) + l_sPrimHeader.s_lNumVerts * sizeof( vertex4f );
		l_lNextPrimOffset = SH3_FindNextCldPrimOffset( &(l_vSectionData[ 0 ]), _lDataSize, l_lPrimStart + l_lPrimMinSize );
		l_lUnknownSize = ( l_lNextPrimOffset == -1 ) ? ( _lDataSize - ( l_lPrimStart + l_lPrimMinSize ) ) : ( l_lNextPrimOffset - ( l_lPrimStart + l_lPrimMinSize ) );

		fseek( inFile, l_lSectionStart + _lOffset + l_lPrimStart, SEEK_SET );

		if( !(l_lRes = l_cPrim.LoadData( inFile, l_lPrimStart, l_lPrimMinSize + l_lUnknownSize, l_lUnknownSize )))
		{
			LogFile( ERROR_LOG, "SH3_CldSet::LoadVerts( ) - ERROR: Couldn't read prim %ld at offset %ld",m_vPrimData.size( ), l_lSectionStart + _lOffset + l_lPrimStart );
			return l_lTotalRead;
		}

		LogFile( ERROR_LOG, "SH3_CldSet::LoadVerts( ) - Prim %ld sectionRel=%ld fileRel=%ld totalSize=%ld knownBytes=%ld unknownBytes=%ld type=%ld numVerts=%ld q1=%ld",
			(long)m_vPrimData.size( ),
			l_lPrimStart,
			_lOffset + l_lPrimStart,
			l_cPrim.m_lTotalSize,
			l_lPrimMinSize,
			(long)l_cPrim.m_vUnknownData.size( ),
			l_cPrim.m_sVertHeader.s_lVertType,
			l_cPrim.m_sVertHeader.s_lNumVerts,
			l_cPrim.m_sVertHeader.q1_sh3_cvh );

		if( !l_cPrim.m_vUnknownData.empty( ) )
			SH3_DumpUnknownBlock( "SH3_CldSet::LoadVerts( ) - Primitive unknown block", _lOffset, l_cPrim.m_lUnknownDataOffset, l_cPrim.m_vUnknownData );

		l_lTotalRead += l_lRes;
		m_vPrimData.push_back( l_cPrim );

		if( l_lNextPrimOffset == -1 )
		{
			l_lPrimStart = _lDataSize;
			break;
		}

		l_lPrimStart = l_lNextPrimOffset;
	}

	m_lParsedBytes = l_lPrimStart;

	if( l_lPrimStart < _lDataSize )
	{
		m_vTrailingData.resize( _lDataSize - l_lPrimStart );
		memcpy( &(m_vTrailingData[ 0 ]), &(l_vSectionData[ l_lPrimStart ]), m_vTrailingData.size( ) );
		SH3_DumpUnknownBlock( "SH3_CldSet::LoadVerts( ) - Section trailing block", _lOffset, l_lPrimStart, m_vTrailingData );
	}

	return l_lTotalRead;
}


long SH3_Collision::Load( char *filename, long _offset )
{
	FILE					*inFile;
	long					l_lCurOffset;
	sh3_cld_index_offsets	l_sIndexOffsets;
	sh3_cld_data_offsets	l_sDataOffsets;
	long					l_lRes;
	long					l_lTotalRead = 0;
	long					l_lNumValidSect = 0;
	long					k;
//	SH3_CldSet				l_cTempCldSet;

	if( !filename || !_offset )
	{
		LogFile( ERROR_LOG, "SH3_Collision::Load( ) - ERROR: No filename or offset was passed in");
		return 0;
	}

	DeleteData( );


	if( ( inFile = fopen( filename, "rb" )) == NULL )
	{
		LogFile( ERROR_LOG, "SH3_Collision::Load( ) - ERROR: Could not load filename '%s' - Reason: %s", filename, strerror( errno ) );
		return 0;
	}

	fseek( inFile, _offset, SEEK_SET );

	l_lRes = _loadBlock( (void *)&m_sHeader, sizeof( m_sHeader ), inFile,
						"SH3_Collision::Load( ) - ERROR: Could not load header", ERROR_LOG );

	if( l_lRes == -1 )
	{
		fclose( inFile );
		return 0;
	}
LogFile( ERROR_LOG, "SH3_Collision::Load( ) - Loading file '%s:%ld'", filename, _offset );
LogFile( ERROR_LOG, "SH3_Collision::Load( ) - CHECK: Header data is:");
debugFloat( m_sHeader.s_fXPos );
debugFloat( m_sHeader.s_fZPos );
debugLongArray( m_sHeader.s_lSectDataSize, 5 );
debugLong( m_sHeader.f_sh3_ch );

	l_lTotalRead += l_lRes;

	for( k = 0; k < 5; k++ )
	{
LogFile( ERROR_LOG, "SH3_Collision::Load( ) - Index Loop %ld of %ld at offset %ld",k+1, 5, ftell( inFile ) );
		l_lCurOffset = ftell( inFile );
		l_lRes = _loadBlock( (void *)&l_sIndexOffsets, sizeof( sh3_cld_index_offsets ), inFile,
						"SH3_Collision::Load( ) - ERROR: Could not load index offsets", ERROR_LOG );

		if( l_lRes == -1 )
		{
			LogFile( ERROR_LOG, "SH3_Collision::Load( ) - ERROR: Filename '%s:%ld', set %ld of %ld, at offset %ld",filename, _offset, k+1, 5, l_lCurOffset );
			fclose( inFile );
			return 0;
		}

		l_lTotalRead += l_lRes;
		l_lCurOffset = ftell( inFile );

		fseek( inFile, _offset, SEEK_SET );

		if( ( k < 4 && m_sHeader.s_lSectDataSize[ k ] > 80 ) || ( k ==4 && m_sHeader.s_lSectDataSize[ k ] > 48 ) )
		{
			l_lRes = m_caCldData[ k ].LoadIndex( inFile, &l_sIndexOffsets );

			if( ! l_lRes )
			{
				LogFile( ERROR_LOG, "SH3_Collision::Load( ) - ERROR: Filename '%s:%ld', set %ld of %ld, couldn't load index data",filename, _offset, k+1, 5 );
//				fclose( inFile );
//				return 0;
			}

			l_lTotalRead += l_lRes;
		}
		else
		{
			m_caCldData[ k ].m_vIndexData.clear( );
		}
		fseek( inFile, l_lCurOffset, SEEK_SET );
	}
LogFile( ERROR_LOG, "SH3_Collision::Load( ) - Done w/ Index Data");
	l_lRes = _loadBlock( (void *)&l_sDataOffsets, sizeof( l_sDataOffsets ), inFile,
						"SH3_Collision::Load( ) - ERROR: Could not load data offsets", ERROR_LOG );

	if( l_lRes == -1 )
	{
		fclose( inFile );
		return 0;
	}

LogFile( ERROR_LOG, "SH3_Collision::Load( ) - The Data offsets are:");
debugLongArray( l_sDataOffsets.s_lOffsetSectData, 5 );
	for( k = 0; k < 5; k++ )
	{
		fseek( inFile, _offset, SEEK_SET );

		if( ( k < 4 && m_sHeader.s_lSectDataSize[ k ] > 80 ) || ( k ==4 && m_sHeader.s_lSectDataSize[ k ] > 48 ) )
		{
			l_lRes = m_caCldData[ k ].LoadVerts( inFile, l_sDataOffsets.s_lOffsetSectData[ k ], m_sHeader.s_lSectDataSize[ k ] );

			if( ! l_lRes )
			{
				LogFile( ERROR_LOG, "SH3_Collision::Load( ) - ERROR: Filename '%s:%ld', set %ld of %ld, couldn't load vertex data",filename, _offset, k+1, 5 );
//				fclose( inFile );
//				return 0;
			}
			else
				l_lNumValidSect++;

			l_lTotalRead += l_lRes;
		}
		else
		{
			m_caCldData[ k ].m_vPrimData.clear( );
		}
		fseek( inFile, l_lCurOffset, SEEK_SET );
	}

	if( l_lNumValidSect == 0 )
	{
		LogFile( ERROR_LOG, "SH3_Collision::Load( ) - NOTICE: This collision file, '%s:%ld', is an 'empty' file.  No data is contained", filename, _offset );
		fclose( inFile );
	}

	fclose( inFile );
	return l_lTotalRead;
}


//void DebugCollision( SH3_Collision *h );
