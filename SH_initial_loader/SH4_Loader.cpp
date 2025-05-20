#include <windows.h>
#include <gl\gl.h>
#include <gl\glu.h>
#include <gl\wglext.h>
#include <gl\glext.h>
#include <gl\glprocs.h>
#include <errno.h>
#include "vertex.h"
#include "matrix.h"
#include "RVertex.h"
#include "SH4_Loader.h"
#include "Renderer.h"
#include "utils.h"

extern TexMgr textureMgr;
extern bool debugMode;
extern int errno;

void debugSH4_MTH( sh4_tex_header *h )
{
	LogFile( ERROR_LOG, " ]>==---==<[ SH4 Model Tex Header (size %ld) ]>==---==<[",sizeof(sh4_tex_header));
	debugShort( h->numT1 );
	debugShort( h->numT2 );
	debugLong( h->f1_th );
	debugLong( h->f2_th );
	debugLong( h->f3_th );
}

void debugSH4_MTT1( sh4_model_tex_type1 *h )
{
	LogFile( ERROR_LOG, " ]>==---==<[ SH4 Model Tex Type 1 (size %ld) ]>==---==<[",sizeof(sh4_model_tex_type1));
	debugLong( h->xRes );
	debugLong( h->yRes );
	debugShort( h->flag1 );
	debugShort( h->flag2 );
	debugLong( h->f1_tt1 );
}

void debugSH4_MTT2( sh4_model_tex_type2 *h )
{
	LogFile( ERROR_LOG, " ]>==---==<[ SH4 Model Tex Type 2 (size %ld) ]>==---==<[",sizeof(sh4_model_tex_type2));
	debugLong( h->marker );
	debugLong( h->numTex );
	debugLong( h->f1_tt2 );
	debugLong( h->offsetType3 );	//Offset from start of THIS struct
}

void debugSH4_MTT3( sh4_model_tex_type3 *h )
{
	LogFile( ERROR_LOG, " ]>==---==<[ SH4 Model Tex Type 3 (size %ld) ]>==---==<[",sizeof(sh4_model_tex_type3));
	debugLong( h->f1_tt3 );
	debugLong( h->f2_tt3 );
	debugLong( h->f3_tt3 );
	debugLong( h->f4_tt3 );
	debugLong( h->f5_tt3 );
	debugLong( h->f6_tt3 );
	debugLong( h->f7_tt3 );
	debugLong( h->f8_tt3 );
	debugLong( h->xRes );
	debugLong( h->yRes );
	debugLong( h->texFormat );
	LogFile( ERROR_LOG, "Format Check: %4.4s\t0x%08x",(char *)(&(h->texFormat)), h->texFormat );
	debugLong( h->mipMapCount );
	debugLong( h->texDataSize );
	debugLong( h->mipMap1Size );
	debugLong( h->mipMap2Size );
	debugLong( h->mipMap3Size );
	debugLong( h->mipMap4Size );
	debugLong( h->mipMap5Size );
	debugLong( h->mipMap6Size );
	debugLong( h->numOffsets );
	debugLong( h->offsetTexData );	//Offset from start of THIS struct
	debugLong( h->mipMap1Off );
	debugLong( h->mipMap2Off );
	debugLong( h->mipMap3Off );
	debugLong( h->mipMap4Off );
	debugLong( h->mipMap5Off );
	debugLong( h->mipMap6Off );
	debugLong( h->q1_tt3 );
}



void debugSH4_MH( sh4_model_header *h )
{
	LogFile( ERROR_LOG, " >===---+---==<[ SH4 M O D E L   H E A D E R  (size %ld) ]>==---+---===<",sizeof(sh4_model_header ) );
	debugLong( h->modelMarker );
	debugLong( h->modelNum );
	debugLong( h->offsetMatSet1 );
	debugLong( h->numMatSet1 );
	debugLong( h->offsetSeqDat1 );
	debugLong( h->numMatSet2 );
	debugLong( h->offsetSeqDat2 );
	debugLong( h->offsetMatSet2 );
	debugLong( h->numVertSects );
	debugLong( h->offsetVertSects );
	debugLong( h->numAltVertSects );
	debugLong( h->offsetAltVertSects );
	debugLong( h->numTexSetIDs );
	debugLong( h->offsetTexSetIDs );
	debugLong( h->numTexIDMapping );
	debugLong( h->offsetTexIDMapping );
	debugLong( h->offsetAltModelHeader );
	debugLong( h->numQuatData );
	debugLong( h->offsetQuatData );
	debugLong( h->numQ5_mh );
	debugLong( h->offsetQ5_mh );
	debugLong( h->f2_mh );
	debugLong( h->offset_f3_mh );
	debugLong( h->offset_f4_mh );
	debugLong( h->offsetExtents );
	debugFloat( h->q6_mh );
	debugLong( h->f5_mh );
	debugLong( h->f6_mh );
	debugLong( h->offset_q7_mh );
	debugLong( h->f7_mh );
	debugLong( h->f8_mh );
	debugLong( h->f9_mh );
}


void debugSH4_VS( sh4_vertex_prim *h )
{
	LogFile( ERROR_LOG, " >==--+--==<[ SH4   V E R T E X   S E C T I O N  (size %ld) ]>==--+--==<",sizeof(sh4_vertex_prim ) );
	debugSH4_VP( &(h->m_cPrimHeader) );
	debugShortArray( h->seqData1, h->m_cPrimHeader.numSeq1Data );
	debugShortArray( h->seqData2, h->m_cPrimHeader.numSeq2Data );
	debugShortArray( h->texIDs, h->m_cPrimHeader.numTexID );
//	debugLong( h->m_lNumVertexData );
//	debugSH4_VD( &(h->m_cVertexData) );
//	long k;
//	for( k = 0; k < 10 && k < h->m_cVertexData.numIndex; k++ )
//		debugShort( h->indicies[ k ] );
//	debugLongArray( h->seqReorder, h->m_cPrimHeader.numSeq1Data + h->m_cPrimHeader.numSeq2Data );
}


void debugSH4_VP( sh4_vertex_prim_header *h )
{
	LogFile( ERROR_LOG, " >=----=<[ SH4   V E R T   H E A D E R  (size %ld) ]>=----=<",sizeof(sh4_vertex_prim_header ) );
	debugLong( h->vertexSectSize );
	debugLong( h->f1_vp );
	debugLong( h->headerSize );
	debugLong( h->q1_vp_modelPartNum );
	debugLong( h->q2_vp_modelPartSelect );	//Seems to be offset added to part# data (maybe L/R side info...)
	debugLong( h->q3_vp_count );
	debugLong( h->q4_vp_offset_q3 );		//Offset for count of q3 data
	debugLong( h->numSeq1Data );			//Count of shorts for matrix selection
	debugLong( h->offsetSeq1Data );
	debugLong( h->numSeq2Data );
	debugLong( h->offsetSeq2Data );
	debugLong( h->q5_vp_lowPart );			//Related to #'s in other vertex primitives, just offset by q2 value
	debugLong( h->q6_vp_highPart );			//So if q2 is 0, this # will be = to another section's val - their q2
	debugLong( h->numTexID );				//I think this is texID count, based on SH3 format
	debugLong( h->offsetTexIDs );			//Offset to shorts containing texture IDs
	debugLong( h->offsetStaticData );		//Offset to 3 weird static #'s like SH3
	debugLong( h->numStaticData );
	debugLong( h->f2_vp );
	debugFloat( h->q7_vp );					//Seem to be similar across primitives
	debugFloat( h->q8_vp );					//Seem to be similar across prims
	debugLong( h->f3_vp );
	debugLong( h->f4_vp );
	debugLong( h->f5_vp );
	debugLong( h->f6_vp );
	debugVertex4f( h->q9_vp );					//All 3 vals seem to be same (except w)
	debugVertex4f( h->q10_vp );					//All 3 vals seem to be same (except w)
	debugVertex4f( h->q11_vp );					//All 3 vals seem to be same (except w)
}


void debugSH4_VDT( sh4_vertex_type_header *h )
{
	debugLongArray( h->staticData, 4 );	//Seems to be same as SH3
	debugLong( h->numVertSet );			//Always 1
	debugLong( h->f1_vd );
	debugLong( h->q2_vd );			//Always 12
}

void debugSH4_VDH( sh4_vertex_data_head *h )
{
	LogFile( ERROR_LOG, "  =--=[ SH4 VERT DATA HEAD(size %ld) ]=--=",sizeof(sh4_vertex_data_head ) );
	debugLongArray( h->fa2_vd, 16 );		//Looks like array of 16 blank floats...
	debugLong( h->numIndex );		//Count of shorts for indicies
	debugLong( h->numVertex );
	debugLong( h->vertexSize );
	debugLong( h->q3_vd );			//Maybe shader #, or tex ID
	debugLong( h->offsetIndexData );//Offset to the index data (after the vertex data)
	debugLong( h->offsetVertData );			//Seems to be 60...Maybe related to default size of 20 for seq ordering data..
	debugLong( h->offsetSeqOrder );	//Offset to array of sequence matrix reordering... (I think this reorders the matrices, or maybe is used by shader to place them in correct position)
}


void debugSH4_VD( sh4_vertex_data *h )
{
	LogFile( ERROR_LOG, " >=----=<[ SH4   V E R T   D A T A  (size %ld) ]>=----=<",sizeof(sh4_vertex_data ) );
	debugSH4_VDH( &h->m_sVertexType );
	debugLong( h->m_lNumSeqReorder );
	debugLongArray( h->seqReorder, h->m_lNumSeqReorder );
}


sh4_model_tex & sh4_model_tex::operator=( const sh4_model_tex & rhs )
{ 
	if( &rhs != this )
	{ 
		deleteData( ); 
		memcpy( (void *)&m_sT1, (void *)&rhs.m_sT1, sizeof( m_sT1 ) ); 
		memcpy( (void *)&m_sT2, (void *)&rhs.m_sT2, sizeof( m_sT2 )); 
	
		m_sT3 = new sh4_model_tex_type3[ m_sT2.numTex ];
		m_puiTexID = new GLuint[ m_sT2.numTex ];

		memcpy( (void *)m_sT3, (void *)rhs.m_sT3, sizeof( sh4_model_tex_type3 ) * m_sT2.numTex ); 
		memcpy( (void *)m_puiTexID, (void *)rhs.m_puiTexID, sizeof( GLuint ) * m_sT2.numTex );
	} 
	return *this; 
}


void sh4_model_tex::saveTexture( sh4_model_tex_type3 *texHeader, BYTE *data, char *filename )
{
	image_t imageDat;

	memset(&imageDat,0,sizeof(imageDat));

	imageDat.info.width			= (short)texHeader->xRes;
	imageDat.info.height		= (short)texHeader->yRes;
	imageDat.info.image_type	= 2;
	imageDat.info.components	= (texHeader->texDataSize/(texHeader->xRes * texHeader->yRes));
	imageDat.info.pixel_depth	= (unsigned char)((texHeader->texDataSize/(texHeader->xRes * texHeader->yRes)) * 8);
	imageDat.info.bytes			= imageDat.info.width * imageDat.info.height * imageDat.info.components;
	LogFile(ERROR_LOG,"saveTexture:\n------------------------------------------------------------------"
		"----------\tTexData: W:%d H:%d  bytes:%d  pixelDepth:%d  components:%d",
		imageDat.info.width,imageDat.info.height,imageDat.info.bytes,imageDat.info.pixel_depth,
		imageDat.info.components);
	imageDat.data = data;
	tgaSave(filename,&imageDat);
}


long sh4_model_tex::loadData( long offsetT1, long offsetT2, FILE *inFile )
{
	long lStartOffset;
	long lTempOffset;
	long lCurOffset;
	long lNumRead;
	long lTotalread = 0;
	GLuint texID;
	long k,j,i;
	char filename[128];

	if( ! inFile )
		return 0;

	deleteData( );

	lStartOffset = ftell( inFile );

	fseek( inFile, lStartOffset + offsetT1, SEEK_SET );

	lNumRead = _loadBlock( (void*)&m_sT1, sizeof(m_sT1), inFile,
						"sh4_model_tex::loadData() - Could not read struct type 1",ERROR_LOG);
	if( lNumRead == -1 )
		return 0;

	debugSH4_MTT1( &m_sT1 );
	lTotalread += lNumRead;

	fseek( inFile, lStartOffset + offsetT2, SEEK_SET );

	lNumRead = _loadBlock( (void*)&m_sT2, sizeof(m_sT2), inFile,
						"sh4_model_tex::loadData() - Could not read struct type 2",ERROR_LOG);
	if( lNumRead == -1 )
		return 0;

	debugSH4_MTT2( &m_sT2 );
	lTotalread += lNumRead;


	m_sT3 = new sh4_model_tex_type3[ m_sT2.numTex ];
	m_puiTexID = new GLuint[ m_sT2.numTex ];

	lCurOffset = lStartOffset + offsetT2 +m_sT2.offsetType3;

	for( k = 0; k < m_sT2.numTex; k++ )
	{
		BYTE *texData;
		fseek( inFile, lCurOffset, SEEK_SET );

		lNumRead = _loadBlock( (void*)&(m_sT3[k]), sizeof(sh4_model_tex_type3), inFile,
							"sh4_model_tex::loadData() - Could not read struct type 3",ERROR_LOG);
		if( lNumRead == -1 )
		{
			LogFile( ERROR_LOG, "sh4_model_tex::loadData( ) - ERROR: Read failed on type3 struct %ld of %ld",k+1, m_sT2.numTex);
			return 0;
		}

		debugSH4_MTT3( &(m_sT3[k]) );
		lTotalread += lNumRead;
		
		lTempOffset = ftell( inFile );
		fseek( inFile, lCurOffset + m_sT3[k].offsetTexData, SEEK_SET );
		lCurOffset = lTempOffset;

		texData = new BYTE[ m_sT3[k].texDataSize ];

		lNumRead = _loadBlock( (void *)texData, m_sT3[k].texDataSize, inFile,
							"sh4_model_tex::loadData() - Could not read image data",ERROR_LOG);
		if( lNumRead == -1 )
		{
			LogFile( ERROR_LOG, "sh4_model_tex::loadData( ) - ERROR: Read failed on image data for tex %ld of %ld",k+1, m_sT2.numTex);
			return 0;
		}

		sprintf( filename, "_%ld.tga", lCurOffset );
		

		


		glGenTextures( 1, &texID ); //&m_puiTexID[ k ] );

		if( texID < 1 ) //m_puiTexID[ k ] < 1 )
		{
			LogFile( ERROR_LOG, "sh4_model_tex::loadData( ) - ERROR: Could not generate a texID for texture at offset %ld",ftell(inFile)-m_sT3[k].offsetTexData);
		}
		else
		{
			m_puiTexID[ k ] = texID;
			LogFile( ERROR_LOG, "CHECK TEX: addr of texID = %ld   texID = %ld for k = %ld   - Actual texID = %ld",this->m_puiTexID,m_puiTexID[k],k, texID);
			glBindTexture ( GL_TEXTURE_2D, m_puiTexID[ k ] );
			LogFile( DATA_LOG, "Check FORMAT: %4.4s",(char*)&m_sT3[k].texFormat );
			if( m_sT3[k].texFormat == 0x31545844 )		//FORMAT = DXT1
			{
				LogFile(ERROR_LOG,"In DXT1");
				unsigned char *pData  = new unsigned char[ m_sT3[k].xRes * m_sT3[k].yRes *4 ];
				sh4_model_tex_type3 tempT3;

				glCompressedTexImage2DARB( GL_TEXTURE_2D, 0, GL_COMPRESSED_RGBA_S3TC_DXT1_EXT, m_sT3[k].xRes, m_sT3[k].yRes, 0, m_sT3[k].texDataSize, texData );

				glGetTexImage( GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, (GLvoid *)pData);
				memcpy( (void *)&tempT3, (void *)&(m_sT3[k]), sizeof( sh4_model_tex_type3 ) );
				tempT3.texDataSize = tempT3.xRes * tempT3.yRes * 4;
				//if( debugMode )saveTexture( &tempT3, pData, filename );
				delete [] pData;
			}
			else if( m_sT3[k].texFormat == 21 )			//FORMAT = UNCOMPRESSED
			{
				LogFile(ERROR_LOG,"In Uncomp");
				//if( debugMode )saveTexture( &(m_sT3[k]), texData, filename );
				for( i = 0; i < m_sT3[k].yRes; i++ )
				{
					for( j = 0; j < m_sT3[k].xRes; j++ )
					{
						BYTE tempB = texData[ i * m_sT3[k].xRes * 4 + j * 4 ];
						texData[ i * m_sT3[k].xRes * 4 + j * 4 ] = texData[ i * m_sT3[k].xRes * 4 + j * 4 + 2 ];
						texData[ i * m_sT3[k].xRes * 4 + j * 4 + 2 ] = tempB;
					}
				}
				glTexImage2D ( GL_TEXTURE_2D, 0, GL_RGBA, m_sT3[k].xRes, m_sT3[k].yRes, 0, GL_RGBA, GL_UNSIGNED_BYTE, texData );  
			}
			else if( m_sT3[k].texFormat == 0x33545844 )		//FORMAT = DXT3
			{
				LogFile(ERROR_LOG,"In DXT3");
				unsigned char *pData  = new unsigned char[ m_sT3[k].xRes * m_sT3[k].yRes *4 ];
				sh4_model_tex_type3 tempT3;

				glCompressedTexImage2DARB( GL_TEXTURE_2D, 0, GL_COMPRESSED_RGBA_S3TC_DXT3_EXT, m_sT3[k].xRes, m_sT3[k].yRes, 0, m_sT3[k].texDataSize, texData );

				glGetTexImage( GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, (GLvoid *)pData);
				memcpy( (void *)&tempT3, (void *)&(m_sT3[k]), sizeof( sh4_model_tex_type3 ) );
				tempT3.texDataSize = tempT3.xRes * tempT3.yRes * 4;
				//if( debugMode )saveTexture( &tempT3, pData, filename );
				delete [] pData;
			}
			else
			{
				LogFile( ERROR_LOG, "sh4_texture::loadTex( ) - ERROR: Unsupported Texture Format: 0x%08x - %4.4s", m_sT3[k].texFormat,(char *)(&m_sT3[k].texFormat));
				debugAll( m_sT3[k].texFormat );
			}

			glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
			glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );

			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
			glTexEnvi ( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE,GL_REPLACE);
		}
		delete [] texData;
	}

	return lTotalread;
}



sh4_quat_assembly & sh4_quat_assembly::operator=( const sh4_quat_assembly & rhs )
{
	if( &rhs != this )
	{
		deleteData( );

		dataCount	=	rhs.dataCount;
		buildData	=	new long[ dataCount * 2 ];
		memcpy( (void *)&buildData, (void *)&rhs.buildData, sizeof(long) * dataCount * 2 );
	}
	return *this;
}


void sh4_quat_assembly::setNullAll( )
{
	dataCount = -1;
	buildData = NULL;
}


long sh4_quat_assembly::loadData( long startOffset, FILE *inFile )
{
	long	lNumRead;
	long	lTotalread	= 0;
	long	lTempOffset;

	if( inFile == 0 )
		return 0;
//LogFile(ERROR_LOG,"sh4_quat_assembly::loadData - 1 ");
	deleteData( );

	if( ( lNumRead = fread( (void *)&dataCount, 1, sizeof(long), inFile ) ) != sizeof( long ) )
	{
		LogFile( ERROR_LOG, "sh4_quat_assembly::loadData() - Could not read count of data (%ld of %ld read) - REASON: %s",
			lNumRead, sizeof(long), strerror( errno ) );
		return 0;
	}
//LogFile(ERROR_LOG,"sh4_quat_assembly::loadData - 2 ");
	lTotalread = lNumRead;

	if( ( lNumRead = fread( (void *)&lTempOffset, 1, sizeof(long), inFile ) ) != sizeof( long ) )
	{
		LogFile( ERROR_LOG, "sh4_quat_assembly::loadData() - Could not read base offset (%ld of %ld read) - REASON: %s",
			lNumRead, sizeof(long), strerror( errno ) );
		return 0;
	}

	lTotalread += lNumRead;

	fseek( inFile, lTempOffset + startOffset, SEEK_SET );

	buildData = new long [ dataCount * 2 ];
//LogFile(ERROR_LOG,"sh4_quat_assembly::loadData - 3");
	if( ( lNumRead = fread( (void *)buildData, 1, sizeof(long) * dataCount * 2, inFile ) ) != sizeof( long ) * dataCount * 2 )
	{
		LogFile( ERROR_LOG, "sh4_quat_assembly::loadData() - Could not read build data (%ld of %ld read) - REASON: %s",
			lNumRead, sizeof(long) * dataCount * 2, strerror( errno ) );
		return 0;
	}

	lTotalread += lNumRead;
//LogFile(ERROR_LOG,"sh4_quat_assembly::loadData - 4");
	return lTotalread;
}





void sh4_quat_assembly::deleteData( )
{
	if( dataCount != -1 && buildData )
		delete [] buildData;
	setNullAll();
}


void sh4_model_header_dynamic::copyData( const sh4_model_header_dynamic &rhs, const sh4_model_header *header )
{
	int k;
	if( !header )
		return;
	
	if( &rhs == this )
		return;

	deleteData( );
	
	matSet1 = new matrix[ header->numMatSet1 ];
	matSet2 = new matrix[ header->numMatSet2 ];
	mSeq1 = new unsigned char[ header->numMatSet1 ];
	mSeq2 = new unsigned char[ header->numMatSet2 * 2 ];	//Each set is 2 BYTES
	texIDs = new long[ header->numTexSetIDs ];
	texIDMapping = new long[ header->numTexIDMapping * 2 ];		//Each element is 8 bytes
	quatData = new short[ header->numQuatData * 3 ];		//Each element is 3 shorts
	quatBuild = new sh4_quat_assembly[ header->numQ5_mh ];

	memcpy( (void *)&matSet1, (void *)&rhs.matSet1, sizeof( matrix ) * header->numMatSet1 );
	memcpy( (void *)&matSet2, (void *)&rhs.matSet2, sizeof( matrix ) * header->numMatSet2 );
	memcpy( (void *)&mSeq1, (void *)&rhs.mSeq1, sizeof( unsigned char ) * header->numMatSet1 );
	memcpy( (void *)&mSeq2, (void *)&rhs.mSeq2, sizeof( unsigned char ) * header->numMatSet2 * 2 );
	memcpy( (void *)&texIDs, (void *)&rhs.texIDs, sizeof( long ) * header->numTexSetIDs );
	memcpy( (void *)&texIDMapping, (void *)&rhs.texIDMapping, sizeof( long ) * header->numTexIDMapping * 2 );
	memcpy( (void *)&quatData, (void *)&rhs.quatData, sizeof( short ) * header->numQuatData * 3 );

	for( k = 0; k < header->numQ5_mh; k++ )
		quatBuild[ k ] = rhs.quatBuild[ k ];
	for( k = 0; k < 8; k++ )
		extents[ k ] = rhs.extents[ k ];
	for( k = 0; k < 4; k++ )
		qa1[ k ] = rhs.qa1[ k ];
	q2 = rhs.q2;
}

		
void sh4_model_header_dynamic::setNullAll( )
{
	matSet1		=	NULL;
	matSet2		=	NULL;
	mSeq1		=	NULL;
	mSeq2		=	NULL;
	texIDs		=	NULL;
	texIDMapping=	NULL;
	quatData	=	NULL;
	quatBuild	=	NULL;
}

long sh4_model_header_dynamic::loadData( long baseOffset, sh4_model_header *loadDat, FILE *inFile )
{
	long	lNumRead;
	long	lTotalread	= 0;
	long	lTempOffset;
	long	k;

	if( !inFile || !loadDat )
		return 0;

	deleteData( );
	
	fseek( inFile, baseOffset + loadDat->offsetMatSet1, SEEK_SET );

	matSet1 = new matrix[ loadDat->numMatSet1 ];

	lNumRead = _loadBlock( (void*)matSet1, sizeof(matrix) * loadDat->numMatSet1, inFile,
						"sh4_model_header_dynamic::loadData() - Could not read first matrix set",ERROR_LOG);
	if( lNumRead == 0 )
		return 0;

	for( k = 0; k < loadDat->numMatSet1; k++ )
		matSet1[ k ].transpose( );

	lTotalread = lNumRead;

	fseek( inFile, baseOffset + loadDat->offsetMatSet2, SEEK_SET );

	matSet2 = new matrix[ loadDat->numMatSet2 ];

	lNumRead = _loadBlock( (void*)matSet2, sizeof(matrix) * loadDat->numMatSet2, inFile,
						"sh4_model_header_dynamic::loadData() - Could not read 2nd matrix set",ERROR_LOG);
	if( lNumRead == -1 )
		return 0;

	for( k = 0; k < loadDat->numMatSet2; k++ )
		matSet2[ k ].transpose( );

	lTotalread += lNumRead;

	fseek( inFile, baseOffset + loadDat->offsetSeqDat1, SEEK_SET );

	mSeq1 = new unsigned char[ loadDat->numMatSet1 ];

	lNumRead = _loadBlock( (void*)mSeq1, sizeof(unsigned char) * loadDat->numMatSet1, inFile,
						"sh4_model_header_dynamic::loadData() - Could not read 1st Sequence set",ERROR_LOG);
	if( lNumRead == -1 )
		return 0;

	lTotalread += lNumRead;


	
	fseek( inFile, baseOffset + loadDat->offsetSeqDat2, SEEK_SET );

	mSeq2 = new unsigned char[ loadDat->numMatSet2 * 2 ];

	lNumRead = _loadBlock( (void*)mSeq2, sizeof(unsigned char) * loadDat->numMatSet2 * 2, inFile,
						"sh4_model_header_dynamic::loadData() - Could not read 2nd Sequence set",ERROR_LOG);
	if( lNumRead == -1 )
		return 0;

	lTotalread += lNumRead;


	fseek( inFile, baseOffset + loadDat->offsetTexSetIDs, SEEK_SET );

	texIDs = new long[ loadDat->numTexSetIDs ];

	lNumRead = _loadBlock( (void*)texIDs, sizeof(long) * loadDat->numTexSetIDs, inFile,
						"sh4_model_header_dynamic::loadData() - Could not read tex IDs",ERROR_LOG);
	if( lNumRead == -1 )
		return 0;

	lTotalread += lNumRead;

	fseek( inFile, baseOffset + loadDat->offsetTexIDMapping, SEEK_SET );

	texIDMapping = new long[ loadDat->numTexIDMapping * 2 ];		//Each element is 8 bytes

	lNumRead = _loadBlock( (void*)texIDMapping, sizeof( long ) * loadDat->numTexIDMapping * 2 , inFile,
						"sh4_model_header_dynamic::loadData() - Could not read Q3 Array",ERROR_LOG);
	if( lNumRead == -1 )
		return 0;

	lTotalread += lNumRead;

	
	fseek( inFile, baseOffset + loadDat->offsetQuatData, SEEK_SET );

	quatData = new short[ loadDat->numQuatData * 3 ];		//Each element is 3 shorts

	lNumRead = _loadBlock( (void*)quatData, sizeof( short ) * loadDat->numQuatData * 3 , inFile,
						"sh4_model_header_dynamic::loadData() - Could not read quat data",ERROR_LOG);
	if( lNumRead == -1 )
		return 0;

	lTotalread += lNumRead;


	quatBuild = new sh4_quat_assembly[ loadDat->numQ5_mh ];

	for( k = 0; k < loadDat->numQ5_mh; k++ )
	{
		fseek( inFile, baseOffset + loadDat->offsetQ5_mh + k * sizeof( long ) * 2, SEEK_SET );

		if( (lNumRead = quatBuild[ k ].loadData( baseOffset, inFile )) == 0 )
		{
			LogFile( ERROR_LOG, "sh4_model_header_dynamic::loadData() - Couldn't read the quat assembly data - %ld of %ld", k+1, loadDat->numQ5_mh);
			deleteData( );
			return 0;
		}

		lTotalread += lNumRead;
	}


	fseek( inFile, baseOffset + loadDat->offsetExtents, SEEK_SET );

	lNumRead = _loadBlock( (void*)extents, sizeof( vertex4f ) * 8 , inFile,
						"sh4_model_header_dynamic::loadData() - Could not read extents",ERROR_LOG);
	if( lNumRead == -1 )
		return 0;

	lTotalread += lNumRead;


	fseek( inFile, baseOffset + loadDat->offset_q7_mh, SEEK_SET );

	lNumRead = _loadBlock( (void*)&lTempOffset, sizeof( long ) , inFile,
						"sh4_model_header_dynamic::loadData() - Could not read q7_mh offset",ERROR_LOG);
	if( lNumRead == -1 )
		return 0;

	lTotalread += lNumRead;
//LogFile(ERROR_LOG,"sh4_model_header_dynamic::loadData - 1 ");
	if( lTempOffset )
	{
		fseek( inFile, baseOffset + lTempOffset, SEEK_SET );

		lNumRead = _loadBlock( (void*)qa1, sizeof( short ) * 4 , inFile,
							"sh4_model_header_dynamic::loadData() - Could not read qa1",ERROR_LOG);
		if( lNumRead == -1 )
			return 0;

		lTotalread += lNumRead;

		lNumRead = _loadBlock( (void*)&q2, sizeof( matrix ) , inFile,
							"sh4_model_header_dynamic::loadData() - Could not read suspected affine",ERROR_LOG);
		if( lNumRead == -1 )
			return 0;

		lTotalread += lNumRead;
	}


//LogFile(ERROR_LOG,"sh4_model_header_dynamic::loadData - 2");
	return lNumRead;
}


void sh4_model_header_dynamic::deleteData( )
{
	delete [] matSet1;
	delete [] matSet2;
	delete [] mSeq1;
	delete [] mSeq2;
	delete [] texIDs;
	delete [] texIDMapping;
	delete [] quatData;
	delete [] quatBuild;
	setNullAll();
}



sh4_vertex_data & sh4_vertex_data::operator=( const sh4_vertex_data & rhs )
{
	if( &rhs != this )
	{
		long k;
		deleteData( );

		m_lStartOffset = rhs.m_lStartOffset;
		m_bHeaderLoaded = rhs.m_bHeaderLoaded;
		m_lNumSeqReorder = rhs.m_lNumSeqReorder;

		memcpy( (void *)&m_sVertexType, (void *)&rhs.m_sVertexType, sizeof( m_sVertexType ) );
		verts = new sh4_model_vert[ m_sVertexType.numVertex ];
		indicies = new short[ m_sVertexType.numIndex ];
		seqReorder = new long[ m_lNumSeqReorder ];
		
		memcpy( (void *)indicies, (void *)rhs.indicies, m_sVertexType.numIndex * sizeof( short ) );
		memcpy( (void *)seqReorder, (void *)rhs.seqReorder, m_lNumSeqReorder * sizeof( long ) );

		for( k = 0; k < m_sVertexType.numVertex; k++ )
			verts[ k ] = rhs.verts[ k ];

	}
	return *this;
}

void sh4_vertex_data::setNullAll( )
{
	verts = NULL;
	indicies = NULL;
	seqReorder = NULL;
	m_bHeaderLoaded = false;
	m_lNumSeqReorder = 0;
}

void sh4_vertex_data::deleteData( )
{
	delete [] verts;
	delete [] indicies;
	delete [] seqReorder;
	setNullAll( );
}


long sh4_vertex_data::loadDataHeader( FILE *inFile )
{
	long lNumRead;

	if( inFile == NULL )
		return 0;

	deleteData( );

	m_lStartOffset = ftell( inFile );

	lNumRead = _loadBlock( (void*)&m_sVertexType, sizeof(m_sVertexType), inFile,
						"sh4_vertex_data::loadDataHeader() - Could not read vertex header data",ERROR_LOG);
	if( lNumRead == -1 )
		return 0;
	//LogFile(ERROR_LOG,"sh4_vertex_prim::loadData - 6");
	//if( debugMode )debugSH4_VDH( &m_sVertexType );
	m_bHeaderLoaded = true;

	return lNumRead;
}



long sh4_vertex_data::loadDataVerts( FILE *inFile )
{
	long lNumRead;
	long lTotalread = 0;

	if( inFile == NULL || m_bHeaderLoaded == false )
		return 0;
	fseek( inFile, m_lStartOffset + m_sVertexType.offsetVertData + 32, SEEK_SET );

	verts = new sh4_model_vert[ m_sVertexType.numVertex ];
	
	lNumRead = _loadBlock( (void*)verts, sizeof(sh4_model_vert) * m_sVertexType.numVertex, inFile,
						"sh4_vertex_data::loadDataHeader() - Could not read vertex data",ERROR_LOG);
	if( lNumRead == -1 )
		return 0;
//LogFile( DATA_LOG,"CHECK: %ld total verts, %ld total indicies",m_sVertexType.numVertex,m_sVertexType.numIndex);
//long k;
//for( k = 0; k < 10 && k < m_sVertexType.numVertex; k++ )
//	LogFile(DATA_LOG,"\tpos: %f %f %f\tnorm: %f %f %f\ttc: %f %f\tmats: %d %d %d %d\tweights: %f %f %f %f",
//		verts[k].pos.x,verts[k].pos.y,verts[k].pos.z,verts[k].normal.x,verts[k].normal.y,verts[k].normal.z,
//		verts[k].tc.s,verts[k].tc.t,int((verts[k].shaderSelect.x - 34.0f)/3.0f),int((verts[k].shaderSelect.y - 34.0f)/3.0f),
//		int((verts[k].shaderSelect.z - 34.0f)/3.0f),int((verts[k].shaderSelect.w - 34.0f)/3.0f),
//		verts[k].weights.x,verts[k].weights.y,verts[k].weights.z,verts[k].weights.w);
	lTotalread += lNumRead;

	fseek( inFile, m_lStartOffset + m_sVertexType.offsetIndexData, SEEK_SET );
	//LogFile(ERROR_LOG,"sh4_vertex_prim::loadData - 7");
	indicies = new short[ m_sVertexType.numIndex ];
	lNumRead = _loadBlock( (void*)indicies, sizeof(short) * m_sVertexType.numIndex, inFile,
						"sh4_vertex_data::loadDataHeader() - Could not read index data",ERROR_LOG);
	if( lNumRead == -1 )
		return 0;

	lTotalread += lNumRead;

	return lTotalread;
}



sh4_vertex_prim & sh4_vertex_prim::operator=( const sh4_vertex_prim & rhs )
{
	if( &rhs != this )
	{
		long k;
		deleteData( );
		memcpy( (void *)&m_cPrimHeader, (void *)&rhs.m_cPrimHeader, sizeof( rhs.m_cPrimHeader ) );
		m_lNumVertexData = rhs.m_lNumVertexData;
	
		m_cVertexData = new sh4_vertex_data[ m_lNumVertexData ];

		for( k = 0; k < m_lNumVertexData; k++ )
			m_cVertexData[ k ] = rhs.m_cVertexData[ k ];

	//	memcpy( (void *)&m_cVertexData, (void *)&rhs.m_cVertexData, sizeof( rhs.m_cVertexData ) );

		seqData1 = new short[ m_cPrimHeader.numSeq1Data ];
		seqData2 = new short[ m_cPrimHeader.numSeq2Data ];
		texIDs = new short[ m_cPrimHeader.numTexID ];
	//	seqReorder = new long[ m_cPrimHeader.numSeq1Data + m_cPrimHeader.numSeq2Data ];

		memcpy( (void *)seqData1, (void *)rhs.seqData1, m_cPrimHeader.numSeq1Data * sizeof( short ) );
		memcpy( (void *)seqData2, (void *)rhs.seqData2, m_cPrimHeader.numSeq2Data * sizeof( short ) );
		memcpy( (void *)texIDs, (void *)rhs.texIDs, m_cPrimHeader.numTexID * sizeof( short ) );
	//	memcpy( (void *)seqReorder, (void *)rhs.seqReorder, (m_cPrimHeader.numSeq1Data + m_cPrimHeader.numSeq2Data) * sizeof( long ) );
	}
	return *this;
}

void sh4_vertex_prim::setNullAll( )
{
	seqData1 = NULL;
	seqData2 = NULL;
	texIDs = NULL;
//	seqReorder = NULL;
	m_cVertexData = NULL;
	m_lNumVertexData = 0;
}

void sh4_vertex_prim::deleteData( )
{
	delete [] seqData1;
	delete [] seqData2;
	delete [] texIDs;
//	delete [] seqReorder;
	delete [] m_cVertexData;
	setNullAll( );
}

long sh4_vertex_prim::loadData( FILE *inFile )
{
	long lNumRead;
	long lTotalread;
	long lTempOffset;
	long lStartOffset;
	sh4_vertex_type_header	l_cVertexType;
	long k;

	if( !inFile )
		return 0;

	lStartOffset  = ftell( inFile );
LogFile(ERROR_LOG,"sh4_vertex_prim::loadData - 1");
	deleteData( );

	lNumRead = _loadBlock( (void*)&m_cPrimHeader, sizeof(sh4_vertex_prim_header) , inFile,
						"sh4_vertex_prim::loadData() - Could not read static prim head",ERROR_LOG);
	if( lNumRead == -1 )
		return 0;
debugSH4_VP( &m_cPrimHeader );
	lTotalread = lNumRead;
LogFile(ERROR_LOG,"sh4_vertex_prim::loadData - 2");
	fseek( inFile, lStartOffset + m_cPrimHeader.offsetSeq1Data, SEEK_SET );

	seqData1 = new short[ m_cPrimHeader.numSeq1Data ];
	lNumRead = _loadBlock( (void*)seqData1, sizeof(short) * m_cPrimHeader.numSeq1Data, inFile,
						"sh4_vertex_prim::loadData() - Could not read seq1 data",ERROR_LOG);
	if( lNumRead == -1 )
		return 0;

	lTotalread += lNumRead;
LogFile(ERROR_LOG,"sh4_vertex_prim::loadData - 3");
	fseek( inFile, lStartOffset + m_cPrimHeader.offsetSeq2Data, SEEK_SET );

	seqData2 = new short[ m_cPrimHeader.numSeq2Data ];
	lNumRead = _loadBlock( (void*)seqData2, sizeof(short) * m_cPrimHeader.numSeq2Data, inFile,
						"sh4_vertex_prim::loadData() - Could not read seq2 data",ERROR_LOG);
	if( lNumRead == -1 )
		return 0;

	lTotalread += lNumRead;

	fseek( inFile, lStartOffset + m_cPrimHeader.offsetTexIDs, SEEK_SET );
LogFile(ERROR_LOG,"sh4_vertex_prim::loadData - 4");	
	texIDs = new short[ m_cPrimHeader.numTexID ];
	lNumRead = _loadBlock( (void*)texIDs, sizeof(short) * m_cPrimHeader.numTexID, inFile,
						"sh4_vertex_prim::loadData() - Could not read texID data",ERROR_LOG);
	if( lNumRead == -1 )
		return 0;

	lTotalread += lNumRead;
LogFile(ERROR_LOG,"sh4_vertex_prim::loadData - 5");
	if( debugMode )debugSH4_VS( this );
	fseek( inFile, lStartOffset + m_cPrimHeader.offsetStaticData, SEEK_SET );

	lNumRead = _loadBlock( (void*)&l_cVertexType, sizeof(l_cVertexType), inFile,
						"sh4_vertex_prim::loadData() - Could not read vertex type header data",ERROR_LOG);
	if( lNumRead == -1 )
		return 0;

	if( debugMode )debugSH4_VDT( &l_cVertexType );
LogFile(ERROR_LOG,"sh4_vertex_prim::loadData - 6");
	lTotalread += lNumRead;

	m_lNumVertexData = l_cVertexType.numVertSet;

		//--[ Get Offset to get index/reorder data from ]--/
	

	m_cVertexData = new sh4_vertex_data[ m_lNumVertexData ];
//###################################################################
	for( k = 0; k < m_lNumVertexData; k++ )
	{
		lTempOffset = ftell( inFile );
LogFile(ERROR_LOG,"Reading %ld vert header of %ld at offset %ld",k+1,m_lNumVertexData,ftell(inFile ));
		if( (lNumRead = m_cVertexData[ k ].loadDataHeader( inFile )) == 0 )
		{
			LogFile( ERROR_LOG, "sh4_vertex_prim::loadData( ) - ERROR: Could not read vertex data at offset %ld",lTempOffset);
			deleteData( );
			return 0;
		}
		
		lTotalread += lNumRead;
	}

	for( k = 0; k < m_lNumVertexData; k ++ )
	{
		long *tempReorder = NULL;
		long tempNumReorder;
		LogFile(ERROR_LOG,"Reading %ld vert data of %ld at offset %ld (cur offset if %ld)",k+1,m_lNumVertexData,m_cVertexData[k].m_lStartOffset+m_cVertexData[k].m_sVertexType.offsetVertData+32,ftell(inFile ));
		if( (lNumRead = m_cVertexData[ k ].loadDataVerts( inFile ) ) == 0 && m_cVertexData[ k ].m_sVertexType.numVertex != 0 )
		{
			LogFile( ERROR_LOG,"sh4_vertex_prim::loadData( ) - ERROR: Could not read vertex data for vert set %ld",k+1);
			deleteData( );
			return 0;
		}
		lTotalread += lNumRead;

		if( k + 1 == m_lNumVertexData )
		{
			long j = 0;
			tempNumReorder = (m_cPrimHeader.vertexSectSize + lStartOffset - m_cVertexData[ k ].m_lStartOffset - m_cVertexData[ k ].m_sVertexType.offsetSeqOrder) / 4;
LogFile( ERROR_LOG,"CHECK: The size of the seq reorder for %ld of %ld is %ld",k+1, m_lNumVertexData, tempNumReorder);

			tempReorder = new long[ tempNumReorder ];

			fseek( inFile, m_cVertexData[ k ].m_lStartOffset + m_cVertexData[ k ].m_sVertexType.offsetSeqOrder, SEEK_SET );

			lNumRead = _loadBlock( (void*)tempReorder, sizeof(long) * tempNumReorder, inFile,
							"sh4_vertex_prim::loadData() - Could not read reorder data",ERROR_LOG);
			if( lNumRead == -1 )
				return 0;

			lTotalread += lNumRead;
	
			while( j < tempNumReorder && tempReorder[ j ] != 0xffffffff )
				++j;
LogFile( ERROR_LOG,"CHECK: The size of the actual seq reorder for %ld of %ld is %ld",k+1, m_lNumVertexData, j);
			m_cVertexData[ k ].m_lNumSeqReorder = j;
			m_cVertexData[ k ].seqReorder = new long[ j ];
			memcpy( (void *)m_cVertexData[ k ].seqReorder, tempReorder, sizeof( long ) * j );
			delete [] tempReorder;
		}
		else
		{
			tempNumReorder = (m_cVertexData[ k+1 ].m_sVertexType.offsetSeqOrder + sizeof( m_cVertexData[ k ].m_sVertexType ) - m_cVertexData[ k ].m_sVertexType.offsetSeqOrder ) / 4;
LogFile( ERROR_LOG,"CHECK: The size of the seq reorder for %ld of %ld is %ld",k+1, m_lNumVertexData, tempNumReorder);

			m_cVertexData[ k ].seqReorder = new long[ tempNumReorder ];
			m_cVertexData[ k ].m_lNumSeqReorder = tempNumReorder;

			fseek( inFile, m_cVertexData[ k ].m_lStartOffset + m_cVertexData[ k ].m_sVertexType.offsetSeqOrder, SEEK_SET );

			lNumRead = _loadBlock( (void*)m_cVertexData[ k ].seqReorder, sizeof(long) * tempNumReorder, inFile,
							"sh4_vertex_prim::loadData() - Could not read reorder data",ERROR_LOG);
			if( lNumRead == -1 )
				return 0;

			lTotalread += lNumRead;
		}
		if( debugMode )debugSH4_VD( &m_cVertexData[ k ] );
	}

	
	//fseek( inFile, m_cVertexData[ 0 ].m_lStartOffset + m_cVertexData[ 0 ].m_sVertexType.offsetSeqOrder, SEEK_SET );
LogFile(ERROR_LOG,"sh4_vertex_prim::loadData - 8");
	//seqReorder = new long[ m_cPrimHeader.numSeq1Data + m_cPrimHeader.numSeq2Data ];
	//lNumRead = _loadBlock( (void*)seqReorder, sizeof(long) * (m_cPrimHeader.numSeq1Data + m_cPrimHeader.numSeq2Data), inFile,
	//					"sh4_vertex_prim::loadData() - Could not read reorder data",ERROR_LOG);
	//if( lNumRead == -1 )
	//	return 0;

	//lTotalread += lNumRead;
	

	fseek( inFile, lStartOffset + m_cPrimHeader.vertexSectSize, SEEK_SET );
LogFile(ERROR_LOG,"sh4_vertex_prim::loadData - 9");
	return lTotalread;
}

void sh4_vertex_prim::Render( )
{

}




sh4_model & sh4_model::operator=( const sh4_model & rhs )
{
	if( & rhs != this )
	{
		long k;
		deleteData( );
		memcpy( (void *)&m_sModelHeader, (void *)&rhs.m_sModelHeader, sizeof( m_sModelHeader ) );
		m_cHeaderData.copyData( rhs.m_cHeaderData, &rhs.m_sModelHeader );
		
		for( k = 0; k < m_sModelHeader.numVertSects; k++ )
			m_cVertPrim[ k ] = rhs.m_cVertPrim[ k ];

		m_lNumTex = rhs.m_lNumTex;
		texIDs = new GLuint[ m_lNumTex ];
		for( k = 0; k < m_lNumTex; k++ )
			texIDs[ k ] = rhs.texIDs[ k ];
		m_lNumTexData = rhs.m_lNumTexData;
		m_cTextures = new sh4_model_tex[ m_lNumTexData ];
		for( k = 0; k < m_lNumTexData; k ++ )
			m_cTextures[ k ] = rhs.m_cTextures[ k ];

		if( rhs.m_pcAnimMat )
		{
			m_pcAnimMat = new matrix[ m_sModelHeader.numMatSet1 ];
			memcpy( (void *)m_pcAnimMat, (void *)rhs.m_pcAnimMat, sizeof( matrix ) * m_sModelHeader.numMatSet1 );
		}

		m_bHasAnim = rhs.m_bHasAnim;
		m_pcAnimDat = rhs.m_pcAnimDat;
	}
	return *this;
}


void sh4_model::setNullAll( )
{
	m_cVertPrim	=	NULL;
	m_cTextures =	NULL;
	texIDs		=	NULL;
	m_pcAnimMat	=	NULL;
	m_bHasAnim	=	false;
	m_pcAnimDat	=	NULL;
}

void sh4_model::deleteData( )
{
	delete [] m_cVertPrim;
	if( texIDs )
		glDeleteTextures( m_lNumTex, texIDs );
	delete [] texIDs;
	delete [] m_cTextures;
	delete [] m_pcAnimMat;
	setNullAll( );
}


long sh4_model::loadDataAux( FILE *inFile )
{
	long lNumRead;
	long lTotalread;
	long lBaseOffset = ftell( inFile );
	long k;

	if( inFile == NULL )
		return 0;

	deleteData( );

	lNumRead = _loadBlock( (void*)&m_sModelHeader, sizeof(m_sModelHeader) , inFile,
						"sh4_model::loadDataAux() - Could not read base model head",ERROR_LOG);
	if( lNumRead == -1 )
		return 0;

	if( debugMode )debugSH4_MH( &m_sModelHeader );

	lTotalread = lNumRead;

	if( (lNumRead = m_cHeaderData.loadData( lBaseOffset, &m_sModelHeader, inFile )) == 0 )
	{
		LogFile( ERROR_LOG, "sh4_model::loadDataAux( ) - Could not load model dynamic header");
		return 0;
	}

	if( debugMode )debugSH4_MD( &m_cHeaderData, &m_sModelHeader );

	lTotalread += lNumRead;

	m_cVertPrim = new sh4_vertex_prim[ m_sModelHeader.numVertSects ];

	fseek( inFile, lBaseOffset + m_sModelHeader.offsetVertSects, SEEK_SET );

	for( k = 0; k < m_sModelHeader.numVertSects && lTotalread < m_sModelHeader.offsetAltVertSects; k++ )
	{
//LogFile(ERROR_LOG,"sh4_model::loadDataAux -1.1.%ld Vert sect %ld of %ld ",k+1,k+1, m_sModelHeader.numVertSects);
		if( debugMode )LogFile( ERROR_LOG, "\n-------------------- P R I M I T I V E   %ld   O F   %ld  --------------------\n\n(off %ld )",k+1,m_sModelHeader.numVertSects,ftell(inFile));

		if( (lNumRead = m_cVertPrim[ k ].loadData( inFile )) == 0 )
		{
//LogFile(ERROR_LOG,"sh4_model::loadDataAux -1.2.%ld",k+1);
			LogFile( ERROR_LOG, "sh4_model::loadDataAux( ) - ERROR: Couldn't load prim %ld of %ld at offset %ld",k,m_sModelHeader.numVertSects, ftell( inFile ) );
			return 0;
		}

		//if( debugMode )debugSH4_VP( &(m_cVertPrim[ k ].m_cPrimHeader) );
		lTotalread += lNumRead;
	}
LogFile(ERROR_LOG,"sh4_model::loadDataAux - 3");
	if( k < m_sModelHeader.numVertSects && lTotalread > m_sModelHeader.offsetAltVertSects )
	{
		LogFile( ERROR_LOG, "sh4_model::loadDataAux( ) - ERROR: Read a total of %ld bytes, but expected %ld bytes at prim %ld of %ld",
				k, m_sModelHeader.numVertSects, lTotalread, m_sModelHeader.offsetAltVertSects );
	}

	return lTotalread;

}


long sh4_model::loadData( char *filename )
{
	long numOffsets;
	long *offsets;
	long curOffset = -1;
	FILE *inFile;
	long sectType;
	short *sDat = (short *)(& sectType);
	long lNumRead;
	long lTotalread;

	if( filename == NULL )
		return 0;
LogFile( DATA_LOG, "\nFilename = %s",filename);
	if( ( inFile = fopen( filename, "rb" ) ) == NULL )
	{
		LogFile( ERROR_LOG, "sh4_model::loadData( %s ) - ERROR: Couldn't open file - REASON: %s",filename,strerror( errno ) );
		return 0;
	}

	LogFile( ERROR_LOG, "sh4_model::loadData( %s ) - Loading file...",filename);

	lNumRead = _loadBlock( (void*)&numOffsets, sizeof(long), inFile,

		"sh4_model::loadData() - Could not read num offsets",ERROR_LOG);
	if( lNumRead == -1 )
	{
		fclose( inFile );
		return 0;
	}
LogFile( ERROR_LOG, "sh4_model::loadData 1");
	offsets = new long[ numOffsets ];

	lNumRead = _loadBlock( (void*)offsets, sizeof(long) * numOffsets, inFile,
						"sh4_model::loadData() - Could not read offsets",ERROR_LOG);
	if( lNumRead == -1 )
	{
		delete [] offsets;
		fclose( inFile );
		return 0;
	}
LogFile( ERROR_LOG, "--sh4_model-------------------[ Total Offsets: %ld ]--",numOffsets);
	do
	{
		++curOffset;
		fseek( inFile, offsets[ curOffset ], SEEK_SET );

		if( fread( (void *)&sectType, 1, sizeof( long ), inFile ) != sizeof( long ) )
		{
			LogFile( ERROR_LOG, "sh4_model::loadData( ) - ERROR: Couldn't read section type for section %ld of %ld - REASON: %s",
				curOffset, numOffsets, strerror( errno ) );
			delete [] offsets;
			fclose( inFile );
			deleteData( );
			return 0;
		}

		LogFile( ERROR_LOG, "\tSect %ld\tOff: %ld\tType: 0x%08x\t\ttex detect: [ %d %d ]",curOffset, offsets[curOffset],sectType, sDat[0], sDat[1] );
		
	}
	while( curOffset + 1 < numOffsets );	//&& sectType != 0xffff0003 );

	//curOffset = numOffsets;
	curOffset = -1;
	do
	{
		//--curOffset;
		++curOffset;
		fseek( inFile, offsets[ curOffset ], SEEK_SET );

		if( fread( (void *)&sectType, 1, sizeof( long ), inFile ) != sizeof( long ) )
		{
			LogFile( ERROR_LOG, "sh4_model::loadData( ) - ERROR: Couldn't read section type for section %ld of %ld - REASON: %s",
				curOffset, numOffsets, strerror( errno ) );
			delete [] offsets;
			fclose( inFile );
			deleteData( );
			return 0;
		}

		LogFile( ERROR_LOG, "\tSect %ld\tOff: %ld\tType: 0x%08x\t\ttex detect: [ %d %d ]",curOffset, offsets[curOffset],sectType, sDat[0], sDat[1] );
		
	}
	//while( curOffset > 0 && sectType != 0xffff0003 );
	while( curOffset < numOffsets && sectType != 0xffff0003 );
LogFile( ERROR_LOG, "sh4_model::loadData 3");

	if( curOffset >= numOffsets )
	{
		LogFile( ERROR_LOG, "sh4_model::loadData( %s ) - ERROR: Couldn't find a model file - No section had type 0xffff0003",filename );
		return 0;
	}

	fseek( inFile, offsets[ curOffset ], SEEK_SET );
	if( (lTotalread = loadDataAux( inFile )) == 0 )
	{
		LogFile( ERROR_LOG, "sh4_model::loadData( %s ) - ERROR: Couldn't load model at offset %ld - Offset Index %ld",filename, offsets[curOffset], curOffset );
		delete [] offsets;
		fclose( inFile );
		deleteData( );
		return 0;
	}

	//debugSH4_MH( &m_sModelHeader );
	//debugSH4_MD( );

	//for( k = 0; k < m_sModelHeader.numVertSects; k++ )
	//{
	//	if( debugMode )LogFile( ERROR_LOG, "\n-------------------- P R I M I T I V E   %ld   O F   %ld --------------------\n",k+1,m_sModelHeader.numVertSects);
	//	debugSH4_VS( &(m_cVertPrim[ k ]) );
	//}

	curOffset = -1;

	do
	{
		++curOffset;
		fseek( inFile, offsets[ curOffset ], SEEK_SET );
		
		if( fread( (void *)&sectType, 1, sizeof( long ), inFile ) != sizeof( long ) )
		{
			LogFile( ERROR_LOG, "sh4_model::loadData( ) - ERROR: Couldn't read section type for section %ld of %ld - REASON: %s",
				curOffset, numOffsets, strerror( errno ) );
			delete [] offsets;
			fclose( inFile );
			deleteData( );
			return 0;
		}

		LogFile( ERROR_LOG, "\tSect %ld\tOff: %ld\tType: 0x%08x\t\ttex detect: [ %d %d ]",curOffset, offsets[curOffset],sectType, sDat[0], sDat[1] );
		
	}
	while( curOffset + 1 < numOffsets && sDat[0] != sDat[1] && sDat[0] != 0 );

	if( curOffset < numOffsets )
	{
		fseek( inFile, offsets[ curOffset ], SEEK_SET );
		if( (lNumRead = loadTex( filename, inFile )) == 0 )
		{
			LogFile( ERROR_LOG, "sh4_model::loadData( %s ) - ERROR: Couldn't load textures at offset %ld - Offset Index %ld",filename,offsets[curOffset+1], curOffset+1);
			delete [] offsets;
			fclose( inFile );
			deleteData( );
			return 0;
		}
	}
	else
		LogFile( ERROR_LOG, "sh4_model::loadData( %s ) - NOTE: Could not find a texture section", filename );

	delete [] offsets;
	fclose( inFile );

	return lNumRead + lTotalread;

}

//------------------------------------------------------------------------------------/
//- sh4_model::Render                                                                -/
//-    Renders the base model, w/o animation data at this point                      -/
//------------------------------------------------------------------------------------/
void sh4_model::Render( )
{
	long k, j, h;
	vertex4f *dispVerts = NULL;
	vertex ones(1.0f,1.0f,1.0f);
	matrix *matArray = NULL,*matArrayOrig = NULL;
	matrix *animMats = NULL;
	static bool printData = false; //true;
	unsigned int *indicies;
	vertex4f tv1,tv2,tv3,tv4;

	char outFile[128];
	
	animMats = new matrix[ m_sModelHeader.numMatSet1 ];

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
				animMats[ k ] = animMats[ m_cHeaderData.mSeq1[ k ] ] * m_pcDispFinal[ k ];
			else
				animMats[ k ] = m_pcDispFinal[ k ];
			if( printData )
			{
				LogFile(ERROR_LOG," %2d. Final Animation Matrix\n---------------------------",k);
				debugMatrix( animMats[ k ] );
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
				animMats[ k ] = animMats[ m_cHeaderData.mSeq1[ k ] ] * m_cHeaderData.matSet1[ k ];
			else
				animMats[ k ] = m_cHeaderData.matSet1[ k ];
			if( printData )
			{
				LogFile(ERROR_LOG," %2d. Final Animation Matrix\n---------------------------",k);
				debugMatrix( animMats[ k ] );
			}
		}
	}

	for( j = 0; j < m_sModelHeader.numVertSects; j++ )
	{
		
		matArrayOrig = new matrix[ m_cVertPrim[j].m_cPrimHeader.numSeq1Data + m_cVertPrim[j].m_cPrimHeader.numSeq2Data];
		matArray = new matrix[ m_cVertPrim[j].m_cPrimHeader.numSeq1Data + m_cVertPrim[j].m_cPrimHeader.numSeq2Data];
		matrix *tempMatArr = new matrix[ m_cVertPrim[j].m_cPrimHeader.numSeq1Data + m_cVertPrim[j].m_cPrimHeader.numSeq2Data];
		for( k = 0; k < m_cVertPrim[j].m_cPrimHeader.numSeq1Data; k ++ )
		{
			matArrayOrig[k] = m_cHeaderData.matSet1[m_cVertPrim[j].seqData1[k]];
//			matArray[k].transpose();
		}
		for( k = m_cVertPrim[j].m_cPrimHeader.numSeq1Data; k < m_cVertPrim[j].m_cPrimHeader.numSeq1Data + m_cVertPrim[j].m_cPrimHeader.numSeq2Data; k++ )
		{
			matArrayOrig[k] = m_cHeaderData.matSet1[ m_cHeaderData.mSeq2[ m_cVertPrim[j].seqData2[k-m_cVertPrim[j].m_cPrimHeader.numSeq1Data] * 2 + 1 ] ]
						* m_cHeaderData.matSet2[m_cVertPrim[j].seqData2[k-m_cVertPrim[j].m_cPrimHeader.numSeq1Data]];
//			matArray[k].transpose();
		}

//		for( k = 0; k < m_cVertPrim[j].m_cPrimHeader.numSeq1Data + m_cVertPrim[j].m_cPrimHeader.numSeq2Data; k++ )
//			tempMatArr[ k ] = matArray[ m_cVertPrim[j].seqReorder[ k ] ];
//		for( k = 0; k < m_cVertPrim[j].m_cPrimHeader.numSeq1Data + m_cVertPrim[j].m_cPrimHeader.numSeq2Data; k++ )
//			matArray[ k ] = tempMatArr[ k ];


//		if( printData && ( j == 18 || j== 19 || j == 40 || j==41 ) )
//		{
//			LogFile( ERROR_LOG, "================= PRIMITIVE %ld =====================",j);
//			for( k = 0; k < m_cVertPrim[j].m_cPrimHeader.numSeq1Data + m_cVertPrim[j].m_cPrimHeader.numSeq2Data; k++ )
//			{
//				LogFile( ERROR_LOG, "------------- Mat %ld of %ld ------------------",k+1,m_cVertPrim[j].m_cPrimHeader.numSeq1Data + m_cVertPrim[j].m_cPrimHeader.numSeq2Data);
//				debugMatrix( matArray[ k ] );
//			}
//		}

//		if( printData )//&& j == numPrimitives - 0 )
//		{
//			LogFile(ERROR_LOG,"\n*********************************** D E B U G I N G   M A T R I C E S **********************");
//			for( k = 0; k < m_cVertPrim[j].m_cPrimHeader.numSeq1Data + m_cVertPrim[j].m_cPrimHeader.numSeq2Data; k++)
//			{
//				debugMatrix(matArray[k]);
//			}
//		}
	
		for( h = 0; h < m_cVertPrim[j].m_lNumVertexData; h++ )
		{
			dispVerts = new vertex4f[m_cVertPrim[j].m_cVertexData[h].m_sVertexType.numVertex];
			indicies = new unsigned int[ m_cVertPrim[j].m_cVertexData[h].m_sVertexType.numIndex ];
			for( k = 0; k < m_cVertPrim[j].m_cVertexData[h].m_sVertexType.numIndex; k++ )
				indicies[k] = m_cVertPrim[j].m_cVertexData[h].indicies[k];
			for( k = 0; k < m_cVertPrim[j].m_cVertexData[h].m_lNumSeqReorder; k++ )
				tempMatArr[ k ] = matArrayOrig[ m_cVertPrim[j].m_cVertexData[h].seqReorder[ k ] ];
			for( k = 0; k < m_cVertPrim[j].m_cVertexData[h].m_lNumSeqReorder; k++ )
				matArray[ k ] = tempMatArr[ k ];
if(printData)LogFile(ERROR_LOG,"Generating Prim %ld of %ld, part %ld of %ld",j+1,this->m_sModelHeader.numVertSects,h+1,m_cVertPrim[j].m_lNumVertexData);
			for( k = 0; k < m_cVertPrim[j].m_cVertexData[h].m_sVertexType.numVertex; k++ )
			{
				

				
//				if(printData)
//					LogFile(ERROR_LOG,"Num Verts: %d\tNum Index: %d\tTex ID: %d",m_cVertPrim[j].m_cVertexData[h].m_sVertexType.numVertex,m_cVertPrim[j].m_cVertexData[h].m_sVertexType.numIndex,m_cVertPrim[j].texIDs[0]);

				
	

			//	if(printData)
			//	{
			//		LogFile(ERROR_LOG,"%d: r0w: %6.6f\n  ( %6.6f %6.6f %6.6f) ( %6.6f %6.6f %6.6f ) [ %d %d %d %d ] ",k,r0w,m_cVertPrim[j].m_cVertexData[h].verts[k].pos.x,m_cVertPrim[j].m_cVertexData[h].verts[k].pos.y,m_cVertPrim[j].m_cVertexData[h].verts[k].pos.z,m_cVertPrim[j].m_cVertexData[h].verts[k].weights.x,m_cVertPrim[j].m_cVertexData[h].verts[k].weights.y,m_cVertPrim[j].m_cVertexData[h].verts[k].weights.z,
			//			int((m_cVertPrim[j].m_cVertexData[h].verts[k].shaderSelect.z - 34.0f)/3.0f),int((m_cVertPrim[j].m_cVertexData[h].verts[k].shaderSelect.y - 34.0f)/3.0f),int((m_cVertPrim[j].m_cVertexData[h].verts[k].shaderSelect.x - 34.0f)/3.0f),int((m_cVertPrim[j].m_cVertexData[h].verts[k].shaderSelect.w - 34.0f)/3.0f));
			//	}
				if( m_cVertPrim[j].m_cPrimHeader.numSeq1Data + m_cVertPrim[j].m_cPrimHeader.numSeq2Data > 1 )
				{
					//if( m_cVertPrim[j].m_cVertexData[h].verts == NULL && m_cVertPrim[j].altVerts != NULL )
					//{
					//	LogFile(TEST_LOG,"SH3_Actor::draw - TERMINAL ERROR: Can't render prim #%d - Small size verts have multi-sequence matricies w/o selector data",j);
					//	break;
					//}

					float	r0w = m_cVertPrim[j].m_cVertexData[h].verts[k].weights.w;	//-VDOT( ones, m_cVertPrim[j].m_cVertexData[h].verts[k].weights ) + 1.0f;
					matrix temp = matArray[int((m_cVertPrim[j].m_cVertexData[h].verts[k].shaderSelect.x - 34.0f)/3.0f)];
					matrix temp2= matArray[int((m_cVertPrim[j].m_cVertexData[h].verts[k].shaderSelect.y - 34.0f)/3.0f)];
					matrix temp3= matArray[int((m_cVertPrim[j].m_cVertexData[h].verts[k].shaderSelect.z - 34.0f)/3.0f)];
					matrix temp4= matArray[int((m_cVertPrim[j].m_cVertexData[h].verts[k].shaderSelect.w - 34.0f)/3.0f)];
	
					if( printData && j == 0)LogFile( ERROR_LOG,"matricies: %ld  %ld  %ld  %ld",
								int((m_cVertPrim[j].m_cVertexData[h].verts[k].shaderSelect.x - 34.0f)/3.0f),
								int((m_cVertPrim[j].m_cVertexData[h].verts[k].shaderSelect.y - 34.0f)/3.0f),
								int((m_cVertPrim[j].m_cVertexData[h].verts[k].shaderSelect.z - 34.0f)/3.0f),
								int((m_cVertPrim[j].m_cVertexData[h].verts[k].shaderSelect.w - 34.0f)/3.0f));
					if(printData)
					{
						if(int((m_cVertPrim[j].m_cVertexData[h].verts[k].shaderSelect.z - 34.0f)/3.0f) > m_cVertPrim[j].m_cPrimHeader.numSeq1Data + m_cVertPrim[j].m_cPrimHeader.numSeq2Data
							|| int((m_cVertPrim[j].m_cVertexData[h].verts[k].shaderSelect.y - 34.0f)/3.0f) > m_cVertPrim[j].m_cPrimHeader.numSeq1Data + m_cVertPrim[j].m_cPrimHeader.numSeq2Data
							|| int((m_cVertPrim[j].m_cVertexData[h].verts[k].shaderSelect.x - 34.0f)/3.0f) > m_cVertPrim[j].m_cPrimHeader.numSeq1Data + m_cVertPrim[j].m_cPrimHeader.numSeq2Data
							|| int((m_cVertPrim[j].m_cVertexData[h].verts[k].shaderSelect.w - 34.0f)/3.0f) > m_cVertPrim[j].m_cPrimHeader.numSeq1Data + m_cVertPrim[j].m_cPrimHeader.numSeq2Data)
							LogFile(ERROR_LOG,"TERMINAL ERROR::: The matrix is out of range [%d %d %d %d]",
								int((m_cVertPrim[j].m_cVertexData[h].verts[k].shaderSelect.z - 34.0f)/3.0f),int((m_cVertPrim[j].m_cVertexData[h].verts[k].shaderSelect.y - 34.0f)/3.0f),
								int((m_cVertPrim[j].m_cVertexData[h].verts[k].shaderSelect.x - 34.0f)/3.0f),int((m_cVertPrim[j].m_cVertexData[h].verts[k].shaderSelect.w - 34.0f)/3.0f));
					}
	
		/*				if(printData && j == numPrimitives - 0 )
					{
						LogFile(ERROR_LOG,"\nVertex %d\n----------------\nr: %d\tg: %d\tb: %d\ta: %d",k,
							int((m_cVertPrim[j].m_cVertexData[h].verts[k].shaderSelect.z - 34.0f)/3.0f),int((m_cVertPrim[j].m_cVertexData[h].verts[k].shaderSelect.y - 34.0f)/3.0f),
							int((m_cVertPrim[j].m_cVertexData[h].verts[k].shaderSelect.x - 34.0f)/3.0f),int((m_cVertPrim[j].m_cVertexData[h].verts[k].shaderSelect.w - 34.0f)/3.0f));
						debugVertex(m_cVertPrim[j].m_cVertexData[h].verts[k].pos);
						debugMatrix(temp);
						debugMatrix(temp2);
					}
		*/
					dispVerts[k]=	(temp * m_cVertPrim[j].m_cVertexData[h].verts[k].pos) * m_cVertPrim[j].m_cVertexData[h].verts[k].weights.x +
									(temp2 * m_cVertPrim[j].m_cVertexData[h].verts[k].pos) * m_cVertPrim[j].m_cVertexData[h].verts[k].weights.y +
									(temp3 * m_cVertPrim[j].m_cVertexData[h].verts[k].pos) * m_cVertPrim[j].m_cVertexData[h].verts[k].weights.z +
									(temp4 * m_cVertPrim[j].m_cVertexData[h].verts[k].pos) * r0w;

					dispVerts[k].w = 1.0f;
				}
				/*else //---[ THERE IS ONLY ONE MATRIX AND SMALL SIZE VERTS ]---/
				{
					if( m_cVertPrim[j].m_cVertexData[h].verts == NULL && m_cVertPrim[j].altVerts != NULL )
					{
						if(printData)LogFile(TEST_LOG,"SH3_Actor::draw - Got Here: Can render prim #%d ? - Small size verts have single matrix",j);
						
						dispVerts[k] = matArray[0] * m_cVertPrim[j].altVerts[k].vert;
						dispVerts[k].w = 1.0f;
					}
					else if( m_cVertPrim[j].m_cVertexData[h].verts != NULL && m_cVertPrim[j].altVerts == NULL )
					{
						dispVerts[k] = matArray[0] * m_cVertPrim[j].m_cVertexData[h].verts[k].pos;
						dispVerts[k].w = 1.0f;
					}
					else
						LogFile(ERROR_LOG,"SH3_Actor::draw - TERMINAL ERROR: Prim %d - VertSize %d",j,m_cVertPrim[j].m_cVertexData[h].m_sVertexType.vertexSize);

				//					if(printData && k == 0)LogFile(ERROR_LOG,"Here: after mult [%d] - VertSIze %d",k,m_cVertPrim[j].m_cVertexData[h].m_sVertexType.vertexSize);
				}*/

			}
	// PRINTING OUT TRANSFORMED VERTICIES 
			if( printData ) //|| ( printData && dumpModel ))//&& j == numPrimitives -0 )
			{
				sprintf(outFile,"transPrim%d.txt",j);
				FILE *tFile = fopen(outFile,"w");
				int i;
				if(tFile != NULL )
				{
					//----[ PRINT OUT COUNTS OF VERTICIES AND INDICIES ]----/
					fprintf(tFile,"%ld %ld\n",m_cVertPrim[j].m_cVertexData[h].m_sVertexType.numVertex,m_cVertPrim[j].m_cVertexData[h].m_sVertexType.numIndex-2);
					fprintf(tFile,"%d %s\n",m_cVertPrim[j].texIDs[0]-1,textureMgr.FindTexName(m_cVertPrim[j].texIDs[0]));
	
					//----[ PRINT OUT VERTICES FOR EITHER 48 BYTE OR 32 BYTE VERTEX TYPES ]----/
					if(m_cVertPrim[j].m_cVertexData[h].m_sVertexType.vertexSize == sizeof(sh4_model_vert))
					{
						for( i = 0; i < m_cVertPrim[j].m_cVertexData[h].m_sVertexType.numVertex; i++)
							fprintf(tFile,"%3d: ( %.4f\t%.4f\t%.4f\t%.4f )\t( %.4f\t%.4f )\n",i,dispVerts[i].x,
								dispVerts[i].y,dispVerts[i].z,dispVerts[i].w,m_cVertPrim[j].m_cVertexData[h].verts[i].tc.s,
								m_cVertPrim[j].m_cVertexData[h].verts[i].tc.t);
					}
					//else if(m_cVertPrim[j].m_cVertexData[h].m_sVertexType.vertexSize == sizeof(sh_static_model_vertex))
					//{
					//	for( i = 0; i < m_cVertPrim[j].m_cVertexData[h].m_sVertexType.numVertex; i++ )
					//		fprintf( tFile,"%3d: ( %.4f\t%.4f\t%.4f\t%.4f )\t( %.4f\t%.4f )\n",i,dispVerts[i].x,
					//			dispVerts[i].y,dispVerts[i].z,dispVerts[i].w,m_cVertPrim[j].altVerts[i].tc.s,
					//			m_cVertPrim[j].altVerts[i].tc.t);
					//}

					//---[ PRINT OUT THE INDICIES ]---/
					for( i = 0; i < m_cVertPrim[j].m_cVertexData[h].m_sVertexType.numIndex-2; i++)
						fprintf(tFile,"%3d: ( %4ld %4ld %4ld ) %2ld %2ld\n",i,m_cVertPrim[j].m_cVertexData[h].indicies[i],
							m_cVertPrim[j].m_cVertexData[h].indicies[i+1],m_cVertPrim[j].m_cVertexData[h].indicies[i+2],m_cVertPrim[j].texIDs[0]-1,
							m_cVertPrim[j].m_cPrimHeader.numStaticData);//.texModify);
					fclose(tFile);
				}
				//debugVertex4fData(outFile,(BYTE *)dispVerts,m_cVertPrim[j].m_cVertexData[h].m_sVertexType.numVertex);
			}
	//*/
if( printData )LogFile(ERROR_LOG,"Point 1");
			//if( m_cVertPrim[j].texModify == 4 )
			//{
	//			glAlphaFunc(GL_GEQUAL, 0.01f);
	//			glEnable(GL_ALPHA_TEST);
			//}
			glFrontFace(GL_CCW);
			checkGLerror(__LINE__,__FILE__,"Before Render");
			glActiveTextureARB(GL_TEXTURE0_ARB);
			checkGLerror(__LINE__,__FILE__,"Before 1");
			glBindTexture(GL_TEXTURE_2D,this->texIDs[m_cVertPrim[j].texIDs[0]]);
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

			if( m_cVertPrim[j].m_cVertexData[h].m_sVertexType.vertexSize == sizeof(sh4_model_vert) )
			{
				glTexCoordPointer(2, GL_FLOAT, m_cVertPrim[j].m_cVertexData[h].m_sVertexType.vertexSize, &m_cVertPrim[j].m_cVertexData[h].verts[0].tc);
				if( printData )LogFile(ERROR_LOG,"Point 4");
			}
			//else if( m_cVertPrim[j].m_cVertexData[h].m_sVertexType.vertexSize == 32 )
			//	glTexCoordPointer(2, GL_FLOAT, m_cVertPrim[j].m_cVertexData[h].m_sVertexType.vertexSize, &m_cVertPrim[j].altVerts[0].tc);
			checkGLerror(__LINE__,__FILE__,"Before 6");
	if( printData )LogFile(ERROR_LOG,"Point 5");
		//if( j== 49 )
			glDrawElements(GL_TRIANGLE_STRIP, m_cVertPrim[j].m_cVertexData[h].m_sVertexType.numIndex, GL_UNSIGNED_INT, indicies ); //m_cVertPrim[j].m_cVertexData[h].indicies);
			checkGLerror(__LINE__,__FILE__,"Before 7");
	if( printData )LogFile(ERROR_LOG,"Point 6");
			glDisableClientState(GL_VERTEX_ARRAY);
			checkGLerror(__LINE__,__FILE__,"Before 8");
			glDisableClientState(GL_TEXTURE_COORD_ARRAY);

			//if( m_cVertPrim[j].texModify == 4 )
				glDisable(GL_ALPHA_TEST);

			delete [] dispVerts;
			
			delete [] indicies;
		}
//		checkGLerror(__LINE__,__FILE__,"After Render");
		//glDisableClientState(GL_NORMAL_ARRAY);
		delete [] tempMatArr;
		delete [] matArray;
		delete [] matArrayOrig;
	}

	delete [] animMats;
	printData = false;




}


#define debugMatrix3( v, k ) ( LogFile(ERROR_LOG,"\t%s[%ld]: \n\t[[ %f\t%f\t%f\t%f ]\n\t [ %f\t%f\t%f\t%f ]\n\t [ %f\t%f\t%f\t%f ]\n\t [ %f\t%f\t%f\t%f ]]",#v,k,v.mat[0],v.mat[1],v.mat[2],v.mat[3],v.mat[4],v.mat[5],v.mat[6],v.mat[7],v.mat[8],v.mat[9],v.mat[10],v.mat[11],v.mat[12],v.mat[13],v.mat[14],v.mat[15]))

void debugSH4_MD( sh4_model_header_dynamic *h, sh4_model_header *mh )
{
	long k;
	//sh4_model_header_dynamic *h = & m_cHeaderData;
	

	LogFile( ERROR_LOG, " >==--+--==<[ SH4   M O D E L   D Y N A M I C   H E A D E R ]>==--+--==<");

	LogFile( ERROR_LOG, "\n\tMatrix Set 1 (%ld matrices)\n  ---------------------------",mh->numMatSet1 );
	for( k = 0; k < mh->numMatSet1; k++ )
		debugMatrix3( h->matSet1[ k ], k );
	LogFile( ERROR_LOG, "\n\tMatrix Set 2 (%ld matrices)\n  ---------------------------",mh->numMatSet2 );
	for( k = 0; k < mh->numMatSet2; k++ )
		debugMatrix3( h->matSet2[ k ], k );
	LogFile( ERROR_LOG, "\n\tModel Heirarchy Data (%ld joints)\n  -----------------------------",mh->numMatSet1 );
	for( k = 0; k < mh->numMatSet1; k++ )
		LogFile( ERROR_LOG, "\t%ld:\t%ld",k,h->mSeq1[k]);
	LogFile( ERROR_LOG, "\n\tModel Build Data (%ld bones)\n  ---------------------------",mh->numMatSet2 * 2 );
	for( k = 0; k < mh->numMatSet2 * 2; k++ )
		LogFile( ERROR_LOG, "\t%ld:\t%ld",k,h->mSeq2[k]);
	LogFile( ERROR_LOG, "\n\tTexture ID Data (%ld texs)\n  ---------------------------",mh->numTexSetIDs );
	for( k = 0; k < mh->numTexSetIDs; k++ )
		LogFile( ERROR_LOG, "\t%ld:\t%ld",k,h->texIDs[k]);
	LogFile( ERROR_LOG, "\n\tTex ID Mapping Data (%ld vals)\n  ---------------------------",mh->numTexIDMapping);
	for( k = 0; k < mh->numTexIDMapping; k++ )
	{
		debugAll( h->texIDMapping[ k * 2 ] );
		debugAll( h->texIDMapping[ k * 2 + 1 ] );
	}
	LogFile( ERROR_LOG, "\n\tMaybe Quat Data (%ld quat)\n  ---------------------------",mh->numQuatData );
/*	for( k = 0; k < mh->numQuatData; k++ )
	{
		quat tQuat = convertToQuat( h->quatData[ k * 3 ], h->quatData[ k * 3 + 1 ], h->quatData[ k * 3 + 2 ] );
		LogFile( ERROR_LOG, "\t%ld:\tx: %f\ty: %f\tz: %f\tw: %f",k,tQuat.x,tQuat.y,tQuat.z,tQuat.w);
	}
*/	for( k = 0; k < mh->numQuatData; k++ )
	{
		LogFile( TEST_LOG,"%4d - [ %04.4x %04.4x %04.4x ]\t [ %d \t%d \t%d ]\t [ %d\t%d \t%d \t%d \t%d \t%d ]",	// [%c%c%c%c%c%c%c%c]",
			k,h->quatData[ k * 3],h->quatData[ k * 3+1],h->quatData[ k * 3 + 2 ],		//(long)(h->quatBuild[ j ].buildData[k*2]),(long)(h->quatBuild[ j ].buildData[k*2+1]),
			h->quatData[ k * 3 ],h->quatData[ k * 3 + 1 ],h->quatData[ k * 3 + 2 ],
			(char)(((char *)&(h->quatData[ k * 3]))[0]),(char)(((char *)&(h->quatData[ k * 3]))[1]),
			(char)(((char *)&(h->quatData[ k * 3+1]))[0]),(char)(((char *)&(h->quatData[ k * 3+1]))[1]),
			(char)(((char *)&(h->quatData[ k * 3+2]))[0]),(char)(((char *)&(h->quatData[ k * 3+2]))[1]));

		LogFile( DATA_LOG, "%.4ld:  [ %d\t%d\t%d\t]\t[ %d\t%d\t%d\t]",k,h->quatData[ k * 3 ], h->quatData[ k * 3 + 1 ], h->quatData[ k * 3 + 2 ],(short)h->quatData[ k *3 ],(short)h->quatData[ k * 3 + 1 ], (short)h->quatData[ k * 3 + 2 ] );
	}

/*	if( mh->numQ5_mh > 0 )
	{
		for( long j= 0; j < mh->numQ5_mh; j++ )
		{
			LogFile( ERROR_LOG,"\n\tQuat Build Data %ld of %ld (%ld elements)\n  -------------------------------",
				j+1, mh->numQ5_mh, h->quatBuild[ j ].dataCount );
			for( k = 0; k < h->quatBuild[ j ].dataCount; k++ )
			{
				LogFile( DATA_LOG,"%4d - [ %08x%08x ] [ %d %d %d %d ] [ %d %d %d %d %d %d %d %d ]",	// [%c%c%c%c%c%c%c%c]",
					k,(long)(h->quatBuild[ j ].buildData[k*2]),(long)(h->quatBuild[ j ].buildData[k*2+1]),		//(long)(h->quatBuild[ j ].buildData[k*2]),(long)(h->quatBuild[ j ].buildData[k*2+1]),
					(short)(((short *)&(h->quatBuild[ j ].buildData[k*2]))[0]),(short)(((short *)&(h->quatBuild[ j ].buildData[k*2]))[1]),
					(short)(((short *)&(h->quatBuild[ j ].buildData[k*2+1]))[0]),(short)(((short *)&(h->quatBuild[ j ].buildData[k*2+1]))[1]),
					(char)(((char *)&(h->quatBuild[ j ].buildData[k*2]))[0]),(char)(((char *)&(h->quatBuild[ j ].buildData[k*2]))[1]),(char)(((char *)&(h->quatBuild[ j ].buildData[k*2]))[2]),(char)(((char *)&(h->quatBuild[ j ].buildData[k*2]))[3]),
					(char)(((char *)&(h->quatBuild[ j ].buildData[k*2+1]))[0]),(char)(((char *)&(h->quatBuild[ j ].buildData[k*2+1]))[1]),(char)(((char *)&(h->quatBuild[ j ].buildData[k*2+1]))[2]),(char)(((char *)&(h->quatBuild[ j ].buildData[k*2+1]))[3]));
			}
		}
	}
*/
	LogFile( ERROR_LOG, "\n\tModel Extents (8 4-verts)\n  ---------------------------");
	for( k = 0; k < 8; k++ )
		debugVertex4f( h->extents[ k ] );
	LogFile( ERROR_LOG, "\n\tMaybe affine header (4 vals)\n  ---------------------------");
	for( k = 0; k < 4; k++ )
		LogFile( ERROR_LOG, "\t%ld:\t%d",k,h->qa1[k]);
	LogFile( ERROR_LOG, "\n\tMaybe affine matrix\n  -----------------------------------" );
	debugMatrix( h->q2 );
}


quat sh4_model::convertToQuat( short int xVal, short int yVal, short int zVal )
{
	quat tempQuat;
	float _2pow32 = 32768.0f;

	tempQuat.x = ((float)xVal) / _2pow32;
	tempQuat.y = ((float)yVal) / _2pow32;
	tempQuat.z = ((float)zVal) / _2pow32;

	tempQuat.computeR( );
	QUnit( &tempQuat );

	return tempQuat;
}


GLuint sh4_model::loadTex( char *texName, FILE *inFile )
{
	long lNumRead;
	long lTotalread = 0;
	char filename[ 128 ];
//	long numT1;
//	long numT2;
	long lStartOffset;
	long k, j;
	long texCount = 0;
	sh4_tex_header texHeader;
	long *offsetsT1;
	long *offsetsT2;
	//GLuint texID;

	if( !inFile )
		return 0;
	
	lStartOffset = ftell( inFile );

	lNumRead = _loadBlock( (void*)&texHeader, sizeof(texHeader), inFile,
						"sh4_model::loadTex() - Could not read tex header",ERROR_LOG);
	if( lNumRead == -1 )
		return 0;
debugSH4_MTH( &texHeader );
	lTotalread += lNumRead;

	offsetsT1 = new long[ texHeader.numT1 ];
	offsetsT2 = new long[ texHeader.numT2 ];

	lNumRead = _loadBlock( (void*)offsetsT1, sizeof(long) * texHeader.numT1, inFile,
						"sh4_model::loadTex() - Could not read tex T1 offsets",ERROR_LOG);
	if( lNumRead == -1 )
		return 0;

	lTotalread += lNumRead;

	lNumRead = _loadBlock( (void*)offsetsT2, sizeof(long) * texHeader.numT2, inFile,
						"sh4_model::loadTex() - Could not read tex T2 offsets",ERROR_LOG);
	if( lNumRead == -1 )
		return 0;

	lTotalread += lNumRead;


	m_cTextures = new sh4_model_tex[ texHeader.numT1 ];
	m_lNumTexData = texHeader.numT1;
	m_lNumTex = 0;

	for( k = 0; k < texHeader.numT1; k++ )
	{
		fseek( inFile, lStartOffset, SEEK_SET );
		if( (lNumRead = m_cTextures[ k ].loadData( offsetsT1[ k ], offsetsT2[ k ], inFile )) == 0 )
			LogFile( ERROR_LOG,"sh4_model::loadTex( ) - Couldn't read texture %ld of %ld",k+1, m_lNumTexData);
		else
			m_lNumTex += m_cTextures[ k ].m_sT2.numTex;
		lTotalread += lNumRead;
	}

	texIDs = new GLuint[ m_lNumTex ];

	for( k = 0; k < m_lNumTexData; k ++ )
	{
		for( j = 0; j < m_cTextures[ k ].m_sT2.numTex; j++ )
		{
			texIDs[ texCount++ ] = m_cTextures[ k ].m_puiTexID[ j ];
		}
	}

	for( k = 0; k < m_lNumTex; k++ )
	{
		sprintf( filename, "%s_%ld",texName, k );
		textureMgr.AddTex( string(filename), texIDs[ k ], 0 );
	}

	return lTotalread;
}
