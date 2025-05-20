#include <windows.h>
#include <gl\gl.h>
#include <gl\glu.h>
#include <gl\wglext.h>
#include <gl\glext.h>
#include <gl\glprocs.h>
#include <stdio.h>
#include <errno.h>
#include <vector>
#include <iterator>

#include "vertex.h"
#include "quat.h"
#include "matrix.h"
#include "typedefs.h"
#include "mathlib.h"
#include "Camera.h"
#include "tgaload.h"
#include "plane.h"
#include "PerPixelLighting.h"
#include "RenderObjs.h"
#include "Renderer.h"
#include "SH2_Loader.h"
#include "SH4_Loader.h"
#include "SH2_Model.h"
#include "debugDrawing.h"
#include "utils.h"

extern TexMgr textureMgr;
extern bool debugMode;
extern bool debugRender;
extern bool dumpModel;
extern bool useOpenGL;
extern sh2_tex_index sh2TexList;

extern int errno;

extern bool displayMatrix;


extern bool test_anim_transpose_seq0;
extern bool test_anim_transpose_seq8;
extern bool test_anim_swapXY_seq0;
extern bool test_anim_swapXY_seq8;
extern bool test_anim_seq0_use1st;
extern bool test_anim_seq8_use1st;
extern bool testMode;

#ifndef  __SH2_ANIM_TEST__
#define test_anim_transpose_s0( x )	( testMode && test_anim_transpose_seq0 && getSeq( (x) ) == 0 )
#define test_anim_transpose_s8( x )	( testMode && test_anim_transpose_seq8 && getSeq( (x) ) == 8 )
#define test_anim_swap_s0( x )	( testMode && test_anim_swapXY_seq0 && getSeq( (x) ) == 0 )
#define test_anim_swap_s8( x )	( testMode && test_anim_swapXY_seq8 && getSeq( (x) ) == 8 )
#define test_anim_use1st_s0( x )	( testMode && test_anim_seq0_use1st && getSeq( (x) ) == 0 )
#define test_anim_use1st_s8( x )	( testMode && test_anim_seq8_use1st && getSeq( (x) ) == 8 )
#endif



void debugM2_MPSH( sh2_model_prim_size_header *h )
{
	LogFile( ERROR_LOG, "   ---==[ Size Header (size %ld )]==---",sizeof(*h));

	debugLong( h->headerSize );
	debugLong( h->f1_vpsh );
}


void debugM2_VPSH( sh2_vertex_prim_seq_header *h )
{
	LogFile( ERROR_LOG, "   ---==[ Size Header (size %ld )]==---",sizeof(*h));

	debugLong( h->numSeq1Data );
	debugLong( h->offsetSeq1Data );
	debugLong( h->numSeq2Data );
	debugLong( h->offsetSeq2Data );
}

void debugM2_VPDH( sh2_vertex_prim_data_header *h )
{
	LogFile( ERROR_LOG, "   ---==[ Data Header (size %ld )]==---",sizeof(*h));

	debugLong( h->numTexID );
	debugLong( h->offsetTexIDs );
	debugLong( h->offsetEndMarker );
	debugLong( h->numVertex4fs );
	debugLong( h->q1_vpdh );
	debugLong( h->f1_vpdh );
	debugLong( h->f2_vpdh );
	debugVertex4f( h->q2_vpdh );
	debugVertex4f( h->q3_vpdh );
	debugVertex4f( h->q4_vpdh );
	debugVertex4f( h->q5_vpdh );
	debugLong( h->startIndex );
	debugLong( h->numIndex );
	debugLong( h->q8_vpdh );
}


void debugM2_VTB( sh2_vertex_tex_block *h )
{
	LogFile( ERROR_LOG, "   ---==[ Tex Header (size %ld )]==---",sizeof(*h));

	debugLong( h->texID );
	debugLong( h->f1_vtb );
	debugLong( h->f2_vtb );
	debugLong( h->f3_vtb );
}

void debugM2_VEM( sh2_vertex_end_marker *h )
{
	LogFile( ERROR_LOG, "   ---==[ End Marker (size %ld )]==---",sizeof(*h));

	debugLong( h->endMarker );
	debugLong( h->f1_vem );
	debugLong( h->f2_vem );
	debugLong( h->f3_vem );
}


void debugM2_VDS( sh2_vertex_data_short *h )
{
	LogFile( ERROR_LOG, "\n ]>==---==<[ V E R T E X   P R I M   S H O R T (size %ld) ]>==---==<[", sizeof(*h));
	debugM2_MPSH( &(h->m_sHeaderSize) );
	debugM2_VPDH( &(h->m_sDataHeader) );
	debugM2_VTB( &(h->m_sTexHeader) );
	debugM2_VEM( &(h->m_sEndHeader) );
}


void debugM2_VDL( sh2_vertex_data_long *h )
{
	LogFile( ERROR_LOG, "\n ]>==---==<[ V E R T E X   P R I M   L O N G (size %ld) ]>==---==<[", sizeof(*h));
	debugM2_MPSH( &(h->m_sHeaderSize) );
	debugM2_VPSH( &(h->m_sSeqHeader) );
	debugM2_VPDH( &(h->m_sDataHeader) );
	debugShortArray( h->seqData1, h->m_sSeqHeader.numSeq1Data );
	debugShortArray( h->seqData2, h->m_sSeqHeader.numSeq2Data );
	debugM2_VTB( &(h->m_sTexHeader) );
	debugM2_VEM( &(h->m_sEndHeader) );
}


void debugM2_VOF( sh2_vertex_offset_header *h )
{
	LogFile( ERROR_LOG, "\n     ---+---==<[ V E R T E X   O F F S E T   H E A D E R (size %ld) ]>==---+---", sizeof(*h));
 	debugLong( h->numSet1Verts );
	debugLong( h->offsetSet1Verts );
	debugLong( h->numSet2Verts );
	debugLong( h->offsetSet2Verts );
	debugLong( h->offsetSet1Index );
	debugLong( h->offsetSet2Index );
	debugLong( h->numQ1_voh );
	debugLong( h->numQ2_voh );
	debugLong( h->offsetQ3_voh );
	debugLong( h->f1_voh );
	debugLong( h->f2_voh );
	debugLong( h->f3_voh );
}



sh2_vertex_data_long & sh2_vertex_data_long::operator=( const sh2_vertex_data_long & rhs )
{
	if( &rhs != this )
	{
		long k;

		deleteData( );

		memcpy( (void *)&m_sHeaderSize, (void *)&rhs.m_sHeaderSize, sizeof( sh2_model_prim_size_header ) );
		memcpy( (void *)&m_sSeqHeader , (void *) &rhs.m_sSeqHeader , sizeof( sh2_vertex_prim_seq_header ) );
		memcpy( (void *)&m_sDataHeader , (void *) &rhs.m_sDataHeader , sizeof( sh2_vertex_prim_data_header ) );
		memcpy( (void *)&m_sTexHeader , (void *) &rhs.m_sTexHeader , sizeof( sh2_vertex_tex_block ) );
		memcpy( (void *)&m_sEndHeader , (void *) &rhs.m_sEndHeader , sizeof( sh2_vertex_end_marker ) );

		seqData1 = new short[ m_sSeqHeader.numSeq1Data ];
		seqData2 = new short[ m_sSeqHeader.numSeq2Data ];

		for( k = 0; k < m_sSeqHeader.numSeq1Data; k++ )
			seqData1[ k ] = rhs.seqData1[ k ];

		for( k = 0; k < m_sSeqHeader.numSeq2Data; k++ )
			seqData2[ k ] = rhs.seqData2[ k ];
	}
	return *this;
}


void sh2_vertex_data_long::setNullAll( )
{
	seqData1 = NULL;
	seqData2 = NULL;
}

void sh2_vertex_data_long::deleteData( )
{
	delete [] seqData1;
	delete [] seqData2;
	setNullAll( );
}


long sh2_vertex_data_long::loadDataHeader( FILE *inFile )
{
	long lNumRead;
	long lTotalread;
	long lStartOffset;

	if( !inFile )
		return 0;

	lStartOffset  = ftell( inFile );

	deleteData( );

	lNumRead = _loadBlock( (void*)&m_sHeaderSize, sizeof(sh2_model_prim_size_header) , inFile,
						"sh2_vertex_data_long::loadDataHeader() - Could not read vertex prim size head",ERROR_LOG);
	if( lNumRead == -1 )
		return 0;

	lTotalread = lNumRead;


	lNumRead = _loadBlock( (void*)&m_sSeqHeader, sizeof(sh2_vertex_prim_seq_header) , inFile,
						"sh2_vertex_data_long::loadDataHeader() - Could not read vertex prim seq head",ERROR_LOG);
	if( lNumRead == -1 )
		return 0;

	lTotalread += lNumRead;


	lNumRead = _loadBlock( (void*)&m_sDataHeader, sizeof(sh2_vertex_prim_data_header) , inFile,
						"sh2_vertex_data_long::loadDataHeader() - Could not read vertex prim data head",ERROR_LOG);
	if( lNumRead == -1 )
		return 0;

	lTotalread += lNumRead;

	
	//--[ CHECK ]--/

	fseek( inFile, lStartOffset + m_sDataHeader.offsetTexIDs, SEEK_SET );
	if( lStartOffset + m_sDataHeader.offsetTexIDs != ftell( inFile ) )
		LogFile( ERROR_LOG, "sh2_vertex_data_long::loadDataHeader( ) - WARNING: Should be at offset %ld, currently at offset %ld",
					lStartOffset + m_sDataHeader.offsetTexIDs, ftell( inFile ) );

	lNumRead = _loadBlock( (void*)&m_sTexHeader, sizeof(sh2_vertex_tex_block) , inFile,
						"sh2_vertex_data_long::loadDataHeader() - Could not read vertex prim data head",ERROR_LOG);
	if( lNumRead == -1 )
		return 0;

	lTotalread += lNumRead;

	seqData1 = new short[ m_sSeqHeader.numSeq1Data ];
	seqData2 = new short[ m_sSeqHeader.numSeq2Data ];

	fseek( inFile, lStartOffset + m_sSeqHeader.offsetSeq1Data, SEEK_SET );

	lNumRead = _loadBlock( (void*)seqData1, sizeof(short) * m_sSeqHeader.numSeq1Data , inFile,
						"sh2_vertex_data_long::loadDataHeader() - Could not read vertex seq 1 data",ERROR_LOG);
	if( lNumRead == -1 )
		return 0;

	lTotalread += lNumRead;


	fseek( inFile, lStartOffset + m_sSeqHeader.offsetSeq2Data, SEEK_SET );

	lNumRead = _loadBlock( (void*)seqData2, sizeof(short) * m_sSeqHeader.numSeq2Data , inFile,
						"sh2_vertex_data_long::loadDataHeader() - Could not read vertex seq 2 data",ERROR_LOG);
	if( lNumRead == -1 )
		return 0;

	lTotalread += lNumRead;


	fseek( inFile, lStartOffset + m_sDataHeader.offsetEndMarker, SEEK_SET );

	lNumRead = _loadBlock( (void*)&m_sEndHeader, sizeof(sh2_vertex_end_marker), inFile,
						"sh2_vertex_data_long::loadDataHeader() - Could not read vertex prim end header",ERROR_LOG);
	if( lNumRead == -1 )
		return 0;

	lTotalread += lNumRead;

	return lTotalread;
}




sh2_vertex_data_short & sh2_vertex_data_short::operator=( const sh2_vertex_data_short & rhs )
{
	if( &rhs != this )
	{
		deleteData( );

		memcpy( (void *)&m_sHeaderSize, (void *)&rhs.m_sHeaderSize, sizeof( sh2_model_prim_size_header ) );
		memcpy( (void *)&m_sDataHeader , (void *) &rhs.m_sDataHeader , sizeof( sh2_vertex_prim_data_header ) );
		memcpy( (void *)&m_sTexHeader , (void *) &rhs.m_sTexHeader , sizeof( sh2_vertex_tex_block ) );
		memcpy( (void *)&m_sEndHeader , (void *) &rhs.m_sEndHeader , sizeof( sh2_vertex_end_marker ) );
	}
	return *this;
}


void sh2_vertex_data_short::setNullAll( )
{

}


void sh2_vertex_data_short::deleteData( )
{
	setNullAll( );
}


long sh2_vertex_data_short::loadDataHeader( FILE *inFile )
{
	long lNumRead;
	long lTotalread;
	long lStartOffset;

	if( !inFile )
		return 0;

	lStartOffset  = ftell( inFile );

	deleteData( );

	lNumRead = _loadBlock( (void*)&m_sHeaderSize, sizeof(sh2_model_prim_size_header) , inFile,
						"sh2_vertex_data_short::loadDataHeader() - Could not read vertex prim size head",ERROR_LOG);
	if( lNumRead == -1 )
		return 0;

	lTotalread = lNumRead;


	lNumRead = _loadBlock( (void*)&m_sDataHeader, sizeof(sh2_vertex_prim_data_header) , inFile,
						"sh2_vertex_data_short::loadDataHeader() - Could not read vertex prim data head",ERROR_LOG);
	if( lNumRead == -1 )
		return 0;

	lTotalread += lNumRead;

	
	//--[ CHECK ]--/
	if( lStartOffset + m_sDataHeader.offsetTexIDs != ftell( inFile ) )
		LogFile( ERROR_LOG, "sh2_vertex_data_short::loadDataHeader( ) - WARNING: Should be at offset %ld, currently at offset %ld",
					lStartOffset + m_sDataHeader.offsetTexIDs, ftell( inFile ) );

	lNumRead = _loadBlock( (void*)&m_sTexHeader, sizeof(sh2_vertex_tex_block) , inFile,
						"sh2_vertex_data_short::loadDataHeader() - Could not read vertex prim data head",ERROR_LOG);
	if( lNumRead == -1 )
		return 0;

	lTotalread += lNumRead;


	fseek( inFile, lStartOffset + m_sDataHeader.offsetEndMarker, SEEK_SET );

	lNumRead = _loadBlock( (void*)&m_sEndHeader, sizeof(sh2_vertex_end_marker), inFile,
						"sh2_vertex_data_short::loadDataHeader() - Could not read vertex prim end header",ERROR_LOG);
	if( lNumRead == -1 )
		return 0;

	lTotalread += lNumRead;

	return lTotalread;
}



sh2_vertex_large_header & sh2_vertex_large_header::operator=( const sh2_vertex_large_header & rhs )
{
	if( &rhs != this )
	{
		long k;

		deleteData( );

		m_lNumHeaders = rhs.m_lNumHeaders;
		m_lNumVerts = rhs.m_lNumVerts;
		m_lNumIndex = rhs.m_lNumIndex;

		m_sVertexType = new sh2_vertex_data_long[ rhs.m_lNumHeaders ];
		verts = new sh2_model_vert_short[ rhs.m_lNumVerts ];
		indicies = new short[ rhs.m_lNumIndex ];
		
		for( k = 0; k < m_lNumHeaders; k ++ )
			m_sVertexType[ k ] = rhs.m_sVertexType[ k ];

		for( k = 0; k < m_lNumVerts; k ++ )
			verts[ k ] = rhs.verts[ k ];

		for( k = 0; k < m_lNumIndex; k ++ )
			indicies[ k ] = rhs.indicies[ k ];
	}
	return *this;
}


void sh2_vertex_large_header::setNullAll( )
{
	m_sVertexType = NULL;
	verts = NULL;
	indicies = NULL;
	m_lNumHeaders = 0;
	m_lNumVerts = 0;
	m_lNumIndex = 0;
}


void sh2_vertex_large_header::deleteData( )
{
//	LogFile( ERROR_LOG, "\tsh2_vertex_large_header - Delete 1");
	delete [] m_sVertexType;
//	LogFile( ERROR_LOG, "\tsh2_vertex_large_header - Delete 2");
	delete [] verts;
//	LogFile( ERROR_LOG, "\tsh2_vertex_large_header - Delete 3");
	delete [] indicies;
//	LogFile( ERROR_LOG, "\tsh2_vertex_large_header - Delete 4");
	
	setNullAll( );
//	LogFile( ERROR_LOG, "\tsh2_vertex_large_header - Delete 5");
}


long sh2_vertex_large_header::loadDataHeader( FILE *inFile, long i_lCount )
{
	long k;
	long lNumRead;
	long lTotalread = 0;
	long lStartOffset;
	long lTempOffset;

	if( !inFile )
		return 0;

	lTempOffset = lStartOffset  = ftell( inFile );

	deleteData( );

	this->m_sVertexType = new sh2_vertex_data_long[ i_lCount ];

	for( k = 0; k < i_lCount; k ++ )
	{
		if( ! (lNumRead = m_sVertexType[ k ].loadDataHeader( inFile ) ) )
		{
			LogFile( ERROR_LOG, "sh2_vertex_large_header::loadDataHeader( ) - ERROR: (Header %ld of %ld) Didn't read header at offset %ld: %s",
				k, i_lCount, lTempOffset, strerror( errno ) );
			return 0;
		}

		if( debugMode )debugM2_VDL( &( m_sVertexType[ k ] ) );

		lTotalread += lNumRead;
		lTempOffset += m_sVertexType[ k ].m_sHeaderSize.headerSize;

		if( lTempOffset != ftell( inFile ) )
		{
			LogFile( ERROR_LOG, "sh2_vertex_large_header::loadDataHeader( ) - CHECK: Should be at offset %ld, but actually at offset %ld",
				lTempOffset, ftell( inFile ) );
			fseek( inFile, lTempOffset, SEEK_SET );
		}
	}

	this->m_lNumHeaders = i_lCount;

	return lTotalread;
}


long sh2_vertex_large_header::loadDataVerts( FILE *inFile, long i_lCount )
{
	long lNumRead;

	if( ! inFile )
		return 0;

	this->verts = new sh2_model_vert_short[ i_lCount ];


	lNumRead = _loadBlock( (void*)verts, sizeof(sh2_model_vert_short) * i_lCount , inFile,
						"sh2_vertex_large_header::loadDataVerts() - Could not read vertex data",ERROR_LOG);
	if( lNumRead == -1 )
		return 0;

	this->m_lNumVerts = i_lCount;
	return lNumRead;
}


long sh2_vertex_large_header::loadDataIndex( FILE *inFile, long i_lCount )
{
	long lNumRead;

	if( ! inFile )
		return 0;

	this->indicies = new short[ i_lCount ];


	lNumRead = _loadBlock( (void*)indicies, sizeof(short) * i_lCount , inFile,
						"sh2_vertex_large_header::loadDataIndex() - Could not read index data",ERROR_LOG);
	if( lNumRead == -1 )
		return 0;

	this->m_lNumIndex = i_lCount;
	return lNumRead;
}



sh2_vertex_small_header & sh2_vertex_small_header::operator=( const sh2_vertex_small_header & rhs )
{
	if( &rhs != this )
	{
		long k;

		deleteData( );

		m_lNumHeaders = rhs.m_lNumHeaders;
		m_lNumVerts = rhs.m_lNumVerts;
		m_lNumIndex = rhs.m_lNumIndex;

		m_sVertexType = new sh2_vertex_data_short[ rhs.m_lNumHeaders ];
		verts = new sh2_model_vert_long[ rhs.m_lNumVerts ];
		indicies = new short[ rhs.m_lNumIndex ];
		
		for( k = 0; k < m_lNumHeaders; k ++ )
			m_sVertexType[ k ] = rhs.m_sVertexType[ k ];

		for( k = 0; k < m_lNumVerts; k ++ )
			verts[ k ] = rhs.verts[ k ];

		for( k = 0; k < m_lNumIndex; k ++ )
			indicies[ k ] = rhs.indicies[ k ];
	}
	return *this;
}


void sh2_vertex_small_header::setNullAll( )
{
	m_sVertexType = NULL;
	verts = NULL;
	indicies = NULL;
	m_lNumHeaders = 0;
	m_lNumVerts = 0;
	m_lNumIndex = 0;
}


void sh2_vertex_small_header::deleteData( )
{
	delete [] m_sVertexType;
	delete [] verts;
	delete [] indicies;
	
	setNullAll( );
}


long sh2_vertex_small_header::loadDataHeader( FILE *inFile, long i_lCount )
{
	long k;
	long lNumRead;
	long lTotalread = 0;
	long lStartOffset;
	long lTempOffset;

	if( !inFile )
		return 0;

	lTempOffset = lStartOffset  = ftell( inFile );

	deleteData( );

	this->m_sVertexType = new sh2_vertex_data_short[ i_lCount ];

	for( k = 0; k < i_lCount; k ++ )
	{
		if( ! (lNumRead = m_sVertexType[ k ].loadDataHeader( inFile ) ) )
		{
			LogFile( ERROR_LOG, "sh2_vertex_small_header::loadDataHeader( ) - ERROR: (Header %ld of %ld) Didn't read header at offset %ld: %s",
				k, i_lCount, lTempOffset, strerror( errno ) );
			return 0;
		}

		if( debugMode )debugM2_VDS( &( m_sVertexType[ k ] ) );

		lTotalread += lNumRead;
		lTempOffset += m_sVertexType[ k ].m_sHeaderSize.headerSize;

		if( lTempOffset != ftell( inFile ) )
		{
			LogFile( ERROR_LOG, "sh2_vertex_small_header::loadDataHeader( ) - CHECK: Should be at offset %ld, but actually at offset %ld",
				lTempOffset, ftell( inFile ) );
			fseek( inFile, lTempOffset, SEEK_SET );
		}
	}

	this->m_lNumHeaders = i_lCount;

	return lTotalread;
}



long sh2_vertex_small_header::loadDataVerts( FILE *inFile, long i_lCount )
{
	long lNumRead;

	if( ! inFile )
		return 0;

	this->verts = new sh2_model_vert_long[ i_lCount ];


	lNumRead = _loadBlock( (void*)verts, sizeof(sh2_model_vert_long) * i_lCount , inFile,
						"sh2_vertex_small_header::loadDataVerts() - Could not read vertex data",ERROR_LOG);
	if( lNumRead == -1 )
		return 0;

	this->m_lNumVerts = i_lCount;
	return lNumRead;
}


long sh2_vertex_small_header::loadDataIndex( FILE *inFile, long i_lCount )
{
	long lNumRead;

	if( ! inFile )
		return 0;

	this->indicies = new short[ i_lCount ];


	lNumRead = _loadBlock( (void*)indicies, sizeof(short) * i_lCount , inFile,
						"sh2_vertex_small_header::loadDataIndex() - Could not read index data",ERROR_LOG);
	if( lNumRead == -1 )
		return 0;

	this->m_lNumIndex = i_lCount;
	return lNumRead;
}


//-----------------------------------------------------------------------------------/
//-- S H 2   M O D E L   C L A S S   C O D E                                       --/
//--   Contains all code for the SH2 Model Class.  These methods manage previously --/
//--   defined classes within this file (and all previously defined classes are a  --/
//--   a part of this class.                                                       --/
//-----------------------------------------------------------------------------------/


sh2_model & sh2_model::operator=( const sh2_model & rhs )
{
	if( &rhs != this )
	{
		long k;

		deleteData( );

		m_bModelLoaded = rhs.m_bModelLoaded;
		m_plTexSetIDs = new long[ rhs.m_sModelHeader.numTexSetIDs ];
		m_pcTexSetMapping = new sh_tex_mapping[ rhs.m_sModelHeader.numTexIDMapping ];

		strcpy( m_pcModelFilename, rhs.m_pcModelFilename );		
		structMemcpy(m_sModelBaseHeader);
		structMemcpy(m_sModelHeader);
		structMemcpy(m_sVertexHeader);
		structArrayMemcpy( m_pcTexSetMapping, rhs.m_sModelHeader.numTexIDMapping );
		structArrayMemcpy( m_plTexSetIDs, rhs.m_sModelHeader.numTexSetIDs );

		m_cHeaderData.copyData( rhs.m_cHeaderData, &m_sModelHeader );
		m_cVertPrimSmall = rhs.m_cVertPrimSmall;
		m_cVertPrimLarge = rhs.m_cVertPrimLarge;
		m_vTexData = rhs.m_vTexData;

		m_pcTransMatSet2 = new matrix[ m_sModelHeader.numMatSet2 ];
		for( k = 0; k < m_sModelHeader.numMatSet2; k++ )
			m_pcTransMatSet2[ k ] = rhs.m_pcTransMatSet2[ k ];

		//---[ COPY ANIMATION DATA ]---/
		m_bHasAnim = rhs.m_bHasAnim;
		if( ! computeInverseMats( ) )
			LogFile( ERROR_LOG, "sh2_model::operator=() - ERROR: Couldn't compute inverse matrices");
		this->m_pcAnimDat = rhs.m_pcAnimDat;
		this->m_pcAnimMat = rhs.m_pcAnimMat;
		this->m_lNumAnimMat = rhs.m_lNumAnimMat;
		this->m_lStartFrame = rhs.m_lStartFrame;
		this->m_lFrameCount = rhs.m_lFrameCount;
		this->m_lCurFrame = rhs.m_lCurFrame;
	}
	return *this;
}


//void sh2_model::debugSH2_MD( );
//quat sh2_model::convertToQuat( short int xVal, short int yVal, short int zVal ){ }

void sh2_model::setNullAll( )
{
	m_plTexSetIDs = NULL;
	m_pcTexSetMapping = NULL;
	m_cHeaderData.setNullAll( );
	m_cVertPrimSmall.setNullAll( );
	m_cVertPrimLarge.setNullAll( );
	m_bHasAnim = false;
	m_pcAnimMat = NULL;
	m_pcTransMatSet2 = NULL;
	m_pcAnimDat = NULL;
	m_pcDispInverse = NULL;
	m_lNumAnimMat = 0;
	m_lStartFrame = 0;
	m_lFrameCount = -1;
	m_lCurFrame = 0;
	m_pcModelFilename[ 0 ] = 0;
	m_bModelLoaded = false;
	firstRender = true;
}


void sh2_model::deleteData( )
{
//	LogFile( ERROR_LOG, "sh2_model - Delete 1");
	if( ! isLoaded( ) )
		return;
	delete [] m_plTexSetIDs;
//	LogFile( ERROR_LOG, "sh2_model - Delete 2");
	delete [] m_pcTexSetMapping;
//	LogFile( ERROR_LOG, "sh2_model - Delete 3");
	m_cHeaderData.deleteData( );
//	LogFile( ERROR_LOG, "sh2_model - Delete 4");
	m_cVertPrimSmall.deleteData( );
//	LogFile( ERROR_LOG, "sh2_model - Delete 5");
	m_cVertPrimLarge.deleteData( );
//	LogFile( ERROR_LOG, "sh2_model - Delete 6");
//	delete [] m_pcAnimMat;
//	LogFile( ERROR_LOG, "sh2_model - Delete 7");
	delete [] m_pcTransMatSet2;
//	delete [] m_pcAnimDat;
//	LogFile( ERROR_LOG, "sh2_model - Delete 7.5");
	if( m_pcAnimDat )m_pcAnimDat->DetachModel( );
//	LogFile( ERROR_LOG, "sh2_model - Delete 8");
	delete [] m_pcDispInverse;
//	LogFile( ERROR_LOG, "sh2_model - Delete 9");

	setNullAll( );
//	LogFile( ERROR_LOG, "sh2_model - Delete 10");
}


long sh2_model::loadAnim( SH_Anim_Loader & i_pcAnimSet, char *i_pcFilename )
{
	long l_lRes;

	if( m_pcAnimDat )
		m_pcAnimDat->DetachModel( this );

	this->m_bHasAnim = false;
	m_pcAnimDat = & i_pcAnimSet;

	m_pcAnimDat->AttachModel_SH2( this );
	if( !( l_lRes = m_pcAnimDat->LoadAnimSH2( i_pcFilename ) ) )
	{
		if( debugMode )
			LogFile( ERROR_LOG, "sh2_model::loadAnim( ) - ERROR: Failed to load animations for model '%s'",m_pcModelFilename );
		return 0;
	}

	if( ! setCurFrameRange( ) )
	{
		if( debugMode )
			LogFile( ERROR_LOG, "sh2_model::loadAnim( ) - ERROR: Unable to set frame range for model" );
	}

	this->m_bHasAnim = true;

	return l_lRes;
}
	



bool sh2_model::setCurFrameRange( long i_lStartFrame, long i_lFrameCount )
{
	long l_lLastFrame;

	if( ! m_pcAnimDat )
	{
		m_lStartFrame = m_lFrameCount = 0;
		return false;
	}

	l_lLastFrame = m_pcAnimDat->GetNumAnimFrames( ) - 1;

	if( i_lStartFrame <= l_lLastFrame )
		m_lStartFrame = i_lStartFrame;
	else
		m_lStartFrame = l_lLastFrame;

	if( i_lFrameCount < 0 || i_lFrameCount > l_lLastFrame - m_lStartFrame + 1 )
		m_lFrameCount = l_lLastFrame - m_lStartFrame + 1;
	else
		m_lFrameCount = i_lFrameCount;

	m_lCurFrame = m_lStartFrame;

	return m_lFrameCount > 0 && getAnimData ( );
}


bool sh2_model::getAnimData( )
{
	if( ! this->m_pcAnimDat )
		return false;
	if( this->m_pcAnimDat->IsFrameValid( m_lCurFrame ) )
	{
		this->m_lNumAnimMat = m_pcAnimDat->m_pcAnimSet[ m_lCurFrame ].getNumMatrix( );
		this->m_pcAnimMat = m_pcAnimDat ->m_pcAnimSet[ m_lCurFrame ].getMatrixArray( );
		return true;
	}
static bool printData = true;
	if( printData )
	{
		long k;
		LogFile( ERROR_LOG, "################### CHEX ######################################");
		for( k = 0; k < m_lNumAnimMat; k++ )
		{
			LogFile(ERROR_LOG," %2d. Animation Matrix\n--------------------------",k);
			debugMatrix( m_pcAnimMat[ k ] );
		}
		LogFile( ERROR_LOG, "###############################################################");
		printData = false;
	}
	if( debugMode )
		LogFile( ERROR_LOG, "sh2_model::getAnimData( ) - ERROR: Current frame %ld is out of range [0...%ld]",
			m_lCurFrame, m_pcAnimDat->GetNumAnimFrames( ) - 1 );
	return false;
}


bool sh2_model::nextFrame( )
{
	bool l_bRes;

	if( m_lFrameCount < 1 )
		return false;
	
	++m_lCurFrame;

	if( m_lCurFrame >= m_lStartFrame + m_lFrameCount )
		m_lCurFrame = m_lStartFrame;

	if( m_lCurFrame + 1 < m_lStartFrame + m_lFrameCount )
		l_bRes = true;
	else
		l_bRes = false;

	if( !getAnimData( ) )
		return false;
	return l_bRes;
}


bool sh2_model::prevFrame( )
{
	bool l_bRes;

	if( m_lFrameCount < 1 )
		return false;
	
	--m_lCurFrame;

	if( m_lCurFrame < m_lStartFrame )
		m_lCurFrame = m_lStartFrame + m_lFrameCount - 1;

	if( m_lCurFrame == m_lStartFrame )
		l_bRes = false;
	else
		l_bRes = true;

	if( ! getAnimData( ) )
		return false;
	return l_bRes;
}


long sh2_model::computeInverseMats( )
{
	long k;

	if( !isLoaded( ) )
		return 0;

	if( m_pcDispInverse )
		delete [] m_pcDispInverse;

	m_pcDispInverse = new matrix[ m_sModelHeader.numMatSet1 ];

	for( k = 0; k < this->m_sModelHeader.numMatSet1; k++ )
	{
		m_pcDispInverse[ k ] = m_cHeaderData.matSet1[ k ];
		m_pcDispInverse[ k ].Inverse( );
		if( test_anim_seq0_use1st )
			m_pcDispInverse[k].clearRot( );
	}

	return k;
}



long sh2_model::loadData( char *filename )
{
	FILE *inFile = NULL;
	long lNumRead;
	long lTotalread = 0;
	long lBaseOffset;
	long k;

	if( ! filename )
		return 0;

	LogFile( ERROR_LOG, "sh2_model::loadData( ) - Starting load '%s'", filename );

	if( isLoaded( ) )
		this->deleteData( );

	if( ( inFile = fopen( filename, "rb" ) ) == 0 )
	{
		LogFile( ERROR_LOG, "sh2_model::loadData( ) - ERROR: Couldn't open file '%s' - %s",filename, strerror(errno));
		return 0;
	}


	//--[ Load Initial Header for Model ]--/

	lNumRead = _loadBlock( (void*)&m_sModelBaseHeader, sizeof(m_sModelBaseHeader) , inFile,
						"sh2_model::loadData() - ERROR: Could not read base model head",ERROR_LOG);
	if( lNumRead == -1 )
	{
		fclose( inFile );
		return 0;
	}

	lTotalread = lNumRead;


	//--[ Get base offset of model data, and load structures ]--/

	lBaseOffset = ftell( inFile );

	lNumRead = _loadBlock( (void*)&m_sModelHeader, sizeof(m_sModelHeader) , inFile,
						"sh2_model::loadData() - Could not read model head",ERROR_LOG);
	if( lNumRead == -1 )
	{
		fclose( inFile );
		return 0;
	}

	if( debugMode )debugSH4_MH( &m_sModelHeader );

	lTotalread += lNumRead;

	//--[ Don't Forget the Vertex Offsets ]--/

	lNumRead = _loadBlock( (void*)&m_sVertexHeader, sizeof(m_sVertexHeader) , inFile,
						"sh2_model::loadData() - Could not read vertex offset head",ERROR_LOG);
	if( lNumRead == -1 )
	{
		fclose( inFile );
		return 0;
	}

	lTotalread += lNumRead;

	if( debugMode ) debugM2_VOF( &m_sVertexHeader );

	//--[ Load The Texture Data ]--/

	fseek( inFile, lBaseOffset + m_sModelHeader.offsetTexSetIDs, SEEK_SET );

	m_plTexSetIDs = new long[ m_sModelHeader.numTexSetIDs ];
		
	lNumRead = _loadBlock( (void*)m_plTexSetIDs, sizeof(long) * m_sModelHeader.numTexSetIDs , inFile,
						"sh2_model::loadData() - Could not read tex set IDs",ERROR_LOG);
	if( lNumRead == -1 )
	{
		fclose( inFile );
		return 0;
	}

	lTotalread += lNumRead;


	m_pcTexSetMapping = new sh_tex_mapping[ m_sModelHeader.numTexIDMapping ];

	fseek( inFile, lBaseOffset + m_sModelHeader.offsetTexIDMapping, SEEK_SET );

	lNumRead = _loadBlock( (void*)m_pcTexSetMapping, sizeof(sh_tex_mapping) * m_sModelHeader.numTexIDMapping , inFile,
						"sh2_model::loadData() - Could not read tex ID mapping",ERROR_LOG);
	if( lNumRead == -1 )
	{
		fclose( inFile );
		return 0;
	}

	lTotalread += lNumRead;


	if( (lNumRead = m_cHeaderData.loadData( lBaseOffset, &m_sModelHeader, inFile )) == 0 )
	{
		LogFile( ERROR_LOG, "sh2_model::loadData( ) - Could not load model dynamic header");
		fclose( inFile );
		return 0;
	}

	if( debugMode )debugSH4_MD( &m_cHeaderData, &m_sModelHeader );

	lTotalread += lNumRead;

	m_pcTransMatSet2 = new matrix[ m_sModelHeader.numMatSet2 ];

	for( k = 0; k < m_sModelHeader.numMatSet2; k++ )
		m_pcTransMatSet2[ k ] = m_cHeaderData.matSet1[ m_cHeaderData.mSeq2[ k * 2 + 1 ] ] * m_cHeaderData.matSet2[ k ];

	//--[ Load large headers ]--/

	fseek( inFile, lBaseOffset + m_sModelHeader.offsetVertSects, SEEK_SET );

	if( (lNumRead = this->m_cVertPrimLarge.loadDataHeader( inFile, m_sModelHeader.numVertSects ) ) == 0 )
	{
		LogFile( ERROR_LOG, "sh2_model::loadData( ) - Could not load large vertex headers");
		fclose( inFile );
		return 0;
	}

	lTotalread += lNumRead;


	//--[ Load small vertex data ]--/

	fseek( inFile, lBaseOffset + m_sVertexHeader.offsetSet1Verts, SEEK_SET );

	if( (lNumRead = this->m_cVertPrimLarge.loadDataVerts( inFile, m_sVertexHeader.numSet1Verts ) ) == 0 )
	{
		LogFile( ERROR_LOG, "sh2_model::loadData( ) - Could not load large vertex headers' Verts");
		fclose( inFile );
		return 0;
	}

	lTotalread += lNumRead;


	//--[ Load large vertex header index data ]--/

	fseek( inFile, lBaseOffset + m_sVertexHeader.offsetSet1Index, SEEK_SET );

	if( (lNumRead = this->m_cVertPrimLarge.loadDataIndex( inFile, 
			m_cVertPrimLarge.m_sVertexType[ m_cVertPrimLarge.m_lNumHeaders - 1 ].m_sDataHeader.startIndex + 
			m_cVertPrimLarge.m_sVertexType[ m_cVertPrimLarge.m_lNumHeaders - 1 ].m_sDataHeader.numIndex ) ) == 0 )
	{
		LogFile( ERROR_LOG, "sh2_model::loadData( ) - Could not load large vertex headers' Indices");
		fclose( inFile );
		return 0;
	}

	lTotalread += lNumRead;


	//--[ Load small headers ]--/

	if( m_sModelHeader.numAltVertSects )
	{
		fseek( inFile, lBaseOffset + m_sModelHeader.offsetAltVertSects, SEEK_SET );

		if( (lNumRead = this->m_cVertPrimSmall.loadDataHeader( inFile, m_sModelHeader.numAltVertSects ) ) == 0 )
		{
			LogFile( ERROR_LOG, "sh2_model::loadData( ) - Could not load small vertex headers");
			fclose( inFile );
			return 0;
		}

		lTotalread += lNumRead;

		//--[ Load large vertex data ]--/

		fseek( inFile, lBaseOffset + m_sVertexHeader.offsetSet2Verts, SEEK_SET );

		if( (lNumRead = this->m_cVertPrimSmall.loadDataVerts( inFile, m_sVertexHeader.numSet2Verts ) ) == 0 )
		{
			LogFile( ERROR_LOG, "sh2_model::loadData( ) - Could not load small vertex headers' Verts");
			fclose( inFile );
			return 0;
		}

		lTotalread += lNumRead;


		//--[ Load Small vertex header index data ]--/

		fseek( inFile, lBaseOffset + m_sVertexHeader.offsetSet2Index, SEEK_SET );

		if( (lNumRead = this->m_cVertPrimSmall.loadDataIndex( inFile, 
				m_cVertPrimSmall.m_sVertexType[ m_cVertPrimSmall.m_lNumHeaders - 1 ].m_sDataHeader.startIndex + 
				m_cVertPrimSmall.m_sVertexType[ m_cVertPrimSmall.m_lNumHeaders - 1 ].m_sDataHeader.numIndex ) ) == 0 )
		{
			LogFile( ERROR_LOG, "sh2_model::loadData( ) - Could not load small vertex headers' Indices");
			fclose( inFile );
			return 0;
		}

		lTotalread += lNumRead;
	}

	m_bModelLoaded = true;

	if( ! this->computeInverseMats( ) )
		LogFile( ERROR_LOG, "sh2_model::loadData( ) - ERROR: Couldn't compute inverse matricies...");

	fclose( inFile );

	LogFile( ERROR_LOG, "sh2_model::loadData( ) - End load SH2 Model '%s'",filename);
	
	strcpy( m_pcModelFilename, filename );

	return lNumRead;
}
	

//long sh2_model::loadTexData( FILE *inFile );
//GLuint sh2_model::loadTex( char *texName, FILE *inFile );
void sh2_model::Render( )
{
	long k, j, i;
	long last_i = 0;
	vertex4f *dispVerts = NULL;
	vertex4f *dispVerts2 = NULL;
	sh2_model_vert_short *largePrimVerts = NULL;
	sh2_model_vert_long *smallPrimVerts = NULL;
	vertex ones(1.0f,1.0f,1.0f);
	matrix *matArray = NULL;
	matrix *l_pcAnimMat = NULL;
	static bool printData = firstRender;
	bool useQuat = true;
	static bool clearRot = true;
	unsigned int *indicies = NULL;
	unsigned int *indicies2 = NULL;
	vertex4f tv1,tv2,tv3,tv4;
	GLuint texID;
	long l_lNumDispMat;
	matrix temp[4];//, temp2, temp3, temp4;
//	rgba tempMatSet;
	vertex4f tempVert;
	vertex4f tempWeights;

//	char outFile[128];
	char tempTexStr[ 128 ];

	printData = firstRender;

	/*if( printData )
		debugRender  = true;
	else
		debugRender = false;*/

	if( !isLoaded( ) )
		return;

	if( this->m_sModelHeader.numAltVertSects )
	{
		if( this->m_sVertexHeader.numSet2Verts )
			dispVerts2 = new vertex4f[ this->m_sVertexHeader.numSet2Verts ];
		else
			dispVerts2 = NULL;

		if( m_cVertPrimSmall.m_lNumIndex )
		{
			indicies2 = new unsigned int[ m_cVertPrimSmall.m_lNumIndex ];

			for( k = 0; k < m_cVertPrimSmall.m_lNumIndex; k++ )
				indicies2[k] = m_cVertPrimSmall.indicies[ k ];
		}
		else
			indicies2 = NULL;
	}


	if( m_bHasAnim )
	{
		if( printData )LogFile( ERROR_LOG, "Starting animation mat setup");
		l_lNumDispMat = this->m_lNumAnimMat;
		if( l_lNumDispMat > m_sModelHeader.numMatSet1 )
			l_lNumDispMat = m_sModelHeader.numMatSet1;
		if( debugRender && printData )LogFile( ERROR_LOG, "It has %ld matricies... and %ld anim mat",l_lNumDispMat, m_lNumAnimMat );
		l_pcAnimMat = new matrix[ l_lNumDispMat ];

		for( k = 0; k < l_lNumDispMat; k++ )
			l_pcAnimMat[ k ].identity( );

//		for( k = 0; k < m_lNumAnimMat && k < l_lNumDispMat; k++ )
//			l_pcAnimMat[ k ] = m_pcAnimMat[ k ];

		matrix *m_pcDispFinal = new matrix[ l_lNumDispMat ];
		matrix *l_pcForwardMat = new matrix[ l_lNumDispMat ];

		//----[ MOVE JOINT POSITION BACK TO ORIGIN AND MAKE IT'S RELATIVE OFFSET ]----/
		for( k = 0; k < l_lNumDispMat; k++ )	//m_pcAnimSet[ m_lCurFrame ].m_lNumMatrix; k++ )
		{	
			//---[ MAKE JOINT POSITION RELATIVE OFFSET TO ORIGIN ]---/
			if( m_cHeaderData.mSeq1[ k ] != 255 && m_cHeaderData.mSeq1[ k ] != -1 )
				l_pcForwardMat[ k ] = m_pcDispInverse[ m_cHeaderData.mSeq1[ k ] ] * m_cHeaderData.matSet1[ k ];
			else
				l_pcForwardMat[ k ].identity( );

//			if( test_anim_seq0_use1st && k < m_lNumAnimMat ) //Only clear rotations from matricies that we have animation mats for...
			if( clearRot )
				l_pcForwardMat[ k ].clearRot( );
			if (GetAsyncKeyState('C') & 0x8000)
				clearRot = !clearRot;
			if( debugRender && printData )
			{
				LogFile(ERROR_LOG," %2d. Joint Position Matrix\n--------------------------",k);
				debugMatrix( l_pcForwardMat[ k ] );
			}
		}
	if( debugRender && printData )
	{
		LogFile( ERROR_LOG, "###############################################################");		for( k = 0; k < l_lNumDispMat; k++ ){	LogFile(ERROR_LOG," %2d. Animation Matrix\n--------------------------",k);	debugMatrix( m_pcAnimMat[ k ] );	}	LogFile( ERROR_LOG, "###############################################################");
	}
useQuat = false;
		if( useQuat )
		{
			QuatAnim *m_pcDispQuat = new QuatAnim[ l_lNumDispMat ];
			QuatAnim *m_pcAnimSet = this->m_pcAnimDat->m_pcAnimSet[ this->m_lCurFrame ].m_pcQuatSet;
			QuatAnim *l_pcFinalQuat = new QuatAnim[ l_lNumDispMat ];
			for( k = 0; k < l_lNumDispMat; k++ )	//m_pcAnimSet[ m_lCurFrame ].m_lNumMatrix; k++ )
			{
				m_pcDispQuat[ k ].m_cAnimPos = m_pcAnimSet[k].m_cAnimPos + vertex( l_pcForwardMat[ k ][ 3 ], l_pcForwardMat[ k ][ 7 ], l_pcForwardMat[ k ][ 11 ] );
				m_pcDispQuat[ k ].m_cAnimAngle = m_pcAnimSet[k].m_cAnimAngle;
			}
			//----[ REBUILD THE MODEL ]----/
			for( k = 0; k < l_lNumDispMat; k++ )	//m_pcAnimSet[ m_lCurFrame ].m_lNumMatrix; k++ )
			{
				if( m_cHeaderData.mSeq1[ k ] != 255 && m_cHeaderData.mSeq1[ k ] != -1 )
					//l_pcFinalQuat[ k ] = m_pcDispQuat[ k ] * l_pcFinalQuat[ m_pcModel_SH3->mSeq1[ k ] ];
					l_pcFinalQuat[ k ] = l_pcFinalQuat[ m_cHeaderData.mSeq1[ k ] ] * m_pcDispQuat[ k ];
				else
					l_pcFinalQuat[ k ] = m_pcDispQuat[ k ];
			}

			for( k = 0; k < l_lNumDispMat; k++ )
				l_pcAnimMat[ k ] = l_pcFinalQuat[ k ].getMatrix( );
			delete [] m_pcDispQuat;
			delete [] l_pcFinalQuat;
		}
		else
		{
			//----[ TWO WAYS TO DO IT: ROTATE ORIGIN AND TRANSLATE BY PARENT, or ROTATE RELATIVE OFFSET AND TRANSLATE PARENT ]--
			for( k = 0; k < l_lNumDispMat; k++ )
			{
				if( k < m_lNumAnimMat )
				{
					//if( m_cHeaderData.mSeq1[ k ] == -1 || m_cHeaderData.mSeq1[ k ] == 255 )
					//	m_pcDispFinal[ k ] = m_pcAnimMat[ k ];
					//	//else if( test_anim_seq0_use1st )
					//	//	m_pcDispFinal[ k ] = m_pcAnimMat[ k ] * l_pcForwardMat[ k ];
					//else
						m_pcDispFinal[ k ] = l_pcForwardMat[ k ] * m_pcAnimMat[ k ];
					
				}
				else
					m_pcDispFinal[ k ] = l_pcForwardMat[ k ];

				if( debugRender && printData )
				{
					LogFile(ERROR_LOG," %2d. Anim & Joint Matrix\n--------------------------",k);
					debugMatrix( m_pcDispFinal[ k ] );
				}
			}
			//----[ REBUILD THE MODEL ]----/
			for( k = 0; k < l_lNumDispMat; k++ )
			{
				if( m_cHeaderData.mSeq1[ k ] != 255 && m_cHeaderData.mSeq1[ k ] != -1 )
					l_pcAnimMat[ k ] = l_pcAnimMat[ m_cHeaderData.mSeq1[ k ] ] * m_pcDispFinal[ k ];
				else
					l_pcAnimMat[ k ] = m_pcDispFinal[ k ];

				if( debugRender && printData )
				{
					LogFile(ERROR_LOG," %2d. Final Animation Matrix\n---------------------------",k);
					debugMatrix( l_pcAnimMat[ k ] );
				}
			}
		}

		if( debugRender && printData )LogFile( ERROR_LOG, "Finished data computations...");

		delete [] m_pcDispFinal;
		delete [] l_pcForwardMat;

		if( debugRender && printData )LogFile( ERROR_LOG, "Deleted extra maticies");
	}
	else
	{
		l_pcAnimMat = new matrix[ m_sModelHeader.numMatSet1 ];
		for( k = 0; k < m_sModelHeader.numMatSet1; k++ )
			l_pcAnimMat[ k ] = m_cHeaderData.matSet1[ k ];
	}

//####################################################################################################
	if( displayMatrix )
	{
		vertex b1,b2;

		for( k = 0; k < l_lNumDispMat; k++ )
		{
			matrix tMat;
			matrix *mPtr = 
					l_pcAnimMat; 
					//matSet2;
		//	b1 = vertex( mPtr[mSeq2[k*2]][ 3 ], mPtr[mSeq2[k*2]][ 7 ], mPtr[mSeq2[k*2]][ 11 ]);
		//	b2 = vertex( mPtr[mSeq2[k*2+1]][ 3 ], mPtr[mSeq2[k*2+1]][ 7 ], mPtr[mSeq2[k*2+1]][ 11 ]);
			if( this->m_cHeaderData.mSeq1[k] != -1 && this->m_cHeaderData.mSeq1[k] != 255 )
			{
				b1 = vertex( mPtr[ this->m_cHeaderData.mSeq1[ k ]][ 3 ], mPtr[ this->m_cHeaderData.mSeq1[ k ]][ 7 ], 
					mPtr[ this->m_cHeaderData.mSeq1[ k ]][ 11 ] );
				b2 = vertex( mPtr[ k ][ 3 ], mPtr[ k ][ 7 ], mPtr[ k ][ 11 ] );
				glLineWidth( 2.0f );
				glColor3f( 1.0f,1.0f,1.0f);
				glBegin( GL_LINES );
					glVertex3fv( &b1.x );
					glVertex3fv( &b2.x );
				glEnd( );
			}
			drawBasisMatrix( &( mPtr[ k ] ), 9.0f,1, k);
		}
	}
//######################################################################################################
	//---[ RENDER SEQUENCED PRIMITIVES (LARGE HEADERS, SMALL VERTS ) ]---/

	for( j = 0; j < m_sModelHeader.numVertSects; j++ )
	{
//		if(printData)LogFile(ERROR_LOG,"Generating Large Prim %ld of %ld\n\tNum Mat = %ld",j+1,this->m_sModelHeader.numVertSects,m_cVertPrimLarge.m_sVertexType[ j ].m_sSeqHeader.numSeq1Data + m_cVertPrimLarge.m_sVertexType[ j ].m_sSeqHeader.numSeq2Data);

		largePrimVerts = new sh2_model_vert_short[ m_cVertPrimLarge.m_sVertexType[ j ].m_sDataHeader.numIndex ];
		dispVerts = new vertex4f[ m_cVertPrimLarge.m_sVertexType[ j ].m_sDataHeader.numIndex ];
		indicies = new unsigned int[ m_cVertPrimLarge.m_sVertexType[ j ].m_sDataHeader.numIndex ];

		for( k = 0; k < m_cVertPrimLarge.m_sVertexType[ j ].m_sDataHeader.numIndex; k++ )
			indicies[k] = k;

		matArray = new matrix[ m_cVertPrimLarge.m_sVertexType[ j ].m_sSeqHeader.numSeq1Data + m_cVertPrimLarge.m_sVertexType[ j ].m_sSeqHeader.numSeq2Data];

		for( k = 0; k < m_cVertPrimLarge.m_sVertexType[ j ].m_sSeqHeader.numSeq1Data; k ++ )
		{
			matArray[k] = l_pcAnimMat[ m_cVertPrimLarge.m_sVertexType[ j ].seqData1[k]]; 
//			matArray[k] = m_cHeaderData.matSet1[ m_cVertPrimLarge.m_sVertexType[ j ].seqData1[k]];
		}

		for( k = m_cVertPrimLarge.m_sVertexType[ j ].m_sSeqHeader.numSeq1Data; k < m_cVertPrimLarge.m_sVertexType[ j ].m_sSeqHeader.numSeq1Data + m_cVertPrimLarge.m_sVertexType[ j ].m_sSeqHeader.numSeq2Data; k++ )
		{
	//		matArray[k] = m_pcTransMatSet2[ m_cVertPrimLarge.m_sVertexType[ j ].seqData2[k-m_cVertPrimLarge.m_sVertexType[ j ].m_sSeqHeader.numSeq1Data] ];
			matArray[k] = l_pcAnimMat[ m_cHeaderData.mSeq2[ m_cVertPrimLarge.m_sVertexType[ j ].seqData2[k-m_cVertPrimLarge.m_sVertexType[ j ].m_sSeqHeader.numSeq1Data] * 2 + 1 ] ]
						* m_cHeaderData.matSet2[m_cVertPrimLarge.m_sVertexType[ j ].seqData2[k-m_cVertPrimLarge.m_sVertexType[ j ].m_sSeqHeader.numSeq1Data]];
//			matArray[k].transpose();
		}


		for( k = 0; k < m_cVertPrimLarge.m_sVertexType[ j ].m_sDataHeader.numIndex; k ++ )
			largePrimVerts[ k ] = m_cVertPrimLarge.verts[ m_cVertPrimLarge.indicies[ m_cVertPrimLarge.m_sVertexType[ j ].m_sDataHeader.startIndex + k ] ];

		for( k = 0; k < m_cVertPrimLarge.m_sVertexType[ j ].m_sDataHeader.numIndex; k++ )
		{
				
			if( m_cVertPrimLarge.m_sVertexType[ j ].m_sSeqHeader.numSeq1Data + m_cVertPrimLarge.m_sVertexType[ j ].m_sSeqHeader.numSeq2Data > 0 )
			{
				//if( m_cVertPrimLarge.m_sVertexType[ j ].m_cVertexData[h].verts == NULL && m_cVertPrimLarge.m_sVertexType[ j ].altVerts != NULL )
				//{
				//	LogFile(TEST_LOG,"SH3_Actor::draw - TERMINAL ERROR: Can't render prim #%d - Small size verts have multi-sequence matricies w/o selector data",j);
				//	break;
				//}
				matrix temp, temp2, temp3, temp4;

				if( largePrimVerts[ k ].mats.r != 255 )
					temp = matArray[ largePrimVerts[ k ].mats.r ];
				else
					temp = matArray[ 0 ];

				if( largePrimVerts[ k ].mats.g != 255 )
					temp2 = matArray[ largePrimVerts[ k ].mats.g ];
				else
					temp2 = matArray[ 0 ];

				if( largePrimVerts[ k ].mats.b != 255 )
					temp3 = matArray[ largePrimVerts[ k ].mats.b ];
				else
					temp3 = matArray[ 0 ];

				if( largePrimVerts[ k ].mats.a != 255 )
					temp4 = matArray[ largePrimVerts[ k ].mats.a ];
				else
					temp4 = matArray[ 0 ];

				if( debugRender && printData && j == 0)LogFile( ERROR_LOG,"matricies: %ld  %ld  %ld  %ld",
						largePrimVerts[ k ].mats.r,largePrimVerts[ k ].mats.g,largePrimVerts[ k ].mats.b,largePrimVerts[ k ].mats.a);
						
				if(printData)
				{
					if( largePrimVerts[ k ].mats.r != 255 && largePrimVerts[ k ].mats.r > m_cVertPrimLarge.m_sVertexType[ j ].m_sSeqHeader.numSeq1Data + m_cVertPrimLarge.m_sVertexType[ j ].m_sSeqHeader.numSeq2Data
						|| largePrimVerts[ k ].mats.g && largePrimVerts[ k ].mats.g > m_cVertPrimLarge.m_sVertexType[ j ].m_sSeqHeader.numSeq1Data + m_cVertPrimLarge.m_sVertexType[ j ].m_sSeqHeader.numSeq2Data
						|| largePrimVerts[ k ].mats.b && largePrimVerts[ k ].mats.b > m_cVertPrimLarge.m_sVertexType[ j ].m_sSeqHeader.numSeq1Data + m_cVertPrimLarge.m_sVertexType[ j ].m_sSeqHeader.numSeq2Data
						|| largePrimVerts[ k ].mats.a && largePrimVerts[ k ].mats.a > m_cVertPrimLarge.m_sVertexType[ j ].m_sSeqHeader.numSeq1Data + m_cVertPrimLarge.m_sVertexType[ j ].m_sSeqHeader.numSeq2Data)
						LogFile(ERROR_LOG,"TERMINAL ERROR::: The matrix is out of range [%d %d %d %d]",
							largePrimVerts[ k ].mats.r,largePrimVerts[ k ].mats.g,largePrimVerts[ k ].mats.b,largePrimVerts[ k ].mats.a);
				}
	
				dispVerts[k]=	(temp * largePrimVerts[ k ].vert) * largePrimVerts[ k ].weights.x +
								(temp2 * largePrimVerts[ k ].vert) * largePrimVerts[ k ].weights.y +
								(temp3 * largePrimVerts[ k ].vert) * largePrimVerts[ k ].weights.z +
								(temp4 * largePrimVerts[ k ].vert) * largePrimVerts[ k ].weights.w;

				dispVerts[k].w = 1.0f;
			}
		}

			//if( m_cVertPrimLarge.m_sVertexType[ j ].texModify == 4 )
			//{
	//			glAlphaFunc(GL_GEQUAL, 0.01f);
	//			glEnable(GL_ALPHA_TEST);
			//}
		glFrontFace(GL_CCW);
		checkGLerror(__LINE__,__FILE__,"Before Render");
		glActiveTextureARB(GL_TEXTURE0_ARB);
		checkGLerror(__LINE__,__FILE__,"Before 1");
			
		sprintf( tempTexStr, "%ld", this->m_cHeaderData.texIDs[ m_cHeaderData.texIDMapping[ m_cVertPrimLarge.m_sVertexType[ j ].m_sTexHeader.texID * 2 ] ]);
		texID = sh2TexList.GetTex( string(tempTexStr), true );
		if( texID < 1 )
			texID = 1;
		if( m_cVertPrimLarge.m_sVertexType[ j ].m_sDataHeader.q1_vpdh == 1 )
			glDisable( GL_ALPHA_TEST );
		else
			glEnable( GL_ALPHA_TEST );
		glBindTexture(GL_TEXTURE_2D, texID );
		checkGLerror(__LINE__,__FILE__,"Before 2");
		glEnable(GL_TEXTURE_2D);

		glEnableClientState(GL_VERTEX_ARRAY);
		checkGLerror(__LINE__,__FILE__,"Before 3");
		glVertexPointer(4, GL_FLOAT, sizeof(vertex4f) , &dispVerts[0]);

		glClientActiveTextureARB(GL_TEXTURE0_ARB);
		checkGLerror(__LINE__,__FILE__,"Before 4");
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);
		checkGLerror(__LINE__,__FILE__,"Before 5");

		//if( m_cVertPrimLarge.m_sVertexType[ j ].m_cVertexData[h].m_sVertexType.vertexSize == sizeof(sh4_model_vert) )
		//{
			glTexCoordPointer(2, GL_FLOAT, sizeof( largePrimVerts[ 0 ] ), &(largePrimVerts[ 0 ].tc) );	// m_cVertPrimLarge.m_sVertexType[ j ].m_cVertexData[h].m_sVertexType.vertexSize, &m_cVertPrimLarge.m_sVertexType[ j ].m_cVertexData[h].verts[0].tc);
		//}
		//else if( m_cVertPrimLarge.m_sVertexType[ j ].m_cVertexData[h].m_sVertexType.vertexSize == 32 )
		//	glTexCoordPointer(2, GL_FLOAT, m_cVertPrimLarge.m_sVertexType[ j ].m_cVertexData[h].m_sVertexType.vertexSize, &m_cVertPrimLarge.m_sVertexType[ j ].altVerts[0].tc);
		checkGLerror(__LINE__,__FILE__,"Before 6");

		//if( j== 49 )
		glDrawElements(GL_TRIANGLE_STRIP, m_cVertPrimLarge.m_sVertexType[ j ].m_sDataHeader.numIndex, GL_UNSIGNED_INT, indicies ); //m_cVertPrimLarge.m_sVertexType[ j ].m_cVertexData[h].indicies);
		checkGLerror(__LINE__,__FILE__,"Before 7");

		glDisableClientState(GL_VERTEX_ARRAY);
		checkGLerror(__LINE__,__FILE__,"Before 8");
		glDisableClientState(GL_TEXTURE_COORD_ARRAY);

		//if( m_cVertPrimLarge.m_sVertexType[ j ].texModify == 4 )
		glDisable(GL_ALPHA_TEST);
		glFlush( );

		delete [] dispVerts;
		delete [] largePrimVerts;
		delete [] indicies;

//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
		for( i = last_i; i < m_sModelHeader.numAltVertSects; i++ )
		{
/*			long start_i, i_step, end_i;
			if( j == 10 )
			{
				start_i = 0; i_step = 2; end_i = 5;
			}
			else if( j == 11 )
			{
				start_i = 1; i_step = 3; end_i = 5;
			}
*/			
//			for( i = start_i; i < end_i; i+= i_step )
			if( m_cVertPrimSmall.m_sVertexType[ i ].m_sDataHeader.q8_vpdh == j )// && j != 2)
			{
//				long z, m;
		//		if( printData )
		//			LogFile( ERROR_LOG, "CHECK: j = %ld\ti = %ld\tlast_i = %ld\tnumIndex = %ld", j, i, last_i,m_cVertPrimSmall.m_sVertexType[i].m_sDataHeader.numIndex );
		//		if( printData ) LogFile( DATA_LOG, "\nsmall vert - set %ld\n-------------------------------",j);
			//	dispVerts2 = new vertex4f[ m_cVertPrimSmall.m_sVertexType[ i ].m_sDataHeader.numIndex ];

				for( k = 0; k < m_cVertPrimSmall.m_sVertexType[ i ].m_sDataHeader.numIndex; k++ )
				{
					long m;			
					long indexNum = m_cVertPrimSmall.indicies[ m_cVertPrimSmall.m_sVertexType[ i ].m_sDataHeader.startIndex + k ];
					tempVert = m_cVertPrimSmall.verts[ indexNum ].vert;
					tempWeights = m_cVertPrimSmall.verts[ indexNum ].weights;

					if( debugRender && printData )LogFile( DATA_LOG, "\tMat index: %ld %ld %ld %ld\tWeights: %f\t%f\t%f\t%f",
						m_cVertPrimSmall.verts[ indexNum ].mats[0],
						m_cVertPrimSmall.verts[ indexNum ].mats[1],
						m_cVertPrimSmall.verts[ indexNum ].mats[2],
						m_cVertPrimSmall.verts[ indexNum ].mats[3],tempWeights.x,tempWeights.y,tempWeights.z,tempWeights.w);

					for( m = 0; m < 4; m++ )
					{
						if( m_cVertPrimSmall.verts[ indexNum ].mats[ m ] != -1 )
						{
							long l_lMatIndex = m_cVertPrimSmall.verts[ indexNum ].mats[ m ];
							if( m == 0 )
								temp[ m ] = l_pcAnimMat[ l_lMatIndex ];//m_cHeaderData.matSet1[ m_cVertPrimSmall.verts[ indexNum ].mats[ m ] ]; //m_pcTransMatSet2[ m_cVertPrimSmall.verts[ indexNum ].mats[ m ] ];//
							else
								temp[ m ] = l_pcAnimMat[ m_cHeaderData.mSeq2[ l_lMatIndex * 2 + 1 ] ] * m_cHeaderData.matSet2[ l_lMatIndex ];
						}
						else
							temp[ m ].identity( );
							
					}
				

					dispVerts2[ indexNum ]=	(temp[ 0 ] * tempVert) * tempWeights.x +
											(temp[ 1 ] * tempVert) * tempWeights.y +
											(temp[ 2 ] * tempVert) * tempWeights.z +
											(temp[ 3 ] * tempVert) * tempWeights.w;

					dispVerts2[ indexNum ].w = 1.0f;
				}

				glActiveTextureARB(GL_TEXTURE0_ARB);
			
				sprintf( tempTexStr, "%ld", this->m_cHeaderData.texIDs[ m_cHeaderData.texIDMapping[ m_cVertPrimSmall.m_sVertexType[ i ].m_sTexHeader.texID * 2 ] ]);
				texID = sh2TexList.GetTex( string(tempTexStr), true );
				if( texID < 1 )
					texID = 1;
				glBindTexture(GL_TEXTURE_2D, texID );
				checkGLerror(__LINE__,__FILE__,"Before 2");
				glEnable(GL_TEXTURE_2D);

				glEnableClientState(GL_VERTEX_ARRAY);
				checkGLerror(__LINE__,__FILE__,"Before 3");
				glVertexPointer(4, GL_FLOAT, sizeof(vertex4f) , &dispVerts2[0]);

				glClientActiveTextureARB(GL_TEXTURE0_ARB);
				checkGLerror(__LINE__,__FILE__,"Before 4-2");
				glEnableClientState(GL_TEXTURE_COORD_ARRAY);
				checkGLerror(__LINE__,__FILE__,"Before 5-2");

				glTexCoordPointer(2, GL_FLOAT, sizeof( sh2_model_vert_long ), &(m_cVertPrimSmall.verts[ 0 ].tc) );	// m_cVertPrimSmall.m_sVertexType[ j ].m_cVertexData[h].m_sVertexType.vertexSize, &m_cVertPrimSmall.m_sVertexType[ j ].m_cVertexData[h].verts[0].tc);
				//	glTexCoordPointer(2, GL_FLOAT, m_cVertPrimSmall.m_sVertexType[ j ].m_cVertexData[h].m_sVertexType.vertexSize, &m_cVertPrimSmall.m_sVertexType[ j ].altVerts[0].tc);
				checkGLerror(__LINE__,__FILE__,"Before 6-2");
	
				glDrawElements(GL_TRIANGLE_STRIP, m_cVertPrimSmall.m_sVertexType[ i ].m_sDataHeader.numIndex, GL_UNSIGNED_INT, &(indicies2[m_cVertPrimSmall.m_sVertexType[ i ].m_sDataHeader.startIndex]) ); //m_cVertPrimSmall.m_sVertexType[ j ].m_cVertexData[h].indicies);
				checkGLerror(__LINE__,__FILE__,"Before 7-2");

				glDisableClientState(GL_VERTEX_ARRAY);
				checkGLerror(__LINE__,__FILE__,"Before 8-2");
				glDisableClientState(GL_TEXTURE_COORD_ARRAY);

				//if( m_cVertPrimSmall.m_sVertexType[ j ].texModify == 4 )
				glDisable(GL_ALPHA_TEST);

				glFlush( );

				
			}
		}

//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

		delete [] matArray;
	}
delete [] dispVerts2;
delete [] indicies2;
	delete [] l_pcAnimMat;
	printData = false;
	firstRender = false;



}


//################################################################################################################

//long sh2_model::loadTexData( FILE *inFile );
//GLuint sh2_model::loadTex( char *texName, FILE *inFile );
void sh2_model::RenderAlt( )
{
	long k, j, i;
	long last_i = 0;
	vertex4f *dispVerts = NULL;
	vertex4f *dispVerts2 = NULL;
	sh2_model_vert_short *largePrimVerts = NULL;
	sh2_model_vert_long *smallPrimVerts = NULL;
	vertex ones(1.0f,1.0f,1.0f);
	matrix *matArray = NULL;
	matrix *l_pcAnimMat = NULL;
	static bool printData = firstRender;
	bool useQuat = true;
	unsigned int *indicies = NULL;
	unsigned int *indicies2 = NULL;
	vertex4f tv1,tv2,tv3,tv4;
	GLuint texID;
	long l_lNumDispMat;
	matrix temp[4];//, temp2, temp3, temp4;
//	rgba tempMatSet;
	vertex4f tempVert;
	vertex4f tempWeights;

//	char outFile[128];
	char tempTexStr[ 128 ];

	printData = firstRender;

	if( !isLoaded( ) )
		return;

	if( this->m_sModelHeader.numAltVertSects )
	{
		if( this->m_sVertexHeader.numSet2Verts )
			dispVerts2 = new vertex4f[ this->m_sVertexHeader.numSet2Verts ];
		else
			dispVerts2 = NULL;

		if( m_cVertPrimSmall.m_lNumIndex )
		{
			indicies2 = new unsigned int[ m_cVertPrimSmall.m_lNumIndex ];

			for( k = 0; k < m_cVertPrimSmall.m_lNumIndex; k++ )
				indicies2[k] = m_cVertPrimSmall.indicies[ k ];
		}
		else
			indicies2 = NULL;
	}


	if( m_bHasAnim )
	{
		if( printData )LogFile( ERROR_LOG, "Starting animation mat setup");
		l_lNumDispMat = this->m_lNumAnimMat;
		if( l_lNumDispMat > m_sModelHeader.numMatSet1 )
			l_lNumDispMat = m_sModelHeader.numMatSet1;
		if( printData )LogFile( ERROR_LOG, "It has %ld matricies... and %ld anim mat",l_lNumDispMat, m_lNumAnimMat );
		l_pcAnimMat = new matrix[ l_lNumDispMat ];

		for( k = 0; k < l_lNumDispMat; k++ )
			l_pcAnimMat[ k ].identity( );

//		for( k = 0; k < m_lNumAnimMat && k < l_lNumDispMat; k++ )
//			l_pcAnimMat[ k ] = m_pcAnimMat[ k ];

		matrix *m_pcDispFinal = new matrix[ l_lNumDispMat ];
		matrix *l_pcForwardMat = new matrix[ l_lNumDispMat ];

		//----[ MOVE JOINT POSITION BACK TO ORIGIN AND MAKE IT'S RELATIVE OFFSET ]----/
		for( k = 0; k < l_lNumDispMat; k++ )	//m_pcAnimSet[ m_lCurFrame ].m_lNumMatrix; k++ )
		{	
			//---[ MAKE JOINT POSITION RELATIVE OFFSET TO ORIGIN ]---/
			if( m_cHeaderData.mSeq1[ k ] != 255 && m_cHeaderData.mSeq1[ k ] != -1 )
				l_pcForwardMat[ k ] = m_pcDispInverse[ m_cHeaderData.mSeq1[ k ] ] * m_cHeaderData.matSet1[ k ];
			else
				l_pcForwardMat[ k ].identity( ); // = m_cHeaderData.matSet1[ k ];

			if( !displayMatrix && k < m_lNumAnimMat ) //Only clear rotations from matricies that we have animation mats for...
				l_pcForwardMat[ k ].clearRot( );
			if( printData )
			{
				LogFile(ERROR_LOG," %2d. Joint Position Matrix\n--------------------------",k);
				debugMatrix( l_pcForwardMat[ k ] );
			}
		}
	if( printData )
	{
		LogFile( ERROR_LOG, "###############################################################");
		for( k = 0; k < l_lNumDispMat; k++ )
		{
			LogFile(ERROR_LOG," %2d. Animation Matrix\n--------------------------",k);
			debugMatrix( m_pcAnimMat[ k ] );
		}
		LogFile( ERROR_LOG, "###############################################################");
	}

	//----[ TWO WAYS TO DO IT: ROTATE ORIGIN AND TRANSLATE BY PARENT, or ROTATE RELATIVE OFFSET AND TRANSLATE PARENT ]--
		for( k = 0; k < l_lNumDispMat; k++ )
		{
			if( k < m_lNumAnimMat )
				m_pcDispFinal[ k ] = l_pcForwardMat[ k ] * m_pcAnimMat[ k ];
			else
				m_pcDispFinal[ k ] = l_pcForwardMat[ k ];

			if( printData )
			{
				LogFile(ERROR_LOG," %2d. Anim & Joint Matrix\n--------------------------",k);
				debugMatrix( m_pcDispFinal[ k ] );
			}
		}

		//----[ REBUILD THE MODEL ]----/
		for( k = 0; k < l_lNumDispMat; k++ )
		{
			if( m_cHeaderData.mSeq1[ k ] != 255 && m_cHeaderData.mSeq1[ k ] != -1 )
				l_pcAnimMat[ k ] = l_pcAnimMat[ m_cHeaderData.mSeq1[ k ] ] * m_pcDispFinal[ k ];
			else
				l_pcAnimMat[ k ] = m_pcDispFinal[ k ];

			if( printData )
			{
				LogFile(ERROR_LOG," %2d. Final Animation Matrix\n---------------------------",k);
				debugMatrix( l_pcAnimMat[ k ] );
			}
		}


		if( printData )LogFile( ERROR_LOG, "Finished data computations...");

		delete [] m_pcDispFinal;
		delete [] l_pcForwardMat;

		if( printData )LogFile( ERROR_LOG, "Deleted extra maticies");
	}
	else
	{
		l_pcAnimMat = new matrix[ m_sModelHeader.numMatSet1 ];
		for( k = 0; k < m_sModelHeader.numMatSet1; k++ )
			l_pcAnimMat[ k ] = m_cHeaderData.matSet1[ k ];
	}

//####################################################################################################
	if( displayMatrix )
	{
		vertex b1,b2;

		for( k = 0; k < l_lNumDispMat; k++ )
		{
			matrix tMat;
			matrix *mPtr = 
					l_pcAnimMat; 
					//matSet2;
		//	b1 = vertex( mPtr[mSeq2[k*2]][ 3 ], mPtr[mSeq2[k*2]][ 7 ], mPtr[mSeq2[k*2]][ 11 ]);
		//	b2 = vertex( mPtr[mSeq2[k*2+1]][ 3 ], mPtr[mSeq2[k*2+1]][ 7 ], mPtr[mSeq2[k*2+1]][ 11 ]);
			if( this->m_cHeaderData.mSeq1[k] != -1 && this->m_cHeaderData.mSeq1[k] != 255 )
			{
				b1 = vertex( mPtr[ this->m_cHeaderData.mSeq1[ k ]][ 3 ], mPtr[ this->m_cHeaderData.mSeq1[ k ]][ 7 ], 
					mPtr[ this->m_cHeaderData.mSeq1[ k ]][ 11 ] );
				b2 = vertex( mPtr[ k ][ 3 ], mPtr[ k ][ 7 ], mPtr[ k ][ 11 ] );
				glLineWidth( 2.0f );
				glColor3f( 1.0f,1.0f,1.0f);
				glBegin( GL_LINES );
					glVertex3fv( &b1.x );
					glVertex3fv( &b2.x );
				glEnd( );
			}
			drawBasisMatrix( &( mPtr[ k ] ), 9.0f,1, k);
		}
	}
//######################################################################################################
	//---[ RENDER SEQUENCED PRIMITIVES (LARGE HEADERS, SMALL VERTS ) ]---/

	for( j = 0; j < m_sModelHeader.numVertSects; j++ )
	{
		if(debugRender && printData)LogFile(ERROR_LOG,"Generating Large Prim %ld of %ld\n\tNum Mat = %ld",j+1,this->m_sModelHeader.numVertSects,m_cVertPrimLarge.m_sVertexType[ j ].m_sSeqHeader.numSeq1Data + m_cVertPrimLarge.m_sVertexType[ j ].m_sSeqHeader.numSeq2Data);

		largePrimVerts = new sh2_model_vert_short[ m_cVertPrimLarge.m_sVertexType[ j ].m_sDataHeader.numIndex ];
		dispVerts = new vertex4f[ m_cVertPrimLarge.m_sVertexType[ j ].m_sDataHeader.numIndex ];
		indicies = new unsigned int[ m_cVertPrimLarge.m_sVertexType[ j ].m_sDataHeader.numIndex ];

		for( k = 0; k < m_cVertPrimLarge.m_sVertexType[ j ].m_sDataHeader.numIndex; k++ )
			indicies[k] = k;

		matArray = new matrix[ m_cVertPrimLarge.m_sVertexType[ j ].m_sSeqHeader.numSeq1Data + m_cVertPrimLarge.m_sVertexType[ j ].m_sSeqHeader.numSeq2Data];

		for( k = 0; k < m_cVertPrimLarge.m_sVertexType[ j ].m_sSeqHeader.numSeq1Data; k ++ )
		{
			matArray[k] = l_pcAnimMat[ m_cVertPrimLarge.m_sVertexType[ j ].seqData1[k]]; 
//			matArray[k] = m_cHeaderData.matSet1[ m_cVertPrimLarge.m_sVertexType[ j ].seqData1[k]];
		}

		for( k = m_cVertPrimLarge.m_sVertexType[ j ].m_sSeqHeader.numSeq1Data; k < m_cVertPrimLarge.m_sVertexType[ j ].m_sSeqHeader.numSeq1Data + m_cVertPrimLarge.m_sVertexType[ j ].m_sSeqHeader.numSeq2Data; k++ )
		{
	//		matArray[k] = m_pcTransMatSet2[ m_cVertPrimLarge.m_sVertexType[ j ].seqData2[k-m_cVertPrimLarge.m_sVertexType[ j ].m_sSeqHeader.numSeq1Data] ];
			matArray[k] = l_pcAnimMat[ m_cHeaderData.mSeq2[ m_cVertPrimLarge.m_sVertexType[ j ].seqData2[k-m_cVertPrimLarge.m_sVertexType[ j ].m_sSeqHeader.numSeq1Data] * 2 + 1 ] ]
						* m_cHeaderData.matSet2[m_cVertPrimLarge.m_sVertexType[ j ].seqData2[k-m_cVertPrimLarge.m_sVertexType[ j ].m_sSeqHeader.numSeq1Data]];
//			matArray[k].transpose();
		}


		for( k = 0; k < m_cVertPrimLarge.m_sVertexType[ j ].m_sDataHeader.numIndex; k ++ )
			largePrimVerts[ k ] = m_cVertPrimLarge.verts[ m_cVertPrimLarge.indicies[ m_cVertPrimLarge.m_sVertexType[ j ].m_sDataHeader.startIndex + k ] ];

		for( k = 0; k < m_cVertPrimLarge.m_sVertexType[ j ].m_sDataHeader.numIndex; k++ )
		{
				
			if( m_cVertPrimLarge.m_sVertexType[ j ].m_sSeqHeader.numSeq1Data + m_cVertPrimLarge.m_sVertexType[ j ].m_sSeqHeader.numSeq2Data > 0 )
			{
				//if( m_cVertPrimLarge.m_sVertexType[ j ].m_cVertexData[h].verts == NULL && m_cVertPrimLarge.m_sVertexType[ j ].altVerts != NULL )
				//{
				//	LogFile(TEST_LOG,"SH3_Actor::draw - TERMINAL ERROR: Can't render prim #%d - Small size verts have multi-sequence matricies w/o selector data",j);
				//	break;
				//}
				matrix temp, temp2, temp3, temp4;

				if( largePrimVerts[ k ].mats.r != 255 )
					temp = matArray[ largePrimVerts[ k ].mats.r ];
				else
					temp = matArray[ 0 ];

				if( largePrimVerts[ k ].mats.g != 255 )
					temp2 = matArray[ largePrimVerts[ k ].mats.g ];
				else
					temp2 = matArray[ 0 ];

				if( largePrimVerts[ k ].mats.b != 255 )
					temp3 = matArray[ largePrimVerts[ k ].mats.b ];
				else
					temp3 = matArray[ 0 ];

				if( largePrimVerts[ k ].mats.a != 255 )
					temp4 = matArray[ largePrimVerts[ k ].mats.a ];
				else
					temp4 = matArray[ 0 ];

				if( printData && j == 0)LogFile( ERROR_LOG,"matricies: %ld  %ld  %ld  %ld",
						largePrimVerts[ k ].mats.r,largePrimVerts[ k ].mats.g,largePrimVerts[ k ].mats.b,largePrimVerts[ k ].mats.a);
						
				if(printData)
				{
					if( largePrimVerts[ k ].mats.r != 255 && largePrimVerts[ k ].mats.r > m_cVertPrimLarge.m_sVertexType[ j ].m_sSeqHeader.numSeq1Data + m_cVertPrimLarge.m_sVertexType[ j ].m_sSeqHeader.numSeq2Data
						|| largePrimVerts[ k ].mats.g && largePrimVerts[ k ].mats.g > m_cVertPrimLarge.m_sVertexType[ j ].m_sSeqHeader.numSeq1Data + m_cVertPrimLarge.m_sVertexType[ j ].m_sSeqHeader.numSeq2Data
						|| largePrimVerts[ k ].mats.b && largePrimVerts[ k ].mats.b > m_cVertPrimLarge.m_sVertexType[ j ].m_sSeqHeader.numSeq1Data + m_cVertPrimLarge.m_sVertexType[ j ].m_sSeqHeader.numSeq2Data
						|| largePrimVerts[ k ].mats.a && largePrimVerts[ k ].mats.a > m_cVertPrimLarge.m_sVertexType[ j ].m_sSeqHeader.numSeq1Data + m_cVertPrimLarge.m_sVertexType[ j ].m_sSeqHeader.numSeq2Data)
						LogFile(ERROR_LOG,"TERMINAL ERROR::: The matrix is out of range [%d %d %d %d]",
							largePrimVerts[ k ].mats.r,largePrimVerts[ k ].mats.g,largePrimVerts[ k ].mats.b,largePrimVerts[ k ].mats.a);
				}
	
				dispVerts[k]=	(temp * largePrimVerts[ k ].vert) * largePrimVerts[ k ].weights.x +
								(temp2 * largePrimVerts[ k ].vert) * largePrimVerts[ k ].weights.y +
								(temp3 * largePrimVerts[ k ].vert) * largePrimVerts[ k ].weights.z +
								(temp4 * largePrimVerts[ k ].vert) * largePrimVerts[ k ].weights.w;

				dispVerts[k].w = 1.0f;
			}
		}

		if( printData )LogFile(ERROR_LOG,"Point 1");
			//if( m_cVertPrimLarge.m_sVertexType[ j ].texModify == 4 )
			//{
	//			glAlphaFunc(GL_GEQUAL, 0.01f);
	//			glEnable(GL_ALPHA_TEST);
			//}
		glFrontFace(GL_CCW);
		checkGLerror(__LINE__,__FILE__,"Before Render");
		glActiveTextureARB(GL_TEXTURE0_ARB);
		checkGLerror(__LINE__,__FILE__,"Before 1");
			
		sprintf( tempTexStr, "%ld", this->m_cHeaderData.texIDs[ m_cHeaderData.texIDMapping[ m_cVertPrimLarge.m_sVertexType[ j ].m_sTexHeader.texID * 2 ] ]);
		texID = sh2TexList.GetTex( string(tempTexStr), true );
		if( texID < 1 )
			texID = 1;
		if( m_cVertPrimLarge.m_sVertexType[ j ].m_sDataHeader.q1_vpdh == 1 )
			glDisable( GL_ALPHA_TEST );
		else
			glEnable( GL_ALPHA_TEST );
		glBindTexture(GL_TEXTURE_2D, texID );
		checkGLerror(__LINE__,__FILE__,"Before 2");
		glEnable(GL_TEXTURE_2D);

		if( printData )LogFile(ERROR_LOG,"Point 2");
		glEnableClientState(GL_VERTEX_ARRAY);
		checkGLerror(__LINE__,__FILE__,"Before 3");
		glVertexPointer(4, GL_FLOAT, sizeof(vertex4f) , &dispVerts[0]);

		if( printData )LogFile(ERROR_LOG,"Point 3");
		glClientActiveTextureARB(GL_TEXTURE0_ARB);
		checkGLerror(__LINE__,__FILE__,"Before 4");
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);
		checkGLerror(__LINE__,__FILE__,"Before 5");

		//if( m_cVertPrimLarge.m_sVertexType[ j ].m_cVertexData[h].m_sVertexType.vertexSize == sizeof(sh4_model_vert) )
		//{
			glTexCoordPointer(2, GL_FLOAT, sizeof( largePrimVerts[ 0 ] ), &(largePrimVerts[ 0 ].tc) );	// m_cVertPrimLarge.m_sVertexType[ j ].m_cVertexData[h].m_sVertexType.vertexSize, &m_cVertPrimLarge.m_sVertexType[ j ].m_cVertexData[h].verts[0].tc);
			if( printData )LogFile(ERROR_LOG,"Point 4");
		//}
		//else if( m_cVertPrimLarge.m_sVertexType[ j ].m_cVertexData[h].m_sVertexType.vertexSize == 32 )
		//	glTexCoordPointer(2, GL_FLOAT, m_cVertPrimLarge.m_sVertexType[ j ].m_cVertexData[h].m_sVertexType.vertexSize, &m_cVertPrimLarge.m_sVertexType[ j ].altVerts[0].tc);
		checkGLerror(__LINE__,__FILE__,"Before 6");

		if( printData )LogFile(ERROR_LOG,"Point 5");
		//if( j== 49 )
		glDrawElements(GL_TRIANGLE_STRIP, m_cVertPrimLarge.m_sVertexType[ j ].m_sDataHeader.numIndex, GL_UNSIGNED_INT, indicies ); //m_cVertPrimLarge.m_sVertexType[ j ].m_cVertexData[h].indicies);
		checkGLerror(__LINE__,__FILE__,"Before 7");

		if( printData )LogFile(ERROR_LOG,"Point 6");
		glDisableClientState(GL_VERTEX_ARRAY);
		checkGLerror(__LINE__,__FILE__,"Before 8");
		glDisableClientState(GL_TEXTURE_COORD_ARRAY);

		//if( m_cVertPrimLarge.m_sVertexType[ j ].texModify == 4 )
		glDisable(GL_ALPHA_TEST);
		glFlush( );

		delete [] dispVerts;
		delete [] largePrimVerts;
		delete [] indicies;

//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
		for( i = last_i; i < m_sModelHeader.numAltVertSects; i++ )
		{
			if( m_cVertPrimSmall.m_sVertexType[ i ].m_sDataHeader.q8_vpdh == j )// && j != 2)
			{
//				long z, m;
				if( printData )
					LogFile( ERROR_LOG, "CHECK: j = %ld\ti = %ld\tlast_i = %ld\tnumIndex = %ld", j, i, last_i,m_cVertPrimSmall.m_sVertexType[i].m_sDataHeader.numIndex );
				if( debugRender && printData ) LogFile( DATA_LOG, "\nsmall vert - set %ld\n-------------------------------",j);
			//	dispVerts2 = new vertex4f[ m_cVertPrimSmall.m_sVertexType[ i ].m_sDataHeader.numIndex ];

				for( k = 0; k < m_cVertPrimSmall.m_sVertexType[ i ].m_sDataHeader.numIndex; k++ )
				{
				if( printData )LogFile( ERROR_LOG, "Here 1" );
					long m;			
					long indexNum = m_cVertPrimSmall.indicies[ m_cVertPrimSmall.m_sVertexType[ i ].m_sDataHeader.startIndex + k ];
					tempVert = m_cVertPrimSmall.verts[ indexNum ].vert;
					tempWeights = m_cVertPrimSmall.verts[ indexNum ].weights;

					if( printData )LogFile( ERROR_LOG, "k = %ld\tindexNum=%ld",k,indexNum);
					if( printData )LogFile( ERROR_LOG, "\tMat index: %ld %ld %ld %ld",
						m_cVertPrimSmall.verts[ indexNum ].mats[0],
						m_cVertPrimSmall.verts[ indexNum ].mats[1],
						m_cVertPrimSmall.verts[ indexNum ].mats[2],
						m_cVertPrimSmall.verts[ indexNum ].mats[3] );
					if( printData )LogFile( DATA_LOG, "\tMat index: %ld %ld %ld %ld\tWeights: %f\t%f\t%f\t%f",
						m_cVertPrimSmall.verts[ indexNum ].mats[0],
						m_cVertPrimSmall.verts[ indexNum ].mats[1],
						m_cVertPrimSmall.verts[ indexNum ].mats[2],
						m_cVertPrimSmall.verts[ indexNum ].mats[3],tempWeights.x,tempWeights.y,tempWeights.z,tempWeights.w);
					if( printData )LogFile( ERROR_LOG, "\tWeights: %f\t%f\t%f\t%f",tempWeights.x,tempWeights.y,tempWeights.z,tempWeights.w);

					for( m = 0; m < 4; m++ )
					{
						if( m_cVertPrimSmall.verts[ indexNum ].mats[ m ] != -1 )
						{
							long l_lMatIndex = m_cVertPrimSmall.verts[ indexNum ].mats[ m ];
							if( m == 0 )
								temp[ m ] = l_pcAnimMat[ l_lMatIndex ];//m_cHeaderData.matSet1[ m_cVertPrimSmall.verts[ indexNum ].mats[ m ] ]; //m_pcTransMatSet2[ m_cVertPrimSmall.verts[ indexNum ].mats[ m ] ];//
							else
								temp[ m ] = l_pcAnimMat[ m_cHeaderData.mSeq2[ l_lMatIndex * 2 + 1 ] ] * m_cHeaderData.matSet2[ l_lMatIndex ];

						}
						else
							temp[ m ].identity( );
							
					}
				

					
					if( printData )LogFile( ERROR_LOG, "Here 2" );
					dispVerts2[ indexNum ]=	(temp[ 0 ] * tempVert) * tempWeights.x +
											(temp[ 1 ] * tempVert) * tempWeights.y +
											(temp[ 2 ] * tempVert) * tempWeights.z +
											(temp[ 3 ] * tempVert) * tempWeights.w;

					dispVerts2[ indexNum ].w = 1.0f;
				}

				glActiveTextureARB(GL_TEXTURE0_ARB);
			
				sprintf( tempTexStr, "%ld", this->m_cHeaderData.texIDs[ m_cHeaderData.texIDMapping[ m_cVertPrimSmall.m_sVertexType[ i ].m_sTexHeader.texID * 2 ] ]);
				texID = sh2TexList.GetTex( string(tempTexStr), true );
				if( texID < 1 )
					texID = 1;
				glBindTexture(GL_TEXTURE_2D, texID );
				checkGLerror(__LINE__,__FILE__,"Before 2");
				glEnable(GL_TEXTURE_2D);

				if( printData )LogFile(ERROR_LOG,"Point 2-2");
				glEnableClientState(GL_VERTEX_ARRAY);
				checkGLerror(__LINE__,__FILE__,"Before 3");
				glVertexPointer(4, GL_FLOAT, sizeof(vertex4f) , &dispVerts2[0]);

				if( printData )LogFile(ERROR_LOG,"Point 3-2");
				glClientActiveTextureARB(GL_TEXTURE0_ARB);
				checkGLerror(__LINE__,__FILE__,"Before 4-2");
				glEnableClientState(GL_TEXTURE_COORD_ARRAY);
				checkGLerror(__LINE__,__FILE__,"Before 5-2");

				glTexCoordPointer(2, GL_FLOAT, sizeof( sh2_model_vert_long ), &(m_cVertPrimSmall.verts[ 0 ].tc) );	// m_cVertPrimSmall.m_sVertexType[ j ].m_cVertexData[h].m_sVertexType.vertexSize, &m_cVertPrimSmall.m_sVertexType[ j ].m_cVertexData[h].verts[0].tc);
				if( printData )LogFile(ERROR_LOG,"Point 4-2");
				//	glTexCoordPointer(2, GL_FLOAT, m_cVertPrimSmall.m_sVertexType[ j ].m_cVertexData[h].m_sVertexType.vertexSize, &m_cVertPrimSmall.m_sVertexType[ j ].altVerts[0].tc);
				checkGLerror(__LINE__,__FILE__,"Before 6-2");

				if( printData )LogFile(ERROR_LOG,"Point 5-2");
	
				glDrawElements(GL_TRIANGLE_STRIP, m_cVertPrimSmall.m_sVertexType[ i ].m_sDataHeader.numIndex, GL_UNSIGNED_INT, &(indicies2[m_cVertPrimSmall.m_sVertexType[ i ].m_sDataHeader.startIndex]) ); //m_cVertPrimSmall.m_sVertexType[ j ].m_cVertexData[h].indicies);
				checkGLerror(__LINE__,__FILE__,"Before 7-2");

				if( printData )LogFile(ERROR_LOG,"Point 6-2");
				glDisableClientState(GL_VERTEX_ARRAY);
				checkGLerror(__LINE__,__FILE__,"Before 8-2");
				glDisableClientState(GL_TEXTURE_COORD_ARRAY);

				//if( m_cVertPrimSmall.m_sVertexType[ j ].texModify == 4 )
				glDisable(GL_ALPHA_TEST);

				glFlush( );

				
			}
		}

//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

		delete [] matArray;
	}
delete [] dispVerts2;
delete [] indicies2;
	delete [] l_pcAnimMat;
	printData = false;
	firstRender = false;



}

//###############################################################################
//###############################################################################

//################################################################################################################


void sh2_model::Render2( )
{
	long k, j, i;
	long last_i = 0;
	vertex4f *dispVerts = NULL;
	vertex4f *dispVerts2 = NULL;
	sh2_model_vert_short *largePrimVerts = NULL;
	sh2_model_vert_long *smallPrimVerts = NULL;
	vertex ones(1.0f,1.0f,1.0f);
	matrix *matArray = NULL;
	matrix *l_pcAnimMat = NULL;
	static bool printData = true;
	unsigned int *indicies = NULL;
	unsigned int *indicies2 = NULL;
	vertex4f tv1,tv2,tv3,tv4;
	GLuint texID;

	matrix temp[4];//, temp2, temp3, temp4;
	rgba tempMatSet;
	vertex4f tempVert;
	vertex4f tempWeights;


//	char outFile[128];
	char tempTexStr[ 128 ];
	
	l_pcAnimMat = new matrix[ m_sModelHeader.numMatSet1 ];
/*
	if( m_bHasAnim )
	{
		matrix *m_pcDispFinal = new matrix[ m_sModelHeader.numMatSet1 ];

		//----[ TWO WAYS TO DO IT: ROTATE ORIGIN AND TRANSLATE BY PARENT, or ROTATE RELATIVE OFFSET AND TRANSLATE PARENT ]--
		for( k = 0; k < m_sModelHeader.numMatSet1; k++ )
			m_pcDispFinal[ k ] = m_cHeaderData.matSet1[ k ] * m_pcAnimMat[ k ];
		//----[ REBUILD THE MODEL ]----/
		for( k = 0; k < m_sModelHeader.numMatSet1; k++ )
		{
			if( m_cHeaderData.mSeq1[ k ] != 255 )	//-1 )
				l_pcAnimMat[ k ] = l_pcAnimMat[ m_cHeaderData.mSeq1[ k ] ] * m_pcDispFinal[ k ];
			else
				l_pcAnimMat[ k ] = m_pcDispFinal[ k ];
			if( printData )
			{
				LogFile(ERROR_LOG," %2d. Final Animation Matrix\n---------------------------",k);
				debugMatrix( l_pcAnimMat[ k ] );
			}
		}

		delete [] m_pcDispFinal;
	}
	else
	{
		//----[ BUILD THE MODEL ]----/
		for( k = 0; k < m_sModelHeader.numMatSet1; k++ )
		{
			if( m_cHeaderData.mSeq1[ k ] != 255 )	//-1 )
				l_pcAnimMat[ k ] = l_pcAnimMat[ m_cHeaderData.mSeq1[ k ] ] * m_cHeaderData.matSet1[ k ];
			else
				l_pcAnimMat[ k ] = m_cHeaderData.matSet1[ k ];
			if( printData )
			{
				LogFile(ERROR_LOG," %2d. Final Animation Matrix\n---------------------------",k);
				debugMatrix( l_pcAnimMat[ k ] );
			}
		}
	}
*/

	//---[ RENDER SEQUENCED PRIMITIVES (LARGE HEADERS, SMALL VERTS ) ]---/

	dispVerts = new vertex4f[ this->m_sVertexHeader.numSet1Verts ];
	indicies = new unsigned int[ m_cVertPrimLarge.m_lNumIndex ];

	for( k = 0; k < m_cVertPrimLarge.m_lNumIndex; k++ )
		indicies[k] = m_cVertPrimLarge.indicies[ k ];

	if( this->m_sModelHeader.numAltVertSects )
	{
		if( this->m_sVertexHeader.numSet2Verts )
			dispVerts2 = new vertex4f[ this->m_sVertexHeader.numSet2Verts ];
		else
			dispVerts2 = NULL;

		if( m_cVertPrimSmall.m_lNumIndex )
		{
			indicies2 = new unsigned int[ m_cVertPrimSmall.m_lNumIndex ];

			for( k = 0; k < m_cVertPrimSmall.m_lNumIndex; k++ )
				indicies2[k] = m_cVertPrimSmall.indicies[ k ];
		}
		else
			indicies2 = NULL;
	}

	if( printData )
	{
		LogFile( ERROR_LOG, "CHECK: There are %ld verts and %ld indicies",m_cVertPrimLarge.m_lNumVerts,
			m_cVertPrimLarge.m_lNumIndex );
		LogFile( ERROR_LOG, "CHECK: Generated 0x%08x for the vert address and 0x%08x for the index",
			dispVerts, indicies );
//		for( k = 0; k < m_cVertPrimLarge.m_lNumIndex; k++ )
//			LogFile( ERROR_LOG, "\t%ld\t %ld\t\t%ld",k, m_cVertPrimLarge.indicies[ k ], indicies[ k ] );
	}

	for( j = 0; j < m_sModelHeader.numVertSects; j++ )
	{
		if(printData)LogFile(ERROR_LOG,"Generating Large Prim %ld of %ld",j+1,this->m_sModelHeader.numVertSects);

		matArray = new matrix[ m_cVertPrimLarge.m_sVertexType[ j ].m_sSeqHeader.numSeq1Data + m_cVertPrimLarge.m_sVertexType[ j ].m_sSeqHeader.numSeq2Data];
		
		for( k = 0; k < m_cVertPrimLarge.m_sVertexType[ j ].m_sSeqHeader.numSeq1Data; k ++ )
		{
			matArray[k] = m_cHeaderData.matSet1[ m_cVertPrimLarge.m_sVertexType[ j ].seqData1[k]];
//			matArray[k].transpose();
		}

		for( k = m_cVertPrimLarge.m_sVertexType[ j ].m_sSeqHeader.numSeq1Data; k < m_cVertPrimLarge.m_sVertexType[ j ].m_sSeqHeader.numSeq1Data + m_cVertPrimLarge.m_sVertexType[ j ].m_sSeqHeader.numSeq2Data; k++ )
		{
			matArray[k] = m_pcTransMatSet2[ m_cVertPrimLarge.m_sVertexType[ j ].seqData2[k-m_cVertPrimLarge.m_sVertexType[ j ].m_sSeqHeader.numSeq1Data] ];
		//	matArray[k] = m_cHeaderData.matSet1[ m_cHeaderData.mSeq2[ m_cVertPrimLarge.m_sVertexType[ j ].seqData2[k-m_cVertPrimLarge.m_sVertexType[ j ].m_sSeqHeader.numSeq1Data] * 2 + 1 ] ]
		//				* m_cHeaderData.matSet2[m_cVertPrimLarge.m_sVertexType[ j ].seqData2[k-m_cVertPrimLarge.m_sVertexType[ j ].m_sSeqHeader.numSeq1Data]];
//			matArray[k].transpose();
		}


		for( k = 0; k < m_cVertPrimLarge.m_sVertexType[ j ].m_sDataHeader.numIndex; k++ )
		{
				
			
			if( m_cVertPrimLarge.m_sVertexType[ j ].m_sSeqHeader.numSeq1Data + m_cVertPrimLarge.m_sVertexType[ j ].m_sSeqHeader.numSeq2Data > 0 )
			{
				//if( m_cVertPrimLarge.m_sVertexType[ j ].m_cVertexData[h].verts == NULL && m_cVertPrimLarge.m_sVertexType[ j ].altVerts != NULL )
				//{
				//	LogFile(TEST_LOG,"SH3_Actor::draw - TERMINAL ERROR: Can't render prim #%d - Small size verts have multi-sequence matricies w/o selector data",j);
				//	break;
				//}
				long indexNum = m_cVertPrimLarge.indicies[ m_cVertPrimLarge.m_sVertexType[ j ].m_sDataHeader.startIndex + k ];
				tempVert = m_cVertPrimLarge.verts[ indexNum ].vert;
				tempWeights = m_cVertPrimLarge.verts[ indexNum ].weights;

				copyRGBA( tempMatSet, m_cVertPrimLarge.verts[ indexNum ].mats );

				if( tempMatSet.r != 255 )
					temp[ 0 ] = matArray[ tempMatSet.r ];
				else
					temp[ 0 ] = matArray[ 0 ];

				if( tempMatSet.g != 255 )
					temp[ 1 ] = matArray[ tempMatSet.g ];
				else
					temp[ 1 ] = matArray[ 0 ];

				if( tempMatSet.b != 255 )
					temp[ 2 ] = matArray[ tempMatSet.b ];
				else
					temp[ 2 ] = matArray[ 0 ];

				if( tempMatSet.a != 255 )
					temp[ 3 ] = matArray[ tempMatSet.a ];
				else
					temp[ 3 ] = matArray[ 0 ];

		//		if( printData )LogFile( ERROR_LOG,"matricies: %ld  %ld  %ld  %ld",
		//				tempMatSet.r,tempMatSet.g,tempMatSet.b,tempMatSet.a);
						
				if(printData)
				{
					if( tempMatSet.r > m_cVertPrimLarge.m_sVertexType[ j ].m_sSeqHeader.numSeq1Data + m_cVertPrimLarge.m_sVertexType[ j ].m_sSeqHeader.numSeq2Data
						|| tempMatSet.g > m_cVertPrimLarge.m_sVertexType[ j ].m_sSeqHeader.numSeq1Data + m_cVertPrimLarge.m_sVertexType[ j ].m_sSeqHeader.numSeq2Data
						|| tempMatSet.b > m_cVertPrimLarge.m_sVertexType[ j ].m_sSeqHeader.numSeq1Data + m_cVertPrimLarge.m_sVertexType[ j ].m_sSeqHeader.numSeq2Data
						|| tempMatSet.a > m_cVertPrimLarge.m_sVertexType[ j ].m_sSeqHeader.numSeq1Data + m_cVertPrimLarge.m_sVertexType[ j ].m_sSeqHeader.numSeq2Data)
						LogFile(ERROR_LOG,"TERMINAL ERROR::: The matrix is out of range [%d %d %d %d]",
							tempMatSet.r,tempMatSet.g,tempMatSet.b,tempMatSet.a);
				}

				dispVerts[ indexNum ]=	(temp[ 0 ] * tempVert) * tempWeights.x +
										(temp[ 1 ] * tempVert) * tempWeights.y +
										(temp[ 2 ] * tempVert) * tempWeights.z +
										(temp[ 3 ] * tempVert) * tempWeights.w;
	
				dispVerts[ indexNum ].w = 1.0f;
			}

		}

		if( printData )LogFile(ERROR_LOG,"Point 1");
			//if( m_cVertPrimLarge.m_sVertexType[ j ].texModify == 4 )
			//{
	//			glAlphaFunc(GL_GEQUAL, 0.01f);
	//			glEnable(GL_ALPHA_TEST);
			//}
		glFrontFace(GL_CCW);
		checkGLerror(__LINE__,__FILE__,"Before Render");
		glActiveTextureARB(GL_TEXTURE0_ARB);
		checkGLerror(__LINE__,__FILE__,"Before 1");
			
		sprintf( tempTexStr, "%ld", this->m_cHeaderData.texIDs[ m_cHeaderData.texIDMapping[ m_cVertPrimLarge.m_sVertexType[ j ].m_sTexHeader.texID * 2 ] ]);
		texID = sh2TexList.GetTex( string(tempTexStr), true );
		if( texID < 1 )
			texID = 1;
		glBindTexture(GL_TEXTURE_2D, texID );
		checkGLerror(__LINE__,__FILE__,"Before 2");
		glEnable(GL_TEXTURE_2D);
		glDisable(GL_ALPHA_TEST);

		if( printData )LogFile(ERROR_LOG,"Point 2");
		glEnableClientState(GL_VERTEX_ARRAY);
		checkGLerror(__LINE__,__FILE__,"Before 3");
		glVertexPointer(4, GL_FLOAT, sizeof(vertex4f) , &dispVerts[0]);

		if( printData )LogFile(ERROR_LOG,"Point 3");
		glClientActiveTextureARB(GL_TEXTURE0_ARB);
		checkGLerror(__LINE__,__FILE__,"Before 4");
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);
		checkGLerror(__LINE__,__FILE__,"Before 5");

		//if( m_cVertPrimLarge.m_sVertexType[ j ].m_cVertexData[h].m_sVertexType.vertexSize == sizeof(sh4_model_vert) )
		//{
			glTexCoordPointer(2, GL_FLOAT, sizeof( sh2_model_vert_short ), &(m_cVertPrimLarge.verts[ 0 ].tc) );	// m_cVertPrimLarge.m_sVertexType[ j ].m_cVertexData[h].m_sVertexType.vertexSize, &m_cVertPrimLarge.m_sVertexType[ j ].m_cVertexData[h].verts[0].tc);
			if( printData )LogFile(ERROR_LOG,"Point 4");
		//}
		//else if( m_cVertPrimLarge.m_sVertexType[ j ].m_cVertexData[h].m_sVertexType.vertexSize == 32 )
		//	glTexCoordPointer(2, GL_FLOAT, m_cVertPrimLarge.m_sVertexType[ j ].m_cVertexData[h].m_sVertexType.vertexSize, &m_cVertPrimLarge.m_sVertexType[ j ].altVerts[0].tc);
		checkGLerror(__LINE__,__FILE__,"Before 6");

		if( printData )LogFile(ERROR_LOG,"Point 5");
		//if( j== 49 )
		glDrawElements(GL_TRIANGLE_STRIP, m_cVertPrimLarge.m_sVertexType[ j ].m_sDataHeader.numIndex, GL_UNSIGNED_INT, &(indicies[m_cVertPrimLarge.m_sVertexType[ j ].m_sDataHeader.startIndex]) ); //m_cVertPrimLarge.m_sVertexType[ j ].m_cVertexData[h].indicies);
		checkGLerror(__LINE__,__FILE__,"Before 7");

		if( printData )LogFile(ERROR_LOG,"Point 6");
		glDisableClientState(GL_VERTEX_ARRAY);
		checkGLerror(__LINE__,__FILE__,"Before 8");
		glDisableClientState(GL_TEXTURE_COORD_ARRAY);

		//if( m_cVertPrimLarge.m_sVertexType[ j ].texModify == 4 )


		glFlush( );

//		if( j == 10 || j == 11 )
		for( i = last_i; i < m_sModelHeader.numAltVertSects; i++ )
		{
/*			long start_i, i_step, end_i;
			if( j == 10 )
			{
				start_i = 0; i_step = 2; end_i = 5;
			}
			else if( j == 11 )
			{
				start_i = 1; i_step = 3; end_i = 5;
			}
*/			
//			for( i = start_i; i < end_i; i+= i_step )
			if( m_cVertPrimSmall.m_sVertexType[ i ].m_sDataHeader.q8_vpdh == j )
			{
//				long z, m;
				if( printData )
					LogFile( ERROR_LOG, "CHECK: j = %ld\ti = %ld\tlast_i = %ld", j, i, last_i );


				for( k = 0; k < m_cVertPrimSmall.m_sVertexType[ i ].m_sDataHeader.numIndex; k++ )
				{
				
			
					long indexNum = m_cVertPrimSmall.indicies[ m_cVertPrimSmall.m_sVertexType[ i ].m_sDataHeader.startIndex + k ];
					tempVert = m_cVertPrimSmall.verts[ indexNum ].vert;
					tempWeights = m_cVertPrimSmall.verts[ indexNum ].weights;
/*

					for( m = 0; m < 4; m++ )
					{
						if( m_cVertPrimSmall.verts[ indexNum ].mats[ m ] != -1 )
						{
							for( z = 0; z < m_cVertPrimLarge.m_sVertexType[ j ].m_sSeqHeader.numSeq1Data; z++ )
							{
								if( m_cVertPrimSmall.verts[ indexNum ].mats[ m ] == m_cVertPrimLarge.m_sVertexType[ j ].seqData1[ z ] )
								{
									z = 255;
									temp[ m ] = m_cHeaderData.matSet1[ m_cVertPrimSmall.verts[ indexNum ].mats[ m ] ];
								}
							}

							if( z != 255 )
							{
								for( z = 0; z < m_cVertPrimLarge.m_sVertexType[ j ].m_sSeqHeader.numSeq2Data; z++ )
								{
									if( m_cVertPrimSmall.verts[ indexNum ].mats[ 0 ] == m_cVertPrimLarge.m_sVertexType[ j ].seqData2[ z ] )
									{
										z = 255;
										temp[ m ] = this->m_pcTransMatSet2[ m_cVertPrimSmall.verts[ indexNum ].mats[ m ] ];
									}
								}
							}
							if( z != 255 )
								temp[ m ] = this->m_pcTransMatSet2[ m_cVertPrimSmall.verts[ indexNum ].mats[ m ] ];
						}
						else
							temp[ m ] = matArray[ m ];
					}
*/				

					if( m_cVertPrimSmall.verts[ indexNum ].mats[0] != -1 )
					{
						if( m_cVertPrimSmall.verts[ indexNum ].mats[0]  > 22 )
							temp[ 0 ] = this->m_pcTransMatSet2[ m_cVertPrimSmall.verts[ indexNum ].mats[0] ];//matArray[ m_cVertPrimSmall.verts[ indexNum ].mats[0] ];
						else
							temp[ 0 ] = m_cHeaderData.matSet1[ m_cVertPrimSmall.verts[ indexNum ].mats[0] ];//matArray[ m_cVertPrimSmall.verts[ indexNum ].mats[0] ];
					}
					else
						temp[ 0 ] = matArray[ 0 ];

					if( m_cVertPrimSmall.verts[ indexNum ].mats[1] != -1 )
					{
						if( m_cVertPrimSmall.verts[ indexNum ].mats[1]  > 22 )
							temp[ 1 ] = this->m_pcTransMatSet2[ m_cVertPrimSmall.verts[ indexNum ].mats[1] ];//matArray[ m_cVertPrimSmall.verts[ indexNum ].mats[1] ];
						else
							temp[ 1 ] = m_cHeaderData.matSet1[ m_cVertPrimSmall.verts[ indexNum ].mats[1] ];//matArray[ m_cVertPrimSmall.verts[ indexNum ].mats[1] ];
					}
					else
						temp[ 1 ] = matArray[ 0 ];

					if( m_cVertPrimSmall.verts[ indexNum ].mats[2] != -1 )
					{
						if( m_cVertPrimSmall.verts[ indexNum ].mats[2]  > 22 )
							temp[ 2 ] = this->m_pcTransMatSet2[ m_cVertPrimSmall.verts[ indexNum ].mats[2] ];//matArray[ m_cVertPrimSmall.verts[ indexNum ].mats[2] ];
						else
							temp[ 2 ] = m_cHeaderData.matSet1[ m_cVertPrimSmall.verts[ indexNum ].mats[2] ];//matArray[ m_cVertPrimSmall.verts[ indexNum ].mats[2] ];
					}
					else
						temp[ 2 ] = matArray[ 0 ];

					if( m_cVertPrimSmall.verts[ indexNum ].mats[3] != -1 )
					{
						if( m_cVertPrimSmall.verts[ indexNum ].mats[3]  > 22 )
							temp[ 3 ] = this->m_pcTransMatSet2[ m_cVertPrimSmall.verts[ indexNum ].mats[3] ];//matArray[ m_cVertPrimSmall.verts[ indexNum ].mats[3] ];
						else
							temp[ 3 ] = m_cHeaderData.matSet1[ m_cVertPrimSmall.verts[ indexNum ].mats[3] ];//matArray[ m_cVertPrimSmall.verts[ indexNum ].mats[3] ];
					}
					else
						temp[ 3 ] = matArray[ 0 ];
						
					if(printData)
					{
						if( m_cVertPrimSmall.verts[ indexNum ].mats[0] > m_sModelHeader.numMatSet1
							||  m_cVertPrimSmall.verts[ indexNum ].mats[1] > m_sModelHeader.numMatSet1
							||  m_cVertPrimSmall.verts[ indexNum ].mats[2] > m_sModelHeader.numMatSet1
							||  m_cVertPrimSmall.verts[ indexNum ].mats[3] > m_sModelHeader.numMatSet1 )
							LogFile(ERROR_LOG,"TERMINAL ERROR::: The matrix is out of range [%d %d %d %d]",
								m_cVertPrimSmall.verts[ indexNum ].mats[0],
								m_cVertPrimSmall.verts[ indexNum ].mats[1],
								m_cVertPrimSmall.verts[ indexNum ].mats[2],
								m_cVertPrimSmall.verts[ indexNum ].mats[3] );
					}

					dispVerts2[ indexNum ]=	(temp[ 0 ] * tempVert) * tempWeights.x +
											(temp[ 1 ] * tempVert) * tempWeights.y +
											(temp[ 2 ] * tempVert) * tempWeights.z +
											(temp[ 3 ] * tempVert) * tempWeights.w;

					dispVerts2[ indexNum ].w = 1.0f;
				}

				glActiveTextureARB(GL_TEXTURE0_ARB);
			
				sprintf( tempTexStr, "%ld", this->m_cHeaderData.texIDs[ m_cHeaderData.texIDMapping[ m_cVertPrimSmall.m_sVertexType[ i ].m_sTexHeader.texID * 2 ] ]);
				texID = sh2TexList.GetTex( string(tempTexStr), true );
				if( texID < 1 )
					texID = 1;
				glBindTexture(GL_TEXTURE_2D, texID );
				checkGLerror(__LINE__,__FILE__,"Before 2");
				glEnable(GL_TEXTURE_2D);

				if( printData )LogFile(ERROR_LOG,"Point 2-2");
				glEnableClientState(GL_VERTEX_ARRAY);
				checkGLerror(__LINE__,__FILE__,"Before 3");
				glVertexPointer(4, GL_FLOAT, sizeof(vertex4f) , &dispVerts2[0]);

				if( printData )LogFile(ERROR_LOG,"Point 3-2");
				glClientActiveTextureARB(GL_TEXTURE0_ARB);
				checkGLerror(__LINE__,__FILE__,"Before 4-2");
				glEnableClientState(GL_TEXTURE_COORD_ARRAY);
				checkGLerror(__LINE__,__FILE__,"Before 5-2");

				glTexCoordPointer(2, GL_FLOAT, sizeof( sh2_model_vert_long ), &(m_cVertPrimSmall.verts[ 0 ].tc) );	// m_cVertPrimSmall.m_sVertexType[ j ].m_cVertexData[h].m_sVertexType.vertexSize, &m_cVertPrimSmall.m_sVertexType[ j ].m_cVertexData[h].verts[0].tc);
				if( printData )LogFile(ERROR_LOG,"Point 4-2");
				//	glTexCoordPointer(2, GL_FLOAT, m_cVertPrimSmall.m_sVertexType[ j ].m_cVertexData[h].m_sVertexType.vertexSize, &m_cVertPrimSmall.m_sVertexType[ j ].altVerts[0].tc);
				checkGLerror(__LINE__,__FILE__,"Before 6-2");

				if( printData )LogFile(ERROR_LOG,"Point 5-2");
	
				glDrawElements(GL_TRIANGLE_STRIP, m_cVertPrimSmall.m_sVertexType[ i ].m_sDataHeader.numIndex, GL_UNSIGNED_INT, &(indicies2[m_cVertPrimSmall.m_sVertexType[ i ].m_sDataHeader.startIndex]) ); //m_cVertPrimSmall.m_sVertexType[ j ].m_cVertexData[h].indicies);
				checkGLerror(__LINE__,__FILE__,"Before 7-2");

				if( printData )LogFile(ERROR_LOG,"Point 6-2");
				glDisableClientState(GL_VERTEX_ARRAY);
				checkGLerror(__LINE__,__FILE__,"Before 8-2");
				glDisableClientState(GL_TEXTURE_COORD_ARRAY);

				//if( m_cVertPrimSmall.m_sVertexType[ j ].texModify == 4 )
				glDisable(GL_ALPHA_TEST);

				glFlush( );
			}
		}

//		checkGLerror(__LINE__,__FILE__,"After Render");
		//glDisableClientState(GL_NORMAL_ARRAY);

		delete [] matArray;
	}

	delete [] indicies;
	delete [] dispVerts;
	delete [] indicies2;
	delete [] dispVerts2;
	delete [] l_pcAnimMat;
	printData = false;




}

//################################################################################

void sh2_model::Render3( )
{
	long k, j;
	vertex4f *dispVerts = NULL;
	sh2_model_vert_short *largePrimVerts = NULL;
	sh2_model_vert_long *smallPrimVerts = NULL;
	vertex ones(1.0f,1.0f,1.0f);
	matrix *matArray = NULL;
	matrix *l_pcAnimMat = NULL;
	static bool printData = true;
	unsigned int *indicies;
	vertex4f tv1,tv2,tv3,tv4;
	GLuint texID;

	matrix temp, temp2, temp3, temp4;
//	rgba tempMatSet;
	vertex4f tempVert;
	vertex4f tempWeights;


//	char outFile[128];
	char tempTexStr[ 128 ];
	
	l_pcAnimMat = new matrix[ m_sModelHeader.numMatSet1 ];
/*
	if( m_bHasAnim )
	{
		matrix *m_pcDispFinal = new matrix[ m_sModelHeader.numMatSet1 ];

		//----[ TWO WAYS TO DO IT: ROTATE ORIGIN AND TRANSLATE BY PARENT, or ROTATE RELATIVE OFFSET AND TRANSLATE PARENT ]--
		for( k = 0; k < m_sModelHeader.numMatSet1; k++ )
			m_pcDispFinal[ k ] = m_cHeaderData.matSet1[ k ] * m_pcAnimMat[ k ];
		//----[ REBUILD THE MODEL ]----/
		for( k = 0; k < m_sModelHeader.numMatSet1; k++ )
		{
			if( m_cHeaderData.mSeq1[ k ] != 255 )	//-1 )
				l_pcAnimMat[ k ] = l_pcAnimMat[ m_cHeaderData.mSeq1[ k ] ] * m_pcDispFinal[ k ];
			else
				l_pcAnimMat[ k ] = m_pcDispFinal[ k ];
			if( printData )
			{
				LogFile(ERROR_LOG," %2d. Final Animation Matrix\n---------------------------",k);
				debugMatrix( l_pcAnimMat[ k ] );
			}
		}

		delete [] m_pcDispFinal;
	}
	else
	{
		//----[ BUILD THE MODEL ]----/
		for( k = 0; k < m_sModelHeader.numMatSet1; k++ )
		{
			if( m_cHeaderData.mSeq1[ k ] != 255 )	//-1 )
				l_pcAnimMat[ k ] = l_pcAnimMat[ m_cHeaderData.mSeq1[ k ] ] * m_cHeaderData.matSet1[ k ];
			else
				l_pcAnimMat[ k ] = m_cHeaderData.matSet1[ k ];
			if( printData )
			{
				LogFile(ERROR_LOG," %2d. Final Animation Matrix\n---------------------------",k);
				debugMatrix( l_pcAnimMat[ k ] );
			}
		}
	}
*/

	//---[ RENDER SEQUENCED PRIMITIVES (LARGE HEADERS, SMALL VERTS ) ]---/

	dispVerts = new vertex4f[ this->m_sVertexHeader.numSet2Verts ];
	indicies = new unsigned int[ m_cVertPrimSmall.m_lNumIndex ];

	for( k = 0; k < m_cVertPrimSmall.m_lNumIndex; k++ )
		indicies[k] = m_cVertPrimSmall.indicies[ k ];

	if( printData )
	{
		LogFile( ERROR_LOG, "CHECK: There are %ld verts and %ld indicies",m_cVertPrimSmall.m_lNumVerts,
			m_cVertPrimSmall.m_lNumIndex );
		LogFile( ERROR_LOG, "CHECK: Generated 0x%08x for the vert address and 0x%08x for the index",
			dispVerts, indicies );
//		for( k = 0; k < m_cVertPrimSmall.m_lNumIndex; k++ )
//			LogFile( ERROR_LOG, "\t%ld\t %ld\t\t%ld",k, m_cVertPrimSmall.indicies[ k ], indicies[ k ] );
	}

	for( j = 0; j < m_sModelHeader.numAltVertSects; j++ )
	{
		if(printData)LogFile(ERROR_LOG,"Generating Large Prim %ld of %ld",j+1,this->m_sModelHeader.numVertSects);

		matArray = m_cHeaderData.matSet1; //this->m_pcTransMatSet2; //
		
		for( k = 0; k < m_cVertPrimSmall.m_sVertexType[ j ].m_sDataHeader.numIndex; k++ )
		{
				
			
			long indexNum = m_cVertPrimSmall.indicies[ m_cVertPrimSmall.m_sVertexType[ j ].m_sDataHeader.startIndex + k ];
			tempVert = m_cVertPrimSmall.verts[ indexNum ].vert;
			tempWeights = m_cVertPrimSmall.verts[ indexNum ].weights;

			//copyRGBA( tempMatSet, m_cVertPrimSmall.verts[ indexNum ].mats );

			if( m_cVertPrimSmall.verts[ indexNum ].mats[0] != -1 )
				temp = matArray[ m_cVertPrimSmall.verts[ indexNum ].mats[0] ];
			else
				temp = matArray[ 0 ];

			if( m_cVertPrimSmall.verts[ indexNum ].mats[1] != -1 )
				temp = matArray[ m_cVertPrimSmall.verts[ indexNum ].mats[1] ];
			else
				temp = matArray[ 0 ];

			if( m_cVertPrimSmall.verts[ indexNum ].mats[2] != -1 )
				temp = matArray[ m_cVertPrimSmall.verts[ indexNum ].mats[2] ];
			else
				temp = matArray[ 0 ];

			if( m_cVertPrimSmall.verts[ indexNum ].mats[3] != -1 )
				temp = matArray[ m_cVertPrimSmall.verts[ indexNum ].mats[3] ];
			else
				temp = matArray[ 0 ];

		//		if( printData )LogFile( ERROR_LOG,"matricies: %ld  %ld  %ld  %ld",
		//				m_cVertPrimSmall.verts[ indexNum ].mats[0],
		//				m_cVertPrimSmall.verts[ indexNum ].mats[1],
		//				m_cVertPrimSmall.verts[ indexNum ].mats[2],
		//				m_cVertPrimSmall.verts[ indexNum ].mats[3] );
						
			if(printData)
			{
				if( m_cVertPrimSmall.verts[ indexNum ].mats[0] > m_sModelHeader.numMatSet1
					||  m_cVertPrimSmall.verts[ indexNum ].mats[1] > m_sModelHeader.numMatSet1
					||  m_cVertPrimSmall.verts[ indexNum ].mats[2] > m_sModelHeader.numMatSet1
					||  m_cVertPrimSmall.verts[ indexNum ].mats[3] > m_sModelHeader.numMatSet1 )
					LogFile(ERROR_LOG,"TERMINAL ERROR::: The matrix is out of range [%d %d %d %d]",
						m_cVertPrimSmall.verts[ indexNum ].mats[0],
						m_cVertPrimSmall.verts[ indexNum ].mats[1],
						m_cVertPrimSmall.verts[ indexNum ].mats[2],
						m_cVertPrimSmall.verts[ indexNum ].mats[3] );
			}

			dispVerts[ indexNum ]=	(temp * tempVert) * tempWeights.x +
									(temp2 * tempVert) * tempWeights.y +
									(temp3 * tempVert) * tempWeights.z +
									(temp4 * tempVert) * tempWeights.w;

			dispVerts[ indexNum ].w = 1.0f;
		}

		if( printData )LogFile(ERROR_LOG,"Point 1");
			//if( m_cVertPrimSmall.m_sVertexType[ j ].texModify == 4 )
			//{
	//			glAlphaFunc(GL_GEQUAL, 0.01f);
	//			glEnable(GL_ALPHA_TEST);
			//}
		glFrontFace(GL_CCW);
		checkGLerror(__LINE__,__FILE__,"Before Render");
		glActiveTextureARB(GL_TEXTURE0_ARB);
		checkGLerror(__LINE__,__FILE__,"Before 1");
			
		sprintf( tempTexStr, "%ld", this->m_cHeaderData.texIDs[ m_cHeaderData.texIDMapping[ m_cVertPrimSmall.m_sVertexType[ j ].m_sTexHeader.texID * 2 ] ]);
		texID = sh2TexList.GetTex( string(tempTexStr), true );
		if( texID < 1 )
			texID = 1;
		glBindTexture(GL_TEXTURE_2D, texID );
		checkGLerror(__LINE__,__FILE__,"Before 2");
		glEnable(GL_TEXTURE_2D);

		if( printData )LogFile(ERROR_LOG,"Point 2");
		glEnableClientState(GL_VERTEX_ARRAY);
		checkGLerror(__LINE__,__FILE__,"Before 3");
		glVertexPointer(4, GL_FLOAT, sizeof(vertex4f) , &dispVerts[0]);

		if( printData )LogFile(ERROR_LOG,"Point 3");
		glClientActiveTextureARB(GL_TEXTURE0_ARB);
		checkGLerror(__LINE__,__FILE__,"Before 4");
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);
		checkGLerror(__LINE__,__FILE__,"Before 5");

		//if( m_cVertPrimSmall.m_sVertexType[ j ].m_cVertexData[h].m_sVertexType.vertexSize == sizeof(sh4_model_vert) )
		//{
			glTexCoordPointer(2, GL_FLOAT, sizeof( sh2_model_vert_long ), &(m_cVertPrimSmall.verts[ 0 ].tc) );	// m_cVertPrimSmall.m_sVertexType[ j ].m_cVertexData[h].m_sVertexType.vertexSize, &m_cVertPrimSmall.m_sVertexType[ j ].m_cVertexData[h].verts[0].tc);
			if( printData )LogFile(ERROR_LOG,"Point 4");
		//}
		//else if( m_cVertPrimSmall.m_sVertexType[ j ].m_cVertexData[h].m_sVertexType.vertexSize == 32 )
		//	glTexCoordPointer(2, GL_FLOAT, m_cVertPrimSmall.m_sVertexType[ j ].m_cVertexData[h].m_sVertexType.vertexSize, &m_cVertPrimSmall.m_sVertexType[ j ].altVerts[0].tc);
		checkGLerror(__LINE__,__FILE__,"Before 6");

		if( printData )LogFile(ERROR_LOG,"Point 5");
		//if( j== 49 )
		glDrawElements(GL_TRIANGLE_STRIP, m_cVertPrimSmall.m_sVertexType[ j ].m_sDataHeader.numIndex, GL_UNSIGNED_INT, &(indicies[m_cVertPrimSmall.m_sVertexType[ j ].m_sDataHeader.startIndex]) ); //m_cVertPrimSmall.m_sVertexType[ j ].m_cVertexData[h].indicies);
		checkGLerror(__LINE__,__FILE__,"Before 7");

		if( printData )LogFile(ERROR_LOG,"Point 6");
		glDisableClientState(GL_VERTEX_ARRAY);
		checkGLerror(__LINE__,__FILE__,"Before 8");
		glDisableClientState(GL_TEXTURE_COORD_ARRAY);

		//if( m_cVertPrimSmall.m_sVertexType[ j ].texModify == 4 )
		glDisable(GL_ALPHA_TEST);

		glFlush( );
	}

	delete [] indicies;
	delete [] dispVerts;
	delete [] l_pcAnimMat;
	printData = false;




}



long testLoadSH2_Model( char *filename )
{
	sh4_model testModel;
	sh2_model_base_header l_sModelBaseHeader;
	FILE *inFile = NULL;
	long lNumRead;
	long lTotalread = 0;

	if( ( inFile = fopen( filename, "rb" ) ) == 0 )
	{
		LogFile( ERROR_LOG, "testLoadSH2_Model( ) - ERROR: Couldn't open file '%s' - %s",filename, strerror(errno));
		return 0;
	}

	lNumRead = _loadBlock( (void*)&l_sModelBaseHeader, sizeof(l_sModelBaseHeader) , inFile,
						"testLoadSH2_Model() - Could not read base model head",ERROR_LOG);
	if( lNumRead == -1 )
		return 0;

	lTotalread = lNumRead;

	lNumRead = testModel.loadDataAux( inFile );

	fclose( inFile );

	if( ! lNumRead )
		LogFile( ERROR_LOG, "testLoadSH2_Model( ) - ERROR: Couldn't load the SH2 Model w/ the SH4 loader");
	else
		LogFile( ERROR_LOG, "testLoadSH2_Model( ) - SUCCESS, I think...");

	return lNumRead;
}


	