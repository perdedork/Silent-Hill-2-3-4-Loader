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
//#include "quat.h"
#include "matrix.h"
#include "typedefs.h"
#include "mathlib.h"
#include "Camera.h"
#include "tgaload.h"
//#include "plane.h"
//#include "PerPixelLighting.h"
#include "RenderObjs.h"
#include "Renderer.h"
#include "SH2_Loader.h"
#include "utils.h"
#include "OBJ_Exporter.h"


extern OBJ_Exporter	OBJ_Dump;
extern TexMgr textureMgr;
extern bool debugMode;
extern bool dumpModel;
extern bool useOpenGL;
extern bool onlyStatic;
extern bool onlyVar;
extern sh2_tex_index sh2TexList;

extern int errno;

long g_lOldList[] = { 770, 514, 515, 258, 259, 1 };
long g_lNewList[] = { 769, 513, 515, 257, 259, 260, 2, 1 };
long g_lBothList[]= { 515, 259, 1, 0 };

/******************** M O V E   T H E S E   T O   T H E   A P P R O P R I A T E   P L A C E **************/
#define debugLongHex( v ) ( LogFile(ERROR_LOG,"\t%s: 0x%08x",#v,v))
#define debugString( v ) ( LogFile(ERROR_LOG,"\t%s: [%s]",#v,(v).c_str()))
#define debugCStr( v ) ( LogFile(ERROR_LOG,"\t%s: [%s]",#v,v))
#define debugBool( v ) ( LogFile(ERROR_LOG,"\t%s: %s",#v,(v)?"True":"False"))

//===------====<[ G E N E R A L   F U N C T I O N S ]>====------===/


static void clearGLerrors(int lineNum, char *filename,char *module)
{
	GLenum errorCode;
	int    eFlag = 0;
	while((errorCode=glGetError())!=GL_NO_ERROR && eFlag < 10)
	{
		if( debugMode )
			LogFile(ERROR_LOG,"CLEAR ERROR - '%s'(%d) openGL ERROR::%s  :%s (%d)\n",filename,lineNum,module,gluErrorString(errorCode),eFlag);
		eFlag ++;
	}
}


/*-----------------------------------------------------------------------------*/
/* debugMFH                                                                    */
/*   Prints out the map main file header.                                      */
/*-----------------------------------------------------------------------------*/
void debugMFH( sh2_map_file_header *h, long offset )
{
	LogFile(ERROR_LOG," File Offset: %ld",offset - sizeof(sh2_map_file_header));
	LogFile(ERROR_LOG,"\n]==---=#=---==[ M A P   M A I N   F I L E   H E A D E R (size: %d)]==---=#=---==[",sizeof(sh2_map_file_header));
	debugLongHex( h->mapFileID );
	debugLong( h->mapFileSize );
	debugLong( h->q1 );
	debugLong( h->f1 );
	debugLong( h->q2 );
	debugLong( h->texDataSize );
	debugLong( h->f2 );
	debugLong( h->f3 );

}


/*-----------------------------------------------------------------------------*/
/* debugTB                                                                     */
/*    Prints out the texture base header, which starts off all texture headers */
/*-----------------------------------------------------------------------------*/
void debugTB( sh2_tex_base_header *h, long offset )
{
	LogFile(ERROR_LOG," File Offset: %ld",offset - sizeof(sh2_tex_base_header));
	LogFile(ERROR_LOG,"\n]=---==--=[ M A P   T E X   B A S E   H E A D E R (size: %d)]=--==---=[",sizeof(sh2_tex_base_header));
	debugLongHex( h->texStructMarker );
	debugLong( h->f1 );
	debugLong( h->f2 );
	debugLong( h->q0 );
}


/*-----------------------------------------------------------------------------*/
/* debugTBH                                                                    */
/*    Prints out the texture batch header, which groups tex headers under it   */
/*-----------------------------------------------------------------------------*/
void debugTBH( sh2_tex_batch_header *h, long offset )
{
	LogFile(ERROR_LOG," File Offset: %ld",offset - sizeof(sh2_tex_batch_header));
	LogFile(ERROR_LOG,"\n]=---==--=[ M A P   T E X   B A T C H   H E A D E R (size: %d)]=--==---=[",sizeof(sh2_tex_batch_header));
	debugLong( h->texSetID );
	debugShort( h->q1 );
	debugShort( h->q2 );
	debugShort( h->q3 );
	debugShort( h->q4 );
	debugLong( h->numTexHeaders );
	debugShort( h->q5 );
	debugShort( h->q6 );
	debugLong( h->f3 );
	debugLong( h->f4 );
	debugLong( h->f5 );

}


/*-----------------------------------------------------------------------------*/
/* debugTH                                                                     */
/*   Print out texture header data (these repeat many times, for some reason)  */
/*-----------------------------------------------------------------------------*/
void debugTH( sh2_tex_header *h, long offset )
{
	LogFile(ERROR_LOG," File Offset: %ld",offset - sizeof(sh2_tex_header));
	LogFile(ERROR_LOG,"\n]==---==[ Map  Tex  Header  (size: %d)]==---==[",sizeof(sh2_tex_header));
	debugLong( h->texID );
	debugLong( h->f1 );
	debugShort( h->width );
	debugShort( h->height );
	debugChar( h->compFormat );
	debugChar( h->compAlt );
	debugShort( h->q1_tdh );
	debugLong( h->offsetNextTexHeader );
	debugLong( h->offsetNextOffset );
	debugLong( h->f3 );
	debugLongHex( h->texHeaderMarker );

}


/*-----------------------------------------------------------------------------*/
/* debugTD                                                                     */
/*   Print out a sh2_tex_data class, which holds info on textures for the maps */
/*-----------------------------------------------------------------------------*/
void debugTD( sh2_tex_data *h, long offset )
{
	LogFile(ERROR_LOG," File Offset: %ld",offset );
	LogFile(ERROR_LOG,"\n]==---==[ Map  Tex  Data  (size: %d)]==---==[",sizeof(sh2_tex_data));
	debugLongArray( h->plTexIDs, h->numIDs );
	debugLong( h->numIDs );
	debugLongArray( h->psQ_1_td, h->numIDs );
	debugTBH( &(h->batchHeader), -1 );
	debugTH( &(h->fullHeader), -1 );
	debugString( h->texFile );
	debugLong( h->texOffset );
	debugLong( h->texID );
	debugBool( h->inMem );
	debugString( h->texName );

}



/*-----------------------------------------------------------------------------*/
/* debugMBH                                                                    */
/*   Prints first header type from the map                                     */
/*-----------------------------------------------------------------------------*/
void debugMBH( sh2_map_base_header *h, long offset )
{
	LogFile(ERROR_LOG," File Offset: %ld",offset - sizeof(sh2_map_base_header));
	LogFile(ERROR_LOG,"\n]=---==--=[ M A P   B A S E   H E A D E R (size: %d)]=--==---=[",sizeof(sh2_map_base_header));
	debugLong( h->mainSceneID );
	debugLong( h->sizeAllPrimData );
	debugLong( h->f1 );
	debugLong( h->f2 );

}


/*-----------------------------------------------------------------------------*/
/* debugMOU                                                                    */
/*   Prints the header for the offsets for the unknown data at the end of file */
/*-----------------------------------------------------------------------------*/
void debugMOU( sh2_map_offset_unknown *h, long offset )
{
	LogFile(ERROR_LOG," File Offset: %ld",offset - sizeof(sh2_map_offset_unknown));
	LogFile(ERROR_LOG,"\n]==---==[ Map  Offset  Unknown  (size: %d)]==---==[",sizeof(sh2_map_offset_unknown));
	debugLong( h->mainSceneMarker );
	debugLong( h->numMapOffsetPrims );
	debugLong( h->offsetUnknownData );
	debugLong( h->numUnknowDataSets );
}


/*-----------------------------------------------------------------------------*/
/* debugMOP                                                                    */
/*   Prints offsets of the map primitive offset index data                     */
/*-----------------------------------------------------------------------------*/
void debugMOP( sh2_map_offset_prims *h, long offset )
{
	LogFile(ERROR_LOG," File Offset: %ld",offset - sizeof(sh2_map_offset_prims));
	LogFile(ERROR_LOG,"\n]==---==[ Map  Offset  Prims  (size: %d)]==---==[",sizeof(sh2_map_offset_prims));
	debugLong( h->f1 );
	debugLong( h->sizeAllPrims );
	debugLong( h->offsetMainPrim );
	debugLong( h->offsetSubPrim );
	debugLong( h->offsetMultiPrim );
}


/*-----------------------------------------------------------------------------*/
/* debugMOI                                                                    */
/*   Prints the primitive offset index listing                                 */
/*-----------------------------------------------------------------------------*/
void debugMOI( sh2_map_offset_index *h, long offset )
{
	LogFile(ERROR_LOG," File Offset: %ld",offset - sizeof(sh2_map_offset_index));
	LogFile(ERROR_LOG,"\n]==---==[ Map  Offset  Index  (size: %d)]==---==[",sizeof(sh2_map_offset_index));
	debugLong( h->lPreOffset );
	debugLong( h->numPrimIndex );
	debugLongArray( h->pOffsets, (h->numPrimIndex) );
}


/*-----------------------------------------------------------------------------*/
/* debugMPBH                                                                   */
/*   Prints the common header for all primitives                               */
/*-----------------------------------------------------------------------------*/
void debugMPBH( sh2_map_prim_base_header *h, long offset )
{
	LogFile(ERROR_LOG," File Offset: %ld",offset - sizeof(sh2_map_prim_base_header));
	LogFile(ERROR_LOG,"\n]=---==--=[ M A P   P R I M   B A S E   H E A D E R (size: %d)]=--==---=[",sizeof(sh2_map_prim_base_header));
	debugVertex4f( h->minExtent );
	debugVertex4f( h->maxExtent );
	debugLong( h->headerSize );
	debugLong( h->offsetIndex );
	debugLong( h->sizeIndexData );
}


/*-----------------------------------------------------------------------------*/
/* debugMTR                                                                    */
/*   Prints the index and vertex range data from the tristrip range            */
/*-----------------------------------------------------------------------------*/
void debugMTR( sh2_map_tri_range *h )
{
	LogFile(ERROR_LOG,"\n]---[ Map Tristrip  Range  (size: %d)]==---==[",sizeof(sh2_map_tri_range));
	debugShort( h->numIndicies );
	debugChar( h->q1_mtr );
	debugChar( h->primType );
	debugShort( h->startVertNum );
	debugShort( h->endVertNum );
}



/*-----------------------------------------------------------------------------*/
/* debugMPVR                                                                   */
/*   Prints the index and vertex range data from the variable primitive header */
/*-----------------------------------------------------------------------------*/
void debugMPVR( sh2_map_prim_variable_range *h, long offset )
{
	LogFile(ERROR_LOG," File Offset: %ld",offset - sizeof(sh2_map_prim_variable_range));
	int k;
	LogFile(ERROR_LOG,"\n]==---==[ Map  Prim  Variable  Range  (size: %d)]==---==[",sizeof(sh2_map_prim_variable_range));
	debugLong( h->texNum );
	debugLong( h->vertexSet );
	debugLong( h->numTriRanges );
	for( k = 0; k < h->numTriRanges; k++ )
		debugMTR( &(h->ranges[ k ]) );
}


/*-----------------------------------------------------------------------------*/
/* debugMPSH                                                                   */
/*   Prints out the header for the static length primitive headers             */
/*-----------------------------------------------------------------------------*/
void debugMPSH( sh2_map_prim_static_ranges *h, long offset )
{
	LogFile(ERROR_LOG," File Offset: %ld",offset - sizeof(sh2_map_prim_static_ranges));
	LogFile(ERROR_LOG,"\n]=--=--=[ M A P   P R I M   S T A T I C   R A N G E S (size: %d)]=--=--=[",sizeof(sh2_map_prim_static_ranges));
	debugLong( h->texNum );
	debugLong( h->vertexSet );
	debugShort( h->primType );
	debugShort( h->q2 );
	debugLong( h->numPrims );
}


/*-----------------------------------------------------------------------------*/
/* debugMVS                                                                    */
/*   Prints out the map vertex sizes class                                     */
/*-----------------------------------------------------------------------------*/
void debugMVS( sh2_map_vertex_sizes *h, long offset )
{
	LogFile(ERROR_LOG," File Offset: %ld",offset -sizeof(sh2_map_vertex_sizes));
	long k;
	LogFile(ERROR_LOG,"\n]==---==[ Map  Vertex  Size  (size: %d)]==---==[",sizeof(sh2_map_vertex_sizes));
	debugMVSH( &h->m_sSizeHeader );
	for( k = 0; k < h->m_sSizeHeader.numVertexSizes; k++ )
		debugMVSD( &(h->m_psSizes[ k ]) );
}


/*-----------------------------------------------------------------------------*/
/* debugMVSH                                                                   */
/*   Prints out the vertex size header structure                               */
/*-----------------------------------------------------------------------------*/
void debugMVSH( sh2_vertex_size_header *h )
{
	LogFile(ERROR_LOG,"\n  ]---[ Map  Vertex  Size  Header(size: %d)]----[",sizeof(sh2_vertex_size_header));
	debugLong( h->sizeVertexData );
	debugLong( h->numVertexSizes );
}


/*-----------------------------------------------------------------------------*/
/* debugMVSD                                                                   */
/*   Prints out the vertex size data structure                                 */
/*-----------------------------------------------------------------------------*/
void debugMVSD( sh2_vertex_size_data *h )
{
	LogFile(ERROR_LOG,"\n  ]---[ Map  Vertex  Size  data(size: %d)]----[",sizeof(sh2_vertex_size_data));
	debugLong( h->offsetToVerts );
	debugLong( h->sizeVertex );
	debugLong( h->sizeVertexData );
}


/*-----------------------------------------------------------------------------*/
/* debugMUD                                                                    */
/*   Prints out the unknown data at the end of the file (after last prim data) */
/*-----------------------------------------------------------------------------*/
void debugMUD( sh2_map_unknown_data *h, long offset )
{
	LogFile(ERROR_LOG," File Offset: %ld",offset-sizeof(sh2_map_unknown_data));
	LogFile(ERROR_LOG,"\n]==---==[ Map  Unknown  Data  (size: %d)]==---==[",sizeof(sh2_map_unknown_data));
	debugShort( h->q1 );
	debugShort( h->id );
	debugChar( h->q2 );
	debugChar( h->q3 );
	debugChar( h->q4 );
	debugChar( h->q5 );
	debugLong( h->endMarker );
	debugLong( h->f1 );
}



/*-----------------------------------------------------------------------------*/
/* SaveSH2Texture                                                              */
/*   Sets up the TGA data so that the TGA file can be written out.             */
/*-----------------------------------------------------------------------------*/
void SaveSH2Texture( const char *filename, sh2_tex_header *texHeader, BYTE *data )
{
	image_t imageDat;
	char imageName[256];

	memset(&imageDat,0,sizeof(imageDat));

	imageDat.info.width			= texHeader->width;
	imageDat.info.height		= texHeader->height;
	imageDat.info.image_type	= 2;
	imageDat.info.components	= 4;
	imageDat.info.pixel_depth	= 32;
	imageDat.info.bytes			= imageDat.info.width * imageDat.info.height * imageDat.info.components;

	if( debugMode )
		LogFile(ERROR_LOG,"\nSaveSH2Texture: [ %s ]\n------------------------------------------------------"
			"----------------------\n\tTexData: W:%d H:%d  bytes:%d  pixelDepth:%d  components:%d\n--------"
			"--------------------------------------------------------------------",filename,
			imageDat.info.width,imageDat.info.height,imageDat.info.bytes,imageDat.info.pixel_depth,
			imageDat.info.components);

	imageDat.data = data;
	sprintf( imageName, "%s.tga", filename );
	tgaSave((char *)imageName,&imageDat, true);
}


//====--==< >==---===<[ T E X T U R E   I N D E X   M E M B E R   F U N C T I O N S ]>===---==< >==--====/

/*-----------------------------------------------------------------------------*/
/* sh2_tex_data::isResident                                                    */
/*    Checks to see if the sh2_tex_data object is resident in memory.          */
/*-----------------------------------------------------------------------------*/
bool sh2_tex_data::isResident( )
{
	GLboolean results = GL_FALSE;

	if(texID != 0 && !glAreTexturesResident( 1, (const GLuint *)&texID, &results))
	{
		if( debugMode )
			LogFile(ERROR_LOG,"sh2_tex_data::isResident( ) - ERROR: Couldn't get info on resident texture for tex [%s] with texID %ld",texName.c_str(),texID);
		clearGLerrors(__LINE__,"sh2_tex_data::isResident( )","Checking for texture residency");
		return false;
	}

	if( results )
		return true;
	return false;
}


/*-----------------------------------------------------------------------------*/
/* isResident                                                                  */
/*    Checks to see if any texture object is resident in memory.               */
/*-----------------------------------------------------------------------------*/
bool isResident( GLuint texID )
{
	GLboolean results = GL_FALSE;

	if(texID != 0 && !glAreTexturesResident( 1, (const GLuint *)&texID, &results))
	{
		if( debugMode )
			LogFile(ERROR_LOG,"isResident( ) - ERROR: Couldn't get info on resident texture for texID %ld",texID);
		clearGLerrors(__LINE__,"isResident( )","Checking for texture residency");
		return false;
	}

	if( results )
		return true;
	return false;
}

/*-----------------------------------------------------------------------------*/
/* sh2_tex_index::DeleteTex                                                    */
/*    Deletes the texture of the given name from memory, by calling the texMgr */
/*-----------------------------------------------------------------------------*/
void sh2_tex_index::DeleteTex( string & name )
{
	long minInd = 0;
	long maxInd = m_vTexList.size() - 1;
	long centerInd = (maxInd - minInd) >> 1;
	long texVal = atol( name.c_str() );
	long texListVal;

	if( debugMode )
		LogFile( ERROR_LOG,"sh2_tex_index::DeleteTex( %s ) ------------------ START ------------------",name.c_str());

	if( m_vTexList.size( ) == 0 )
		return;
	if( m_vTexList.size( ) == 1 )
	{
		if( m_vTexList[ centerInd ].texFile != name )
		{
			LogFile(  TEST_LOG, "sh2_tex_index::DeleteTex( %s ) - ERROR: Texture [ %ld ] does not exist in texture list: Only one texture in list - %s",name.c_str(),texVal, m_vTexList[ centerInd ].texFile.c_str() );
			LogFile( ERROR_LOG, "sh2_tex_index::DeleteTex( %s ) - ERROR: Texture [ %ld ] does not exist in texture list: Only one texture in list - %s",name.c_str(),texVal, m_vTexList[ centerInd ].texFile.c_str() );
			return;
		}
	}
	else
	{
		//---[ Find Texture ]---/
		do
		{
			texListVal = atol( m_vTexList[ centerInd ].texName.c_str() );

			if( texVal < texListVal )
				maxInd = centerInd;
			else
				minInd = centerInd;

			centerInd = minInd + ((maxInd - minInd) >> 1);

		}while( minInd != maxInd && centerInd != minInd);


		//---[ Determine Before or After ]---/
		texListVal = atol( m_vTexList[ centerInd ].texName.c_str() );

		if( texVal != texListVal )
		{
			++centerInd;
			texListVal = atol( m_vTexList[ centerInd ].texName.c_str() );

			if( texVal != texListVal )
			{
				LogFile(  TEST_LOG, "sh2_tex_index::DeleteTex( %s ) - ERROR: Texture [ %ld ] does not exist in texture list: Should be between %s and %s",name.c_str(),texVal, m_vTexList[ centerInd - 1 ].texFile.c_str(), m_vTexList[ centerInd ].texFile.c_str() );
				LogFile( ERROR_LOG, "sh2_tex_index::DeleteTex( %s ) - ERROR: Texture [ %ld ] does not exist in texture list: Should be between %s and %s",name.c_str(),texVal, m_vTexList[ centerInd - 1 ].texFile.c_str(), m_vTexList[ centerInd ].texFile.c_str() );
				return;
			}
		}
	}

	//---[ Determine the texture's state and delete it ]---/
	if( m_vTexList[ centerInd ].inMem )
	{
		if( debugMode )
			LogFile( ERROR_LOG, "sh2_tex_index::DeleteTex( %s ) - CHECK: Tex ID to delete is [%ld]",name.c_str(),m_vTexList[ centerInd ].texID);
		glDeleteTextures( 1, &(m_vTexList[ centerInd ].texID) );
		checkGLerror( __LINE__,__FILE__,"sh2_tex_index::DeleteTex - After glDeleteTextures");
	}
	m_vTexList[ centerInd ].inMem = false;
//CHECKYOSELF	m_vTexList[ centerInd ].texID = 0;

	if( debugMode )
		LogFile( ERROR_LOG,"sh2_tex_index::DeleteTex( %s ) ------------------- END -------------------",name.c_str());

}


/*-----------------------------------------------------------------------------*/
/* sh2_tex_index::DeleteAllTex                                                 */
/*    Uses DeleteTex to delete all the texs from mem. Does not affect metadata */
/*-----------------------------------------------------------------------------*/
void sh2_tex_index::DeleteAllTex( )
{
	int k;
	bool origDebugMode = debugMode;

	debugMode = false;
	
	for( k = m_vTexList.size(); k > 0; k-- )
	{		
		if( m_vTexList[ k-1 ].isResident( ) )
		{
			DeleteTex( m_vTexList[k-1].texName );
			//LogFile( ERROR_LOG,"sh2_tex_index::DeleteAllTex( ) - CHECK TEX: Texture [%s] at index %ld with ID %ld still in mem",m_vTexList[ k-1 ].texName.c_str(),k,m_vTexList[ k-1 ].texID);
			//LogFile(  TEST_LOG,"sh2_tex_index::DeleteAllTex( ) - CHECK TEX: Texture [%s] at index %ld with ID %ld still in mem",m_vTexList[ k-1 ].texName.c_str(),k,m_vTexList[ k-1 ].texID);
		}
	}
	debugMode = origDebugMode;
}


/*-----------------------------------------------------------------------------*/
/* sh2_tex_index::CountTexInMem                                                */
/*    Counts the number of textures in memory. If passed true, checks resident */
/*-----------------------------------------------------------------------------*/
long sh2_tex_index::CountTexInMem( bool _bCheckResident )
{
	long k;
	long count = 0;
	
	for( k = m_vTexList.size(); k > 0; k-- )
	{
		if( _bCheckResident )
		{
			if( m_vTexList[ k-1 ].inMem && m_vTexList[ k-1 ].isResident( ) )
				++count;	
		}
		else if( m_vTexList[ k-1 ].inMem )
			++count;
	}
	return count;
}


/*-----------------------------------------------------------------------------*/
/* sh2_tex_index::ReleaseAllTex                                                */
/*    Deletes and releases all of the textures in mem, and emptys the metadata */
/*-----------------------------------------------------------------------------*/
void sh2_tex_index::ReleaseAllTex( )
{
	if( debugMode ) LogFile( ERROR_LOG, "sh2_tex_index::ReleaseAllTex( ) - About to release tex in mem");
	DeleteAllTex( );
	if( debugMode ) LogFile( ERROR_LOG, "sh2_tex_index::ReleaseAllTex( ) - About to clear metadata");
	m_vTexList.clear( );
	if( debugMode ) LogFile( ERROR_LOG, "sh2_tex_index::ReleaseAllTex( ) - Done");
	if( m_uiBadTex )
		glDeleteTextures( 1, &m_uiBadTex );
	m_uiBadTex = 0;
}

/*-----------------------------------------------------------------------------*/
/* sh2_tex_index::GetTex                                                       */
/*    Gets the texture from texMgr, and if not in mem, loads it from it's map  */
/*-----------------------------------------------------------------------------*/
GLuint sh2_tex_index::GetTex( string & name, bool alpha )
{
	long minInd = 0;
	long maxInd = m_vTexList.size() - 1;
	long centerInd = (maxInd - minInd) >> 1;
	long texVal = atol( name.c_str() );
	long texListVal;

	if( m_vTexList.size( ) == 0 )
	{
		return GetBadTex( );
		return 0;
	}
	if( m_vTexList.size( ) == 1 )
	{
		if( m_vTexList[ centerInd ].texName != name )
		{
			return GetBadTex( );
			if( debugMode )
			{
				LogFile(  TEST_LOG, "sh2_tex_index::GetTex( %s ) - ERROR: Texture [ %ld ] does not exist in texture list: Only one texture in list - %s",name.c_str(),texVal, m_vTexList[ centerInd ].texName.c_str() );
				LogFile( ERROR_LOG, "sh2_tex_index::GetTex( %s ) - ERROR: Texture [ %ld ] does not exist in texture list: Only one texture in list - %s",name.c_str(),texVal, m_vTexList[ centerInd ].texName.c_str() );
			}
			return 0;
		}
	}
	else
	{
		//---[ Find Texture ]---/
		do
		{
			
			texListVal = atol( m_vTexList[ centerInd ].texName.c_str() );

			if( texVal < texListVal )
				maxInd = centerInd;
			else
				minInd = centerInd;

			centerInd = minInd + ((maxInd - minInd) >> 1);

		}while( minInd != maxInd && centerInd != minInd);


		//---[ Determine Before or After ]---/
		texListVal = atol( m_vTexList[ centerInd ].texName.c_str() );

		if( texVal != texListVal )
		{
			++centerInd;
			texListVal = atol( m_vTexList[ centerInd ].texName.c_str() );
		
			if( texVal != texListVal )
			{
				return GetBadTex( );
				if( debugMode )
				{
					LogFile(  TEST_LOG, "sh2_tex_index::GetTex( %s ) - ERROR: Texture [ %ld ] does not exist in texture list: Should be between %s and %s",name.c_str(),texVal, m_vTexList[ centerInd - 1 ].texName.c_str(), m_vTexList[ centerInd ].texName.c_str() );
					LogFile( ERROR_LOG, "sh2_tex_index::GetTex( %s ) - ERROR: Texture [ %ld ] does not exist in texture list: Should be between %s and %s",name.c_str(),texVal, m_vTexList[ centerInd - 1 ].texName.c_str(), m_vTexList[ centerInd ].texName.c_str() );
				}
				return 0;
			}
		}
	}
	
	if( alpha && m_vTexList[ centerInd ].fullHeader.compFormat > 1 )
	{
		glEnable(GL_ALPHA_TEST);
		glAlphaFunc(GL_GEQUAL, 0.08f);
		//glEnable(GL_BLEND);
		//glBlendFunc(GL_SRC_ALPHA,GL_DST_COLOR);
	}

	//---[ Determine the texture's state and Load it/return it ]---/
	if( dumpModel && m_vTexList[ centerInd ].inMem )
		DeleteTex( m_vTexList[ centerInd ].texName );

	if( m_vTexList[ centerInd ].inMem )
		return m_vTexList[ centerInd ].texID;
	return LoadTex( &(m_vTexList[ centerInd ]) );
	
}


/*-----------------------------------------------------------------------------*/
/* sh2_tex_index::LoadTex                                                      */
/*    Loads the tex from the map that it is located in, and sets inMem to true */
/*-----------------------------------------------------------------------------*/
GLuint sh2_tex_index::LoadTex( sh2_tex_data *pTD )
{
	FILE *inFile;
	long lRes;
	unsigned char *pixels = NULL,
		*pData = NULL;
	bool badInit = false;
	
	if( debugMode )
		LogFile( ERROR_LOG, "sh2_tex_index::LoadTex( %s - tex: %s ) ---------------------------- Start ---------------------------- ", pTD->texFile.c_str(),pTD->texName.c_str() );


	//---[ Open the file, and set the filename ]---/
if(debugMode)LogFile(ERROR_LOG,"About to open file %s",pTD->texFile.c_str());
	if( ( inFile = fopen( pTD->texFile.c_str() , "rb" ) ) == NULL )
	{
		LogFile(  TEST_LOG, "sh2_tex_index::LoadTex - ERROR: Couldn't open map file '%s' for tex: %s -  Exiting\n\tREASON: %s", pTD->texFile.c_str(),pTD->texName.c_str(),strerror( errno ) );
		LogFile( ERROR_LOG, "sh2_tex_index::LoadTex - ERROR: Couldn't open map file '%s' for tex: %s -  Exiting\n\tREASON: %s", pTD->texFile.c_str(),pTD->texName.c_str(),strerror( errno ) );
		return 0;
	}

	//---[ R E A D   I N   T H E   C O M P R E S S E D   P I X E L   D A T A ]---/

	SAFEDELETE( pixels );
	pixels = new unsigned char[ pTD->fullHeader.offsetNextTexHeader ];
	pData  = new unsigned char[ pTD->fullHeader.height * pTD->fullHeader.width * 4 ];
	fseek( inFile, pTD->texOffset, SEEK_SET );
	
//LogFile( ERROR_LOG, "TEST: The values of the arguments are: pixels: %ld\toffsetNextTexHeader: %ld\tinFile:%ld",pixels,pTD->fullHeader.offsetNextTexHeader, inFile ); 

	if( ( lRes = fread( (void *)pixels, 1,pTD->fullHeader.offsetNextTexHeader, inFile ) ) < pTD->fullHeader.offsetNextTexHeader)
	{
		LogFile(  TEST_LOG, "sh2_tex_index::LoadTex( %s - tex: %s ) - ERROR: Couldn't read pixel data (%d of %d bytes read)  -  Exiting\n\t\tREASON: %s", pTD->texFile.c_str(),pTD->texName.c_str() , lRes,pTD->fullHeader.offsetNextTexHeader, strerror(errno));
		LogFile( ERROR_LOG, "sh2_tex_index::LoadTex( %s - tex: %s ) - ERROR: Couldn't read pixel data (%d of %d bytes read)  -  Exiting\n\t\tREASON: %s", pTD->texFile.c_str(),pTD->texName.c_str() , lRes,pTD->fullHeader.offsetNextTexHeader, strerror(errno));
		SAFEDELETE( pixels );
		SAFEDELETE( pData );
		fclose( inFile );
		return 0;
	}
		
	if( useOpenGL )
	{
		GLint iFormat;

		glGenTextures( 1, &pTD->texID );

		if( debugMode )
			debugTD( pTD, ftell(inFile) );

		if( pTD->texID < 1 )
		{
			LogFile( TEST_LOG,"sh2_tex_index::LoadTex( %s - tex: %s ) - ERROR: Couldn't generate more textures!!!!! glGenTextures returned %d as the tex ID", pTD->texFile.c_str(),pTD->texName.c_str() ,pTD->texID);
			LogFile(ERROR_LOG,"sh2_tex_index::LoadTex( %s - tex: %s ) - ERROR: Couldn't generate more textures!!!!! glGenTextures returned %d as the tex ID", pTD->texFile.c_str(),pTD->texName.c_str() ,pTD->texID);
		}
		else
		{
			switch( pTD->fullHeader.compFormat )
			{
				case 0:
				case 1:	iFormat = GL_COMPRESSED_RGB_S3TC_DXT1_EXT;
						break;
				//case 2:	iFormat = GL_COMPRESSED_RGBA_S3TC_DXT1_EXT;
				//		break;
				case 2:
				case 3:	iFormat = GL_COMPRESSED_RGBA_S3TC_DXT3_EXT;
						break;
				case 4: 
				case 5:	iFormat = GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
						break;
				default:
					LogFile( ERROR_LOG, "sh2_tex_index::LoadTex( %s - tex: %s ) - ERROR: Undetermined compression format: %d", pTD->texFile.c_str(),pTD->texName.c_str() ,pTD->fullHeader.compFormat );
					iFormat = GL_COMPRESSED_RGB_S3TC_DXT1_EXT;
			}

			glBindTexture ( GL_TEXTURE_2D, pTD->texID );
			glCompressedTexImage2DARB( GL_TEXTURE_2D, 0, iFormat, pTD->fullHeader.width, pTD->fullHeader.height, 0, pTD->fullHeader.offsetNextTexHeader, pixels );
			checkGLerror(__LINE__,__FILE__,"SH2_MapLoader::LoadTex( ) - Compressed Tex Creation");
			glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
			glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );

			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
			glTexEnvi ( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE,GL_REPLACE);

			if( dumpModel )
			{
				glGetTexImage( GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, (GLvoid *)pData);
				//LogFile(ERROR_LOG,"sh2_tex_index::LoadTex - TEST: After getting uncompressed data: pData = 0x%08x\tpixels = 0x%08x",pData,pixels);
				checkGLerror(__LINE__,__FILE__,"SH2_MapLoader::LoadTex( ) - Compressed Tex Dump");
				SaveSH2Texture( pTD->texName.c_str(), &(pTD->fullHeader), pData );
			}
		}
	}
	else
	{
		LogFile( ERROR_LOG, "sh2_tex_index::LoadTex( %s - tex: %s ) - NOTICE::\n----------------------------------\n\tDIRECT X  IS  NOT  IMPLEMENTED\n----------------------------------", pTD->texFile.c_str(),pTD->texName.c_str() );
	}


	if( pTD->texID > 0 )
	{
		pTD->inMem = true;
		if( ! pTD->isResident( ) )
		{
			LogFile( ERROR_LOG, "sh2_tex_index::LoadTex( %s ) - TERMINAL ERROR: Texture ID not in MEMORY", pTD->texFile.c_str(),pTD->texID);
			LogFile(  TEST_LOG, "sh2_tex_index::LoadTex( %s ) - TERMINAL ERROR: Texture ID not in MEMORY", pTD->texFile.c_str(),pTD->texID);
		}
	}

	if( debugMode )
		LogFile( ERROR_LOG, "sh2_tex_index::LoadTex( %s ) -------------- End --------------", pTD->texFile.c_str() );
	
	SAFEDELETE( pixels );
	SAFEDELETE( pData );
	fclose( inFile );

	return pTD->texID;
}



/*-----------------------------------------------------------------------------*/
/* sh2_tex_index::LoadFileInfo                                                 */
/*    Loads tex info from a map & saves it in ascending order in the tex list  */
/*-----------------------------------------------------------------------------*/
int sh2_tex_index::LoadFileInfo( char *mapName )
{
	sh2_map_file_header		m_sFileHeader;
	FILE *inFile;
	long lRes;
	long lTotalRead = 0;


	if( debugMode )
		LogFile( ERROR_LOG, "sh2_tex_index::LoadFileInfo( %s ) ----------------------------- Start -----------------------------", mapName);


	//---[ Open the file, and set the filename ]---/

	if( ( inFile = fopen( mapName, "rb" ) ) == NULL )
	{
		LogFile(  TEST_LOG, "sh2_tex_index::LoadFileInfo - ERROR: Couldn't open map file '%s'  -  Exiting",mapName);
		LogFile( ERROR_LOG, "sh2_tex_index::LoadFileInfo - ERROR: Couldn't open map file '%s'  -  Exiting",mapName);
		return 0;
	}

	if( ( lRes = fread( (void *)&m_sFileHeader, 1, sizeof( sh2_map_file_header ), inFile ) ) < sizeof( sh2_map_file_header ))
	{
		LogFile(  TEST_LOG, "sh2_tex_index::LoadFileInfo( %s ) - ERROR: Couldn't read sh2_map_file_header (%d of %d bytes read)  -  Exiting\n\t\tREASON: %s",mapName,lRes,sizeof(sh2_map_file_header),strerror(errno));
		LogFile( ERROR_LOG, "sh2_tex_index::LoadFileInfo( %s ) - ERROR: Couldn't read sh2_map_file_header (%d of %d bytes read)  -  Exiting\n\t\tREASON: %s",mapName,lRes,sizeof(sh2_map_file_header),strerror(errno));
		fclose( inFile );
		return 0;
	}

	if( debugMode )
		debugMFH( &m_sFileHeader, ftell(inFile) );

	return LoadFileInfoAux( inFile, mapName, m_sFileHeader.texDataSize );
}



/*-----------------------------------------------------------------------------*/
/* sh2_tex_index::LoadModelFileInfo                                            */
/*    Loads tex info from a map & saves it in ascending order in the tex list  */
/*-----------------------------------------------------------------------------*/
int sh2_tex_index::LoadModelFileInfo( char *modelName )
{
	sh2_model_base_header	m_sFileHeader;
	FILE *inFile;
	long lRes;
	long lTotalRead = 0;


	if( debugMode )
		LogFile( ERROR_LOG, "sh2_tex_index::LoadModelFileInfo( %s ) ----------------------------- Start -----------------------------", modelName);
	

	//---[ Open the file, and set the filename ]---/



	if( ( inFile = fopen( modelName, "rb" ) ) == NULL )
	{
		LogFile(  TEST_LOG, "sh2_tex_index::LoadModelFileInfo - ERROR: Couldn't open model file '%s'  -  Exiting",modelName);
		LogFile( ERROR_LOG, "sh2_tex_index::LoadModelFileInfo - ERROR: Couldn't open model file '%s'  -  Exiting",modelName);
		return 0;
	}

	if( ( lRes = fread( (void *)&m_sFileHeader, 1, sizeof( sh2_model_base_header ), inFile ) ) < sizeof( sh2_model_base_header ))
	{
		LogFile(  TEST_LOG, "sh2_tex_index::LoadModelFileInfo( %s ) - ERROR: Couldn't read sh2_model_base_header (%d of %d bytes read)  -  Exiting\n\t\tREASON: %s",modelName,lRes,sizeof(sh2_model_base_header),strerror(errno));
		LogFile( ERROR_LOG, "sh2_tex_index::LoadModelFileInfo( %s ) - ERROR: Couldn't read sh2_model_base_header (%d of %d bytes read)  -  Exiting\n\t\tREASON: %s",modelName,lRes,sizeof(sh2_model_base_header),strerror(errno));
		fclose( inFile );
		return 0;
	}

	if( m_sFileHeader.offsetTex == 0 )
	{
		LogFile(  TEST_LOG, "sh2_tex_index::LoadModelFileInfo( %s ) - NOTE: No offset for texture data",modelName);
		LogFile( ERROR_LOG, "sh2_tex_index::LoadModelFileInfo( %s ) - NOTE: No offset for texture data",modelName);
		fclose( inFile );
		return 0;
	}
//	if( debugMode )
//		debugMFH( &m_sFileHeader );
	fseek( inFile, m_sFileHeader.offsetTex, SEEK_SET );

	return LoadFileInfoAux( inFile, modelName, getFileSize( modelName ) - m_sFileHeader.offsetTex );
}



/*-----------------------------------------------------------------------------*/
/* sh2_tex_index::LoadFileInfoAux                                              */
/*    Loads tex info from a starting point & saves it in order in the tex list */
/*-----------------------------------------------------------------------------*/
int sh2_tex_index::LoadFileInfoAux( FILE *inFile, char *mapName, long _lTexDataSize )
{
	sh2_tex_base_header		m_sTexBaseHeader;
	sh2_tex_batch_header	m_sTexBatchHeader;
	sh2_tex_header			*m_psTexHeader = NULL;		//Number of sh2_tex_header is in sh2_tex_batch_header
	sh2_tex_data			sTexData;
	long lnextOffset;
	long lRes;
	long lTotalRead = 0;
	long texDataSize;
	long k;
	char texStr[128];
	
	//---[ S T A R T   T E X T U R E   L O A D,   C O M P L E T E   W H E N   M A X   B Y T E S   R E A D ]---/

	texDataSize = _lTexDataSize;

	if( ( lRes = fread( (void *)&m_sTexBaseHeader, 1, sizeof( sh2_tex_base_header ), inFile ) ) < sizeof( sh2_tex_base_header ))
	{
		LogFile(  TEST_LOG, "sh2_tex_index::LoadFileInfoAux( %s ) off: %ld  read: %ld - ERROR: Couldn't read sh2_tex_base_header (%d of %d bytes read)  -  Exiting\n\t\tREASON: %s", mapName, ftell( inFile ), lTotalRead, lRes,sizeof(sh2_tex_base_header),strerror(errno));
		LogFile( ERROR_LOG, "sh2_tex_index::LoadFileInfoAux( %s ) off: %ld  read: %ld - ERROR: Couldn't read sh2_tex_base_header (%d of %d bytes read)  -  Exiting\n\t\tREASON: %s", mapName, ftell( inFile ), lTotalRead, lRes,sizeof(sh2_tex_base_header),strerror(errno));
		fclose( inFile );
		return 0;
	}

	if( debugMode )
		debugTB( &m_sTexBaseHeader, ftell(inFile) );

	if( m_sTexBaseHeader.texStructMarker == 0x20010730 )	// != 0x19990901 )
	{
		LogFile(  TEST_LOG, "sh2_tex_index::LoadFileInfoAux( %s ) - ERROR: Incorrect Tex Batch Marker (0x20010730) - Skipping file.", mapName );
		LogFile( ERROR_LOG, "sh2_tex_index::LoadFileInfoAux( %s ) - ERROR: Incorrect Tex Batch Marker (0x20010730) - Skipping file.", mapName );
		fclose( inFile );
		return 0;
	}

	lTotalRead += lRes;

	if( m_sTexBaseHeader.texStructMarker != 0x19990901 )
	{
		LogFile(  TEST_LOG, "sh2_tex_index::LoadFileInfoAux( %s ) - ERROR: Invalid Tex Batch Marker (0x%08x, should be 0x19990901) - Skipping file.", mapName, m_sTexBaseHeader.texStructMarker );
		LogFile( ERROR_LOG, "sh2_tex_index::LoadFileInfoAux( %s ) - ERROR: Invalid Tex Batch Marker (0x%08x, should be 0x19990901) - Skipping file.", mapName, m_sTexBaseHeader.texStructMarker );
		fclose( inFile );
		return 0;
	}

	while( lTotalRead < texDataSize && texDataSize - lTotalRead > sizeof(sh2_tex_batch_header) + sizeof(sh2_tex_header))
	{

		//---[ R E A D   I N   B A T C H   H E A D E R ]---/

		if( ( lRes = fread( (void *)&m_sTexBatchHeader, 1, sizeof( sh2_tex_batch_header ), inFile ) ) < sizeof( sh2_tex_batch_header ))
		{
			LogFile(  TEST_LOG, "sh2_tex_index::LoadFileInfoAux( %s ) off: %ld  read: %ld - ERROR: Couldn't read sh2_tex_batch_header (%d of %d bytes read)  -  Exiting\n\t\tREASON: %s",mapName, ftell( inFile ), lTotalRead, lRes,sizeof(sh2_tex_batch_header),strerror(errno));
			LogFile( ERROR_LOG, "sh2_tex_index::LoadFileInfoAux( %s ) off: %ld  read: %ld - ERROR: Couldn't read sh2_tex_batch_header (%d of %d bytes read)  -  Exiting\n\t\tREASON: %s",mapName, ftell( inFile ), lTotalRead, lRes,sizeof(sh2_tex_batch_header),strerror(errno));
			fclose( inFile );
			return 0;
		}

		if( debugMode )
			debugTBH( &m_sTexBatchHeader, ftell(inFile) );

		lTotalRead += lRes;


		//---[ R E A D   I N   A L L   T E X T U R E   H E A D E R S ]---/

		SAFEDELETE( m_psTexHeader );

		m_psTexHeader = new sh2_tex_header[ m_sTexBatchHeader.numTexHeaders ];

		for( k = 0; k < m_sTexBatchHeader.numTexHeaders; k++ )
		{
			if( ( lRes = fread( (void *)&m_psTexHeader[ k ], 1, sizeof( sh2_tex_header ), inFile ) ) < sizeof( sh2_tex_header ))
			{
				LogFile(  TEST_LOG, "sh2_tex_index::LoadFileInfoAux( %s ) off: %ld  read: %ld - ERROR: Couldn't read sh2_tex_header #%d of %d (%d of %d bytes read)  -  Exiting\n\t\tREASON: %s",mapName, ftell( inFile ), lTotalRead, k+1,m_sTexBatchHeader.numTexHeaders, lRes,sizeof(sh2_tex_header),strerror(errno));
				LogFile( ERROR_LOG, "sh2_tex_index::LoadFileInfoAux( %s ) off: %ld  read: %ld - ERROR: Couldn't read sh2_tex_header #%d of %d (%d of %d bytes read)  -  Exiting\n\t\tREASON: %s",mapName, ftell( inFile ), lTotalRead, k+1,m_sTexBatchHeader.numTexHeaders, lRes,sizeof(sh2_tex_header),strerror(errno));
				fclose( inFile );
				return 0;
			}

			if( debugMode )
				debugTH( &m_psTexHeader[ k ], ftell(inFile) );

			lTotalRead += lRes;
		}

		//---[ C R E A T E   sh2_tex_data   C L A S S   D A T A ]---/

		
		sTexData.numIDs = m_sTexBatchHeader.numTexHeaders - 1;
		sTexData.inMem = false;
		sTexData.texFile = string(mapName);
		sTexData.texID = 0;
		memcpy( (void *)&sTexData.batchHeader, (void *)&m_sTexBatchHeader, sizeof( sh2_tex_batch_header ));
		memcpy( (void *)&sTexData.fullHeader, (void *)&(m_psTexHeader[ m_sTexBatchHeader.numTexHeaders - 1 ]), sizeof( sh2_tex_header ));
		sprintf( texStr, "%ld",sTexData.batchHeader.texSetID);
		sTexData.texName = string( texStr );
		sTexData.texOffset = ftell( inFile );
		SAFEDELETE( sTexData.plTexIDs );
		SAFEDELETE( sTexData.psQ_1_td );
		sTexData.plTexIDs = new long[ sTexData.numIDs ];
		sTexData.psQ_1_td = new short[ sTexData.numIDs ];

		lnextOffset = sTexData.texOffset + sTexData.fullHeader.offsetNextTexHeader;

		for( k = 0; k < sTexData.numIDs; k ++ )
		{
			sTexData.plTexIDs[ k ] = m_psTexHeader[ k ].texID;
			sTexData.psQ_1_td[ k ] = m_psTexHeader[ k ].q1_tdh;
		}

		//---[ S K I P   T H E   C O M P R E S S E D   P I X E L   D A T A ]---/
		lTotalRead += sTexData.fullHeader.offsetNextTexHeader;
		fseek( inFile, sTexData.texOffset + sTexData.fullHeader.offsetNextTexHeader, SEEK_SET );

		//---[ Sanity Check ]---/
		if( lnextOffset != ftell( inFile ))
			LogFile( ERROR_LOG, "sh2_tex_index::LoadFileInfoAux( %s ) - WARNING: After reading pixels, the file offset doesn't match what was expected:  %ld act  %ld exp", mapName ,ftell(inFile), lnextOffset);

		//---[ Add This Texture To List ]---/
		if( AddToTexList( &sTexData ) < 0 )
			LogFile( ERROR_LOG, "sh2_tex_index::LoadFileInfoAux( %s ) off: %ld - WARNING: TexID %s could not be added",mapName, sTexData.texOffset, sTexData.texFile.c_str() );
	}


	if( debugMode )
		LogFile( ERROR_LOG, "sh2_tex_index::LoadFileInfoAux( ) -------------- End --------------");

	SAFEDELETE( sTexData.plTexIDs );
	SAFEDELETE( sTexData.psQ_1_td );
	SAFEDELETE( m_psTexHeader );
	fclose( inFile );

	return lTotalRead;
}


/*-----------------------------------------------------------------------------*/
/* sh2_tex_index::AddToTexList                                                 */
/*    Loads the tex from the map that it is located in, and sets inMem to true */
/*-----------------------------------------------------------------------------*/
int sh2_tex_index::AddToTexList( sh2_tex_data *pTD )
{
	long minInd = 0;
	long maxInd = m_vTexList.size() - 1;
	long centerInd = (maxInd - minInd) >> 1;
	long texVal = atol( pTD->texName.c_str() );
	long texListVal;
	vector<sh2_tex_data>::iterator tItr = m_vTexList.begin();


	if( debugMode )
		LogFile( ERROR_LOG, "sh2_tex_index::AddToTexList( ) -------------------- Start ---------------------");

	if( m_vTexList.size( ) == 0 )
	{
		m_vTexList.push_back( (*pTD) );

		if( debugMode )
			LogFile( ERROR_LOG, "sh2_tex_index::AddToTexList( ) -------------------- End ---------------------");

		return 1;
	}

	if( m_vTexList.size( ) == 1 )
	{
		texListVal = atol( m_vTexList[ centerInd ].texName.c_str() );

		if( texVal > texListVal )
		{
			m_vTexList.push_back( (*pTD) );
			return 2;
		}
		else if( texVal < texListVal )
		{
			m_vTexList.insert( tItr, (*pTD));
	        //m_vTexList[ 0 ] = (*pTD);
			//m_vTexList.push_back( tempDat );
			return 1;
		}
		LogFile(  TEST_LOG, "sh2_tex_index::AddToTexList( ) - ERROR: Texture [ %s ] Repeated in following files:\n\t%s\n\t%s",pTD->texName.c_str(),m_vTexList[ centerInd ].texFile.c_str(), pTD->texFile.c_str() );
		LogFile( ERROR_LOG, "sh2_tex_index::AddToTexList( ) - ERROR: Texture [ %s ] Repeated in following files:\n\t%s\n\t%s",pTD->texName.c_str(),m_vTexList[ centerInd ].texFile.c_str(), pTD->texFile.c_str() );
		return 0;
	}


	//---[ Find Place to Put In Texture ]---/
	do
	{
		
		texListVal = atol( m_vTexList[ centerInd ].texName.c_str() );

		if( texVal < texListVal )
			maxInd = centerInd;
		else
			minInd = centerInd;

		centerInd = minInd + ((maxInd - minInd) >> 1);

	}while( minInd != maxInd && centerInd != minInd);

	
	//---[ Determine Before or After ]---/
	texListVal = atol( m_vTexList[ centerInd ].texName.c_str() );

	if( texVal == texListVal )
	{
		LogFile(  TEST_LOG, "sh2_tex_index::AddToTexList( ) - ERROR: Texture [ %s ] Repeated in following files:\n\t%s\n\t%s",pTD->texName.c_str(),m_vTexList[ centerInd ].texFile.c_str(), pTD->texFile.c_str() );
		LogFile( ERROR_LOG, "sh2_tex_index::AddToTexList( ) - ERROR: Texture [ %s ] Repeated in following files:\n\t%s\n\t%s",pTD->texName.c_str(),m_vTexList[ centerInd ].texFile.c_str(), pTD->texFile.c_str() );
		if( debugMode )
			LogFile( ERROR_LOG, "sh2_tex_index::AddToTexList( ) -------------------- End ---------------------");

		return -1;
	}
	else if( texVal < texListVal )
	{

		advance( tItr, centerInd );
		m_vTexList.insert( tItr, (*pTD) );

		if( debugMode )
			LogFile( ERROR_LOG, "sh2_tex_index::AddToTexList( ) -------------------- End ---------------------");

		return centerInd;
	}

	//---[ Check For The Last Value ]---/
	if( centerInd + 1 == m_vTexList.size() - 1 )
	{
		texListVal = atol( m_vTexList[  m_vTexList.size() - 1 ].texName.c_str() );
		if( texVal > texListVal )
		{
			m_vTexList.push_back( (*pTD) );
			return m_vTexList.size() - 1;
		}
	}
	advance( tItr, centerInd + 1 );
	m_vTexList.insert( tItr, (*pTD) );

	if( debugMode )
		LogFile( ERROR_LOG, "sh2_tex_index::AddToTexList( ) -------------------- End ---------------------");

	return centerInd;
	

}


/*-----------------------------------------------------------------------------*/
/* sh2_tex_index::BuildTexList                                                 */
/*    Builds Tex List from map files from starting dir & records them in order */
/*-----------------------------------------------------------------------------*/
void sh2_tex_index::BuildTexList( char *pDirName )
{
	WIN32_FIND_DATA fileData;	
	HANDLE fileHandle;
	char dirName[512];
	char fileName[512];
	bool addSlash = false;
	bool origDebug;


	if( pDirName[ strlen( pDirName ) - 1 ] == '\\' )
	{
		addSlash = false;
		sprintf( dirName,"%s*",pDirName );
	}
	else
	{
		addSlash = true;
		sprintf( dirName, "%s\\*",pDirName );
	}
		
	origDebug = debugMode;
	//debugMode = false;
		
	if((fileHandle = FindFirstFile( dirName, &fileData)) != INVALID_HANDLE_VALUE )
	{
		do
		{
			if(debugMode)
				LogFile(ERROR_LOG, "sh2_tex_index::BuildTexList( %s ) - Cur File Name is [%s]",pDirName,fileData.cFileName);
			if( fileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY && fileData.cFileName[0] != '.' )
			{
				if( addSlash )
					sprintf( dirName, "%s\\%s",pDirName, fileData.cFileName );
				else
					sprintf( dirName, "%s%s",pDirName, fileData.cFileName );

				if( debugMode )
					LogFile( ERROR_LOG, "sh2_tex_index::BuildTexList( %s ) - Switching to directory '%s'",pDirName, dirName );

				BuildTexList( dirName );

				if( addSlash )
					sprintf( dirName, "%s\\*",pDirName );
				else
					sprintf( dirName, "%s*",pDirName );
			}
			else
			{
				if( strncmp( &( fileData.cFileName[ strlen( fileData.cFileName ) - 4 ] ), ".map", 4 ) == 0 )
				{
					if( addSlash )
						sprintf( fileName, "%s\\%s",pDirName, fileData.cFileName );
					else
						sprintf( fileName, "%s%s",pDirName, fileData.cFileName );

					if( ! LoadFileInfo( fileName ) )
					{
						LogFile(  TEST_LOG, "sh2_tex_index::BuildTexList( %s ) - ERROR: Couldn't Add file \"%s\" to texture list", pDirName, fileName );
						LogFile( ERROR_LOG, "sh2_tex_index::BuildTexList( %s ) - ERROR: Couldn't Add file \"%s\" to texture list", pDirName, fileName );
					}
					else if( debugMode )
						LogFile( ERROR_LOG, "sh2_tex_index::BuildTexList( %s ) - NOTE: \n\tSuccessfully added file \"%s\" to texture list", pDirName, fileName );
				}
				else if( strncmp( &( fileData.cFileName[ strlen( fileData.cFileName ) - 4 ] ), ".mdl", 4 ) == 0 )
				{
					if( addSlash )
						sprintf( fileName, "%s\\%s",pDirName, fileData.cFileName );
					else
						sprintf( fileName, "%s%s",pDirName, fileData.cFileName );

					if( ! LoadModelFileInfo( fileName ) )
					{
						LogFile(  TEST_LOG, "sh2_tex_index::BuildTexList( %s ) - ERROR: Couldn't Add file \"%s\" to texture list", pDirName, fileName );
						LogFile( ERROR_LOG, "sh2_tex_index::BuildTexList( %s ) - ERROR: Couldn't Add file \"%s\" to texture list", pDirName, fileName );
					}
					else if( debugMode )
						LogFile( ERROR_LOG, "sh2_tex_index::BuildTexList( %s ) - NOTE: \n\tSuccessfully added file \"%s\" to texture list", pDirName, fileName );
				}
			}
			
		}
		while(FindNextFile(fileHandle, &fileData));
		FindClose(fileHandle);
	}
	else
	{
		LogFile( TEST_LOG,"sh2_tex_index::BuildTexList( %s ) - ERROR: Unable to find first file",pDirName );
		LogFile(ERROR_LOG,"sh2_tex_index::BuildTexList( %s ) - ERROR: Unable to find first file",pDirName );
	}
	
	debugMode = origDebug;
	//{
	//	int k;
	//	for( k = 0; k < m_vTexList.size(); k++ )
	//	{
	//		debugString( m_vTexList[k].texName );
	//		debugString( m_vTexList[k].texFile );
	//	}
	//}

}


GLuint sh2_tex_index::GetBadTex( )
{
	sh2_tex_header	l_sTexHeader;
	long k, j;
	unsigned char *l_pcPixels = NULL;

	if( m_uiBadTex > 0 )
	{
		if( ! dumpModel && isResident( m_uiBadTex ) )
			return m_uiBadTex;
		else
			glDeleteTextures( 1, &m_uiBadTex );
	}

	//---[ Generate The Bad Texture ]---/

	l_pcPixels = new unsigned char[ 128 * 128 * 4 ];

	for( k = 0; k < 128; k++ )
	{
		for( j = 0; j < 128; j++ )
		{
			l_pcPixels[ k * 128 * 4 + j * 4 + 0 ] = (((k >> 3) % 2) ^ ((j >> 3) % 2)) * (     k>>3) * (16 - j>>3);
			l_pcPixels[ k * 128 * 4 + j * 4 + 1 ] = (((k >> 3) % 2) ^ ((j >> 3) % 2)) * (     k>>3) * (     j>>3);
			l_pcPixels[ k * 128 * 4 + j * 4 + 2 ] = (((k >> 3) % 2) ^ ((j >> 3) % 2)) * (16 - k>>3) * (     j>>3);
			l_pcPixels[ k * 128 * 4 + j * 4 + 3 ] = 128;
		}
	}

	//---[ Fill in the texture header data, in case we want to save it ]---/
	l_sTexHeader.height = 128;
	l_sTexHeader.width = 128;
	



	//---[ Load The Texture Into OpenGL ]---/

	glGenTextures( 1, & m_uiBadTex );

	glBindTexture ( GL_TEXTURE_2D, m_uiBadTex );
	glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, l_sTexHeader.width, l_sTexHeader.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, l_pcPixels );
	checkGLerror(__LINE__,__FILE__,"sh2_tex_index::GetBadTex( ) - ");
	glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
	glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexEnvi ( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE,GL_REPLACE);

	if( dumpModel )
	{
		//LogFile(ERROR_LOG,"sh2_tex_index::LoadTex - TEST: After getting uncompressed data: pData = 0x%08x\tpixels = 0x%08x",pData,pixels);
		checkGLerror(__LINE__,__FILE__,"SH2_MapLoader::LoadTex( ) - Compressed Tex Dump");
		SaveSH2Texture( "0", &l_sTexHeader, l_pcPixels );
	}

	delete [] l_pcPixels;

	return m_uiBadTex;
}



/*-----------------------------------------------------------------------------*/
/* sh2_map_prim_variable_range::operator=                                      */
/*    Assignment operator for the variable primitive texture ranges            */
/*-----------------------------------------------------------------------------*/
sh2_map_prim_variable_range & sh2_map_prim_variable_range::operator=( const sh2_map_prim_variable_range & rhs )
{
	if( this != &rhs )
	{
		texNum = rhs.texNum;
		vertexSet = rhs.vertexSet;
		numTriRanges = rhs.numTriRanges;
		ranges = new sh2_map_tri_range[ numTriRanges ];
		memcpy( (void *)ranges, (void *)rhs.ranges, sizeof(sh2_map_tri_range) * numTriRanges );
	}
	return *this;
}

/*-----------------------------------------------------------------------------*/
/* sh2_map_prim_variable_range::LoadTexRange                                   */
/*    Loads data for the var prim tex range, returns bytes read, or -1 if fail */
/*-----------------------------------------------------------------------------*/
long sh2_map_prim_variable_range::LoadTexRange( FILE *inFile )
{
	long lStartOff = ftell( inFile );
	unsigned long lRes;
	long lTotalRead = 0;
	sh2_map_tri_header tempHeader;

	if( ( lRes = fread( (void *)&tempHeader, 1, sizeof( sh2_map_tri_header ), inFile ) ) < sizeof( sh2_map_tri_header ))
	{
		LogFile(  TEST_LOG, "sh2_map_prim_variable_range::LoadTexRange( off: %ld ) - ERROR: Couldn't read sh2_map_tri_header (%d of %d bytes read)  -  Exiting\n\t\tREASON: %s",lStartOff,lRes,sizeof(sh2_map_tri_header),strerror(errno));
		LogFile( ERROR_LOG, "sh2_map_prim_variable_range::LoadTexRange( off: %ld ) - ERROR: Couldn't read sh2_map_tri_header (%d of %d bytes read)  -  Exiting\n\t\tREASON: %s",lStartOff,lRes,sizeof(sh2_map_tri_header),strerror(errno));
		return -1;
	}

	lTotalRead += lRes;
	
	this->numTriRanges = tempHeader.numTriStripRanges;
	this->texNum = tempHeader.texNum;
	this->vertexSet = tempHeader.vertexSet;

	this->ranges = new sh2_map_tri_range[ numTriRanges ];

	
	if( ( lRes = fread( (void *)ranges, 1, sizeof( sh2_map_tri_range ) * numTriRanges, inFile ) ) < sizeof( sh2_map_tri_range ) * numTriRanges)
	{
		LogFile(  TEST_LOG, "sh2_map_prim_variable_range::LoadTexRange( off: %ld ) - ERROR: Couldn't read sh2_map_tri_ranges - %ld total (%d of %d bytes read)  -  Exiting\n\t\tREASON: %s",lStartOff,numTriRanges,lRes,sizeof(sh2_map_tri_range) * numTriRanges,strerror(errno));
		LogFile( ERROR_LOG, "sh2_map_prim_variable_range::LoadTexRange( off: %ld ) - ERROR: Couldn't read sh2_map_tri_ranges - %ld total (%d of %d bytes read)  -  Exiting\n\t\tREASON: %s",lStartOff,numTriRanges,lRes,sizeof(sh2_map_tri_range) * numTriRanges,strerror(errno));
		SAFEDELETE( ranges );
		return -1;
	}

	lTotalRead += lRes;

	if( debugMode )
	{
		long k;

		for( k = 0; k < numTriRanges; k++ )
		{
			if( ranges[ k ].primType != 1 && ranges[ k ].primType != 3 || ranges[ k ].q1_mtr != 0 )
				LogFile( TEST_LOG, "sh2_map_prim_variable_range::LoadTexRange( off: %ld ) - CHECK: Values for variable range %ld of %ld different from norm:\n\t\tprimType: %d\n\t\tq1_mtr  : %d", lStartOff, k+1, numTriRanges, ranges[ k ].primType, ranges[ k ].q1_mtr );
		}
	}
	
	
	return lTotalRead;
}


/*-----------------------------------------------------------------------------*/
/* sh2_map_vertex_sizes::operator=                                             */
/*    Assignment operator for the vertex size class                            */
/*-----------------------------------------------------------------------------*/
sh2_map_vertex_sizes & sh2_map_vertex_sizes::operator=( const sh2_map_vertex_sizes & rhs )
{
	if( &rhs != this )
	{
		memcpy( (void *)&m_sSizeHeader, (void *)&rhs.m_sSizeHeader, sizeof( sh2_vertex_size_header ));
		SAFEDELETE( m_psSizes );
		m_psSizes = new sh2_vertex_size_data[ m_sSizeHeader.numVertexSizes ];
		memcpy( (void *)m_psSizes, (void *)rhs.m_psSizes, sizeof( sh2_vertex_size_data ) * m_sSizeHeader.numVertexSizes );
	}
	return *this;
}


/*-----------------------------------------------------------------------------*/
/* sh2_map_vertex_sizes::LoadVertexSizes                                       */
/*    Loads vertex size structure for the primitive types                      */
/*-----------------------------------------------------------------------------*/
int sh2_map_vertex_sizes::LoadVertexSizes( FILE *inFile )
{
	unsigned long lRes;
	long lTotalRead = 0;
	long lStartOff = ftell( inFile );


	if( ( lRes = fread( (void *)&m_sSizeHeader, 1, sizeof( sh2_vertex_size_header ), inFile ) ) < sizeof( sh2_vertex_size_header ))
	{
		LogFile(  TEST_LOG, "sh2_map_vertex_sizes::LoadVertexSizes( off: %ld ) - ERROR: Couldn't read sh2_vertex_size_header (%d of %d bytes read)  -  Exiting\n\t\tREASON: %s",lStartOff,lRes,sizeof(sh2_vertex_size_header),strerror(errno));
		LogFile( ERROR_LOG, "sh2_map_vertex_sizes::LoadVertexSizes( off: %ld ) - ERROR: Couldn't read sh2_vertex_size_header (%d of %d bytes read)  -  Exiting\n\t\tREASON: %s",lStartOff,lRes,sizeof(sh2_vertex_size_header),strerror(errno));
		return -1;
	}

	lTotalRead += lRes;

		//---[ Load Complex Vertex Size Header ]---/
	m_psSizes = new sh2_vertex_size_data[ m_sSizeHeader.numVertexSizes ];

	if( ( lRes = fread( (void *)m_psSizes, 1, sizeof( sh2_vertex_size_data ) * m_sSizeHeader.numVertexSizes, inFile ) ) < sizeof( sh2_vertex_size_data ) * m_sSizeHeader.numVertexSizes)
	{
		LogFile(  TEST_LOG, "sh2_map_vertex_sizes::LoadVertexSizes( off: %ld ) - ERROR: Couldn't read sh2_vertex_size_data (%d of %d bytes read)  -  Exiting\n\t\tREASON: %s",lStartOff,lRes,sizeof(sh2_vertex_size_data) * m_sSizeHeader.numVertexSizes,strerror(errno));
		LogFile( ERROR_LOG, "sh2_map_vertex_sizes::LoadVertexSizes( off: %ld ) - ERROR: Couldn't read sh2_vertex_size_data (%d of %d bytes read)  -  Exiting\n\t\tREASON: %s",lStartOff,lRes,sizeof(sh2_vertex_size_data) * m_sSizeHeader.numVertexSizes,strerror(errno));
		SAFEDELETE( m_psSizes );
		return -1;
	}

	lTotalRead += lRes;

	return lTotalRead;
}



/*-----------------------------------------------------------------------------*/
/* SH2_MapPrimitiveStatic::operator=                                           */
/*    Assignment operator for the static map primitive                         */
/*-----------------------------------------------------------------------------*/
SH2_MapPrimitiveStatic & SH2_MapPrimitiveStatic::operator=( const SH2_MapPrimitiveStatic & rhs )
{
	if( &rhs != this )
	{
		m_lNumIndicies		= rhs.m_lNumIndicies;
		m_lNumVerts			= rhs.m_lNumVerts;
		m_lNumSimpleVerts	= rhs.m_lNumSimpleVerts;
		m_lNumColorVerts	= rhs.m_lNumColorVerts;
		m_lNumShaderVerts	= rhs.m_lNumShaderVerts;

		SAFEDELETE( m_psVerts );
		SAFEDELETE( m_psSimpleVerts );
		SAFEDELETE( m_psColorVerts );
		SAFEDELETE( m_psShaderVerts );
		m_psVerts		= new sh2_map_vertex[ m_lNumVerts ];
		m_psSimpleVerts = new sh2_vertex[ m_lNumSimpleVerts ];
		m_psColorVerts	= new sh2_color_vertex[ m_lNumColorVerts ];
		m_psShaderVerts		= new sh2_shader_vertex[ m_lNumShaderVerts ];
		memcpy( (void *)m_psVerts, (void *)rhs.m_psVerts, sizeof( sh2_map_vertex ) * m_lNumVerts );
		memcpy( (void *)m_psSimpleVerts, (void *)rhs.m_psSimpleVerts, sizeof( sh2_vertex ) * m_lNumSimpleVerts );
		memcpy( (void *)m_psColorVerts, (void *)rhs.m_psColorVerts, sizeof( sh2_color_vertex ) * m_lNumColorVerts );
		memcpy( (void *)m_psShaderVerts, (void *)rhs.m_psShaderVerts, sizeof( sh2_shader_vertex ) * m_lNumShaderVerts );

		SAFEDELETE( m_pusIndicies );
		m_pusIndicies	= new unsigned int[ m_lNumIndicies ];
		memcpy( (void *)m_pusIndicies, (void *)rhs.m_pusIndicies, sizeof( unsigned int ) * m_lNumIndicies );

		memcpy( (void *)&m_sBaseHeader, (void *)&rhs.m_sBaseHeader, sizeof( sh2_map_prim_base_header ));
		m_lNumTexRanges = rhs.m_lNumTexRanges;
		m_sStaticTexRanges = new sh2_map_prim_static_ranges[ m_lNumTexRanges ];
		memcpy( (void *)m_sStaticTexRanges, (void *)rhs.m_sStaticTexRanges, sizeof( sh2_map_prim_static_ranges ) * m_lNumTexRanges);
		m_sVertexSize = rhs.m_sVertexSize;
		m_lSegmentID = rhs.m_lSegmentID;
		m_bFirstTime = rhs.m_bFirstTime;
	}
	return *this;
}


/*-----------------------------------------------------------------------------*/
/* SH2_MapPrimitiveStatic::LoadPrimitive                                       */
/*   Loads the primtive for the current location. Needs validations and checks */
/*-----------------------------------------------------------------------------*/
int  SH2_MapPrimitiveStatic::LoadPrimitive( FILE *inFile, long lPrimSize )
{
	long lRes;
	long lTotalRead = 0;
	long k;
	unsigned short int *tIndicies;
	long lStartOff = ftell( inFile );
	void *pVertDest = NULL;
	long *pVCountDest = NULL;

	if( debugMode )
		LogFile( ERROR_LOG, "SH2_MapPrimitiveStatic::LoadPrimitive( off: %ld ) -------------- Start --------------",lStartOff);

	//---[ READ THE HEADER PORTIONS FOR THE PRIMITIVE ]---/

	if( ( lRes = fread( (void *)&m_sBaseHeader, 1, sizeof( sh2_map_prim_base_header ), inFile ) ) < sizeof( sh2_map_prim_base_header ))
	{
		LogFile(  TEST_LOG, "SH2_MapPrimitiveStatic::LoadPrimitive( off: %ld ) - ERROR: Couldn't read sh2_map_prim_base_header (%d of %d bytes read)  -  Exiting\n\t\tREASON: %s",lStartOff,lRes,sizeof(sh2_map_prim_base_header),strerror(errno));
		LogFile( ERROR_LOG, "SH2_MapPrimitiveStatic::LoadPrimitive( off: %ld ) - ERROR: Couldn't read sh2_map_prim_base_header (%d of %d bytes read)  -  Exiting\n\t\tREASON: %s",lStartOff,lRes,sizeof(sh2_map_prim_base_header),strerror(errno));
		return 0;
	}

	lTotalRead += lRes;

	if( debugMode )
		debugMPBH( &m_sBaseHeader , ftell(inFile));


	if( ( lRes = fread( (void *)&m_lNumTexRanges, 1, sizeof( long ), inFile ) ) < sizeof( long ))
	{
		LogFile(  TEST_LOG, "SH2_MapPrimitiveStatic::LoadPrimitive( off: %ld ) - ERROR: Couldn't read m_lNumTexRanges (%d of %d bytes read)  -  Exiting\n\t\tREASON: %s",lStartOff,lRes,sizeof(long),strerror(errno));
		LogFile( ERROR_LOG, "SH2_MapPrimitiveStatic::LoadPrimitive( off: %ld ) - ERROR: Couldn't read m_lNumTexRanges (%d of %d bytes read)  -  Exiting\n\t\tREASON: %s",lStartOff,lRes,sizeof(long),strerror(errno));
		return 0;
	}

	lTotalRead += lRes;

	m_sStaticTexRanges = new sh2_map_prim_static_ranges[ m_lNumTexRanges ];
	
	for( k = 0; k < m_lNumTexRanges; k++ )
	{

		if( ( lRes = fread( (void *)&m_sStaticTexRanges[ k ] , 1, sizeof( sh2_map_prim_static_ranges ), inFile ) ) < sizeof( sh2_map_prim_static_ranges ))
		{
			LogFile(  TEST_LOG, "SH2_MapPrimitiveStatic::LoadPrimitive( off: %ld ) - ERROR: Couldn't read sh2_map_prim_static_ranges (%d of %d bytes read)  -  Exiting\n\t\tREASON: %s",lStartOff,lRes,sizeof(sh2_map_prim_static_ranges),strerror(errno));
			LogFile( ERROR_LOG, "SH2_MapPrimitiveStatic::LoadPrimitive( off: %ld ) - ERROR: Couldn't read sh2_map_prim_static_ranges (%d of %d bytes read)  -  Exiting\n\t\tREASON: %s",lStartOff,lRes,sizeof(sh2_map_prim_static_ranges),strerror(errno));
			SAFEDELETE( m_sStaticTexRanges );
			return 0;
		}

		lTotalRead += lRes;

		if( debugMode )
			debugMPSH( &m_sStaticTexRanges[ k ], ftell(inFile) );
	}

	
	//---[ LOAD IN VERTEX SIZE STRUCTURE ]---/
	lRes = m_sVertexSize.LoadVertexSizes( inFile );
	if( lRes == -1 )
	{
		LogFile(  TEST_LOG, "SH2_MapPrimitiveStatic::LoadPrimitive( off: %ld ) - ERROR: Couldn't load the vertex size structure  -  Exiting\n\t\tREASON: %s",lStartOff, strerror( errno ) );
		LogFile( ERROR_LOG, "SH2_MapPrimitiveStatic::LoadPrimitive( off: %ld ) - ERROR: Couldn't load the vertex size structure  -  Exiting\n\t\tREASON: %s",lStartOff, strerror( errno ) );
		return 0;
	}

	if( debugMode )
		debugMVS( &m_sVertexSize , ftell(inFile));
	lTotalRead += lRes;


	//---[ READ IN THE VERTICIES AND INDICIES ]---/

	//--[ Check basic data to ensure proper format ]--/
	for( k = 0; k < m_sVertexSize.m_sSizeHeader.numVertexSizes; k++ )
	{
		switch( m_sVertexSize.m_psSizes[ k ].sizeVertex )
		{
			case sizeof(sh2_map_vertex):
				SAFEDELETE( m_psVerts );
					//--[ Set size of vertex array ]--/
				m_lNumVerts = m_sVertexSize.m_psSizes[ k ].sizeVertexData / m_sVertexSize.m_psSizes[ k ].sizeVertex;
				pVCountDest = &m_lNumVerts;
				m_psVerts = new sh2_map_vertex[ m_lNumVerts ];
				pVertDest = (void *)m_psVerts;
				break;
			case sizeof(sh2_vertex):
				SAFEDELETE( m_psSimpleVerts );
					//--[ Set size of vertex array ]--/
				m_lNumSimpleVerts = m_sVertexSize.m_psSizes[ k ].sizeVertexData / m_sVertexSize.m_psSizes[ k ].sizeVertex;
				pVCountDest = &m_lNumSimpleVerts;
				m_psSimpleVerts = new sh2_vertex[ m_lNumSimpleVerts ];
				pVertDest = (void *)m_psSimpleVerts;
				break;
			case sizeof(sh2_color_vertex):
				SAFEDELETE( m_psColorVerts );
					//--[ Set size of vertex array ]--/
				m_lNumColorVerts = m_sVertexSize.m_psSizes[ k ].sizeVertexData / m_sVertexSize.m_psSizes[ k ].sizeVertex;
				pVCountDest = &m_lNumColorVerts;
				m_psColorVerts = new sh2_color_vertex[ m_lNumColorVerts ];
				pVertDest = (void *)m_psColorVerts;
				break;
			case sizeof(sh2_shader_vertex):
				SAFEDELETE( m_psShaderVerts );
					//--[ Set size of vertex array ]--/
				m_lNumShaderVerts = m_sVertexSize.m_psSizes[ k ].sizeVertexData / m_sVertexSize.m_psSizes[ k ].sizeVertex;
				pVCountDest = &m_lNumShaderVerts;
				m_psShaderVerts = new sh2_shader_vertex[ m_lNumShaderVerts ];
				pVertDest = (void *)m_psShaderVerts;
				break;
			default:
				LogFile( ERROR_LOG, "SH2_MapPrimitiveStatic::LoadPrimitive( off: %ld ) - ERROR: Unexpected Vertex Size: %ld bytes  -  Exiting\n",lStartOff, m_sVertexSize.m_psSizes[ k ].sizeVertex );
				LogFile(  TEST_LOG, "SH2_MapPrimitiveStatic::LoadPrimitive( off: %ld ) - ERROR: Unexpected Vertex Size: %ld bytes  -  Exiting\n",lStartOff, m_sVertexSize.m_psSizes[ k ].sizeVertex );
				DeleteDynamicData( );
				return 0;
		}


	//--[ Load in vertex data ]--/
	
		if( ( lRes = fread( pVertDest, 1, m_sVertexSize.m_psSizes[ k ].sizeVertex * (*pVCountDest), inFile ) ) < m_sVertexSize.m_psSizes[ k ].sizeVertex * (*pVCountDest))
		{
			LogFile(  TEST_LOG, "SH2_MapPrimitiveStatic::LoadPrimitive( off: %ld ) - ERROR: Couldn't read %ld byte verticies for vertex group %ld (%d of %d bytes read, %ld predicted)  -  Exiting",lStartOff,m_sVertexSize.m_psSizes[ k ].sizeVertex, k + 1,lRes,m_sVertexSize.m_psSizes[ k ].sizeVertex * (*pVCountDest), m_sVertexSize.m_psSizes[ k ].sizeVertexData);
			LogFile( ERROR_LOG, "SH2_MapPrimitiveStatic::LoadPrimitive( off: %ld ) - ERROR: Couldn't read %ld byte verticies for vertex group %ld (%d of %d bytes read, %ld predicted)  -  Exiting",lStartOff,m_sVertexSize.m_psSizes[ k ].sizeVertex, k + 1,lRes,m_sVertexSize.m_psSizes[ k ].sizeVertex * (*pVCountDest), m_sVertexSize.m_psSizes[ k ].sizeVertexData);
			DeleteDynamicData( );
			return 0;
		}

		lTotalRead += lRes;
	}

	//--[ Determine size of index array ]--/
	
	m_lNumIndicies = m_sBaseHeader.sizeIndexData / sizeof( unsigned short int );

	//--[ Load in index data ]--/
	SAFEDELETE( m_pusIndicies );
	m_pusIndicies = new unsigned int[ m_lNumIndicies ];
	tIndicies = new unsigned short int[ m_lNumIndicies ];
	if( ( lRes = fread( (void *)tIndicies, 1, sizeof( unsigned short int ) * m_lNumIndicies, inFile ) ) < (long)sizeof( unsigned short int ) * m_lNumIndicies)
	{
		LogFile(  TEST_LOG, "SH2_MapPrimitiveStatic::LoadPrimitive( off: %ld ) - ERROR: Couldn't read indicies (%d of %d bytes read, %ld predicted)  -  Exiting",lStartOff,lRes,sizeof( unsigned short int ) * m_lNumIndicies, m_sBaseHeader.sizeIndexData);
		LogFile( ERROR_LOG, "SH2_MapPrimitiveStatic::LoadPrimitive( off: %ld ) - ERROR: Couldn't read indicies (%d of %d bytes read, %ld predicted)  -  Exiting",lStartOff,lRes,sizeof( unsigned short int ) * m_lNumIndicies, m_sBaseHeader.sizeIndexData);
		DeleteDynamicData( );
		return 0;
	}

	lTotalRead += lRes;

	for( k = 0; k < m_lNumIndicies; k ++ )
		m_pusIndicies[ k ] = tIndicies[ k ];
	delete [] tIndicies;

	if( debugMode )
		LogFile( ERROR_LOG, "SH2_MapPrimitiveStatic::LoadPrimitive( ) -------------- End --------------");
	
	return lTotalRead;
}


/*-----------------------------------------------------------------------------*/
/* SH2_MapPrimitiveStatic::RenderPrimitive                                     */
/*                                                                             */
/*-----------------------------------------------------------------------------*/
void SH2_MapPrimitiveStatic::RenderPrimitive( sh2_map_unknown_data *psUnknownData )
{
	char texName[16];
	long k,j, i, l;
	long texNum;
	GLuint texID;
	unsigned short int startInd = 0;
	bool alphaBlend = false;
	bool alphaTest = false;
	//static bool m_bFirstTime= true;
	GLenum primType;
	
	if( m_lSegmentID != 0 )
	{

		if( !onlyVar )
		{
			bool foundIt = false;
			for( k = 0; k < sizeof( g_lNewList) / sizeof( g_lNewList[ 0 ] ); k++ )
			{
				if( g_lNewList[ k ] == m_lSegmentID )
					foundIt = true;
			}
			if( !foundIt )
				return;

		}
		else if( onlyVar )
		{
			bool foundIt = false;
			for( k = 0; k < sizeof( g_lOldList) / sizeof( g_lOldList[ 0 ] ); k++ )
			{
				if( g_lOldList[ k ] == m_lSegmentID )
					foundIt = true;
			}
			if( !foundIt )
				return;
		}

		if( !onlyStatic )
		{
			for( k = 0; k < sizeof( g_lBothList) / sizeof( g_lBothList[ 0 ] ); k++ )
			{
				if( g_lBothList[ k ] == m_lSegmentID )
					return;
			}
		}
	}

	
	glEnableClientState(GL_VERTEX_ARRAY);
	glClientActiveTextureARB(GL_TEXTURE0_ARB);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	//glEnable( GL_POLYGON_OFFSET_FILL );	

if( m_bFirstTime && debugMode )	LogFile( ERROR_LOG, "Point ");	
//	checkGLerror(__LINE__,__FILE__,"scenePrimitive::draw - After client size pointer initialization");

	if( m_bFirstTime && dumpModel )
	{
		long	l_lNumVertSets = 0;

		//---[ Determine The Number Of Different Vertex Types ]---/
		for( k = 0; k < m_lNumTexRanges; k++ )
		{
			if( m_sStaticTexRanges[ k ].vertexSet > l_lNumVertSets )
				l_lNumVertSets = m_sStaticTexRanges[ k ].vertexSet;
		}

		//---[ Loop Through The Different Vertex Types, First Printing All Verts, Then The Faces ]---/
		for( i = 0; i < l_lNumVertSets + 1; i++ )
		{
			OBJ_Dump.StartNewVertexRange( );
			
			//---[ For Each Vertex Set, Print Out The Verts ]---/
			switch( m_sVertexSize.m_psSizes[ i ].sizeVertex )
			{
				case sizeof(sh2_map_vertex):
					for( j = 0; j < m_sVertexSize.m_psSizes[ i ].sizeVertexData / m_sVertexSize.m_psSizes[ i ].sizeVertex; j++ )
					{
						OBJ_Dump.AddVertex( m_psVerts[ j ].vert );
						OBJ_Dump.AddNormal( m_psVerts[ j ].norm );
						OBJ_Dump.AddTexcoord( m_psVerts[ j ].tc );
					}
					break;
				case sizeof(sh2_color_vertex):
					for( j = 0; j < m_sVertexSize.m_psSizes[ i ].sizeVertexData / m_sVertexSize.m_psSizes[ i ].sizeVertex; j++ )
					{
						OBJ_Dump.AddVertex( m_psColorVerts[ j ].vert );
						OBJ_Dump.AddNormal( m_psColorVerts[ j ].norm );
						OBJ_Dump.AddTexcoord( m_psColorVerts[ j ].tc );
					}
					break;
				case sizeof(sh2_vertex):
					for( j = 0; j < m_sVertexSize.m_psSizes[ i ].sizeVertexData / m_sVertexSize.m_psSizes[ i ].sizeVertex; j++ )
					{
						OBJ_Dump.AddVertex( m_psSimpleVerts[ j ].vert );
						OBJ_Dump.AddTexcoord( m_psSimpleVerts[ j ].tc );
					}
					break;
				case sizeof(sh2_shader_vertex):
					for( j = 0; j < m_sVertexSize.m_psSizes[ i ].sizeVertexData / m_sVertexSize.m_psSizes[ i ].sizeVertex; j++ )
					{
						OBJ_Dump.AddVertex( m_psShaderVerts[ j ].vert );
						OBJ_Dump.AddTexcoord( m_psShaderVerts[ j ].tc );
					}
					break;
			}

			//---[ For Each Texture Range, Change The Group, And Change The Material ]---/
			for( k = 0; k < m_lNumTexRanges; k++ )
			{
				texNum = psUnknownData[ m_sStaticTexRanges[ k ].texNum ].id;
				sprintf( texName, "%15ld",texNum );
				
				if( m_sStaticTexRanges[ k ].vertexSet == i && m_sStaticTexRanges[ k ].primType == 3 )
				{
					OBJ_Dump.AddGroup( );
					OBJ_Dump.AddMaterial( texName, 0.08f );

					OBJ_Dump.AddComment( "Adding Triangles for texrange %ld of %ld", k+1, this->m_lNumTexRanges );
					for( j = 0; j < m_sStaticTexRanges[ k ].primType * m_sStaticTexRanges[ k ].numPrims; j+=3 )
					{
						switch( m_sVertexSize.m_psSizes[ m_sStaticTexRanges[ k ].vertexSet ].sizeVertex )
						{
							case sizeof(sh2_map_vertex):
							case sizeof(sh2_color_vertex):
								OBJ_Dump.AddFace( &(m_pusIndicies[ startInd + j ]), true, true );
								break;
							case sizeof(sh2_vertex):	
							case sizeof(sh2_shader_vertex):
								OBJ_Dump.AddFace( &(m_pusIndicies[ startInd + j ]), true, false );
								break;
						}
					}
					startInd += m_sStaticTexRanges[ k ].primType * m_sStaticTexRanges[ k ].numPrims;
				}
				else if( m_sStaticTexRanges[ k ].vertexSet == i )
				{
					OBJ_Dump.AddGroup( );
					OBJ_Dump.AddMaterial( texName, 0.08f );

					OBJ_Dump.AddComment( "Adding Tristrip for texrange %ld of %ld", k+1, this->m_lNumTexRanges );
					OBJ_Dump.DecodeTriStrip( true );
					for( j = 0; j < m_sStaticTexRanges[ k ].numPrims; j ++	)//= m_sStaticTexRanges[ k ].primType )
					{
						switch( m_sVertexSize.m_psSizes[ m_sStaticTexRanges[ k ].vertexSet ].sizeVertex )
						{
							case sizeof(sh2_map_vertex):
							case sizeof(sh2_color_vertex):
								for( l = 0; l < m_sStaticTexRanges[ k ].primType - 2 || l == 0 ; l++ )
									OBJ_Dump.AddFace( &(m_pusIndicies[ startInd + j * m_sStaticTexRanges[ k ].primType + l ]), true, true );
								break;
							case sizeof(sh2_vertex):	
							case sizeof(sh2_shader_vertex):
								for( l = 0; l < m_sStaticTexRanges[ k ].primType - 2 || l == 0 ; l++ )
									OBJ_Dump.AddFace( &(m_pusIndicies[ startInd + j * m_sStaticTexRanges[ k ].primType + l ]), true, false );
								break;
						}
						//startInd += m_sStaticTexRanges[ k ].primType;
					}
					startInd += m_sStaticTexRanges[ k ].primType * m_sStaticTexRanges[ k ].numPrims;
				}
				OBJ_Dump.DecodeTriStrip( false );
			}
		}		//---[ End VERTEX TYPE LOOP ]---/

	}	//---[ END MODEL DUMP ]---/

	startInd = 0;

	for( k = 0; k < m_lNumTexRanges; k ++ )
	{
		glEnable(GL_TEXTURE_2D);
		texNum = psUnknownData[ m_sStaticTexRanges[ k ].texNum ].id;
		sprintf( texName, "%15ld",texNum );
	/*	if( m_bFirstTime && dumpModel )
		{
			OBJ_Dump.AddGroup( );
			OBJ_Dump.StartNewVertexRange( );
			OBJ_Dump.AddMaterial( texName );

			
			switch( m_sVertexSize.m_psSizes[ m_sStaticTexRanges[ k ].vertexSet ].sizeVertex )
			{
				case sizeof(sh2_map_vertex):
					for( j = 0; j < m_sVertexSize.m_psSizes[ m_sStaticTexRanges[ k ].vertexSet ].sizeVertexData / m_sVertexSize.m_psSizes[ m_sStaticTexRanges[ k ].vertexSet ].sizeVertex; j++ )
					{
						OBJ_Dump.AddVertex( m_psVerts[ j ].vert );
						OBJ_Dump.AddNormal( m_psVerts[ j ].norm );
						OBJ_Dump.AddTexcoord( m_psVerts[ j ].tc );
					}
					break;
				case sizeof(sh2_color_vertex):
					for( j = 0; j < m_sVertexSize.m_psSizes[ m_sStaticTexRanges[ k ].vertexSet ].sizeVertexData / m_sVertexSize.m_psSizes[ m_sStaticTexRanges[ k ].vertexSet ].sizeVertex; j++ )
					{
						OBJ_Dump.AddVertex( m_psColorVerts[ j ].vert );
						OBJ_Dump.AddNormal( m_psColorVerts[ j ].norm );
						OBJ_Dump.AddTexcoord( m_psColorVerts[ j ].tc );
					}
					break;
				case sizeof(sh2_vertex):
					for( j = 0; j < m_sVertexSize.m_psSizes[ m_sStaticTexRanges[ k ].vertexSet ].sizeVertexData / m_sVertexSize.m_psSizes[ m_sStaticTexRanges[ k ].vertexSet ].sizeVertex; j++ )
					{
						OBJ_Dump.AddVertex( m_psSimpleVerts[ j ].vert );
						OBJ_Dump.AddTexcoord( m_psSimpleVerts[ j ].tc );
					}
					break;
				case sizeof(sh2_shader_vertex):
					for( j = 0; j < m_sVertexSize.m_psSizes[ m_sStaticTexRanges[ k ].vertexSet ].sizeVertexData / m_sVertexSize.m_psSizes[ m_sStaticTexRanges[ k ].vertexSet ].sizeVertex; j++ )
					{
						OBJ_Dump.AddVertex( m_psShaderVerts[ j ].vert );
						OBJ_Dump.AddTexcoord( m_psShaderVerts[ j ].tc );
					}
					break;
			}
		}
*/
		texID = sh2TexList.GetTex( string( texName ), true);
		if( !isResident( texID ) && debugMode )
			LogFile(ERROR_LOG,"SH2_MapPrimitiveStatic::RenderPrimitive( ) - TERMINAL ERROR\n#############################\n  TexID %ld is not resident!!!\n##############################",texID);
//		if( psUnknownData[ m_sStaticTexRanges[ k ].texNum ].q1 != 1 )
			glEnable( GL_BLEND );
		if( m_bFirstTime && debugMode )
			LogFile(ERROR_LOG,"RIGHT AFTER GET TEX -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-==-=-=-=-=-");

		//if( texID < 1 )
		//	texID = 1;
		glActiveTextureARB(GL_TEXTURE0_ARB);
		glBindTexture(GL_TEXTURE_2D, texID);
//		checkGLerror(__LINE__,__FILE__,"SH2_MapLoader::RenderMap( ) - after static texture bind");

		if( m_sStaticTexRanges[ k ].primType == 3 )
			primType = GL_TRIANGLES;
		else if( m_sStaticTexRanges[ k ].primType < 3 )
		{
			if( debugMode && m_bFirstTime )
			{
				LogFile( ERROR_LOG, "SH2_MapPrimitiveStatic::RenderPrimitive( ) - ERROR: Cannot render primitive:  Invalid Primitive type: %d\n\tSkipping...",m_sStaticTexRanges[ k ].primType );
				LogFile(  TEST_LOG, "SH2_MapPrimitiveStatic::RenderPrimitive( ) - ERROR: Cannot render primitive:  Invalid Primitive type: %d\n\tSkipping...",m_sStaticTexRanges[ k ].primType );
			}
			return;
		}
		else
			primType = GL_TRIANGLE_STRIP;
		
		switch( m_sVertexSize.m_psSizes[ m_sStaticTexRanges[ k ].vertexSet ].sizeVertex )
		{
			case sizeof(sh2_map_vertex):
				glVertexPointer(3, GL_FLOAT, sizeof(sh2_map_vertex) , &(m_psVerts[ 0 ].vert));
				glTexCoordPointer(2, GL_FLOAT, sizeof(sh2_map_vertex), &(m_psVerts[0].tc));
				break;
			case sizeof(sh2_vertex):
				glVertexPointer(3, GL_FLOAT, sizeof(sh2_vertex) , &(m_psSimpleVerts[ 0 ].vert));
				glTexCoordPointer(2, GL_FLOAT, sizeof(sh2_vertex), &(m_psSimpleVerts[0].tc));
				break;
			case sizeof(sh2_color_vertex):
				glVertexPointer(3, GL_FLOAT, sizeof(sh2_color_vertex) , &(m_psColorVerts[ 0 ].vert));
				glTexCoordPointer(2, GL_FLOAT, sizeof(sh2_color_vertex), &(m_psColorVerts[0].tc));
				break;
			case sizeof(sh2_shader_vertex):
				glVertexPointer(3, GL_FLOAT, sizeof(sh2_shader_vertex) , &(m_psShaderVerts[ 0 ].vert));
				glTexCoordPointer(2, GL_FLOAT, sizeof(sh2_shader_vertex), &(m_psShaderVerts[0].tc));
				break;
			default:
				if( m_bFirstTime )
				{
					LogFile( ERROR_LOG, "SH2_MapPrimitiveStatic::RenderPrimitive( ) - ERROR: Unexpected Vertex Size: %ld bytes in variable range %ld of %ld  -  Exiting\n", m_sVertexSize.m_psSizes[ m_sStaticTexRanges[ k ].vertexSet ].sizeVertex, k + 1, m_lNumTexRanges );
					LogFile(  TEST_LOG, "SH2_MapPrimitiveStatic::RenderPrimitive( ) - ERROR: Unexpected Vertex Size: %ld bytes in variable range %ld of %ld  -  Exiting\n", m_sVertexSize.m_psSizes[ m_sStaticTexRanges[ k ].vertexSet ].sizeVertex, k + 1, m_lNumTexRanges );
				}
				return;
		}

		//---[ Output the data ]---/
/*		if( dumpModel && m_bFirstTime )
		{
			if( primType == GL_TRIANGLES )
			{
				OBJ_Dump.AddComment( "Adding Triangles for texrange %ld of %ld", k+1, this->m_lNumTexRanges );
				for( j = 0; j < m_sStaticTexRanges[ k ].primType * m_sStaticTexRanges[ k ].numPrims; j+=3 )
				{
					switch( m_sVertexSize.m_psSizes[ m_sStaticTexRanges[ k ].vertexSet ].sizeVertex )
					{
						case sizeof(sh2_map_vertex):
						case sizeof(sh2_color_vertex):
							OBJ_Dump.AddFace( &(m_pusIndicies[ startInd + j ]), true, true );
							break;
						case sizeof(sh2_vertex):	
						case sizeof(sh2_shader_vertex):
							OBJ_Dump.AddFace( &(m_pusIndicies[ startInd + j ]), true, false );
							break;
					}
				}
			}
			else
			{
				OBJ_Dump.AddComment( "Adding Tristrip for texrange %ld of %ld", k+1, this->m_lNumTexRanges );
				OBJ_Dump.DecodeTriStrip( true );
				for( j = 0; j < m_sStaticTexRanges[ k ].numPrims; j += m_sStaticTexRanges[ k ].primType )
				{
					switch( m_sVertexSize.m_psSizes[ m_sStaticTexRanges[ k ].vertexSet ].sizeVertex )
					{
						case sizeof(sh2_map_vertex):
						case sizeof(sh2_color_vertex):
							for( i = 0; i < m_sStaticTexRanges[ k ].primType - 2; i++ )
								OBJ_Dump.AddFace( &(m_pusIndicies[ startInd + j + i ]), true, true );
							break;
						case sizeof(sh2_vertex):	
						case sizeof(sh2_shader_vertex):
							for( i = 0; i < m_sStaticTexRanges[ k ].primType - 2; i++ )
								OBJ_Dump.AddFace( &(m_pusIndicies[ startInd + j + i ]), true, false );
							break;
					}
				}
			}
			OBJ_Dump.DecodeTriStrip( false );
		}
*/

		//---[ Render Each Texture Range ]---/

		if( m_sStaticTexRanges[ k ].primType == 3 )
		{
			if( texNum != 0 )
				glDrawElements(GL_TRIANGLES, m_sStaticTexRanges[ k ].primType * m_sStaticTexRanges[ k ].numPrims, GL_UNSIGNED_INT, &(m_pusIndicies[ startInd ]));
			else if( m_bFirstTime )
				LogFile( ERROR_LOG,"SH2_MapPrimitiveStatic::RenderPrimitive - ERROR: BAD TEX: Range: %ld of %ld   Prim: Static   Type: %ld   Q2: %ld   VertSet: %ld ",k+1,m_lNumTexRanges,m_sStaticTexRanges[ k ].primType,m_sStaticTexRanges[ k ].q2,m_sStaticTexRanges[ k ].vertexSet);
			startInd += m_sStaticTexRanges[ k ].primType * (unsigned short)m_sStaticTexRanges[ k ].numPrims;
		}
		else
		{
			for( j = 0; j < m_sStaticTexRanges[ k ].numPrims; j ++)
			{
				if( texNum != 0 )
					glDrawElements( primType, m_sStaticTexRanges[ k ].primType, GL_UNSIGNED_INT, &(m_pusIndicies[ startInd ]) );
				else if( m_bFirstTime )
					LogFile( ERROR_LOG,"SH2_MapPrimitiveStatic::RenderPrimitive - ERROR: BAD TEX: Range: %ld of %ld   Prim: Static   Type: %ld   Q2: %ld   VertSet: %ld ",k+1,m_lNumTexRanges,m_sStaticTexRanges[ k ].primType,m_sStaticTexRanges[ k ].q2,m_sStaticTexRanges[ k ].vertexSet);
				startInd += m_sStaticTexRanges[ k ].primType;
				checkGLerror(__LINE__,__FILE__,"scenePrimitive::draw - After draw command");
			}
		}
		glDisable(GL_TEXTURE_2D);
	}
	
//	checkGLerror(__LINE__,__FILE__,"scenePrimitive::draw - After completed all draw command for current prim");


	//glDisable( GL_POLYGON_OFFSET_FILL );
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);

//	if(alphaBlend )
		glDisable(GL_BLEND);
//	if(alphaTest)
		glDisable(GL_ALPHA_TEST);

	m_bFirstTime = false;
}


/*-----------------------------------------------------------------------------*/
/* SH2_MapPrimitiveVar::operator=                                              */
/*    Assignment operator for the variable map primitive                       */
/*-----------------------------------------------------------------------------*/
SH2_MapPrimitiveVar & SH2_MapPrimitiveVar::operator=( const SH2_MapPrimitiveVar & rhs )
{
	int k;
	if( &rhs != this )
	{
		m_lNumIndicies		= rhs.m_lNumIndicies;
		m_lNumVerts			= rhs.m_lNumVerts;
		m_lNumSimpleVerts	= rhs.m_lNumSimpleVerts;
		m_lNumColorVerts	= rhs.m_lNumColorVerts;
		m_lNumShaderVerts	= rhs.m_lNumShaderVerts;

		SAFEDELETE( m_psVerts );
		SAFEDELETE( m_psSimpleVerts );
		SAFEDELETE( m_psColorVerts );
		SAFEDELETE( m_psShaderVerts );
		m_psVerts		= new sh2_map_vertex[ m_lNumVerts ];
		m_psSimpleVerts = new sh2_vertex[ m_lNumSimpleVerts ];
		m_psColorVerts	= new sh2_color_vertex[ m_lNumColorVerts ];
		m_psShaderVerts		= new sh2_shader_vertex[ m_lNumShaderVerts ];
		memcpy( (void *)m_psVerts, (void *)rhs.m_psVerts, sizeof( sh2_map_vertex ) * m_lNumVerts );
		memcpy( (void *)m_psSimpleVerts, (void *)rhs.m_psSimpleVerts, sizeof( sh2_vertex ) * m_lNumSimpleVerts );
		memcpy( (void *)m_psColorVerts, (void *)rhs.m_psColorVerts, sizeof( sh2_color_vertex ) * m_lNumColorVerts );
		memcpy( (void *)m_psShaderVerts, (void *)rhs.m_psShaderVerts, sizeof( sh2_shader_vertex ) * m_lNumShaderVerts );

		SAFEDELETE( m_pusIndicies );
		m_pusIndicies	= new unsigned int[ m_lNumIndicies ];
		memcpy( (void *)m_pusIndicies, (void *)rhs.m_pusIndicies, sizeof( unsigned int ) * m_lNumIndicies );

		memcpy( (void *)&m_sBaseHeader, (void *)&rhs.m_sBaseHeader, sizeof( sh2_map_prim_base_header ));
		m_lNumTexRanges = rhs.m_lNumTexRanges;

		SAFEDELETE( m_psVariableRange );
		m_psVariableRange = new sh2_map_prim_variable_range[ m_lNumTexRanges ];
		for( k = 0; k < rhs.m_lNumTexRanges; k ++ )
			m_psVariableRange[ k ] = rhs.m_psVariableRange[ k ];

		m_sVertexSize = rhs.m_sVertexSize;
		m_lSegmentID = rhs.m_lSegmentID;
		m_bFirstTime = rhs.m_bFirstTime;
	}
	return *this;
}


/*-----------------------------------------------------------------------------*/
/* SH2_MapPrimitiveVar::LoadPrimitive                                          */
/*   Loads the primtive for the current location. Needs validations and checks */
/*-----------------------------------------------------------------------------*/
int  SH2_MapPrimitiveVar::LoadPrimitive( FILE *inFile, long lPrimSize )
{
	long k;
	long lRes;
	long lTotalRead = 0;
	long lVertDataRead = 0;
	long lSizePrimData;
	long lNumTexRanges;
	void *pVertDest = NULL;
	long *pVCountDest = NULL;
	unsigned short int *tIndicies;
	long lStartOff = ftell( inFile );
	bool loadComplex = false;

	if( debugMode )
		LogFile( ERROR_LOG, "SH2_MapPrimitiveVar::LoadPrimitive( off: %ld ) -------------- Start --------------",lStartOff);

	//---[ READ THE HEADER PORTIONS FOR THE PRIMITIVE ]---/

	if( ( lRes = fread( (void *)&m_sBaseHeader, 1, sizeof( sh2_map_prim_base_header ), inFile ) ) < sizeof( sh2_map_prim_base_header ))
	{
		LogFile(  TEST_LOG, "SH2_MapPrimitiveVar::LoadPrimitive( off: %ld ) - ERROR: Couldn't read sh2_map_prim_base_header (%d of %d bytes read)  -  Exiting\n\t\tREASON: %s",lStartOff,lRes,sizeof(sh2_map_prim_base_header),strerror(errno));
		LogFile( ERROR_LOG, "SH2_MapPrimitiveVar::LoadPrimitive( off: %ld ) - ERROR: Couldn't read sh2_map_prim_base_header (%d of %d bytes read)  -  Exiting\n\t\tREASON: %s",lStartOff,lRes,sizeof(sh2_map_prim_base_header),strerror(errno));
		return 0;
	}

	lTotalRead += lRes;

	if( debugMode )
		debugMPBH( &m_sBaseHeader, ftell(inFile) );


	if( ( lRes = fread( (void *)&lSizePrimData, 1, sizeof( long ), inFile ) ) < sizeof( long ))
	{
		LogFile(  TEST_LOG, "SH2_MapPrimitiveVar::LoadPrimitive( off: %ld ) - ERROR: Couldn't read lSizePrimData (%d of %d bytes read)  -  Exiting\n\t\tREASON: %s",lStartOff,lRes,sizeof(long),strerror(errno));
		LogFile( ERROR_LOG, "SH2_MapPrimitiveVar::LoadPrimitive( off: %ld ) - ERROR: Couldn't read lSizePrimData (%d of %d bytes read)  -  Exiting\n\t\tREASON: %s",lStartOff,lRes,sizeof(long),strerror(errno));
		return 0;
	}

	lTotalRead += lRes;

	if( ( lRes = fread( (void *)&lNumTexRanges, 1, sizeof( long ), inFile ) ) < sizeof( long ))
	{
		LogFile(  TEST_LOG, "SH2_MapPrimitiveVar::LoadPrimitive( off: %ld ) - ERROR: Couldn't read lNumTexRanges (%d of %d bytes read)  -  Exiting\n\t\tREASON: %s",lStartOff,lRes,sizeof(long),strerror(errno));
		LogFile( ERROR_LOG, "SH2_MapPrimitiveVar::LoadPrimitive( off: %ld ) - ERROR: Couldn't read lNumTexRanges (%d of %d bytes read)  -  Exiting\n\t\tREASON: %s",lStartOff,lRes,sizeof(long),strerror(errno));
		return 0;
	}

	lTotalRead += lRes;

	//---[ Load in all primitive ranges for the variable header ]---/
	//lNumTexRanges = (m_sBaseHeader.headerSize - sizeof( sh2_map_prim_base_header ) - sizeof( sh2_map_prim_variable_header ))/sizeof(sh2_map_prim_variable_range);
	//lRes = (m_sBaseHeader.headerSize - sizeof( sh2_map_prim_base_header ) - sizeof( long ) * 2)%sizeof(sh2_map_prim_variable_range);


	//if( lRes != 0)
	//{
	//	LogFile( ERROR_LOG, "SH2_MapPrimitiveVar::LoadPrimitive( off: %ld ) - ERROR: %s: HeaderSize: %ld  Expected Tex Ranges: %ld  Unexpected Bytes: %ld",lStartOff,(lRes)?"Header size incorrect for tristrip":"Num Tex Ranges in header and by computation don't match",m_sBaseHeader.headerSize,lNumTexRanges, lRes);
	//	LogFile(  TEST_LOG, "SH2_MapPrimitiveVar::LoadPrimitive( off: %ld ) - ERROR: %s: HeaderSize: %ld  Expected Tex Ranges: %ld  Unexpected Bytes: %ld",lStartOff,(lRes)?"Header size incorrect for tristrip":"Num Tex Ranges in header and by computation don't match",m_sBaseHeader.headerSize,lNumTexRanges, lRes);
	//	return 0;
	//}
	
	m_psVariableRange = new sh2_map_prim_variable_range[ lNumTexRanges ];

	m_lNumTexRanges = lNumTexRanges;

	for( k = 0; k < lNumTexRanges; k ++ )
	{
		lRes = m_psVariableRange[ k ].LoadTexRange( inFile );

		if( lRes == -1 )
		{
			LogFile(  TEST_LOG, "SH2_MapPrimitiveVar::LoadPrimitive( off: %ld ) - ERROR: Couldn't read sh2_map_prim_variable_range %ld of %ld (%d of %d bytes read)  -  Exiting\n\t\tREASON: %s",lStartOff,k+1,lNumTexRanges,lRes,sizeof(sh2_map_prim_variable_range),strerror(errno));
			LogFile( ERROR_LOG, "SH2_MapPrimitiveVar::LoadPrimitive( off: %ld ) - ERROR: Couldn't read sh2_map_prim_variable_range %ld of %ld (%d of %d bytes read)  -  Exiting\n\t\tREASON: %s",lStartOff,k+1,lNumTexRanges,lRes,sizeof(sh2_map_prim_variable_range),strerror(errno));
			SAFEDELETE( m_psVariableRange );
			return 0;
		}

		lTotalRead += lRes;

		if( debugMode )
			debugMPVR( &( m_psVariableRange[ k ] ), ftell(inFile) );

		//if( m_psVariableRange[ k ].numTriRanges != 1 && m_psVariableRange[ k ].numTriRanges != 3 )
		//{
		//	LogFile( ERROR_LOG, "SH2_MapPrimitiveVar::LoadPrimitive( off: %ld ) - ERROR: Primitive Type Undefined: Expected: 1 or 3   Actual: %d",lStartOff, m_psVariableRange[k].numTriRanges );
		//	LogFile(  TEST_LOG, "SH2_MapPrimitiveVar::LoadPrimitive( off: %ld ) - ERROR: Primitive Type Undefined: Expected: 1 or 3   Actual: %d",lStartOff, m_psVariableRange[k].numTriRanges );
		//}

		//if( m_psVariableRange[ k ].vertexSet != 0 )
		//{
		//	LogFile( TEST_LOG, "SH2_MapPrimitiveVar::LoadPrimitive( off: %ld ) - CHECK: The vertex set is: %ld",lStartOff, m_psVariableRange[ k ].vertexSet );
		//	loadComplex = true;
		//}
	}



		//---[ LOAD IN VERTEX SIZE STRUCTURE ]---/
	lRes = m_sVertexSize.LoadVertexSizes( inFile );
	if( lRes == -1 )
	{
		LogFile(  TEST_LOG, "SH2_MapPrimitiveVar::LoadPrimitive( off: %ld ) - ERROR: Couldn't load the vertex size structure  -  Exiting\n\t\tREASON: %s",lStartOff, strerror( errno ) );
		LogFile( ERROR_LOG, "SH2_MapPrimitiveVar::LoadPrimitive( off: %ld ) - ERROR: Couldn't load the vertex size structure  -  Exiting\n\t\tREASON: %s",lStartOff, strerror( errno ) );
		return 0;
	}

	if( debugMode )
		debugMVS( &m_sVertexSize, ftell(inFile) );
	lTotalRead += lRes;

	//---[ READ IN THE VERTICIES AND INDICIES ]---/

	//--[ Check basic data to ensure proper format ]--/
	for( k = 0; k < m_sVertexSize.m_sSizeHeader.numVertexSizes; k++ )
	{
		switch( m_sVertexSize.m_psSizes[ k ].sizeVertex )
		{
			case sizeof(sh2_map_vertex):
				SAFEDELETE( m_psVerts );
					//--[ Set size of vertex array ]--/
				m_lNumVerts = m_sVertexSize.m_psSizes[ k ].sizeVertexData / m_sVertexSize.m_psSizes[ k ].sizeVertex;
				pVCountDest = &m_lNumVerts;
				m_psVerts = new sh2_map_vertex[ m_lNumVerts ];
				pVertDest = (void *)m_psVerts;
				break;
			case sizeof(sh2_vertex):
				SAFEDELETE( m_psSimpleVerts );
					//--[ Set size of vertex array ]--/
				m_lNumSimpleVerts = m_sVertexSize.m_psSizes[ k ].sizeVertexData / m_sVertexSize.m_psSizes[ k ].sizeVertex;
				pVCountDest = &m_lNumSimpleVerts;
				m_psSimpleVerts = new sh2_vertex[ m_lNumSimpleVerts ];
				pVertDest = (void *)m_psSimpleVerts;
				break;
			case sizeof(sh2_color_vertex):
				SAFEDELETE( m_psColorVerts );
					//--[ Set size of vertex array ]--/
				m_lNumColorVerts = m_sVertexSize.m_psSizes[ k ].sizeVertexData / m_sVertexSize.m_psSizes[ k ].sizeVertex;
				pVCountDest = &m_lNumColorVerts;
				m_psColorVerts = new sh2_color_vertex[ m_lNumColorVerts ];
				pVertDest = (void *)m_psColorVerts;
				break;
			case sizeof(sh2_shader_vertex):
				SAFEDELETE( m_psShaderVerts );
					//--[ Set size of vertex array ]--/
				m_lNumShaderVerts = m_sVertexSize.m_psSizes[ k ].sizeVertexData / m_sVertexSize.m_psSizes[ k ].sizeVertex;
				pVCountDest = &m_lNumShaderVerts;
				m_psShaderVerts = new sh2_shader_vertex[ m_lNumShaderVerts ];
				pVertDest = (void *)m_psShaderVerts;
				break;
			default:
				LogFile(  TEST_LOG, "SH2_MapPrimitiveVar::LoadPrimitive( off: %ld ) - ERROR: Unexpected Vertex Size: %ld bytes  -  Exiting\n",lStartOff, m_sVertexSize.m_psSizes[ k ].sizeVertex );
				LogFile( ERROR_LOG, "SH2_MapPrimitiveVar::LoadPrimitive( off: %ld ) - ERROR: Unexpected Vertex Size: %ld bytes  -  Exiting\n",lStartOff, m_sVertexSize.m_psSizes[ k ].sizeVertex );
				DeleteDynamicData( );
				return 0;
		}


		//--[ Sanity Check on offsets ]--/

		if( ftell(inFile) != lStartOff + lTotalRead
			|| lStartOff + lTotalRead != lStartOff + m_sBaseHeader.offsetIndex - m_sVertexSize.m_sSizeHeader.sizeVertexData + m_sVertexSize.m_psSizes[ k ].offsetToVerts )
		{
			LogFile(  TEST_LOG, "SH2_MapPrimitiveVar::LoadPrimitive( off: %ld ) - CHECK: VERTEX Curr Offset %ld\tRead Offset: %ld\tExpected Offset %ld",lStartOff,ftell(inFile),lStartOff + lTotalRead, lStartOff + m_sBaseHeader.offsetIndex - m_sVertexSize.m_sSizeHeader.sizeVertexData + m_sVertexSize.m_psSizes[ k ].offsetToVerts );
			LogFile( ERROR_LOG, "SH2_MapPrimitiveVar::LoadPrimitive( off: %ld ) - CHECK: VERTEX Curr Offset %ld\tRead Offset: %ld\tExpected Offset %ld",lStartOff,ftell(inFile),lStartOff + lTotalRead, lStartOff + m_sBaseHeader.offsetIndex - m_sVertexSize.m_sSizeHeader.sizeVertexData + m_sVertexSize.m_psSizes[ k ].offsetToVerts );
		}

		//--[ Load in vertex data ]--/

		if( ( lRes = fread( pVertDest, 1, m_sVertexSize.m_psSizes[ k ].sizeVertex * (*pVCountDest), inFile ) ) < m_sVertexSize.m_psSizes[ k ].sizeVertex * (*pVCountDest))
		{
			LogFile(  TEST_LOG, "SH2_MapPrimitiveVar::LoadPrimitive( off: %ld ) - ERROR: Couldn't read %ld byte verticies for vertex group %ld (%d of %d bytes read, %ld predicted)  -  Exiting",lStartOff,m_sVertexSize.m_psSizes[ k ].sizeVertex, k + 1,lRes,m_sVertexSize.m_psSizes[ k ].sizeVertex * (*pVCountDest), m_sVertexSize.m_psSizes[ k ].sizeVertexData);
			LogFile( ERROR_LOG, "SH2_MapPrimitiveVar::LoadPrimitive( off: %ld ) - ERROR: Couldn't read %ld byte verticies for vertex group %ld (%d of %d bytes read, %ld predicted)  -  Exiting",lStartOff,m_sVertexSize.m_psSizes[ k ].sizeVertex, k + 1,lRes,m_sVertexSize.m_psSizes[ k ].sizeVertex * (*pVCountDest), m_sVertexSize.m_psSizes[ k ].sizeVertexData);
			DeleteDynamicData( );
			return 0;
		}

		lTotalRead += lRes;
	}


	//--[ Sanity Check on offsets ]--/

	if( ftell(inFile) != lStartOff + lTotalRead
		|| lStartOff + lTotalRead != lStartOff + m_sBaseHeader.offsetIndex )
	{
		LogFile( ERROR_LOG, "SH2_MapPrimitiveVar::LoadPrimitive( off: %ld ) - CHECK: INDEX Curr Offset %ld\tRead Offset: %ld\tExpected Offset %ld",lStartOff,ftell(inFile),lStartOff + lTotalRead, lStartOff + m_sBaseHeader.offsetIndex );
		LogFile(  TEST_LOG, "SH2_MapPrimitiveVar::LoadPrimitive( off: %ld ) - CHECK: INDEX Curr Offset %ld\tRead Offset: %ld\tExpected Offset %ld",lStartOff,ftell(inFile),lStartOff + lTotalRead, lStartOff + m_sBaseHeader.offsetIndex );
	}

	//--[ Load in index data ]--/

	m_lNumIndicies = m_sBaseHeader.sizeIndexData / sizeof( unsigned short int );
	SAFEDELETE( m_pusIndicies );
	m_pusIndicies = new unsigned int[ m_lNumIndicies ];
	tIndicies = new unsigned short int[ m_lNumIndicies ];
	if( ( lRes = fread( (void *)tIndicies, 1, sizeof( unsigned short int ) * m_lNumIndicies, inFile ) ) < (long)sizeof( unsigned short int ) * m_lNumIndicies)
	{
		LogFile(  TEST_LOG, "SH2_MapPrimitiveVar::LoadPrimitive( off: %ld ) - ERROR: Couldn't read indicies (%d of %d bytes read, %ld predicted)  -  Exiting",lStartOff,lRes,sizeof( unsigned short int ) * m_lNumIndicies, m_sBaseHeader.sizeIndexData);
		LogFile( ERROR_LOG, "SH2_MapPrimitiveVar::LoadPrimitive( off: %ld ) - ERROR: Couldn't read indicies (%d of %d bytes read, %ld predicted)  -  Exiting",lStartOff,lRes,sizeof( unsigned short int ) * m_lNumIndicies, m_sBaseHeader.sizeIndexData);
		SAFEDELETE( m_psVerts );
		SAFEDELETE( m_pusIndicies );
		return 0;
	}

	lTotalRead += lRes;

	for( k = 0; k < m_lNumIndicies; k ++ )
		m_pusIndicies[ k ] = tIndicies[ k ];
	delete [] tIndicies;


	if( debugMode )
		LogFile( ERROR_LOG, "SH2_MapPrimitiveVar::LoadPrimitive( ) -------------- End --------------");

	return lTotalRead;
}


/*-----------------------------------------------------------------------------*/
/* SH2_MapPrimitiveVar::RenderPrimitive                                        */
/*   Renders the primitives for the variable length header                     */
/*-----------------------------------------------------------------------------*/
void SH2_MapPrimitiveVar::RenderPrimitive( sh2_map_unknown_data *psUnknownData )
{
	char texName[16];
	long j, k, i, l, h;
	long texNum;
	GLuint texID;
//	GLenum primType;
	bool alphaBlend = false;
	bool alphaTest = false;
	//static bool m_bFirstTime = true;
	
	unsigned short indexOffset = 0;

//	for( j = indexOffset; j < 512; j+=16 )
//					LogFile( ERROR_LOG, "%3d - %3d : %3d %3d %3d %3d %3d %3d %3d %3d %3d %3d %3d %3d %3d %3d %3d %3d\n",j,j+15,
//						m_pusIndicies[ indexOffset + j + 0 ],m_pusIndicies[ indexOffset + j + 1 ],m_pusIndicies[ indexOffset + j + 2 ],m_pusIndicies[ indexOffset + j + 3 ],
//						m_pusIndicies[ indexOffset + j + 4 ],m_pusIndicies[ indexOffset + j + 5 ],m_pusIndicies[ indexOffset + j + 6 ],m_pusIndicies[ indexOffset + j + 7 ],
//						m_pusIndicies[ indexOffset + j + 8 ],m_pusIndicies[ indexOffset + j + 9 ],m_pusIndicies[ indexOffset + j +10 ],m_pusIndicies[ indexOffset + j +11 ],
//						m_pusIndicies[ indexOffset + j +12 ],m_pusIndicies[ indexOffset + j +13 ],m_pusIndicies[ indexOffset + j +14 ],m_pusIndicies[ indexOffset + j +15 ]);
	glEnableClientState(GL_VERTEX_ARRAY);
	glClientActiveTextureARB(GL_TEXTURE0_ARB);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);

	if( m_lSegmentID != 0 )
	{

		if( !onlyVar )
		{
			bool foundIt = false;
			for( k = 0; k < sizeof( g_lNewList) / sizeof( g_lNewList[ 0 ] ); k++ )
			{
				if( g_lNewList[ k ] == m_lSegmentID )
					foundIt = true;
			}
			if( !foundIt )
				return;

		}
		else if( onlyVar )
		{
			bool foundIt = false;
			for( k = 0; k < sizeof( g_lOldList) / sizeof( g_lOldList[ 0 ] ); k++ )
			{
				if( g_lOldList[ k ] == m_lSegmentID )
					foundIt = true;
			}
			if( !foundIt )
				return;
		}

		if( !onlyStatic )
		{
			for( k = 0; k < sizeof( g_lBothList) / sizeof( g_lBothList[ 0 ] ); k++ )
			{
				if( g_lBothList[ k ] == m_lSegmentID )
					return;
			}
		}
	}

	//---[ EXPORT THE MODEL ]---/

	if( m_bFirstTime && dumpModel )
	{
		long	l_lNumVertSets = 0;

		//---[ Determine The Number Of Different Vertex Types ]---/
		for( k = 0; k < m_lNumTexRanges; k++ )
		{
			if( m_psVariableRange[ k ].vertexSet > l_lNumVertSets )
				l_lNumVertSets = m_psVariableRange[ k ].vertexSet;
		}

		//---[ Loop Through The Different Vertex Types, First Printing All Verts, Then The Faces ]---/
		for( i = 0; i < l_lNumVertSets + 1; i++ )
		{
			OBJ_Dump.StartNewVertexRange( );
			
			//---[ For Each Vertex Set, Print Out The Verts ]---/
			switch( m_sVertexSize.m_psSizes[ i ].sizeVertex )
			{
				case sizeof(sh2_map_vertex):
					for( j = 0; j < m_sVertexSize.m_psSizes[ i ].sizeVertexData / m_sVertexSize.m_psSizes[ i ].sizeVertex; j++ )
					{
						OBJ_Dump.AddVertex( m_psVerts[ j ].vert );
						OBJ_Dump.AddNormal( m_psVerts[ j ].norm );
						OBJ_Dump.AddTexcoord( m_psVerts[ j ].tc );
					}
					break;
				case sizeof(sh2_color_vertex):
					for( j = 0; j < m_sVertexSize.m_psSizes[ i ].sizeVertexData / m_sVertexSize.m_psSizes[ i ].sizeVertex; j++ )
					{
						OBJ_Dump.AddVertex( m_psColorVerts[ j ].vert );
						OBJ_Dump.AddNormal( m_psColorVerts[ j ].norm );
						OBJ_Dump.AddTexcoord( m_psColorVerts[ j ].tc );
					}
					break;
				case sizeof(sh2_vertex):
					for( j = 0; j < m_sVertexSize.m_psSizes[ i ].sizeVertexData / m_sVertexSize.m_psSizes[ i ].sizeVertex; j++ )
					{
						OBJ_Dump.AddVertex( m_psSimpleVerts[ j ].vert );
						OBJ_Dump.AddTexcoord( m_psSimpleVerts[ j ].tc );
					}
					break;
				case sizeof(sh2_shader_vertex):
					for( j = 0; j < m_sVertexSize.m_psSizes[ i ].sizeVertexData / m_sVertexSize.m_psSizes[ i ].sizeVertex; j++ )
					{
						OBJ_Dump.AddVertex( m_psShaderVerts[ j ].vert );
						OBJ_Dump.AddTexcoord( m_psShaderVerts[ j ].tc );
					}
					break;
			}

			//---[ For Each Texture Range, Change The Group, And Change The Material ]---/
			for( k = 0; k < m_lNumTexRanges; k++ )
			{
				texNum = psUnknownData[ m_psVariableRange[ k ].texNum ].id;
				sprintf( texName, "%15ld",texNum );
				
				for( h = 0; h < m_psVariableRange[ k ].numTriRanges; h++ )
				{
					if( m_psVariableRange[ k ].vertexSet == i && m_psVariableRange[ k ].ranges[ h ].primType == 3 )
					{
						OBJ_Dump.AddGroup( );
						if( m_psVariableRange[ k ].ranges[ h ].q1_mtr != 0 )
							OBJ_Dump.AddMaterial( texName, 0.08f );
						else
							OBJ_Dump.AddMaterial( texName );

						OBJ_Dump.AddComment( "Adding Triangles for texrange %ld of %ld", k+1, this->m_lNumTexRanges );
						for( j = 0; j < 3 * m_psVariableRange[ k ].ranges[ h ].numIndicies; j+=3 )
						{
							switch( m_sVertexSize.m_psSizes[ m_psVariableRange[ k ].vertexSet ].sizeVertex )
							{
								case sizeof(sh2_map_vertex):
								case sizeof(sh2_color_vertex):
									OBJ_Dump.AddFace( &(m_pusIndicies[ indexOffset + j ]), true, true );
									break;
								case sizeof(sh2_vertex):	
								case sizeof(sh2_shader_vertex):
									OBJ_Dump.AddFace( &(m_pusIndicies[ indexOffset + j ]), true, false );
									break;
							}
						}
						indexOffset += m_psVariableRange[ k ].ranges[ h ].numIndicies * 3;
					}
					else if( m_psVariableRange[ k ].vertexSet == i )
					{
						OBJ_Dump.AddGroup( );
						if( m_psVariableRange[ k ].ranges[ h ].q1_mtr != 0 )
							OBJ_Dump.AddMaterial( texName, 0.08f );
						else
							OBJ_Dump.AddMaterial( texName );


						OBJ_Dump.AddComment( "Adding Tristrip for texrange %ld of %ld", k+1, this->m_lNumTexRanges );
						OBJ_Dump.DecodeTriStrip( true );
						for( j = 0; j < m_psVariableRange[ k ].ranges[ h ].numIndicies - 2; j ++	)//= m_sStaticTexRanges[ k ].primType )
						{
							switch( m_sVertexSize.m_psSizes[ m_psVariableRange[ k ].vertexSet ].sizeVertex )
							{
								case sizeof(sh2_map_vertex):
								case sizeof(sh2_color_vertex):
									//for( l = 0; l < m_sStaticTexRanges[ k ].primType - 2; l++ )
										OBJ_Dump.AddFace( &(m_pusIndicies[ indexOffset + j ]), true, true );
									break;
								case sizeof(sh2_vertex):	
								case sizeof(sh2_shader_vertex):
									//for( l = 0; l < m_sStaticTexRanges[ k ].primType - 2; l++ )
										OBJ_Dump.AddFace( &(m_pusIndicies[ indexOffset + j ]), true, false );
									break;
							}
							//startInd += m_sStaticTexRanges[ k ].primType;
						}
						indexOffset += m_psVariableRange[ k ].ranges[ h ].numIndicies;
					
						OBJ_Dump.DecodeTriStrip( false );
					}
				}
			}
		}		//---[ End VERTEX TYPE LOOP ]---/

	}	//---[ END MODEL DUMP ]---/


	//---[ START RENDER ]---/
	indexOffset = 0;

	for( k = 0; k < m_lNumTexRanges; k ++ )
	{
		glEnable(GL_TEXTURE_2D);
		texNum = psUnknownData[ m_psVariableRange[ k ].texNum ].id;
		sprintf( texName, "%15ld", texNum);
		if( debugMode && m_bFirstTime )LogFile( ERROR_LOG, "Here 1 - texName: %s", texName);

		texID = sh2TexList.GetTex( string( texName ), true);

		if( !isResident( texID ) && debugMode )
			LogFile(ERROR_LOG,"SH2_MapPrimitiveVar::RenderPrimitive( ) - TERMINAL ERROR\n#############################\n  TexID %ld is not resident!!!\n##############################",texID);

		
		//		if( psUnknownData[ m_psVariableRange[ k ].texNum ].q1 != 4 )
//			glEnable( GL_BLEND );
			
//		if( texID < 1 )
//			texID = 1;
		glActiveTextureARB(GL_TEXTURE0_ARB);
		glBindTexture(GL_TEXTURE_2D, texID);
		
	//	checkGLerror(__LINE__,__FILE__,"SH2_MapLoader::RenderMap( ) - after variable texture bind");

		switch( m_sVertexSize.m_psSizes[ m_psVariableRange[ k ].vertexSet ].sizeVertex )
		{
			case sizeof(sh2_map_vertex):
				glVertexPointer(3, GL_FLOAT, sizeof(sh2_map_vertex) , &(m_psVerts[ 0 ].vert));
				glTexCoordPointer(2, GL_FLOAT, sizeof(sh2_map_vertex), &(m_psVerts[0].tc));
				break;
			case sizeof(sh2_vertex):
				glVertexPointer(3, GL_FLOAT, sizeof(sh2_vertex) , &(m_psSimpleVerts[ 0 ].vert));
				glTexCoordPointer(2, GL_FLOAT, sizeof(sh2_vertex), &(m_psSimpleVerts[0].tc));
				break;
			case sizeof(sh2_color_vertex):
				glVertexPointer(3, GL_FLOAT, sizeof(sh2_color_vertex) , &(m_psColorVerts[ 0 ].vert));
				glTexCoordPointer(2, GL_FLOAT, sizeof(sh2_color_vertex), &(m_psColorVerts[0].tc));
				break;
			case sizeof(sh2_shader_vertex):
				glVertexPointer(3, GL_FLOAT, sizeof(sh2_shader_vertex) , &(m_psShaderVerts[ 0 ].vert));
				glTexCoordPointer(2, GL_FLOAT, sizeof(sh2_shader_vertex), &(m_psShaderVerts[0].tc));
				break;
			default:
				if( m_bFirstTime )
				{
					LogFile( ERROR_LOG, "SH2_MapPrimitiveVar::RenderPrimitive( ) - ERROR: Unexpected Vertex Size: %ld bytes in variable range %ld of %ld  -  Exiting\n", m_sVertexSize.m_psSizes[ m_psVariableRange[ k ].vertexSet ].sizeVertex, k + 1, m_lNumTexRanges );
					LogFile(  TEST_LOG, "SH2_MapPrimitiveVar::RenderPrimitive( ) - ERROR: Unexpected Vertex Size: %ld bytes in variable range %ld of %ld  -  Exiting\n", m_sVertexSize.m_psSizes[ m_psVariableRange[ k ].vertexSet ].sizeVertex, k + 1, m_lNumTexRanges );
				}
				return;
		}


/*		if( m_bFirstTime )
		{
			if( m_psVariableRange[ k ].vertexSet < 2 )
			{
				for( j = indexOffset; j < m_psVariableRange[ k ].ranges[ 0 ].numIndicies + indexOffset; j+=16 )
					LogFile( ERROR_LOG, "%3d - %3d : %3d %3d %3d %3d %3d %3d %3d %3d %3d %3d %3d %3d %3d %3d %3d %3d\n",j,j+15,
						m_pusIndicies[ j + 0 ],m_pusIndicies[ j + 1 ],m_pusIndicies[ j + 2 ],m_pusIndicies[ j + 3 ],
						m_pusIndicies[ j + 4 ],m_pusIndicies[ j + 5 ],m_pusIndicies[ j + 6 ],m_pusIndicies[ j + 7 ],
						m_pusIndicies[ j + 8 ],m_pusIndicies[ j + 9 ],m_pusIndicies[ j +10 ],m_pusIndicies[ j +11 ],
						m_pusIndicies[ j +12 ],m_pusIndicies[ j +13 ],m_pusIndicies[ j +14 ],m_pusIndicies[ j +15 ]);
			}
		}
*/

	//	checkGLerror(__LINE__,__FILE__,"scenePrimitive::draw - After client size pointer initialization");

		for( j = 0; j < m_psVariableRange[ k ].numTriRanges; j++ )
		{
			if( m_psVariableRange[ k ].ranges[ j ].q1_mtr != 0 )
			{
				glEnable(GL_BLEND);
				glBlendFunc(GL_SRC_ALPHA,GL_DST_COLOR);
			}

			if( m_psVariableRange[ k ].ranges[ j ].primType == 3 )
			{
				if( texNum != 0 )
					glDrawElements(GL_TRIANGLES, m_psVariableRange[ k ].ranges[ j ].numIndicies*3, GL_UNSIGNED_INT, &(m_pusIndicies[ indexOffset ]));
				else if( m_bFirstTime )
					LogFile( ERROR_LOG,"SH2_MapPrimitiveVar::RenderPrimitive( ) - ERROR: BADTEX: Range: %ld  of %ld  Prim: Variable  Type: %ld  Q1: %ld",j+1,m_psVariableRange[ k ].numTriRanges,m_psVariableRange[ k ].ranges[ j ].primType,m_psVariableRange[ k ].ranges[ j ].q1_mtr);
				indexOffset += m_psVariableRange[ k ].ranges[ j ].numIndicies * 3;
			}
			else if( m_psVariableRange[ k ].ranges[ j ].primType == 1 )
			{
				if( texNum != 0 )
					glDrawElements(GL_TRIANGLE_STRIP, m_psVariableRange[ k ].ranges[ j ].numIndicies, GL_UNSIGNED_INT, &(m_pusIndicies[ indexOffset ]));
				else if( m_bFirstTime )
					LogFile( ERROR_LOG,"RENDER BADTEX: Range: %ld  of %ld  Prim: Variable  Type: %ld  Q1: %ld",j+1,m_psVariableRange[ k ].numTriRanges,m_psVariableRange[ k ].ranges[ j ].primType,m_psVariableRange[ k ].ranges[ j ].q1_mtr);
				indexOffset += m_psVariableRange[ k ].ranges[ j ].numIndicies;
			}
			else if( m_bFirstTime )
			{
				LogFile( ERROR_LOG, "SH2_MapPrimitiveVar::RenderPrimitive( ) - ERROR: Unexpected prim type: %d ",m_psVariableRange[ k ].ranges[ j ].primType );
				LogFile(  TEST_LOG, "SH2_MapPrimitiveVar::RenderPrimitive( ) - ERROR: Unexpected prim type: %d ",m_psVariableRange[ k ].ranges[ j ].primType );
				return;
			}
			
			glDisable(GL_BLEND);
			//glDrawElements(GL_TRIANGLE_STRIP, m_psVariableRange[ k ].ranges[ j ].numIndicies, GL_UNSIGNED_INT, &(m_pusIndicies[ indexOffset ]));
			//indexOffset += m_psVariableRange[ k ].ranges[ j ].numIndicies;
			//if( m_bFirstTime ) LogFile(ERROR_LOG,"CHECK: indexOffset: %d\tnumIndicies: %ld",indexOffset,m_psVariableRange[ k ].ranges[ j ].numIndicies);
		}

		glDisable(GL_TEXTURE_2D);
		checkGLerror(__LINE__,__FILE__,"scenePrimitive::draw - After draw command");

		
	}

	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);

	if(alphaBlend )
		glDisable(GL_BLEND);
	if(alphaTest)
		glDisable(GL_ALPHA_TEST);

	glDisable(GL_ALPHA_TEST);
	glDisable(GL_BLEND);

	m_bFirstTime = false;
}



/*-----------------------------------------------------------------------------*/
/* SH2_MapLoader::LoadMap                                                      */
/*   Loads the map, calling sub funcs for textures and primitives.             */
/*-----------------------------------------------------------------------------*/
int SH2_MapLoader::LoadMap( char *fileName )
{
	SH2_MapPrimitiveStatic	*primStatic = NULL;	//=[ Temp Variables For Primitives ]=/
	SH2_MapPrimitiveVar		*primVar = NULL;
	long	lmaxStatic = 10;					//=[ Counters For Temp Primitives ]=/
	long	lcurStatic = 0;
	long	lmaxVar    = 10;
	long	lcurVar    = 0;
	sh2_map_offset_index	primOffsets;		//=[ Primitive offsets, for current subset ]=/
	FILE	*inFile;							//=[ Map File Tracking Variables ]=/
	long	lCurOffset = 0;		//Current offset, when needed
	long	lPreOffset = 0;		//Used before reading a struct with an offset based on starting offset
	long	lNextOffset= 0;		//Used when calling a subfunction, to go to next section after data to read
	long	lRes;
	long	lPrimSize;
	long	*lpNextPrimOffset;	//Used as a fake array for the primitive index offsets
	sh2_map_prim_base_header	tempPBHeader;
	sh2_tex_base_header			m_sTexBaseHeader;
	int		k, j, i, h;
	vector<long>	loadedSections;
	bool alreadyDone = false;

	if( debugMode )
		LogFile( ERROR_LOG, "SH2_MapLoader::LoadMap( %s ) -------------- Start --------------", fileName);

	//---[ Reset Render Debug Mode Flag ]---/
	DeleteDynamicData( );

	m_bFirstTime = true;

	
	//---[ Open the file, and set the filename ]---/

	if( ( inFile = fopen( fileName, "rb" ) ) == NULL )
	{
		LogFile(  TEST_LOG, "SH2_MapLoader::LoadMap - ERROR: Couldn't open map file '%s'  -  Exiting",fileName);
		LogFile( ERROR_LOG, "SH2_MapLoader::LoadMap - ERROR: Couldn't open map file '%s'  -  Exiting",fileName);
		return 0;
	}

	m_sFilename = string( fileName );

	//---[ Read in map file header ]---/

	if( ( lRes = fread( (void *)&m_sFileHeader, 1, sizeof( sh2_map_file_header ), inFile ) ) < sizeof( sh2_map_file_header ))
	{
		LogFile(  TEST_LOG, "SH2_MapLoader::LoadMap( %s ) - ERROR: Couldn't read sh2_map_file_header (%d of %d bytes read)  -  Exiting\n\t\tREASON: %s",fileName,lRes,sizeof(sh2_map_file_header),strerror(errno));
		LogFile( ERROR_LOG, "SH2_MapLoader::LoadMap( %s ) - ERROR: Couldn't read sh2_map_file_header (%d of %d bytes read)  -  Exiting\n\t\tREASON: %s",fileName,lRes,sizeof(sh2_map_file_header),strerror(errno));
		fclose( inFile );
		return 0;
	}

	if( debugMode )
		debugMFH( &m_sFileHeader, ftell(inFile) );

	lCurOffset = ftell( inFile );

	
	//---[ R E A D   I N   B A S E   H E A D E R ]---/

	if( ( lRes = fread( (void *)&m_sTexBaseHeader, 1, sizeof( sh2_tex_base_header ), inFile ) ) < sizeof( sh2_tex_base_header ))
	{
		LogFile(  TEST_LOG, "SH2_MapLoader::LoadMap( %s ) read: %ld - ERROR: Couldn't read sh2_tex_base_header (%d of %d bytes read)  -  Exiting\n\t\tREASON: %s",fileName,lCurOffset, lRes,sizeof(sh2_tex_base_header),strerror(errno));
		LogFile( ERROR_LOG, "SH2_MapLoader::LoadMap( %s ) read: %ld - ERROR: Couldn't read sh2_tex_base_header (%d of %d bytes read)  -  Exiting\n\t\tREASON: %s",fileName,lCurOffset, lRes,sizeof(sh2_tex_base_header),strerror(errno));
		fclose( inFile );
		return 0;
	}

	//---[ C H E C K   I F   T H E R E   A R E   N O   T E X T U R E S ]---/

	if( m_sTexBaseHeader.texStructMarker == 0x20010730 )
	{
		//---[ Copy Primitive Data Already Loaded Over ]---/
		memcpy( (void *)&m_sBaseHeader, (void *)&(m_sFileHeader.q2), sizeof( sh2_map_base_header ));
		memcpy( (void *)&m_sOffsetUnknown,(void *)&m_sTexBaseHeader, sizeof( sh2_map_offset_unknown ));

	}
	else if( m_sTexBaseHeader.texStructMarker == 0x19990901 )
	{
		lNextOffset = m_sFileHeader.texDataSize + lCurOffset;	//Skip Texture data, go to start of primitive data.

		fseek( inFile, lNextOffset, SEEK_SET );


		//==-----==[ L O A D   P R I M I T I V E   H E A D E R   D A T A ]==-----==/

		//---[ Load in base header ]---/

		if( ( lRes = fread( (void *)&m_sBaseHeader, 1, sizeof( sh2_map_base_header ), inFile ) ) < sizeof( sh2_map_base_header ))
		{
			LogFile(  TEST_LOG, "SH2_MapLoader::LoadMap( %s : off: %ld ) - ERROR: Couldn't read sh2_map_base_header (%d of %d bytes read)  -  Exiting\n\t\tREASON: %s",fileName,lNextOffset,lRes,sizeof(sh2_map_base_header),strerror(errno));
			LogFile( ERROR_LOG, "SH2_MapLoader::LoadMap( %s : off: %ld ) - ERROR: Couldn't read sh2_map_base_header (%d of %d bytes read)  -  Exiting\n\t\tREASON: %s",fileName,lNextOffset,lRes,sizeof(sh2_map_base_header),strerror(errno));
			fclose( inFile );
			return 0;
		}

		if( debugMode )
			debugMBH( &m_sBaseHeader, ftell(inFile) );


		//---[ Load in unknown header and data at end of file ]---/

		lPreOffset = ftell( inFile );

		if( ( lRes = fread( (void *)&m_sOffsetUnknown, 1, sizeof( sh2_map_offset_unknown ), inFile ) ) < sizeof( sh2_map_offset_unknown ))
		{
			LogFile(  TEST_LOG, "SH2_MapLoader::LoadMap( %s : off: %ld ) - ERROR: Couldn't read sh2_map_offset_unknown (%d of %d bytes read)  -  Exiting\n\t\tREASON: %s",fileName,lPreOffset,lRes,sizeof(sh2_map_offset_unknown),strerror(errno));
			LogFile( ERROR_LOG, "SH2_MapLoader::LoadMap( %s : off: %ld ) - ERROR: Couldn't read sh2_map_offset_unknown (%d of %d bytes read)  -  Exiting\n\t\tREASON: %s",fileName,lPreOffset,lRes,sizeof(sh2_map_offset_unknown),strerror(errno));
			fclose( inFile );
			return 0;
		}

		if( debugMode )
			debugMOU( &m_sOffsetUnknown, ftell(inFile) );
   
	}

	//---[ S T A R T   N O N - F O R M A T   D E P E N D E N T   L O A D ]---/

	lCurOffset = ftell( inFile );

	if( m_sOffsetUnknown.numUnknowDataSets )
	{
		lNextOffset = lPreOffset + m_sOffsetUnknown.offsetUnknownData;
		fseek( inFile, lNextOffset, SEEK_SET );

		m_psUnknownData = new sh2_map_unknown_data[ m_sOffsetUnknown.numUnknowDataSets ];

		for( k = 0; k < m_sOffsetUnknown.numUnknowDataSets; k ++ )
		{
			if( ( lRes = fread( (void *)&(m_psUnknownData[k]), 1, sizeof( sh2_map_unknown_data ), inFile ) ) < sizeof( sh2_map_unknown_data ))
			{
				LogFile(  TEST_LOG, "SH2_MapLoader::LoadMap( %s : off: %ld ) - ERROR: Couldn't read sh2_map_unknown_data %d of %d (%d of %d bytes read)  -  Exiting\n\t\tREASON: %s",fileName,ftell( inFile ) - sizeof( sh2_map_unknown_data ), k+1,m_sOffsetUnknown.numUnknowDataSets,lRes,sizeof(sh2_map_unknown_data),strerror(errno));
				LogFile( ERROR_LOG, "SH2_MapLoader::LoadMap( %s : off: %ld ) - ERROR: Couldn't read sh2_map_unknown_data %d of %d (%d of %d bytes read)  -  Exiting\n\t\tREASON: %s",fileName,ftell( inFile ) - sizeof( sh2_map_unknown_data ), k+1,m_sOffsetUnknown.numUnknowDataSets,lRes,sizeof(sh2_map_unknown_data),strerror(errno));
				fclose( inFile );
				return 0;
			}

			if( debugMode )
			{
				LogFile( ERROR_LOG, " -- Unknown Data: %d of %d ---------------------------------------------------",k+1,m_sOffsetUnknown.numUnknowDataSets);
				debugMUD( &(m_psUnknownData[k]), ftell(inFile) );
			}
			/****************** D O   A N A L Y S I S *************************/
			//if( m_psUnknownData[k].q1 != 1 )
				LogFile( TEST_LOG, "NOTE: Unknkown data %d - TEX: %ld  f1 [%d]  q1 [%d]  q2 [%d]  q3 [%d]  q4 [%d]  q5 [%d]  f1 [%d]  endMarker [ 0x%08x ]",k + 1, m_psUnknownData[k].id,m_psUnknownData[k].f1,m_psUnknownData[k].q1, m_psUnknownData[k].q2, m_psUnknownData[k].q3, m_psUnknownData[k].q4, m_psUnknownData[k].q5, m_psUnknownData[k].f1, m_psUnknownData[k].endMarker);
			/****************** D O   A N A L Y S I S *************************/
		}
	}
	else
		m_psUnknownData = NULL;


	//====[ L O A D   I N   A L L   O F   T H E   P R I M I T I V E  D A T A   A N D   H E A D E R S ]====/

	//--[ Set Starting Offset ]--/
	lPreOffset = lCurOffset;

	//--[ Initialize temp prim data ]--/
	primStatic = new SH2_MapPrimitiveStatic[ lmaxStatic ];
	primVar = new SH2_MapPrimitiveVar[ lmaxVar ];

	//--[ Place File At Start Of Primitives ]--/
	fseek( inFile, lCurOffset, SEEK_SET );

	for( h = 0; h < m_sOffsetUnknown.numMapOffsetPrims; h++ )
	{

		//--[ Set Starting Offset ]--/
		lPreOffset = lCurOffset;


		//--[ Check To Make Sure We Aren't Reading Data Wrong ]--/
		if( ftell( inFile ) > lCurOffset )
		{
			LogFile( ERROR_LOG, "SH2_MapLoader::LoadMap( %s : off: %ld ) - ERROR: Incorrect file position after reading primitive block %d ( %ld current position, %ld expected position)",fileName,lPreOffset,h,ftell( inFile ), lCurOffset);
			LogFile(  TEST_LOG, "SH2_MapLoader::LoadMap( %s : off: %ld ) - ERROR: Incorrect file position after reading primitive block %d ( %ld current position, %ld expected position)",fileName,lPreOffset,h,ftell( inFile ), lCurOffset);
		}

		fseek( inFile, lCurOffset, SEEK_SET );

		if( ( lRes = fread( (void *)&m_sOffsetPrims, 1, sizeof( sh2_map_offset_prims ), inFile ) ) < sizeof( sh2_map_offset_prims ))
		{
			LogFile(  TEST_LOG, "SH2_MapLoader::LoadMap( %s : off: %ld ) - ERROR: Couldn't read sh2_map_offset_prims (%d of %d bytes read)  -  Exiting\n\t\tREASON: %s",fileName,lPreOffset,lRes,sizeof(sh2_map_offset_prims),strerror(errno));
			LogFile( ERROR_LOG, "SH2_MapLoader::LoadMap( %s : off: %ld ) - ERROR: Couldn't read sh2_map_offset_prims (%d of %d bytes read)  -  Exiting\n\t\tREASON: %s",fileName,lPreOffset,lRes,sizeof(sh2_map_offset_prims),strerror(errno));
			fclose( inFile );
			return 0;
		}

		if( debugMode )
			debugMOP( &m_sOffsetPrims, ftell(inFile) );

		LogFile( ERROR_LOG, "LoadMap( %s : off: %ld ) - TRACKING: Prim Block[ %d of %d ]  -  Value of f1 var in prim offset header:  %ld",fileName,lPreOffset,h+1,m_sOffsetUnknown.numMapOffsetPrims,m_sOffsetPrims.f1);
		LogFile(  TEST_LOG, "LoadMap( %s : off: %ld ) - TRACKING: Prim Block[ %d of %d ]  -  Value of f1 var in prim offset header:  %ld",fileName,lPreOffset,h+1,m_sOffsetUnknown.numMapOffsetPrims,m_sOffsetPrims.f1);

		//--[ Set Offset of Next Prim Block ]--/
		lCurOffset = lCurOffset + m_sOffsetPrims.sizeAllPrims;

		//if( m_sOffsetPrims.offsetMainPrim != 20 )
		//{
		//	LogFile( ERROR_LOG, "SH2_MapLoader::LoadMap( ) - ERROR: Offset of Main primitive implies different format: Expected: 20  Actual: %ld", m_sOffsetPrims.offsetMainPrim );
		//	LogFile(  TEST_LOG, "SH2_MapLoader::LoadMap( ) - ERROR: Offset of Main primitive implies different format: Expected: 20  Actual: %ld", m_sOffsetPrims.offsetMainPrim );
		//}

		//==-----==[ L O A D   E A C H   P R I M I T I V E   G R O U P   B Y   O F F S E T   I N D E X ]==-----==/

	
		//--[ Set offset to main primitive, and start reading ]--/
		lpNextPrimOffset = &m_sOffsetPrims.offsetMainPrim;

		i = 0;

		do
		{
	
			lNextOffset = lpNextPrimOffset[ i ] + lPreOffset;

			if( lpNextPrimOffset[ i ] != 0 && lNextOffset < m_sFileHeader.mapFileSize )
			{
				fseek( inFile, lNextOffset, SEEK_SET );

				if( ! LoadPrimOffsetIndex( inFile, &primOffsets ) )
				{
					LogFile(  TEST_LOG, "SH2_MapLoader::LoadMap( %s : off: %ld ) - ERROR: Couldn't read sh2_map_offset_index  -  Exiting",fileName,lNextOffset);
					LogFile( ERROR_LOG, "SH2_MapLoader::LoadMap( %s : off: %ld ) - ERROR: Couldn't read sh2_map_offset_index  -  Exiting",fileName,lNextOffset);
					fclose( inFile );
					return 0;
				}

				for( k = 0; k < primOffsets.numPrimIndex; k++ )
				{
					//---[ Resize Primitive Temp Arrays If Needed ]---/
					if( lcurStatic == lmaxStatic )	//Static Size Primitives
					{
						lmaxStatic += lmaxStatic;
						SH2_MapPrimitiveStatic *tempStatic = new SH2_MapPrimitiveStatic[ lmaxStatic ];
						for( j = 0; j < lcurStatic; j++)
							tempStatic[j] = primStatic[j];
						delete [] primStatic;
						primStatic = tempStatic;
					}
					if( lcurVar == lmaxVar )	//Variable Size Primitives
					{
						lmaxVar += lmaxVar;
						SH2_MapPrimitiveVar *tempVar = new SH2_MapPrimitiveVar[ lmaxVar ];
						for( j = 0; j < lcurVar; j++)
							tempVar[j] = primVar[j];
						delete [] primVar;
						primVar = tempVar;
					}

					//---[ Position File At Start Of Current Primitive and compute approx prim size ]---/
					lNextOffset = primOffsets.lPreOffset + primOffsets.pOffsets[ k ];
					fseek( inFile, lNextOffset, SEEK_SET );

					if( primOffsets.numPrimIndex - k - 1 )	//--[ If not 0, till more primitives in this index set ]--/
						lPrimSize = primOffsets.pOffsets[ k + 1 ] - primOffsets.pOffsets[ k ];
					else if( lpNextPrimOffset[ i + 1 ] > lpNextPrimOffset[ i ]		//--[ If there is another set of primtive index, use their start ]--/
							&& lpNextPrimOffset[ i + 1 ] < m_sOffsetPrims.sizeAllPrims )
						lPrimSize = lpNextPrimOffset[ i + 1 ] - primOffsets.pOffsets[ k ];
					else
						lPrimSize =  m_sOffsetPrims.sizeAllPrims - primOffsets.pOffsets[ k ];

					//---[ Read sh2_map_prim_base_header and determine primitive type ]---/
					if( ( lRes = fread( (void *)&tempPBHeader, 1, sizeof( sh2_map_prim_base_header ), inFile ) ) < sizeof( sh2_map_prim_base_header ) )
					{
						LogFile(  TEST_LOG, "SH2_MapLoader::LoadMap( %s : off: %ld ) - ERROR: Couldn't read sh2_map_prim_base_header for prim %d of %d -  Exiting",fileName,lNextOffset,k,primOffsets.numPrimIndex);	
						LogFile( ERROR_LOG, "SH2_MapLoader::LoadMap( %s : off: %ld ) - ERROR: Couldn't read sh2_map_prim_base_header for prim %d of %d -  Exiting",fileName,lNextOffset,k,primOffsets.numPrimIndex);	
						delete [] primVar;
						delete [] primStatic;
						fclose( inFile );
						return 0;
					}

					fseek( inFile, lNextOffset, SEEK_SET );

					alreadyDone = false;

/*					if( m_sOffsetPrims.f1 > 0 || loadedSections.size() )
					{
						for( int h = 0; h < loadedSections.size(); h++ )
							if( (loadedSections[ h ] & m_sOffsetPrims.f1) == m_sOffsetPrims.f1 )//loadedSections[ h ] )
							{
								alreadyDone = true;
								LogFile( TEST_LOG,"Bad Math? ls[%d] = %ld,  f1 = %ld,  ls[%d] | f1 = %ld",h,loadedSections[h],m_sOffsetPrims.f1,h,loadedSections[h]&m_sOffsetPrims.f1);
							}
							else
								alreadyDone = false;
					}

alreadyDone = false;

long lowVal = m_sOffsetPrims.f1 & 0x0000000f;
long hiVal = (m_sOffsetPrims.f1 & 0x00000f00 ) >> 8;

LogFile( TEST_LOG, "TEST: lowVal = %ld,  hiVal = %ld,  lowVal | hiVal = %ld",lowVal,hiVal,lowVal | hiVal);
if( m_sOffsetPrims.f1 != 2 
   && m_sOffsetPrims.f1 != 770 )//if( (lowVal | hiVal) == 3 )//||(lowVal | hiVal) == 0 )
	alreadyDone = true;
else
	alreadyDone = false;
*/
					if( i < 2 ) //(tempPBHeader.headerSize - 52)% sizeof(sh2_map_prim_variable_range) == 0 ) //Variable Primitive
					{
						SH2_MapPrimitiveVar tPrimVar;

						if( (lRes = tPrimVar.LoadPrimitive( inFile, lPrimSize )) != 0 )
						{
							if( debugMode )
								LogFile( ERROR_LOG, "SH2_MapLoader::LoadMap( %s : off: %ld ) - TEST: Var Prim Loader for prim %d of %d loaded %ld bytes for %ld projected bytes",fileName,lNextOffset,k,primOffsets.numPrimIndex, lRes, lPrimSize);	

							if( alreadyDone )
							{
								LogFile(  TEST_LOG, "SH2_MapLoader::LoadMap( %s : off: %ld ) - NOTICE: Var Prim %d of %d not loaded - Value of offset seq is %ld",fileName,lNextOffset,k,primOffsets.numPrimIndex, m_sOffsetPrims.f1);	
								LogFile( ERROR_LOG, "SH2_MapLoader::LoadMap( %s : off: %ld ) - NOTICE: Var Prim %d of %d not loaded - Value of offset seq is %ld",fileName,lNextOffset,k,primOffsets.numPrimIndex, m_sOffsetPrims.f1);	
							}
							else
							{
								//loadedSections.push_back( m_sOffsetPrims.f1 );
							primVar[ lcurVar ] = tPrimVar;
							primVar[ lcurVar ].m_lSegmentID = m_sOffsetPrims.f1;
							if( m_sOffsetPrims.f1 )
								m_bHasMultiView = true;
							++lcurVar;
							}
						}
						else
						{
							LogFile(  TEST_LOG, "SH2_MapLoader::LoadMap( %s : off: %ld ) - ERROR: Var Prim Loader failed for prim %d of %d with size of %ld projected bytes",fileName,lNextOffset,k,primOffsets.numPrimIndex, lPrimSize);	
							LogFile( ERROR_LOG, "SH2_MapLoader::LoadMap( %s : off: %ld ) - ERROR: Var Prim Loader failed for prim %d of %d with size of %ld projected bytes",fileName,lNextOffset,k,primOffsets.numPrimIndex, lPrimSize);	
						}

					}
					else if( i == 2 ) //(tempPBHeader.headerSize - 48) % sizeof(sh2_map_prim_static_ranges) == 0) //Static Primitive
					{
						SH2_MapPrimitiveStatic tPrimStatic;

						if( (lRes = tPrimStatic.LoadPrimitive( inFile, lPrimSize )) != 0 )
						{
							if( debugMode )
								LogFile( ERROR_LOG, "SH2_MapLoader::LoadMap( %s : off: %ld ) - TEST: Static Prim Loader for prim %d of %d loaded %ld bytes for %ld projected bytes",fileName,lNextOffset,k,primOffsets.numPrimIndex, lRes, lPrimSize);	

							if( alreadyDone )
							{
								LogFile(  TEST_LOG, "SH2_MapLoader::LoadMap( %s : off: %ld ) - NOTICE: Var Prim %d of %d not loaded - Value of offset seq is %ld",fileName,lNextOffset,k,primOffsets.numPrimIndex, m_sOffsetPrims.f1);	
								LogFile( ERROR_LOG, "SH2_MapLoader::LoadMap( %s : off: %ld ) - NOTICE: Var Prim %d of %d not loaded - Value of offset seq is %ld",fileName,lNextOffset,k,primOffsets.numPrimIndex, m_sOffsetPrims.f1);	
							}
							else
							{
								//loadedSections.push_back( m_sOffsetPrims.f1 );
							primStatic[ lcurStatic ] = tPrimStatic;
							primStatic[ lcurStatic ].m_lSegmentID = m_sOffsetPrims.f1;
							if( m_sOffsetPrims.f1 )
								m_bHasMultiView = true;
							++lcurStatic;
							}
						}
						else
						{
							LogFile(  TEST_LOG, "SH2_MapLoader::LoadMap( %s : off: %ld ) - ERROR: Static Prim Loader failed for prim %d of %d with size of %ld projected bytes",fileName,lNextOffset,k,primOffsets.numPrimIndex, lPrimSize);	
							LogFile( ERROR_LOG, "SH2_MapLoader::LoadMap( %s : off: %ld ) - ERROR: Static Prim Loader failed for prim %d of %d with size of %ld projected bytes",fileName,lNextOffset,k,primOffsets.numPrimIndex, lPrimSize);	
						}
					}
					else
					{
						LogFile(  TEST_LOG, "SH2_MapLoader::LoadMap( %s : off: %ld ) - ERROR: sh2_map_prim_base_header for prim %d of %d does not appear to have correct format  -  Exiting",fileName,lNextOffset,k,primOffsets.numPrimIndex);	
						LogFile( ERROR_LOG, "SH2_MapLoader::LoadMap( %s : off: %ld ) - ERROR: sh2_map_prim_base_header for prim %d of %d does not appear to have correct format  -  Exiting",fileName,lNextOffset,k,primOffsets.numPrimIndex);	
						debugMPBH( &tempPBHeader, ftell(inFile) );
					}
				}	//END FOR LOOP
			}
			else
				LogFile( ERROR_LOG, "SH2_MapLoader::LoadMap( %s : off: %ld ) - WARNING: Main primitive offset is invalid",fileName,lNextOffset);

			++i;

		}while(	(( lpNextPrimOffset[ i ] > lpNextPrimOffset[ i - 1 ] )
				|| (lpNextPrimOffset[ i ] == 0 && i == 1 ) )
			  && i < 10 );

		if( !alreadyDone )
			loadedSections.push_back( m_sOffsetPrims.f1 );

		//--[ TEST: Terminating Condition ]--/
		if( debugMode )
			LogFile( ERROR_LOG, "TEST: The terminating condition was on\n\tlpNextPrimOffset[ %d ] = %ld and lpNextPrimOffset[ %d ] = %ld\n\ti = %ld",i-1,lpNextPrimOffset[ i-1 ], i, lpNextPrimOffset[ i ], i);
	}

	//---[ Copy Over Static Primitives ]---/
	SAFEDELETE( m_pcStaticPrim );

	m_pcStaticPrim = new SH2_MapPrimitiveStatic[ lcurStatic ];
	m_lNumStaticPrim = lcurStatic;

	for( k = 0; k < lcurStatic; k++ )
		m_pcStaticPrim[ k ] = primStatic[ k ];

	SAFEDELETE( primStatic );


	//---[ Copy Over Variable Primitives ]---/
	SAFEDELETE( m_pcVarPrim );

	m_pcVarPrim = new SH2_MapPrimitiveVar[ lcurVar ];
	m_lNumVarPrim = lcurVar;

	for( k = 0; k < lcurVar; k++ )
		m_pcVarPrim[ k ] = primVar[ k ];

	SAFEDELETE( primVar );

	fclose( inFile );

	if( debugMode )
		LogFile( ERROR_LOG, "SH2_MapLoader::LoadMap( ) -------------- End --------------");

	return lcurStatic + lcurVar;
}



/*-----------------------------------------------------------------------------*/
/* SH2_MapLoader::LoadTextureData                                              */
/*   Loads texture data, saving it in the member variables for the class       */
/*-----------------------------------------------------------------------------*/
int SH2_MapLoader::LoadTextureData( FILE *inFile, long texDataSize )
{
	sh2_tex_base_header		m_sTexBaseHeader;
	sh2_tex_batch_header	m_sTexBatchHeader;
	sh2_tex_header			*m_psTexHeader = NULL;		//Number of sh2_tex_header is in sh2_tex_batch_header
	sh2_tex_data			sTexData;
	long lcurOffset;
	long lnextOffset;
	long lRes;
	long lTotalRead = 0;
	long k;
	unsigned char *pixels = NULL;
	char texStr[128];

	if( debugMode )
		LogFile( ERROR_LOG, "SH2_MapLoader::LoadTextureData( ) -------------- Start -------------- (total size: %ld)", texDataSize);

	//---[ S T A R T   T E X T U R E   L O A D,   C O M P L E T E   W H E N   M A X   B Y T E S   R E A D ]---/

	if( ( lRes = fread( (void *)&m_sTexBaseHeader, 1, sizeof( sh2_tex_base_header ), inFile ) ) < sizeof( sh2_tex_base_header ))
	{
		LogFile( ERROR_LOG, "SH2_MapLoader::LoadTextureData( ) off: %ld  read: %ld - ERROR: Couldn't read sh2_tex_base_header (%d of %d bytes read)  -  Exiting\n\t\tREASON: %s",ftell( inFile ), lTotalRead, lRes,sizeof(sh2_tex_base_header),strerror(errno));
		return 0;
	}

	if( debugMode )
		debugTB( &m_sTexBaseHeader, ftell(inFile) );

	lTotalRead += lRes;


	while( lTotalRead < texDataSize && texDataSize - lTotalRead > sizeof(sh2_tex_batch_header) + sizeof(sh2_tex_header))
	{

		//---[ R E A D   I N   B A T C H   H E A D E R ]---/

		if( ( lRes = fread( (void *)&m_sTexBatchHeader, 1, sizeof( sh2_tex_batch_header ), inFile ) ) < sizeof( sh2_tex_batch_header ))
		{
			LogFile( ERROR_LOG, "SH2_MapLoader::LoadTextureData( ) off: %ld  read: %ld - ERROR: Couldn't read sh2_tex_batch_header (%d of %d bytes read)  -  Exiting\n\t\tREASON: %s",ftell( inFile ), lTotalRead, lRes,sizeof(sh2_tex_batch_header),strerror(errno));
			fclose( inFile );
			return 0;
		}

		if( debugMode )
			debugTBH( &m_sTexBatchHeader , ftell(inFile));

		lTotalRead += lRes;


		//---[ R E A D   I N   A L L   T E X T U R E   H E A D E R S ]---/
LogFile( ERROR_LOG, "TEST: lTotalRead = %ld",lTotalRead);
		SAFEDELETE( m_psTexHeader );
LogFile( ERROR_LOG, "TEST: lTotalRead = %ld",lTotalRead);
		m_psTexHeader = new sh2_tex_header[ m_sTexBatchHeader.numTexHeaders ];

		for( k = 0; k < m_sTexBatchHeader.numTexHeaders; k++ )
		{
			if( ( lRes = fread( (void *)&m_psTexHeader[ k ], 1, sizeof( sh2_tex_header ), inFile ) ) < sizeof( sh2_tex_header ))
			{
				LogFile( ERROR_LOG, "SH2_MapLoader::LoadTextureData( ) off: %ld  read: %ld - ERROR: Couldn't read sh2_tex_header #%d of %d (%d of %d bytes read)  -  Exiting\n\t\tREASON: %s",ftell( inFile ), lTotalRead, k+1,m_sTexBatchHeader.numTexHeaders, lRes,sizeof(sh2_tex_header),strerror(errno));
				fclose( inFile );
				return 0;
			}

			if( debugMode )
				debugTH( &m_psTexHeader[ k ], ftell(inFile) );

			lTotalRead += lRes;
		}

		//---[ C R E A T E   sh2_tex_data   C L A S S   D A T A ]---/

		sTexData.numIDs = m_sTexBatchHeader.numTexHeaders - 1;
		sTexData.texID = 0;
		memcpy( (void *)&sTexData.batchHeader, (void *)&m_sTexBatchHeader, sizeof( sh2_tex_batch_header ));
		memcpy( (void *)&sTexData.fullHeader, (void *)&m_psTexHeader[ m_sTexBatchHeader.numTexHeaders - 1 ], sizeof( sh2_tex_header ));
		SAFEDELETE( sTexData.plTexIDs );
		sTexData.plTexIDs = new long[ sTexData.numIDs ];

		for( k = 0; k < sTexData.numIDs; k ++ )
			sTexData.plTexIDs[ k ] = m_psTexHeader[ k ].texID;

		//---[ R E A D   I N   T H E   C O M P R E S S E D   P I X E L   D A T A ]---/

		SAFEDELETE( pixels );
		pixels = new unsigned char[ sTexData.fullHeader.offsetNextTexHeader ];
		lcurOffset = ftell( inFile );
		lnextOffset = lcurOffset + sTexData.fullHeader.offsetNextTexHeader;
LogFile( ERROR_LOG, "TEST: The values of the arguments are: pixels: %ld\toffsetNextTexHeader: %ld\tinFile:%ld",pixels,sTexData.fullHeader.offsetNextTexHeader, inFile ); 

		if( ( lRes = fread( (void *)pixels, 1,sTexData.fullHeader.offsetNextTexHeader, inFile ) ) < sTexData.fullHeader.offsetNextTexHeader)
		{
			LogFile( ERROR_LOG, "SH2_MapLoader::LoadTextureData( ) off[ start:%ld  cur: ] total tex read: %ld - ERROR: Couldn't read pixel data (%d of %d bytes read)  -  Exiting\n\t\tREASON: %s",lcurOffset, lTotalRead, lRes,sTexData.fullHeader.offsetNextTexHeader, strerror(errno));
			SAFEDELETE( pixels );
			SAFEDELETE( sTexData.plTexIDs );
			SAFEDELETE( m_psTexHeader );
			fclose( inFile );
			return 0;
		}

		lTotalRead += lRes;
LogFile( ERROR_LOG, "TEST: lTotalRead = %ld",lTotalRead);
		//---[ Sanity Check ]---/
		if( lnextOffset != ftell( inFile ))
			LogFile( ERROR_LOG, "SH2_MapLoader::LoadTextureData( ) off: %ld read: %ld - WARNING: After reading pixels, the file offset doesn't match what was expected:  %ld act  %ld exp",ftell(inFile), lnextOffset);

		if( useOpenGL )
		{
			GLint iFormat;
LogFile( ERROR_LOG, "TEST: Inside openGL - lTotalRead = %ld",lTotalRead);
			glGenTextures( 1, &sTexData.texID );
			if( sTexData.texID < 1 )
				LogFile(ERROR_LOG,"SH2_MapLoader::LoadTextureData( ) - ERROR: Couldn't generate more textures!!!!! glGenTextures returned %d as the tex ID",sTexData.texID);
			else
			{
LogFile( ERROR_LOG, "TEST: compFormat: %d   lTotalRead = %ld",sTexData.fullHeader.compFormat,lTotalRead);
				switch( sTexData.fullHeader.compFormat )
				{
					case 0:
					case 1:	iFormat = GL_COMPRESSED_RGB_S3TC_DXT1_EXT;
							break;
					//case 2:	iFormat = GL_COMPRESSED_RGBA_S3TC_DXT1_EXT;
					//		break;
					case 2:
					case 3:	iFormat = GL_COMPRESSED_RGBA_S3TC_DXT3_EXT;
							break;
					case 4: 
					case 5:	iFormat = GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
							break;
					default:
						LogFile( ERROR_LOG, "SH2_MapLoader::LoadTextureData( ) - ERROR: Undetermined compression format: %d",sTexData.fullHeader.compFormat );
						iFormat = GL_COMPRESSED_RGB_S3TC_DXT1_EXT;
				}
LogFile( ERROR_LOG, "TEST: Inside openGL - texID = %d",sTexData.texID);
				glBindTexture ( GL_TEXTURE_2D, sTexData.texID );
LogFile( ERROR_LOG, "TEST: Inside openGL - format = %d",sTexData.fullHeader.compFormat);
LogFile( ERROR_LOG, "TEST: Values: format: 0x%04x  width: %d  height: %d  size = %ld  pixels: 0x%08x",iFormat,sTexData.fullHeader.width, sTexData.fullHeader.height,sTexData.fullHeader.offsetNextTexHeader, pixels);
				glCompressedTexImage2DARB( GL_TEXTURE_2D, 0, iFormat, sTexData.fullHeader.width, sTexData.fullHeader.height, 0, sTexData.fullHeader.offsetNextTexHeader, pixels );
LogFile( ERROR_LOG, "TEST - PASSED COMPRESSED TEX CALL");
				checkGLerror(__LINE__,__FILE__,"SH2_MapLoader::LoadTextureData( ) - Compressed Tex Creation");
				glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
				glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );

				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
				glTexEnvi ( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE,GL_REPLACE);
			}
		}
		else
		{
			LogFile( ERROR_LOG, "SH2_MapLoader::LoadTextureData( ) - NOTICE::\n----------------------------------\n\tDIRECT X  IS  NOT  IMPLEMENTED\n----------------------------------");
		}

		//k = m_sFilename.find_last_of('\\');

		//sprintf( texStr, "%s_%ld",( k > 0 )?( m_sFilename.substr( k )).c_str():m_sFilename.c_str(), m_vTexData.size());
		sprintf( texStr, "%ld",sTexData.batchHeader.texSetID);
LogFile( ERROR_LOG, "TEST - PASSED Here?");
		m_vTexData.push_back( sTexData );
LogFile( ERROR_LOG, "TEST - PASSED Or Here? - Total read = %ld",lTotalRead);
		textureMgr.AddTex(string(texStr), sTexData.texID, 0);
LogFile( ERROR_LOG, "TEST - PASSED Or Here? 2 - Total read = %ld",lTotalRead);
	}


	if( debugMode )
		LogFile( ERROR_LOG, "SH2_MapLoader::LoadTextureData( ) -------------- End --------------");

	SAFEDELETE( pixels );
	SAFEDELETE( sTexData.plTexIDs );
	SAFEDELETE( m_psTexHeader );

	fclose( inFile );

	return lTotalRead;

}


/*-----------------------------------------------------------------------------*/
/* SH2_MapLoader::LoadPrimOffsetIndex                                          */
/*   This function will load the variable length primitive offsets             */
/*-----------------------------------------------------------------------------*/
int	SH2_MapLoader::LoadPrimOffsetIndex( FILE *inFile, sh2_map_offset_index *pOffIndex )
{
	unsigned long	lRes;

	if( pOffIndex == NULL )
	{
		LogFile(  TEST_LOG, "SH2_MapLoader::LoadPrimOffsetIndex( ) - ERROR: No structure to hold offset indicies was passed in");
		LogFile( ERROR_LOG, "SH2_MapLoader::LoadPrimOffsetIndex( ) - ERROR: No structure to hold offset indicies was passed in");
		return 0;
	}

	pOffIndex->lPreOffset = ftell( inFile );

	if( ( lRes = fread( (void *)&(pOffIndex->numPrimIndex), 1, sizeof( long ), inFile ) ) < sizeof( long ) )
	{
		LogFile(  TEST_LOG, "SH2_MapLoader::LoadPrimOffsetIndex( ) - ERROR: Couldn't read the count of offsets (%d of %d bytes read)",lRes,sizeof(long));
		LogFile( ERROR_LOG, "SH2_MapLoader::LoadPrimOffsetIndex( ) - ERROR: Couldn't read the count of offsets (%d of %d bytes read)",lRes,sizeof(long));
		return 0;
	}

	pOffIndex->pOffsets = new long[ pOffIndex->numPrimIndex ];

	if( ( lRes = fread( (void *)pOffIndex->pOffsets, 1, sizeof( long ) * pOffIndex->numPrimIndex, inFile ) ) < sizeof( long ) * pOffIndex->numPrimIndex )
	{
		LogFile(  TEST_LOG, "SH2_MapLoader::LoadPrimOffsetIndex( ) - ERROR: Couldn't read the primitive offsets (%d of %d bytes read)",lRes,sizeof(long) * pOffIndex->numPrimIndex );
		LogFile( ERROR_LOG, "SH2_MapLoader::LoadPrimOffsetIndex( ) - ERROR: Couldn't read the primitive offsets (%d of %d bytes read)",lRes,sizeof(long) * pOffIndex->numPrimIndex );
		return 0;
	}

	return pOffIndex->numPrimIndex;
}


/*-----------------------------------------------------------------------------*/
/* SH2_MapLoader::RenderMap                                                    */
/*   Renders the map by calling the render functions for the primitives        */
/*-----------------------------------------------------------------------------*/
void SH2_MapLoader::RenderMap( )
{
	bool origDebugMode = debugMode;
	long k;
	unsigned short int startInd = 0;

	if( !m_bFirstTime )
		debugMode = false;

	if( m_bFirstTime && debugMode )
	{
		LogFile(ERROR_LOG,"+++++++++++++++++++++++++++++++++++++++++++ RENDER START +++++++++++++++++++++++++++++++++++++");
		LogFile( ERROR_LOG, "SH2_MapLoader::RenderMap() - Values and count of arrays:\n\tStatic: #%ld\tVal: %ld\n\tVar   : #%ld\tVal: %ld",m_lNumStaticPrim,m_pcStaticPrim, m_lNumVarPrim,m_pcVarPrim);
	}

	glEnable(GL_TEXTURE_2D);

	if( m_bFirstTime && dumpModel )
	{
		char baseFilename[ 256 ];
		if( !baseName( const_cast<char *>( m_sFilename.c_str( ) ), baseFilename ) )
		{
			LogFile( ERROR_LOG, "SH_MapLoader::RenderMap( ) - ERROR: Couldn't get basename for filename");
			strcpy( baseFilename, m_sFilename.c_str( ) );
		}

		if( ! OBJ_Dump.CreateFiles( baseFilename ) )
		{
			LogFile( ERROR_LOG, "SH2_MapLoader::RenderMap( ) - ERROR: Cannot open files for dump");
			dumpModel = false;
		}
	}

//	if( onlyVar )
	{
		for( k = 0; k < m_lNumVarPrim; k ++ )
		{
			startInd = 0;

			if( m_bFirstTime && debugMode )
				LogFile( ERROR_LOG, "SH2_MapLoader::RenderMap() - Var Prim #%d\n\t#Verts: %ld  Verts: %ld\n\t#SimpVerts: %ld  SimpVerts: %ld\n\tIndicies: %ld\n\t",k,m_pcVarPrim[ k ].m_lNumVerts,m_pcVarPrim[ k ].m_psVerts,m_pcVarPrim[ k ].m_lNumSimpleVerts,m_pcVarPrim[ k ].m_psSimpleVerts, m_pcVarPrim[ k ].m_pusIndicies );


			m_pcVarPrim[ k ].RenderPrimitive( m_psUnknownData );

			glDisable(GL_ALPHA_TEST);
			glDisable(GL_BLEND);
		//	glFlush();
		}
	}

	//glFlush();
	
//	if( onlyStatic )
	{
		for( k = 0; k < m_lNumStaticPrim; k ++ )
		{
			if( m_bFirstTime && debugMode )
				LogFile( ERROR_LOG, "SH2_MapLoader::RenderMap() - Static Prim #%d of %ld\n\tVerts: %ld\n\tIndicies: %ld\n\tTexVal? %ld",k,m_lNumStaticPrim,m_pcStaticPrim[ k ].m_psVerts, m_pcStaticPrim[ k ].m_pusIndicies, m_pcStaticPrim[ k ].m_sStaticTexRanges );

			m_pcStaticPrim[ k ].RenderPrimitive( m_psUnknownData );
			glDisable(GL_ALPHA_TEST);
		//	glFlush();

		}
	}

	glFlush();

	if( m_bFirstTime && dumpModel )
	{
		OBJ_Dump.CloseFiles( );
	}

	if(m_bFirstTime && debugMode )
		LogFile(ERROR_LOG,"+++++++++++++++++++++++++++++++++++++++++++ RENDER COMPLETE +++++++++++++++++++++++++++++++++++++");	
	
	glDisable(GL_TEXTURE_2D);

	debugMode = origDebugMode;
	m_bFirstTime = false;
}
