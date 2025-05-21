#ifndef  __OBJ_EXPORTER_H__
#define  __OBJ_EXPORTER_H__

#include <windows.h>
#include <stdio.h>
#include <string>
#include <vector>

#include "vertex.h"
#include "RVertex.h"

using std::string;
using std::vector;


class OBJ_Exporter
{
public:
	OBJ_Exporter( ){ SetNullAll( ); }
	OBJ_Exporter( const OBJ_Exporter & rhs ){ SetNullAll( ); operator=( rhs ); }
	~OBJ_Exporter( ){ CloseFiles( ); DeleteData( ); }
	OBJ_Exporter & operator=( const OBJ_Exporter & rhs );


	void	SetNullAll( );
	void	DeleteData( );

	bool	isOBJ_Open( ){ return m_pfOBJ_File != NULL; }
	bool	isMTL_Open( ){ return m_pfMTL_File != NULL; }

	bool	CreateFiles( char *_pBasename );
	bool	CloseFiles( );
	bool	AppendFiles( char *_pBasename );

	bool	AddMaterial( string & texName, float alpha = 1.0f  );
	bool	AddMaterial( char *texName, float alpha = 1.0f );

	bool	AddGroup( string & _groupName );
	bool	AddGroup( );

	bool	AddVertex( vertex & _vert );
	bool	AddNormal( vertex & _norm );
	bool	AddTexcoord( texcoord & _tc );
	bool	AddFace( unsigned int *index, bool _hasTC = true, bool _hasNorm = true );
	bool	AddFace( int *index, bool _hasTC = true, bool _hasNorm = true );
	bool	AddFace( long *index, bool _hasTC = true, bool _hasNorm = true );
	bool	AddFace( vector<long> & index, bool _hasTC = true, bool _hasNorm = true );

	void	AddComment( char *s, ...);

	void	DecodeTriStrip( bool setting = true ){ m_bDecodeTristrip = setting; }
	void	StartNewVertexRange( );
	//---[ Member Variables ]---/
	string		m_sOutFile;
	string		m_sOutMaterial;
	long		m_lCurVert;
	long		m_lCurTexcoord;
	long		m_lCurNormal;
	long		m_lStartVert;
	long		m_lStartTexcoord;
	long		m_lStartNormal;
	long		m_lCurGroup;
	bool		m_bDecodeTristrip;
	bool		m_bReverseDir;
	FILE		*m_pfOBJ_File;
	FILE		*m_pfMTL_File;
	vector<string>		m_vAddedMaterials;


};

#endif /*__OBJ_EXPORTER_H__*/
