#include <windows.h>
#include <stdio.h>
#include <vector>
#include <errno.h>

#include "SH3_Loader.h"
#include "SH_Collision.h"
#include "typedefs.h"

extern int errno;

long SH3_CldIndex::LoadData( FILE *inFile, long _lDataSize )
{
	long	l_lRes;
	
	if( ! inFile )
	{
		LogFile( ERROR_LOG, "SH3_CldIndex::LoadData( ) - ERROR: No file pointer was passed in");
		return 0;
	}

	DeleteData( );

	m_lNumIndex = _lDataSize / sizeof( long ) - 1;	//The -1 is to account for the last index, which is always invalid

	m_plIndices = new long[ m_lNumIndex ];

	l_lRes = _loadBlock( (void *)m_plIndices, m_lNumIndex * sizeof( long ), inFile,
						"SH3_CldIndex::LoadData( ) - ERROR: Could not load index data", ERROR_LOG );

	if( l_lRes == -1 )
		return 0;

	return l_lRes;
}

	

long SH3_CldPrim::LoadData( FILE *inFile )
{
	long	l_lRes;
	long	l_lTotalRead = 0;

	if( ! inFile )
	{
		LogFile( ERROR_LOG, "SH3_CldPrim::LoadData( ) - ERROR: No file pointer was passed in");
		return 0;
	}

	DeleteData( );

	l_lRes = _loadBlock( (void *)&m_sVertHeader, sizeof( m_sVertHeader ), inFile,
						"SH3_CldPrim::LoadData( ) - ERROR: Could not load header data", ERROR_LOG );

	if( l_lRes == -1 )
		return 0;

	l_lTotalRead = l_lRes;

	m_pcVerts = new vertex4f[ m_sVertHeader.s_lNumVerts ];

	l_lRes = _loadBlock( (void *)m_pcVerts, m_sVertHeader.s_lNumVerts * sizeof( vertex4f ), inFile,
						"SH3_CldPrim::LoadData( ) - ERROR: Could not load vertex data", ERROR_LOG );

	if( l_lRes == -1 )
		return 0;

	l_lTotalRead += l_lRes;

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
	long			l_lOffset;
	long			l_lTotalRead = 0;
	long			l_lRes;
	long			k;
	long			l_lTempSize;
	SH3_CldPrim		l_cPrim;

	if( ! inFile || !_lOffset || !_lDataSize )
	{
		LogFile( ERROR_LOG, "SH3_CldSet::LoadVerts( ) - ERROR: No file or offset or data size was passed in");
		return 0;
	}

	m_vPrimData.clear( );
	l_lOffset = ftell( inFile );

	fseek( inFile, l_lOffset + _lOffset, SEEK_SET );

	while( l_lTotalRead < _lDataSize - 80 )
	{
		l_lOffset = ftell( inFile );

		if( !(l_lRes = l_cPrim.LoadData( inFile )))
		{
			LogFile( ERROR_LOG, "SH3_CldSet::LoadVerts( ) - ERROR: Couldn't read prim %ld at offset %ld",m_vPrimData.size( ), l_lOffset );
			return l_lTotalRead;
		}
		
		l_lTotalRead += l_lRes;
		m_vPrimData.push_back( l_cPrim );
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
