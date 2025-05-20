#ifndef __SH2_LOADER_H__
#define __SH2_LOADER_H__


#include <windows.h>
#include <gl\gl.h>
#include <gl\glu.h>
#include <gl\wglext.h>
#include <gl\glext.h>
#include <gl\glprocs.h>
#include <stdio.h>
#include <vector>

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

using namespace std;

//----===[ V E R T E X   T Y P E S ]===----/

typedef struct
{
	vertex vert;
	vertex norm;
	texcoord tc;
}sh2_map_vertex;

typedef struct
{
	vertex vert;
	texcoord tc;
}sh2_vertex;

typedef struct
{
	vertex vert;
	vertex norm;
	rgba color;
	texcoord tc;
}sh2_color_vertex;

typedef struct
{
	vertex vert;
	rgba color;
	texcoord tc;
}sh2_shader_vertex;

//----===[ M A P   F I L E   H E A D E R   T Y P E S ]===----/

typedef struct
{
	long	mapFileID;				//Seems to always be 0x20010510
	long	mapFileSize;			//Size of whole file, in bytes
	long	q1;
	long	f1;
	long	q2;
	long	texDataSize;			//Adding this to offset after reading this struct will go to main primitive header
	long	f2;
	long	f3;
}sh2_map_file_header;


//---===[ M O D E L   F I L E   H E A D E R S ]===---/

//--[ Struct for the initial data for a model file.  This is the start of the model file. ]--/
typedef struct
{
	long	f0_mbh;
	long	modelID;
	long	numTex;
	long	offsetTex;
	long	f1_mbh;
	long	headerSize;
	long	f2_mbh;
	long	f3_mbh;
	long	f4_mbh;
	long	f5_mbh;
	long	f6_mbh;
	long	f7_mbh;
	long	f8_mbh;
	long	f9_mbh;
	long	f10_mbh;
	long	f11_mbh;
}sh2_model_base_header;


//----===[ T E X T U R E   H E A D E R   T Y P E S ]===----/


	//--=[ Tex Batch Header ]=--/
typedef struct
{
	long	texStructMarker;		//Seems to always be 0x19990901
	long	f1;
	long	f2;
	long	q0;						//Seen 1,
}sh2_tex_base_header;

typedef struct
{
	long	texSetID;				//?Relates to values at end of file
	unsigned short int	q1;
	unsigned short int	q2;
	unsigned short int	q3;
	unsigned short int	q4;
	long	numTexHeaders;			//Count of tex_header
	unsigned short int	q5;
	unsigned short int	q6;
	long	f3;
	long	f4;
	long	f5;
}sh2_tex_batch_header;


typedef struct
{
	long	texID;				//ID for this texture
	long	f1;
	unsigned short int	width;	//Width
	unsigned short int	height;	//Height
	unsigned char	compFormat;	//?Compression format
	unsigned char	compAlt;	//?Alternate compression maybe?
	short int	q1_tdh;
	long	offsetNextTexHeader;//Offset to next sh2_tex_header, from the
	long	offsetNextOffset;	//This offset will go to the next sh2_tex_header.offsetNextOffset (which points to another sh2_tex_header, of course)
	long	f3;
	long	texHeaderMarker;	//Seems to always be 0x00000099
}sh2_tex_header;


//===---===[ T E X T U R E   M A N A G E M E N T   C L A S S E S ]===---===/

class sh2_tex_data
{
public:
	sh2_tex_data( ){ plTexIDs = NULL; psQ_1_td = NULL; texID = 0; numIDs = 0; inMem = false; }
	sh2_tex_data( const sh2_tex_data & rhs ){ plTexIDs = NULL; psQ_1_td = NULL; operator=( rhs ); }
	~sh2_tex_data( ){ /*LogFile( ERROR_LOG, "sh2_tex_data - Check: plTexIDs = 0x%08x\tpsQ_1_td = 0x%08x",plTexIDs,psQ_1_td);*/SAFEDELETE( plTexIDs ); SAFEDELETE( psQ_1_td ); }
	sh2_tex_data & operator=( const sh2_tex_data & rhs )
	{
		if( &rhs != this )
		{
			SAFEDELETE( plTexIDs );
			SAFEDELETE( psQ_1_td );
			numIDs = rhs.numIDs;
			texID = rhs.texID;
			if( numIDs )
			{
				plTexIDs = new long[ numIDs ];
				psQ_1_td = new short[ numIDs ];
			}
			else
			{
				plTexIDs = NULL;
				psQ_1_td = NULL;
			}
			memcpy( (void *)plTexIDs, (void *)rhs.plTexIDs, sizeof(long) * rhs.numIDs);
			memcpy( (void *)psQ_1_td, (void *)rhs.psQ_1_td, sizeof(short) * rhs.numIDs );
			memcpy( (void *)&batchHeader, (void *)&rhs.batchHeader, sizeof( sh2_tex_batch_header ));
			memcpy( (void *)&fullHeader, (void *)&rhs.fullHeader, sizeof( sh2_tex_header ));
			texOffset = rhs.texOffset;
			texFile = rhs.texFile;
			inMem = rhs.inMem;
			texName = rhs.texName;
		}
		return *this;
	}

	bool isResident( );

	long			*plTexIDs;	//IDs for each sh2_tex_header
	short			*psQ_1_td;	//Unknown data
	long			numIDs;		//Number of IDs = sh2_tex_batch_header.numTexHeaders - 1
	sh2_tex_batch_header batchHeader;	//Batch header for each texture
	sh2_tex_header	fullHeader;	//Last sh2_tex_header is the only full one
	string			texFile;	//Mapfile with the texture in it.
	long			texOffset;	//Offset in file for texture;
	GLuint			texID;		//openGL texture handle
	bool			inMem;		//Is the texture loaded right now?
	string			texName;	//String representation of the texture name
};


class sh2_tex_index
{
public:
	sh2_tex_index( ){ m_uiBadTex = 0; }
	sh2_tex_index( const sh2_tex_index & rhs ){ ReleaseAllTex( ); operator=( rhs ); }
	~sh2_tex_index( ){ ReleaseAllTex( ); }	//int k; for( k = 0; k < m_vTexList.size(); k++ ) DeleteTex( m_vTexList[k].texName );}
	sh2_tex_index & operator=( const sh2_tex_index & rhs )
	{
		if( &rhs != this )
		{
			m_vTexList = rhs.m_vTexList;
			m_uiBadTex = rhs.m_uiBadTex;
		}
		return *this;
	}

	long CountTexInMem( 
		bool _bCheckResident = false );			//Counts number of textures currently in memory
	void DeleteTex( string & name );			//Removes a texture from memory
	void DeleteAllTex( );						//Deletes all textures from memory, but does not affect the metadata on the textures
	void ReleaseAllTex( );						//Deletes all textures from memory, as well as deletes the data loaded for the textures
	GLuint GetTex( string & name, 				//Returns the texture ID for it's openGL handle
				bool alpha = false );			//If alpha is set to true, it will turn on alpha if the tex is RGBA
	GLuint LoadTex( sh2_tex_data *pTD );		//Loads the texture by name from it's map file
	int LoadFileInfo( char *mapName );			//Loads a single map file, and reads the texture info, saving in the texList
	int LoadModelFileInfo( char *modelName );	//Loads a single model file, and reads the texture info, saving in the texList
	int LoadFileInfoAux( FILE *inFile,char *mapName, 
		long _lTexDataSize );		//Aux to Load*FileInfo functions.  Since the only part that differs is the beginning..
	int AddToTexList( sh2_tex_data *pTD );		//Adds the texture to the texlist in ascending order
	void BuildTexList( char *pDirName );		//Loads all files in the directory, and subdirectory, and reads in the texture info
	GLuint	GetBadTex( );						//Returns an ID for the "Bad Texture" - A texture that doesn't exist

	vector< sh2_tex_data >	m_vTexList;
	GLuint					m_uiBadTex;
};


//----===[ S C E N E   H E A D E R   S T R U C T U R E S ]===----/

//NOTE: All offsets vary in terms of what they include, and what they don't... It's weird.  See each's comments

#ifdef  __SH2_START_OFFSETS__
long	startOffset;	//**** SPECIAL VALUE ****
#endif/*__SH2_START_OFFSETS__*/

#ifdef  __SH2_START_OFFSETS__
#define SIZE_SH2_MBH	(sizeof(sh2_map_base_header) - sizeof(long))
#else
#define SIZE_SH2_MBH	(sizeof(sh2_map_base_header))
#endif/*__SH2_START_OFFSETS__*/

#ifdef  __SH2_START_OFFSETS__
#define SIZE_SH2_MOU	(sizeof(sh2_map_offset_unknown) - sizeof(long))
#else
#define SIZE_SH2_MOU	(sizeof(sh2_map_offset_unknown))
#endif/*__SH2_START_OFFSETS__*/

#ifdef  __SH2_START_OFFSETS__
#define SIZE_SH2_MOP	(sizeof(sh2_map_offset_prims) - sizeof(long))
#else
#define SIZE_SH2_MOP	(sizeof(sh2_map_offset_prims))
#endif/*__SH2_START_OFFSETS__*/

#ifdef  __SH2_START_OFFSETS__
#define SIZE_SH2_MOI	(sizeof(sh2_map_offset_index) - sizeof(long))
#else
#define SIZE_SH2_MOI	(sizeof(sh2_map_offset_index))
#endif/*__SH2_START_OFFSETS__*/


//---==[ MAIN SCENE HEADER ]==---/

//#define READ_SET_ADDRESS( d, s, f ) ( (d).startOffset=ftell((f)),if(fread((void*)&(d),1,sizeof((s)),(f)) < sizeof((s))){})


	//--=[ Base Header ]=--/
typedef struct
{
	long	mainSceneID;		//? Could be - it's a 1, with an offset...
	long	sizeAllPrimData;	//? Size of primitive data and unknown data, and headers (add this to texDataSize and map_file_header size, and it's = size of file)
	long	f1;
	long	f2;
}sh2_map_base_header;



	//--=[ Base Offsets Headers ]=--/

//NOTE: THis could be a variable header... Need more analysis
typedef struct
{
	long	mainSceneMarker;	//Seems to always be 0x20010730
	long	numMapOffsetPrims;	//Number of sh2_map_offset_prims, where the current sizeAllPrims goes to the next one
	long	offsetUnknownData;	//Offset from start of this struct to the unknow data at the end of the file
	long	numUnknowDataSets;	//Number of unknown data sets at end of file
	//long	f1;
}sh2_map_offset_unknown;


typedef struct
{
	long	f1;			//NOTE: I'm sticking this in here, cuz the offsets will lead to the start of each sh2_map_offset_index
	long	sizeAllPrims;		// Size of primitive data, from start of this struct (excludes unknown data)
	long	offsetMainPrim;		//Offset to main primitive for map
	long	offsetSubPrim;		//Offset to sub primitive for map
	long	offsetMultiPrim;	//Offset to multiple small primitives for map
}sh2_map_offset_prims;


typedef struct
{
	long	lPreOffset;			//Offset for which all pOffsets are relative to
	long	numPrimIndex;		//Number of primitive indicies in this struct
	long	*pOffsets;			//Struct must be read in specially. Offset is from start of this struct
}sh2_map_offset_index;


	//--=[ Primitive Base Header ]=--/
typedef struct
{
	vertex4f	minExtent;		//Bounding box around all verticies
	vertex4f	maxExtent;
	long		headerSize;		//Size of header:  I've seen 64,72, and 132
	long		offsetIndex;	//Offset to index data, from the start of this structure
	long		sizeIndexData;	//Size of the index array data.
}sh2_map_prim_base_header;



//sh2_map_prim_variable_header;

//********************************************************/
//* NEW STRUCTURES FOR SH2 - BASED ON NEW ANALYSIS       */
//********************************************************/

//--[ THESE ARE USED FOR THE FIRST TWO PRIMITIVE SETS FROM THE OFFSET DATA ]--/

typedef struct
{
	long		texNum;					//texture number at end of file
	long		vertexSet;				//Which set from vertex size struct this primitive uses
	long		numTriStripRanges;		//How many sh2_map_tri_range structs are under this header
}sh2_map_tri_header;

typedef struct
{
	unsigned short int	numIndicies;	//Number of indices this range uses
	unsigned char		q1_mtr;
	unsigned char		primType;		//Look out for values other than 256******************
	unsigned short int	startVertNum;	//Minimum Vertex for the range
	unsigned short int	endVertNum;		//Maximum Vertex for the range
}sh2_map_tri_range;

typedef struct
{
	sh2_map_tri_header		triHeader;
	sh2_map_tri_range		*psRanges;
}sh2_map_tristrip;


//--[ THE THIRD SET HASN'T CHANGED MUCH FROM THE STATIC STRUCTS ]--/

	//--=[ Static Primitive Header ]=--/
typedef struct
{
	long		texNum;
	long		vertexSet;
	unsigned short int	primType;		//Primitive type: 1 - TriStrip, 2 - ??, 3 - ??, 4 - Quad List
	unsigned short int	q1_mqr;			//Look out for non-zero values *************
	long		numPrims;				//Look out for values where data would be too large*************
}sh2_map_quads_range;


	//--=[ Vertex Size Header ]=--/
typedef struct
{
	long		sizeVertexData;			//Size of all of the vertex data for this primitive
	long		numVertexSizes;			//Number of vertex sets
}sh2_vertex_size_header;

typedef struct
{
	long		offsetToVerts;			//Offset from end of size structs to vertex data
	long		sizeVertex;				//Size of a single vertex (num verts = sizeVertexData / sizeVertex)
	long		sizeVertexData;			//2nd Size of all of the vertex data for this primitive (same as first size in this struct)
}sh2_vertex_size_data;

class sh2_map_vertex_sizes
{
public:
	sh2_map_vertex_sizes( ){ m_psSizes = NULL; }
	sh2_map_vertex_sizes( const sh2_map_vertex_sizes & rhs ){ operator=( rhs ); }
	~sh2_map_vertex_sizes( ){ SAFEDELETE( m_psSizes ); }
	sh2_map_vertex_sizes & operator=( const sh2_map_vertex_sizes & rhs );

	int LoadVertexSizes( FILE *inFile );
	int GetCount( ){ return m_sSizeHeader.numVertexSizes; }
	sh2_vertex_size_header	m_sSizeHeader;
	sh2_vertex_size_data	*m_psSizes;
};

//***************************************************************/
//* END OF NEW STRUCT TYPES - WHETHEr I USE THEM IS NOT KNOWN   */
//***************************************************************/

	//--=[ Variaable Tex Ranges ]=--/
class sh2_map_prim_variable_range
{
public:
	sh2_map_prim_variable_range( ){ memset((void *)this,0, sizeof( sh2_map_prim_variable_range )); }
	sh2_map_prim_variable_range( const sh2_map_prim_variable_range & rhs ){ operator=( rhs ); }
	~sh2_map_prim_variable_range( ){ SAFEDELETE( ranges ); }
	sh2_map_prim_variable_range & operator=( const sh2_map_prim_variable_range & rhs );
	
	long	LoadTexRange( FILE *inFile );

	long		texNum;					//? I think this is the texture number
	long		vertexSet;
	long		numTriRanges;			//Type of primitive (usually 1 for these)
	sh2_map_tri_range *ranges;

	//unsigned short int	numIndicies;	//Number of indices this range uses
	//unsigned short int	q1;				//Usually 256. 
	//unsigned short int	startVertNum;	//Minimum Vertex for the range
	//unsigned short int	endVertNum;		//Maximum Vertex for the range
};

/*
########################################################################
NOTES: Move sizeIndexData into base prim header, for variable, read sizePrimData var seperately, and for
		   both, read num tex range.  Leave variable range as it is, and create one for static.  Modify
		   the load funcions, the map render function, and the debug functions
#####################################################################################
*/
	//--=[ Static Primitive Header ]=--/
typedef struct
{
	long		texNum;
	long		vertexSet;
	unsigned short int	primType;		//Primitive type: 1 - TriStrip, 2 - ??, 3 - ??, 4 - Quad List
	unsigned short int	q2;				//Usually 0 for the static primitive header
	long		numPrims;				//Number of prims
}sh2_map_prim_static_ranges;


	//--=[ Vertex Size Header ]=--/
typedef struct
{
	long		sizeVertexData;			//Size of all of the vertex data for this primitive
	long		numVertexSizes;			//Number of vertex sets
	long		offsetToVerts;			//Offset from end of this struct to vertex data
	long		sizeVertex;				//Size of a single vertex (num verts = sizeVertexData / sizeVertex)
	long		altSizeVertexData;		//2nd Size of all of the vertex data for this primitive (same as first size in this struct)
}sh2_map_simple_vertex_size;


	//--=[ Vertex Size Header ]=--/
typedef struct
{
	long		sizeVertexData;			//Size of all of the vertex data for this primitive
	long		numVertexSizes;
	long		offsetToVerts1;
	long		sizeVertex1;			//Size of a single vertex (num verts = sizeVertexData / sizeVertex)
	long		sizeSet1VertexData;		//2nd Size of all of the vertex data for this primitive (same as first size in this struct)
	long		offsetToVerts2;
	long		sizeVertex2;			//Size of a single vertex (num verts = sizeVertexData / sizeVertex)
	long		sizeSet2VertexData;		//2nd Size of all of the vertex data for this primitive (same as first size in this struct)
}sh2_map_complex_vertex_size;

//*************************************************************************/
//*  This structure seems to relate to the batch tex struct at the start  */
//*  of the file.  This could be referencing other texture files for the  */
//*  current scene...                                                     */
//*************************************************************************/

	//--=[ Unknown Map Data Struct ]=--/
typedef struct
{
	unsigned short int	q1;				//Usually 1
	unsigned short int	id;				//This value seems to correspond to sh2_tex_batch_header.texBatchID
	unsigned char		q2;				//Seen b2
	unsigned char		q3;				//Seen b2
	unsigned char		q4;				//Seen b2
	unsigned char		q5;				//Seen ff
	long				endMarker;		//Usually 0xff000000
	long				f1;
}sh2_map_unknown_data;


class SH2_MapPrimitiveVar
{
public:
	SH2_MapPrimitiveVar( ){ memset((void *)this,0,sizeof((*this))); m_bFirstTime = true; }
	SH2_MapPrimitiveVar( const SH2_MapPrimitiveVar & rhs ){ operator=( rhs ); }
	~SH2_MapPrimitiveVar( ){ DeleteDynamicData(); }

	SH2_MapPrimitiveVar & operator=( const SH2_MapPrimitiveVar & rhs );

	void DeleteDynamicData( ){ SAFEDELETE(m_psVariableRange); SAFEDELETE(m_psVerts);SAFEDELETE( m_psSimpleVerts );SAFEDELETE( m_psColorVerts ); SAFEDELETE(m_psShaderVerts); SAFEDELETE(m_pusIndicies); }
	int  LoadPrimitive( FILE *inFile, long lPrimSize );		//lPrimSize is a check val.  If sizes or values inside LoadPrim exceed the size, you have a problem
	void RenderPrimitive( sh2_map_unknown_data *psUnknownData );

	//---[ Header Data ]---/
	sh2_map_prim_base_header		m_sBaseHeader;
	long							m_lNumTexRanges;
	sh2_map_prim_variable_range		*m_psVariableRange;		//The number of these is set in m_sVariableHeader
	sh2_map_vertex_sizes			m_sVertexSize;

	long							m_lSegmentID;

	bool							m_bFirstTime;
	//---[ Primitive Data ]---/
	long							m_lNumVerts;
	long							m_lNumSimpleVerts;
	long							m_lNumColorVerts;
	long							m_lNumShaderVerts;
	long							m_lNumIndicies;
	sh2_map_vertex					*m_psVerts;
	sh2_vertex						*m_psSimpleVerts;
	sh2_color_vertex				*m_psColorVerts;
	sh2_shader_vertex				*m_psShaderVerts;
	unsigned int					*m_pusIndicies;
};



class SH2_MapPrimitiveStatic
{
public:
	SH2_MapPrimitiveStatic( ){ memset((void *)this,0,sizeof((*this))); m_bFirstTime = true; }
	SH2_MapPrimitiveStatic( const SH2_MapPrimitiveStatic & rhs ){ operator=( rhs ); }
	~SH2_MapPrimitiveStatic( ){ DeleteDynamicData(); }

	SH2_MapPrimitiveStatic & operator=( const SH2_MapPrimitiveStatic & rhs );

	void DeleteDynamicData( ){ SAFEDELETE(m_psVerts);SAFEDELETE( m_psSimpleVerts );SAFEDELETE( m_psColorVerts ); SAFEDELETE(m_psShaderVerts); SAFEDELETE(m_pusIndicies); SAFEDELETE( m_sStaticTexRanges );}
	int  LoadPrimitive( FILE *inFile, long lPrimSize );		//lPrimSize is a check val.  If sizes or values inside LoadPrim exceed the size, you have a problem
	void RenderPrimitive( sh2_map_unknown_data *psUnknownData );

	//---[ Header Data ]---/
	sh2_map_prim_base_header		m_sBaseHeader;
	long							m_lNumTexRanges;
	sh2_map_prim_static_ranges		*m_sStaticTexRanges;
	sh2_map_vertex_sizes			m_sVertexSize;
	long							m_lSegmentID;
	bool							m_bFirstTime;

	//---[ Primitive Data ]---/
	long							m_lNumVerts;
	long							m_lNumSimpleVerts;
	long							m_lNumColorVerts;
	long							m_lNumShaderVerts;
	long							m_lNumIndicies;
	sh2_map_vertex					*m_psVerts;
	sh2_vertex						*m_psSimpleVerts;
	sh2_color_vertex				*m_psColorVerts;
	sh2_shader_vertex				*m_psShaderVerts;
	unsigned int					*m_pusIndicies;
};


class SH2_MapLoader
{
public:
	SH2_MapLoader( ){ memset( (void*)this,0, sizeof((*this))); m_bFirstTime = true;}
	SH2_MapLoader( char *fileName ){ LoadMap( fileName ); }
	~SH2_MapLoader( ){ DeleteDynamicData( ); }

	void DeleteDynamicData( ){ SAFEDELETE( m_psUnknownData ); SAFEDELETE( m_pcVarPrim ); SAFEDELETE( m_pcStaticPrim ); }
	int LoadMap( char *fileName );
	int LoadTextureData( FILE *inFile, long texDataSize );
	int	LoadPrimOffsetIndex( FILE *inFile, sh2_map_offset_index *pOffIndex );
	void RenderMap( );
	bool isSceneValid( ){ return (m_lNumStaticPrim && m_pcStaticPrim) || (m_lNumVarPrim && m_pcVarPrim ); }

	//---[ General Data ]---/
	string					m_sFilename;
	bool					m_bFirstTime;
	bool					m_bHasMultiView;

	//---[ Primary File Headers ]---/
	sh2_map_file_header		m_sFileHeader;

	//---[ Texture Data Headers ]---/
	vector<sh2_tex_data>	m_vTexData;
	
	//---[ Primitive Loc Headers ]---/
	sh2_map_base_header		m_sBaseHeader;
	sh2_map_offset_unknown	m_sOffsetUnknown;
	sh2_map_offset_prims	m_sOffsetPrims;

	//---[ Unknown Data Records ]---/
	sh2_map_unknown_data	*m_psUnknownData;	//Number of sh2_map_unknown_data is in 

	//---[ Primitive Data Records ]---/
	long					m_lNumStaticPrim;
	long					m_lNumVarPrim;
	SH2_MapPrimitiveStatic	*m_pcStaticPrim;
	SH2_MapPrimitiveVar		*m_pcVarPrim;

};

void debugMFH( sh2_map_file_header *h, long offset );
void debugTB( sh2_tex_base_header *h, long offset );
void debugTBH( sh2_tex_batch_header *h, long offset );
void debugTH( sh2_tex_header *h, long offset );
void debugTD( sh2_tex_data *h, long offset );
void debugMBH( sh2_map_base_header *h, long offset );
void debugMOU( sh2_map_offset_unknown *h, long offset );
void debugMOP( sh2_map_offset_prims *h, long offset );
void debugMOI( sh2_map_offset_index *h, long offset );
void debugMPBH( sh2_map_prim_base_header *h, long offset );
void debugMPVR( sh2_map_prim_variable_range *h, long offset );
void debugMPSH( sh2_map_prim_static_ranges *h, long offset );
void debugMVS( sh2_map_vertex_sizes *h, long offset );
void debugMVSH( sh2_vertex_size_header *h );
void debugMVSD( sh2_vertex_size_data *h );
void debugMUD( sh2_map_unknown_data *h, long offset );
void debugMTR( sh2_map_tri_range *h );
void SaveSH2Texture( const char *filename, sh2_tex_header *texHeader, BYTE *data );

#endif /*__SH2_LOADER_H__*/