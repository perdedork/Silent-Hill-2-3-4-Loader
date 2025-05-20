#ifndef __SH3LOADER__
#define __SH3LOADER__

#include <windows.h>
#include <gl\gl.h>
#include <gl\glu.h>
#include <gl\wglext.h>
#include <gl\glext.h>
#include <gl\glprocs.h>
#include <stdio.h>

#include <vector>
#include <string>

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

#include <SMD_Vertex.h>
#include <SMD_Model.h>

using std::vector;
using std::string;

//############ MOVE THIS WHERE MORE APPROPRIATE ###############
void flipHorizontal( int xDim, int yDim, BYTE *data );
void flipVertical( int xDim, int yDim, BYTE *data );


#define colorRGB  0
#define colorGRB  1
#define colorBGR  2
#define colorComp 3

#define convertRed( x ) ( ((short)(x >> 0) & (short)0x01f ) * 8 )
#define convertGreen( x ) ( ((short)(x >> 5) & (short)0x01f ) * 8 )
#define convertBlue( x ) ( ((short)(x >> 10) & (short)0x01f ) * 8 )
#define convertAlpha( x ) ( ((short)(x >> 15) & (short)0x01 ) * 255 )


//----===[ V E R T E X   T Y P E S ]===----/

typedef struct{
	vertex4f vert;
	rgbf	 color;
	vertex	 normal;
	texcoord tc;
}sh_vertex_old;

typedef struct
{
	vertex vert;
	vertex norm;
	texcoord tc;
	rgba color;
}sh_map_vertex;

typedef struct
{
	vertex vert;
	vertex v1;
	rgba   color;
	vertex normal;
	texcoord tc;
}sh_vertex;

typedef struct
{
	vertex vert;
	vertex norm;
	texcoord tc;
}sh_static_model_vertex;


//----===[ M O D E L   H E A D E R   T Y P E S ]===----/

typedef struct
{
	long	unknown;		//Usually '0'
	long	modelID;		//Seems to be model ID
	long	numTex;			//Number of textures for the model;
	long	model_size;		// Size of the whole 3d model file (without the texture)
	char	misc2[192];
	matrix m1; // Transformation matrices
	matrix m2;
	matrix m3;
	matrix m4;
	matrix m5;
	char  misc3[68];
	long  vertex_count; // Number of vertices in the vertex buffer
	long  MAYBE_vertpart_size;
	long  index_count;  // Number of indices in the index buffer
	char  misc4[112];	//Fills in space between the end of the header an the start of the verticies
} model_header;


typedef struct
{
	long	f1;				//Filler
	long	modelID;		//Model ID for character/item/etc
	long	numTex;			//Number of textures included w/ model
	long	texOffset;		//Offset of embedded textures
	long	modelBaseSize;	//Size of model w/o textures
	long	baseHeaderSize;	//Size of initial base header;
	long	f2;
	long	always1_1;		//Seems to always be 1
	long	fa3[8];
} model_base_header;


typedef struct
{
	long	headerSizeTexOffsets[4];	//This is a grouping always at end of base model header.  Its size depends on the # of textures: 1-2=1 struct,3-4=2, etc
}model_size_offset;


//NOTE: All of these offsets are relative to the start of this structure
typedef struct
{
	long	modelDataMarker;//Denotes the model data header
	long	always3_1;		//Seems to always be 3
	long	offsetMatSet1;	//Offset of the first matrix group
	long	numMatSet1;		//Number of matricies in set 1
	long	offsetMatSeq1;	//Offset of the first matrix sequence set
	long	numMatSet2;		//Number of matricies in set 2
	long	offsetMatSeq2;	//Offset of the second matrix sequence set
	long	offsetMatSet2;	//Offset of the second matrix group
	long	numVertexArrays;//Number of vertex arrays w/ a header, verts, and indicies
	long	offsetVertHead;	//Offset of the first vertex header
	long	numAltVertArrays;
	long	offsetAltVertHead;//Offset of the textures' header
	long	numTex;			//number of textures;
	//---[ I Think These have to do with textures or animation ]---/
	long	offsetTexIndex;	//Offset of texture indicies
	long	totalModelTex;	//Total count of textures used - including local ones, and ones located other places
	long	offsetTexSeq;	//Offset of data used to decide on primitive texture and texture render states
	long	qa4[2];
	long	offsetQ5;
	long	q6;
	long	offsetQ7;
	long	q8;
	long	offsetQ9;
	long	offsetQ10;
	long	offsetQ11;
	long	always1_1;		//?*?Seems to always be 3
	long	fa2[6];
}model_data_header;


typedef struct
{
	long	vertSegmentSize;//Size of header, verticies, and indicies
	long	f1;
	long	vertHeaderSize;	//Size of header
	long	q1;
	long	numIndex;		//Count of indicies
	long	numAltSeq;		//Num of alt Seq Sets (there are 3 shorts to a set)
	long	altOffsetSeq1;	//Offset of what appears to be a sequence (maybe animations?)
	long	numSeq1;		//Number of sequences in set 1
	long	offsetSeq1;		//Alternate Offset of what appears to be a sequence (maybe animations?)
	long	numSeq2;		//Number of sequences in set 2
	long	offsetSeq2;		//Offset of what appears to be a seq
	long	q2;
	long	q3;
	long	always1_1;
	long	offsetTexIndex;	//Offset of texture index for verticies
	long	offsetVertConst;//Offset of vertex header constants
	long	altHeaderSize;	//Alternate vertex header size
	long	numVerts;		//# of verticies
	long	vertSectionSize;//Size of just the vertex section
	long	altNumIndex;	//Alternate number of indicies
	long	numFloatSets;	//Num of floating point sets (I think these may be verticies, weights, or color)
	long	always1_2;
	long	fa3[2];
	float	floatSet0[4];	//used if value of numFloatSets is 4 ONLY
	float	floatSet1[4];	//used if value of numFloatSets is 1, 2, or 4
	float	floatSet2[4];	//used if value of numFloatSets is 2, or 4
	float	floatSet3[4];	//used if value of numFloatSets is 4 ONLY
}model_vertex_header;


typedef struct
{
	model_vertex_header	vertHeader;
	short	*altSeq1;
	short	*seqData1;			//First Sequence Data
	short	*seqData2;			//Second Sequence Data
	short	texNum;
	long	always8372234_1;
	long	always2044_1;
	long	always97_1;
	long	f1;
	int		vertSize;
	long	texModify;
	sh_vertex	*verts;
	sh_static_model_vertex *altVerts;
	long	*indicies;
	GLuint	texID;
}model_primitive;


class SH3_Actor
{
public:
	SH3_Actor(){ m_pmSizeOffsets = NULL;  matSet1 = NULL;  mSeq1 = NULL;  mSeq2 = NULL;  matSet2 = NULL;
				m_pmPrimitive = NULL;  texIndexData = NULL;  texSeqData = NULL;}
	~SH3_Actor(){ releaseModelData(); }

	GLuint loadTex( long offset, char *texName, int tTexIndex, FILE *infile );
	void draw( int modelPart = -1 );
	bool isModel( model_base_header *pHeader );
	int  loadModel( char *filename, int modelNum );
	int  loadModelPrimitive( long offset, model_primitive *pPrim, FILE *infile);
	void releaseModelData();
	int  getMinModel( char *filename );
	int  getMaxModel( char *filename );

	int  findAnimFile( long modelNum, char *filename );			//Returns -1 if index not found
	int  loadAnimFile( FILE *infile, long offset, long size );
	int  loadAnim( FILE *infile, long modelID, long segNum);
	int  loadAnim( char *filename, long modelID, long segNum);

	//---[ Export Functions ]---/
	void ExportSMD(  );
	void AddVerticies( SMD::SMD_Model & _Model );
	SMD::SMD_Vertex BuildSMDVertex( sh_vertex & vert, long *_pSeq );
	SMD::SMD_Vertex BuildSMDVertex( sh_static_model_vertex & vert, long _parent );


	long				baseOffset;

	model_base_header	m_mBaseHeader;
	model_size_offset	*m_pmSizeOffsets;
	model_data_header	m_mDataHeader;
	matrix				*matSet1;
	char				*mSeq1;
	char				*mSeq2;
	matrix				*matSet2;
	long				*texIndexData;
	long				*texSeqData;		//Data between end of matricies and 1st model_vertex_header
	

	long				numSeq1;
	long				numSeq2;
	long				numTexIndex;
	long				numTexSeq;

	long numPrimitives;
	model_primitive		*m_pmPrimitive;
	char				modelFilename[128];
};

//----===[ T E X T U R E   H E A D E R   T Y P E S ]===----/

//NOTE: This structure is made up of two seperate headers.  Since the texture headers always seem to be 128
//      bytes, I'm putting it together like this.
typedef struct
{
	long texBatchSegMarker;	//All segment markers are -1
	long f1;				//All f-labels are filler
	long texBatchHeaderSize;//Size of this batch header (from texBatchSegMarker to just before texHeaderSegMarker)
	long texBatchSize;		// = res * res * (bpp/8) * #tex + 128 * #tex (like in a map file after the verticies)
	long f2;
	long numBatchedTex;		//The count of batched textures
	long fa3[2];			//All fa-lables are filler arrays
	long texHeaderSegMarker;//2ndary segment marker - differentiates between subsequent tex headers an main
	long f4;
	unsigned short int width;		//Texture width
	unsigned short int height;		//Texture Height
	unsigned char bpp;				//Bits per pixel
	unsigned char endFillerSize;	//# bytes from texHeaderSize+16 to 128 filled w/ 0
	unsigned short int	f5;
	long texSize;			// = res * res * (bpp/8)
	long texHeaderSegSize;	// = texSize + texHeaderSize + 16 + endFillerSize
	long f6;
	long q1;				//*?*All q-labels I'm not sure what they're for...
	long q2;				//*?*Always seems to be 1 (note: for the 1st scene, all tex scene headers referring to this file were '3, ?, 1')
	long fa7[3];
	long fa8[12];			//This is the 'endFillerSize'/4
} texture_header;

//NOTE: This is the batch texture header.  It lets you know how many textures are contained in the texture file or at the texture offset
typedef struct
{
	long texBatchSegMarker;	//All segment markers are -1
	long f1;				//All f-labels are filler
	long texBatchHeaderSize;//Size of this batch header (from texBatchSegMarker to just before texHeaderSegMarker)
	long texBatchSize;		// = res * res * (bpp/8) * #tex + 128 * #tex (like in a map file after the verticies)
	long f2;
	long numBatchedTex;		//The count of batched textures
	long fa3[2];			//All fa-lables are filler arrays
} texture_batch_header;

//NOTE: This is the header for batched textures after the main (first) one
typedef struct
{
	long texHeaderSegMarker;//Sub-Header segment marker - differentiates between subsequent tex headers and main tex batch header
	long f4;
	unsigned short int width;		//Texture width
	unsigned short int height;		//Texture Height
	unsigned char bpp;				//Bits per pixel
	unsigned char endFillerSize;	//# bytes from texHeaderSize+16 to 128 filled w/ 0
	unsigned short int	f5;
	long texSize;			// = res * res * (bpp/8)
	long texHeaderSegSize;	// = texSize + texHeaderSize + 16 + endFillerSize
	long f6;
	long q1;				//*?*All q-labels I'm not sure what they're for...
	long q2;				//*?*Always seems to be 1 (note: for the 1st scene, all tex scene headers referring to this file were '3, ?, 1')
	long fa7[3];
	long fa8[12];			//This is the 'endFillerSize'/4
} texture_sub_header;



//----===[ S C E N E   H E A D E R   S T R U C T U R E S ]===----/

//NOTE: All offsets are in relation to the 'baseOffset' which indexes at the start of each 'arc' file

//---==[ MAIN SCENE HEADER ]==---/
typedef struct
{
	//--=[ Main Header Start ]=--/
	long	mainHeaderSegMarker;	//Marks start of main header
	long	fa1[2];					//Filler Array
	long	mainHeaderSize;			//*?*Could be main header size
	long	texSegOffset;			//Location of start of local textures
	long	f2;
	long	altMainHeaderSize;		//*?*Could also be main header size
	long	totalMainHeaderSize;	// = mainHeaderSize + secondaryHeaderSize (could also be starting offset of scene headers)
	long	f3;
	long	sceneStartHeaderOffset;	//Offset of start of the scene.  Could be in middle of array of scene parts..
	long	fa4[2];
	long	altTexSegOffset;		//Location of start of local textures
	long	transMatOffset;			//Location of four 4x4 translation matricies
	long	someWeirdDataOffset;	//*?*location w/ a number of offsets w/ matricies, verts, and unidentified float groupings (could be interaction spots, maybe?)
	long	f9;
	unsigned short maybeTotalTex;	//*?*Could be total textures
	//	long maybeID;					//*?* could be an ID (there are two other #'s like it, but slightly larger
	unsigned short maybeLocalTexBaseIndex;	//*?*Could be base index # for local textures
	unsigned short numLocalTex;			//Number of textures contained in this file part
	unsigned short q1;
	long	fa5[2];
	//--=[ Secondary Header Start ]=--/
	long	transHeaderSize;		//Size of scene transform header
	long	floatCount;				//*?* could be # of floats (that look like skybox)
	long	secondaryHeaderSize;	//Size of 2ndary Header, including the matrix and vertex group
	long	f7;
	long	transSectID;			//This is '1' for default, and > 1 to signifiy the transform section.
	long	transSectSubID;			//Defines subsection, or possibly frame to display
	long	fa8[2];
	matrix	transMat;				//Translation Matrix for scene
	vertex4f skybox[8];				//*?* These verticies form a cube - NOT TRUE!!!.  Could also be scene max extents...	
}main_scene_header;


typedef struct
{
	long	transHeaderSize;		//Size of scene transform header
	long	floatCount;				//*?* could be # of floats (that look like skybox)
	long	secondaryHeaderSize;	//Size of 2ndary Header, including the matrix and vertex group
	long	f7;
	long	transSectID;			//This is '1' for default, and > 1 to signifiy the transform section.
	long	transSectSubID;			//Defines subsection, or possibly frame to display
	long	fa8[2];
	matrix	transMat;				//Translation Matrix for scene
	vertex4f skybox[8];				//*?* These verticies form a cube - NOT TRUE!!!.  Could also be scene max extents...	
}transform_scene_header;
	
//---==[ SCENE TRANS MATRICIES AND SCENE INTERACTION DATA ]==---/

	//NOTE: I believe these and the members of the next structures either deal with camera data, or 
	//      interaction data (things the player interacts with) it's more likely camera data though.
	//      It could also contain character placement data when starting and entering an area.
typedef struct
{
	matrix	transMat1;
	matrix	transMat2;
	matrix	transMat3;
	matrix	transMat4;
}scene_trans_header;


typedef struct
{
	float	rangeVal[2];			//For the 12 float segments, they are {-,-},{-,+},{+,-},{+,+}.  
									//I think these are quadrant ranges.  If camera is in quadrant, these 
									//are max distances in  plane defined by camera tilt.  Heather is origin
	vertex	cameraTilt;				//These values are in the range [0,1]. 
	vertex	revCameraTile;			//Like the cameraTilt, but order is reversed (ie .5,1,0 -> 0,1,.5)
	float	upDownFreeLookAngles[2];//Angles that can be looked up and down from heather's view
	float	leftRightFreeLookAngles[2];	//Angles that can be looked left and right from heather's view
}camera_data;

	//NOTE: These two structures are basically the same.  Read the first in from disk, copy to the 2nd, along
	//      with the vertex data pointed to by the vertOffset
typedef struct
{
	matrix	transMat;
	long	maybeID;				//*?* Like other maybeIDs...Could also be a two short int #
	long	q1;						//*?* Not sure, but definitely not a float (too large to be useful)
	long	fa1[2];
	long	vertOffset;				//Offset to dynamic # of vertex4f
	long	floatDataOffset;		//Offset to 48 floats (seem to be camera data - I'll treat it like so for now)
	long	nextInteractionOffset;	//Offset of next scene interaction data struct.  If 0, no more...
	float	f2;
	long	fa3[4];
}scene_interact_base;

typedef struct
{
	matrix	transMat;
	long	maybeID;				//*?* Like other maybeIDs...Could also be a two short int #
	long	q1;						//*?* Not sure, but definitely not a float (too large to be useful)
	long	fa1[2];
	long	vertOffset;				//Offset to dynamic # of vertex4f
	long	floatDataOffset;		//Offset to 48 floats (seem to be camera data - I'll treat it like so for now)
	long	nextInteractionOffset;	//Offset of next scene interaction data struct.  If 0, no more...
	float	f2;
	long	fa3[4];
	int		numVerts;				// = (floatDataOffset - vertOffset) / sizeof( vertex4f )
	vertex4f *verts;				//Vertex Data
	camera_data cameraViews[4];		//These are for the 4 quadrants.  If heather is origin, the camera is always in one...
}scene_interact;

	//NOTE: This structure cannot be scan-read.  It is more dynamic in nature
typedef struct
{
	long	startOffset;			//Location of first scene_interact_base
	long	numSceneInteract;		//Number of scene_interact structs;
	scene_interact	*sceneInteract;	//scene interaction data (I don't know how it works yet...
}scene_interact_header;


//----===[ S C E N E   V E R T E X   A N D   R E N D E R   H E A D E R S ]===----/
typedef struct
{
	long	nextTexHeader;			//Offset of next scene data header w/ scene texture header
	long	headerSize;				//Size of texture header (always 48)
	long	segmentSize;			//This + current header offset = nextTexHeader
	long	f1;
	long	texNumLoc;				//*?*Seems to refer to the location of the texture (local or external file)
	long	texNum;					//*?*Texture Number.  May start w/ an offset defined in main header (see maybeLocalTexBaseIndex)
	long	texRenderModifier;		//*?*Maybe affects render states
	long	fa2[5];
}scene_tex_header;

typedef struct
{
	long	nextRenderHeader;		//Offset of next scene data header w/ scene render header
	long	headerSize;				//Size of render header (always 48)
	long	segmentSize;			//This + current header offset = nextRenderHeader
	long	f1;
	long	renderFlag1;			//Affects render flags
	long	renderFlag2;			//Controls shader # in some cases, and alpha test (if 0, alpha test=0, if 196608 or 65536 and shader = 0, alpha test=1)
	long	fa2[6];
}scene_render_header;

typedef struct
{
	long	nextShaderHeader;		//Offset of next scene data header w/ scene shader header
	long	headerSize;				//Size of shader header (always 48)
	long	segmentSize;			//This + current header offset = nextShaderHeader
	long	f1;
	long	shaderNum;				//Controls current shader (if 0, it's that 213 shader)
	long	fa2[3];
	float	q1;
	float	q2;
	long	f3;
	float	q3;
}scene_shader_header;

typedef struct
{
	long	nextSceneHeader;		//Offset of next scene header -can also contain render/shader/texture headers
	long	sceneHeaderSize;		//Size of scene header (always 64)
	long	sceneSegmentSize;		// = 64 + 36 * #verticies
	long	f1;
	long	numVerts;				// Number of verticies
	long	stencilSwitch;			//shader switch control(if 1, it's normal, if 2 and shader is 9, shader
									//is switched to '21' next time renderFlag2 is '0')
	long	seqNum;					//Ordered ascending for all scene headers, from multi-header grouping to next multi-header grouping
	long	alphaBlendFlag;			//If = 60, alpha blend = 0.  If 124, alpha blend = 1
	long	fa2[8];
}scene_header;



//----===[ A C T O R   A N I M A T I O N   H E A D E R S ]===----/
#pragma pack(push)
#pragma pack(1)

/*-------------------------------------------------------/
/-- Legend:                                            --/
/--     *Marker - 1st val in struct; Always constant.  --/
/--     vals -  Floating point values w/o a doubt.     --/
/--     sa -  Sequenced data.  Could be index,floats   --/
/--     fa -  filler data.  Always 0                   --/
/--     ma -  Marker values. Constant across anim files--/
/-------------------------------------------------------*/

typedef struct
{
	vertex	position;		//orientation position
	short	angles[3];		//orientation angles
}sh3_anim_pa;

typedef struct				//SIZE: 76 bytes
{
	long	baseAnimMarker;			//Usually 91999aa hex
	float	vals1[3];				//First set of values
	BYTE	sa1[6];
	float	vals2[3];
	BYTE	sa2[30];
	BYTE	fa1[6];
	BYTE	sa3[6];
}sh3_base_anim;

typedef struct				//SIZE: 52 bytes
{
	long	altAnimMarker;			//Usually 99999999 hex
	BYTE	fa1[4];
	BYTE	sa1[2];
	BYTE	fa2[4];
	BYTE	sa2[2];
	BYTE	ma1[6];
	BYTE	sa3[6];
	BYTE	fa3[4];
	BYTE	sa4[2];
	BYTE	fa4[4];
	BYTE	sa5[14];
}sh3_alt1_anim;

typedef struct				//SIZE: 62 bytes
{
	long	altAnimMarker;			//Usually 99999999 hex
	BYTE	sa1[6];
	BYTE	fa1[4];
	BYTE	sa2[2];
	BYTE	fa2[4];
	BYTE	sa3[2];
	BYTE	fa3[4];
	BYTE	sa4[2];
	BYTE	fa4[4];
	BYTE	sa5[2];
	BYTE	fa5[4];
	BYTE	sa6[2];
	BYTE	fa6[4];
	BYTE	sa7[2];
	BYTE	ma1[16];
}sh3_alt2_anim;

#pragma pack(pop)


//----===[ A R C   I N D E X   D A T A ]===----/

class index_record
{
public:
	index_record( long _offset = 0, long _q1 = 0, long _size = 0 ):offset( _offset ),q1( _q1 ),size( _size ),size2( _size ){ }
	index_record( const index_record & rhs ){ operator=( rhs ); }
	~index_record( ){}
	index_record & operator=( const index_record & rhs ){ if( &rhs != this ){ offset = rhs.offset; q1 = rhs.q1; size = rhs.size; size2 = rhs.size2; }return *this; }

	void	ChangeSize( long newSize ){ if( newSize >= 0 ){ size2 = newSize; }}
	long	getChangedSize( ){ return size2; }
	void	RealignSize( ){ size2 = size; }
	long	writeChanged( FILE *outFile, long _offset )
	{
		fwrite((void *)&_offset, sizeof( long ), 1, outFile );
		fwrite((void *)&q1, sizeof( long ), 1, outFile );
		fwrite((void *)&size2, sizeof( long ), 1, outFile );
		fwrite((void *)&size2, sizeof( long ), 1, outFile );
		return size2;
	}

	long	offset;					//Offset of subfile
	long	q1;						//*?* Subfile ID, or Could be differece (unused portion)
	long	size;					//Size of subfile
	long	size2;					//Size of subfile
};



class arc_index_data
{
public:
	arc_index_data( char *_filename = NULL ){ arcFilename[ 0 ] = '\0';  index = NULL; numIndex = 0; if( _filename )LoadData( _filename ); }
	arc_index_data( const arc_index_data & rhs ){ index = NULL; operator=( rhs ); }
	~arc_index_data( ){ SAFEDELETE(index); internalFilenames.clear( ); }
	arc_index_data & operator=( const arc_index_data & rhs )
	{
		if( &rhs != this )
		{ 
			SAFEDELETE( index ); 
			internalFilenames.clear();
			strcpy( arcFilename, rhs.arcFilename );
			indexID = rhs.indexID; 
			numIndex = rhs.numIndex; 
			indexSize = rhs.indexSize; 
			f1 = rhs.f1; 
			index = new index_record[ numIndex ]; 
			for( long k = 0; k < numIndex; k++ ) 
				index[ k ] = rhs.index[ k ]; 
			internalFilenames = rhs.internalFilenames;
		}
		return *this; 
	}


	long	LoadData( char *filename );
	long	LoadFullData( char *_pcDir, char *_pcBaseFilename );
	bool	RegenOffsets( char *outFile );

	index_record &operator[](int ind);                // Mutator
	index_record  operator[](int ind) const;          // Accessor

	long			indexID;				//Used to identify that file starts with index list
	long			numIndex;				//# of indexes
	long			indexSize;				//Size of index section
	long			f1;
	index_record	*index;			//Indexes of subfiles
	char			arcFilename[ 256 ];		//Name of file that index belongs to.
	vector<string>	internalFilenames;
};
	


//----===[ G E N E R A L   F U N C T I O N S ]===----/

int loadArcIndex( FILE *infile, arc_index_data *index);
int getArcOffset( FILE *infile, int indexNum, long *offset);

void debugMainHeader( main_scene_header *sh );
void debugTransformHeader( transform_scene_header *sh );
void debugSceneTexHeader( scene_tex_header *h );
void debugSceneRenderHeader( scene_render_header *h );
void debugSceneShaderHeader( scene_shader_header *h );
void debugSceneHeader( scene_header *h );
void debugTextureSubHeader( texture_sub_header *sh );
void debugTextureBatchHeader( texture_batch_header *h );

void debugModelBaseHeader( model_base_header *h );
void debugModelDataHeader( model_data_header *h );
void debugModelVertHeader( model_vertex_header *h );
void debugModelPrimitive( model_primitive *h );

void debugVertexData( char *outputName, BYTE *data, int size, int numVerts );
void debugVertex4fData( char *outputName, BYTE *data, int numVerts );

void debugBaseAnim( sh3_base_anim *h );
void debugAltAnim1( sh3_alt1_anim *h );
void debugAltAnim2( sh3_alt2_anim *h );

void saveTexture( texture_sub_header *texHeader, BYTE *data, char *filename );

//----===[ S C E N E   D E S C R I P T I O N   D A T A ]===----/

class scenePrimitive
{
public:
	//scenePrimitive( ){ verts = NULL; numPrim = 0; }
	scenePrimitive( sh_map_vertex *_verts = NULL, GLuint _primitiveType = GL_TRIANGLE_STRIP, int _texID = -1, 
		int _startVert = 0, int _numPrim = 0,int _alphaTest = 0, int _alphaBlend = 0, int _stencilRef = 64, 
		int _shaderNum = 9, int _transformNum = 0 )
		{	verts = _verts; primitiveType = _primitiveType; startVert = _startVert; numPrim = _numPrim;
			texID = _texID; alphaTest = _alphaTest; alphaBlend = _alphaBlend; stencilRef = _stencilRef;
			shaderNum = _shaderNum; transformNum = _transformNum; }
	~scenePrimitive( ){ SAFEDELETE(verts);}

	scenePrimitive & operator=( const scenePrimitive & rhs );
	void computeMinMax( );
	void draw();

	sh_map_vertex *	verts;
	vertex			min;
	vertex			max;
	GLuint			primitiveType;
	GLuint			texID;
	int				startVert;
	int				numPrim;
	int				alphaTest;
	int				alphaBlend;
	int				stencilRef;
	int				shaderNum;
	int				transformNum;
};


class SceneMap
{
public:
	SceneMap( ){ sceneData = NULL; numScenePrimitives = 0; maxScenePrimitives = 0; baseOffset = 0;
				 nextTexOffset = 0; nextRenderOffset = 0; nextShaderOffset = 0; nextSceneOffset = 0;
				 interact.sceneInteract = NULL; numSceneTransform = 0; sceneTransform = NULL; sceneFilename[0] = 0;}
	SceneMap( FILE *sceneFile ){ loadScene(sceneFile, 0); }
	SceneMap( char *filename, int sceneNum ){ loadArcScene( filename, sceneNum ); }
	~SceneMap( ){ deleteScene();}

	void deleteScene();
	int determineHeaderType( long *data );
	GLuint loadTex( FILE *infile, scene_tex_header *h );				//Loads a texture from the map file, depending on scene data

	int  loadScene( FILE *sceneFile, long bOffset );	//Loads scene from file that already points to scene start
	void loadTexture( FILE *sceneFile );
	int  loadArcScene( char *filename, int sceneNum );	//Loads scene from arc file - uses loadScene
	int  getMinScene( char *filename );					//Determines first scene offset
	int  getMaxScene( char *filename );					//Returns last scene offset
	bool isSceneValid(){ return (numScenePrimitives > 0 && sceneData );}	//Determines if a scene is loaded
	bool isMainSceneHeader( main_scene_header *h );		//Determines if a main header is actually a main header

	void renderScene( );						//Renders sceneData

	scenePrimitive * getSceneData( ){ return sceneData; }
	int getNumScenePrimitives( ){ return numScenePrimitives; }

	//-----[ SCENE DATA ]-----/
	char					sceneFilename[256];	//Filename of scene
	long					baseOffset;			//Start of scene in arc file

	scenePrimitive			*sceneData;			//All of the primitives for the scene
	int						numScenePrimitives;

	main_scene_header		mainHeader;
	transform_scene_header	*sceneTransform;
	int						numSceneTransform;	//# of transform_scene_headers after main header and default transform_scene_header

	//-----[ ODD DATA ]-----/
	scene_trans_header		transMatSet;		//Group of 4 transform matricies, for what, I don't know
	scene_interact_header	interact;			//Interaction Data, I think (this appears to be camera data.)

	//--=[ Variables Used During Loading ]=--/
	int						maxScenePrimitives;
	long					nextTexOffset;
	long					nextRenderOffset;
	long					nextShaderOffset;
	long					nextSceneOffset;
	GLuint					curTexID;
	long					checkBaseIndex;

};

/*
class SH3_Model
{
public:
	SH3_Model();
	SH3_Model( char *filename, int modelNum );
	~SH3_Model();

	void ReleaseModel();
	int LoadModel( char *filename, int modelNum );
	int SaveModelData(char *indexBase, char *vertBase);
	void Render();
	//int LoadTex();
	//int LoadHeader();
	//int LoadVertexData( int count );
	//int LoadIndexData( int count );

	int texWidth, texHeight;
	model_header mHeader;
	int temp_index_count; //Number of indicies to process
	int txStartVert,txEndVert;
	sh_vertex *verts;
	vertex4f *dispVerts;
	long *index;
	int model_num;
	long offsetInFile;
	GLuint *texID;
};
*/
#endif
