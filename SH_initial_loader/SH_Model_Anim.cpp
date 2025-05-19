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
#include "SH3_Loader.h"
#include "SH_Model_Anim.h"
#include "SH2_Model.h"
#include "debugDrawing.h"
#include "utils.h"

#include <SMD_Animation_Frame.h>
#include <SMD_vertex.h>
#include <SMD_Triangle.h>
#include <SMD_Model.h>

using namespace SMD;

extern TexMgr textureMgr;
extern bool debugMode;
extern bool animDebugMode;
extern bool dumpModel;
extern bool displayMatrix;

extern bool sh2_mode;

extern bool sh2_run;

extern int testModeInt;

//---------------[ G E N E R A L   F U N C T I O N S ]------------//
void debugAA( sh_anim_ang *h, long offset )
{
	//LogFile( ERROR_LOG, "\n]----+--[ S i l e n t   H i l l   A n i m   A n g l e s (size: %ld  offset: %ld ) ]--+----[",sizeof(sh_anim_ang),offset);
	//debugShort( h->xQuatL );
	//debugShort( h->yQuatL );
	//debugShort( h->zQuatL ); 

	LogFile( ERROR_LOG,"  Ang    -Flag %2d  Ang: %d\t%d\t%d",(unsigned char)offset,h->xQuatL,h->yQuatL,h->zQuatL);
};


void debugAPA( sh_anim_pos_ang *h, long offset )
{
	//LogFile( ERROR_LOG, "\n]----+--[ S i l e n t   H i l l   A n i m   P o s   A n g l e s (size: %ld  offset: %ld ) ]--+----[",sizeof(sh_anim_pos_ang),offset);
	//debugFloat( h->xPosL );
	//debugFloat( h->yPosL );
	//debugFloat( h->zPosL );
	//debugShort( h->xQuatL );
	//debugShort( h->yQuatL );
	//debugShort( h->zQuatL );

	LogFile( ERROR_LOG,"  PosAng -Flag %2d   pos: %f\t%f\t%f\n                    Ang: %d\t%d\t%d",
		(unsigned char)offset,h->xPosL,h->yPosL,h->zPosL,h->xQuatL,h->yQuatL,h->zQuatL);
};

/*

vertex QuatAnim::operator*( vertex & rhs )
{
	return vertex( this->m_cAnimAngle.rotateVertex( rhs ) + this->m_cAnimPos );
}

QuatAnim QuatAnim::operator*( QuatAnim & rhs)
{
	QuatAnim temp;

	temp.m_cAnimPos = this->m_cAnimAngle.rotateVertex( rhs.m_cAnimPos ) + this->m_cAnimPos;
	temp.m_cAnimAngle = this->m_cAnimAngle * rhs.m_cAnimAngle;
	return QuatAnim( temp.m_cAnimAngle, temp.m_cAnimPos );
}
*/
vertex operator*( QuatAnim & lhs, vertex & rhs )
{
	return vertex( lhs.m_cAnimAngle.rotateVertex( rhs ) + lhs.m_cAnimPos );
}

QuatAnim operator*( QuatAnim & lhs, QuatAnim & rhs )
{
	QuatAnim temp;
	quat rt=rhs.m_cAnimAngle,lt=lhs.m_cAnimAngle;

	QUnit( &rt );
	QUnit( &lt );
temp.m_cAnimPos = lt.rotateVertex( rhs.m_cAnimPos );
//temp.m_cAnimPos = rt.rotateVertex( rhs.m_cAnimPos );
	temp.m_cAnimPos = temp.m_cAnimPos + lhs.m_cAnimPos;
	//temp.m_cAnimPos = lhs.m_cAnimAngle.rotateVertex( rhs.m_cAnimPos ) + lhs.m_cAnimPos;
temp.m_cAnimAngle = lt * rt;
//temp.m_cAnimAngle = rt * lt;
	//temp.m_cAnimAngle = lhs.m_cAnimAngle * rhs.m_cAnimAngle;

	return QuatAnim( temp.m_cAnimAngle, temp.m_cAnimPos );
}



SH_AnimSet & SH_AnimSet::operator=( const SH_AnimSet & rhs )
{
	if( &rhs != this )
	{
//LogFile(ERROR_LOG,"STUPID PIECE OF SHIT - m_plFlags = %08x\tm_pcMatrixSet = %08x",m_plFlags,m_pcMatrixSet);
		DeleteAllData( );
		
		m_lNumFlags = rhs.m_lNumFlags;
		m_lNumMatrix= rhs.m_lNumMatrix;
//		m_lNumMatrix2= rhs.m_lNumMatrix2;

		m_pcMatrixSet = new matrix[ rhs.m_lNumMatrix ];
//		m_pcMatrixSet2 = new matrix[ rhs.m_lNumMatrix2 ];
		m_pcQuatSet = new QuatAnim[ rhs.m_lNumMatrix ];
		m_plFlags = new unsigned long[ rhs.m_lNumFlags ];
//LogFile(ERROR_LOG,"STUPID PIECE OF SHIT - #Flag = %ld\t#Mat = %ld",m_lNumFlags,m_lNumMatrix);
		memcpy( (void *)m_plFlags, (void *)rhs.m_plFlags, sizeof( unsigned long ) * rhs.m_lNumFlags );
		memcpy( (void *)m_pcMatrixSet, (void *)rhs.m_pcMatrixSet, sizeof( matrix ) * rhs.m_lNumMatrix );
//		memcpy( (void *)m_pcMatrixSet2, (void *)rhs.m_pcMatrixSet2, sizeof( matrix ) * rhs.m_lNumMatrix2 );
		memcpy( (void *)m_pcQuatSet, (void *)rhs.m_pcQuatSet, sizeof( QuatAnim ) * rhs.m_lNumMatrix );
	}
	return *this; 
}






matrix &SH_AnimSet::operator[](int index)				// Mutator
{
	int place= ( index >= m_lNumMatrix )?m_lNumMatrix:index;
	if(place < 0)
		place = 0;
	return m_pcMatrixSet[place];
}


matrix  SH_AnimSet::operator[](int index) const
{
	int place= ( index >= m_lNumMatrix )?m_lNumMatrix:index;
	if(place < 0)
		place = 0;
	return m_pcMatrixSet[place];
}


long GetFileInfo( char *filename )
{
	long longSet[50];
	FILE *infile;
	int k;
	long nextID;
	arc_index_data indexData;

	//if( debugMode )
		LogFile( ERROR_LOG, "GetFileInfo(  ) ---------------[ S T A R T ]------------");


	//---[ START READING IN DATA TO FIND ANIMATION ]---/

	if(( infile = fopen( filename, "rb" ))== NULL )
	{
		LogFile(ERROR_LOG,"GetFileInfo( ) - ERROR: Unable to open file '%s': %s",filename,strerror(errno));
		return 0;
	}

	if(!loadArcIndex(infile,&indexData))
	{
		LogFile(ERROR_LOG,"GetFileInfo( ) - ERROR: Unable to read index from file '%s'",filename);
		fclose( infile );
		return 0;
	}

	//---[ FIND ANIMATION FOR THE MODEL ]---/

	for( k = 0; k < indexData.numIndex; k++ )
	{
		if( k == indexData.numIndex - 1 )
			nextID = indexData.index[ k ].q1;
		else
			nextID = indexData.index[ k+1 ].q1;

		fseek( infile, indexData.index[k].offset,SEEK_SET);

		if(fread( longSet, 50, sizeof(long),infile) > 0 )
		{

			if( longSet[0] < 0 )
				LogFile( ERROR_LOG,"%s\t\t%ld - TEX\t       \tDiff: %ld",filename,k,nextID-indexData.index[ k ].q1);
			else if( longSet[ 0] == 0 )
			{
		//		LogFile(ERROR_LOG,"TEST: longSet[6]==%ld\t /4 = %ld",longSet[5],longSet[5]/4);
				if( longSet[ 1 ] > 0 && longSet[ 5 ] / 4 >= 0 && longSet[ 5 ] / 4 < 50 && longSet[ longSet[5]/4 ]== -65533 )
					LogFile( ERROR_LOG,"%s\t\t%ld - MDL\tid: %ld\tDiff: %ld",filename,k,longSet[1],nextID-indexData.index[ k ].q1);
				else
					LogFile( ERROR_LOG,"%s\t\t%ld - KG1\tid: %ld\tDiff: %ld",filename,k,longSet[1],nextID-indexData.index[ k ].q1);
			}
			else
				LogFile( ERROR_LOG,"%s\t\t%ld - ANM\tid: %ld\tDiff: %ld",filename,k,longSet[0],nextID-indexData.index[ k ].q1);
		}
		else
			LogFile(ERROR_LOG,"GetFileInfo( ) - ERROR: Unable to read data at offset %ld : %s",
				indexData.index[k].offset,strerror(errno));
	}

	fclose( infile );
	if( debugMode )
		LogFile( ERROR_LOG, "GetFileInfo(  ) -----------------[ E N D ]--------------");

	return 0;
}



void SH_Anim_Loader::ReleaseData( )
{
	unsigned long k;
	m_lRefCount = 0;
	if( debugMode )
		LogFile( ERROR_LOG, "SH_Anim_Loader::ReleaseData( ) - Start");
	for( k = 0; k < m_vAttachedSH2.size( ); k++ )
		m_vAttachedSH2[ k ]->DetachFromAnim( );
	if( debugMode )LogFile( ERROR_LOG, "SH_Anim_Loader::ReleaseData( ) - Completed detaching models");
	if( m_vAttachedSH2.size( ) ) 
		LogFile( ERROR_LOG, "SH_Anim_Loader::ReleaseData( ) - ERROR: Unable to detach all SH2 Models: %ld left",m_vAttachedSH2.size( ) );

	m_vAttachedSH2.clear( );
	if( debugMode )LogFile( ERROR_LOG, "SH_Anim_Loader::ReleaseData( ) - Completed clearing Attached models");
	m_pcModel_SH3 = NULL;
	m_pcModel_SH2 = NULL;
	SAFEDELETE( m_pcDispQuat );
	SAFEDELETE( m_pcAnimSet );
	if( debugMode )LogFile( ERROR_LOG, "SH_Anim_Loader::ReleaseData( ) - Deleted loaded animations");
	m_lNumSets = 0;
	m_bAnimLoaded = false;
	m_lModelIndex = -1;
	m_cAnimFile[ 0 ] = '\0';
	m_lNumDispMat = 0;
	m_lNumDispMat2 = 0;
	SAFEDELETE( m_pcDispMat );
	SAFEDELETE( m_pcDispMat2 );
	SAFEDELETE( m_pcDispFinal );
	SAFEDELETE( m_pcDispInverse );
	if( debugMode )LogFile( ERROR_LOG, "SH_Anim_Loader::ReleaseData( ) - Deleted display data");
	m_lCurFrame = 0; 
	SetNullAll( );
}

long SH_Anim_Loader::LoadAnim( char *_fName, long modelNum, long startInd )
{
	long firstLong;
	FILE *infile;
	int k;
	char filename[256];
	long lModelNum;
	arc_index_data indexData;
	long t_lModelIndex = 0;
	long startIndex;

//	debugMode = true;
//	LogFile( ERROR_LOG,"HERE");
//	LogFile( ERROR_LOG,"Debug Mode is: %s",(debugMode)?"TRUE":"False");
	if( debugMode )
		LogFile( ERROR_LOG, "SH_AnimSet::LoadAnim(  ) ---------------[ S T A R T ]------------");


	//---[ V A L I D A T E   D A T A    P A S S E D   F U N C T I O N ]---/
	if( !m_pcModel_SH3 )
	{
		LogFile( ERROR_LOG, "SH_AnimSet::LoadAnim( ) - ERROR: No SH3 Model is attached!!!");
		return 0;
	}

	if( m_pcModel_SH3 && _fName == NULL )
		strcpy( filename, m_pcModel_SH3->modelFilename );
	else if( _fName )
		strcpy( filename, _fName );
	else
	{
		LogFile( ERROR_LOG, "SH_Anim_Loader::LoadAnim( ) - No filename was passed, and no model is attached");
		return 0;
	}
//LogFile( ERROR_LOG,"HERE");
	if( !m_pcModel_SH3 && modelNum == -1 )
	{
		LogFile( ERROR_LOG, "SH_Anim_Loader::LoadAnim( ) - No model is attached, and a model ID was not given");
		return 0;
	}
	else if( modelNum == -1 )
		lModelNum = m_pcModel_SH3->m_mBaseHeader.modelID;
	else
		lModelNum = modelNum;

	if( !m_pcModel_SH3 && lModelNum == 532 )
	{
		LogFile( ERROR_LOG, "SH_Anim_Loader::LoadAnim( ) - ERROR: Special Case - Cannot Load Anim Without Being Attached (ID# %ld)",lModelNum);
		return 0;
	}


	//---[ START READING IN DATA TO FIND ANIMATION ]---/

	if(( infile = fopen( filename, "rb" ))== NULL )
	{
		LogFile(ERROR_LOG,"SH_Anim_Loader::LoadAnim( ) - ERROR: Unable to open file '%s': %s",filename,strerror(errno));
		return 0;
	}

	if(!loadArcIndex(infile,&indexData))
	{
		LogFile(ERROR_LOG,"SH_Anim_Loader::LoadAnim( ) - ERROR: Unable to read index from file '%s'",filename);
		fclose( infile );
		return 0;
	}

	if( startInd > 0 )
	{
		if( startInd + 1 < indexData.numIndex )
			startIndex = startInd + 1;
		else
			startIndex = 0;
	}
	else
		startIndex = 0;

	//---[ FIND ANIMATION FOR THE MODEL ]---/

	for( k = startIndex; k < indexData.numIndex; k++ )
	{
		fseek( infile, indexData.index[k].offset,SEEK_SET);

		if(fread( &firstLong, 1, sizeof(long),infile) == sizeof(long))
		{
			if( firstLong != 0 && firstLong != -1 && lModelNum == firstLong )
			{
				long numLoaded;

				//---[ Save Temporary Data in case it doesn't load ]---/
				t_lModelIndex = k;
				//fseek( infile, indexData.index[k].offset,SEEK_SET);
				if( debugMode || animDebugMode )
				{
					LogFile( TEST_LOG," SH_AnimSet::LoadAnim - Looking for animation in '%s' for model %ld at index %ld",
						filename,lModelNum,t_lModelIndex);
				}

				if( (numLoaded = LoadAnim( infile, (unsigned char*)m_pcModel_SH3->mSeq1, 
								m_pcModel_SH3->m_mDataHeader.numMatSet1, indexData.index[k].size - sizeof(long), false )))
				{
					if( debugMode || animDebugMode )
						LogFile( ERROR_LOG,"SH_AnimSet::LoadAnim(  ) - DEBUG: Loaded %ld sets for model %ld at index %ld",
							numLoaded, lModelNum,t_lModelIndex);
					m_lModelIndex = t_lModelIndex;
					m_lCurFrame = 0;
					m_lNumSets = numLoaded;
					m_bAnimLoaded = true;
					if( _fName )
						strcpy( m_cAnimFile, _fName );
					fclose( infile );

					ApplyAnimFrame( 0 );

					if( debugMode )
						LogFile( ERROR_LOG, "SH_AnimSet::LoadAnim(  ) -----------------[ E N D ]--------------");

					return numLoaded;
				}
				else
					LogFile(ERROR_LOG,"SH_Anim_Loader::LoadAnim( ) - ERROR: Unable to load animation data at offset %ld",indexData.index[k]);
			}

		}
		else
			LogFile(ERROR_LOG,"SH_Anim_Loader::LoadAnim( ) - ERROR: Unable to read data at offset %ld : %s",
				indexData.index[k].offset,strerror(errno));
	}

	m_lCurFrame = 0;
	fclose( infile );
	if( debugMode )
		LogFile( ERROR_LOG, "SH_AnimSet::LoadAnim(  ) - ERROR: Didn't find animations for model %ld -----------------[ E N D ]--------------",lModelNum);

	return 0;
}


long SH_Anim_Loader::LoadAnimSH2( char *_fName, long startInd )
{
//	long firstLong;
	FILE *infile;
//	int k;
	char filename[512];
//	long lModelNum;
	long t_lModelIndex = 0;
	long startIndex;

//	debugMode = true;
//	LogFile( ERROR_LOG,"HERE");
//	LogFile( ERROR_LOG,"Debug Mode is: %s",(debugMode)?"TRUE":"False");
	if( debugMode )
		LogFile( ERROR_LOG, "SH_AnimSet::LoadAnimSH2(  ) ---------------[ S T A R T ]------------");


	//---[ V A L I D A T E   D A T A    P A S S E D   F U N C T I O N ]---/
	if( !m_pcModel_SH2 )
	{
		LogFile( ERROR_LOG, "SH_AnimSet::LoadAnimSH2( ) - ERROR: No SH2 Model is attached!!!");
		return 0;
	}

	if( m_pcModel_SH2 && _fName == NULL )
	{
		char *l_pcEndPoint;
		strcpy( filename, m_pcModel_SH2->m_pcModelFilename );
		l_pcEndPoint = strrchr( filename, '.' );
		if( l_pcEndPoint )
			sprintf( l_pcEndPoint, ".anm" );
		else
			sprintf( filename, "%s.anm", m_pcModel_SH2 );

		LogFile( TEST_LOG, "CHECK:  The generated filename is [%s]",filename );
	}
	else if( _fName )
		strcpy( filename, _fName );
	else
	{
		LogFile( ERROR_LOG, "SH_Anim_Loader::LoadAnimSH2( ) - No filename was passed, and no model is attached");
		return 0;
	}
//LogFile( ERROR_LOG,"HERE");

	//---[ START READING IN DATA TO FIND ANIMATION ]---/

//	if( m_bAnimLoaded )
//		ReleaseData( );

	if(( infile = fopen( filename, "rb" ))== NULL )
	{
		LogFile(ERROR_LOG,"SH_Anim_Loader::LoadAnimSH2( ) - ERROR: Unable to open file '%s': %s",filename,strerror(errno));
		return 0;
	}


	if( startInd > 0 )
	{
		LogFile( ERROR_LOG, "\n\n########################################################\n\nN O   C O D E   F O R   D E A L I N G   W I T H   O T H E R   N A M E D   F I L E S   Y E T\n\n##########################################################\n");
		startIndex = 0;
	}
	else
		startIndex = 0;

	//---[ FIND ANIMATION FOR THE MODEL ]---/

	long numLoaded;

	if( debugMode || animDebugMode )
		LogFile( TEST_LOG," SH_AnimSet::LoadAnimSH2 - Looking for animation in '%s'", filename);
	
	if( (numLoaded = LoadAnim(	infile, m_pcModel_SH2->m_cHeaderData.mSeq1, 
								m_pcModel_SH2->m_sModelHeader.numMatSet1, getFileSize( filename ), true )))
	{
		if( debugMode || animDebugMode )
			LogFile( ERROR_LOG,"SH_AnimSet::LoadAnimSH2(  ) - DEBUG: Loaded %ld sets for model '%s'",numLoaded, filename);
		m_lModelIndex = t_lModelIndex;
		m_lCurFrame = 0;
		m_lNumSets = numLoaded;
		m_bAnimLoaded = true;
		if( _fName )
			strcpy( m_cAnimFile, _fName );
		fclose( infile );
		if( debugMode )
			LogFile( ERROR_LOG, "SH_AnimSet::LoadAnimSH2(  ) -----------------[ E N D ]--------------");

		return numLoaded;
	}
	else
		LogFile( ERROR_LOG, "SH_AnimSet::LoadAnimSH2(  ) - ERROR: Didn't find animations for model '%s'   -----------------[ E N D ]--------------",filename);

	m_lCurFrame = 0;
	fclose( infile );
	if( debugMode )
		LogFile( ERROR_LOG, "SH_AnimSet::LoadAnimSH2(  ) - ERROR: Didn't find animations for model '%s' -----------------[ E N D ]--------------",filename);

	return 0;
}



long SH_Anim_Loader::LoadAnim( FILE *inFile, unsigned char *ps_mSeq, long _lNumSeq, long sectionSize, bool sh2_load )
{
	unsigned long k;
	vector<SH_AnimSet>	animVec;
	SH_AnimSet	curAnim;
	long lStartOffset = ftell( inFile );
	long lCurOffset  = lStartOffset;
	long lProjectedEndOffset = lStartOffset + sectionSize;
	long lastReadMat;
	long lModelNum;

	if( m_pcModel_SH3 )
		lModelNum = m_pcModel_SH3->m_mBaseHeader.modelID;
	else if( m_pcModel_SH2 )
		lModelNum = m_pcModel_SH2->m_sModelBaseHeader.modelID;
	else
		lModelNum = -1;

	while( lCurOffset < lProjectedEndOffset )
	{	
		lastReadMat = curAnim.LoadAnimSet( inFile, ps_mSeq, _lNumSeq, sh2_load );

		if( lastReadMat > 0 )
		{
//			LogFile(ERROR_LOG,"STUPID PIECE OF SHIT - numread = %d",lastReadMat);
			animVec.push_back( curAnim );
//			LogFile(ERROR_LOG,"STUPID PIECE OF SHIT - Section Size = %ld",sectionSize);
			lCurOffset = ftell( inFile );
			
			if( debugMode )
				LogFile( ERROR_LOG, "SH_Anim_Loader::LoadAnim( ) - Read %ld matricies for %ld anim frame.  %ld of %ld bytes read",lastReadMat, animVec.size( ), lCurOffset - lStartOffset, sectionSize );
			if( animDebugMode )
				LogFile(  TEST_LOG, "SH_Anim_Loader::LoadAnim( ) - Read %ld matricies for %ld anim frame.  %ld of %ld bytes read",lastReadMat, animVec.size( ), lCurOffset - lStartOffset, sectionSize );
		}
		else
		{
			lCurOffset = ftell( inFile );
			LogFile( ERROR_LOG, "SH_Anim_Loader::LoadAnim( ) - ERROR: Read NO matricies for %ld anim frame.  %ld of %ld bytes read", animVec.size( ), lCurOffset - lStartOffset, sectionSize );
			if( !animVec.size( ) )
			{
				LogFile( ERROR_LOG, "\t\tExiting...");
				return 0;
			}

			SAFEDELETE( m_pcAnimSet );
			m_lNumSets = animVec.size( );
			m_pcAnimSet = new SH_AnimSet[ m_lNumSets ];

			for( k = 0; k < animVec.size( ); k++ )
				m_pcAnimSet[ k ] = animVec[ k ];
			m_bAnimLoaded = true;

			return m_lNumSets;
		}
	}


	if( !animVec.size( ) )
	{
		LogFile( ERROR_LOG, "SH_Anim_Loader::LoadAnim( ) - ERROR: Read NO matricies for ALL anim frame.  %ld of %ld bytes read", lCurOffset - lStartOffset, sectionSize );
		LogFile( ERROR_LOG, "\t\tExiting...");
		return 0;
	}

	//---[ SET UP CLASS DATA FOR DISPLAY ]---/

	SAFEDELETE( m_pcAnimSet );
	m_lNumSets = animVec.size( );
	m_pcAnimSet = new SH_AnimSet[ m_lNumSets ];

	for( k = 0; k < animVec.size( ); k++ )
		m_pcAnimSet[ k ] = animVec[ k ];
	m_bAnimLoaded = true;
	m_lCurFrame = 0;

	return m_lNumSets;
}


bool SH_Anim_Loader::LoadNextAnim( char * _fName )
{
	if( ! LoadAnim( _fName, -1, m_lModelIndex ) )
	{
		if( m_lModelIndex > 0 )
		{
			m_lModelIndex = 0;
			return ( LoadAnim( _fName, -1, m_lModelIndex ) != 0);
		}
		else
			return false;
	}
	return true;
}

//################################################### S  M  D    W  R  I  T  E    T  O    F  I  L  E ################

void SH_Anim_Loader::ExportSMDAnimation(  )
{
		int k;
	int j;

	SMD::SMD_Animation_Frame	l_AnimFrame;
	SMD::SMD_Triangle			l_Triangle;
	SMD::SMD_Vertex				l_Vertex;
	SMD::SMD_Model				l_Model;


	if( !m_pcModel_SH3 )
	{
			LogFile( ERROR_LOG, "SH_Anim_Loader::LoadAnim( ) - ERROR: No Model is Attached to this animation to render" );
			return;
	}

	if( !m_pcAnimSet )
	{
			LogFile( ERROR_LOG, "SH_Anim_Loader::LoadAnim( ) - ERROR: No Animation is loaded for this Model to render" );
			return;
	}

//	matSet1 = m_pcModel_SH3->matSet1;
//	matSet2 = m_pcModel_SH3->matSet2;
//	m_pmPrimitive = m_pcModel_SH3 ->m_pmPrimitive;
	
	//---[ A D D   N O D E S ]---/

	//--[ Node Sequence 1 ]--/
	for( k = 0; k < this->m_pcModel_SH3->m_mDataHeader.numMatSet1; k++ )
	{
		l_Model.AddNode( this->m_pcModel_SH3->mSeq1[ k ] );
	}

	//--[ Node Sequence 2 ]--/
	for( k = 0; k < this->m_pcModel_SH3->m_mDataHeader.numMatSet2; k++ )
	{
		l_Model.AddNode( this->m_pcModel_SH3->mSeq2[ k*2 + 1 ] );
	}

	//---[ G O   T H R O U G H   A L L   A N I M A T I O N S ]---/
	//---[ Generate The Animation Data ]---/
	for( j = 0; j < this->m_lNumSets; j++ )
	{
		//--[ Clear Animation Frame For Next One ]--/
		l_AnimFrame.DeleteData();

		long l_lNumDispMat = m_pcAnimSet[ m_lCurFrame ].m_lNumMatrix;

		if( l_lNumDispMat < m_pcModel_SH3->m_mDataHeader.numMatSet1 )
			l_lNumDispMat = m_pcModel_SH3->m_mDataHeader.numMatSet1;


		//--[ Generate Next Frame ]--/
		this->ApplyAnimFrame( j );

		matrix	*l_pcDispInverse	=	new matrix[ this->m_pcModel_SH3->m_mDataHeader.numMatSet1 ];
		matrix	*l_pcForwardMat		=	new matrix[ this->m_pcModel_SH3->m_mDataHeader.numMatSet1 ];
		matrix	*l_pcAnimMat		=	new matrix[ this->m_pcModel_SH3->m_mDataHeader.numMatSet1 ];

		//---[ Init Matrix Array For Animation Matricies that Don't have enough Mats ]---/
		l_pcAnimMat = new matrix[ l_lNumDispMat ];

//		for( k = 0; k < l_lNumDispMat; k++  )
//			l_pcAnimMat[ k ].identity( );
		for( k = 0; k < m_lNumDispMat; k++ )
		{
			l_pcAnimMat[ k ] = m_pcDispMat[ k ];	//m_pcAnimSet[ m_lCurFrame ][ k ];
		}

		//----[ GENERATE INVERSE MATRICIES ]----/
		for( k = 0; k < m_lNumDispMat; k++ )	//m_pcAnimSet[ m_lCurFrame ].m_lNumMatrix; k++ )
		{
			l_pcDispInverse[ k ] = l_pcAnimMat[ k ];	//this->m_pcDispMat[ k ];	//m_pcModel_SH3->matSet1[ k ];
			l_pcDispInverse[ k ].Inverse( );
		}
		//----[ MOVE JOINT POSITION BACK TO ORIGIN AND MAKE IT'S RELATIVE OFFSET ]----/
		for( k = 0; k < m_lNumDispMat; k++ )
		{	
			//---[ MAKE JOINT POSITION RELATIVE OFFSET TO ORIGIN ]---/
			if( m_pcModel_SH3->mSeq1[ k ] != -1 )
				l_pcForwardMat[ k ] = l_pcDispInverse[ m_pcModel_SH3->mSeq1[ k ] ] * l_pcAnimMat[ k ];	//m_pcDispMat[ k ];	//m_pcModel_SH3->matSet1[ k ];
			else
				l_pcForwardMat[ k ] = l_pcAnimMat[ k ];	//m_pcDispMat[ k ];	//m_pcModel_SH3->matSet1[ k ];
				//l_pcForwardMat[ k ].identity();
		
			//if( m_pcAnimSet[ m_lCurFrame ].getFlag( k ) != 0 )
			//	l_pcForwardMat[ k ].clearRot( );
		}

		//---[ Add Matricies To Model ]---/

		for( k = 0; k < this->m_pcModel_SH3->m_mDataHeader.numMatSet1; k++ )
		{
			l_AnimFrame.AddTransform( l_pcForwardMat[ k ] );
			//matrix tempMat = l_pcForwardMat[ k ] * l_pcAnimMat[ k ];
			//l_AnimFrame.AddTransform( tempMat );
			//l_AnimFrame.AddTransform( this->m_pcDispMat[ k ] );
		}

		for( k = 0; k < this->m_pcModel_SH3->m_mDataHeader.numMatSet2; k++ )
		{
			l_AnimFrame.AddTransform(  this->m_pcModel_SH3->matSet2[ k ] );
			//l_AnimFrame.AddTransform( this->m_pcDispInverse[ k ] );
		}

		delete [] l_pcForwardMat;
		delete [] l_pcDispInverse;
		delete [] l_pcAnimMat;


		l_Model.AddAnimationFrame( l_AnimFrame );

	}

	std::fstream  l_OutputFile;
	char l_OutputModelName[ 256 ];

	sprintf( l_OutputModelName, "%s_%ld_anim.smd",baseName( mstring(this->m_pcModel_SH3->modelFilename) ).c_str(), this->m_pcModel_SH3->baseOffset );

	l_OutputFile.open( l_OutputModelName, ios::out | ios::trunc );
	
	l_OutputFile << l_Model;

	l_OutputFile.close( );
}


//######################################## E  X  P  O  R  T     S  M  D     M  E  T  H  O  D  S ################################################

void SH_Anim_Loader::AddVerticies( SMD::SMD_Model & _Model )
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

	this->ApplyAnimFrame( 0 );

	for( j = 0; j < this->m_pcModel_SH3->numPrimitives; j++ )
	{
		//---[ Initialize Dynamic Data ]---/

		dispVerts = new vertex4f[this->m_pcModel_SH3->m_pmPrimitive[j].vertHeader.numVerts];
		dispNorms = new vertex  [this->m_pcModel_SH3->m_pmPrimitive[j].vertHeader.numVerts];

		matArray = new matrix[ this->m_pcModel_SH3->m_pmPrimitive[j].vertHeader.numSeq1 + this->m_pcModel_SH3->m_pmPrimitive[j].vertHeader.numSeq2];

		primSeq = new long[ this->m_pcModel_SH3->m_pmPrimitive[j].vertHeader.numSeq1 + this->m_pcModel_SH3->m_pmPrimitive[j].vertHeader.numSeq2 ];

		//---[ Set Node Number And Generate Matrix Arrays For Current Primitive ]---/

		for( k = 0; k < this->m_pcModel_SH3->m_pmPrimitive[j].vertHeader.numSeq1; k ++ )
		{
			matArray[k] = m_pcDispMat[this->m_pcModel_SH3->m_pmPrimitive[j].seqData1[k]];
			primSeq[ k ] = this->m_pcModel_SH3->m_pmPrimitive[j].seqData1[k];
		}
		for( k = this->m_pcModel_SH3->m_pmPrimitive[j].vertHeader.numSeq1; k < this->m_pcModel_SH3->m_pmPrimitive[j].vertHeader.numSeq1 + this->m_pcModel_SH3->m_pmPrimitive[j].vertHeader.numSeq2; k++ )
		{
			matArray[k] = m_pcDispMat[this->m_pcModel_SH3->mSeq2[(this->m_pcModel_SH3->m_pmPrimitive[j].seqData2[k-this->m_pcModel_SH3->m_pmPrimitive[j].vertHeader.numSeq1])*2+1]] * this->m_pcModel_SH3->matSet2[this->m_pcModel_SH3->m_pmPrimitive[j].seqData2[k-this->m_pcModel_SH3->m_pmPrimitive[j].vertHeader.numSeq1]];
			primSeq[ k ] = this->m_pcModel_SH3->m_pmPrimitive[j].seqData2[ k - this->m_pcModel_SH3->m_pmPrimitive[j].vertHeader.numSeq1 ] + this->m_pcModel_SH3->m_mDataHeader.numMatSet1;
		}

		//---[ Transform All Verts ]---/
		for( k = 0; k < this->m_pcModel_SH3->m_pmPrimitive[j].vertHeader.numVerts; k++ )
		{
			if( this->m_pcModel_SH3->m_pmPrimitive[j].vertHeader.numSeq1 + this->m_pcModel_SH3->m_pmPrimitive[j].vertHeader.numSeq2 > 1 )
			{
				if( this->m_pcModel_SH3->m_pmPrimitive[j].verts == NULL && this->m_pcModel_SH3->m_pmPrimitive[j].altVerts != NULL )
				{
					LogFile(TEST_LOG,"SH3_Actor::AddVertices - TERMINAL ERROR: Can't render prim #%d - Small size verts have multi-sequence matricies w/o selector data",j);
					break;
				}

				float	r0w = 1.0f - ( this->m_pcModel_SH3->m_pmPrimitive[j].verts[k].v1.x + this->m_pcModel_SH3->m_pmPrimitive[j].verts[k].v1.y + this->m_pcModel_SH3->m_pmPrimitive[j].verts[k].v1.z );
				
				matrix temp = matArray[this->m_pcModel_SH3->m_pmPrimitive[j].verts[k].color.b];
				matrix temp2= matArray[this->m_pcModel_SH3->m_pmPrimitive[j].verts[k].color.g];
				matrix temp3= matArray[this->m_pcModel_SH3->m_pmPrimitive[j].verts[k].color.r];
				matrix temp4= matArray[this->m_pcModel_SH3->m_pmPrimitive[j].verts[k].color.a];

				//---[ Build Transformed Vertex ]---/
				dispVerts[ k ]=	(temp * this->m_pcModel_SH3->m_pmPrimitive[j].verts[k].vert) * this->m_pcModel_SH3->m_pmPrimitive[j].verts[k].v1.x +
								(temp2 * this->m_pcModel_SH3->m_pmPrimitive[j].verts[k].vert) * this->m_pcModel_SH3->m_pmPrimitive[j].verts[k].v1.y +
								(temp3 * this->m_pcModel_SH3->m_pmPrimitive[j].verts[k].vert) * this->m_pcModel_SH3->m_pmPrimitive[j].verts[k].v1.z +
								(temp4 * this->m_pcModel_SH3->m_pmPrimitive[j].verts[k].vert) * r0w;

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

				dispNorms[ k ]=	(temp * this->m_pcModel_SH3->m_pmPrimitive[j].verts[k].normal) * this->m_pcModel_SH3->m_pmPrimitive[j].verts[k].v1.x +
								(temp2 * this->m_pcModel_SH3->m_pmPrimitive[j].verts[k].normal) * this->m_pcModel_SH3->m_pmPrimitive[j].verts[k].v1.y +
								(temp3 * this->m_pcModel_SH3->m_pmPrimitive[j].verts[k].normal) * this->m_pcModel_SH3->m_pmPrimitive[j].verts[k].v1.z +
								(temp4 * this->m_pcModel_SH3->m_pmPrimitive[j].verts[k].normal) * r0w;
			}
			else //---[ THERE IS ONLY ONE MATRIX AND SMALL SIZE VERTS ]---/
			{
				matrix temp = matArray[ 0 ];

				if( this->m_pcModel_SH3->m_pmPrimitive[j].verts == NULL && this->m_pcModel_SH3->m_pmPrimitive[j].altVerts != NULL )
				{						
					dispVerts[k] = temp * this->m_pcModel_SH3->m_pmPrimitive[j].altVerts[k].vert;
					dispVerts[k].w = 1.0f;
					temp.clearTrans();
					temp.Inverse();
					dispNorms[k] = temp * this->m_pcModel_SH3->m_pmPrimitive[j].altVerts[k].norm;
				}
				else if( this->m_pcModel_SH3->m_pmPrimitive[j].verts != NULL && this->m_pcModel_SH3->m_pmPrimitive[j].altVerts == NULL )
				{
					dispVerts[k] = temp * this->m_pcModel_SH3->m_pmPrimitive[j].verts[k].vert;
					dispVerts[k].w = 1.0f;
					temp.clearTrans();
					temp.Inverse();
					dispNorms[k] = temp * this->m_pcModel_SH3->m_pmPrimitive[j].verts[k].normal;
				}
				else
					LogFile(ERROR_LOG,"SH3_Actor::AddVertices - TERMINAL ERROR: Prim %d - VertSize %d",j,this->m_pcModel_SH3->m_pmPrimitive[j].vertSize);
			}

		}

		//---[ Assemble Each Vertex ]---/
		for( k = 0; k < this->m_pcModel_SH3->m_pmPrimitive[j].vertHeader.numIndex - 2 ; k++ )
		{
			l_Triangle.DeleteData();
			l_Vertex.DeleteData();

			char l_TexName[ 128 ];

			sprintf( l_TexName, "%d_%d.tga", this->m_pcModel_SH3->baseOffset, this->m_pcModel_SH3->texSeqData[ this->m_pcModel_SH3->m_pmPrimitive[ j ].texNum * 2 ] % this->m_pcModel_SH3->m_mDataHeader.numTex );

			l_Triangle.setMaterial( mstring( l_TexName ) );

			if( this->m_pcModel_SH3->m_pmPrimitive[j].verts == NULL && this->m_pcModel_SH3->m_pmPrimitive[j].altVerts == NULL )
			{
				LogFile(TEST_LOG,"SH3_Actor::ExportSMD - TERMINAL ERROR: Can't render prim #%d - Missing Vert Data",j);
				break;
			}
			else if( this->m_pcModel_SH3->m_pmPrimitive[ j ].verts != NULL )
			{
				if( l_reverseDir == true )
				{
					l_Vertex = this->BuildSMDVertex( this->m_pcModel_SH3->m_pmPrimitive[ j ].verts[ this->m_pcModel_SH3->m_pmPrimitive[ j ].indicies[ k + 1 ] ], primSeq );
					l_Vertex.setPosition( dispVerts[ this->m_pcModel_SH3->m_pmPrimitive[ j ].indicies[ k + 1 ] ] );
					l_Vertex.setNormal( dispNorms[ this->m_pcModel_SH3->m_pmPrimitive[ j ].indicies[ k + 1 ] ] );
					l_Triangle.AddVertex( l_Vertex );
					l_Vertex = this->BuildSMDVertex( this->m_pcModel_SH3->m_pmPrimitive[ j ].verts[ this->m_pcModel_SH3->m_pmPrimitive[ j ].indicies[ k ] ], primSeq );
					l_Vertex.setPosition( dispVerts[ this->m_pcModel_SH3->m_pmPrimitive[ j ].indicies[ k ] ] );
					l_Vertex.setNormal( dispNorms[ this->m_pcModel_SH3->m_pmPrimitive[ j ].indicies[ k ] ] );
					l_Triangle.AddVertex( l_Vertex );
					l_Vertex = this->BuildSMDVertex( this->m_pcModel_SH3->m_pmPrimitive[ j ].verts[ this->m_pcModel_SH3->m_pmPrimitive[ j ].indicies[ k + 2 ] ], primSeq );
					l_Vertex.setPosition( dispVerts[ this->m_pcModel_SH3->m_pmPrimitive[ j ].indicies[ k + 2 ] ] );
					l_Vertex.setNormal( dispNorms[ this->m_pcModel_SH3->m_pmPrimitive[ j ].indicies[ k + 2 ] ] );
					l_Triangle.AddVertex( l_Vertex );
				}
				else
				{
					l_Vertex = this->BuildSMDVertex( this->m_pcModel_SH3->m_pmPrimitive[ j ].verts[ this->m_pcModel_SH3->m_pmPrimitive[ j ].indicies[ k ] ], primSeq );
					l_Vertex.setPosition( dispVerts[ this->m_pcModel_SH3->m_pmPrimitive[ j ].indicies[ k ] ] );
					l_Vertex.setNormal( dispNorms[ this->m_pcModel_SH3->m_pmPrimitive[ j ].indicies[ k ] ] );
					l_Triangle.AddVertex( l_Vertex );
					l_Vertex = this->BuildSMDVertex( this->m_pcModel_SH3->m_pmPrimitive[ j ].verts[ this->m_pcModel_SH3->m_pmPrimitive[ j ].indicies[ k + 1 ] ], primSeq );
					l_Vertex.setPosition( dispVerts[ this->m_pcModel_SH3->m_pmPrimitive[ j ].indicies[ k + 1 ] ] );
					l_Vertex.setNormal( dispNorms[ this->m_pcModel_SH3->m_pmPrimitive[ j ].indicies[ k + 1 ] ] );
					l_Triangle.AddVertex( l_Vertex );
					l_Vertex = this->BuildSMDVertex( this->m_pcModel_SH3->m_pmPrimitive[ j ].verts[ this->m_pcModel_SH3->m_pmPrimitive[ j ].indicies[ k + 2 ] ], primSeq );
					l_Vertex.setPosition( dispVerts[ this->m_pcModel_SH3->m_pmPrimitive[ j ].indicies[ k + 2 ] ] );
					l_Vertex.setNormal( dispNorms[ this->m_pcModel_SH3->m_pmPrimitive[ j ].indicies[ k + 2 ] ] );
					l_Triangle.AddVertex( l_Vertex );
				}
			}
//#################################################################################
			else //---[ THERE IS ONLY ONE MATRIX AND SMALL SIZE VERTS ]---/
			{
				if( l_reverseDir == true )
				{
					l_Vertex = this->BuildSMDVertex( this->m_pcModel_SH3->m_pmPrimitive[ j ].altVerts[ this->m_pcModel_SH3->m_pmPrimitive[ j ].indicies[ k + 1 ] ], primSeq[ 0 ] );
					l_Vertex.setPosition( dispVerts[ this->m_pcModel_SH3->m_pmPrimitive[ j ].indicies[ k + 1 ] ] );
					l_Vertex.setNormal( dispNorms[ this->m_pcModel_SH3->m_pmPrimitive[ j ].indicies[ k + 1 ] ] );
					l_Triangle.AddVertex( l_Vertex );
					l_Vertex = this->BuildSMDVertex( this->m_pcModel_SH3->m_pmPrimitive[ j ].altVerts[ this->m_pcModel_SH3->m_pmPrimitive[ j ].indicies[ k ] ], primSeq[ 0 ] );
					l_Vertex.setPosition( dispVerts[ this->m_pcModel_SH3->m_pmPrimitive[ j ].indicies[ k ] ] );
					l_Vertex.setNormal( dispNorms[ this->m_pcModel_SH3->m_pmPrimitive[ j ].indicies[ k ] ] );
					l_Triangle.AddVertex( l_Vertex );
					l_Vertex = this->BuildSMDVertex( this->m_pcModel_SH3->m_pmPrimitive[ j ].altVerts[ this->m_pcModel_SH3->m_pmPrimitive[ j ].indicies[ k + 2 ] ], primSeq[ 0 ] );
					l_Vertex.setPosition( dispVerts[ this->m_pcModel_SH3->m_pmPrimitive[ j ].indicies[ k + 2 ] ] );
					l_Vertex.setNormal( dispNorms[ this->m_pcModel_SH3->m_pmPrimitive[ j ].indicies[ k + 2 ] ] );
					l_Triangle.AddVertex( l_Vertex );
				}
				else
				{
					l_Vertex = this->BuildSMDVertex( this->m_pcModel_SH3->m_pmPrimitive[ j ].altVerts[ this->m_pcModel_SH3->m_pmPrimitive[ j ].indicies[ k ] ], primSeq[ 0 ] );
					l_Vertex.setPosition( dispVerts[ this->m_pcModel_SH3->m_pmPrimitive[ j ].indicies[ k ] ] );
					l_Vertex.setNormal( dispNorms[ this->m_pcModel_SH3->m_pmPrimitive[ j ].indicies[ k ] ] );
					l_Triangle.AddVertex( l_Vertex );
					l_Vertex = this->BuildSMDVertex( this->m_pcModel_SH3->m_pmPrimitive[ j ].altVerts[ this->m_pcModel_SH3->m_pmPrimitive[ j ].indicies[ k + 1 ] ], primSeq[ 0 ] );
					l_Vertex.setPosition( dispVerts[ this->m_pcModel_SH3->m_pmPrimitive[ j ].indicies[ k + 1 ] ] );
					l_Vertex.setNormal( dispNorms[ this->m_pcModel_SH3->m_pmPrimitive[ j ].indicies[ k + 1 ] ] );
					l_Triangle.AddVertex( l_Vertex );
					l_Vertex = this->BuildSMDVertex( this->m_pcModel_SH3->m_pmPrimitive[ j ].altVerts[ this->m_pcModel_SH3->m_pmPrimitive[ j ].indicies[ k + 2 ] ], primSeq[ 0 ] );
					l_Vertex.setPosition( dispVerts[ this->m_pcModel_SH3->m_pmPrimitive[ j ].indicies[ k + 2 ] ] );
					l_Vertex.setNormal( dispNorms[ this->m_pcModel_SH3->m_pmPrimitive[ j ].indicies[ k + 2 ] ] );
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


void SH_Anim_Loader::ExportSMD(  )
{
	int k;

	SMD::SMD_Animation_Frame	l_AnimFrame;
	SMD::SMD_Model				l_Model;


	//---[ A D D   N O D E S ]---/

	//--[ Node Sequence 1 ]--/
	for( k = 0; k < this->m_pcModel_SH3->m_mDataHeader.numMatSet1; k++ )
	{
		l_Model.AddNode( this->m_pcModel_SH3->mSeq1[ k ] );
	}

	//--[ Node Sequence 2 ]--/
	for( k = 0; k < this->m_pcModel_SH3->m_mDataHeader.numMatSet2; k++ )
	{
		l_Model.AddNode( this->m_pcModel_SH3->mSeq2[ k*2 + 1 ] );
	}

	//---[ G O   T H R O U G H   A L L   P R I M I T I V E S ]---/
	this->AddVerticies( l_Model );


	matrix	*m_pcDispInverse	=	new matrix[ this->m_pcModel_SH3->m_mDataHeader.numMatSet1 ];
	matrix	*l_pcForwardMat		=	new matrix[ this->m_pcModel_SH3->m_mDataHeader.numMatSet1 ];
	//---[ Generate The Animation Data ]---/

		//----[ GENERATE INVERSE MATRICIES ]----/
	for( k = 0; k < this->m_pcModel_SH3->m_mDataHeader.numMatSet1; k++ )	//m_pcAnimSet[ m_lCurFrame ].m_lNumMatrix; k++ )
	{
			m_pcDispInverse[ k ] = this->m_pcDispMat[ k ];	//this->m_pcModel_SH3->matSet1[ k ];
			m_pcDispInverse[ k ].Inverse( );
	}
	//----[ MOVE JOINT POSITION BACK TO ORIGIN AND MAKE IT'S RELATIVE OFFSET ]----/
	for( k = 0; k < this->m_pcModel_SH3->m_mDataHeader.numMatSet1; k++ )	//m_pcAnimSet[ m_lCurFrame ].m_lNumMatrix; k++ )
	{	
		//---[ MAKE JOINT POSITION RELATIVE OFFSET TO ORIGIN ]---/
		if( this->m_pcModel_SH3->mSeq1[ k ] != -1 )
			l_pcForwardMat[ k ] = m_pcDispInverse[ this->m_pcModel_SH3->mSeq1[ k ] ] * this->m_pcDispMat[ k ];	//m_pcModel_SH3->matSet1[ k ];
		else
			l_pcForwardMat[ k ] = this->m_pcDispMat[ k ];	//m_pcModel_SH3->matSet1[ k ];

		//if( m_pcAnimSet[ m_lCurFrame ].getFlag( k ) != 0 )
		//		l_pcForwardMat[ k ].clearRot( );
	}

	for( k = 0; k < this->m_pcModel_SH3->m_mDataHeader.numMatSet1; k++ )
	{
		l_AnimFrame.AddTransform( l_pcForwardMat[ k ] );
		//matrix tempMat = l_pcForwardMat[ k ] * this->m_pcAnimSet[ 0 ][ k ];
		//l_AnimFrame.AddTransform( tempMat );
		//l_AnimFrame.AddTransform( this->matSet1[ k ] );
	}

	delete [] l_pcForwardMat;
	delete [] m_pcDispInverse;

	for( k = 0; k < this->m_pcModel_SH3->m_mDataHeader.numMatSet2; k++ )
	{
		//matrix l_TempMatrix = this->matSet1[ mSeq2[k*2+1] ] * this->matSet2[ k ];
		l_AnimFrame.AddTransform(  this->m_pcModel_SH3->matSet2[ k ] );
	}
		
	l_Model.AddAnimationFrame( l_AnimFrame );

	std::fstream  l_OutputFile;
	char l_OutputModelName[ 256 ];

	sprintf( l_OutputModelName, "%s_%ld.smd",baseName( mstring(this->m_pcModel_SH3->modelFilename) ).c_str(), this->m_pcModel_SH3->baseOffset );

	l_OutputFile.open( l_OutputModelName, ios::out | ios::trunc );
	
	l_OutputFile << l_Model;

	l_OutputFile.close( );
}


SMD::SMD_Vertex SH_Anim_Loader::BuildSMDVertex( sh_vertex & _vert, long *_pSeq )
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


SMD::SMD_Vertex SH_Anim_Loader::BuildSMDVertex( sh_static_model_vertex & _vert, long _parent )
{
	SMD_Vertex	l_Vertex;

	l_Vertex.setPosition( _vert.vert );
	l_Vertex.setNormal( _vert.norm );
	l_Vertex.setTexcoord( _vert.tc );
	l_Vertex.AddWeight( 1.0f, _parent );
	
	return l_Vertex;
}



//################################################### M  O  D  E  L    R  E  N  D  E  R #############################
void SH_Anim_Loader::RenderModel( long modelPart )
{
		int k;
	int j;
	int startPt, endPt;
	vertex4f *dispVerts = NULL;
	vertex	ones(1.0f,1.0f,1.0f);
	matrix	*matArray = NULL;
	matrix	*matSet1 = NULL;
	matrix	*matSet2 = NULL;
	char	*mSeq1;
	char	*mSeq2;
	model_primitive *m_pmPrimitive;

	static bool printData = true;

	vertex4f tv1,tv2,tv3,tv4;

	if( !m_pcModel_SH3 )
	{
			LogFile( ERROR_LOG, "SH_Anim_Loader::LoadAnim( ) - ERROR: No Model is Attached to this animation to render" );
			return;
	}

	if( !m_pcAnimSet )
	{
			LogFile( ERROR_LOG, "SH_Anim_Loader::LoadAnim( ) - ERROR: No Animation is loaded for this Model to render" );
			return;
	}

//LogFile(ERROR_LOG,"ApplyAnimFrame - STUPID PIECE OF SHIT - iteration");
	mSeq1 = m_pcModel_SH3->mSeq1;
	mSeq2 = m_pcModel_SH3->mSeq2;
	matSet1 = m_pcModel_SH3->matSet1;
	matSet2 = m_pcModel_SH3->matSet2;
	m_pmPrimitive = m_pcModel_SH3 ->m_pmPrimitive;

	if( modelPart == -1 || modelPart < -1 || modelPart > m_pcModel_SH3->numPrimitives )
	{
		startPt = 0;
		endPt = m_pcModel_SH3->numPrimitives;
	}
	else
	{
		startPt = modelPart;
		endPt = startPt + 1;
	}

	for( j = startPt; j < endPt; j++ )
	{
//		if(printData)LogFile( DATA_LOG, "Primitive %ld of %ld -------------------",j,endPt);
//LogFile(ERROR_LOG,"ApplyAnimFrame - STUPID PIECE OF SHIT - outer iteration %ld",j);
//		if(printData)
//			LogFile(ERROR_LOG,"Num Verts: %d\tNum Index: %d\tTex ID: %d",m_pmPrimitive[j].vertHeader.numVerts,m_pmPrimitive[j].vertHeader.numIndex,m_pmPrimitive[j].texID);
		dispVerts = new vertex4f[m_pmPrimitive[j].vertHeader.numVerts];

		matArray = new matrix[ m_pmPrimitive[j].vertHeader.numSeq1 + m_pmPrimitive[j].vertHeader.numSeq2];

//for( k = 0; k < m_mDataHeader.numMatSet2; k++)
//{
//	matSet2[k] = tMatSet1[mSeq2[k*2+1]] * tMatSet2[k];
//	matSet2[k].transpose();
//}
if( displayMatrix )
{
	vertex b1,b2;

	for( k = 0; k < m_pcModel_SH3->m_mDataHeader.numMatSet2; k++ ) //m_lNumDispMat; k++)
	{
		matrix *mPtr = m_pcDispInverse; //matSet1;
		b1 = vertex( mPtr[k][ 3 ], mPtr[k][ 7 ], mPtr[k][ 11 ]);
		b2 = vertex( m_pcDispMat[mSeq2[k*2+1]][ 3 ], m_pcDispMat[mSeq2[k*2+1]][ 7 ], m_pcDispMat[mSeq2[k*2+1]][ 11 ]);
		//if( mSeq1[k] != -1 )
		//{
			//b1 = vertex( mPtr[ mSeq1[ k ]][ 3 ], mPtr[ mSeq1[ k ]][ 7 ], mPtr[ mSeq1[ k ]][ 11 ] );
			//b2 = vertex( mPtr[ k ][ 3 ], mPtr[ k ][ 7 ], mPtr[ k ][ 11 ] );
			glLineWidth( 2.0f );
			glBegin( GL_LINES );
				glVertex3fv( &b1.x );
				glVertex3fv( &b2.x );
			glEnd( );
		//}
		drawBasisMatrix( &( mPtr[ k ] ), 9.0f,0,k );
	}
	for( k = 0; k < m_lNumDispMat; k++ )
	{
		matrix tMat;
		matrix *mPtr = 
				m_pcDispMat; 
				//matSet2;
	//	b1 = vertex( mPtr[mSeq2[k*2]][ 3 ], mPtr[mSeq2[k*2]][ 7 ], mPtr[mSeq2[k*2]][ 11 ]);
	//	b2 = vertex( mPtr[mSeq2[k*2+1]][ 3 ], mPtr[mSeq2[k*2+1]][ 7 ], mPtr[mSeq2[k*2+1]][ 11 ]);
	if( mSeq1[k] != -1 )
		{
			b1 = vertex( mPtr[ mSeq1[ k ]][ 3 ], mPtr[ mSeq1[ k ]][ 7 ], mPtr[ mSeq1[ k ]][ 11 ] );
			b2 = vertex( mPtr[ k ][ 3 ], mPtr[ k ][ 7 ], mPtr[ k ][ 11 ] );
			glLineWidth( 2.0f );
			glColor3f( 1.0f,1.0f,1.0f);
			glBegin( GL_LINES );
				glVertex3fv( &b1.x );
				glVertex3fv( &b2.x );
			glEnd( );
		}
		tMat = matSet1[k] * m_pcDispMat[k] ;//* matSet2[k];
		//drawBasisMatrix( &tMat , 9.0f, 1 );
		drawBasisMatrix( &( mPtr[ k ] ), 9.0f,1, k);
	}
}
//LogFile(ERROR_LOG,"ApplyAnimFrame - STUPID PIECE OF SHIT - About to set 1st set of matricies");
		for( k = 0; k < m_pmPrimitive[j].vertHeader.numSeq1; k ++ )
		{
///*WORKS*/	matArray[k] = matSet1[m_pmPrimitive[j].seqData1[k]];
/****************************************************************************/
			matArray[k] = 
			//	matSet1[m_pmPrimitive[j].seqData1[k]]
			//	*
				m_pcDispMat[m_pmPrimitive[j].seqData1[k]]
			//	*
			//	m_pcDispInverse[m_pmPrimitive[j].seqData1[k]]
				;

				//if(printData)LogFile( DATA_LOG,"Mat %ld",m_pmPrimitive[j].seqData1[k]);
/****************************************************************************/
			//matArray[k].transpose();
//			if( displayMatrix )
//				drawBasisMatrix( &( matArray[k] ), 6.0f );
		}

		for( k = m_pmPrimitive[j].vertHeader.numSeq1; k < m_pmPrimitive[j].vertHeader.numSeq1 + m_pmPrimitive[j].vertHeader.numSeq2; k++ )
		{
			//* matSet1[mSeq2[(m_pmPrimitive[j].seqData2[k-m_pmPrimitive[j].vertHeader.numSeq1])*2+1]] 
			//matArray[k] = m_pcDispMat[mSeq2[(m_pmPrimitive[j].seqData2[k-m_pmPrimitive[j].vertHeader.numSeq1])*2+1]]* matSet1[mSeq2[(m_pmPrimitive[j].seqData2[k-m_pmPrimitive[j].vertHeader.numSeq1])*2+1]]  * matSet2[m_pmPrimitive[j].seqData2[k-m_pmPrimitive[j].vertHeader.numSeq1]];

///*WORKS*/			matArray[k] = matSet1[mSeq2[(m_pmPrimitive[j].seqData2[k-m_pmPrimitive[j].vertHeader.numSeq1])*2+1]] * matSet2[m_pmPrimitive[j].seqData2[k-m_pmPrimitive[j].vertHeader.numSeq1]];
/****************************************************************************/
			matArray[k] = 
			//	m_pcDispMat[mSeq2[(m_pmPrimitive[j].seqData2[k-m_pmPrimitive[j].vertHeader.numSeq1])*2+1]]
			//	*
			//	matSet2[m_pmPrimitive[j].seqData2[k-m_pmPrimitive[j].vertHeader.numSeq1]]
				m_pcDispInverse[m_pmPrimitive[j].seqData2[k-m_pmPrimitive[j].vertHeader.numSeq1]]
				;
				//if(printData)LogFile( DATA_LOG,"Mat %ld * %ld",mSeq2[(m_pmPrimitive[j].seqData2[k-m_pmPrimitive[j].vertHeader.numSeq1])*2+1],m_pmPrimitive[j].seqData2[k-m_pmPrimitive[j].vertHeader.numSeq1]);
			//if( getSeq( getCurFlags( m_pcAnimSet[ m_lCurFrame ].m_plFlags, mSeq2[(m_pmPrimitive[j].seqData2[k-m_pmPrimitive[j].vertHeader.numSeq1])*2+1] )) == 12 )
			//	matArray[ k ] = m_pcDispInverse[ mSeq2[(m_pmPrimitive[j].seqData2[k-m_pmPrimitive[j].vertHeader.numSeq1])*2+1]] * matArray[ k ];
/****************************************************************************/			
			//matArray[k] = matSet1[mSeq2[(m_pmPrimitive[j].seqData2[k-m_pmPrimitive[j].vertHeader.numSeq1])*2+1]] * matSet2[m_pmPrimitive[j].seqData2[k-m_pmPrimitive[j].vertHeader.numSeq1]]
			//matArray[k] = matSet2[m_pmPrimitive[j].seqData2[k-m_pmPrimitive[j].vertHeader.numSeq1]];
			//matArray[k].transpose();
						
//			if( displayMatrix )
//				drawBasisMatrix( &( matArray[k] ), 6.0f );
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
			
//LogFile(ERROR_LOG,"ApplyAnimFrame - STUPID PIECE OF SHIT - About to transform for %ld vert",k);
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
					if(printData)LogFile(TEST_LOG,"SH3_Actor::draw - Got Here: Can render prim #%d ? - Small size verts have single matrix",j);
						
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
//*/

//***********  CHECKING MODEL RENDERING *********
		if( j >= m_pcModel_SH3->m_mDataHeader.numVertexArrays )	//m_pmPrimitive[j].texModify == 4 )
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

		if( !displayMatrix )
		glDrawElements(GL_TRIANGLE_STRIP, m_pmPrimitive[j].vertHeader.numIndex, GL_UNSIGNED_INT, m_pmPrimitive[j].indicies);
		checkGLerror(__LINE__,__FILE__,"Before 7");

		glDisableClientState(GL_VERTEX_ARRAY);
//		checkGLerror(__LINE__,__FILE__,"Before 8");
		glDisableClientState(GL_TEXTURE_COORD_ARRAY);

		if( j >= m_pcModel_SH3->m_mDataHeader.numVertexArrays )	//m_pmPrimitive[j].texModify == 4 )
		{
			glDisable(GL_ALPHA_TEST);
//			glDisable(GL_BLEND);
		}

		delete [] dispVerts;
		delete [] matArray;

//		checkGLerror(__LINE__,__FILE__,"After Render");
		//glDisableClientState(GL_NORMAL_ARRAY);
	}
	printData = false;
}


void SH_Anim_Loader::ApplyNextAnimFrame( )
{
	if( !m_pcModel_SH3 || !m_bAnimLoaded )
		return;

	++m_lCurFrame;

	ApplyAnimFrame( m_lCurFrame );
}


void SH_Anim_Loader::ApplyAnimFrame( long _animNum )
{
	static bool fTime = true;
	long k;
	matrix *l_pcAnimMat = NULL;
	matrix *l_pcForwardMat = NULL;
	QuatAnim *l_pcFinalQuat = NULL;
	matrix testMat;
matrix bO;

	if( ! m_pcModel_SH3 )
		return;

	if( _animNum < 0 )
		m_lCurFrame = m_lNumSets - 1;
	m_lCurFrame = (_animNum % m_lNumSets);

//if( fTime )LogFile(ERROR_LOG,"BEFORE DELETES");
	SAFEDELETE( m_pcDispMat );
	SAFEDELETE( m_pcDispFinal );
	SAFEDELETE( m_pcDispInverse );
	SAFEDELETE( m_pcDispQuat );
//if( fTime )LogFile(ERROR_LOG,"AFTER DELETES");

	//---[ Get # of animation mats, and make sure it at least matches the # of mats for the 1st mat set in model ]---/
	m_lNumDispMat = m_pcAnimSet[ m_lCurFrame ].m_lNumMatrix;

	if( m_lNumDispMat < m_pcModel_SH3->m_mDataHeader.numMatSet1 )
		m_lNumDispMat = m_pcModel_SH3->m_mDataHeader.numMatSet1;

	//---[ Init Matrix Array For Animation Matricies that Don't have enough Mats ]---/
	l_pcAnimMat = new matrix[ m_lNumDispMat ];

	for( k = 0; k < m_lNumDispMat; k++  )
		l_pcAnimMat[ k ].identity( );
	for( k = 0; k < m_pcAnimSet[ m_lCurFrame ].m_lNumMatrix && k < m_lNumDispMat; k++ )
	{
		l_pcAnimMat[ k ] = m_pcAnimSet[ m_lCurFrame ][ k ];

		//---[ DEBUG: DISPLAY ORIGINAL ANIMATION MATRIX ]---/
		if( fTime && animDebugMode )
		{
			LogFile(ERROR_LOG," %2d. Base Animation Matrix\n--------------------------",k);
			debugMatrix( l_pcAnimMat[ k ] );
		}
	}
	

	//---[ Initialize Matrix Arrays ]---/
	m_pcDispMat = new matrix[ m_lNumDispMat ];		//m_pcAnimSet[ m_lCurFrame ].m_lNumMatrix ];
	m_pcDispFinal = new matrix[ m_lNumDispMat ];		//m_pcAnimSet[ m_lCurFrame ].m_lNumMatrix ];
	m_pcDispInverse = new matrix[ m_lNumDispMat ];		//m_pcAnimSet[ m_lCurFrame ].m_lNumMatrix ];
	m_pcDispQuat = new QuatAnim[ m_lNumDispMat ];		//m_pcAnimSet[ m_lCurFrame ].m_lNumMatrix ];
	
	l_pcFinalQuat = new QuatAnim[ m_lNumDispMat ];
	l_pcForwardMat = new matrix[ m_lNumDispMat ];		//m_pcAnimSet[ m_lCurFrame ].m_lNumMatrix ];

//LogFile(ERROR_LOG,"ApplyAnimFrame - STUPID PIECE OF SHIT - new m_pcDispMat = %08x",m_pcDispMat);
	//---[ C O M P U T E   T H E   M A T R I X   S E T   F O R   C U R R E N T   F R A M E ]---/
	
	

if( testModeInt < 4 || testModeInt > 5 )
	testModeInt = 4;
/***************************************************************************************/
	if( testModeInt == 4 )
	{
		//----[ GENERATE INVERSE MATRICIES ]----/
		for( k = 0; k < m_lNumDispMat; k++ )	//m_pcAnimSet[ m_lCurFrame ].m_lNumMatrix; k++ )
		{
			m_pcDispInverse[ k ] = m_pcModel_SH3->matSet1[ k ];
			m_pcDispInverse[ k ].Inverse( );
		}
		//----[ MOVE JOINT POSITION BACK TO ORIGIN AND MAKE IT'S RELATIVE OFFSET ]----/
		for( k = 0; k < m_lNumDispMat; k++ )	//m_pcAnimSet[ m_lCurFrame ].m_lNumMatrix; k++ )
		{	
			//---[ MAKE JOINT POSITION RELATIVE OFFSET TO ORIGIN ]---/
			if( m_pcModel_SH3->mSeq1[ k ] != -1 )
				l_pcForwardMat[ k ] = m_pcDispInverse[ m_pcModel_SH3->mSeq1[ k ] ] * m_pcModel_SH3->matSet1[ k ];
			else
				l_pcForwardMat[ k ] = m_pcModel_SH3->matSet1[ k ];
			//---[ DEBUG: DISPLAY THE JOINT POSITION MATRIX ]---/
			if( fTime && animDebugMode )
			{
				LogFile(ERROR_LOG," %2d. Joint Position Matrix\n--------------------------",k);
				debugMatrix( l_pcForwardMat[ k ] );
			}
			//if( k < m_pcAnimSet[ m_lCurFrame ].m_lNumMatrix )
		//	if( m_pcModel_SH3->mSeq1[ k ] == -1 )
			if( m_pcAnimSet[ m_lCurFrame ].getFlag( k ) != 0 )
				l_pcForwardMat[ k ].clearRot( );

	//		if( m_lCurFrame == 37 )
	//			LogFile( DATA_LOG, "The Flag at %ld is %ld",k,m_pcAnimSet[ m_lCurFrame ].getFlag(k ));
		}

		//----[ TWO WAYS TO DO IT: ROTATE ORIGIN AND TRANSLATE BY PARENT, or ROTATE RELATIVE OFFSET AND TRANSLATE PARENT ]--
		for( k = 0; k < m_lNumDispMat; k++ )	//m_pcAnimSet[ m_lCurFrame ].m_lNumMatrix; k++ )
		{
			m_pcDispFinal[ k ] = l_pcForwardMat[ k ] * l_pcAnimMat[ k ];		//m_pcAnimSet[ m_lCurFrame ][ k ];
				// ---------------------- OR -----------------------------------
		//	if( m_pcModel_SH3->mSeq1[ k ] != -1 )
		//		m_pcDispFinal[ k ] = m_pcAnimSet[ m_lCurFrame ][ m_pcModel_SH3->mSeq1[ k ] ] * l_pcForwardMat[ k ];
		//	else
		//		m_pcDispFinal[ k ] = l_pcForwardMat[ k ];
		}
		//----[ REBUILD THE MODEL ]----/
		for( k = 0; k < m_lNumDispMat; k++ )	//m_pcAnimSet[ m_lCurFrame ].m_lNumMatrix; k++ )
		{
			if( m_pcModel_SH3->mSeq1[ k ] != -1 )
				m_pcDispMat[ k ] = m_pcDispMat[ m_pcModel_SH3->mSeq1[ k ] ] * m_pcDispFinal[ k ];
			else
				m_pcDispMat[ k ] = m_pcDispFinal[ k ];
			if( fTime && animDebugMode )
			{
				LogFile(ERROR_LOG," %2d. Final Animation Matrix\n---------------------------",k);
				debugMatrix( m_pcDispMat[ k ] );
			}
			//---[ Make Inv Matrix ]---/
			//m_pcDispInverse[ k ] = m_pcDispMat[ k ];
			m_pcDispInverse[ k ] = l_pcAnimMat[ k ];
			m_pcDispInverse[ k ].Inverse( );
		}

		/************************************* TEST THEORY ABOUT THE 2nd SEQ SET ***************/
		delete [] m_pcDispInverse;
		m_pcDispInverse = new matrix[ m_pcModel_SH3->m_mDataHeader.numMatSet2 ];
		for( k = 0; k < m_pcModel_SH3->m_mDataHeader.numMatSet2; k++ )
		{
			if( m_pcModel_SH3->mSeq1[ m_pcModel_SH3->mSeq2[ k*2+1 ] ] == -1 )
				m_pcDispInverse[ k ] = 
									m_pcDispMat[ m_pcModel_SH3->mSeq2[ k*2+1 ] ]		//Joint's Parent Position (Anim Applied)
									* 
								//	m_pcModel_SH3->matSet2[ m_pcModel_SH3->mSeq2[ k*2 ] ]							//Mat Set For Sizing
								//	*
									m_pcModel_SH3->matSet2[ k ]							//Mat Set For Sizing (Alt Pos)
									;
			else
				m_pcDispInverse[ k ] = 
									m_pcDispMat[ m_pcModel_SH3->mSeq2[ k*2+1 ] ]			//Joint's Parent Position (Anim Applied)
									* 
								//	m_pcModel_SH3->matSet2[ m_pcModel_SH3->mSeq2[ k*2 ] ]							//Mat Set For Sizing
								//	*
								//	m_pcDispFinal[ m_pcModel_SH3->mSeq2[ k*2+1 ] ]		//Joint at origin
								//	* 
									m_pcModel_SH3->matSet2[ k ]							//Mat Set For Sizing (Alt Pos)
									;
		}
fTime = false;

		delete [] l_pcAnimMat;
		delete [] l_pcForwardMat;
		delete [] l_pcFinalQuat;
		return;
	}
/***************************************************************************************/
	if( testModeInt == 5 )
	{
LogFile( ERROR_LOG,"Got In TestMode 5");
		//----[ GENERATE INVERSE MATRICIES ]----/
		for( k = 0; k < m_lNumDispMat; k++ )	//m_pcAnimSet[ m_lCurFrame ].m_lNumMatrix; k++ )
		{
			m_pcDispInverse[ k ] = m_pcModel_SH3->matSet1[ k ];
			m_pcDispInverse[ k ].Inverse( );
		}
		//----[ MOVE JOINT POSITION BACK TO ORIGIN AND MAKE IT'S RELATIVE OFFSET ]----/
		for( k = 0; k < m_lNumDispMat; k++ )	//m_pcAnimSet[ m_lCurFrame ].m_lNumMatrix; k++ )
		{	
			//---[ MAKE JOINT POSITION RELATIVE OFFSET TO ORIGIN ]---/
			if( m_pcModel_SH3->mSeq1[ k ] != -1 )
				l_pcForwardMat[ k ] = m_pcDispInverse[ m_pcModel_SH3->mSeq1[ k ] ] * m_pcModel_SH3->matSet1[ k ];
			else
				l_pcForwardMat[ k ] = m_pcModel_SH3->matSet1[ k ];
			if( k < m_pcAnimSet[ m_lCurFrame ].m_lNumMatrix )
				l_pcForwardMat[ k ].clearRot( );
		}

		//----[ TWO WAYS TO DO IT: ROTATE ORIGIN AND TRANSLATE BY PARENT, or ROTATE RELATIVE OFFSET AND TRANSLATE PARENT ]--
		for( k = 0; k < m_lNumDispMat; k++ )	//m_pcAnimSet[ m_lCurFrame ].m_lNumMatrix; k++ )
		{
			m_pcDispQuat[ k ].m_cAnimPos = m_pcAnimSet[ m_lCurFrame ].m_pcQuatSet[k].m_cAnimPos + vertex( l_pcForwardMat[ k ][ 3 ], l_pcForwardMat[ k ][ 7 ], l_pcForwardMat[ k ][ 11 ] );
			m_pcDispQuat[ k ].m_cAnimAngle = m_pcAnimSet[ m_lCurFrame ].m_pcQuatSet[k].m_cAnimAngle;
		}
		//----[ REBUILD THE MODEL ]----/
		for( k = 0; k < m_lNumDispMat; k++ )	//m_pcAnimSet[ m_lCurFrame ].m_lNumMatrix; k++ )
		{
			if( m_pcModel_SH3->mSeq1[ k ] != -1 )
	//			l_pcFinalQuat[ k ] = m_pcDispQuat[ k ] * l_pcFinalQuat[ m_pcModel_SH3->mSeq1[ k ] ];
				l_pcFinalQuat[ k ] = l_pcFinalQuat[ m_pcModel_SH3->mSeq1[ k ] ] * m_pcDispQuat[ k ];
			else
				l_pcFinalQuat[ k ] = m_pcDispQuat[ k ];
		}

		for( k = 0; k < m_lNumDispMat; k++ )
			m_pcDispMat[ k ] = l_pcFinalQuat[ k ].getMatrix( );

fTime = false;

		delete [] l_pcAnimMat;
		delete [] l_pcForwardMat;
		delete [] l_pcFinalQuat;
		return;
	}

}





quat SH_AnimSet::convertToQuat( short int xVal, short int yVal, short int zVal )
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

quat SH_AnimSet::convertToQuatUnsigned( short int xVal, short int yVal, short int zVal )
{
	quat tempQuat;
	float _2pow16 = 65536.0f;
	float _2pow32 = 32768.0f;
	unsigned short x = (unsigned short)xVal;
	unsigned short y = (unsigned short)yVal;
	unsigned short z = (unsigned short)zVal;

	tempQuat.x = ((float)x) / _2pow16;
	tempQuat.y = ((float)y) / _2pow16;
	tempQuat.z = ((float)z) / _2pow16;
//	tempQuat.x = ((float)xVal) / _2pow32;
//	tempQuat.y = ((float)yVal) / _2pow32;
//	tempQuat.z = ((float)zVal) / _2pow32;

//	tempQuat.x -= 1.0f;
//	tempQuat.y -= 1.0f;
//	tempQuat.z -= 1.0f;
	QUnit( &tempQuat );
	tempQuat.computeR( );


	return tempQuat;
}


quat SH_AnimSet::convertToQuatMat( short int xVal, short int yVal, short int zVal )
{
	quat tempQuat;
	matrix xMat, yMat, zMat, total;
	float _2pow32 = 32768.0f;

	tempQuat.x = ((float)xVal) / _2pow32;
	tempQuat.y = ((float)yVal) / _2pow32;
	tempQuat.z = ((float)zVal) / _2pow32;

	MROTATEX(xMat, tempQuat.x );
	MROTATEX(yMat, tempQuat.y );
	MROTATEX(zMat, tempQuat.z );

	total = xMat * yMat * zMat;

	MTOQ( &tempQuat, &total );
	//tempQuat.computeR( );
	QUnit( &tempQuat );

	return tempQuat;
}


unsigned long SH_AnimSet::getFlag( long _lIndex )
{ 
	if( _lIndex < 0 || _lIndex >= m_lNumMatrix )
		return 0; 
	
	switch( _lIndex % 8 )
	{
	case 0:
		return byte1_low( m_plFlags[ _lIndex >> 3 ]);
	case 1:
		return byte1_high( m_plFlags[ _lIndex >> 3 ]);
	case 2:
		return byte2_low( m_plFlags[ _lIndex >> 3 ]);
	case 3:
		return byte2_high( m_plFlags[ _lIndex >> 3 ]);
	case 4:
		return byte3_low( m_plFlags[ _lIndex >> 3 ]);
	case 5:
		return byte3_high( m_plFlags[ _lIndex >> 3 ]);
	case 6:
		return byte4_low( m_plFlags[ _lIndex >> 3 ]);
	case 7:
		return byte4_high( m_plFlags[ _lIndex >> 3 ]);
	}

	return 0;
}
//#######################################################################################33
// R E F O R M   T H I S   F U C K E D   U P   C O D E
//################################################################################
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
/*
int LoadAnimSet2( FILE * inFile, unsigned char *ps_mSeq, long _lNumSeq, bool sh2_load )
//int SH_AnimSet::LoadAnimSet( FILE * inFile, unsigned char *ps_mSeq, long _lNumSeq, bool sh2_load )
{
	vector<unsigned long>	flagVec;
	vector<matrix>	matVec;
	vector<matrix>	matVec2;
	vector<QuatAnim> quatVec;
	matrix			tempMat;
	matrix			tempMat2;
	quat			tempQuat;
	QuatAnim		tempQAnim;
	QuatAnim		tempQAnim2;
	long			startFlagOffset = ftell( inFile );
	long			preOffset;
	unsigned long	nextFlag = 0;
	long			res;
	long			lTotalRead = 0;
	long			lStartTotal = 0;
	long			k, j;
//	long			curMainFlag;
	unsigned char	flagArray[8];
	unsigned char	mainFlags[4];
	sh_anim_ang		l_sAnimAng;
	sh_anim_pos_ang	l_sAnimPosAng;
	bool			bFullSpecial = false;
	bool			bPartialSpecial = false;
	bool			bLoadTrans = false;
	bool			bEndSpecial = false;
	bool			l_bStopCondition = false;

	static bool printData = true;

	if( debugMode )
		LogFile( ERROR_LOG, "SH_AnimSet::LoadAnimSet( off: %ld ) ---------------[ S T A R T ]------------", startFlagOffset);


	if( ( res = fread( (void *)&nextFlag, 1, sizeof(long), inFile )) != sizeof( long ) )
	{
		LogFile( ERROR_LOG, "SH_AnimSet::LoadAnimSet( ) - ERROR: Could not read animation block flags at offset %ld  -  REASON: %s",startFlagOffset, strerror( errno ));
		return 0;
	}

	lTotalRead += res;

	for( j = 0; j < _lNumSeq && !l_bStopCondition ; j+=8 )
	{
		if( debugMode || animDebugMode )
			LogFile( ERROR_LOG, "SH_AnimSet::LoadAnimSet( ) - Reading animation block at offset %ld\tFlags[ %d %d %d %d ]", startFlagOffset, ((unsigned char*)&nextFlag)[0], ((unsigned char*)&nextFlag)[1], ((unsigned char*)&nextFlag)[2], ((unsigned char*)&nextFlag)[3]);
		if( debugMode || animDebugMode )
			LogFile( TEST_LOG, "SH_AnimSet # %.3d - FLAGS: [ %08x ]\tSeq: [ %.2d %.2d %.2d %.2d %.2d %.2d %.2d %.2d ]",flagVec.size(), nextFlag, 
					getSeq( byte1_low(nextFlag) ), getSeq( byte1_high(nextFlag) ), getSeq( byte2_low(nextFlag) ), 
					getSeq( byte2_high(nextFlag) ), getSeq( byte3_low(nextFlag) ), getSeq( byte3_high(nextFlag) ), 
					getSeq( byte4_low(nextFlag) ), getSeq( byte4_high(nextFlag) ));

		lStartTotal = 0;

		//---[ Save Off All Data In An Array To Make Load More Efficient ]---/
		mainFlags[ 0 ] = byte1_all( nextFlag );
		mainFlags[ 1 ] = byte2_all( nextFlag );
		mainFlags[ 2 ] = byte3_all( nextFlag );
		mainFlags[ 3 ] = byte4_all( nextFlag );
		
		flagArray[ 0 ] = byte1_low( nextFlag );
		flagArray[ 1 ] = byte1_high( nextFlag );
		flagArray[ 2 ] = byte2_low( nextFlag );
		flagArray[ 3 ] = byte2_high( nextFlag );
		flagArray[ 4 ] = byte3_low( nextFlag );
		flagArray[ 5 ] = byte3_high( nextFlag );
		flagArray[ 6 ] = byte4_low( nextFlag );
		flagArray[ 7 ] = byte4_high( nextFlag );

		//---[ Check for a stop condition after these flags ]---/
		if( mainFlags[ 2 ] == 0 && mainFlags[ 3 ] == 0 )
			l_bStopCondition = true;

		//---[ Save Flags For Later Use ]---/
		flagVec.push_back( nextFlag );

		//---[ Load Up All Data For Block, And Convert To Matricies ]---/

		for( k = 0; k < 8; k++ )
		{
			preOffset = ftell( inFile );

			if( j + k < _lNumSeq && (ps_mSeq[ j + k ] == 255 || ps_mSeq[ j + k ] == -1) && flagArray[ k ] != 0 )
			{
				if( ( res = fread( (void *)&l_sAnimPosAng, 1, sizeof(sh_anim_pos_ang), inFile )) != sizeof( sh_anim_pos_ang ) )
				{
					LogFile( ERROR_LOG, "SH_AnimSet::LoadAnimSet( ) - ERROR: Could not read animation position/angle struct for seq[%ld] = %d at offset %ld for flag[%d] = %d -  REASON: %s",j+k,ps_mSeq[j+k],preOffset, k, flagArray[k],strerror( errno ));
					return 0;
				}

				lStartTotal += res;

				if( animDebugMode )
					debugAPA( &l_sAnimPosAng, 101 );
//test_anim_seq0_use1st
				if( test_anim_seq8_use1st && (test_anim_swap_s0( flagArray[ k ] ) || test_anim_swap_s8( flagArray[ k ] ) ) &&sh2_load )
					tempQuat = convertToQuatUnsigned( l_sAnimPosAng.xQuatL, l_sAnimPosAng.yQuatL, l_sAnimPosAng.zQuatL );//(test_anim_swap_s8( flagArray[ k ] ) )?l_sAnimPosAng.xQuatL:0.0f, (test_anim_swap_s0( flagArray[ k ] ) )?l_sAnimPosAng.yQuatL:0.0f, (test_anim_seq0_use1st)?l_sAnimPosAng.zQuatL:0.0f );
				else
				if( (test_anim_swap_s0( flagArray[ k ] ) || test_anim_swap_s8( flagArray[ k ] ) ) && sh2_load )
					tempQuat = convertToQuat( l_sAnimPosAng.yQuatL, l_sAnimPosAng.xQuatL, l_sAnimPosAng.zQuatL );
				else
					tempQuat = convertToQuat( l_sAnimPosAng.xQuatL, l_sAnimPosAng.yQuatL, l_sAnimPosAng.zQuatL );

				QUnit( &tempQuat );
				QTOM( &tempQuat, &tempMat );

				if( animDebugMode && printData )
				{
					float xVal, yVal, zVal;
					tempQuat.getAxisAngles( &xVal, &yVal, &zVal );
					LogFile( ERROR_LOG, "AXIS ANGLES: x: %.3f\ty: %.3f\tz: %.3f",xVal, yVal, zVal );
				}
				QINVERSE(&tempQuat,&tempQuat);		//---[ Needed to Make Quat Animation Work ]---/

				tempQAnim = QuatAnim( tempQuat, vertex( l_sAnimPosAng.xPosL, l_sAnimPosAng.yPosL, l_sAnimPosAng.zPosL ) );

//				if( test_anim_seq0_use1st )//(test_anim_use1st_s0( flagArray[ k ] ) || test_anim_use1st_s8( flagArray[ k ] ) ) && sh2_load )
//					tempMat.swapRow( 0, 1 );

	//			if( getSeq( flagArray[ k ] ) == 0 ) //|| getSeq( flagArray[ k ] ) == 0 )	
				if( ((test_anim_transpose_s0( flagArray[ k ] ) || test_anim_transpose_s8( flagArray[ k ] ) ) && sh2_load) || ( !sh2_load && (getSeq( flagArray[ k ] ) == 8 || getSeq( flagArray[ k ] ) == 0 )))
					tempMat.transpose( );


				tempMat.mat[  3 ] = l_sAnimPosAng.xPosL;
				tempMat.mat[  7 ] = l_sAnimPosAng.yPosL;
				tempMat.mat[ 11 ] = l_sAnimPosAng.zPosL;
				matVec.push_back( tempMat );
				quatVec.push_back( tempQAnim );

//				if( sh2_load )
//				{
//					matVec.push_back( tempMat2 );
//					quatVec.push_back( tempQAnim2 );
//				}
if( animDebugMode && printData)debugMatrix( tempMat );
			}
			else if( j + k < _lNumSeq && flagArray[ k ] != 0 && flagArray[ k ] & 0x02 )
			{
				if( ( res = fread( (void *)&l_sAnimAng, 1, sizeof(sh_anim_ang), inFile )) != sizeof( sh_anim_ang ) )
				{
					LogFile( ERROR_LOG, "SH_AnimSet::LoadAnimSet( ) - ERROR: Could not read animation angle struct for seq[%ld] = %d at offset %ld for flag[%d] = %d -  REASON: %s",j+k,ps_mSeq[j+k],preOffset, k, flagArray[k],strerror( errno ));
					return 0;
				}

				lStartTotal += res;

				if( animDebugMode )
					debugAA( &l_sAnimAng, 102);//-12345 );

				if( test_anim_seq8_use1st && (test_anim_swap_s0( flagArray[ k ] ) || test_anim_swap_s8( flagArray[ k ] ) ) && sh2_load )
					tempQuat = convertToQuatUnsigned( l_sAnimAng.xQuatL, l_sAnimAng.yQuatL, l_sAnimAng.zQuatL );//(test_anim_swap_s8( flagArray[ k ] ) )?l_sAnimAng.xQuatL:0.0f, (test_anim_swap_s0( flagArray[ k ] ))?l_sAnimAng.yQuatL:0.0f, (test_anim_seq0_use1st)?l_sAnimAng.zQuatL:0.0f );
				else
				if( (test_anim_swap_s0( flagArray[ k ] ) || test_anim_swap_s8( flagArray[ k ] ) ) && sh2_load )
					tempQuat = convertToQuat( l_sAnimAng.yQuatL, l_sAnimAng.xQuatL, l_sAnimAng.zQuatL );
				else
					tempQuat = convertToQuat( l_sAnimAng.xQuatL, l_sAnimAng.yQuatL, l_sAnimAng.zQuatL );

				QUnit( &tempQuat );
				QTOM( &tempQuat, &tempMat2 );

				if(  animDebugMode && printData )
				{
					float xVal, yVal, zVal;
					tempQuat.getAxisAngles( &xVal, &yVal, &zVal );
					LogFile( ERROR_LOG, "AXIS ANGLES: x: %.3f\ty: %.3f\tz: %.3f",xVal, yVal, zVal );
				}
				QINVERSE(&tempQuat,&tempQuat);
				tempQAnim = QuatAnim( tempQuat, vertex( 0, 0, 0 ) );

	//			if( getSeq( flagArray[ k ] ) == 0 ) //|| getSeq( flagArray[ k ] ) == 0 )	
				if( ((test_anim_transpose_s0( flagArray[ k ] ) || test_anim_transpose_s8( flagArray[ k ] ) ) && sh2_load) || ( !sh2_load && (getSeq( flagArray[ k ] ) == 8 || getSeq( flagArray[ k ] ) == 0 )))
					tempMat2.transpose( );

				
				matVec2.push_back( tempMat2 );
				
				//		quatVec2.push_back( tempQAnim );
		

if( animDebugMode && printData)		{	LogFile( ERROR_LOG, "---- Alt Mat --------");debugMatrix( tempMat2 );	}

				if( ( res = fread( (void *)&l_sAnimAng, 1, sizeof(sh_anim_ang), inFile )) != sizeof( sh_anim_ang ) )
				{
					LogFile( ERROR_LOG, "SH_AnimSet::LoadAnimSet( ) - ERROR: Could not read animation angle struct for seq[%ld] = %d at offset %ld for flag[%d] = %d -  REASON: %s",j+k,ps_mSeq[j+k],preOffset, k, flagArray[k],strerror( errno ));
					return 0;
				}

				lStartTotal += res;

				if( animDebugMode )
					debugAA( &l_sAnimAng, 103);

				if( test_anim_seq8_use1st && (test_anim_swap_s0( flagArray[ k ] ) || test_anim_swap_s8( flagArray[ k ] ) ) && sh2_load )
					tempQuat = convertToQuatUnsigned( l_sAnimAng.xQuatL, l_sAnimAng.yQuatL, l_sAnimAng.zQuatL );//(test_anim_swap_s8( flagArray[ k ] ) )?l_sAnimAng.xQuatL:0.0f, (test_anim_swap_s0( flagArray[ k ] ))?l_sAnimAng.yQuatL:0.0f, (test_anim_seq0_use1st)?l_sAnimAng.zQuatL:0.0f );
				else
				if( (test_anim_swap_s0( flagArray[ k ] ) || test_anim_swap_s8( flagArray[ k ] ) ) && sh2_load )
					tempQuat = convertToQuat( l_sAnimAng.yQuatL, l_sAnimAng.xQuatL, l_sAnimAng.zQuatL );
				else
					tempQuat = convertToQuat( l_sAnimAng.xQuatL, l_sAnimAng.yQuatL, l_sAnimAng.zQuatL );

				QUnit( &tempQuat );
				QTOM( &tempQuat, &tempMat );
				if(  animDebugMode && printData )
				{
					float xVal, yVal, zVal;
					tempQuat.getAxisAngles( &xVal, &yVal, &zVal );
					LogFile( ERROR_LOG, "AXIS ANGLES: x: %.3f\ty: %.3f\tz: %.3f",xVal, yVal, zVal );
				}
				QINVERSE(&tempQuat,&tempQuat);		//---[ Needed to make quat animations work ]---/

				tempQAnim = QuatAnim( tempQuat, vertex( 0, 0, 0 ) );
				quatVec.push_back( tempQAnim );

				if( ((test_anim_transpose_s0( flagArray[ k ] ) || test_anim_transpose_s8( flagArray[ k ] ) ) && sh2_load) || ( !sh2_load && (getSeq( flagArray[ k ] ) == 8 || getSeq( flagArray[ k ] ) == 0 )))
	//			if( getSeq( flagArray[ k ] ) == 0 ) //|| getSeq( flagArray[ k ] ) == 0 )	
					tempMat.transpose( );


//				if( test_anim_use1st_s0( flagArray[ k ] ) && sh2_load )
//					tempMat = tempMat2 * tempMat;
//				else //if( test_anim_seq8_use1st ) //test_anim_use1st_s8( flagArray[ k ] ) && sh2_load )
//					tempMat = tempMat * tempMat2;
		//		if( (test_anim_use1st_s0( flagArray[ k ] ) || test_anim_use1st_s8( flagArray[ k ] ) ) && sh2_load )
		
				matVec.push_back( tempMat );
if( animDebugMode && printData)debugMatrix( tempMat );
			}
			else if( j + k < _lNumSeq && flagArray[ k ] != 0 && flagArray[ k ] & 0x01 )
			{
				if( ( res = fread( (void *)&l_sAnimAng, 1, sizeof(sh_anim_ang), inFile )) != sizeof( sh_anim_ang ) )
				{
					LogFile( ERROR_LOG, "SH_AnimSet::LoadAnimSet( ) - ERROR: Could not read animation angle struct for seq[%ld] = %d at offset %ld for flag[%d] = %d -  REASON: %s",j+k,ps_mSeq[j+k],preOffset, k, flagArray[k],strerror( errno ));
					return 0;
				}

				lStartTotal += res;

				if( animDebugMode )
					debugAA( &l_sAnimAng, 103);

				if( test_anim_seq8_use1st && (test_anim_swap_s0( flagArray[ k ] ) || test_anim_swap_s8( flagArray[ k ] ) ) && sh2_load )
					tempQuat = convertToQuatUnsigned( l_sAnimAng.xQuatL, l_sAnimAng.yQuatL, l_sAnimAng.zQuatL );//(test_anim_swap_s8( flagArray[ k ] ) )?l_sAnimAng.xQuatL:0.0f, (test_anim_swap_s0( flagArray[ k ] ))?l_sAnimAng.yQuatL:0.0f, (test_anim_seq0_use1st)?l_sAnimAng.zQuatL:0.0f );
				else
				if( (test_anim_swap_s0( flagArray[ k ] ) || test_anim_swap_s8( flagArray[ k ] ) ) && sh2_load )
					tempQuat = convertToQuat( l_sAnimAng.yQuatL, l_sAnimAng.xQuatL, l_sAnimAng.zQuatL );
				else
					tempQuat = convertToQuat( l_sAnimAng.xQuatL, l_sAnimAng.yQuatL, l_sAnimAng.zQuatL );

				QUnit( &tempQuat );
				QTOM( &tempQuat, &tempMat );
				if(  animDebugMode && printData )
				{
					float xVal, yVal, zVal;
					tempQuat.getAxisAngles( &xVal, &yVal, &zVal );
					LogFile( ERROR_LOG, "AXIS ANGLES: x: %.3f\ty: %.3f\tz: %.3f",xVal, yVal, zVal );
				}
				QINVERSE(&tempQuat,&tempQuat);		//---[ Needed to make quat animations work ]---/

				tempQAnim = QuatAnim( tempQuat, vertex( 0, 0, 0 ) );
				quatVec.push_back( tempQAnim );

				if( ((test_anim_transpose_s0( flagArray[ k ] ) || test_anim_transpose_s8( flagArray[ k ] ) ) && sh2_load) || ( !sh2_load && (getSeq( flagArray[ k ] ) == 8 || getSeq( flagArray[ k ] ) == 0 )))	
	//			if( getSeq( flagArray[ k ] ) == 0 ) //|| getSeq( flagArray[ k ] ) == 0 )	
					tempMat.transpose( );


//				if( test_anim_use1st_s0( flagArray[ k ] ) && sh2_load )
//					tempMat = tempMat2 * tempMat;
//				else //if( test_anim_seq8_use1st ) //test_anim_use1st_s8( flagArray[ k ] ) && sh2_load )
//					tempMat = tempMat * tempMat2;
		//		if( (test_anim_use1st_s0( flagArray[ k ] ) || test_anim_use1st_s8( flagArray[ k ] ) ) && sh2_load )
		
				matVec.push_back( tempMat );
if( animDebugMode && printData)debugMatrix( tempMat );
			}
			else
			{
				if( debugMode )
					LogFile( ERROR_LOG, "SH_AnimSet::LoadAnimSet( ) - NOTE: No data at offset %ld for flag[%d] = %d\t(We are at seq #%ld of %ld",preOffset, k, flagArray[k], j+k, _lNumSeq );
				if( j + k < _lNumSeq )
				{
					tempMat.identity( );
					matVec.push_back( tempMat );
					tempQuat = quat::UNIT;
					tempQAnim = QuatAnim( tempQuat, vertex( 0, 0, 0 ) );
					quatVec.push_back( tempQAnim );
				}
			}
		}


		lTotalRead += lStartTotal;

		//----[ GET THE NEXT FLAGS ]----/

		startFlagOffset = ftell( inFile );

//		if( lTotalRead == maxBytesToRead )
//		{
//			startFlagOffset = ftell( inFile );
//		}
//		else
		{
			if( ( res = fread( (void *)&nextFlag, 1, sizeof(long), inFile )) != sizeof( long ) )
			{
				LogFile( ERROR_LOG, "SH_AnimSet::LoadAnimSet( ) - ERROR: Could not read animation block flags at offset %ld  -  REASON: %s",startFlagOffset, strerror( errno ));
				return 0;
			}
			lTotalRead += res;
		}
	}

	//---[ Set File Position Back to Start of next Block
	fseek( inFile, startFlagOffset, SEEK_SET );

//	if( matVec.size( ) < _lNumSeq )
//	{
//		tempMat.identity( );
//		tempQuat = quat::UNIT;
//		tempQAnim = QuatAnim( tempQuat, vertex( 0, 0, 0 ) );
//
//		for( k = matVec.size( ); k < _lNumSeq; k++ )
//		{
//			matVec.push_back( tempMat );
//			quatVec.push_back( tempQAnim );
//		}
//	}


	this->DeleteAllData( );
	
	m_lNumFlags = flagVec.size( );
	m_lNumMatrix= matVec.size( );
	m_lNumMatrix2= matVec2.size( );

	if( m_lNumMatrix >= 300 )
	{
		LogFile( ERROR_LOG, "LoadAnimSet - ERROR: Invalid Animation Format - CANNOT LOAD");
		return 0;
	}

	m_pcMatrixSet = new matrix[ m_lNumMatrix ];
	m_pcMatrixSet2 = new matrix[ m_lNumMatrix2 ];
	m_pcQuatSet = new QuatAnim[ m_lNumMatrix ];
//	m_pcQuatSet2 = new QuatAnim[ m_lNumMatrix2 ];
	m_plFlags = new unsigned long[ m_lNumFlags ];

	for( k = 0; k < matVec.size( ); k++ )
	{
		m_pcMatrixSet[ k ] = matVec[ k ];
		m_pcQuatSet[ k ] = quatVec[ k ];
	}

	for( k = 0; k < matVec2.size( ); k++ )
	{
		m_pcMatrixSet2[ k ] = matVec2[ k ];
		//m_pcQuatSe2t[ k ] = quatVec2[ k ];
	}

	for( k = 0; k < flagVec.size( ); k++ )
		m_plFlags[ k ] = flagVec[ k ];

//	LogFile( TEST_LOG, "Seq & Flags Sanity Check\n\t%ld Seq\t%ld Flags * 8", _lNumSeq, m_lNumFlags );
//	for( j = 0; j < _lNumSeq; j+=8 )
//	{
//		for( k = 0; k < 8; k++ )
//		{
//			if( j + k < _lNumSeq )
//				LogFile( TEST_LOG, "\t%d\t%x",ps_mSeq[ j+k ], (m_plFlags[ j>>3 ] >> (4 * k )) & 0x0f );
//			else
//				LogFile( TEST_LOG, "\t---\t%x",(m_plFlags[ j>>3 ] >> (4 * k )) & 0x0f );
//		}
//	}
printData = false;
	if( debugMode )LogFile( TEST_LOG, "Loaded %ld set 1 matricies, and %ld set 2 matricies\n",m_lNumMatrix, m_lNumMatrix2);
	return m_lNumMatrix;
}
*/



//###########################################################################################

// R E T R Y   T H E   L O A D    H E R E

//##########################################################################################


int SH_AnimSet::LoadAnimSet( FILE * inFile, unsigned char *ps_mSeq, long _lNumSeq, bool sh2_load )
{
	vector<unsigned long>	flagVec;
	vector<matrix>	matVec;
	//vector<matrix>	matVec2;
	vector<QuatAnim> quatVec;
	matrix			tempMat;
	//matrix			tempMat2;
	quat			tempQuat;
	QuatAnim		tempQAnim;
	QuatAnim		tempQAnim2;
	long			startFlagOffset = ftell( inFile );
	long			preOffset;
	unsigned long	nextFlag = 0;
	long			res;
	long			lTotalRead = 0;
	long			lStartTotal = 0;
	long			k, j;
//	long			curMainFlag;
	unsigned char	flagArray[8];
	unsigned char	mainFlags[4];
	sh_anim_ang		l_sAnimAng;
	sh_anim_pos_ang	l_sAnimPosAng;
	bool			bFullSpecial = false;
	bool			bPartialSpecial = false;
	bool			bLoadTrans = false;
	bool			bEndSpecial = false;
	bool			l_bStopCondition = false;

	extern int loadOrder;
	extern int loadOrder2;
	extern int transpose;
	static bool printData = true;

	if( debugMode )
		LogFile( ERROR_LOG, "SH_AnimSet::LoadAnimSet( off: %ld ) ---------------[ S T A R T ]------------", startFlagOffset);


	if( ( res = fread( (void *)&nextFlag, 1, sizeof(long), inFile )) != sizeof( long ) )
	{
		LogFile( ERROR_LOG, "SH_AnimSet::LoadAnimSet( ) - ERROR: Could not read animation block flags at offset %ld  -  REASON: %s",startFlagOffset, strerror( errno ));
		return 0;
	}

	lTotalRead += res;


	for( j = 0; j < _lNumSeq && !l_bStopCondition ; j+=8 )
	{
		if( debugMode || animDebugMode )
			LogFile( ERROR_LOG, "SH_AnimSet::LoadAnimSet( ) - Reading animation block at offset %ld\tFlags[ %d %d %d %d ]", startFlagOffset, ((unsigned char*)&nextFlag)[0], ((unsigned char*)&nextFlag)[1], ((unsigned char*)&nextFlag)[2], ((unsigned char*)&nextFlag)[3]);
		if( debugMode || animDebugMode )
			LogFile( TEST_LOG, "SH_AnimSet # %.3d - FLAGS: [ %08x ]\tSeq: [ %.2d %.2d %.2d %.2d %.2d %.2d %.2d %.2d ] - %ld mats in vec",flagVec.size(), nextFlag, 
					getSeq( byte1_low(nextFlag) ), getSeq( byte1_high(nextFlag) ), getSeq( byte2_low(nextFlag) ), 
					getSeq( byte2_high(nextFlag) ), getSeq( byte3_low(nextFlag) ), getSeq( byte3_high(nextFlag) ), 
					getSeq( byte4_low(nextFlag) ), getSeq( byte4_high(nextFlag) ), matVec.size());

		lStartTotal = 0;

		//---[ Save Flags For Later Use ]---/
		flagVec.push_back( nextFlag );

		//---[ Save Off All Data In An Array To Make Load More Efficient ]---/
		mainFlags[ 0 ] = byte1_all( nextFlag );
		mainFlags[ 1 ] = byte2_all( nextFlag );
		mainFlags[ 2 ] = byte3_all( nextFlag );
		mainFlags[ 3 ] = byte4_all( nextFlag );
		
		flagArray[ 0 ] = byte1_low( nextFlag );
		flagArray[ 1 ] = byte1_high( nextFlag );
		flagArray[ 2 ] = byte2_low( nextFlag );
		flagArray[ 3 ] = byte2_high( nextFlag );
		flagArray[ 4 ] = byte3_low( nextFlag );
		flagArray[ 5 ] = byte3_high( nextFlag );
		flagArray[ 6 ] = byte4_low( nextFlag );
		flagArray[ 7 ] = byte4_high( nextFlag );

		//---[ Check for a stop condition after these flags ]---/
		if( mainFlags[ 2 ] == 0 && mainFlags[ 3 ] == 0 )
			l_bStopCondition = true;

		//---[ Load Up All Data For Block, And Convert To Matricies ]---/

		for( k = 0; k < 8; k++ )
		{
			preOffset = ftell( inFile );

			if( j + k < _lNumSeq )
			{
				switch( flagArray[ k ] )
				{
					case 2:
					case 6:
					case 10:
					case 14:
						if(ps_mSeq[ j + k ] != 255 && ps_mSeq[ j + k ] != -1)
						{
							if( ( res = fread( (void *)&l_sAnimAng, 1, sizeof(sh_anim_ang), inFile )) != sizeof( sh_anim_ang ) )
							{
								LogFile( ERROR_LOG, "SH_AnimSet::LoadAnimSet( ) - ERROR: Could not read animation angle struct for seq[%ld] = %d at offset %ld for flag[%d] = %d -  REASON: %s",j+k,ps_mSeq[j+k],preOffset, k, flagArray[k],strerror( errno ));
								return 0;
							}

							lStartTotal += res;

							if( sh2_load )
							{
								if( sh2_load )
								{
									switch( loadOrder )
									{
									case 0://XYZ
										tempQuat = convertToQuat( l_sAnimAng.xQuatL, l_sAnimAng.yQuatL, l_sAnimAng.zQuatL );
										break;
									case 1://XZY
										tempQuat = convertToQuat( l_sAnimAng.xQuatL, l_sAnimAng.zQuatL, l_sAnimAng.yQuatL );
										break;
									case 2://YXZ
										tempQuat = convertToQuat( l_sAnimAng.yQuatL, l_sAnimAng.xQuatL, l_sAnimAng.zQuatL );
										break;
									case 3://YZX
										tempQuat = convertToQuat( l_sAnimAng.yQuatL, l_sAnimAng.zQuatL, l_sAnimAng.xQuatL );
										break;
									case 4://ZXY
										tempQuat = convertToQuat( l_sAnimAng.zQuatL, l_sAnimAng.xQuatL, l_sAnimAng.yQuatL );
										break;
									case 5://ZYX
										tempQuat = convertToQuat( l_sAnimAng.zQuatL, l_sAnimAng.yQuatL, l_sAnimAng.xQuatL );
										break;
									}
								}
								else
									tempQuat = convertToQuat( l_sAnimAng.xQuatL, l_sAnimAng.yQuatL, l_sAnimAng.zQuatL );

								QUnit( &tempQuat );
								QTOM( &tempQuat, &tempMat );
								QINVERSE(&tempQuat,&tempQuat);
								tempQAnim = QuatAnim( tempQuat, vertex( 0, 0, 0 ) );

								if( getSeq( flagArray[ k ] ) == 8 || getSeq( flagArray[ k ] ) == 0 )
								//if( !sh2_load && getSeq( flagArray[ k ] ) == 8 || getSeq( flagArray[ k ] ) == 0 )
								//if(  sh2_load && transpose & 0x01 || (!sh2_load &&( getSeq( flagArray[ k ] ) == 8 || getSeq( flagArray[ k ] ) == 0 )))
									tempMat.transpose();

//								matVec.push_back( tempMat );
//								quatVec.push_back( tempQAnim );


							}
							if( ( res = fread( (void *)&l_sAnimAng, 1, sizeof(sh_anim_ang), inFile )) != sizeof( sh_anim_ang ) )
							{
								LogFile( ERROR_LOG, "SH_AnimSet::LoadAnimSet( ) - ERROR: Could not read animation angle struct for seq[%ld] = %d at offset %ld for flag[%d] = %d -  REASON: %s",j+k,ps_mSeq[j+k],preOffset, k, flagArray[k],strerror( errno ));
								return 0;
							}

							lStartTotal += res;

							if( sh2_load )
							{
								switch( loadOrder )
								{
								case 0://XYZ
									tempQuat = convertToQuat( l_sAnimAng.xQuatL, l_sAnimAng.yQuatL, l_sAnimAng.zQuatL );
									break;
								case 1://XZY
									tempQuat = convertToQuat( l_sAnimAng.xQuatL, l_sAnimAng.zQuatL, l_sAnimAng.yQuatL );
									break;
								case 2://YXZ
									tempQuat = convertToQuat( l_sAnimAng.yQuatL, l_sAnimAng.xQuatL, l_sAnimAng.zQuatL );
									break;
								case 3://YZX
									tempQuat = convertToQuat( l_sAnimAng.yQuatL, l_sAnimAng.zQuatL, l_sAnimAng.xQuatL );
									break;
								case 4://ZXY
									tempQuat = convertToQuat( l_sAnimAng.zQuatL, l_sAnimAng.xQuatL, l_sAnimAng.yQuatL );
									break;
								case 5://ZYX
									tempQuat = convertToQuat( l_sAnimAng.zQuatL, l_sAnimAng.yQuatL, l_sAnimAng.xQuatL );
									break;
								}
							}
							else
								tempQuat = convertToQuat( l_sAnimAng.xQuatL, l_sAnimAng.yQuatL, l_sAnimAng.zQuatL );

							QUnit( &tempQuat );
							QTOM( &tempQuat, &tempMat );
							QINVERSE(&tempQuat,&tempQuat);
							tempQAnim = QuatAnim( tempQuat, vertex( 0, 0, 0 ) );
							
							if( getSeq( flagArray[ k ] ) == 8 || getSeq( flagArray[ k ] ) == 0 )
							//if( !sh2_load && getSeq( flagArray[ k ] ) == 8 || getSeq( flagArray[ k ] ) == 0 )
							//if(  sh2_load && transpose & 0x01 || (!sh2_load &&( getSeq( flagArray[ k ] ) == 8 || getSeq( flagArray[ k ] ) == 0 )))
								tempMat.transpose();

							matVec.push_back( tempMat );
							quatVec.push_back( tempQAnim );
		
						}
						else
						{
							if( ( res = fread( (void *)&l_sAnimPosAng, 1, sizeof(sh_anim_pos_ang), inFile )) != sizeof( sh_anim_pos_ang ) )
							{
								LogFile( ERROR_LOG, "SH_AnimSet::LoadAnimSet( ) - ERROR: Could not read animation position/angle struct for seq[%ld] = %d at offset %ld for flag[%d] = %d -  REASON: %s",j+k,ps_mSeq[j+k],preOffset, k, flagArray[k],strerror( errno ));
								return 0;
							}

							lStartTotal += res;

							if( animDebugMode )
								debugAPA( &l_sAnimPosAng, 101 );

							if( sh2_load )
							{
								switch( loadOrder2 )
								{
								case 0://XYZ
									tempQuat = convertToQuat( l_sAnimPosAng.xQuatL, l_sAnimPosAng.yQuatL, l_sAnimPosAng.zQuatL );
									break;
								case 1://XZY
									tempQuat = convertToQuat( l_sAnimPosAng.xQuatL, l_sAnimPosAng.zQuatL, l_sAnimPosAng.yQuatL );
									break;
								case 2://YXZ
									tempQuat = convertToQuat( l_sAnimPosAng.yQuatL, l_sAnimPosAng.xQuatL, l_sAnimPosAng.zQuatL );
									break;
								case 3://YZX
									tempQuat = convertToQuat( l_sAnimPosAng.yQuatL, l_sAnimPosAng.zQuatL, l_sAnimPosAng.xQuatL );
									break;
								case 4://ZXY
									tempQuat = convertToQuat( l_sAnimPosAng.zQuatL, l_sAnimPosAng.xQuatL, l_sAnimPosAng.yQuatL );
									break;
								case 5://ZYX
									tempQuat = convertToQuat( l_sAnimPosAng.zQuatL, l_sAnimPosAng.yQuatL, l_sAnimPosAng.xQuatL );
									break;
								}
							}
							else
								tempQuat = convertToQuat( l_sAnimPosAng.xQuatL, l_sAnimPosAng.yQuatL, l_sAnimPosAng.zQuatL );


							QUnit( &tempQuat );
							QTOM( &tempQuat, &tempMat );
							QINVERSE(&tempQuat,&tempQuat);		//---[ Needed to Make Quat Animation Work ]---/

							tempQAnim = QuatAnim( tempQuat, vertex( l_sAnimPosAng.xPosL, l_sAnimPosAng.yPosL, l_sAnimPosAng.zPosL ) );

							if( getSeq( flagArray[ k ] ) == 8 || getSeq( flagArray[ k ] ) == 0 )
							//if( !sh2_load && getSeq( flagArray[ k ] ) == 8 || getSeq( flagArray[ k ] ) == 0 )
							//if( sh2_load && transpose & 0x10 || (!sh2_load &&( getSeq( flagArray[ k ] ) == 8 || getSeq( flagArray[ k ] ) == 0 )))
								tempMat.transpose();

							tempMat.mat[  3 ] = l_sAnimPosAng.xPosL;
							tempMat.mat[  7 ] = l_sAnimPosAng.yPosL;
							tempMat.mat[ 11 ] = l_sAnimPosAng.zPosL;

							matVec.push_back( tempMat );
							quatVec.push_back( tempQAnim );

						}							
						break;
					case 1:
					case 5:
					case 9:
					case 13:
						if( ( res = fread( (void *)&l_sAnimAng, 1, sizeof(sh_anim_ang), inFile )) != sizeof( sh_anim_ang ) )
						{
							LogFile( ERROR_LOG, "SH_AnimSet::LoadAnimSet( ) - ERROR: Could not read animation angle struct for seq[%ld] = %d at offset %ld for flag[%d] = %d -  REASON: %s",j+k,ps_mSeq[j+k],preOffset, k, flagArray[k],strerror( errno ));
							return 0;
						}

						lStartTotal += res;

						if( sh2_load )
									tempQuat = convertToQuat( l_sAnimAng.xQuatL, l_sAnimAng.yQuatL, l_sAnimAng.zQuatL );
								else
									tempQuat = convertToQuat( l_sAnimAng.xQuatL, l_sAnimAng.yQuatL, l_sAnimAng.zQuatL );


						QUnit( &tempQuat );
						QTOM( &tempQuat, &tempMat );
						QINVERSE(&tempQuat,&tempQuat);
						tempQAnim = QuatAnim( tempQuat, vertex( 0, 0, 0 ) );


						if( getSeq( flagArray[ k ] ) == 8 || getSeq( flagArray[ k ] ) == 0 )
								//if( !sh2_load && ( getSeq( flagArray[ k ] ) == 8 || getSeq( flagArray[ k ] ) == 0 ) )
									tempMat.transpose();

				
						matVec.push_back( tempMat );
						quatVec.push_back( tempQAnim );
		
						break;
					case 3:
					case 4:
					case 7:
					case 8:
					case 11:
					case 12:
					case 15:
						LogFile( DATA_LOG, "SH_AnimSet::LoadAnimSet( ) - We have a  offset %ld for flag[%d] = %d\t(We are at seq #%ld of %ld",preOffset, k, flagArray[k], j+k, _lNumSeq );
						break;
					case 0:
						tempMat.identity( );
						matVec.push_back( tempMat );
						tempQuat = quat::UNIT;
						tempQAnim = QuatAnim( tempQuat, vertex( 0, 0, 0 ) );
						quatVec.push_back( tempQAnim );
						break;
				}
			}
		}		//---[ END FOR K LOOP ]---/


		lTotalRead += lStartTotal;

		//----[ GET THE NEXT FLAGS ]----/

		startFlagOffset = ftell( inFile );

		if( ( res = fread( (void *)&nextFlag, 1, sizeof(long), inFile )) != sizeof( long ) )
		{
			LogFile( ERROR_LOG, "SH_AnimSet::LoadAnimSet( ) - ERROR: Could not read animation block flags at offset %ld  -  REASON: %s",startFlagOffset, strerror( errno ));
			return 0;
		}
		lTotalRead += res;
	}


	//---[ Set File Position Back to Start of next Block
	fseek( inFile, startFlagOffset, SEEK_SET );

	this->DeleteAllData( );
	
	m_lNumFlags = flagVec.size( );
	m_lNumMatrix= matVec.size( );
	//m_lNumMatrix2= matVec2.size( );

	if( m_lNumMatrix >= 300 )
	{
		LogFile( ERROR_LOG, "LoadAnimSet - ERROR: Invalid Animation Format - CANNOT LOAD");
		return 0;
	}

	//LogFile( ERROR_LOG, "SEG FAULT: Sizes are: NumFlags  = %ld\tNumMat = %ld",this->m_lNumFlags, this->m_lNumMatrix );
	m_pcMatrixSet = new matrix[ m_lNumMatrix ];
	m_pcQuatSet = new QuatAnim[ m_lNumMatrix ];
	m_plFlags = new unsigned long[ m_lNumFlags ];

	for( k = 0; k < matVec.size( ); k++ )
	{
		m_pcMatrixSet[ k ] = matVec[ k ];
		m_pcQuatSet[ k ] = quatVec[ k ];
	}

	for( k = 0; k < flagVec.size( ); k++ )
		m_plFlags[ k ] = flagVec[ k ];

	if( debugMode )LogFile( TEST_LOG, "Loaded %ld set 1 matricies\n",m_lNumMatrix);
	return m_lNumMatrix;
}
