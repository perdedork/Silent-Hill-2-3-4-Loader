#ifndef __SH4_LOADER_H__
#define __SH4_LOADER_H__

#include <windows.h>
#include <gl\gl.h>
#include <gl\glu.h>
#include <gl\wglext.h>
#include <gl\glext.h>
#include <gl\glprocs.h>
#include "vertex.h"
#include "matrix.h"
#include "quat.h"
#include "RVertex.h"
#include "typedefs.h"
#include <errno.h>

extern int errno;
/*****************************************************************
MOVE THIS SECTION TO BETTER PLACE!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
****************************************************************/



/*********************************************************
MOVE PREVIOUS SECTION TO BETTER PLACE!!!!!!!!!!!!!!!!!!!!!!
*********************************************************/
#pragma pack(push)
#pragma pack(1)

class sh4_model_vert
{
public:
	sh4_model_vert( ){ }
	sh4_model_vert( vertex & _pos, vertex & _norm, texcoord & _tc, vertex4f & _shaderMatOff, vertex4f & _weights )
		:pos(_pos),normal(_norm), tc(_tc),shaderSelect(_shaderMatOff),weights(_weights){ }
	sh4_model_vert( const sh4_model_vert & rhs ){ operator=( rhs ); }
	~sh4_model_vert( ){ }
	sh4_model_vert & operator=( const sh4_model_vert & rhs ){ if( &rhs != this ){ pos=rhs.pos;normal=rhs.normal;shaderSelect=rhs.shaderSelect;weights=rhs.weights;}return *this; }

	vertex pos;
	vertex normal;
	texcoord tc;
	vertex4f shaderSelect;
	vertex4f weights;
};


typedef struct
{
	short	numT1;
	short	numT2;
	long	f1_th;
	long	f2_th;
	long	f3_th;
}sh4_tex_header;

typedef struct
{
	long	xRes;
	long	yRes;
	short	flag1;
	short	flag2;
	long	f1_tt1;
}sh4_model_tex_type1;

typedef struct
{
	long	marker;
	long	numTex;
	long	f1_tt2;
	long	offsetType3;	//Offset from start of THIS struct
}sh4_model_tex_type2;

typedef struct
{
	long	f1_tt3;
	long	f2_tt3;
	long	f3_tt3;
	long	f4_tt3;
	long	f5_tt3;
	long	f6_tt3;
	long	f7_tt3;
	long	f8_tt3;
	long	xRes;
	long	yRes;
	long	texFormat;
	long	mipMapCount;
	long	texDataSize;
	long	mipMap1Size;
	long	mipMap2Size;
	long	mipMap3Size;
	long	mipMap4Size;
	long	mipMap5Size;
	long	mipMap6Size;
	long	numOffsets;
	long	offsetTexData;	//Offset from start of THIS struct
	long	mipMap1Off;
	long	mipMap2Off;
	long	mipMap3Off;
	long	mipMap4Off;
	long	mipMap5Off;
	long	mipMap6Off;
	long	q1_tt3;
}sh4_model_tex_type3;


class sh4_model_tex
{
public:
	sh4_model_tex( long offsetT1 = 0, long offsetT2 = 0,FILE *inFile = NULL ){ setNullAll( ); loadData( offsetT1, offsetT2, inFile ); }
	sh4_model_tex( const sh4_model_tex & rhs ){ setNullAll( ); operator=( rhs ); }
	~sh4_model_tex( ){ deleteData( ); }
	sh4_model_tex & operator=( const sh4_model_tex & rhs );

	void setNullAll( ){ m_sT3 = NULL; m_puiTexID = NULL; };
	long loadData( long offsetT1, long offsetT2,FILE *inFile = NULL );
	void deleteData( ){ delete [] m_sT3; delete [] m_puiTexID; setNullAll( ); }
	void saveTexture( sh4_model_tex_type3 *texHeader, BYTE *data, char *filename );

	sh4_model_tex_type1		m_sT1;
	sh4_model_tex_type2		m_sT2;
	sh4_model_tex_type3		*m_sT3;
	GLuint					*m_puiTexID;
};

typedef struct
{
	long	modelMarker;		// 0xffff0003
	long	modelNum;			// Is this right?
	long	offsetMatSet1;
	long	numMatSet1;
	long	offsetSeqDat1;
	long	numMatSet2;
	long	offsetSeqDat2;
	long	offsetMatSet2;
	long	numVertSects;		//? Maybe num vert sections?
	long	offsetVertSects;	//Offset to 1st vertex section
	long	numAltVertSects;
	long	offsetAltVertSects;	//Seems a little short, but we'll see...
	long	numTexSetIDs;		//Count of texID array
	long	offsetTexSetIDs;
	long	numTexIDMapping;	//Could be texture flags, each element is 8 bytes...
	long	offsetTexIDMapping;	//Offset to data for q2
	long	offsetAltModelHeader;//NOTE: I'm not sure how this is used, but in SH2 it's about the same data as the first header...
	long	numQuatData;		//Each element is 6 bytes (3 shorts)
	long	offsetQuatData;
	long	numQ5_mh;			//Count of short headers - 8 bytes each - First is a count, then is an offset from the start of this header.
	long	offsetQ5_mh;		//This section has it's own count, as well as an offset from start of model.  The section it points two appears to be 
	long	f2_mh;				//Filler in both SH2 and SH4
	long	offset_f3_mh;
	long	offset_f4_mh;
	long	offsetExtents;		//Offsets of 8 vertex4fs
	float	q6_mh;
	long	f5_mh;
	long	f6_mh;
	long	offset_q7_mh;		//Has offset and counts;
	long	f7_mh;
	long	f8_mh;
	long	f9_mh;
}sh4_model_header;


typedef struct
{
	long		vertexSectSize;		//Size of vertex section, including the vertices and indices
	long		f1_vp;
	long		headerSize;
	long		q1_vp_modelPartNum;
	long		q2_vp_modelPartSelect;	//Seems to be offset added to part# data (maybe L/R side info...)
	long		q3_vp_count;
	long		q4_vp_offset_q3;		//Offset for count of q3 data
	long		numSeq1Data;			//Count of shorts for matrix selection
	long		offsetSeq1Data;
	long		numSeq2Data;
	long		offsetSeq2Data;
	long		q5_vp_lowPart;			//Related to #'s in other vertex primitives, just offset by q2 value
	long		q6_vp_highPart;			//So if q2 is 0, this # will be = to another section's val - their q2
	long		numTexID;				//I think this is texID count, based on SH3 format
	long		offsetTexIDs;			//Offset to shorts containing texture IDs
	long		offsetStaticData;		//Offset to 3 weird static #'s like SH3
	long		numStaticData;
	long		f2_vp;
	float		q7_vp;					//Seem to be similar across primitives
	float		q8_vp;					//Seem to be similar across prims
	long		f3_vp;
	long		f4_vp;
	long		f5_vp;
	long		f6_vp;
	vertex4f	q9_vp;					//All 3 vals seem to be same (except w)
	vertex4f	q10_vp;					//All 3 vals seem to be same (except w)
	vertex4f	q11_vp;					//All 3 vals seem to be same (except w)
}sh4_vertex_prim_header;

typedef struct
{
	long		staticData[4];	//Seems to be same as SH3
	long		numVertSet;			//Always 1
	long		f1_vd;
	long		q2_vd;			//Always 12
}sh4_vertex_type_header;


typedef struct
{
	long		fa2_vd[16];		//Looks like array of 16 blank floats...
	long		numIndex;		//Count of shorts for indicies
	long		numVertex;
	long		vertexSize;
	long		q3_vd;			//Maybe shader #, or tex ID
	long		offsetIndexData;//Offset to the index data (after the vertex data)
	long		offsetVertData;	//From start of this struct, add 32 to this offset to get to the verts
	long		offsetSeqOrder;	//Offset to array of sequence matrix reordering... (I think this reorders the matrices, or maybe is used by shader to place them in correct position)
}sh4_vertex_data_head;


class sh4_vertex_data
{
public:
	sh4_vertex_data( FILE *inFile = NULL ){ setNullAll( ); loadDataHeader( inFile ); }
	sh4_vertex_data( const sh4_vertex_data & rhs ){ setNullAll( ); operator=( rhs ); }
	~sh4_vertex_data( ){ deleteData( ); }
	sh4_vertex_data & operator=( const sh4_vertex_data & rhs );

	void setNullAll( );
	void deleteData( );
	long loadDataHeader( FILE *inFile = NULL );
	long loadDataVerts( FILE *inFile = NULL );

	sh4_vertex_data_head	m_sVertexType;
	sh4_model_vert			*verts;
	short					*indicies;

	long					m_lNumSeqReorder;
	long					*seqReorder;
	long					m_lStartOffset;
	bool					m_bHeaderLoaded;
};

class sh4_quat_assembly
{
public:
	sh4_quat_assembly( long baseOffset = -1, FILE *inFile = NULL ){ setNullAll( ); loadData( baseOffset, inFile ); }
	sh4_quat_assembly( const sh4_quat_assembly & rhs ){ setNullAll( ); operator=( rhs ); }
	~sh4_quat_assembly( ){ deleteData( ); }
	sh4_quat_assembly & operator=( const sh4_quat_assembly & rhs );

	void setNullAll( );
	long loadData( long startOffset, FILE *inFile );
	void deleteData( );

	long dataCount;		//Count of buildData
	long *buildData;	//Each buildData element is 8 bytes

};



class sh4_model_header_dynamic
{
public:
	matrix				*matSet1;
	matrix				*matSet2;
	unsigned char		*mSeq1;
	unsigned char		*mSeq2;
	long				*texIDs;
	long				*texIDMapping;		//8 bytes each
	short				*quatData;			//numQuatData * 3
	sh4_quat_assembly	*quatBuild;
	vertex4f			extents[8];

	//-------------NOT SURE - COULD BE DATA FOR TRANSFORMING TO AFFLINE MATRICIES-------------------/
	short				qa1[4];
	matrix				q2;

	sh4_model_header_dynamic( long baseOffset = -1, sh4_model_header *loadDat = NULL, FILE *inFile = NULL ){ setNullAll(); loadData( baseOffset, loadDat, inFile );}
	//sh4_model_header_dynamic( const sh4_model_header_dynamic & rhs ){ operator=( rhs ); }
	~sh4_model_header_dynamic( ){ deleteData( ); }
	//sh4_model_header_dynamic & operator=( const sh4_model_header_dynamic & rhs );

	void copyData( const sh4_model_header_dynamic &rhs, const sh4_model_header *header );
	void setNullAll( );
	long loadData( long baseOffset, sh4_model_header *loadDat, FILE *inFile );
	void deleteData( );
};


class sh4_vertex_prim
{
public:
	sh4_vertex_prim( FILE *inFile = NULL ){ setNullAll(); loadData( inFile ); }
	sh4_vertex_prim( const sh4_vertex_prim & rhs ){ setNullAll( ); operator=( rhs ); }
	~sh4_vertex_prim( ){ deleteData( ); }
	sh4_vertex_prim & operator=( const sh4_vertex_prim & rhs );

	void setNullAll( );
	long loadData( FILE *inFile );
	void deleteData( );
	void Render( );

	sh4_vertex_prim_header	m_cPrimHeader;
	short					*seqData1;
	short					*seqData2;
	short					*texIDs;
	long					m_lNumVertexData;
	sh4_vertex_data			*m_cVertexData;

//	long					*seqReorder;		//Count = # seqData1 + # seqData2

};


class sh4_anim
{
public:
	sh4_anim( FILE *inFile = NULL ){ setNullAll( ); loadData( inFile ); }
	sh4_anim( const sh4_anim & rhs ){ setNullAll( ); operator=( rhs ); }
	~sh4_anim( ){ deleteData( ); }
	sh4_anim & operator=( const sh4_anim & rhs ){ if( &rhs != this ){  }return *this; }

	void deleteData( ){ setNullAll( ); }
	void setNullAll( ){ }

	long loadData( FILE *inFile = NULL ){ if( !inFile ) return 0; return -1; }
};


class sh4_model
{
public:
	sh4_model( char *filename = NULL){ setNullAll( ); loadData( filename ); }
	sh4_model( const sh4_model & rhs ){ setNullAll( ); operator=( rhs ); }
	~sh4_model( ){ deleteData( ); }
	sh4_model & operator=( const sh4_model & rhs );

//	void debugSH4_MD( );
	quat convertToQuat( short int xVal, short int yVal, short int zVal );

	void setNullAll( );
	void deleteData( );
	long loadData( char *filename = NULL );
	long loadDataAux( FILE *inFile = NULL );
	GLuint loadTex( char *texName, FILE *inFile );
	void Render( );

	bool AttachAnim( sh4_anim *_pcAnimDat = NULL ){ m_pcAnimDat = _pcAnimDat; return( m_pcAnimDat != 0 ); }
	bool CreateAnim( long curFrame ){ return true; }

	sh4_model_header			m_sModelHeader;
	sh4_model_header_dynamic	m_cHeaderData;
	sh4_vertex_prim				*m_cVertPrim;
	long						m_lNumTexData;
	sh4_model_tex				*m_cTextures;
	long						m_lNumTex;
	GLuint						*texIDs;
	bool						m_bHasAnim;
	matrix						*m_pcAnimMat;
	sh4_anim					*m_pcAnimDat;
};

#pragma pack(pop)

void debugSH4_MTH( sh4_tex_header *h );
void debugSH4_MTT1( sh4_model_tex_type1 *h );
void debugSH4_MTT2( sh4_model_tex_type2 *h );
void debugSH4_MTT3( sh4_model_tex_type3 *h );
void debugSH4_MH( sh4_model_header *h );
void debugSH4_VS( sh4_vertex_prim *h );
void debugSH4_VP( sh4_vertex_prim_header *h );
void debugSH4_VDT( sh4_vertex_type_header *h );
void debugSH4_VDH( sh4_vertex_data_head *h );
void debugSH4_VD( sh4_vertex_data *h );

void debugSH4_MD( sh4_model_header_dynamic *h, sh4_model_header *mh );

#endif /*__SH4_LOADER_H__*/