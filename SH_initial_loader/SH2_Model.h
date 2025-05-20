#ifndef  __SH2_MODEL_H__
#define  __SH2_MODEL_H__

#include <windows.h>
#include <gl\gl.h>
#include <gl\glu.h>
#include <gl\wglext.h>
#include <gl\glext.h>
#include <gl\glprocs.h>
#include <stdio.h>
//#include <vector>

#include "vertex.h"
#include "quat.h"
#include "matrix.h"
#include "typedefs.h"
#include "mathlib.h"
#include "Camera.h"
#include "tgaload.h"
#include "RenderObjs.h"
#include "Renderer.h"
#include "IBaseTypes.h"

#include "SH2_Loader.h"			//--[ NEEDED FOR THE SH2 TEX MANAGER ]--/
#include "SH_Model_Anim.h"		//--[ NEEDED FOR QUAT ANGLE TYPES ]--/
#include "SH4_Loader.h"			//--[ NEEDED FOR VARIOUS HEADERS ]--/
#include "utils.h"

using namespace std;

//----===[ V E R T E X   T Y P E S ]===----/

#pragma pack(push)
#pragma pack(1)

class sh2_model_vert_long
{
public:
	sh2_model_vert_long( ){ }
	sh2_model_vert_long( const sh2_model_vert_long & rhs ){ operator=( rhs ); }
	~sh2_model_vert_long( ){ }
	sh2_model_vert_long & operator=( const sh2_model_vert_long & rhs )
	{ 
		if( &rhs != this )
		{ 
			long k; 
			vert = rhs.vert; 
			weights = rhs.weights;
			normal = rhs.normal; 
			tc = rhs.tc; 
			f0_mvl = rhs.f0_mvl; 
			f1_mvl = rhs.f1_mvl; 
			for( k = 0; k < 4; k ++ )
				mats[ k ] = rhs.mats[ k ];
		}
		return *this;
	}

	vertex4f	vert;			// Initial position
	vertex4f	weights;		// Weights - Corresponds to the mats[4] variable
	vertex4f	normal;			// Norm
	texcoord	tc;				// Texcoord
	long		f0_mvl;			// Filler data (they are using 4 floats for each type of data...
	long		f1_mvl;			// Filler data (they are using 4 floats for each type of data...
	short int	mats[4];		// Matricies to use to determine vert's final pos
};


class sh2_model_vert_short
{
public:
	sh2_model_vert_short( ){ }
	sh2_model_vert_short( const sh2_model_vert_short & rhs ){ operator=( rhs ); }
	~sh2_model_vert_short( ){ }
	sh2_model_vert_short & operator=( const sh2_model_vert_short & rhs )
	{
		if( & rhs != this )
		{
			vert = rhs.vert;
			weights = rhs.weights;
			quatVals.xQuatL = rhs.quatVals.xQuatL;
			quatVals.yQuatL = rhs.quatVals.yQuatL;
			quatVals.zQuatL = rhs.quatVals.zQuatL;
			f0_mvs = rhs.f0_mvs;
			tc = rhs.tc;
			copyRGBA(mats, rhs.mats);
		}
		return *this;
	}

	vertex		vert;			// Initial position
	vertex4f	weights;		// Weights - Corresponds to the mats[4] variable
	sh_anim_ang	quatVals;		// I belive this is a quat value - it looks like it...
	short int	f0_mvs;			// Filler data (this vert struct has everything aligned on 32 bits)
	texcoord	tc;				// Texcoord
	rgba		mats;			// Matricies to use to determine vert's final pos
};



//---===[ M O D E L   F I L E   H E A D E R S ]===---/

//--[ Struct for the initial data for a model file.  This is the start of the model file. ]--/
/*
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
*/

//--[ Struct of data offsets, counts, etc.  For verticies and indices Can appear multiple times in a model file ]--/
typedef struct
{
	long	numSet1Verts;	//Count of verts in 1st vertex type set
	long	offsetSet1Verts;//Offset of the 1st vertex type set
	long	numSet2Verts;
	long	offsetSet2Verts;
	long	offsetSet1Index;//Offset of 1st index set
	long	offsetSet2Index;
	long	numQ1_voh;		//Count of ???
	long	numQ2_voh;
	long	offsetQ3_voh;	//Offset of ?? seq data (if it matches numQ1_voh, then it's 7 bytes each)
	long	f1_voh;
	long	f2_voh;
	long	f3_voh;
}sh2_vertex_offset_header;


//--[ Texture Remapping ]--/

typedef struct
{
	long	texSetIndex;	//Points to one of the tex set Indexs
	long	texID;			//Tex ID from the substructures
}sh_tex_mapping;

//--[ Structs to hold vertex headers.  Similar to SH4, but different in some ways... ]--/
typedef struct
{
	long		headerSize;
	long		f1_vpsh;
}sh2_model_prim_size_header;

typedef struct
{
	long		numSeq1Data;			//Count of shorts for matrix selection
	long		offsetSeq1Data;
	long		numSeq2Data;
	long		offsetSeq2Data;
}sh2_vertex_prim_seq_header;

typedef struct
{
	long		numTexID;				//I think this is texID count, based on SH3 format
	long		offsetTexIDs;			//Offset to shorts containing texture IDs
	long		offsetEndMarker;
	long		numVertex4fs;			//Count of vertex4fs. Only seen 2 or 4.  If 2, the first is zeros
	long		q1_vpdh;				//?Maybe a flag - seems to be either 1 or 0
	long		f1_vpdh;
	long		f2_vpdh;
	vertex4f	q2_vpdh;
	vertex4f	q3_vpdh;
	vertex4f	q4_vpdh;
	vertex4f	q5_vpdh;
	long		startIndex;
	long		numIndex;
	long		q8_vpdh;				//In 2nd set of headers, this increases by 1, from 0 to 4 (0,1,2,3,4)
}sh2_vertex_prim_data_header;

typedef struct
{
	long		texID;
	long		f1_vtb;
	long		f2_vtb;
	long		f3_vtb;
}sh2_vertex_tex_block;

typedef struct
{
	long		endMarker;
	long		f1_vem;
	long		f2_vem;
	long		f3_vem;
}sh2_vertex_end_marker;


typedef struct
{
	long		numData;		//Count of data of 8 bytes each (These appear to be shorts)
	long		offsetData;		//Offset of data
}sh2_model_q1_data;


/*
class sh2_model_q1_set
{
public:
	
	sh2_model_q1_set( ){ setNullAll( ); }
	sh2_model_q1_set( const sh2_model_q1_set & rhs ){ operator=( rhs ); }
	~sh2_model_q1_set( ){ deleteData( ); }
	sh2_model_q1_set & operator=( const sh2_model_q1_set & rhs )
	{
		if( &rhs != this )
		{
			long k;

			deleteData( );
			m_lNumSets = rhs.m_lNumSets;
			m_sHeaders = new sh2_model_q1_data[ m_lNumSets ];

			structArrayMemcpy( m_sHeaders, m_lNumSets );

			m_sData = new utype8*[ m_lNumSets ];
			for( k = 0; k < m_lNumSets; k++ )
			{
				m_sData[ k ] = new utype8[ m_sHeaders[ k ].numData ];
				structArrayMemcpy( m_sData[ k ], m_sHeaders[ k ].numData );
			}
		}
		return *this;
	}

	void setNullAll( )
	{
		m_sData = NULL;
		m_sHeaders = NULL;
		m_lNumSets = 0;
	}

	void deleteData( )
	{
		long k;

		delete [] m_sHeaders;

		for( k = 0; k < m_lNumSets; k++ )
			delete [] m_sData[ k ];
		delete [] m_sData;

		setNullAll( );
	}

	long loadData( FILE *inFile, long i_lDataOffset, long i_lDataCount ){ return 0; }

	long				m_lNumSets;
	sh2_model_q1_data	*m_sHeaders;
	utype8				**m_sData;

};
*/


class sh2_vertex_data_long
{
public:
	sh2_vertex_data_long( ){ setNullAll( ); }
	sh2_vertex_data_long( const sh2_vertex_data_long & rhs ){ setNullAll( ); operator=( rhs ); }
	~sh2_vertex_data_long( ){ deleteData( ); }
	sh2_vertex_data_long & operator=( const sh2_vertex_data_long & rhs );

	void setNullAll( );
	void deleteData( );
	long loadDataHeader( FILE *inFile = NULL );

	sh2_model_prim_size_header		m_sHeaderSize;
	sh2_vertex_prim_seq_header	m_sSeqHeader;
	sh2_vertex_prim_data_header	m_sDataHeader;
	sh2_vertex_tex_block		m_sTexHeader;
	short						*seqData1;
	short						*seqData2;

	sh2_vertex_end_marker		m_sEndHeader;
};



class sh2_vertex_data_short
{
public:
	sh2_vertex_data_short( ){ setNullAll( ); }
	sh2_vertex_data_short( const sh2_vertex_data_short & rhs ){ setNullAll( ); operator=( rhs ); }
	~sh2_vertex_data_short( ){ deleteData( ); }
	sh2_vertex_data_short & operator=( const sh2_vertex_data_short & rhs );

	void setNullAll( );
	void deleteData( );
	long loadDataHeader( FILE *inFile = NULL );

	sh2_model_prim_size_header		m_sHeaderSize;
	sh2_vertex_prim_data_header	m_sDataHeader;
	sh2_vertex_tex_block		m_sTexHeader;
	sh2_vertex_end_marker		m_sEndHeader;
};


class sh2_vertex_large_header
{
public:
	sh2_vertex_large_header( ){ setNullAll( ); }
	sh2_vertex_large_header( const sh2_vertex_large_header & rhs ){ setNullAll( ); operator=( rhs ); }
	~sh2_vertex_large_header( ){ deleteData( ); }
	sh2_vertex_large_header & operator=( const sh2_vertex_large_header & rhs );

	void setNullAll( );
	void deleteData( );
	long loadDataHeader( FILE *inFile = NULL, long i_lCount = 0 );
	long loadDataVerts( FILE *inFile = NULL, long i_lCount = 0 );
	long loadDataIndex( FILE *inFile = NULL, long i_lCount = 0  );

	sh2_vertex_data_long	*m_sVertexType;
	long					m_lNumHeaders;
	long					m_lNumVerts;
	long					m_lNumIndex;
	sh2_model_vert_short	*verts;
	short					*indicies;
};


class sh2_vertex_small_header
{
public:
	sh2_vertex_small_header( ){ setNullAll( ); }
	sh2_vertex_small_header( const sh2_vertex_small_header & rhs ){ setNullAll( ); operator=( rhs ); }
	~sh2_vertex_small_header( ){ deleteData( ); }
	sh2_vertex_small_header & operator=( const sh2_vertex_small_header & rhs );

	void setNullAll( );
	void deleteData( );
	long loadDataHeader( FILE *inFile = NULL, long i_lCount = 0 );
	long loadDataVerts( FILE *inFile = NULL, long i_lCount = 0 );
	long loadDataIndex( FILE *inFile = NULL, long i_lCount = 0 );

	sh2_vertex_data_short	*m_sVertexType;
	long					m_lNumHeaders;
	long					m_lNumVerts;
	long					m_lNumIndex;
	sh2_model_vert_long		*verts;
	short					*indicies;
};


class sh2_model
{
public:
	sh2_model( char *filename = NULL){ setNullAll( );loadData( filename ); }
	sh2_model( const sh2_model & rhs ){ setNullAll( ); operator=( rhs ); }
	~sh2_model( ){ deleteData( ); }
	sh2_model & operator=( const sh2_model & rhs ); //NOTE: Don't copy animation data

	void debugSH2_MD( );
	quat convertToQuat( short int xVal, short int yVal, short int zVal );

	void setNullAll( );
	void deleteData( );
	long loadData( char *filename = NULL );
	long loadTexData( FILE *inFile = NULL );
	GLuint loadTex( char *texName, FILE *inFile );
	bool isLoaded( ){ return m_bModelLoaded; }
	void Render( );
	void RenderAlt( );
	void Render2( );
	void Render3( );

	//---[ ANIMATION METHODS ]---/
	long loadAnim( SH_Anim_Loader & i_pcAnimSet, char *i_pcFilename = NULL );
	bool setCurFrameRange( long i_lStartFrame = 0, long i_lFrameCount = -1 );
	bool nextFrame( );		//Increments current frame data.  Returns true until the EndFrame is reached.  Calling after that point repeats the sequence at the start
	bool prevFrame( );		//Decrements current frame data.  Returns true until the StartFrame is reached.  Calling after that point repeats the sequence at the end
	long computeInverseMats( );	//Computes the inverse of the matrix set 1, and populates member 'm_pcDispInverse'
	bool getAnimData( );
	void DetachFromAnim( ){ m_bHasAnim = false; m_pcAnimMat = NULL; if( m_pcAnimDat ) m_pcAnimDat->DetachModel( this ); m_pcAnimDat = NULL; }

	//bool AttachAnim( sh4_anim *_pcAnimDat = NULL ){ m_pcAnimDat = _pcAnimDat; return( m_pcAnimDat != 0 ); }
	//bool CreateAnim( long curFrame ){ return true; }

	char						m_pcModelFilename[512];
	sh2_model_base_header		m_sModelBaseHeader;
	sh4_model_header			m_sModelHeader;
	sh2_vertex_offset_header	m_sVertexHeader;
	sh4_model_header_dynamic	m_cHeaderData;
	long						*m_plTexSetIDs;
	sh_tex_mapping				*m_pcTexSetMapping;
	sh2_vertex_small_header		m_cVertPrimSmall;
	sh2_vertex_large_header		m_cVertPrimLarge;
	bool						m_bModelLoaded;
	bool firstRender;
	//sh2_model_q1_set			m_cQ1_Data;			//Unknown data (8 bytes each, multiple sets)

	//---[ Texture Data Headers ]---/
	vector<sh2_tex_data>	m_vTexData;

	//---[ Animation Data ]---/
	bool						m_bHasAnim;
	matrix						*m_pcAnimMat;
	matrix						*m_pcDispInverse;	//Inverse of 1st set of matricies.  Used for animations
	matrix						*m_pcTransMatSet2;
	SH_Anim_Loader				*m_pcAnimDat;
	long						m_lNumAnimMat;		//Count of data in referential pointer 'm_pcAnimMat' (don't delete pointer, owned by m_pcAnimDat)
	long						m_lStartFrame;
	long						m_lFrameCount;
	long						m_lCurFrame;
	//sh4_anim					*m_pcAnimDat;
};

#pragma pack(pop)

void debugM2_MPSH( sh2_model_prim_size_header *h );
void debugM2_VPSH( sh2_vertex_prim_seq_header *h );
void debugM2_VPDH( sh2_vertex_prim_data_header *h );
void debugM2_VTB( sh2_vertex_tex_block *h );
void debugM2_VEM( sh2_vertex_end_marker *h );
void debugM2_VDS( sh2_vertex_data_short *h );
void debugM2_VDL( sh2_vertex_data_long *h );
void debugM2_VOF( sh2_vertex_offset_header *h );

#endif /*__SH2_MODEL_H__*/