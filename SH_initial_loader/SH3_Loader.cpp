#include <windows.h>
#include <gl\gl.h>
#include <gl\glu.h>
#include <gl\wglext.h>
#include <gl\glext.h>
#include <gl\glprocs.h>
#include <stdio.h>
#include <errno.h>

#include "vertex.h"
//#include "quat.h"
#include "matrix.h"
#include "typedefs.h"
#include "mathlib.h"
#include "Camera.h"
#include "tgaload.h"
//#include "PerPixelLighting.h"
#include "RenderObjs.h"
#include "Renderer.h"
#include "debugDrawing.h"
#include "SH3_Loader.h"
#include "SH3_ArcFilenames.h"

#include <SMD_Animation_Frame.h>
#include <SMD_vertex.h>
#include <SMD_Triangle.h>
#include <SMD_Model.h>
#include <ostream>
#include <fstream>

using namespace SMD;


extern int errno;

extern TexMgr textureMgr;
extern bool debugMode;
extern bool dumpModel;
extern bool dumpScene;

extern bool displayMatrix;
extern bool onlyVar;
extern bool onlyStatic;

//\------====[ G E N E R A L   F U N C T I O N S ]====------/

int loadArcIndex( FILE *infile, arc_index_data *index)
{
	int res;

	if(index == NULL)
		return 0;

	SAFEDELETE(index->index);

	fseek(infile,0,SEEK_SET);
	
	if((res = fread(index,1,sizeof(long)*4,infile)) < sizeof(long)*4)
	{
		LogFile(ERROR_LOG,"ERROR: Unable to read arc index data (read %d of %d bytes)",res,sizeof(long)*4);
		return 0;
	}

	if( index->indexID != 537068807 )
	{
		LogFile(ERROR_LOG,"ERROR: Arc Index ID is incorrect - Value is %ld - Should be 537068807",index->indexID);
		return 0;
	}

	if( index->numIndex <= 0 )
	{
		LogFile(ERROR_LOG,"ERROR: Arc Index Count is incorrect - Value is %ld",index->numIndex);
		return 0;
	}

	index->index = new index_record[index->numIndex];

	if((res = fread(index->index,1,sizeof(index_record) * index->numIndex,infile)) < sizeof(index_record) * index->numIndex )
	{
		LogFile(ERROR_LOG,"ERROR: Unable to read arc index records (read %d of %d bytes, %d of %d records)",res,sizeof(index_record) * index->numIndex,res/sizeof(index_record),index->numIndex);
		return 0;
	}

	for( res = 0; res < index->numIndex; res ++ )
	{
		if( index->index[res].offset <= 0 )
		{
			LogFile(ERROR_LOG,"WARNING: Offset %d is invalid (%ld) - Setting data to 0",res,index->index[res].offset);
			memset(&(index->index[res]),0,sizeof(index_record));
		}

		if( index->index[res].size < 0 )
		{
			LogFile(ERROR_LOG,"WARNING: Size of offset %d is invalid (%ld) - Setting data to 0",res,index->index[res].size);
			memset(&(index->index[res]),0,sizeof(index_record));
		}
	}

	return 1;
}


long arc_index_data::LoadData( char *filename )
{
	FILE *inFile;

	if( (inFile = fopen( filename, "rb" ) ) == NULL )
	{
		LogFile( ERROR_LOG, "arc_index_data::LoadData( %s ) - ERROR: Couldn't open file - REASON: %s", filename, strerror( errno ) );
		return 0;
	}

	if( ! loadArcIndex( inFile, this ) )
	{
		LogFile( ERROR_LOG, "arc_index_data::LoadData( %s ) - ERROR: Couldn't load data", filename );
		return 0;
	}

	strcpy( arcFilename, filename );
	fclose( inFile );
	return this->numIndex;
}


long	arc_index_data::LoadFullData( char *_pcDir, char *_pcBaseFilename )
{
	char		l_pcFilename[ 256 ];
	char		l_pcArcFilename[ 256 ];
	long		l_lNumArcIndex;
	long		l_lNumArcFiles;
	sh3_arcfile	l_cArcList;	
	long		k;
	long		j;

	if( !_pcBaseFilename )
	{
		LogFile( ERROR_LOG, "arc_index_data::LoadFullData( ) - ERROR: No base filename.  Cannot continue..");
		return 0;
	}

	//---[ Create The File Name of the Arc File ]---/

	if( !_pcDir )
	{
		LogFile( ERROR_LOG, "arc_index_data::LoadFullData( ) - No SH3 Directory Passed in - Assuming current dir");
		sprintf( l_pcFilename, "%s.arc", _pcBaseFilename );
		sprintf( l_pcArcFilename, "arc.arc" );
	}
	else
	{
		if( _pcDir[ strlen( _pcDir ) - 1 ] == '\\' )
		{
			sprintf( l_pcFilename, "%s%s.arc", _pcDir, _pcBaseFilename );
			sprintf( l_pcArcFilename, "%sarc.arc", _pcDir );
		}
		else
		{
			sprintf( l_pcFilename, "%s\\%s.arc", _pcDir, _pcBaseFilename );
			sprintf( l_pcArcFilename, "%s\\arc.arc", _pcDir );
		}
	}

	internalFilenames.clear( );

	//---[ Load the Arc Index Data ]---/

	if( ! (l_lNumArcIndex = LoadData( l_pcFilename ) ) )
	{
		LogFile( ERROR_LOG, "arc_index_data::LoadFullData( ) - ERROR: No arc index data found for file [%s]", l_pcFilename );
		return 0;
	}

	if( l_cArcList.Load( l_pcArcFilename ) < 1 )
	{
		LogFile( ERROR_LOG, "arc_index_data::LoadFullData( ) - ERROR: No file data found for arc file [%s]", l_pcArcFilename );
		return 0;
	}

	for( k = 0; k < l_cArcList.m_sArcData.arcFileCount; k++ )
	{
		if( strcmp( l_cArcList.m_pcArcSections[ k ].sectionName, _pcBaseFilename ) == 0 )
			break;
	}

	if( k >= l_cArcList.m_sArcData.arcFileCount )
	{
		LogFile( ERROR_LOG, "arc_index_data::LoadFullData( ) - ERROR: Could not find subfile [%s] data for arc file [%s]", _pcBaseFilename, l_pcArcFilename );
		return 0;
	}

	for( j = 0; j < l_cArcList.m_pcArcSections[ k ].nameCount; j++ )
	{
		internalFilenames.push_back( string( l_cArcList.m_pcArcSections[ k ].m_pcArcFilenames[ j ].fileName ) );
	}

	if( internalFilenames.size( ) != l_lNumArcIndex )
		LogFile( ERROR_LOG, "arc_index_data::LoadFullData( ) - WARNING: subfile [%s] data in arc.arc differs in num files from [%s] - %ld Arc indicies, %ld internal names", _pcBaseFilename, l_pcFilename, l_lNumArcIndex, internalFilenames.size( ) );

	return internalFilenames.size( );
}


index_record &arc_index_data::operator[](int ind)
{
	if( ind < 0 )
		return index[ 0 ];
	return index[ ind % numIndex ];
}

index_record  arc_index_data::operator[](int ind) const
{
	if( ind < 0 )
		return index[ 0 ];
	return index[ ind % numIndex ];
}


bool	arc_index_data::RegenOffsets( char *outFilename )
{
	FILE *outFile;
	FILE *inFile;
	char *l_pcTempData;
	long l_lTempDataSize;
	long l_lReadDataSize;
	long l_lCurOffset;
	long l_lRes;
	long k;

	if( strcmp( outFilename, arcFilename ) == 0 )
	{
		LogFile( ERROR_LOG, "arc_index_data::RegenOffsets( ) - ERROR: Filenames cannot Match (Original and new are both '%s'", outFilename );
		return false;
	}

	if( ! index || ! numIndex )
	{
		LogFile( ERROR_LOG, "arc_index_data::RegenOffsets( ) - ERROR: No data contained in index list");
		return false;
	}

	if( (outFile = fopen( outFilename, "wb+" )) == NULL )
	{
		LogFile( ERROR_LOG, "arc_index_data::RegenOffsets( ) - ERROR: Cannot write to '%s' - REASON: %s", outFilename, strerror( errno ));
		return false;
	}

	if( (inFile = fopen( arcFilename, "rb" )) == NULL )
	{
		LogFile( ERROR_LOG, "arc_index_data::RegenOffsets( ) - ERROR: Cannot read from '%s' - REASON: %s",arcFilename, strerror( errno ) );
		fclose( outFile );
		return false;
	}

	fwrite( (void *)&indexID, sizeof( long ), 1, outFile );
	fwrite( (void *)&numIndex, sizeof( long ), 1, outFile );
	fwrite( (void *)&indexSize, sizeof( long ), 1, outFile );
	fwrite( (void *)&f1, sizeof( long ), 1, outFile );

	l_lCurOffset = index[ 0 ].offset;


	for( k = 0; k < numIndex; k++ )
		l_lCurOffset += index[ k ].writeChanged( outFile,l_lCurOffset );

	fseek( inFile, ftell( outFile ), SEEK_SET );
	l_lTempDataSize = index[ 0 ].offset - ftell( inFile );

	l_pcTempData = new char[ l_lTempDataSize ];
	
	if( (l_lRes = fread((void *)l_pcTempData, 1, 1, inFile )) < l_lTempDataSize  )
	{
		LogFile( ERROR_LOG, "arc_index_data::RegenOffsets( ) - ERROR: Cannot read enough data from '%s' (%ld of %ld bytes read) - REASON: %s",arcFilename, l_lRes, l_lTempDataSize, strerror( errno ) );
		fclose( outFile );
		fclose( inFile );
		return false;
	}

	if( (l_lRes = fwrite((void *)l_pcTempData, l_lTempDataSize, 1, outFile )) < l_lTempDataSize  )
	{
		LogFile( ERROR_LOG, "arc_index_data::RegenOffsets( ) - ERROR: Cannot write enough data to '%s' (%ld of %ld bytes read) - REASON: %s",outFilename, l_lRes, l_lTempDataSize, strerror( errno ) );
		fclose( outFile );
		fclose( inFile );
		return false;
	}

	delete [] l_pcTempData;


	//---[ Write Out All Sections For The file, filling any overage with 0's ]---/

	for( k = 0; k < numIndex; k++ )
	{
		l_lTempDataSize = index[ k ].getChangedSize( );
		if( l_lTempDataSize )
		{
			l_pcTempData = new char[ l_lTempDataSize ];
			memset( l_pcTempData, 0, l_lTempDataSize );

			l_lReadDataSize = (l_lTempDataSize < index[ k ].size )?l_lTempDataSize:index[ k ].size;

			if( l_lReadDataSize && (l_lRes = fread((void *)l_pcTempData, 1, 1, inFile )) < l_lTempDataSize  )
			{
				LogFile( ERROR_LOG, "arc_index_data::RegenOffsets( ) - ERROR: Cannot read enough data from '%s' for index %ld of %ld (%ld of %ld bytes read) - REASON: %s",arcFilename, k+1, numIndex, l_lRes, l_lTempDataSize, strerror( errno ) );
				fclose( outFile );
				fclose( inFile );
				delete [] l_pcTempData;
				return false;
			}

			if( (l_lRes = fwrite((void *)l_pcTempData, l_lTempDataSize, 1, outFile )) < l_lTempDataSize  )
			{
				LogFile( ERROR_LOG, "arc_index_data::RegenOffsets( ) - ERROR: Cannot write enough data to '%s' for index %ld of %ld (%ld of %ld bytes read) - REASON: %s",outFilename, k+1, numIndex, l_lRes, l_lTempDataSize, strerror( errno ) );
				fclose( outFile );
				fclose( inFile );
				delete [] l_pcTempData;
				return false;
			}
		}
	}

	//---[ Re-Align the sizes in the current index set (so the data is still valid ) ]---/
	for( k = 0; k < numIndex; k++ )
		index[ k ].RealignSize( );

	fclose( inFile );
	fclose( outFile );
	delete [] l_pcTempData;

	return true;
}


int getArcOffset( FILE *infile, int indexNum, long *offset)
{
	arc_index_data index;

	if(indexNum < 0)
	{
		LogFile(ERROR_LOG,"getArcOffset:TERMINAL ERROR: Invalid file number (%d).... Exiting function",indexNum);
		return 0;
	}

	if(!loadArcIndex( infile, &index ))
	{
		LogFile(ERROR_LOG,"getArcOffset:TERMINAL ERROR: Unable to get valid index data... Exiting function");
		return 0;
	}

	if(indexNum >= index.numIndex )
	{
		LogFile(ERROR_LOG,"getArcOffset:TERMINAL ERROR: Invalid file number (%d, file index range [0,%d]).... Exiting function",indexNum,index.numIndex-1);
		return 0;
	}

	if( index.index[indexNum].offset == 0 || index.index[indexNum].size == 0 )
	{
		LogFile(ERROR_LOG,"getArcOffset:TERMINAL ERROR: Index number %d - Data doesn't exist or is empty.... Exiting function",indexNum);
		return 0;
	}

	*offset = index.index[indexNum].offset;
	
	return 1;
}




void debugMainHeader( main_scene_header *sh )
{
	LogFile(ERROR_LOG,"\n]==---=#=---==[ M A I N   H E A D E R (size: %d)]==---=#=---==[",sizeof(main_scene_header));
//#define MSHdebugLong( v ) ( LogFile(ERROR_LOG,"\t%s: %ld",#v,v))
//#define MSHdebugFloat( v ) ( LogFile(ERROR_LOG,"\t%s: %f",#v,v))
//#define MSHdebugShort( v ) ( LogFile(ERROR_LOG,"\t%s: %d",#v,v))
//#define MSHdebugAll( v ) ( LogFile(ERROR_LOG,"\t%s: float[ %f ]  Long[ %ld ]  Short[ %d %d ]  Char[ %d %d %d %d ]",#v,(float)(v),(long)(v),((short*)&v)[0],((short*)&v)[1],((char*)&v)[0],((char*)&v)[1],((char*)&v)[2],((char*)&v)[3]))
//#define MSHdebugVertex( v ) ( LogFile(ERROR_LOG,"\t%s: %f %f %f",#v,v.x,v.y,v.z))
//#define MSHdebugVertex4f( v ) ( LogFile(ERROR_LOG,"\t%s: %f %f %f %f",#v,v.x,v.y,v.z,v.w))
//#define MSHdebugMatrix( v ) ( LogFile(ERROR_LOG,"\t%s: \n\t[[ %.3f\t%.3f\t%.3f\t%.3f ]\n\t [ %.3f\t%.3f\t%.3f\t%.3f ]\n\t [ %.3f\t%.3f\t%.3f\t%.3f ]\n\t [ %.3f\t%.3f\t%.3f\t%.3f ]]",#v,v.mat[0],v.mat[1],v.mat[2],v.mat[3],v.mat[4],v.mat[5],v.mat[6],v.mat[7],v.mat[8],v.mat[9],v.mat[10],v.mat[11],v.mat[12],v.mat[13],v.mat[14],v.mat[15]))
	debugLong(sh->mainHeaderSegMarker);
	debugLong(sh->fa1[0]);
	debugLong(sh->fa1[1]);
	debugLong(sh->mainHeaderSize);
	debugLong(sh->texSegOffset);
	debugLong(sh->f2);
	debugLong(sh->altMainHeaderSize);
	debugLong(sh->totalMainHeaderSize);
	debugLong(sh->f3);
	debugLong(sh->sceneStartHeaderOffset);
	if( sh->fa4 )
		debugLongArray(sh->fa4,2);
	debugLong(sh->altTexSegOffset);
	debugLong(sh->transMatOffset);
	debugLong(sh->someWeirdDataOffset);
	debugLong(sh->f9);
	debugShort(sh->maybeTotalTex);
	debugShort(sh->maybeLocalTexBaseIndex);
	debugShort(sh->numLocalTex);
	debugShort(sh->q1);
	if( sh->fa5 )
		debugLongArray(sh->fa5,2);
	//--=[ Secondary Header Start ]=--/
	LogFile(ERROR_LOG,"   ]==---===[ Main Header - Default Transform Header ]===---==[");
	debugLong(sh->transHeaderSize);
	debugLong(sh->floatCount);
	debugLong(sh->secondaryHeaderSize);
	debugLong(sh->f7);
	debugLong(sh->transSectID);
	debugLong(sh->transSectSubID);
	if( sh->fa8 )
		debugLongArray(sh->fa8,2);
	//debugLong((sh->fa8[1]));
	//debugLong((sh->fa8[2]));
	debugMatrix(sh->transMat);
	debugVertex4f(sh->skybox[0]);
	debugVertex4f(sh->skybox[1]);
	debugVertex4f(sh->skybox[2]);
	debugVertex4f(sh->skybox[3]);
	debugVertex4f(sh->skybox[4]);
	debugVertex4f(sh->skybox[6]);
	debugVertex4f(sh->skybox[5]);
	debugVertex4f(sh->skybox[7]);
}


void debugTransformHeader( transform_scene_header *sh )
{
	LogFile(ERROR_LOG,"\n]==---==[ S C E N E   T R A N S F O R M   H E A D E R (size: %d)]==---==[",sizeof(transform_scene_header));
	debugLong(sh->transHeaderSize);
	debugLong(sh->floatCount);
	debugLong(sh->secondaryHeaderSize);
	debugLong(sh->f7);
	debugLong(sh->transSectID);
	debugLong(sh->transSectSubID);
	if( sh->fa8 )
		debugLongArray(sh->fa8,2);
	debugMatrix(sh->transMat);
	debugVertex4f(sh->skybox[0]);
	debugVertex4f(sh->skybox[1]);
	debugVertex4f(sh->skybox[2]);
	debugVertex4f(sh->skybox[3]);
	debugVertex4f(sh->skybox[4]);
	debugVertex4f(sh->skybox[6]);
	debugVertex4f(sh->skybox[5]);
	debugVertex4f(sh->skybox[7]);
}


void debugSceneTexHeader( scene_tex_header *h )
{
	LogFile(ERROR_LOG,"]==---==[ T E X   H E A D E R (size: %d)]==---==[",sizeof(scene_tex_header));
	debugLong(h->nextTexHeader);
	debugLong(h->headerSize);
	debugLong(h->segmentSize);
	debugLong(h->f1);
	debugLong(h->texNumLoc);
	debugLong(h->texNum);
	debugLong(h->texRenderModifier);
	if( h->fa2 )
		debugLongArray(h->fa2,5);
}


void debugSceneRenderHeader( scene_render_header *h )
{
	LogFile(ERROR_LOG,"]==---==[ R E N D E R   H E A D E R (size: %d)]==---==[",sizeof(scene_render_header));
	debugLong(h->nextRenderHeader);
	debugLong(h->headerSize);
	debugLong(h->segmentSize);
	debugLong(h->f1);
	debugLong(h->renderFlag1);
	debugLong(h->renderFlag2);
	if( h->fa2 )
		debugLongArray(h->fa2,6);
}


void debugSceneShaderHeader( scene_shader_header *h )
{
	LogFile(ERROR_LOG,"]==---==[ S H A D E R   H E A D E R (size: %d)]==---==[",sizeof(scene_shader_header));
	debugLong(h->nextShaderHeader);
	debugLong(h->headerSize);
	debugLong(h->segmentSize);
	debugLong(h->f1);
	debugLong(h->shaderNum);
	if( h->fa2 )
		debugLongArray(h->fa2,3);
	debugFloat(h->q1);
	debugFloat(h->q2);
	debugLong(h->f3);
	debugFloat(h->q3);
}


void debugSceneHeader( scene_header *h )
{
	LogFile(ERROR_LOG,"]==---==[ P R I M I T I V E   H E A D E R (size: %d)]==---==[",sizeof(scene_header));
	debugLong(h->nextSceneHeader);
	debugLong(h->sceneHeaderSize);
	debugLong(h->sceneSegmentSize);
	debugLong(h->f1);
	debugLong(h->numVerts);
	debugLong(h->stencilSwitch);
	debugLong(h->seqNum);
	debugLong(h->alphaBlendFlag);
	if( h->fa2 )
		debugLongArray(h->fa2,8);
}


void debugTextureSubHeader( texture_sub_header *sh )
{
	LogFile(ERROR_LOG,"\n]==---==[ T E X T U R E   S U B   H E A D E R (size: %d)]==---==[",sizeof(texture_sub_header));
	debugLong( sh->texHeaderSegMarker );
	debugLong( sh->f4 );
	debugShort( sh->width );
	debugShort( sh->height );
	debugChar( sh->bpp );
	debugChar( sh->endFillerSize );
	debugShort( sh->f5 );
	debugLong( sh->texSize );
	debugLong( sh->texHeaderSegSize );
	debugLong( sh->f6 );
	debugAll( sh->q1 );
	debugLong( sh->q2 );
	if( sh->fa7 )
		debugLongArray( sh->fa7,3 );
	if( sh->fa8 )
		debugLongArray( sh->fa8,12 );
}


void debugTextureBatchHeader( texture_batch_header *h )
{
	LogFile(ERROR_LOG,"\n]==---==[ T E X T U R E   B A T C H   H E A D E R (size: %d)]==---==[",sizeof(texture_batch_header));
	debugLong( h->texBatchSegMarker );
	debugLong( h->f1 );
	debugLong( h->texBatchHeaderSize );
	debugLong( h->texBatchSize );
	debugLong( h->f2 );
	debugLong( h->numBatchedTex );
	if( h->fa3 )
		debugLongArray( h->fa3,2 );
}


void debugModelBaseHeader( model_base_header *h )
{
	LogFile(ERROR_LOG,"\n]==---==[ M O D E L   B A S E   H E A D E R (size: %d)]==---==[",sizeof(model_base_header));
	debugLong( h->f1);
	debugLong( h->modelID);
	debugLong( h->numTex);
	debugLong( h->texOffset);
	debugLong( h->modelBaseSize);
	debugLong( h->baseHeaderSize);
	debugLong( h->f2);
	debugLong( h->always1_1);
	if( h->fa3 )
		debugLongArray( h->fa3,8);
}

void debugModelDataHeader( model_data_header *h )
{
	LogFile(ERROR_LOG,"\n]==---==[ M O D E L   D A T A   H E A D E R (size: %d)]==---==[",sizeof(model_data_header));
	debugLong( h->modelDataMarker);
	debugLong( h->always3_1);
	debugLong( h->offsetMatSet1);
	debugLong( h->numMatSet1);
	debugLong( h->offsetMatSeq1);
	debugLong( h->numMatSet2);
	debugLong( h->offsetMatSeq2);
	debugLong( h->offsetMatSet2);
	debugLong( h->numVertexArrays);
	debugLong( h->offsetVertHead);
	debugLong( h->numAltVertArrays);
	debugLong( h->offsetAltVertHead);
	debugLong( h->numTex);
	debugLong( h->offsetTexIndex);
	debugLong( h->totalModelTex);
	debugLong( h->offsetTexSeq);
	if( h->qa4 )
		debugLongArray( h->qa4,2);
	debugLong( h->offsetQ5);
	debugLong( h->q6);
	debugLong( h->offsetQ7);
	debugLong( h->q8);
	debugLong( h->offsetQ9);
	debugLong( h->offsetQ10);
	debugLong( h->offsetQ11);
	debugLong( h->always1_1);
	if( h->fa2 )
		debugLongArray( h->fa2,6);
}


void debugModelVertHeader( model_vertex_header *h )
{
	LogFile(ERROR_LOG,"\n]==---==[ M O D E L   V E R T E X   H E A D E R (size: %d)]==---==[",sizeof(model_vertex_header));
	debugLong( h->vertSegmentSize);
	debugLong( h->f1);
	debugLong( h->vertHeaderSize);
	debugLong( h->q1);
	debugLong( h->numIndex);
	debugLong( h->numAltSeq);
	debugLong( h->altOffsetSeq1);
	debugLong( h->numSeq1);
	debugLong( h->offsetSeq1);
	debugLong( h->numSeq2);
	debugLong( h->offsetSeq2);
	debugLong( h->q2);
	debugLong( h->q3);
	debugLong( h->always1_1);
	debugLong( h->offsetTexIndex);
	debugLong( h->offsetVertConst);
	debugLong( h->altHeaderSize);
	debugLong( h->numVerts);
	debugLong( h->vertSectionSize);
	debugLong( h->altNumIndex);
	debugLong( h->numFloatSets);
	debugLong( h->always1_2);
	if( h->fa3 )
		debugLongArray( h->fa3,2);
	if( h->floatSet0 )
		debugFloatArray( h->floatSet0,4);
	if( h->floatSet1 )
		debugFloatArray( h->floatSet1,4);
	if( h->floatSet2 )
		debugFloatArray( h->floatSet2,4);
	if( h->floatSet3 )
		debugLongArray( h->floatSet3,4);
}


void debugModelPrimitive( model_primitive *h )
{
	LogFile(ERROR_LOG,"\n]##===##[ M O D E L   P R I M I T I V E (size: %d)]##===##[",sizeof(model_primitive));
	debugModelVertHeader( &h->vertHeader);
	if( h->altSeq1 )
		debugShortArray( h->altSeq1,h->vertHeader.numAltSeq*3);
	if( h->seqData1 )
		debugShortArray( h->seqData1,h->vertHeader.numSeq1);
	if( h->seqData2 )
		debugShortArray( h->seqData2,h->vertHeader.numSeq2);
	debugShort( h->texNum);
	debugLong( h->always8372234_1);
	debugLong( h->always2044_1);
	debugLong( h->always97_1);
	debugLong( h->f1);
	debugLong( h->vertSize );
	debugLong( h->texModify );
}


void debugBaseAnim( sh3_base_anim *h )
{
	LogFile(ERROR_LOG,"\n]==---==[ B A S E   A N I M   H E A D E R (size: %d)]==---==[",sizeof(sh3_base_anim));
	debugLong(h->baseAnimMarker);
	if( h->vals1 )
		debugFloatArray(h->vals1,3);
	if( h->sa1 )
		debugCharArray(h->sa1,6);
	if( h->vals2 )
		debugFloatArray(h->vals2,3);
	if( h->sa2 )
		debugCharArray(h->sa2,30);
	if( h->fa1 )
		debugCharArray(h->fa1,6);
	if( h->sa3 )
		debugCharArray(h->sa3,6);

	int k;
	short *sDat = (short *)h->sa1;
	long  *lDat = (long *)h->sa2;
	long  *lDat2= (long *)&(h->sa2[2]);
	LogFile(ERROR_LOG,"  ----[ Check Alternate Values ]----");
	if( h->sa1 )
		debugShortArray(sDat,3);
	for( k = 0; k < 10; k ++ )
	{
		debugAll(lDat[k]);
		debugAll(lDat2[k]);
	}

}

void debugAltAnim1( sh3_alt1_anim *h )
{
	LogFile(ERROR_LOG,"\n]==---==[ F I R S T   A L T   A N I M   H E A D E R (size: %d)]==---==[",sizeof(sh3_alt1_anim));
	debugLong(h->altAnimMarker);			//Usually 99999999 hex
	debugCharArray(h->fa1,4);
	debugCharArray(h->sa1,2);
	debugCharArray(h->fa2,4);
	debugCharArray(h->sa2,2);
	debugCharArray(h->ma1,6);
	debugCharArray(h->sa3,6);
	debugCharArray(h->fa3,4);
	debugCharArray(h->sa4,2);
	debugCharArray(h->fa4,4);
	debugCharArray(h->sa5,14);

	int k;
	long  *lDat = (long *)h->fa1;
	long  *lDat2= (long *)&(h->fa1[2]);
	LogFile(ERROR_LOG,"  ----[ Check Alternate Values ]----");
	for( k = 0; k < 11; k ++ )
	{
		debugAll(lDat[k]);
		debugAll(lDat2[k]);
	}
	debugAll(lDat[12]);

}

void debugAltAnim2( sh3_alt2_anim *h )
{
	LogFile(ERROR_LOG,"\n]==---==[ S E C O N D   A L T   A N I M   H E A D E R (size: %d)]==---==[",sizeof(sh3_alt2_anim));
	debugLong(h->altAnimMarker);
	debugCharArray(h->sa1,6);
	debugCharArray(h->fa1,4);
	debugCharArray(h->sa2,2);
	debugCharArray(h->fa2,4);
	debugCharArray(h->sa3,2);
	debugCharArray(h->fa3,4);
	debugCharArray(h->sa4,2);
	debugCharArray(h->fa4,4);
	debugCharArray(h->sa5,2);
	debugCharArray(h->fa5,4);
	debugCharArray(h->sa6,2);
	debugCharArray(h->fa6,4);
	debugCharArray(h->sa7,2);
	debugCharArray(h->ma1,16);

	int k;
	long  *lDat = (long *)h->sa1;
	long  *lDat2= (long *)&(h->sa1[2]);
	LogFile(ERROR_LOG,"  ----[ Check Alternate Values ]----");
	for( k = 0; k < 15; k ++ )
	{
		debugAll(lDat[k]);
		debugAll(lDat2[k]);
	}


}


void debugVertexData( char *outputName, BYTE *data, int size, int numVerts )
{
	FILE *outFile;
	int k;
	vertex ones(1.0f, 1.0f, 1.0f);
	sh_vertex *tVert;
	
	if((outFile = fopen(outputName,"w+"))!=NULL)
	{
		float *fDat;
		long  *lDat;

		for( k = 0; k < numVerts*size && !size == 0; k += size)
		{
			fDat = (float *)&data[k];
			lDat = (long  *)&data[k];
			tVert = (sh_vertex*)&data[k];

			float	r0w = -VDOT( ones, tVert->v1 ) + 1.0f;
			float	r0w2= -(tVert->v1.x + tVert->v1.y + tVert->v1.z) + 1.0f;

			if( size == 48 )
			{
				fprintf(outFile,"%d: r0w: %.3f\t( %.3f\t%.3f\t%.3f\t) ( %.3f\t%.3f\t%.3f\t) [ %d  %d  %d  %d ] ( %.3f\t%.3f\t%.3f ) ( %.3f\t%.3f )\n",k/size,
					r0w,fDat[0],fDat[1],fDat[2],fDat[3],
					fDat[4],fDat[5],data[k+24],data[k+25],data[k+26],data[k+27],fDat[7],
					fDat[8],fDat[9],fDat[10],fDat[11]);
			}
			else if( size == 32 )
			{
				fprintf(outFile, "%4d: ( %.3f\t%.3f\t%.3f )\t( %.3f\t%.3f\t%.3f )\t( %.3f\t%.3f )\n",k,
					fDat[0],fDat[1],fDat[2],fDat[3],fDat[4],fDat[5],fDat[6],fDat[7]);
				//LogFile(ERROR_LOG,"---===[ UNSUPPORTED VERTEX STRIDE(%d) ]===---",size);
			}
		}
	}
	fclose(outFile);
}


void debugVertex4fData( char *outputName, BYTE *data, int numVerts )
{
	FILE *outFile;
	int k;
	int size = sizeof(vertex4f);
	
	if((outFile = fopen(outputName,"w+"))!=NULL)
	{
		float *fDat;

		for( k = 0; k < numVerts*size && !size == 0; k += size)
		{
			fDat = (float *)&data[k];
			
			fprintf(outFile,"%d: ( %.5f\t%.5f\t%.5f\t%.5f)\n",k/size,
				fDat[0],fDat[1],fDat[2],fDat[3]);
		}
	}
	fclose(outFile);
}


void flipHorizontal( int xDim, int yDim, BYTE *data )
{
	int i,j,k;
	BYTE temp;

	for( j = 0; j < yDim; j ++ )
	{
		for( k = 0; k < xDim/2; k ++ )
		{
			for( i = 0; i < 4; i++ )
			{
				temp = data[(j*xDim+k)*4+i];
				data[(j*xDim+k)*4+i] = data[(j*xDim+(xDim-k-1))*4+i];
				data[(j*xDim+(xDim-k-1))*4+i] = temp;
			}
		}
	}
}


void flipVertical( int xDim, int yDim, BYTE *data )
{
	int i,j,k;
	BYTE temp;

	for( j = 0; j < yDim/2; j ++ )
	{
		for( k = 0; k < xDim; k ++ )
		{
			for( i = 0; i < 4; i++ )
			{
				temp = data[(j*xDim+k)*4+i];
				data[(j*xDim+k)*4+i] = data[((yDim-j-1)*xDim+k)*4+i];
				data[((yDim-j-1)*xDim+k)*4+i] = temp;
			}
		}
	}
}


void saveTexture( texture_sub_header *texHeader, BYTE *data, char *filename )
{
	image_t imageDat;

	memset(&imageDat,0,sizeof(imageDat));

	imageDat.info.width			= texHeader->width;
	imageDat.info.height		= texHeader->height;
	imageDat.info.image_type	= 2;
	imageDat.info.components	= texHeader->bpp/8;
	imageDat.info.pixel_depth	= texHeader->bpp;
	imageDat.info.bytes			= imageDat.info.width * imageDat.info.height * imageDat.info.components;
	LogFile(ERROR_LOG,"saveTexture:\n------------------------------------------------------------------"
		"----------\tTexData: W:%d H:%d  bytes:%d  pixelDepth:%d  components:%d",
		imageDat.info.width,imageDat.info.height,imageDat.info.bytes,imageDat.info.pixel_depth,
		imageDat.info.components);
	imageDat.data = data;
	tgaSave(filename,&imageDat);
}



//\-----====[ scenePrimitive   M E M B E R   F U N C T I O N S ]====-----/

scenePrimitive & scenePrimitive::operator=( const scenePrimitive & rhs )
{
	if(&rhs != this)
	{
		SAFEDELETE(verts);
		verts = new sh_map_vertex [ rhs.numPrim + 2 ];
		if(verts == NULL)
			LogFile(TEST_LOG,"ERROR: Couldn't allocate space");
		memcpy(verts,rhs.verts,sizeof(sh_map_vertex)*(rhs.numPrim +2));
		if(memcmp(verts,rhs.verts,sizeof(sh_map_vertex)*(rhs.numPrim+2))!=0)
			LogFile(TEST_LOG,"ERROR: Copied strings don't match!!!!");
		
		primitiveType = rhs.primitiveType;
		alphaBlend = rhs.alphaBlend;
		alphaTest = rhs.alphaTest;
		numPrim = rhs.numPrim;
		startVert = rhs.startVert;
		stencilRef = rhs.stencilRef;
		texID = rhs.texID;
		transformNum = rhs.transformNum;
		max = rhs.max;
		min = rhs.min;
	}
	return *this;
}


void scenePrimitive::computeMinMax( )
{
	long k;

	if( numPrim == 0 )
	{
		min = vertex( 0.0f, 0.0f, 0.0f );
		max = vertex( 0.0f, 0.0f, 1.0f );
		return;
	}

	min = verts[0].vert;
	max = verts[0].vert;

	for( k = 0; k < numPrim + 2; k++ )
	{
		min.setMins( verts[ k ].vert );
		max.setMins( verts[ k ].vert );
	}
}
	

void scenePrimitive::draw()
{
	GLuint *wordArray,k;

	//	LogFile(ERROR_LOG,"scenePrimitive::draw - Start: texID: %d\tnumPrim: %d\t", texID,numPrim);

	if(alphaTest)
	{
		glAlphaFunc(GL_GEQUAL, 0.01f);
		glEnable(GL_ALPHA_TEST);
	}
	else if(alphaBlend)
	{
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA,GL_DST_COLOR);
	}

	//----[ CREATE INDICIES ]----/	
	wordArray = new GLuint[numPrim+2];

	for( k = 0; k < numPrim+2; k ++ )
		wordArray[k]=k;

	glActiveTextureARB(GL_TEXTURE0_ARB);
	glBindTexture(GL_TEXTURE_2D,texID);
	checkGLerror(__LINE__,__FILE__,"scenePrimitive::draw - after texture bind");
	glEnable(GL_TEXTURE_2D);
	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(3, GL_FLOAT, sizeof(sh_map_vertex) , &verts[0].vert);

	glClientActiveTextureARB(GL_TEXTURE0_ARB);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glTexCoordPointer(2, GL_FLOAT, sizeof(sh_map_vertex), &verts[0].tc);
	checkGLerror(__LINE__,__FILE__,"scenePrimitive::draw - After client size pointer initialization");

	glDrawElements(GL_TRIANGLE_STRIP, numPrim+2, GL_UNSIGNED_INT, wordArray);

	checkGLerror(__LINE__,__FILE__,"scenePrimitive::draw - After draw command");

	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);

	SAFEDELETE(wordArray);

	if(alphaBlend )
		glDisable(GL_BLEND);
	if(alphaTest)
		glDisable(GL_ALPHA_TEST);
}



//\-----====[ SceneMap   M E M B E R   F U N C T I O N S ]====-----/
void SceneMap::deleteScene()
{
	int k;

	if(debugMode)
		LogFile(ERROR_LOG,"~SceneMap - Start");
	
	for(k = 0; k < numScenePrimitives; k ++)
	{
		SAFEDELETE(sceneData[k].verts);
		if(debugMode)
			LogFile(ERROR_LOG,"~SceneMap - verts [%d]",k);
	}

	if(debugMode)
		LogFile(ERROR_LOG,"~SceneMap - After deleting all verts");
	
	SAFEDELETE(sceneData);

	SAFEDELETE(sceneTransform);
	
	if(debugMode)
		LogFile(ERROR_LOG,"~SceneMap - Before Interact Delete");

	if(interact.sceneInteract)
	{
		for( k = 0; k < interact.numSceneInteract; k++ )
			SAFEDELETE(interact.sceneInteract[k].verts);
	}
	SAFEDELETE(interact.sceneInteract);

	numScenePrimitives = 0;
	baseOffset = 0;
	curTexID = 0;
	numSceneTransform = 0;

	if(debugMode)
		LogFile(ERROR_LOG,"~SceneMap - Finish");
}


int SceneMap::determineHeaderType( long *data )
{
	scene_tex_header *tP = (scene_tex_header *)data;
	scene_render_header *rP = (scene_render_header *)data;
	scene_shader_header *sP = (scene_shader_header *)data;
	scene_header *pP = (scene_header *)data;

	int k;

	if( data[1] == 64 )
		return 4;
	else if( data[1] == 48 )
	{
		if( sP->q1 > 0.01 || sP->q2 > 0.01 || sP->q3 > 0.01 )
			return 3;
		if( rP->renderFlag1 > 65535 )
			return 2;
		if( tP->texNumLoc + tP->texRenderModifier > 1 && tP->texNum + tP->texNumLoc + tP->texRenderModifier < 61 )
			return 1;
	}

	LogFile(ERROR_LOG,"****** E R R O R ******\nCouldn't determine Header type:");
	for( k = 0; k < 16; k++ )
		debugAll((data[k]));

	return 0;
}

//#define convertRed( x ) ( ((x >> 12) & 0x0f ) * 16 )
//#define convertGreen( x ) ( ((x >> 8) & 0x0f ) * 16 )
//#define convertBlue( x ) ( ((x >> 4) & 0x0f ) * 16 )
//#define convertAlpha( x ) ( ((x >> 0) & 0x0f ) * 16 )
//#define convertRed( x ) ( ((x >> 11) & 0x01f ) * 8 )
//#define convertGreen( x ) ( ((x >> 5) & 0x03f ) * 4 )
//#define convertBlue( x ) ( ((x >> 0) & 0x01f ) * 8 )
//#define convertAlpha( x ) ( 255 )
//#define convertRed( x ) ( ((x >> 11) & 0x01f ) * 8 )
//#define convertGreen( x ) ( ((x >> 6) & 0x01f ) * 8 )
//#define convertBlue( x ) ( ((x >> 1) & 0x0f ) * 8 )
//#define convertAlpha( x ) ( ((x >> 0) & 0x01 ) * 255 )

//------------[ REVERSE ORDER CONVERSION ]--------------/
//#define convertRed( x ) ( ((x >> 0) & 0x0f ) * 16 )
//#define convertGreen( x ) ( ((x >> 4) & 0x0f ) * 16 )
//#define convertBlue( x ) ( ((x >> 8) & 0x0f ) * 16 )
//#define convertAlpha( x ) ( ((x >> 12) & 0x0f ) * 16 )
//#define convertRed( x ) ( ((x >> 0) & 0x01f ) * 8 )
//#define convertGreen( x ) ( ((x >> 5) & 0x03f ) * 4 )
//#define convertBlue( x ) ( ((x >> 11) & 0x01f ) * 8 )
//#define convertAlpha( x ) ( 255 )

//#define convertRed( x ) ( ((short)(x >> 0) & (short)0x01f ) * 8 )
//#define convertGreen( x ) ( ((short)(x >> 5) & (short)0x01f ) * 8 )
//#define convertBlue( x ) ( ((short)(x >> 10) & (short)0x01f ) * 8 )
//#define convertAlpha( x ) ( ((short)(x >> 15) & (short)0x01 ) * 255 )

#define		debugShortHex( v ) ( LogFile(ERROR_LOG,"\t%s: [ %d ] [ 0x%04.4x ]",#v,v,v))
#define		debugLongHex( v ) ( LogFile(ERROR_LOG,"\t%s: [ %ld ] [ 0x%08.8x ]",#v,v,v))

GLuint SceneMap::loadTex( FILE *infile, scene_tex_header *h )
{
	texture_batch_header texBatch;
	texture_sub_header texHeader;
	int texIndex;
	int res;
	int j,k;
	BYTE temp;
	BYTE *tBuffer = NULL;
	short *altBuffer = NULL;
	short *altBuffer2 = NULL;
	long altTexFileOffset = 0;
	long texBatchOffset = 0;
	long numAltFound = 0;
	GLuint texID;
	arc_index_data arcIndex;


	//----[ CORRECT TEXTURE INDEX FOR LOCAL TEXTURES ]----/
	if(h->texNumLoc != 3 )
		texIndex = h->texNum;
	else
	{
		if( checkBaseIndex == -1 )
		{
			checkBaseIndex = h->texNum;
			if( checkBaseIndex != mainHeader.maybeLocalTexBaseIndex )
			{
				LogFile(ERROR_LOG,"ERROR: Local Tex Base Index Does NOT Match: baseInd - %ld\t1st Tex - %ld",mainHeader.maybeLocalTexBaseIndex,checkBaseIndex);
				mainHeader.maybeLocalTexBaseIndex = checkBaseIndex;
			}
		}
		texIndex = h->texNum - mainHeader.maybeLocalTexBaseIndex;
	}

	//----[ DETERMINE WHERE TO LOAD TEXTURE (OFFSET) ]----/

	if(h->texNumLoc != 3 )		//---[ DEAL WITH EXTERNAL TEXTURES ]---/
	{

	//	figure out how to find other texture file (how, I can't tell yet)
		if(	loadArcIndex( infile, &arcIndex ) == 0 )
			LogFile(ERROR_LOG,"FUCKFUCKFUCKFUCKFUCKFUCKFUCKFUCK");

		k = 0; 
		while( k < arcIndex.numIndex)
		{
			if( arcIndex.index[k].size < 128*128*4 ) 
				k ++;
			else
				break;
		}

		for( j = k; j < arcIndex.numIndex; j++ )
		{
			fseek( infile, arcIndex.index[j].offset, SEEK_SET);
			
			if((res = fread(&texBatch,1,sizeof(texture_batch_header),infile)) < sizeof(texture_batch_header))
			{
				LogFile(ERROR_LOG,"SceneMap::loadTex:ERROR: Couldn't read enough bytes for texture header in alternate tex file path - index #%d (%d of %d read)\nTry debug anyways...",j,res,sizeof(texture_batch_header));
				debugTextureBatchHeader(&texBatch);
				return 0;
			}
			
			if(	(	texBatch.texBatchSegMarker == -1 
					&& texBatch.texBatchHeaderSize == 32 
					&&	
					(	texBatch.f1 == 0 || texBatch.f2 == 0 
				)) 
				&&	
				(	(	h->texNumLoc == 1 && texBatch.numBatchedTex > h->texNum
						&& texBatch.numBatchedTex > 1
					)
					||
					(	h->texNumLoc == 2 
						//&& arcIndex.index[j].offset > baseOffset 
						&& texBatch.numBatchedTex == 1													//&& arcIndex.index[j].size == 256*256*4+sizeof(texture_batch_header)+sizeof(texture_sub_header)
			  )))					//( h->texNumLoc == 1 && ( arcIndex.index[j].size % (512*512*4+96) == 0 || (arcIndex.index[j].size-sizeof(texture_batch_header)) % (512*512*4+sizeof(texture_sub_header)) == 0)) ||
			{
				LogFile(ERROR_LOG,"NOTE: Index %d - Offset %ld  - Size %ld fits the correct size for just textures",j,arcIndex.index[j].offset,arcIndex.index[j].size);
//NOTE: ONLY DO THIS NEXT 'if' WHEN THERE IS MORE THAN ONE 'GROUP' TEXTURE FILE
				if( altTexFileOffset == 0 || abs((baseOffset-altTexFileOffset)) > abs((baseOffset-arcIndex.index[j].offset)))
					altTexFileOffset = arcIndex.index[j].offset;
				////numAltFound++;
				////if(numAltFound == h->texNumLoc || h-)
				//if( h->texNumLoc == 2 )
				//{
				//	LogFile(ERROR_LOG,"NOTE: Breaking here since the count of alternate texture locations match the tex location number from the header");
				//	break;
				//}
			}
		}

		if( altTexFileOffset == 0 )
		{
			scene_tex_header temphead;
			LogFile(ERROR_LOG,"ERROR: Couldn't find a correct texture segment.  Substituting texture number %d (%d)",h->texNum + mainHeader.maybeLocalTexBaseIndex,h->texNum);
			memcpy(&temphead,h,sizeof(scene_tex_header));
			temphead.texNum += mainHeader.maybeLocalTexBaseIndex;
			temphead.texNumLoc = 3;
			SAFEDELETE(arcIndex.index);
			return loadTex(infile,&temphead);
		}
		SAFEDELETE(arcIndex.index);
	}
	else	//----[ SET THE altTexFileOffset FOR THE LOCAL TEXTURES ]----/
	{
		altTexFileOffset = baseOffset + mainHeader.texSegOffset;
	}

	//----[ LOAD THE TEXTURE BATCH HEADER ]----/
	fseek(infile,altTexFileOffset,SEEK_SET);

	if((res = fread(&texBatch,1,sizeof(texBatch),infile)) < sizeof(texture_batch_header))
	{
		LogFile(ERROR_LOG,"SceneMap::loadTex:ERROR: Couldn't read enough bytes for texture batch header in alternate tex file path - index #%d (%d of %d read)\nTry debug anyways...",j,res,sizeof(texture_batch_header));
		debugTextureBatchHeader(&texBatch);
		return 0;
	}

	if(debugMode)
		debugTextureBatchHeader(&texBatch);

	//--[ Make Sure The Index Is In Range ]--/
	if( texIndex >= texBatch.numBatchedTex )
	{
			LogFile(ERROR_LOG,"SceneMap::loadTex:ERROR: Texture Number is out of range [%d of %d]",texIndex,texBatch.numBatchedTex-1);
			return 0;
	}
	else if( h->texNum == -1 )
	{
		LogFile(ERROR_LOG,"SceneMap::loadTex - NOTE: There is a -1 texture index.  Assuming it is last texture in the alternate tex file");
		texIndex = texBatch.numBatchedTex - 1;
	}

	//--[ Reset The Location To After Texture Batch Header ]--/
	altTexFileOffset += sizeof(texture_batch_header);


	//----[ GET FIRST TEX HEADER TO DETERMINE SIZE AND OFFSET OF TEXTURES ]----/
		
	fseek( infile, altTexFileOffset, SEEK_SET);
			
	//----[ FIND THE TEXTURE TO LOAD]----//
	for( k = 0; k < texIndex; k++ )
	{
		fseek( infile, altTexFileOffset + texBatchOffset, SEEK_SET);
		
		if((res = fread(&texHeader,1,sizeof(texHeader),infile)) < sizeof(texture_sub_header))
		{
			LogFile(ERROR_LOG,"SceneMap::loadTex:ERROR: Couldn't read enough bytes for texture header while iterating through textures - texture #%d in (%d of %d bytes read)\nTry debug anyways...",k,res,sizeof(texture_sub_header));
			debugTextureSubHeader(&texHeader);
			return 0;
		}

		texBatchOffset += texHeader.texHeaderSegSize;
	}


	//----[ LOAD HEADER FOR TEXTURE TO USE ]----/
	fseek( infile, altTexFileOffset + texBatchOffset, SEEK_SET);
		
	if((res = fread(&texHeader,1,sizeof(texHeader),infile)) < sizeof(texture_sub_header))
	{
		LogFile(ERROR_LOG,"SceneMap::loadTex:ERROR: Couldn't read enough bytes for actual used texture header (%d of %d)\nTry debug anyways...",res,sizeof(texture_sub_header));
		debugTextureSubHeader(&texHeader);
		return 0;
	}

	if(debugMode)
		debugTextureSubHeader(&texHeader);
	
	if( texHeader.texHeaderSegSize - texHeader.texSize == 128 )
		fseek( infile, altTexFileOffset + texBatchOffset + 128 , SEEK_SET);

	tBuffer = new BYTE[texHeader.width * texHeader.height * 4];

	if(tBuffer == NULL )
	{
		LogFile(ERROR_LOG,"SceneMap::loadTex:ERROR: Unable to allocate memory for texture");
		return 0;
	}

	//----[ READ TEXTURE DATA ]----/
	//if( texHeader.bpp != 32 )//h->texRenderModifier > 9) //texHeader.f6 != 0 )		//IMAGE IS INTERLACED
	//{
	//	for( j = 0; j < texHeader.height/2; j ++ )
	//	{
	//		fread(&tBuffer[j*texHeader.width*texHeader.bpp/8],sizeof(char),texHeader.width*texHeader.bpp/8,infile);
	//		fread(&tBuffer[(j+texHeader.height/2)*texHeader.width*texHeader.bpp/8],sizeof(char),texHeader.width*texHeader.bpp/8,infile);
	//	}
	//}
	//else
	//############## TEST - CHECK VARIOUS TEXTURE VALUES ################
	if( /*h->texNum == -1 ) */h->texRenderModifier == 0 )
	{
		for( j = 0; j < texHeader.height; j++ )
		{
			for( k = 0; k < texHeader.width; k ++ )
			{
				tBuffer[(j*texHeader.width+k)*4+0] = 0;
				tBuffer[(j*texHeader.width+k)*4+1] = 0;
				tBuffer[(j*texHeader.width+k)*4+2] = (((j / 32 ) % 2) ^ ((k / 32) % 2 )) * 255;
				tBuffer[(j*texHeader.width+k)*4+3] = 255;
			}
		}
	}
	else
		fread(tBuffer,sizeof(BYTE),texHeader.width * texHeader.height * texHeader.bpp/8, infile);

	//---[ CONVERT BUFFER IF BPP IS NOT 32 ]---/
	if( texHeader.bpp != 32 )
	{
		altBuffer = new short[texHeader.width * texHeader.height * texHeader.bpp/sizeof(short)];
		//altBuffer2 = new short[texHeader.width * texHeader.height * texHeader.bpp/sizeof(short)];
		if(debugMode)
			LogFile(ERROR_LOG,"sceneMap::loadTex - Converting texture w/ %d bpp",texHeader.bpp);
		memcpy(altBuffer,tBuffer,texHeader.width * texHeader.height * texHeader.bpp/8);

		//VERTICAL INTERLACING****************
		//for( j = 0; j < texHeader.height; j ++ )
		//{
		//	for( k = 0; k < texHeader.width; k++ )
		//	{
		//		if( k%2 == 0 )
		//			altBuffer2[ j*texHeader.width+(k/2) ] = altBuffer[ j*texHeader.width+k];
		//		else
		//			altBuffer2[ j*texHeader.width+((texHeader.width-1)-k)/2+k ] = altBuffer[ j*texHeader.width+k];
		//	}
		//}

		//LogFile(ERROR_LOG,"TEST CONVERSION: Check Short Values");
		//for( k = 0; k < 16; k ++ )
		//	debugShortHex( altBuffer[k] );

		for( j = 0; j < texHeader.height; j ++ )
		{
			for( k = 0; k < texHeader.width; k ++ )
			{
				tBuffer[(j*texHeader.width+k)*4+0] = convertRed(altBuffer[j*texHeader.width+k]);
				tBuffer[(j*texHeader.width+k)*4+1] = convertGreen(altBuffer[j*texHeader.width+k]);
				tBuffer[(j*texHeader.width+k)*4+2] = convertBlue(altBuffer[j*texHeader.width+k]);
				tBuffer[(j*texHeader.width+k)*4+3] = convertAlpha(altBuffer[j*texHeader.width+k]);
			}
		}

		//long *lDat = (long *)tBuffer;
		//LogFile(ERROR_LOG,"TEST CONVERSION: Check Long Values");
		//for( k = 0; k < 16; k ++ )
		//	debugLongHex( lDat[k] );
		SAFEDELETE(altBuffer);
		SAFEDELETE(altBuffer2);
	}


	



	//switch( h->texRenderModifier  )
	//{
	//	case 3: flipVertical(texHeader.width,texHeader.height,tBuffer); break;
	//	case 2: flipHorizontal(texHeader.width,texHeader.height,tBuffer); break;
	//}

	glGenTextures ( 1, &texID );
	if(texID < 1 )
		LogFile(ERROR_LOG,"SceneMap::loadTex: ERROR: Couldn't generate more textures!!!!! glGenTextures returned %d as the tex ID",texID);
	else
	{

		if( dumpScene )
		{
			char outTex[128];
			sprintf( outTex,"%ld_%d.tga",baseOffset, texID );
			saveTexture( &texHeader, tBuffer, outTex );
		}

			//----[ CORRECTLY ORDER COLORS ]----/
		for( j = 0; j < texHeader.height; j ++ )
		{
			for( k = 0; k < texHeader.width; k ++ )
			{
				temp = tBuffer[(j*texHeader.width+k)*4+0];
				tBuffer[(j*texHeader.width+k)*4+0] = tBuffer[(j*texHeader.width+k)*4+2];
				tBuffer[(j*texHeader.width+k)*4+2]=temp;
			}
		}

		glBindTexture ( GL_TEXTURE_2D, texID );
		glTexImage2D ( GL_TEXTURE_2D, 0, GL_RGBA, texHeader.width, texHeader.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, tBuffer );  

		glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
		glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexEnvi ( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE,GL_REPLACE);
	}

	delete [] tBuffer;
	

	return texID;
}


int SceneMap::loadScene( FILE *sceneFile, long bOffset )
{
	int res;
	long l_numTH = 0, l_numRH = 0, l_numSH = 0, l_numPH = 0;
	scene_tex_header	curTH;
	scene_render_header	curRH;
	scene_shader_header	curSH;
	scene_header		curPH;
	scenePrimitive		*tSP = NULL;
	scenePrimitive		*cSP;
	BYTE				tempData[208];
	long				sceneIndex;				//Indexes current place while looking for headers
	bool				stencilSwitch = false;
	sh_map_vertex		*tempVerts;
	int k;

	if(debugMode)
		LogFile(ERROR_LOG,"loadScene - Start Offset is %ld",bOffset);

	//----[ INITIALIZE DATA FOR LOAD ]----/
	for( k = 0; k < numScenePrimitives; k ++ )
		SAFEDELETE(sceneData[k].verts);
	SAFEDELETE(sceneData);
	SAFEDELETE(sceneTransform);
	numScenePrimitives = 0;
	maxScenePrimitives = 200;
	baseOffset = bOffset;
	nextTexOffset = 0;
	nextRenderOffset = 0;
	nextShaderOffset = 0;
	nextSceneOffset = 0;
	curTexID = 0;
	checkBaseIndex = -1;
	numSceneTransform = 0;
	
	deleteScene();

	curTH.nextTexHeader = 0;
	curRH.nextRenderHeader = 0;
	curSH.nextShaderHeader = 0;
	curPH.nextSceneHeader = 0;

	baseOffset = bOffset;

	tSP = new scenePrimitive[maxScenePrimitives];

	//----[ LOAD ALL MAIN HEADERS, AND SPECIAL DATA ]----/
	fseek(sceneFile,bOffset,SEEK_SET);
	
	if((res = fread(&mainHeader,1,sizeof(main_scene_header),sceneFile)) < sizeof(main_scene_header))
	{
		LogFile(ERROR_LOG,"SceneMap::loadScene ERROR: Couldn't load main scene header (%d of %d bytes read)",
			res,sizeof(main_scene_header));
		delete [] tSP;
		return 0;
	}
	else //if(debugMode)
		debugMainHeader( &mainHeader );

	if(  !isMainSceneHeader(&mainHeader) )
	{
		LogFile(ERROR_LOG,"SceneMap::loadScene TERMINAL ERROR: This data is not a scene - Load aborted");
		delete [] tSP;
		return 0;
	}

	//---[ DETERMINE NUMBER OF SCENE TRANSFORMATIONS ]---//

	if( mainHeader.totalMainHeaderSize )
	{
		numSceneTransform = (mainHeader.totalMainHeaderSize - sizeof(main_scene_header))/sizeof(transform_scene_header);
	}
	else if( (mainHeader.sceneStartHeaderOffset-sizeof(main_scene_header))%sizeof(transform_scene_header) == 0 )
	{
		numSceneTransform = (mainHeader.sceneStartHeaderOffset - sizeof(main_scene_header))/sizeof(transform_scene_header);
		mainHeader.totalMainHeaderSize = mainHeader.sceneStartHeaderOffset;
	}
	else if( mainHeader.f3 )
	{
/*SPECIAL DEBUG*/		LogFile(DATA_LOG,"SceneMap::loadScene NOTE: Using the f3 filler variable - it is %ld",mainHeader.f3);
		numSceneTransform = (mainHeader.f3 - sizeof(main_scene_header))/sizeof(transform_scene_header);
		mainHeader.totalMainHeaderSize = mainHeader.f3;
	}
	else
		/*SPECIAL DEBUG*/LogFile(DATA_LOG,"SceneMap::loadScene ERROR: totalMainHeaderSize = 0, sceneStartHeaderOffset isn't right after main scene header and f3 = 0: %s - offset: %ld",sceneFilename,baseOffset);

	
	sceneTransform = new transform_scene_header[ numSceneTransform ];

	for( k = 0; k < numSceneTransform; k ++ )
	{
		if((res = fread(&(sceneTransform[k]),1,sizeof(transform_scene_header),sceneFile)) < sizeof(transform_scene_header))
			LogFile(ERROR_LOG,"SceneMap::loadScene ERROR: Couldn't load transform scene header #%d (%d of %d bytes read)",
					k,res,sizeof(transform_scene_header));
		else
		{
			if(debugMode)
				debugTransformHeader( &(sceneTransform[k]) );
///*REMOVE*/			sceneTransform[k].transMat.mat[12] -= mainHeader.transMat.mat[12];
///*REMOVE*/			sceneTransform[k].transMat.mat[13] -= mainHeader.transMat.mat[13];
///*REMOVE*/			sceneTransform[k].transMat.mat[14] -= mainHeader.transMat.mat[14];
		}
	}

	if( mainHeader.transMatOffset )
	{
		fseek( sceneFile, bOffset + mainHeader.transMatOffset, SEEK_SET );
		if((res = fread( &transMatSet, 1, sizeof(scene_trans_header), sceneFile )) < sizeof(scene_trans_header))
			LogFile(ERROR_LOG,"SceneMap::loadScene ERROR: Couldn't load transMatSet (%d of %d bytes read)",res, sizeof(scene_trans_header));
		else if(debugMode)
		{
			LogFile(ERROR_LOG,"\n]==---==[ T R A N S   S E T   H E A D E R (size: %d)]==---==[",sizeof(scene_trans_header));
			debugMatrix(transMatSet.transMat1);
			debugMatrix(transMatSet.transMat2);
			debugMatrix(transMatSet.transMat3);
			debugMatrix(transMatSet.transMat4);
		}

		if( !mainHeader.someWeirdDataOffset )
		{
//			long *localData;
			long sizeLocal = mainHeader.texSegOffset - (mainHeader.transMatOffset + sizeof(scene_trans_header));

			/*if( sizeLocal )
			{
				localData = new long[sizeLocal/sizeof(long)];

				if((res = fread( localData, 1, sizeLocal, sceneFile )) < sizeLocal)
				{
					LogFile(ERROR_LOG,"SceneMap::loadScene ERROR: Couldn't load additional data after transMatOffset (%d of %d bytes read)",res,sizeLocal);
				}
				else
				{
					LogFile(ERROR_LOG,"\n]==---==[ UNKNOWN DATA AFTER TRANS SET HEADER(size: %d)]==---==[",sizeLocal);
					for( k = 0; k < sizeLocal/sizeof(long); k++)
						debugAll(localData[k]);
				}
				delete [] localData;

			}
			*/
		}
		else	//Load scene_interact_data
		{

		}
			
	}


	//-----[ L O A D   A L L   P R I M I T I V E S   F O R   S C E N E ]-----/

	nextSceneOffset = mainHeader.sceneStartHeaderOffset;

	do
	{

		if(debugMode)
			LogFile(ERROR_LOG,"*******************\nPrimitive: %d\tOffset: %ld\n********************",numScenePrimitives,nextSceneOffset);

		if( numScenePrimitives == maxScenePrimitives )
		{
			maxScenePrimitives *= 2;
			cSP = new scenePrimitive[maxScenePrimitives];
			
			for( k = 0; k < maxScenePrimitives/2; k ++ )
				cSP[k] = tSP[k];

			delete [] tSP;
			tSP = cSP;
		}

		sceneIndex = 0;

		fseek(sceneFile,bOffset+nextSceneOffset,SEEK_SET);

		//Read The Max Amount of Data For All 4 Headers
		if((res = fread(tempData,sizeof(BYTE),208,sceneFile)) < 208 )
			LogFile(ERROR_LOG,"ERROR: Only read %d of %d bytes while trying to read scene headers",res,208);
	
		//Determine Which Headers We Have
		while((res = determineHeaderType((long*)&(tempData[sceneIndex]))) < 4 && res != 0)
		{
			if( res == 1 )
			{
				char texStr[64];
				memcpy(&curTH,&(tempData[sceneIndex]),sizeof(scene_tex_header));
				l_numTH++;

				if(debugMode)
					debugSceneTexHeader(&curTH);

				sceneIndex += curTH.headerSize;
				if( curTH.texNumLoc == 3 )
					sprintf(texStr,"%ld_%d_%d",baseOffset,curTH.texNumLoc,curTH.texNum);
				else
					sprintf(texStr,"%d_%d",curTH.texNumLoc,curTH.texNum);

				if((curTexID = textureMgr.GetTexture(string(texStr))) == 0 )
				{
					if((curTexID = loadTex(sceneFile,&curTH)) != 0 )
						textureMgr.AddTex(string(texStr),curTexID,0);

					if(debugMode)
						LogFile(ERROR_LOG,"loadScene - CurTexID is %ud",curTexID);
				}
				nextTexOffset = curTH.nextTexHeader;
			}
			else if( res == 2 )
			{
				memcpy(&curRH,&(tempData[sceneIndex]),sizeof(scene_render_header));
				l_numRH++;
				
				if(debugMode)
					debugSceneRenderHeader(&curRH);

				sceneIndex += curRH.headerSize;
				nextRenderOffset = curRH.nextRenderHeader;
			}
			else if( res == 3 )
			{
				memcpy(&curSH,&(tempData[sceneIndex]),sizeof(scene_shader_header));
				l_numSH++;

				if(debugMode)
					debugSceneShaderHeader(&curSH);

				sceneIndex += curSH.headerSize;
				nextShaderOffset = curSH.nextShaderHeader;
			}

		}

		//Either Capture The Error or Load the Scene Data
		if( res == 0 )
		{
			LogFile(ERROR_LOG,"TERMINAL ERROR: Breaking load... Only loaded %d Primitives",numScenePrimitives);
			nextSceneOffset = mainHeader.sceneStartHeaderOffset;
		}
		else
		{
			memcpy(&curPH,&(tempData[sceneIndex]),sizeof(scene_header));
			l_numPH++;
				
			if(debugMode)
				debugSceneHeader(&curPH);
			sceneIndex += curPH.sceneHeaderSize;
			


			if( curPH.stencilSwitch == 2 && curSH.shaderNum == 9 )
				stencilSwitch = !stencilSwitch;
			
			//Jump to the start of the vertex data
			fseek(sceneFile,bOffset + nextSceneOffset + sceneIndex,SEEK_SET);

			tempVerts = new sh_map_vertex[curPH.numVerts];

			if(tempVerts == NULL )
				LogFile(TEST_LOG,"ERROR: COuldn't allocat memory in loadScene");

			nextSceneOffset = curPH.nextSceneHeader;

			if((res = fread(tempVerts,1,sizeof(sh_map_vertex) * curPH.numVerts,sceneFile)) < sizeof(sh_map_vertex)*curPH.numVerts)
			{
				LogFile(ERROR_LOG,"TERMINAL ERROR: Unable to load verticies for scene %d (%d of %d bytes read)",numScenePrimitives,res,sizeof(sh_map_vertex)*curPH.numVerts);
				nextSceneOffset = mainHeader.sceneStartHeaderOffset;
			}
			else	//Successfully got Scene Data .. Load it into array and set primitive data
			{
				tSP[numScenePrimitives] = scenePrimitive(tempVerts,GL_TRIANGLE_STRIP,curTexID,0,
														 curPH.numVerts-2,(curRH.renderFlag2 == 196608 || (curRH.renderFlag2 == 65536 && curSH.shaderNum == 0))?1:0,
														 (curPH.alphaBlendFlag == 124)?1:0,(stencilSwitch && curRH.renderFlag2 == 0)?0:64,
														 (stencilSwitch && curRH.renderFlag2 == 0)?21:curSH.shaderNum,(curPH.stencilSwitch == 3)?curPH.seqNum:0);
				LogFile( ERROR_LOG, "Data Comparison:\n----------------------------------------------\n"
									"  Primitive: %ld\n  TexID: %ld\n  #prim: %ld\n  alphaBld: %ld\n  alphaTest: %ld\n  Shader#: %ld"
									"  Start Vert#: %ld\n  stencilRef: %ld\n  Trans#: %ld",
									numScenePrimitives, curTexID,tSP[numScenePrimitives].numPrim,tSP[numScenePrimitives].alphaBlend,tSP[numScenePrimitives].alphaTest,
									tSP[numScenePrimitives].shaderNum, tSP[numScenePrimitives].startVert,tSP[numScenePrimitives].stencilRef,
									tSP[numScenePrimitives].transformNum);
				tSP[numScenePrimitives].computeMinMax( );

				numScenePrimitives++;
			}

			//SAFEDELETE(tempVerts);

			if( nextTexOffset == 0 && nextRenderOffset == 0 && nextShaderOffset == 0 && nextSceneOffset == 0 )
				nextSceneOffset = mainHeader.totalMainHeaderSize;	//Just in case this isn't a size, but a 1st offset
			else
			{
				nextTexOffset = (nextTexOffset == 0)?2147483646:nextTexOffset;			//LogFile(ERROR_LOG,"TEST nextTexOffset = %ld",nextTexOffset);
				nextRenderOffset = (nextRenderOffset == 0)?2147483646:nextRenderOffset; //LogFile(ERROR_LOG,"TEST nextRenderOffset = %ld",nextRenderOffset);
				nextShaderOffset = (nextShaderOffset == 0)?2147483646:nextShaderOffset; //LogFile(ERROR_LOG,"TEST nextShaderOffset = %ld",nextShaderOffset);
				nextSceneOffset = (nextSceneOffset == 0)?2147483646:nextSceneOffset;	//LogFile(ERROR_LOG,"TEST nextSceneOffset = %ld",nextSceneOffset);

				nextSceneOffset = ( nextSceneOffset < nextTexOffset )?nextSceneOffset:nextTexOffset;
				nextSceneOffset = ( nextSceneOffset < nextRenderOffset )?nextSceneOffset:nextRenderOffset;
				nextSceneOffset = ( nextSceneOffset < nextShaderOffset )?nextSceneOffset:nextShaderOffset;

				nextTexOffset = (nextTexOffset == 2147483646)?0:nextTexOffset;
				nextRenderOffset = (nextRenderOffset == 2147483646)?0:nextRenderOffset;
				nextShaderOffset = (nextShaderOffset == 2147483646)?0:nextShaderOffset;

				if(debugMode)
					LogFile(ERROR_LOG,"--------------------------\nnextSceneOffset: %ld\n--------------------------",nextSceneOffset);
			}

		}

		if( bOffset + nextSceneOffset > ftell( sceneFile ) && nextSceneOffset != 304 )
			LogFile( TEST_LOG, "CHECK UNKNOWN DATA: At offset %ld, but next load offset is %ld",ftell( sceneFile ), bOffset + nextSceneOffset );

	}while( nextSceneOffset != mainHeader.sceneStartHeaderOffset );


	if(debugMode)
		LogFile(ERROR_LOG,"loadScene - Done load, copying data");

	if(!numScenePrimitives)
		return 0;

	sceneData = new scenePrimitive[numScenePrimitives];

	for( k = 0; k < numScenePrimitives; k ++ )
		sceneData[k] = tSP[k];
				
	delete [] tSP;
				
	LogFile( ERROR_LOG, "CHECK: The number of each type of primitive header is:\n\tTexture:\t%ld\n\tShader:\t%ld\n\tRender:\t%ld\n\tPrimitive:\t%ld",l_numTH, l_numSH, l_numRH, l_numPH );
	if(debugMode)
		LogFile(ERROR_LOG,"loadScene - Finished");
	return 1;

}



int SceneMap::loadArcScene( char *filename, int sceneNum )
{
	FILE *infile;
	long offset;

	LogFile(ERROR_LOG,"\n--------------------------------------------------\nSceneMap::loadArcScene - Loading '%s' - scene # %d\n--------------------------------------------------\n",filename,sceneNum);

	if((infile = fopen(filename,"rb")) == NULL)
	{
		LogFile(ERROR_LOG,"SceneMap::loadArcScene:TERMINAL ERROR: Unable to open '%s' for reading.... Exiting function",filename);
		return 0;
	}

	if(!getArcOffset(infile,sceneNum,&offset))
	{
		LogFile(ERROR_LOG,"SceneMap::loadArcScene:TERMINAL ERROR: Unable to get valid index data from '%s'... Exiting function",filename);
		fclose( infile );
		return 0;
	}

	strcpy(sceneFilename, filename );
	
	offset = loadScene( infile, offset );
	fclose( infile );
	return offset;
}


//------------------------------------------------------------------------/
//-- getMinScene                                                        --/
//--   Returns the minimum scene number in a bg*.arc file               --/
//------------------------------------------------------------------------/
int SceneMap::getMinScene( char *filename )
{
	FILE *infile;
	arc_index_data index;
	main_scene_header tempMainHeader;
	int k, res;

	if((infile = fopen(filename,"rb")) == NULL)
	{
		LogFile(ERROR_LOG,"SceneMap::getMinScene:TERMINAL ERROR: Unable to open '%s' for reading.... Exiting function",filename);
		return 0;
	}

	if(!loadArcIndex( infile, &index ))
	{
		LogFile(ERROR_LOG,"SceneMap::loadArcScene:TERMINAL ERROR: Unable to get valid index data from '%s'... Exiting function",filename);
		fclose( infile );
		return 0;
	}

	for( k = 0; k < index.numIndex; k ++ )
	{
		if( index.index[k].size > 512*512*4 )
		{
			fseek( infile, index.index[k].offset, SEEK_SET);
			
			if((res = fread(&tempMainHeader,1,sizeof(main_scene_header),infile)) < sizeof(main_scene_header))
			{
				LogFile(ERROR_LOG,"SceneMap::getMinScene:ERROR: Couldn't read enough bytes for main scene header in alternate tex file path - index #%d (%d of %d read)",k,res,sizeof(main_scene_header));
				debugMainHeader( &tempMainHeader );
			}
			else
			{
				if( isMainSceneHeader(&tempMainHeader))
				{
					fclose( infile );
					return k;
				}
			}
		
		}
	}
	fclose( infile );
	return -1;

}

//------------------------------------------------------------------------/
//-- getMaxScene                                                        --/
//--   Returns the maximum scene number in a bg*.arc file               --/
//------------------------------------------------------------------------/
int SceneMap::getMaxScene( char *filename )
{
	FILE *infile;
	arc_index_data index;

	if((infile = fopen(filename,"rb")) == NULL)
	{
		LogFile(ERROR_LOG,"SceneMap::getMaxScene:TERMINAL ERROR: Unable to open '%s' for reading.... Exiting function",filename);
		return 0;
	}

	if(!loadArcIndex( infile, &index ))
	{
		LogFile(ERROR_LOG,"SceneMap::getMaxScene:TERMINAL ERROR: Unable to get valid index data from '%s'... Exiting function",filename);
		fclose( infile );
		return 0;
	}

	fclose( infile );
	return index.numIndex;
}


//-----------------------------------------------------------------------------/
//-- SceneMap::isMainSceneHeader                                             --/
//--      Using common fields among all scene headers, the current one is    --/
//--      validated, avoiding seg faults                                     --/
//-----------------------------------------------------------------------------/
bool SceneMap::isMainSceneHeader( main_scene_header *h )
{
	if( h->mainHeaderSegMarker == -1
		&& h->mainHeaderSize == 80
		&& h->texSegOffset == h->altTexSegOffset
		&& h->secondaryHeaderSize == 224
		&& h->floatCount == 32
		&& h->transSectID == 1)
	{
		return true;
	}
	return false;
}


//-----------------------------------------------------------------------------/
//-- SceneMap::renderScene                                                   --/
//--      Renders entire level section.  Also transforms all primitives with --/
//--      an associated transformation.                                      --/
//-- ALTERNATE:  Uncomment all '/*REMOVE*/ lines to allow correct rendering  --/
//--      of multiple sections. The section skipping code in the main module --/
//--      will need to be changed - the textures will be incorrectly deleted --/
//-----------------------------------------------------------------------------/
void SceneMap::renderScene( )
{
	int k, j;
	
	glMatrixMode(GL_MODELVIEW);

	glPushMatrix();
	glMultMatrixf(mainHeader.transMat.mat);

	for( k = 0; k < numScenePrimitives; k++ )
	{
		if( sceneData[k].transformNum 
			&& (( onlyVar && !onlyStatic && sceneData[k].transformNum % 2 == 0 ) ||
				( !onlyVar && onlyStatic && sceneData[k].transformNum % 2 == 1 )))
		{

			glPushMatrix();
			for( j = 0; j < numSceneTransform && sceneData[k].transformNum !=  sceneTransform[j].transSectSubID; j ++ );

			if( j != numSceneTransform )
				glLoadMatrixf(sceneTransform[j].transMat.mat);
		//		glMultMatrixf(sceneTransform[j].transMat.mat);
			else
				LogFile(ERROR_LOG,"RENDER ERROR: Couldn't find scene transform for primitive %d with transform num %d",k,sceneData[k].transformNum);
/*NEW*/		sceneData[k].draw();
/*NEW*/		glPopMatrix();
		}
/*NEW*/	else if( !sceneData[k].transformNum )
/*NEW*/	{


/*REMOVE*///		else
/*REMOVE*///		{
/*REMOVE*///			glPushMatrix();
/*REMOVE*///
/*REMOVE*///			glMultMatrixf(mainHeader.transMat.mat);
/*REMOVE*///		}

			sceneData[k].draw();
/*NEW*/	}

//NEW		if( sceneData[k].transformNum )
//NEW			glPopMatrix();
/*REMOVE*///		else
/*REMOVE*///			glPopMatrix();
	}

	glPopMatrix();
	

}


//\-----====[ SH3_Actor   M E M B E R   F U N C T I O N S ]====-----/
GLuint SH3_Actor::loadTex( long offset, char *texName, int tTexIndex, FILE *infile )
{
	long altTexFileOffset = offset;
	long texBatchOffset = 0;
	texture_batch_header texBatch;
	texture_sub_header texHeader;
	BYTE *tBuffer = NULL;
	short *altBuffer = NULL;
	int j,k;
	int res;
	int texIndex = tTexIndex;
	GLuint texID;
	char temp;


	fseek(infile,altTexFileOffset,SEEK_SET);

	if((res = fread(&texBatch,1,sizeof(texBatch),infile)) < sizeof(texture_batch_header))
	{
		LogFile(ERROR_LOG,"SH3_Actor::loadTex:ERROR: Couldn't read enough bytes for texture batch header - index #%d (%d of %d read)\nTry debug anyways...",texIndex,res,sizeof(texture_batch_header));
		debugTextureBatchHeader(&texBatch);
		return 0;
	}

	if(debugMode)
		debugTextureBatchHeader(&texBatch);

	//--[ Make Sure The Index Is In Range ]--/
	if( texIndex >= texBatch.numBatchedTex )
	{
		if( texIndex >= m_mDataHeader.totalModelTex)
		{
			LogFile(ERROR_LOG,"SceneMap::loadTex:ERROR: Texture Number is out of range [%d of %d]",texIndex,texBatch.numBatchedTex-1);
			return 0;
		}
		else
		{
			LogFile(ERROR_LOG,"SH3_Actor::loadTex - WARNING: TexIndex is out of range of batch Header(%d of %d) but is within range of model texture count (%d of %d)",
				texIndex,texBatch.numBatchedTex,texIndex,m_mDataHeader.totalModelTex);
//			while(texIndex >= m_mDataHeader.numTex)
				texIndex = texSeqData[texIndex*2 ]; 
				//texIndex = m_mDataHeader.numTex -1;

				texIndex %= m_mDataHeader.numTex;
			LogFile(ERROR_LOG,"\tSetting value to %d",texIndex);	//texSeqData[texIndex + (m_mDataHeader.offsetTexSeq-m_mDataHeader.offsetTexIndex)/sizeof(long)] % m_mDataHeader.numTex);

//texIndex = m_mDataHeader.numTex -1;
			/*m_mDataHeader.totalModelTex + m_mDataHeader.numTex*/
			//(texIndex-m_mDataHeader.numTex) //(m_mDataHeader.offsetTexSeq-m_mDataHeader.offsetTexIndex)/sizeof(long)
		}
	}
	else if( texIndex == -1 )
	{
		LogFile(ERROR_LOG,"SceneMap::loadTex - NOTE: There is a -1 texture index.  Assuming it is last texture in the alternate tex file");
		texIndex = texBatch.numBatchedTex - 1;
	}
	//else
	//{
	//	texIndex = texSeqData[(m_mDataHeader.offsetTexSeq-m_mDataHeader.offsetTexIndex)/sizeof(long) + texIndex*2 ]; 
	//	texIndex %= m_mDataHeader.numTex;
	//}


	//--[ Reset The Location To After Texture Batch Header ]--/
	altTexFileOffset += sizeof(texture_batch_header);


	//----[ GET FIRST TEX HEADER TO DETERMINE SIZE AND OFFSET OF TEXTURES ]----/
		
	fseek( infile, altTexFileOffset, SEEK_SET);
			
	//----[ FIND THE TEXTURE TO LOAD]----//
	for( k = 0; k < texIndex; k++ )
	{
		fseek( infile, altTexFileOffset + texBatchOffset, SEEK_SET);
		
		if((res = fread(&texHeader,1,sizeof(texHeader),infile)) < sizeof(texture_sub_header))
		{
			LogFile(ERROR_LOG,"SceneMap::loadTex:ERROR: Couldn't read enough bytes for texture header while iterating through textures - texture #%d in (%d of %d bytes read)\nTry debug anyways...",k,res,sizeof(texture_sub_header));
			debugTextureSubHeader(&texHeader);
			return 0;
		}

		texBatchOffset += texHeader.texHeaderSegSize;
	}


	//----[ LOAD HEADER FOR TEXTURE TO USE ]----/
	fseek( infile, altTexFileOffset + texBatchOffset, SEEK_SET);
		
	if((res = fread(&texHeader,1,sizeof(texHeader),infile)) < sizeof(texture_sub_header))
	{
		LogFile(ERROR_LOG,"SceneMap::loadTex:ERROR: Couldn't read enough bytes for actual used texture header (%d of %d)\nTry debug anyways...",res,sizeof(texture_sub_header));
		debugTextureSubHeader(&texHeader);
		return 0;
	}

	if(debugMode)
		debugTextureSubHeader(&texHeader);
	
	if( texHeader.texHeaderSegSize - texHeader.texSize == 128 )
		fseek( infile, altTexFileOffset + texBatchOffset + 128 , SEEK_SET);

	tBuffer = new BYTE[texHeader.width * texHeader.height * 4];

	if(tBuffer == NULL )
	{
		LogFile(ERROR_LOG,"SceneMap::loadTex:ERROR: Unable to allocate memory for texture");
		return 0;
	}

	
	//----[ MAKE SURE TO READ CORRECT NUMBER OF BYTES FROM FILE ]---/
	if( texHeader.texSize / (texHeader.width * texHeader.height) != texHeader.bpp/sizeof(char) )
	{
		LogFile(ERROR_LOG,"SH3_Actor - DATA ERROR: bpp = %d\ttexSize/(width*height) = %d\n\tChanging bpp to %d", 
			texHeader.bpp,texHeader.texSize/(texHeader.width * texHeader.height) * 8,
			texHeader.texSize/(texHeader.width * texHeader.height) * 8);
		texHeader.bpp = texHeader.texSize/(texHeader.width * texHeader.height) * 8;
	}


	//----[ READ TEXTURE DATA ]----/
	//if( texHeader.bpp != 32 )//h->texRenderModifier > 9) //texHeader.f6 != 0 )		//IMAGE IS INTERLACED
	//{
	//	for( j = 0; j < texHeader.height/2; j ++ )
	//	{
	//		fread(&tBuffer[j*texHeader.width*texHeader.bpp/8],sizeof(char),texHeader.width*texHeader.bpp/8,infile);
	//		fread(&tBuffer[(j+texHeader.height/2)*texHeader.width*texHeader.bpp/8],sizeof(char),texHeader.width*texHeader.bpp/8,infile);
	//	}
	//}
	if( /*h->texNum == -1 ) *//*h->texRenderModifier ==*/ 0 )
	{
		for( j = 0; j < texHeader.height; j++ )
		{
			for( k = 0; k < texHeader.width; k ++ )
			{
				tBuffer[(j*texHeader.width+k)*4+0] = 0;
				tBuffer[(j*texHeader.width+k)*4+1] = 0;
				tBuffer[(j*texHeader.width+k)*4+2] = (((j / 32 ) % 2) ^ ((k / 32) % 2 )) * 255;
				tBuffer[(j*texHeader.width+k)*4+3] = 255;
			}
		}
	}
	else
		fread(tBuffer,sizeof(BYTE),texHeader.width * texHeader.height * texHeader.bpp/8, infile);

	//---[ CONVERT BUFFER IF BPP IS NOT 32 ]---/
	if( texHeader.bpp == 16 )
	{
		altBuffer = new short[texHeader.width * texHeader.height * texHeader.bpp/sizeof(short)];

		if(debugMode)
			LogFile(ERROR_LOG,"sceneMap::loadTex - Converting texture w/ %d bpp",texHeader.bpp);
		memcpy(altBuffer,tBuffer,texHeader.width * texHeader.height * texHeader.bpp/8);

		for( j = 0; j < texHeader.height; j ++ )
		{
			for( k = 0; k < texHeader.width; k ++ )
			{
				tBuffer[(j*texHeader.width+k)*4+0] = convertRed(altBuffer[j*texHeader.width+k]);
				tBuffer[(j*texHeader.width+k)*4+1] = convertGreen(altBuffer[j*texHeader.width+k]);
				tBuffer[(j*texHeader.width+k)*4+2] = convertBlue(altBuffer[j*texHeader.width+k]);
				tBuffer[(j*texHeader.width+k)*4+3] = convertAlpha(altBuffer[j*texHeader.width+k]);
			}
		}

		SAFEDELETE(altBuffer);
	}
	else if( texHeader.bpp == 24 )
	{
		if( texHeader.texSize / (texHeader.width * texHeader.height) * 8 == 32 )
			LogFile(ERROR_LOG,"SH3_Actor::loadTex - WARNING: bpp = 24, but actual size of pixel = 32 bpp");
		else
		{
			BYTE *cBuffer = new BYTE[ texHeader.width * texHeader.height * 4];

			memcpy(cBuffer,tBuffer, texHeader.bpp/8 * texHeader.height * texHeader.width);

			for( j = 0; j < texHeader.height; j++ )
			{
				for( k = 0; k < texHeader.width; k++ )
				{
					tBuffer[((j*texHeader.width)+k)*4+0] = cBuffer[((j*texHeader.width)+k)*3+0];
					tBuffer[((j*texHeader.width)+k)*4+1] = cBuffer[((j*texHeader.width)+k)*3+1];
					tBuffer[((j*texHeader.width)+k)*4+2] = cBuffer[((j*texHeader.width)+k)*3+2];
					if( tBuffer[((j*texHeader.width)+k)*4+0] == 0 
						&& tBuffer[((j*texHeader.width)+k)*4+1]
						&& tBuffer[((j*texHeader.width)+k)*4+2] )
						tBuffer[((j*texHeader.width)+k)*4+3] = 0;
					else
						tBuffer[((j*texHeader.width)+k)*4+3] = 255;
				}
			}

			delete [] cBuffer;
		}
	}

	
	//switch( h->texRenderModifier  )
	//{
	//	case 3: flipVertical(texHeader.width,texHeader.height,tBuffer); break;
	//	case 2: flipHorizontal(texHeader.width,texHeader.height,tBuffer); break;
	//}

	glGenTextures ( 1, &texID );
	if(texID < 1 )
		LogFile(ERROR_LOG,"SceneMap::loadTex: ERROR: Couldn't generate more textures!!!!! glGenTextures returned %d as the tex ID",texID);
	else
	{
		char outtexName[128];

		if( dumpModel )
		{
			sprintf(outtexName,"%s.tga",texName);
			saveTexture(&texHeader,tBuffer,outtexName);
		}


			//----[ CORRECTLY ORDER COLORS ]----/
		for( j = 0; j < texHeader.height; j ++ )
		{
			for( k = 0; k < texHeader.width; k ++ )
			{
				temp = tBuffer[(j*texHeader.width+k)*4+0];
				tBuffer[(j*texHeader.width+k)*4+0] = tBuffer[(j*texHeader.width+k)*4+2];
				tBuffer[(j*texHeader.width+k)*4+2]=temp;
//				if( tBuffer[ (j*texHeader.width+k)*4+0 ] == 0 
//					&& tBuffer[ (j*texHeader.width+k)*4+1 ] == 0
//					&& tBuffer[ (j*texHeader.width+k)*4+2 ] == 0 )
//					tBuffer[ (j*texHeader.width+k)*4+3 ] = 0;
			}
		}

		//----[ CREATE TEXTURE IN VIDEO MEMORY ]----/
		glBindTexture ( GL_TEXTURE_2D, texID );
		glTexImage2D ( GL_TEXTURE_2D, 0, GL_RGBA, texHeader.width, texHeader.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, tBuffer );  

		glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
		glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexEnvi ( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE,GL_REPLACE);

	}

	delete [] tBuffer;
	

	return texID;
}



int SH3_Actor::findAnimFile( long modelNum, char *filename )
{
	long firstLong;
	FILE *infile;
	int k;
	arc_index_data indexData;

	if(( infile = fopen( filename, "rb" ))== NULL )
	{
		LogFile(ERROR_LOG,"SH3_Actor::findAnimFile - ERROR: Unable to open file '%s': %s",filename,strerror(errno));
		return -1;
	}

	if(!loadArcIndex(infile,&indexData))
	{
			LogFile(ERROR_LOG,"SH3_Actor::fileAnimFile - ERROR: Unable to read index from file '%s'",filename);
		fclose( infile );
		return -1;
	}

	for( k = 0; k < indexData.numIndex; k++ )
	{
		fseek( infile, indexData.index[k].offset,SEEK_SET);

		if(fread( &firstLong, 1, sizeof(long),infile) == sizeof(long))
		{
			if( modelNum == firstLong )
			{
				fclose( infile );
				return k;
			}
		}
		else
			LogFile(ERROR_LOG,"SH3_Actor::findAnimFile - ERROR: Unable to read data at offset %ld : %s",
				indexData.index[k].offset,strerror(errno));
	}

	fclose( infile );
	return -1;
}



void SH3_Actor::draw( int modelPart )
{
	int k;
	int j;
	int startPt, endPt;
	vertex4f *dispVerts = NULL;
	vertex ones(1.0f,1.0f,1.0f);
	matrix *matArray = NULL;
	char *lastSlash = strrchr( modelFilename, '\\' );
	static bool printData = true;

	vertex4f tv1,tv2,tv3,tv4;

	char outFile[128];
	//if(printData)
	//	LogFile(ERROR_LOG,"NumPrim: %d",numPrimitives);

	if( modelPart < 0 || modelPart > numPrimitives )
	{
		startPt = 0;
		endPt = numPrimitives;
	}
	else
	{
		startPt = modelPart;
		endPt = startPt + 1;
	}

	for( j = startPt; j < endPt; j++ )
	{
//		if(printData)
//			LogFile(ERROR_LOG,"Num Verts: %d\tNum Index: %d\tTex ID: %d",m_pmPrimitive[j].vertHeader.numVerts,m_pmPrimitive[j].vertHeader.numIndex,m_pmPrimitive[j].texID);
		dispVerts = new vertex4f[m_pmPrimitive[j].vertHeader.numVerts];

		matArray = new matrix[ m_pmPrimitive[j].vertHeader.numSeq1 + m_pmPrimitive[j].vertHeader.numSeq2];

		for( k = 0; k < m_pmPrimitive[j].vertHeader.numSeq1; k ++ )
		{
			matArray[k] = matSet1[m_pmPrimitive[j].seqData1[k]];
			//matArray[k].transpose();
			if( displayMatrix )
				drawBasisMatrix( &( matArray[k] ), 6.0f );
		}
		for( k = m_pmPrimitive[j].vertHeader.numSeq1; k < m_pmPrimitive[j].vertHeader.numSeq1 + m_pmPrimitive[j].vertHeader.numSeq2; k++ )
		{
			matArray[k] = matSet1[mSeq2[(m_pmPrimitive[j].seqData2[k-m_pmPrimitive[j].vertHeader.numSeq1])*2+1]] * matSet2[m_pmPrimitive[j].seqData2[k-m_pmPrimitive[j].vertHeader.numSeq1]];
			//matArray[k] = matSet2[m_pmPrimitive[j].seqData2[k-m_pmPrimitive[j].vertHeader.numSeq1]];
			//matArray[k].transpose();
			if( displayMatrix )
				drawBasisMatrix( &( matArray[k] ), 6.0f );
		}


//		if( printData )//&& j == numPrimitives - 0 )
//		{
//			LogFile(ERROR_LOG,"\n*********************************** D E B U G I N G   M A T R I C E S **********************");
//			for( k = 0; k < m_pmPrimitive[j].vertHeader.numSeq1 + m_pmPrimitive[j].vertHeader.numSeq2; k++)
//			{
//				debugMatrix(matArray[k]);
//			}
//		}
	

		for( k = 0; k < m_pmPrimitive[j].vertHeader.numVerts; k++ )
		{
			

		//	if(printData)
		//	{
		//		LogFile(ERROR_LOG,"%d: r0w: %6.6f\n  ( %6.6f %6.6f %6.6f) ( %6.6f %6.6f %6.6f ) [ %d %d %d %d ] ",k,r0w,m_pmPrimitive[j].verts[k].vert.x,m_pmPrimitive[j].verts[k].vert.y,m_pmPrimitive[j].verts[k].vert.z,m_pmPrimitive[j].verts[k].v1.x,m_pmPrimitive[j].verts[k].v1.y,m_pmPrimitive[j].verts[k].v1.z,
		//			m_pmPrimitive[j].verts[k].color.r,m_pmPrimitive[j].verts[k].color.g,m_pmPrimitive[j].verts[k].color.b,m_pmPrimitive[j].verts[k].color.a);
		//	}
			if( m_pmPrimitive[j].vertHeader.numSeq1 + m_pmPrimitive[j].vertHeader.numSeq2 > 1 )
			{
				if( m_pmPrimitive[j].verts == NULL && m_pmPrimitive[j].altVerts != NULL )
				{
					LogFile(TEST_LOG,"SH3_Actor::draw - TERMINAL ERROR: Can't render prim #%d - Small size verts have multi-sequence matricies w/o selector data",j);
					break;
				}

				float	r0w = -VDOT( ones, m_pmPrimitive[j].verts[k].v1 ) + 1.0f;
				matrix temp = matArray[m_pmPrimitive[j].verts[k].color.b];
				matrix temp2= matArray[m_pmPrimitive[j].verts[k].color.g];
				matrix temp3= matArray[m_pmPrimitive[j].verts[k].color.r];
				matrix temp4= matArray[m_pmPrimitive[j].verts[k].color.a];

				if(printData)
				{
					if(m_pmPrimitive[j].verts[k].color.r > m_pmPrimitive[j].vertHeader.numSeq1 + m_pmPrimitive[j].vertHeader.numSeq2
						|| m_pmPrimitive[j].verts[k].color.g > m_pmPrimitive[j].vertHeader.numSeq1 + m_pmPrimitive[j].vertHeader.numSeq2
						|| m_pmPrimitive[j].verts[k].color.b > m_pmPrimitive[j].vertHeader.numSeq1 + m_pmPrimitive[j].vertHeader.numSeq2
						|| m_pmPrimitive[j].verts[k].color.a > m_pmPrimitive[j].vertHeader.numSeq1 + m_pmPrimitive[j].vertHeader.numSeq2)
						LogFile(ERROR_LOG,"TERMINAL ERROR::: The matrix is out of range [%d %d %d %d]",
							m_pmPrimitive[j].verts[k].color.r,m_pmPrimitive[j].verts[k].color.g,
							m_pmPrimitive[j].verts[k].color.b,m_pmPrimitive[j].verts[k].color.a);
				}
	
	/*				if(printData && j == numPrimitives - 0 )
				{
					LogFile(ERROR_LOG,"\nVertex %d\n----------------\nr: %d\tg: %d\tb: %d\ta: %d",k,
						m_pmPrimitive[j].verts[k].color.r,m_pmPrimitive[j].verts[k].color.g,
						m_pmPrimitive[j].verts[k].color.b,m_pmPrimitive[j].verts[k].color.a);
					debugVertex(m_pmPrimitive[j].verts[k].vert);
					debugMatrix(temp);
					debugMatrix(temp2);
				}
				tv1 = (temp * m_pmPrimitive[j].verts[k].vert);
				if(printData && j == numPrimitives - 0 )
					debugVertex4f(tv1);
				tv1 = tv1 * m_pmPrimitive[j].verts[k].v1.x;
				if(printData && j == numPrimitives - 0 )
					debugVertex4f(tv1);
				tv2 = (temp2 * m_pmPrimitive[j].verts[k].vert);
				if(printData && j == numPrimitives - 0 )
					debugVertex4f(tv2);
				tv2 = tv2 * m_pmPrimitive[j].verts[k].v1.y;
				if(printData && j == numPrimitives - 0 )
					debugVertex4f(tv2);
	
				tv3 = (temp3 * m_pmPrimitive[j].verts[k].vert);
				if(printData && j == numPrimitives - 0 )
					debugVertex4f(tv3);
				tv3 = tv3 * m_pmPrimitive[j].verts[k].v1.z;
				if(printData && j == numPrimitives - 0 )
					debugVertex4f(tv3);

				tv4 = (temp4 * m_pmPrimitive[j].verts[k].vert);
				if(printData && j == numPrimitives - 0 )
					debugVertex4f(tv4);
				tv4 = tv4 * r0w;
				if(printData && j == numPrimitives - 0 )
					debugVertex4f(tv4);
					

				dispVerts[k] = tv1 + tv2 + tv3 + tv4;
				if(printData && j == numPrimitives - 0 )
					debugVertex4f(dispVerts[k]);
*/
				dispVerts[k]=	(temp * m_pmPrimitive[j].verts[k].vert) * m_pmPrimitive[j].verts[k].v1.x +
								(temp2 * m_pmPrimitive[j].verts[k].vert) * m_pmPrimitive[j].verts[k].v1.y +
								(temp3 * m_pmPrimitive[j].verts[k].vert) * m_pmPrimitive[j].verts[k].v1.z +
								(temp4 * m_pmPrimitive[j].verts[k].vert) * r0w;

				dispVerts[k].w = 1.0f;
			}
			else //---[ THERE IS ONLY ONE MATRIX AND SMALL SIZE VERTS ]---/
			{
				if( m_pmPrimitive[j].verts == NULL && m_pmPrimitive[j].altVerts != NULL )
				{
					//if(printData)LogFile(TEST_LOG,"SH3_Actor::draw - Got Here: Can render prim #%d ? - Small size verts have single matrix",j);
						
					dispVerts[k] = matArray[0] * m_pmPrimitive[j].altVerts[k].vert;
					dispVerts[k].w = 1.0f;
				}
				else if( m_pmPrimitive[j].verts != NULL && m_pmPrimitive[j].altVerts == NULL )
				{
					dispVerts[k] = matArray[0] * m_pmPrimitive[j].verts[k].vert;
					dispVerts[k].w = 1.0f;
				}
				else
					LogFile(ERROR_LOG,"SH3_Actor::draw - TERMINAL ERROR: Prim %d - VertSize %d",j,m_pmPrimitive[j].vertSize);

//				if(printData && k == 0)LogFile(ERROR_LOG,"Here: after mult [%d] - VertSIze %d",k,m_pmPrimitive[j].vertSize);
			}

		}

//***********  CHECKING MODEL RENDERING *********
		if( j >= m_mDataHeader.numVertexArrays )	//m_pmPrimitive[j].texModify == 4 )
		{
			glAlphaFunc(GL_GEQUAL, 0.6f);
			glEnable(GL_ALPHA_TEST);

		}
//************ END CHECK MODEL RENDER *************/

		glFrontFace(GL_CCW);
//		checkGLerror(__LINE__,__FILE__,"Before Render");
		glActiveTextureARB(GL_TEXTURE0_ARB);
//		checkGLerror(__LINE__,__FILE__,"Before 1");
		glBindTexture(GL_TEXTURE_2D,m_pmPrimitive[j].texID);
//		checkGLerror(__LINE__,__FILE__,"Before 2");
		glEnable(GL_TEXTURE_2D);

		glEnableClientState(GL_VERTEX_ARRAY);
//		checkGLerror(__LINE__,__FILE__,"Before 3");
		glVertexPointer(4, GL_FLOAT, sizeof(vertex4f) , &dispVerts[0]);

		glClientActiveTextureARB(GL_TEXTURE0_ARB);
//		checkGLerror(__LINE__,__FILE__,"Before 4");
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);
//		checkGLerror(__LINE__,__FILE__,"Before 5");

		if( m_pmPrimitive[j].vertSize == 48 )
			glTexCoordPointer(2, GL_FLOAT, m_pmPrimitive[j].vertSize, &m_pmPrimitive[j].verts[0].tc);
		else if( m_pmPrimitive[j].vertSize == 32 )
			glTexCoordPointer(2, GL_FLOAT, m_pmPrimitive[j].vertSize, &m_pmPrimitive[j].altVerts[0].tc);
//		checkGLerror(__LINE__,__FILE__,"Before 6");

		glDrawElements(GL_TRIANGLE_STRIP, m_pmPrimitive[j].vertHeader.numIndex, GL_UNSIGNED_INT, m_pmPrimitive[j].indicies);
		checkGLerror(__LINE__,__FILE__,"Before 7");

		glDisableClientState(GL_VERTEX_ARRAY);
//		checkGLerror(__LINE__,__FILE__,"Before 8");
		glDisableClientState(GL_TEXTURE_COORD_ARRAY);

		if( j >= m_mDataHeader.numVertexArrays )	//m_pmPrimitive[j].texModify == 4 )
		{
			glDisable(GL_ALPHA_TEST);
		}

		delete [] dispVerts;
		delete [] matArray;

//		checkGLerror(__LINE__,__FILE__,"After Render");
		//glDisableClientState(GL_NORMAL_ARRAY);
	}
	printData = false;
}


//----------------------------------------------------------------------------------/
//-- SH3_Actor::isModel                                                           --/
//--    Checks a model_base_header to make sure certain attributes match what is  --/
//--    expected of the base header for a model                                   --/
//----------------------------------------------------------------------------------/
bool SH3_Actor::isModel( model_base_header *pHeader )
{
	if( pHeader->numTex > 5 || pHeader->always1_1 != 1 || pHeader->texOffset < 0 
		|| pHeader->baseHeaderSize % 16 != 0 || pHeader->modelID < 0 || pHeader->texOffset < 748 )
		return false;
	return true;
}


//----------------------------------------------------------------------------------/
//-- SH3_Actor::loadAnimFile                                                      --/
//--    Loads the animation file at <offset> for a total size of <size>           --/
//--  NOTE: Temporarily, this just prints out what it loads, and doesn't save any --/
//--    Data read in.                                                             --/
//----------------------------------------------------------------------------------/
int  SH3_Actor::loadAnimFile( FILE *infile, long offset, long size )
{
	int res;
	sh3_base_anim	baseAnim;
	sh3_alt1_anim	altAnim1;
	sh3_alt2_anim	altAnim2;
	BYTE *tBuffer = NULL;
	long sizeRead = 0;
	long *lDat;


	//---[ VALIDATE INPUT DATA ]---/
	if( size < 72 || offset < 0 )
	{
		LogFile(ERROR_LOG,"SH3_Actor::loadAnimFile - ERROR: size %ld or offset %ld is invalid",size,offset);
		return 0;
	}

	LogFile(ERROR_LOG,"TEST: (size - sizeRead)/sizeof(long) = %ld",(size - sizeRead)/sizeof(long));

	//---[ READ IN ANIMATION SUB-FILE ]---/
	fseek(infile,offset + 4,SEEK_SET);

	tBuffer = new BYTE[size];

	if((res = fread(tBuffer,sizeof(BYTE),size,infile)) < size )
	{
		LogFile(ERROR_LOG,"SH3_Actor::loadAnimFile - ERROR: Could only read %d of %d bytes: %s",res,size,strerror(errno));
		SAFEDELETE(tBuffer);
		return 0;
	}

	//---[ CONVERT BUFFER TO ANIMATION STRUCTURES ]---/
	while( sizeRead < size )
	{
		lDat = (long *)&(tBuffer[sizeRead]);

		//if( lDat[0] & 0x00ffff0f == 0x0099990a )//0x919999aa )		//---[ Load in sh3_base_anim structure ]---/
		//{
		//	memcpy(&baseAnim,lDat,sizeof(sh3_base_anim));
		//	debugBaseAnim(&baseAnim);
		//	sizeRead += sizeof(sh3_base_anim);
		//}else
		if( lDat[0] == 0x99999999 && lDat[13] == 0x99999999 )	//---[ Load in sh3_alt1_anim struct ]---/
		{
			memcpy( &altAnim1, lDat, sizeof(sh3_alt1_anim));
			debugAltAnim1(&altAnim1);
			sizeRead += sizeof(sh3_alt1_anim);
		}
		else if( lDat[0] == 0x99999999 && lDat[13] != 0x99999999 )	//---[ Load in sh3_alt2_anim struct ]---/
		{
			memcpy( &altAnim2, lDat, sizeof(sh3_alt2_anim));
			debugAltAnim2(&altAnim2);
			sizeRead += sizeof(sh3_alt2_anim);
		}
		else //if( lDat[0] & 0x00ffff0f == 0x0099990a )//0x919999aa )		//---[ Load in sh3_base_anim structure ]---/
		{
			memcpy(&baseAnim,lDat,sizeof(sh3_base_anim));
			debugBaseAnim(&baseAnim);
			sizeRead += sizeof(sh3_base_anim);
		}
		//else
		//{
		//	LogFile(ERROR_LOG,"SH3_Actor::loadAnimFile - ERROR: Can't determine structure type - Dumping rest of data");
		//	for( k = 0; k < (size - sizeRead)/sizeof(long) ; k++ )
		//		debugAll(lDat[k]);
		//	sizeRead = size;
		//}
		LogFile(ERROR_LOG,"TEST: sizeRead = %ld",sizeRead);
	}

	SAFEDELETE(tBuffer);
	return 1;

}

//----------------------------------------------------------------------------------/
//-- SH3_Actor::loadAnim                                                          --/
//--    Searches through subfiles for the <segNum>th animation subfile for the    --/
//--    animation file w/ the modelID that matches the model's modelID.           --/
//----------------------------------------------------------------------------------/
int  SH3_Actor::loadAnim( FILE *infile, long modelID, long segNum )
{
	long tempModelID;
	arc_index_data index;
	int k;
	int res;
	int segCount = 0;
	bool foundSeg = false;

	if(!loadArcIndex(infile, &index))
	{
		LogFile(ERROR_LOG,"SH3_Actor::loadAnim - ERROR: Unable to load arc file index");
		return 0;
	}

	for( k = 0; k < index.numIndex; k++ )
	{
		if( index.index[k].size > 0 )
		{
			fseek(infile,index.index[k].offset,SEEK_SET);
			if((res = fread(&tempModelID,1,sizeof(long),infile)) < sizeof(long))
			{
				LogFile(ERROR_LOG,"SH3_Actor::loadAnim - ERROR: Couldn't read modelID for segment %d  offset: %ld - %s",k,index.index[k].offset,strerror(errno));
				return 0;
			}

			if( tempModelID == modelID )
			{
				
				if( segNum == segCount )
				{
					if(!loadAnimFile( infile,index.index[k].offset, index.index[k].size ))
						return 0;
					foundSeg = true;
					k = index.numIndex;
				}
				++segCount;
			}
		}
	}

	if( !foundSeg )
		LogFile(ERROR_LOG,"SH3_Actor::loadAnim - ERROR: Didn't find segment %d for modelID %ld",segNum,modelID);
	
	return foundSeg;
}
	
					
//----------------------------------------------------------------------------------/
//-- SH3_Actor::loadAnim                                                          --/
//--    Opens <filename>, and then passes the data to the overloaded version of   --/
//--    loadAnim                                                                  --/
//----------------------------------------------------------------------------------/
int  SH3_Actor::loadAnim( char *filename, long modelID, long segNum )
{
	FILE *infile;

	if((infile = fopen(filename,"rb")) == NULL )
	{
		LogFile(ERROR_LOG,"SH3_Actor::loadAnim - ERROR: Couldn't open file '%s' in base version of loadAnim: %s",filename,strerror(errno));
		return 0;
	}

	return loadAnim(infile,modelID,segNum);
}



//----------------------------------------------------------------------------------/
//-- SH3_Actor::loadModel                                                         --/
//--    Loads a model from an arc file based upone the filename and model number. --/
//--    Calls the loadModelPrimitive to load each tri-strip section of the model  --/
//----------------------------------------------------------------------------------/
int  SH3_Actor::loadModel( char *filename, int modelNum )
{
	FILE *infile;
	int dataCount;
	int res;
	int numTempPrim = 0;
	int k;
	long curOffset;
	BYTE *unknownData;

	//----[ OPEN FILE AND GET INDEX INFO ]----/
	LogFile(ERROR_LOG,"\n--------------------------------------------------\nSH3_Actor::loadModel - Loading '%s' - model # %d\n--------------------------------------------------\n",filename,modelNum);

	//----[ Release Old Data ]----/
	releaseModelData();

	matrix *tMatSet1 = NULL;
	matrix *tMatSet2 = NULL;


	if((infile = fopen(filename,"rb")) == NULL)
	{
		LogFile(ERROR_LOG,"SH3_Actor::loadModel:TERMINAL ERROR: Unable to open '%s' for reading.... Exiting function",filename);
		return 0;
	}

	if(!getArcOffset( infile, modelNum,&baseOffset))
	{
		LogFile(ERROR_LOG,"SH3_Actor::loadModel:TERMINAL ERROR: Unable to get valid index data from '%s'... Exiting function",filename);
		fclose(infile);
		return 0;
	}

	curOffset = baseOffset;

	strcpy(modelFilename, filename );


	//----[ LOAD BASE AND DATA HEADERS ]----/

	fseek( infile, curOffset, SEEK_SET );

	if(( res = fread( &m_mBaseHeader, 1, sizeof(model_base_header),infile)) < sizeof( model_base_header ))
	{
		LogFile(ERROR_LOG,"SH3_Actor::loadModel - ERROR: Didn't read enough model_base_header data (%d of %d bytes read)",res,sizeof(model_base_header));
		fclose(infile);
		if( debugMode )debugModelBaseHeader( &m_mBaseHeader );
		return 0;
	}

	if(!isModel( &m_mBaseHeader ))
	{
		LogFile(ERROR_LOG,"SH3_Actor::loadModel - ERROR: This does not appear to be a model");
		fclose(infile);
		if( debugMode )debugModelBaseHeader( &m_mBaseHeader );
		return 0;
	}

	dataCount = (m_mBaseHeader.baseHeaderSize - sizeof(model_base_header))/sizeof(model_size_offset);

	m_pmSizeOffsets = new model_size_offset[ dataCount ];

	if(( res = fread( m_pmSizeOffsets, 1, dataCount * sizeof(model_size_offset),infile )) < dataCount * sizeof(model_size_offset))
	{
		LogFile(ERROR_LOG,"SH3_Actor::loadModel - ERROR: Couldn't read enough data for size and offsets (%d of %d bytes read)",res, dataCount * sizeof(model_size_offset));
		fclose(infile);
		if( debugMode )debugModelBaseHeader( &m_mBaseHeader );
		return 0;
	}

	if(( res = fread( &m_mDataHeader, 1, sizeof(model_data_header), infile)) < sizeof(model_data_header))
	{
		LogFile(ERROR_LOG,"SH3_Actor::loadModel - ERROR: Couldn't read enough data for model data header (%d of %d bytes read)",res,sizeof(model_data_header));
		fclose(infile);
		if( debugMode )debugModelBaseHeader( &m_mBaseHeader );
		if( debugMode )debugModelDataHeader( &m_mDataHeader );
		return 0;
	}

	if(debugMode)
	{
		debugModelBaseHeader( &m_mBaseHeader );
		LogFile( ERROR_LOG,"  --Model Size Offsets (# %ld)--",dataCount);
		for( k = 0; k < dataCount; k++ )
			debugLong(m_pmSizeOffsets[ k ].headerSizeTexOffsets);
		debugModelDataHeader( &m_mDataHeader );
	}

/*********************** UNKNOWN FIELD ANALYSIS *********************************/
	if( debugMode )
		LogFile( DATA_LOG,"\nFILE \"%s\"\n\tBASE: f1 %ld  always1 %ld\tDATA: always3 %ld  f1 %ld  qa4[ %ld %4ld ] sizeUnknown1 %5ld  q6 %3ld  sizeUnknown2 %5ld  q8 %ld  sizeUnknown3 %5ld  sizeUnknown4 %5ld  always1 %f",
			filename,m_mBaseHeader.f1,m_mBaseHeader.always1_1,m_mDataHeader.always3_1,m_mDataHeader.numAltVertArrays,m_mDataHeader.qa4[0],m_mDataHeader.qa4[1],m_mDataHeader.offsetQ7-m_mDataHeader.offsetQ5,
			m_mDataHeader.q6,m_mDataHeader.offsetQ9-m_mDataHeader.offsetQ7,m_mDataHeader.q8,m_mDataHeader.offsetQ10-m_mDataHeader.offsetQ9,m_mDataHeader.offsetQ11-m_mDataHeader.offsetQ10,m_mDataHeader.always1_1);
/*********************** UNKNOWN FIELD ANALYSIS *********************************/
	curOffset += m_mBaseHeader.baseHeaderSize;


	//----[ LOAD ANIMATION MATRIX DATA ]----/

	matSet1 = new matrix[ m_mDataHeader.numMatSet1 ];
	tMatSet1 = new matrix[ m_mDataHeader.numMatSet1 ];
	fseek(infile,curOffset + m_mDataHeader.offsetMatSet1,SEEK_SET);

	if((res = fread(tMatSet1,1,sizeof(matrix) * m_mDataHeader.numMatSet1,infile)) < sizeof(matrix) * m_mDataHeader.numMatSet1)
	{
		LogFile(ERROR_LOG,"SH3_Actor::loadModel - ERROR: Couldn't read enough data for matrix set 1 (%d of %d bytes read)",res,sizeof(matrix) * m_mDataHeader.numMatSet1);
		fclose(infile);
		debugModelBaseHeader( &m_mBaseHeader );
		debugModelDataHeader( &m_mDataHeader );
		return 0;
	}

	matSet2 = new matrix[ m_mDataHeader.numMatSet2 ];
	tMatSet2 = new matrix[ m_mDataHeader.numMatSet2 ];
	fseek(infile,curOffset + m_mDataHeader.offsetMatSet2,SEEK_SET);

	if((res = fread(tMatSet2,1,sizeof(matrix) * m_mDataHeader.numMatSet2,infile)) < sizeof(matrix) * m_mDataHeader.numMatSet2)
	{
		LogFile(ERROR_LOG,"SH3_Actor::loadModel - ERROR: Couldn't read enough data for matrix set 2 (%d of %d bytes read)",res,sizeof(matrix) * m_mDataHeader.numMatSet2);
		fclose(infile);
		if( debugMode )debugModelBaseHeader( &m_mBaseHeader );
		if( debugMode )debugModelDataHeader( &m_mDataHeader );
		return 0;
	}


	//----[ LOAD SEQUENCE DATA ]----/

	dataCount = m_mDataHeader.offsetMatSeq2 - m_mDataHeader.offsetMatSeq1;
	numSeq1 = dataCount;
	mSeq1 = new char[ dataCount ];
	fseek(infile,curOffset + m_mDataHeader.offsetMatSeq1,SEEK_SET);
	if((res = fread(mSeq1,1,dataCount,infile)) < dataCount)
	{
		LogFile(ERROR_LOG,"SH3_Actor::loadModel - ERROR: Couldn't read enough data for matrix seq 1 (%d of %d bytes read)",res,dataCount);
		fclose(infile);
		if( debugMode )debugModelBaseHeader( &m_mBaseHeader );
		if( debugMode )debugModelDataHeader( &m_mDataHeader );
		return 0;
	}

	dataCount = m_mDataHeader.offsetMatSet2 - m_mDataHeader.offsetMatSeq2;
	numSeq2 = dataCount;
	mSeq2 = new char[ dataCount ];
	fseek(infile,curOffset + m_mDataHeader.offsetMatSeq2,SEEK_SET);
	if((res = fread(mSeq2,1,dataCount,infile)) < dataCount)
	{
		LogFile(ERROR_LOG,"SH3_Actor::loadModel - ERROR: Couldn't read enough data for matrix seq 2 (%d of %d bytes read)",res,dataCount);
		fclose(infile);
		if( debugMode )debugModelBaseHeader( &m_mBaseHeader );
		if( debugMode )debugModelDataHeader( &m_mDataHeader );
		return 0;
	}


	//----[ COMPUTE MODEL POSE MATRICIES FROM MATRIX SET 1 & 2, AND SEQUENCE SET 2 ]----/
	for( k = 0; k < m_mDataHeader.numMatSet1; k++)
		tMatSet1[k].transpose();

	for( k = 0; k < m_mDataHeader.numMatSet2; k++)
		tMatSet2[k].transpose();

	for( k = 0; k < m_mDataHeader.numMatSet2; k++)
	{
//ORIG ->   matSet2[k] = tMatSet1[mSeq2[k*2+1]] * tMatSet2[k];//mSeq2[k*2+1]];
		matSet2[ k ] = tMatSet2[ k ];

		//matSet2[k].transpose();
	}
	for( k = 0; k < m_mDataHeader.numMatSet1; k++)
	{
		matSet1[k] = tMatSet1[k];// * tMatSet2[mSeq1[k*2+1]];
		//matSet1[k].transpose();
	}
	delete [] tMatSet1;
	delete [] tMatSet2;
#define debugMatrix2( v ) ( LogFile(ERROR_LOG,"\t%s: \n\t[[ %f\t%f\t%f\t%f ]\n\t [ %f\t%f\t%f\t%f ]\n\t [ %f\t%f\t%f\t%f ]\n\t [ %f\t%f\t%f\t%f ]]",#v,v.mat[0],v.mat[1],v.mat[2],v.mat[3],v.mat[4],v.mat[5],v.mat[6],v.mat[7],v.mat[8],v.mat[9],v.mat[10],v.mat[11],v.mat[12],v.mat[13],v.mat[14],v.mat[15]))
	//----[ DEBUG DATA - PRINT OUT MATRICIES AND SEQUNCE DATA ]----/
	for( k = 0; k < m_mDataHeader.numMatSet1; k++)
	{
		//matSet1[k].transpose();
		if( debugMode )debugMatrix2(matSet1[k]);
		//matSet1[k].transpose();
	}
	for( k = 0; k < m_mDataHeader.numMatSet2; k++)
	{
		//matSet2[k].transpose();
		if( debugMode )debugMatrix2(matSet2[k]);
		//matSet2[k].transpose();
	}

	dataCount = (m_mDataHeader.offsetTexSeq - m_mDataHeader.offsetTexIndex)/sizeof(long);
	numTexIndex = dataCount;
	texIndexData = new long[dataCount];

	fseek(infile, curOffset + m_mDataHeader.offsetTexIndex, SEEK_SET);
	if((res = fread(texIndexData,1,sizeof(long)*dataCount,infile)) < sizeof(long)*dataCount)
	{
		LogFile(ERROR_LOG,"SH3_Actor::loadModel - ERROR: Couldn't read enough data for texIndexData (%d of %d bytes read)",res,sizeof(long)*dataCount);
		fclose(infile);
		if( debugMode )debugModelBaseHeader( &m_mBaseHeader );
		if( debugMode )debugModelDataHeader( &m_mDataHeader );
		return 0;
	}

	dataCount = (m_mDataHeader.offsetQ5 - m_mDataHeader.offsetTexSeq)/sizeof(long);
	numTexSeq = dataCount;
	texSeqData = new long[dataCount];

	fseek(infile, curOffset + m_mDataHeader.offsetTexSeq, SEEK_SET);
	if((res = fread(texSeqData,1,sizeof(long)*dataCount,infile)) < sizeof(long)*dataCount)
	{
		LogFile(ERROR_LOG,"SH3_Actor::loadModel - ERROR: Couldn't read enough data for texSeqData (%d of %d bytes read)",res,sizeof(long)*dataCount);
		fclose(infile);
		if( debugMode )debugModelBaseHeader( &m_mBaseHeader );
		if( debugMode )debugModelDataHeader( &m_mDataHeader );
		return 0;
	}

	if( debugMode )
	{
		LogFile(ERROR_LOG,"---------------SEQ DATA 1---------------");
		for( k = 0; k < numSeq1; k++ )
			LogFile(ERROR_LOG,"\t%d:\t%d",k,mSeq1[k]);
		LogFile(ERROR_LOG,"---------------MATRIX SEQ DATA 2---------------");
		for( k = 0; k < numSeq2; k++ )
			LogFile(ERROR_LOG,"\t%d:\t%d",k,mSeq2[k]);
		LogFile(ERROR_LOG,"---------------TEXTURE INDEX DATA (%d count)---------------",numTexIndex);
		for( k = 0; k < numTexIndex; k++ )
			LogFile(ERROR_LOG,"\t%d:\t%d",k,texIndexData[k]);
		LogFile(ERROR_LOG,"---------------TEXTURE SEQ DATA (%d count)---------------",numTexSeq);
		for( k = 0; k < numTexSeq/2; k++ )
			LogFile(ERROR_LOG,"\t%d:\tTex Index: %ld\n\t\t\tTex Modifier: %ld",k,texSeqData[2*k],texSeqData[2*k]+1);
	}

	//------------[ U N K N O W N   D A T A ]----------------/
	
	dataCount = m_mDataHeader.offsetQ7 - m_mDataHeader.offsetQ5;
	if( debugMode )LogFile(ERROR_LOG,"---------------UNKNOWN DATA 1 (qa4[1]: %ld  byte count: %ld)-------------------",
		m_mDataHeader.qa4[1],dataCount);
	unknownData = new BYTE[dataCount];
	if((res = fread(unknownData,1,dataCount,infile)) < dataCount)
	{
		LogFile(ERROR_LOG,"SH3_Actor::loadModel - ERROR: Didn't read enough data from unknown section (%d of %d bytes read)",
			res,dataCount);
	}
	else
	{
		long *lDat = (long *)unknownData;
		for( k = 0; k < dataCount / sizeof(long); k++)
			if( debugMode )debugAll( lDat[k] );
	}

	SAFEDELETE( unknownData );

	if( debugMode )LogFile(ERROR_LOG,"\n\nSize of Other Unknown Data Segments:"
		"\n\tU9-U7: %d\n\tU10-U9: %d\n\tU11-U10: %d\n\tVertDataStart-U11: %d",
		m_mDataHeader.offsetQ9-m_mDataHeader.offsetQ7,m_mDataHeader.offsetQ10-m_mDataHeader.offsetQ9,
		m_mDataHeader.offsetQ11-m_mDataHeader.offsetQ10,m_mDataHeader.offsetVertHead-m_mDataHeader.offsetQ11);

	dataCount = m_mDataHeader.offsetVertHead - m_mDataHeader.offsetQ10;
	if( debugMode )LogFile(ERROR_LOG,"---------------UNKNOWN DATA 1 (qa4[1]: %ld  byte count: %ld)-------------------",
		m_mDataHeader.qa4[1],dataCount);
	unknownData = new BYTE[dataCount];
	if((res = fread(unknownData,1,dataCount,infile)) < dataCount)
	{
		LogFile(ERROR_LOG,"SH3_Actor::loadModel - ERROR: Didn't read enough data from unknown section (%d of %d bytes read)",
			res,dataCount);
	}
	else
	{
		long *lDat = (long *)unknownData;
		for( k = 0; k < dataCount / sizeof(long); k++)
			if( debugMode )debugAll( lDat[k] );
	}

	SAFEDELETE( unknownData );


	curOffset += m_mDataHeader.offsetVertHead;

	m_pmPrimitive = new model_primitive[ m_mDataHeader.numVertexArrays + m_mDataHeader.numAltVertArrays ];
	memset(m_pmPrimitive,0,sizeof(model_primitive) * m_mDataHeader.numVertexArrays);

	numPrimitives = m_mDataHeader.numVertexArrays + m_mDataHeader.numAltVertArrays;

	for( k = 0; k < m_mDataHeader.numVertexArrays; k++ )
	{
		if(debugMode)LogFile(ERROR_LOG,"---------------------------------------------\n SH3_Actor::loadModel - Loading Primitive %d\n---------------------------------------------",k);

//		char outFile[128];
//		sprintf(outFile,"prim%d.txt",k);
		if( debugMode )
			LogFile( DATA_LOG,"Primitive %ld",k);

		if(!loadModelPrimitive( curOffset, &m_pmPrimitive[k],infile))
		{
			LogFile(ERROR_LOG,"SH3_Actor::loadModel - ERROR: Couldn't load model primitive #%d",k);
			if( debugMode )debugModelPrimitive(&m_pmPrimitive[k]);
			SAFEDELETE( m_pmPrimitive );;
			fclose(infile);
			return 0;
		}
//		if(m_pmPrimitive[k].vertSize == sizeof(sh_vertex))
//			debugVertexData( outFile, (BYTE *)&(m_pmPrimitive[k].verts[0]),sizeof(sh_vertex),m_pmPrimitive[k].vertHeader.numVerts);
//		else if(m_pmPrimitive[k].vertSize == sizeof(sh_static_model_vertex))
//			debugVertexData( outFile, (BYTE *)&(m_pmPrimitive[k].altVerts[0]),sizeof(sh_static_model_vertex),m_pmPrimitive[k].vertHeader.numVerts);
//		else
//			LogFile(ERROR_LOG,"SH3_Actor::loadModel - ERROR: Vertex Size is not expected: Size of %d in prim %d",m_pmPrimitive[k].vertSize,k);
		curOffset += m_pmPrimitive[k].vertHeader.vertSegmentSize;
		if( debugMode )LogFile(ERROR_LOG,"SH3_Actor::loadModel - Finished Loading Primitive %d",k);
	}

	dataCount = 0;
	while( curOffset < m_mBaseHeader.texOffset )
	{
		model_vertex_header tempVertHeader;
		fseek(infile,curOffset,SEEK_SET);
		fread(&tempVertHeader,1,sizeof(model_vertex_header),infile);
		curOffset += tempVertHeader.vertSegmentSize;
		dataCount ++;
	}

	if( dataCount > 0 )
	{
		LogFile(ERROR_LOG,"SH3_Actor::loadModel - ERROR: Loaded %d primitives, but %d remained",m_mDataHeader.numVertexArrays,dataCount);
	}

	//----[ LOADING ALTERNATE VERTEX ARRAYS ]----/
	curOffset = baseOffset + this->m_mBaseHeader.baseHeaderSize + m_mDataHeader.offsetAltVertHead;

	for( k = m_mDataHeader.numVertexArrays; k < numPrimitives; k++ )
	{
		if( debugMode )LogFile(ERROR_LOG,"---------------------------------------------\n SH3_Actor::loadModel - Loading Alt Primitive %d\n---------------------------------------------",k);

//		char outFile[128];
//		sprintf(outFile,"prim%d.txt",k);
		if( debugMode )
			LogFile( DATA_LOG,"Primitive %ld",k);

		if(!loadModelPrimitive( curOffset, &m_pmPrimitive[k],infile))
		{
			LogFile(ERROR_LOG,"SH3_Actor::loadModel - ERROR: Couldn't load model primitive #%d",k);
			if( debugMode )debugModelPrimitive(&m_pmPrimitive[k]);
			SAFEDELETE( m_pmPrimitive );;
			fclose(infile);
			return 0;
		}
//		if(m_pmPrimitive[k].vertSize == sizeof(sh_vertex))
//			debugVertexData( outFile, (BYTE *)&(m_pmPrimitive[k].verts[0]),sizeof(sh_vertex),m_pmPrimitive[k].vertHeader.numVerts);
//		else if(m_pmPrimitive[k].vertSize == sizeof(sh_static_model_vertex))
//			debugVertexData( outFile, (BYTE *)&(m_pmPrimitive[k].altVerts[0]),sizeof(sh_static_model_vertex),m_pmPrimitive[k].vertHeader.numVerts);
//		else
//			LogFile(ERROR_LOG,"SH3_Actor::loadModel - ERROR: Vertex Size is not expected: Size of %d in prim %d",m_pmPrimitive[k].vertSize,k);
		curOffset += m_pmPrimitive[k].vertHeader.vertSegmentSize;
		if( debugMode )LogFile(ERROR_LOG,"SH3_Actor::loadModel - Finished Loading Primitive %d",k);
	}
	//----[ LOADING ANIMATION FILE ]----/
//	loadAnim(infile,m_mBaseHeader.modelID,0);

	fclose(infile);

	return 1;
}



//----------------------------------------------------------------------------------/
//-- SH3_Actor::loadModelPrimitive                                                --/
//--    Loads the vertex and index info, along with the header relating how to    --/
//--    render the data.  Loads the texture as well using texMgr class.           --/
//----------------------------------------------------------------------------------/
int  SH3_Actor::loadModelPrimitive( long offset, model_primitive *pPrim, FILE *infile)
{
	int res;
	int dataCount;
	int k;
	char texStr[128];
	long vertSize;
	void *vertDest;
	short texIndex;

	memset( pPrim, 0, sizeof(model_primitive));

	fseek( infile, offset, SEEK_SET );
	if((res = fread(&(pPrim->vertHeader), 1, sizeof( model_vertex_header ), infile)) != sizeof(model_vertex_header))
	{
		LogFile(ERROR_LOG,"SH3_Actor::loadModelPrimitive - ERROR: Couldn't read model_vertex_header (%d of %d bytes read)",res,sizeof(model_vertex_header));
		return 0;
	}

	//debugModelVertHeader( &(pPrim->vertHeader));


	//----[ LOAD ALTERNATE OFFSET DATA ]----/

	dataCount = pPrim->vertHeader.numAltSeq * 3;  //pPrim->vertHeader.offsetSeq1 - pPrim->vertHeader.altOffsetSeq1;

	pPrim->altSeq1 = new short[ dataCount ];

	if((res = fread(pPrim->altSeq1,sizeof(short),dataCount,infile)) != dataCount)
	{
		LogFile(ERROR_LOG,"SH3_Actor::loadModelPrimitive - ERROR: Couldn't read enough alt seq Data (%d of %d elements read)",res,dataCount);
		SAFEDELETE( pPrim->altSeq1 );
		return 0;
	}

//	LogFile(ERROR_LOG,"DEBUG: ------- ALTERNATE SEQ DATA -------");
//	for( k = 0; k < dataCount; k++ )
//		LogFile(ERROR_LOG,"%d: [ %d ]",k,pPrim->altSeq1[k]);
//	LogFile(ERROR_LOG,"DEBUG: ------- END ALTERNATE DATA -------");


	//----[ LOAD AND VALIDATE PRIMITIVE ORIENTATION MATRIX DATA SEQUENCES - SET 1 ]----/

	dataCount = (pPrim->vertHeader.offsetSeq2 - pPrim->vertHeader.offsetSeq1)/sizeof(short);

	pPrim->seqData1 = new short[dataCount];

	if((res = fread(pPrim->seqData1,sizeof(short),dataCount,infile)) != dataCount)
	{
		LogFile(ERROR_LOG,"SH3_Actor::loadModelPrimitive - ERROR: Couldn't read enough seq 1 Data (%d of %d elements read)",res,dataCount);
		SAFEDELETE( pPrim->seqData1 );
		SAFEDELETE( pPrim->altSeq1 );
		return 0;
	}

	//----[ Validate Sequence Data ]----/
	for( k = 0; k < dataCount; k++ )
	{
		if( pPrim->seqData1[k] >= m_mDataHeader.numMatSet1 )
		{
			LogFile(ERROR_LOG,"******\n** WARNING: Seq Set 1, Index %d out of range (%d of %d) - Setting to %d\n******",k,
				pPrim->seqData1[k], m_mDataHeader.numMatSet1,pPrim->seqData1[k] % m_mDataHeader.numMatSet1 );
			pPrim->seqData1[k] = pPrim->seqData1[k] % m_mDataHeader.numMatSet1;
		}
	}


	//----[ LOAD AND VALIDATE PRIMITIVE ORIENTATION MATRIX DATA SEQUENCES - SET 2 ]----/

	dataCount = (pPrim->vertHeader.offsetTexIndex - pPrim->vertHeader.offsetSeq2)/sizeof(short);

	pPrim->seqData2 = new short[dataCount];

	if((res = fread(pPrim->seqData2,sizeof(short),dataCount,infile)) != dataCount)
	{
		LogFile(ERROR_LOG,"SH3_Actor::loadModelPrimitive - ERROR: Couldn't read enough seq 2 Data (%d of %d elements read)",res,dataCount);
		SAFEDELETE( pPrim->seqData1 );
		SAFEDELETE( pPrim->seqData2 );
		SAFEDELETE( pPrim->altSeq1 );
		return 0;
	}

	//----[ Validate Sequence Data ]----/
	for( k = 0; k < dataCount; k++ )
	{
		if( pPrim->seqData2[k] >= m_mDataHeader.numMatSet2 )
		{
			LogFile(ERROR_LOG,"******\n** WARNING: Seq Set 2, Index %d out of range (%d of %d) - Setting to %d\n******",k,
				pPrim->seqData2[k], m_mDataHeader.numMatSet2,pPrim->seqData2[k] % m_mDataHeader.numMatSet2 );
			pPrim->seqData2[k] = pPrim->seqData2[k] % m_mDataHeader.numMatSet2;
		}
	}

	//----[ GET TEX INDEX AND CONSTANT DATA ]----/
	fseek(infile,offset + pPrim->vertHeader.offsetTexIndex, SEEK_SET);

	if((res = fread(&(pPrim->texNum),sizeof(short),1,infile)) != 1 )
	{
		LogFile(ERROR_LOG,"SH3_Actor::loadModelPrimitive - ERROR: Couldn't read texture number from offset %ld (%d read): %s", offset+pPrim->vertHeader.offsetTexIndex,res,strerror(errno));
		SAFEDELETE( pPrim->seqData1 );
		SAFEDELETE( pPrim->seqData2 );
		SAFEDELETE( pPrim->altSeq1 );
		return 0;
	}

	fseek(infile,offset + pPrim->vertHeader.offsetVertConst, SEEK_SET);

	if((res = fread(&(pPrim->always8372234_1),1,sizeof(long)*4,infile)) != sizeof(long)*4 )
	{
		LogFile(ERROR_LOG,"SH3_Actor::loadModelPrimitive - ERROR: Couldn't read constant data at end of model_primitive(%d of %d bytes read): %s", res,sizeof(long)*4,strerror(errno));
		SAFEDELETE( pPrim->seqData1 );
		SAFEDELETE( pPrim->seqData2 );
		SAFEDELETE( pPrim->altSeq1 );
		return 0;
	}

	//----[ LOAD VERTEX AND INDEX DATA ]----/
	vertSize = (pPrim->vertHeader.vertSectionSize - pPrim->vertHeader.vertHeaderSize)/pPrim->vertHeader.numVerts;
	
	if( vertSize == 48 )
	{
		pPrim->verts = new sh_vertex[pPrim->vertHeader.numVerts];
		pPrim->altVerts = NULL;
		vertDest = (void *)pPrim->verts;
	}
	else if( vertSize == 32 )
	{
		pPrim->verts = NULL;
		pPrim->altVerts = new sh_static_model_vertex[pPrim->vertHeader.numVerts];
		vertDest = (void *)pPrim->altVerts;
	}
	else
	{
		LogFile(ERROR_LOG,"SH3_Actor::loadModelPrimtive - TERMINAL ERROR: Unsupported Vertex Type - Size %d",vertSize);
		SAFEDELETE( pPrim->seqData1 );
		SAFEDELETE( pPrim->seqData2 );
		SAFEDELETE( pPrim->altSeq1 );
		SAFEDELETE( pPrim->verts );
		SAFEDELETE( pPrim->altVerts );
		SAFEDELETE( pPrim->indicies );
		return 0;
	}


	//LogFile(ERROR_LOG,"SH3_Actor::loadModelPrimitive - CHECK: vertSize = %ld\n\t\tverts: 0x%08x\n\t\taltVerts: 0x%08x\n\t\tvertDest: 0x%08x",
	//	vertSize,pPrim->verts,pPrim->altVerts,vertDest);

	pPrim->indicies = new long[pPrim->vertHeader.numIndex];
	pPrim->vertSize = vertSize;

	fseek( infile, offset + pPrim->vertHeader.vertHeaderSize, SEEK_SET);


	//if((res = fread(pPrim->verts,1,sizeof(sh_vertex)*pPrim->vertHeader.numVerts,infile)) != sizeof(sh_vertex)*pPrim->vertHeader.numVerts)
	if((res = fread(vertDest,1,vertSize*pPrim->vertHeader.numVerts,infile)) != vertSize*pPrim->vertHeader.numVerts)
	{
		LogFile(ERROR_LOG,"SH3_Actor::loadModelPrimitive - ERROR: Couldn't read verticies (%d of %d bytes read): %s", res,vertSize*pPrim->vertHeader.numVerts,strerror(errno));
		SAFEDELETE( pPrim->seqData1 );
		SAFEDELETE( pPrim->seqData2 );
		SAFEDELETE( pPrim->altSeq1 );
		SAFEDELETE( pPrim->verts );
		SAFEDELETE( pPrim->altVerts );
		SAFEDELETE( pPrim->indicies );
		return 0;
	}
	


	if((res = fread(pPrim->indicies,1,sizeof(long)*pPrim->vertHeader.numIndex,infile)) != sizeof(long)*pPrim->vertHeader.numIndex)
	{
		LogFile(ERROR_LOG,"SH3_Actor::loadModelPrimitive - ERROR: Couldn't read Indicies (%d of %d bytes read): %s", res,sizeof(long)*pPrim->vertHeader.numIndex,strerror(errno));
		SAFEDELETE( pPrim->seqData1 );
		SAFEDELETE( pPrim->seqData2 );
		SAFEDELETE( pPrim->altSeq1 );
		SAFEDELETE( pPrim->verts );
		SAFEDELETE( pPrim->altVerts );
		SAFEDELETE( pPrim->indicies );
		return 0;
	}

//	LogFile(ERROR_LOG,"SH3_Actor::loadModelPrimitive - CHECK:\n\t\tstart offset: %ld\n\t\t  end offset: %ld",offset,ftell(infile));

	if( debugMode )debugModelPrimitive( pPrim );
	if( debugMode )LogFile(ERROR_LOG,"********************************* TEXTURE LOAD PORTION ****************************");

	texIndex = texSeqData[pPrim->texNum*2 ]; 
	texIndex %= m_mDataHeader.numTex;

	pPrim->texModify = texSeqData[pPrim->texNum*2+1 ];

	sprintf( texStr,"%ld_%d",baseOffset,texIndex);

	if((pPrim->texID = textureMgr.GetTexture(string(texStr))) == 0 )
	{
		//if((pPrim->texID = loadTex( baseOffset + m_mBaseHeader.baseHeaderSize + m_mDataHeader.offsetAltVertHead, texStr, pPrim->texNum, infile)) != 0 )
		if((pPrim->texID = loadTex( baseOffset + m_mBaseHeader.texOffset, texStr, texIndex, infile)) != 0 )
			textureMgr.AddTex(string(texStr),pPrim->texID,0);
		else
			LogFile(ERROR_LOG, "SH3_Actor::loadModelPrimitive - ERROR: Couldn't load texture %s",texStr);
	}

/*********************** UNKNOWN FIELD ANALYSIS *********************************/
	if( debugMode )
		LogFile( DATA_LOG,"\tnumAltSeq %2ld   q1,q2,q3 %5ld %5ld %5ld  numFloatSet %ld  always1s %ld,%ld  numsAtEnd %7ld  %4ld  %2ld   f1 %ld  texModify %ld",
			pPrim->vertHeader.numAltSeq,pPrim->vertHeader.q1,pPrim->vertHeader.q2,pPrim->vertHeader.q3,pPrim->vertHeader.numFloatSets,
			pPrim->vertHeader.always1_1,pPrim->vertHeader.always1_2,pPrim->always8372234_1,pPrim->always2044_1,pPrim->always97_1,pPrim->f1,pPrim->texModify);
/*********************** UNKNOWN FIELD ANALYSIS *********************************/

	return 1;
}


void SH3_Actor::releaseModelData()
{
	SAFEDELETE(m_pmSizeOffsets);
	SAFEDELETE(matSet1);
	SAFEDELETE(mSeq1);
	SAFEDELETE(mSeq2);			
	SAFEDELETE(matSet2);
	SAFEDELETE(texIndexData);
	SAFEDELETE(texSeqData);
				
	for( int k = 0; m_pmPrimitive && k < numPrimitives; k++)			
	{
		SAFEDELETE(m_pmPrimitive[k].seqData1);
		SAFEDELETE(m_pmPrimitive[k].seqData2);
		SAFEDELETE(m_pmPrimitive[k].altSeq1 );
		SAFEDELETE(m_pmPrimitive[k].verts);
		SAFEDELETE(m_pmPrimitive[k].altVerts);
		SAFEDELETE(m_pmPrimitive[k].indicies);
	}
				
	SAFEDELETE(m_pmPrimitive);
}

//------------------------------------------------------------------------/
//-- getMinModel                                                        --/
//--   Returns the minimum model number in a ch*.arc file               --/
//------------------------------------------------------------------------/
int SH3_Actor::getMinModel( char *filename )
{
	FILE *infile;
	arc_index_data index;
	model_base_header tempMainHeader;
	int k, res;

	if((infile = fopen(filename,"rb")) == NULL)
	{
		LogFile(ERROR_LOG,"SH3_Actor::getMinModel() - TERMINAL ERROR: Unable to open '%s' for reading.... Exiting function",filename);
		return 0;
	}

	if(!loadArcIndex( infile, &index ))
	{
		LogFile(ERROR_LOG,"SH3_Actor::getMinModel() - TERMINAL ERROR: Unable to get valid index data from '%s'... Exiting function",filename);
		return 0;
	}

	for( k = 0; k < index.numIndex; k ++ )
	{
		if( index.index[k].size > 128*128*4 )
		{
			fseek( infile, index.index[k].offset, SEEK_SET);
			
			if((res = fread(&tempMainHeader,1,sizeof(model_base_header),infile)) < sizeof(model_base_header))
			{
				LogFile(ERROR_LOG,"SH3_Actor::getMinModel() - ERROR: Couldn't read enough bytes for model_base_header - index #%d (%d of %d read)",k,res,sizeof(model_base_header));
				debugModelBaseHeader( &tempMainHeader );
			}
			else
			{
				if( isModel(&tempMainHeader))
				{
					fclose( infile );
					return k;
				}
			}
		
		}
	}

	return 1;

}

//------------------------------------------------------------------------/
//-- getMaxModel                                                        --/
//--   Returns the maximum model number in a ch*.arc file               --/
//------------------------------------------------------------------------/
int SH3_Actor::getMaxModel( char *filename )
{
	FILE *infile;
	arc_index_data index;
	model_base_header tempMainHeader;
	int k, res;

	if((infile = fopen(filename,"rb")) == NULL)
	{
		LogFile(ERROR_LOG,"SH3_Actor::getMaxModel() - TERMINAL ERROR: Unable to open '%s' for reading.... Exiting function",filename);
		return 0;
	}

	if(!loadArcIndex( infile, &index ))
	{
		LogFile(ERROR_LOG,"SH3_Actor::getMaxModel() - TERMINAL ERROR: Unable to get valid index data from '%s'... Exiting function",filename);
		return 0;
	}

	for( k = index.numIndex; k > 0;  k -- )
	{
		if( index.index[k].size > 512*512*4 )
		{
			fseek( infile, index.index[k].offset, SEEK_SET);
			
			if((res = fread(&tempMainHeader,1,sizeof(model_base_header),infile)) < sizeof(model_base_header))
			{
				LogFile(ERROR_LOG,"SH3_Actor::getMaxModel() - ERROR: Couldn't read enough bytes for model_base_header - index #%d (%d of %d read)",k,res,sizeof(model_base_header));
				debugModelBaseHeader( &tempMainHeader );
			}
			else
			{
				if( isModel(&tempMainHeader))
				{
					fclose( infile );
					return k;
				}
			}
		
		}
	}

	return index.numIndex;
}

//######################################## E  X  P  O  R  T     S  M  D     M  E  T  H  O  D  S ################################################

void SH3_Actor::AddVerticies( SMD::SMD_Model & _Model )
{
	int k;
	int j;
	vertex4f	*dispVerts = NULL;
	vertex		*dispNorms = NULL;
	matrix		*matArray = NULL;
	bool		l_reverseDir = false;
	long		*primSeq = NULL;
	vertex4f	tv1,
				tv2,
				tv3,
				tv4;
	SMD::SMD_Triangle			l_Triangle;
	SMD::SMD_Vertex				l_Vertex;
	

	for( j = 0; j < this->numPrimitives; j++ )
	{
		//---[ Initialize Dynamic Data ]---/

		dispVerts = new vertex4f[m_pmPrimitive[j].vertHeader.numVerts];
		dispNorms = new vertex  [m_pmPrimitive[j].vertHeader.numVerts];

		matArray = new matrix[ m_pmPrimitive[j].vertHeader.numSeq1 + m_pmPrimitive[j].vertHeader.numSeq2];

		primSeq = new long[ m_pmPrimitive[j].vertHeader.numSeq1 + m_pmPrimitive[j].vertHeader.numSeq2 ];

		//---[ Set Node Number And Generate Matrix Arrays For Current Primitive ]---/

		for( k = 0; k < m_pmPrimitive[j].vertHeader.numSeq1; k ++ )
		{
			matArray[k] = matSet1[m_pmPrimitive[j].seqData1[k]];
			primSeq[ k ] = m_pmPrimitive[j].seqData1[k];
		}
		for( k = m_pmPrimitive[j].vertHeader.numSeq1; k < m_pmPrimitive[j].vertHeader.numSeq1 + m_pmPrimitive[j].vertHeader.numSeq2; k++ )
		{
			matArray[k] = matSet1[mSeq2[(m_pmPrimitive[j].seqData2[k-m_pmPrimitive[j].vertHeader.numSeq1])*2+1]] * matSet2[m_pmPrimitive[j].seqData2[k-m_pmPrimitive[j].vertHeader.numSeq1]];
			primSeq[ k ] = m_pmPrimitive[j].seqData2[ k - m_pmPrimitive[j].vertHeader.numSeq1 ] + this->m_mDataHeader.numMatSet1;
		}

		//---[ Transform All Verts ]---/
		for( k = 0; k < m_pmPrimitive[j].vertHeader.numVerts; k++ )
		{
			if( m_pmPrimitive[j].vertHeader.numSeq1 + m_pmPrimitive[j].vertHeader.numSeq2 > 1 )
			{
				if( m_pmPrimitive[j].verts == NULL && m_pmPrimitive[j].altVerts != NULL )
				{
					LogFile(TEST_LOG,"SH3_Actor::AddVertices - TERMINAL ERROR: Can't render prim #%d - Small size verts have multi-sequence matricies w/o selector data",j);
					break;
				}

				float	r0w = 1.0f - ( m_pmPrimitive[j].verts[k].v1.x + m_pmPrimitive[j].verts[k].v1.y + m_pmPrimitive[j].verts[k].v1.z );
				
				matrix temp = matArray[m_pmPrimitive[j].verts[k].color.b];
				matrix temp2= matArray[m_pmPrimitive[j].verts[k].color.g];
				matrix temp3= matArray[m_pmPrimitive[j].verts[k].color.r];
				matrix temp4= matArray[m_pmPrimitive[j].verts[k].color.a];

				//---[ Build Transformed Vertex ]---/

				dispVerts[ k ]=	(temp * m_pmPrimitive[j].verts[k].vert) * m_pmPrimitive[j].verts[k].v1.x +
								(temp2 * m_pmPrimitive[j].verts[k].vert) * m_pmPrimitive[j].verts[k].v1.y +
								(temp3 * m_pmPrimitive[j].verts[k].vert) * m_pmPrimitive[j].verts[k].v1.z +
								(temp4 * m_pmPrimitive[j].verts[k].vert) * r0w;

				dispVerts[k].w = 1.0f;

				//---[ Build Transformed Normal ]---/
				temp.clearTrans();
				temp.Inverse();
				temp2.clearTrans();
				temp2.Inverse();
				temp3.clearTrans();
				temp3.Inverse();
				temp4.clearTrans();
				temp4.Inverse();

				dispNorms[ k ]=	(temp * m_pmPrimitive[j].verts[k].normal) * m_pmPrimitive[j].verts[k].v1.x +
								(temp2 * m_pmPrimitive[j].verts[k].normal) * m_pmPrimitive[j].verts[k].v1.y +
								(temp3 * m_pmPrimitive[j].verts[k].normal) * m_pmPrimitive[j].verts[k].v1.z +
								(temp4 * m_pmPrimitive[j].verts[k].normal) * r0w;
			}
			else //---[ THERE IS ONLY ONE MATRIX AND SMALL SIZE VERTS ]---/
			{
				matrix temp = matArray[ 0 ];

				if( m_pmPrimitive[j].verts == NULL && m_pmPrimitive[j].altVerts != NULL )
				{						
					dispVerts[k] = temp * m_pmPrimitive[j].altVerts[k].vert;
					dispVerts[k].w = 1.0f;
					temp.clearTrans();
					temp.Inverse();
					dispNorms[k] = temp * m_pmPrimitive[j].altVerts[k].norm;
				}
				else if( m_pmPrimitive[j].verts != NULL && m_pmPrimitive[j].altVerts == NULL )
				{
					dispVerts[k] = temp * m_pmPrimitive[j].verts[k].vert;
					dispVerts[k].w = 1.0f;
					temp.clearTrans();
					temp.Inverse();
					dispNorms[k] = temp * m_pmPrimitive[j].verts[k].normal;
				}
				else
					LogFile(ERROR_LOG,"SH3_Actor::AddVertices - TERMINAL ERROR: Prim %d - VertSize %d",j,m_pmPrimitive[j].vertSize);
			}

		}

		//---[ Assemble Each Vertex ]---/
		for( k = 0; k < m_pmPrimitive[j].vertHeader.numIndex - 2 ; k++ )
		{
			l_Triangle.DeleteData();
			l_Vertex.DeleteData();

			char l_TexName[ 128 ];

			sprintf( l_TexName, "%d_%d.tga", this->baseOffset, this->texSeqData[ m_pmPrimitive[ j ].texNum * 2 ] % this->m_mDataHeader.numTex );

			l_Triangle.setMaterial( mstring( l_TexName ) );

			if( m_pmPrimitive[j].verts == NULL && m_pmPrimitive[j].altVerts == NULL )
			{
				LogFile(TEST_LOG,"SH3_Actor::ExportSMD - TERMINAL ERROR: Can't render prim #%d - Missing Vert Data",j);
				break;
			}
			else if( m_pmPrimitive[ j ].verts != NULL )
			{
				if( l_reverseDir == true )
				{
					l_Vertex = this->BuildSMDVertex( m_pmPrimitive[ j ].verts[ m_pmPrimitive[ j ].indicies[ k + 1 ] ], primSeq );
					l_Vertex.setPosition( dispVerts[ m_pmPrimitive[ j ].indicies[ k + 1 ] ] );
					l_Vertex.setNormal( dispNorms[ m_pmPrimitive[ j ].indicies[ k + 1 ] ] );
					l_Triangle.AddVertex( l_Vertex );
					l_Vertex = this->BuildSMDVertex( m_pmPrimitive[ j ].verts[ m_pmPrimitive[ j ].indicies[ k ] ], primSeq );
					l_Vertex.setPosition( dispVerts[ m_pmPrimitive[ j ].indicies[ k ] ] );
					l_Vertex.setNormal( dispNorms[ m_pmPrimitive[ j ].indicies[ k ] ] );
					l_Triangle.AddVertex( l_Vertex );
					l_Vertex = this->BuildSMDVertex( m_pmPrimitive[ j ].verts[ m_pmPrimitive[ j ].indicies[ k + 2 ] ], primSeq );
					l_Vertex.setPosition( dispVerts[ m_pmPrimitive[ j ].indicies[ k + 2 ] ] );
					l_Vertex.setNormal( dispNorms[ m_pmPrimitive[ j ].indicies[ k + 2 ] ] );
					l_Triangle.AddVertex( l_Vertex );
				}
				else
				{
					l_Vertex = this->BuildSMDVertex( m_pmPrimitive[ j ].verts[ m_pmPrimitive[ j ].indicies[ k ] ], primSeq );
					l_Vertex.setPosition( dispVerts[ m_pmPrimitive[ j ].indicies[ k ] ] );
					l_Vertex.setNormal( dispNorms[ m_pmPrimitive[ j ].indicies[ k ] ] );
					l_Triangle.AddVertex( l_Vertex );
					l_Vertex = this->BuildSMDVertex( m_pmPrimitive[ j ].verts[ m_pmPrimitive[ j ].indicies[ k + 1 ] ], primSeq );
					l_Vertex.setPosition( dispVerts[ m_pmPrimitive[ j ].indicies[ k + 1 ] ] );
					l_Vertex.setNormal( dispNorms[ m_pmPrimitive[ j ].indicies[ k + 1 ] ] );
					l_Triangle.AddVertex( l_Vertex );
					l_Vertex = this->BuildSMDVertex( m_pmPrimitive[ j ].verts[ m_pmPrimitive[ j ].indicies[ k + 2 ] ], primSeq );
					l_Vertex.setPosition( dispVerts[ m_pmPrimitive[ j ].indicies[ k + 2 ] ] );
					l_Vertex.setNormal( dispNorms[ m_pmPrimitive[ j ].indicies[ k + 2 ] ] );
					l_Triangle.AddVertex( l_Vertex );
				}
			}
//#################################################################################
			else //---[ THERE IS ONLY ONE MATRIX AND SMALL SIZE VERTS ]---/
			{
				if( l_reverseDir == true )
				{
					l_Vertex = this->BuildSMDVertex( m_pmPrimitive[ j ].altVerts[ m_pmPrimitive[ j ].indicies[ k + 1 ] ], primSeq[ 0 ] );
					l_Vertex.setPosition( dispVerts[ m_pmPrimitive[ j ].indicies[ k + 1 ] ] );
					l_Vertex.setNormal( dispNorms[ m_pmPrimitive[ j ].indicies[ k + 1 ] ] );
					l_Triangle.AddVertex( l_Vertex );
					l_Vertex = this->BuildSMDVertex( m_pmPrimitive[ j ].altVerts[ m_pmPrimitive[ j ].indicies[ k ] ], primSeq[ 0 ] );
					l_Vertex.setPosition( dispVerts[ m_pmPrimitive[ j ].indicies[ k ] ] );
					l_Vertex.setNormal( dispNorms[ m_pmPrimitive[ j ].indicies[ k ] ] );
					l_Triangle.AddVertex( l_Vertex );
					l_Vertex = this->BuildSMDVertex( m_pmPrimitive[ j ].altVerts[ m_pmPrimitive[ j ].indicies[ k + 2 ] ], primSeq[ 0 ] );
					l_Vertex.setPosition( dispVerts[ m_pmPrimitive[ j ].indicies[ k + 2 ] ] );
					l_Vertex.setNormal( dispNorms[ m_pmPrimitive[ j ].indicies[ k + 2 ] ] );
					l_Triangle.AddVertex( l_Vertex );
				}
				else
				{
					l_Vertex = this->BuildSMDVertex( m_pmPrimitive[ j ].altVerts[ m_pmPrimitive[ j ].indicies[ k ] ], primSeq[ 0 ] );
					l_Vertex.setPosition( dispVerts[ m_pmPrimitive[ j ].indicies[ k ] ] );
					l_Vertex.setNormal( dispNorms[ m_pmPrimitive[ j ].indicies[ k ] ] );
					l_Triangle.AddVertex( l_Vertex );
					l_Vertex = this->BuildSMDVertex( m_pmPrimitive[ j ].altVerts[ m_pmPrimitive[ j ].indicies[ k + 1 ] ], primSeq[ 0 ] );
					l_Vertex.setPosition( dispVerts[ m_pmPrimitive[ j ].indicies[ k + 1 ] ] );
					l_Vertex.setNormal( dispNorms[ m_pmPrimitive[ j ].indicies[ k + 1 ] ] );
					l_Triangle.AddVertex( l_Vertex );
					l_Vertex = this->BuildSMDVertex( m_pmPrimitive[ j ].altVerts[ m_pmPrimitive[ j ].indicies[ k + 2 ] ], primSeq[ 0 ] );
					l_Vertex.setPosition( dispVerts[ m_pmPrimitive[ j ].indicies[ k + 2 ] ] );
					l_Vertex.setNormal( dispNorms[ m_pmPrimitive[ j ].indicies[ k + 2 ] ] );
					l_Triangle.AddVertex( l_Vertex );
				}
			}

			//--[ Go The Other Way Next Time ]--/
			l_reverseDir = ! l_reverseDir;

			//--[ Add Triangle To Model ]--/
			_Model.AddTriangle( l_Triangle );
		}

		delete [] dispVerts;
		delete [] dispNorms;
		delete [] matArray;
		delete [] primSeq;
	}
}


void SH3_Actor::ExportSMD(  )
{
	int k;

	SMD::SMD_Animation_Frame	l_AnimFrame;
	SMD::SMD_Model				l_Model;


	//---[ A D D   N O D E S ]---/

	//--[ Node Sequence 1 ]--/
	for( k = 0; k < this->m_mDataHeader.numMatSet1; k++ )
	{
		l_Model.AddNode( this->mSeq1[ k ] );
	}

	//--[ Node Sequence 2 ]--/
	for( k = 0; k < this->m_mDataHeader.numMatSet2; k++ )
	{
		l_Model.AddNode( this->mSeq2[ k*2 + 1 ] );
	}

	//---[ G O   T H R O U G H   A L L   P R I M I T I V E S ]---/
	this->AddVerticies( l_Model );


	matrix	*m_pcDispInverse	=	new matrix[ this->m_mDataHeader.numMatSet1 ];
	matrix	*l_pcForwardMat		=	new matrix[ this->m_mDataHeader.numMatSet1 ];
	//---[ Generate The Animation Data ]---/

		//----[ GENERATE INVERSE MATRICIES ]----/
	for( k = 0; k < this->m_mDataHeader.numMatSet1; k++ )	//m_pcAnimSet[ m_lCurFrame ].m_lNumMatrix; k++ )
	{
			m_pcDispInverse[ k ] = this->matSet1[ k ];
			m_pcDispInverse[ k ].Inverse( );
	}
	//----[ MOVE JOINT POSITION BACK TO ORIGIN AND MAKE IT'S RELATIVE OFFSET ]----/
	for( k = 0; k < this->m_mDataHeader.numMatSet1; k++ )	//m_pcAnimSet[ m_lCurFrame ].m_lNumMatrix; k++ )
	{	
		//---[ MAKE JOINT POSITION RELATIVE OFFSET TO ORIGIN ]---/
		if( this->mSeq1[ k ] != -1 )
			l_pcForwardMat[ k ] = m_pcDispInverse[ this->mSeq1[ k ] ] * this->matSet1[ k ];
		else
			l_pcForwardMat[ k ] = this->matSet1[ k ];
	}

	for( k = 0; k < this->m_mDataHeader.numMatSet1; k++ )
	{
		l_AnimFrame.AddTransform( l_pcForwardMat[ k ] );
		//l_AnimFrame.AddTransform( this->matSet1[ k ] );
	}

	delete [] l_pcForwardMat;
	delete [] m_pcDispInverse;

	for( k = 0; k < this->m_mDataHeader.numMatSet2; k++ )
	{
		//matrix l_TempMatrix = this->matSet1[ mSeq2[k*2+1] ] * this->matSet2[ k ];
		l_AnimFrame.AddTransform(  this->matSet2[ k ] );
	}
		
	l_Model.AddAnimationFrame( l_AnimFrame );

	std::fstream  l_OutputFile;
	char l_OutputModelName[ 256 ];

	sprintf( l_OutputModelName, "%s_%ld.smd",baseName( mstring(this->modelFilename) ).c_str(), this->baseOffset );

	l_OutputFile.open( l_OutputModelName, ios::out | ios::trunc );
	
	l_OutputFile << l_Model;

	l_OutputFile.close( );
}


SMD::SMD_Vertex SH3_Actor::BuildSMDVertex( sh_vertex & _vert, long *_pSeq )
{
	float	w = 1.0f - ( _vert.v1.x + _vert.v1.y + _vert.v1.z );
	SMD_Vertex	l_Vertex;

	if( _pSeq == NULL )
		return l_Vertex;

	l_Vertex.setPosition( _vert.vert );
	l_Vertex.setNormal( _vert.normal );
	l_Vertex.setTexcoord( _vert.tc );
	l_Vertex.setParent( _pSeq[ _vert.color.b ] );
	l_Vertex.AddWeight( _vert.v1.x, _pSeq[ _vert.color.b ] );
	if( _vert.v1.y > 0.0f )
		l_Vertex.AddWeight( _vert.v1.y, _pSeq[ _vert.color.g ] );
	if( _vert.v1.z > 0.0f )
		l_Vertex.AddWeight( _vert.v1.z, _pSeq[ _vert.color.r ] );
	if( w > 0.0f )
		l_Vertex.AddWeight( w, _pSeq[ _vert.color.a ] );

	return l_Vertex;
}


SMD::SMD_Vertex SH3_Actor::BuildSMDVertex( sh_static_model_vertex & _vert, long _parent )
{
	SMD_Vertex	l_Vertex;

	l_Vertex.setPosition( _vert.vert );
	l_Vertex.setNormal( _vert.norm );
	l_Vertex.setTexcoord( _vert.tc );
	l_Vertex.AddWeight( 1.0f, _parent );
	
	return l_Vertex;
}
