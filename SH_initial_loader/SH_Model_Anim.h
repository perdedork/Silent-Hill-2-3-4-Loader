#ifndef  __SH_MODEL_ANIM_H__
#define  __SH_MODEL_ANIM_H__


#include <windows.h>
#include <gl\gl.h>
#include <gl\glu.h>
#include <gl\wglext.h>
#include <gl\glext.h>
#include <gl\glprocs.h>
#include <stdio.h>

#include "vertex.h"
#include "quat.h"
#include "matrix.h"
#include "typedefs.h"
#include "mathlib.h"
#include "Camera.h"
#include "plane.h"
#include "SH3_Loader.h"
//#include "SH2_Model.h"

#include <SMD_Vertex.h>


long GetFileInfo( char *filename );

//--------------------------------------------------------------------------------------------/
//--  A N I M A T I O N   C L A S S E S  -  S T U C T S   &   D E F I N E S   F O R   S H 3 --/
//--                                                                                        --/
//--------------------------------------------------------------------------------------------/

#define		SHA_BYTE_1				0x000000ff
#define		SHA_BYTE_1_LOW			0x0000000f
#define		SHA_BYTE_1_HIGH			0x000000f0
#define		SHA_BYTE_2				0x0000ff00
#define		SHA_BYTE_2_LOW			0x00000f00
#define		SHA_BYTE_2_HIGH			0x0000f000
#define		SHA_BYTE_3				0x00ff0000
#define		SHA_BYTE_3_LOW			0x000f0000
#define		SHA_BYTE_3_HIGH			0x00f00000
#define		SHA_BYTE_4				0xff000000
#define		SHA_BYTE_4_LOW			0x0f000000
#define		SHA_BYTE_4_HIGH			0xf0000000

#define		SHA_BYTE_1_OFF			0
#define		SHA_BYTE_1_LOW_OFF		0
#define		SHA_BYTE_1_HIGH_OFF		4
#define		SHA_BYTE_2_OFF			8
#define		SHA_BYTE_2_LOW_OFF		8
#define		SHA_BYTE_2_HIGH_OFF		12
#define		SHA_BYTE_3_OFF			16
#define		SHA_BYTE_3_LOW_OFF		16
#define		SHA_BYTE_3_HIGH_OFF		20
#define		SHA_BYTE_4_OFF			24
#define		SHA_BYTE_4_LOW_OFF		24
#define		SHA_BYTE_4_HIGH_OFF		28

#define		SHA_GET_FLAG_DATA( f, m, o )	( ( f & m ) >> o )
#define		byte1_all( f )					( (unsigned char)SHA_GET_FLAG_DATA( (f), SHA_BYTE_1,      SHA_BYTE_1_OFF) )
#define		byte1_low( f )					( (unsigned char)SHA_GET_FLAG_DATA( (f), SHA_BYTE_1_LOW,  SHA_BYTE_1_LOW_OFF) )
#define		byte1_high( f )					( (unsigned char)SHA_GET_FLAG_DATA( (f), SHA_BYTE_1_HIGH, SHA_BYTE_1_HIGH_OFF) )
#define		byte2_all( f )					( (unsigned char)SHA_GET_FLAG_DATA( (f), SHA_BYTE_2,      SHA_BYTE_2_OFF) )
#define		byte2_low( f )					( (unsigned char)SHA_GET_FLAG_DATA( (f), SHA_BYTE_2_LOW,  SHA_BYTE_2_LOW_OFF) )
#define		byte2_high( f )					( (unsigned char)SHA_GET_FLAG_DATA( (f), SHA_BYTE_2_HIGH, SHA_BYTE_2_HIGH_OFF) )
#define		byte3_all( f )					( (unsigned char)SHA_GET_FLAG_DATA( (f), SHA_BYTE_3,      SHA_BYTE_3_OFF) )
#define		byte3_low( f )					( (unsigned char)SHA_GET_FLAG_DATA( (f), SHA_BYTE_3_LOW,  SHA_BYTE_3_LOW_OFF) )
#define		byte3_high( f )					( (unsigned char)SHA_GET_FLAG_DATA( (f), SHA_BYTE_3_HIGH, SHA_BYTE_3_HIGH_OFF) )
#define		byte4_all( f )					( (unsigned char)SHA_GET_FLAG_DATA( (f), SHA_BYTE_4,      SHA_BYTE_4_OFF) )
#define		byte4_low( f )					( (unsigned char)SHA_GET_FLAG_DATA( (f), SHA_BYTE_4_LOW,  SHA_BYTE_4_LOW_OFF) )
#define		byte4_high( f )					( (unsigned char)SHA_GET_FLAG_DATA( (f), SHA_BYTE_4_HIGH, SHA_BYTE_4_HIGH_OFF) )

#define		SHA_MASK_QUAT 			0x00000001
#define		SHA_MASK_TRANS			0x00000002
#define		SHA_MASK_SEQ			0x0000000c
#define		SHA_MASK_FLAGS			0x00000003

#define		hasQuat( f )					( ( f ) & SHA_MASK_QUAT  )
#define		hasTrans( f )					( ( f ) & SHA_MASK_TRANS )
#define		getSeq( f )						( ( f ) & SHA_MASK_SEQ   )
#define		getFlags( f )					( ( f ) & SHA_MASK_FLAGS )

#define		getCurFlags( fa, i )			(unsigned char)(( (((fa)[ (i)/(sizeof(long) * 2) ]) >> (((i)%(sizeof(long)*2))<<2))) & 0x0f)

	//---[ BASIC ANIMATION STRUCTURES - Used for loading ]---/

#pragma pack(push)
#pragma pack(1)

typedef struct
{
		//This is the 3 angles for the low order bits
	short int	xQuatL;		//These values are = to sin( (xQuat / 2^32 ) * .5 ) = component of a quat
	short int	yQuatL;		//All that needs to be done is to compute the 'w' component (aka 'r' component)
	short int	zQuatL;	
}sh_anim_ang;


typedef struct
{
	float		xPosL;		//This is the translation qty of the animation
	float		yPosL;
	float		zPosL;
	short int	xQuatL;
	short int	yQuatL;
	short int	zQuatL;
}sh_anim_pos_ang;


typedef struct
{
		//This is the 3 angles for the low order bits
	short int	xQuatL;		//These values are = to sin( (xQuat / 2^32 ) * .5 ) = component of a quat
	short int	yQuatL;		//All that needs to be done is to compute the 'w' component (aka 'r' component)
	short int	zQuatL;	
	//This is the 3 angles for the high order bits
	short int	xQuatH;		//These values are = to sin( (xQuat / 2^32 ) * .5 ) = component of a quat
	short int	yQuatH;		//All that needs to be done is to compute the 'w' component (aka 'r' component)
	short int	zQuatH;
}sh_anim_2ang;


typedef struct
{
		//This is the 3 pos and 3 angles for the low order bits
	float		xPosL;		//This is the translation qty of the animation
	float		yPosL;
	float		zPosL;
	short int	xQuatL;
	short int	yQuatL;
	short int	zQuatL;
		//This is the 3 pos and 3 angles for the high order bits
	float		xPosH;		//This is the translation qty of the animation
	float		yPosH;
	float		zPosH;
	short int	xQuatH;
	short int	yQuatH;
	short int	zQuatH;
}sh_anim_2pos_ang;

#pragma pack(pop)

//---[ ANIMATION STRUCTS AND CLASSES ]---/

class QuatAnim
{
public:
	QuatAnim( ){ }
	QuatAnim( quat & _cAnimAngle, vertex & _cAnimPos ):m_cAnimAngle( _cAnimAngle ),m_cAnimPos( _cAnimPos ){ }
	~QuatAnim( ){ }

	QuatAnim & operator=( const QuatAnim & rhs ){ if( &rhs != this ){ m_cAnimAngle = rhs.m_cAnimAngle; m_cAnimPos = rhs.m_cAnimPos; }return *this; }

	vertex * getPos( ){ return &m_cAnimPos; }
	quat * getAngle( ){ return &m_cAnimAngle; }
	matrix getMatrix( ){ matrix l_cMat; QTOM( &m_cAnimAngle, &l_cMat ); 
						l_cMat.mat[3]=m_cAnimPos.x; l_cMat.mat[7]=m_cAnimPos.y; l_cMat.mat[11]=m_cAnimPos.z; return l_cMat; }

//	vertex operator*( vertex & rhs );
//	QuatAnim operator*( QuatAnim & rhs);

	friend vertex operator*( QuatAnim & lhs, vertex & rhs );
	friend QuatAnim operator*( QuatAnim & lhs, QuatAnim & rhs );

	quat		m_cAnimAngle;
	vertex		m_cAnimPos;
};

class SH_AnimSet
{
public:
	SH_AnimSet( ){ SetNullAll( ); }
	SH_AnimSet( const SH_AnimSet & rhs ){ SetNullAll( ); operator=( rhs ); }
	~SH_AnimSet( ){ DeleteAllData( ); }
	SH_AnimSet & operator=( const SH_AnimSet & rhs );

	int LoadAnimSet( FILE * inFile, unsigned char *ps_mSeq, long _lNumSeq, bool sh2_load = false );	//Returns num matricies created for set, or zero for failure. Loads until another set w/ transform matricies is encountered
	//int LoadAnimSet( FILE * inFile, unsigned char *ps_mSeq, long _lNumSeq, bool sh2_load = false );	//Returns num matricies created for set, or zero for failure. Loads until another set w/ transform matricies is encountered
	void SetNullAll( )
	{
		m_pcMatrixSet = NULL; 
		//m_pcMatrixSet2 = NULL; 
		m_plFlags = NULL; m_pcQuatSet = NULL; m_lNumMatrix = 0; 
		//m_lNumMatrix2 = 0; 
		m_lNumFlags = 0;
	}

	void DeleteAllData( )
	{
		SAFEDELETE( m_pcMatrixSet );
		//SAFEDELETE( m_pcMatrixSet2 ); 
		SAFEDELETE( m_pcQuatSet ); SAFEDELETE( m_plFlags );
		SetNullAll( );
	}

	matrix * getMatrixArray( ){ return m_pcMatrixSet; }
	long getNumMatrix( ){ return m_lNumMatrix; }
	unsigned long * getFlagArray( ){ return m_plFlags; }
	long getNumFlags( ){ return m_lNumFlags; }
	quat convertToQuat( short int xVal, short int yVal, short int zVal );
	quat convertToQuatUnsigned( short int xVal, short int yVal, short int zVal );
	quat convertToQuatMat( short int xVal, short int yVal, short int zVal );

	matrix &operator[](int index);                // Mutator
	matrix  operator[](int index) const;          // Accessor

	unsigned long getFlag( long _lIndex );

	//--[ Member Variables ]--/
	long			m_lNumFlags;	//Count of flags to describe the animation frames
	unsigned long	*m_plFlags;		//Flags to determine number and type of animation data
	long			m_lNumMatrix;	//Number of matricies in this set
	//long			m_lNumMatrix2;	//Number of matricies in this set
	matrix			*m_pcMatrixSet;	//Matricies that have been transformed from animation data.
	//matrix			*m_pcMatrixSet2;//Matricies for the 2nd set from model
	QuatAnim		*m_pcQuatSet;	//Original Animation Data.
};
//===[ NOTE: CHANGE THIS TO TRACK THE ATTACHED MODELS ]===/

//---[ Forward Declaraction ]---/
class sh2_model;

class SH_Anim_Loader
{
public:
	SH_Anim_Loader( ){  }
	~SH_Anim_Loader( ){ ReleaseData( ); }

	void SetNullAll( )
	{
		//LogFile( ERROR_LOG, "SH_Anim_Loader::SetNullAll( ) - Start");
		m_pcAnimSet = NULL;
		m_pcDispQuat = NULL; 
		m_lRefCount = 0; 
		m_lNumSets = 0; 
		m_pcModel_SH3 = NULL; 
		m_pcModel_SH2 = NULL; 
		m_bAnimLoaded = false; 
		m_lModelIndex = -1; 
		m_cAnimFile[ 0 ] = '\0'; 
		m_lNumDispMat = 0; 
		m_lNumDispMat2 = 0;
		m_pcDispMat = NULL;
		m_pcDispMat2 = NULL;
		m_pcDispFinal = NULL; 
		m_pcDispInverse = NULL; 
		m_lCurFrame = 0;
		//LogFile( ERROR_LOG, "SH_Anim_Loader::SetNullAll( ) - End");
	}

	void ReleaseData( );
	void AttachModel( SH3_Actor *_pcModel ){ if( !_pcModel ) return; if( m_pcModel_SH2 ) ReleaseData( ); m_pcModel_SH3 = _pcModel; m_lRefCount++; }
	void AttachModel_SH2( sh2_model *_pcModel ){ if( !_pcModel ) return; if( m_pcModel_SH3 ) ReleaseData( ); m_pcModel_SH2 = _pcModel; m_vAttachedSH2.push_back( _pcModel ); m_lRefCount++; }
	void DetachModel( sh2_model *_pcModel = NULL )
	{ 
		if( _pcModel )
		{ 
			unsigned long k; 
			for( k = 0; k < m_vAttachedSH2.size( ); k++ )
			{ 
				if( _pcModel == m_vAttachedSH2[ k ] )
				{
					m_vAttachedSH2.erase( m_vAttachedSH2.begin( ) + k );
					m_lRefCount--; 
				}
			}
		}
		if( m_lRefCount == 0 )
			ReleaseData( );
	}
	long GetNumAnimFrames( ){ return m_lNumSets; }
	bool IsAnimLoaded( ){ return m_bAnimLoaded; }
	bool IsFrameValid( long _lFrame ){ if( _lFrame < 0 || _lFrame >= m_lNumSets ) return false; return true; }

	long LoadAnim( FILE *inFile, unsigned char *ps_mSeq, long _lNumSeq, long sectionSize, bool sh2_load = false );	//Returns # of Animation Frames, or 0 for failure
	long LoadAnim( char *_fName = NULL, long modelNum = -1, long startInd = 0 );	//Returns # of animation frames, or 0 for failure
	long LoadAnimSH2( char *_fName = NULL, long startInd = 0 );	//Returns # of animation frames, or 0 for failure. Var 'startInd' refers in this case to the index of the file within the directory
	bool LoadNextAnim( char * _fName = NULL);			//Returns true or false based on finding and loading another animation for the current model

	//---[ Export Functions ]---/
	void ExportSMD(  );
	void ExportSMDAnimation(  );
	void AddVerticies( SMD::SMD_Model & _Model );
	SMD::SMD_Vertex BuildSMDVertex( sh_vertex & _vert, long *_pSeq );
	SMD::SMD_Vertex BuildSMDVertex( sh_static_model_vertex & _vert, long _parent );

	//---[ Render Functions ]---/
	void RenderModel(  long modelPart = -1);
	void ApplyNextAnimFrame( );
	void ApplyAnimFrame( long _animNum );

//	SH_AnimSet &operator[](int index);                // Mutator
//	SH_AnimSet  operator[](int index) const;          // Accessor

	SH3_Actor		*m_pcModel_SH3;
	sh2_model		*m_pcModel_SH2;
	vector<sh2_model *>	m_vAttachedSH2;
	long			m_lNumSets;			//Number of m_pcAnimSet
	SH_AnimSet	*m_pcAnimSet;		//Each Animation Frame
	long			m_lNumDispMat;		//Number of matricies for current frame of animation
	long			m_lNumDispMat2;		//Number of matricies for current frame of animation
	matrix			*m_pcDispMat;		//Matricies created using 
	matrix			*m_pcDispMat2;		//Matricies created using 
	matrix			*m_pcDispFinal;
	matrix			*m_pcDispInverse;
	QuatAnim		*m_pcDispQuat;
	bool			m_bAnimLoaded;		//Is there animation data in memory?
	long			m_lModelIndex;		//Index in arc file of current animation
	char			m_cAnimFile[256];	//File that current animation was loaded from (only tracked in different from the SH3_Actor->modelFilename, or the m_pcModel_SH3 is NULL
	long			m_lCurFrame;		//Current animation frame, in range [0, m_lNumSets -1 ]
	long			m_lRefCount;		//Count of models currently attached to this animation set
};

#endif /*__SH_MODEL_ANIM_H__*/