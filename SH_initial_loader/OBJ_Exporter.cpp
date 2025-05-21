#include <windows.h>
#include <stdio.h>
#include <string>
#include <vector>
#include <errno.h>

#include "vertex.h"
#include "RVertex.h"

#include "OBJ_Exporter.h"
#include "typedefs.h"

//-------------------------/
//-                       -/
//-                       -/
//-------------------------/
OBJ_Exporter & OBJ_Exporter::operator=( const OBJ_Exporter & rhs )
{
	if( & rhs != this )
	{
		m_sOutFile			=	rhs.m_sOutFile;
		m_sOutMaterial		=	rhs.m_sOutMaterial;
		m_lCurVert			=	rhs.m_lCurVert;
		m_lCurTexcoord		=	rhs.m_lCurTexcoord;
		m_lCurNormal		=	rhs.m_lCurNormal;
		m_lStartVert		=	rhs.m_lStartVert;
		m_lStartTexcoord	=	rhs.m_lStartTexcoord;
		m_lStartNormal		=	rhs.m_lStartNormal;
		m_lCurGroup			=	rhs.m_lCurGroup;
		m_pfOBJ_File		=	NULL;
		m_pfMTL_File		=	NULL;
		m_vAddedMaterials	=	rhs.m_vAddedMaterials;
		m_bDecodeTristrip	=	rhs.m_bDecodeTristrip;
		m_bReverseDir		=	rhs.m_bReverseDir;
	}
	return *this;
}


//-------------------------/
//-                       -/
//-                       -/
//-------------------------/
void	OBJ_Exporter::SetNullAll( )
{
	m_sOutFile		=	string("");
	m_sOutMaterial	=	string("");
	m_lCurVert		=	1;
	m_lCurTexcoord	=	1;
	m_lCurNormal	=	1;
	m_lStartVert	=	1;
	m_lStartTexcoord=	1;
	m_lStartNormal	=	1;
	m_lCurGroup		=	0;
	m_pfOBJ_File	=	NULL;
	m_pfMTL_File	=	NULL;
	m_bDecodeTristrip=	false;
	m_bReverseDir	=	false;
	//m_vAddedMaterials

}


//-------------------------/
//-                       -/
//-                       -/
//-------------------------/
void	OBJ_Exporter::DeleteData( )
{
	//m_sOutFile
	//m_sOutMaterial
	//m_lCurVertn	//m_lCurTexcoordn	//m_lCurNormaln	delete [] m_pfOBJ_File;
	//delete [] m_pfMTL_File;
	m_vAddedMaterials.clear( );

	SetNullAll( ); 
}


//-------------------------/
//-                       -/
//-                       -/
//-------------------------/
bool	OBJ_Exporter::CreateFiles( char *_pBasename )
{
	if( ! _pBasename )
	{
		LogFile( ERROR_LOG, "OBJ_Exporter::CreateFiles( ) - ERROR: No base filename was passed");
		return false;
	}

	//---[ Open the Material File, Overwriting Anything There ]---/
	if( ! isMTL_Open( ) )
	{
		m_sOutMaterial = string( _pBasename ) + ".mtl";
		if( ( m_pfMTL_File = fopen( m_sOutMaterial.c_str( ), "w+" ) ) == NULL )
		{
			LogFile( ERROR_LOG, "OBJ_Exporter::CreateFiles( ) - ERROR: Could not open '%s' for writing - REASON: %s", m_sOutMaterial.c_str( ), strerror( errno ) );
			return false;
		}

		fprintf( m_pfMTL_File, "# Silent Hill Viewer - Model Export\n#\n# Created by perdedork - E-mail: perdedork@yahoo.com\n#\n\n");
	}
	else
	{
		LogFile( ERROR_LOG, "OBJ_Exporter::CreateFiles( ) - ERROR: Material file already open - must close first");
		return false;
	}


	if( ! isOBJ_Open( ) )
	{
		m_sOutFile = string( _pBasename ) + ".obj";
		if( ( m_pfOBJ_File = fopen( m_sOutFile.c_str( ), "w+" ) ) == NULL )
		{
			LogFile( ERROR_LOG, "OBJ_Exporter::CreateFiles( ) - ERROR: Could not open '%s' for writing - REASON: %s", m_sOutFile.c_str( ), strerror( errno ) );
			return false;
		}

		fprintf( m_pfOBJ_File, "# Silent Hill Viewer - Model Export\n#\n# Created by perdedork - E-mail: perdedork@yahoo.com\n#\n\nmtllib %s\n\n", m_sOutMaterial.c_str( ) );
	}
	else
	{
		LogFile( ERROR_LOG, "OBJ_Exporter::CreateFiles( ) - ERROR: Object file already open - must close first");
		return false;
	}

	return true;
}


//-------------------------/
//-                       -/
//-                       -/
//-------------------------/
bool	OBJ_Exporter::CloseFiles( )
{
	if( m_pfOBJ_File != NULL )
		fclose( m_pfOBJ_File );
	if( m_pfMTL_File != NULL )
		fclose( m_pfMTL_File );

	SetNullAll( );

	return true;
}


//-------------------------/
//-                       -/
//-                       -/
//-------------------------/
bool	OBJ_Exporter::AppendFiles( char *_pBasename )
{
	if( ! _pBasename )
	{
		LogFile( ERROR_LOG, "OBJ_Exporter::AppendFiles( ) - ERROR: No base filename was passed");
		return false;
	}

	//---[ Open the Material File, Overwriting Anything There ]---/
	if( ! isMTL_Open( ) )
	{
		m_sOutMaterial = string( _pBasename ) + ".mtl";
		if( ( m_pfMTL_File = fopen( m_sOutMaterial.c_str( ), "a+" ) ) == NULL )
		{
			LogFile( ERROR_LOG, "OBJ_Exporter::AppendFiles( ) - ERROR: Could not open '%s' for writing - REASON: %s", m_sOutMaterial.c_str( ), strerror( errno ) );
			return false;
		}

		fprintf( m_pfMTL_File, "# Silent Hill Viewer - Model Export\n#\n# Created by perdedork - E-mail: perdedork@yahoo.com\n#\n\n");
	}
	else
	{
		LogFile( ERROR_LOG, "OBJ_Exporter::AppendFiles( ) - ERROR: Material file already open - must close first");
		return false;
	}


	if( ! isOBJ_Open( ) )
	{
		m_sOutFile = string( _pBasename ) + ".obj";
		if( ( m_pfOBJ_File = fopen( m_sOutFile.c_str( ), "a+" ) ) == NULL )
		{
			LogFile( ERROR_LOG, "OBJ_Exporter::AppendFiles( ) - ERROR: Could not open '%s' for writing - REASON: %s", m_sOutFile.c_str( ), strerror( errno ) );
			return false;
		}

		fprintf( m_pfOBJ_File, "# Silent Hill Viewer - Model Export\n#\n# Created by perdedork - E-mail: perdedork@yahoo.com\n#\n\nmtllib %s\n", m_sOutMaterial.c_str( ) );
	}
	else
	{
		LogFile( ERROR_LOG, "OBJ_Exporter::AppendFiles( ) - ERROR: Object file already open - must close first");
		return false;
	}

	return true;
}





//-------------------------/
//-                       -/
//-                       -/
//-------------------------/
bool	OBJ_Exporter::AddMaterial( string & texName, float alpha )
{
	string alphaName( texName );
	//---[ Ensure that Material File is Open for Writing ]---/
	if( ! isMTL_Open( ) )
	{
		LogFile( ERROR_LOG, "OBJ_Exporter::AddMaterial( ) - ERROR: Material File is not open - Cannot add material '%s_mtl'",texName.c_str( ) );
		return false;
	}

	//---[ Ensure that Object File is Open for Writing ]---/
	if( ! isOBJ_Open( ) )
	{
		LogFile( ERROR_LOG, "OBJ_Exporter::AddVertex( ) - ERROR: Object File is not open - Cannot add material '%s_mtl'",texName.c_str( ) );
		return false;
	}

	if( alpha < 1.0f )
		alphaName+= "alpha";
	//---[ Check if Material Already Added ]---/
	for( long k = 0; k < m_vAddedMaterials.size( ); k++ )
	{
		if( m_vAddedMaterials[ k ] == alphaName )
		{
			fprintf( m_pfOBJ_File, "\nusemtl  %s_mtl\n\n", alphaName.c_str( ) );
			LogFile( ERROR_LOG, "OBJ_Exporter::AddMaterial( ) - NOTE: Material '%s_mtl' already in file.  Not added again.", alphaName.c_str( ) );
			return false;
		}
	}

	//---[ Add Material to List and to File ]---/
	m_vAddedMaterials.push_back( alphaName );

	fprintf( m_pfMTL_File, "newmtl  %s_mtl\nKd      1.0000 1.000 1.000\nKa      1.0000 1.000 1.000\nKs      1.0000 1.000 1.000\nillum 0\nd %f\nmap_Kd %s.tga\n\n\n",alphaName.c_str( ), alpha, texName.c_str( ) );

	fprintf( m_pfOBJ_File, "\nusemtl  %s_mtl\n\n", alphaName.c_str( ) );
	return true;
}




//-------------------------/
//-                       -/
//-                       -/
//-------------------------/
bool	OBJ_Exporter::AddMaterial( char *texName, float alpha )
{
	string texString( texName );

	if( !texName )
	{
		LogFile( ERROR_LOG, "OBJ_Exporter::AddMaterial( ) - ERROR: Null texture name passed");
		return false;
	}

	return AddMaterial( texString, alpha );
}



//-------------------------/
//-                       -/
//-                       -/
//-------------------------/
bool	OBJ_Exporter::AddGroup( string & _groupName )
{
	//---[ Ensure that Object File is Open for Writing ]---/
	if( ! isOBJ_Open( ) )
	{
		LogFile( ERROR_LOG, "OBJ_Exporter::AddGroup( ) - ERROR: Object File is not open - Cannot add group 'gr_%s'", _groupName.c_str( ) );
		return false;
	}

	fprintf( m_pfOBJ_File, "\n\n\ng  gr_%s\n\n", _groupName.c_str( ) );

	m_bReverseDir = false;

	return true;
}




//-------------------------/
//-                       -/
//-                       -/
//-------------------------/
bool	OBJ_Exporter::AddGroup( )
{
	char	l_pcGroupNum[ 20 ];

	sprintf( l_pcGroupNum, "%ld", m_lCurGroup );

	++m_lCurGroup;

	return AddGroup( string( l_pcGroupNum ) );
}


//-------------------------/
//-                       -/
//-                       -/
//-------------------------/
bool	OBJ_Exporter::AddVertex( vertex & _vert )
{
	//---[ Ensure that Object File is Open for Writing ]---/
	if( ! isOBJ_Open( ) )
	{
		LogFile( ERROR_LOG, "OBJ_Exporter::AddVertex( ) - ERROR: Object File is not open - Cannot add vertex" );
		return false;
	}

	fprintf( m_pfOBJ_File, "v  %f %f %f\n", _vert.x, _vert.y, _vert.z );

	m_lCurVert++;

	return true;
}



//-------------------------/
//-                       -/
//-                       -/
//-------------------------/
bool	OBJ_Exporter::AddNormal( vertex & _norm )
{
	//---[ Ensure that Object File is Open for Writing ]---/
	if( ! isOBJ_Open( ) )
	{
		LogFile( ERROR_LOG, "OBJ_Exporter::AddNormal( ) - ERROR: Object File is not open - Cannot add normal" );
		return false;
	}

	fprintf( m_pfOBJ_File, "vn  %f %f %f\n", _norm.x, _norm.y, _norm.z );

	m_lCurNormal++;

	return true;
}



//-------------------------/
//-                       -/
//-                       -/
//-------------------------/
bool	OBJ_Exporter::AddTexcoord( texcoord & _tc )
{
	//---[ Ensure that Object File is Open for Writing ]---/
	if( ! isOBJ_Open( ) )
	{
		LogFile( ERROR_LOG, "OBJ_Exporter::AddTexcoord( ) - ERROR: Object File is not open - Cannot add texcoord" );
		return false;
	}

	fprintf( m_pfOBJ_File, "vt  %f %f 0\n", _tc.s, _tc.t );

	m_lCurTexcoord++;

	return true;
}



bool	OBJ_Exporter::AddFace( unsigned int *index, bool _hasTC, bool _hasNorm )
{
	//---[ Ensure that Object File is Open for Writing ]---/
	if( ! isOBJ_Open( ) )
	{
		LogFile( ERROR_LOG, "OBJ_Exporter::AddFace( ) - ERROR: Object File is not open - Cannot add face" );
		return false;
	}

	if( !index )
	{
		LogFile( ERROR_LOG, "OBJ_Exporter::AddFace( ) - ERROR: No index data was passed");
		return false;
	}

	vector< long > _index;

	if( m_bDecodeTristrip && m_bReverseDir )
	{
		_index.push_back( index[ 1 ] );
		_index.push_back( index[ 0 ] );
		_index.push_back( index[ 2 ] );
		m_bReverseDir = false;
	}
	else
	{
		_index.push_back( index[ 0 ] );
		_index.push_back( index[ 1 ] );
		_index.push_back( index[ 2 ] );
		m_bReverseDir = true;
	}

	return AddFace( _index, _hasTC, _hasNorm );
}


bool	OBJ_Exporter::AddFace( int *index, bool _hasTC, bool _hasNorm )
{
	//---[ Ensure that Object File is Open for Writing ]---/
	if( ! isOBJ_Open( ) )
	{
		LogFile( ERROR_LOG, "OBJ_Exporter::AddFace( ) - ERROR: Object File is not open - Cannot add face" );
		return false;
	}

	if( !index )
	{
		LogFile( ERROR_LOG, "OBJ_Exporter::AddFace( ) - ERROR: No index data was passed");
		return false;
	}

	vector< long > _index;

	if( m_bDecodeTristrip && m_bReverseDir )
	{
		_index.push_back( index[ 1 ] );
		_index.push_back( index[ 0 ] );
		_index.push_back( index[ 2 ] );
		m_bReverseDir = false;
	}
	else
	{
		_index.push_back( index[ 0 ] );
		_index.push_back( index[ 1 ] );
		_index.push_back( index[ 2 ] );
		m_bReverseDir = true;
	}

	return AddFace( _index, _hasTC, _hasNorm );
}


bool	OBJ_Exporter::AddFace( long *index, bool _hasTC, bool _hasNorm )
{
	//---[ Ensure that Object File is Open for Writing ]---/
	if( ! isOBJ_Open( ) )
	{
		LogFile( ERROR_LOG, "OBJ_Exporter::AddFace( ) - ERROR: Object File is not open - Cannot add face" );
		return false;
	}

	if( !index )
	{
		LogFile( ERROR_LOG, "OBJ_Exporter::AddFace( ) - ERROR: No index data was passed");
		return false;
	}

	vector< long > _index;

	if( m_bDecodeTristrip && m_bReverseDir )
	{
		_index.push_back( index[ 1 ] );
		_index.push_back( index[ 0 ] );
		_index.push_back( index[ 2 ] );
		m_bReverseDir = false;
	}
	else
	{
		_index.push_back( index[ 0 ] );
		_index.push_back( index[ 1 ] );
		_index.push_back( index[ 2 ] );
		m_bReverseDir = true;
	}

	return AddFace( _index, _hasTC, _hasNorm );
}

bool	OBJ_Exporter::AddFace( vector<long> & index, bool _hasTC, bool _hasNorm )
{
	//---[ Ensure that Object File is Open for Writing ]---/
	if( ! isOBJ_Open( ) )
	{
		LogFile( ERROR_LOG, "OBJ_Exporter::AddFace( ) - ERROR: Object File is not open - Cannot add face" );
		return false;
	}


	if( index.size( ) < 3 )
	{
		LogFile( ERROR_LOG, "OBJ_Exporter::AddFace( ) - ERROR: Index list only has %ld elements - Cannot add face", index.size( ) );
		return false;
	}

	if( _hasTC )
	{
		if( _hasNorm )	//---[ Had Vert, TC, and Norm ]---/
			fprintf( m_pfOBJ_File, "f  %ld/%ld/%ld %ld/%ld/%ld %ld/%ld/%ld\n",
				index[ 0 ] + this->m_lStartVert, 
				index[ 0 ] + this->m_lStartTexcoord, 
				index[ 0 ] + this->m_lStartNormal,
				index[ 1 ] + this->m_lStartVert,
				index[ 1 ] + this->m_lStartTexcoord,
				index[ 1 ] + this->m_lStartNormal,
				index[ 2 ] + this->m_lStartVert,
				index[ 2 ] + this->m_lStartTexcoord,
				index[ 2 ] + this->m_lStartNormal );
		else			//---[ Has Vert and TC ]---/
			fprintf( m_pfOBJ_File, "f  %ld/%ld %ld/%ld %ld/%ld\n",
				index[ 0 ] + this->m_lStartVert,
				index[ 0 ] + this->m_lStartTexcoord,
				index[ 1 ] + this->m_lStartVert,
				index[ 1 ] + this->m_lStartTexcoord,
				index[ 2 ] + this->m_lStartVert,
				index[ 2 ] + this->m_lStartTexcoord );
	}
	else if( _hasNorm )	//---[ Has Vert and Norm ]---/
	{
		fprintf( m_pfOBJ_File, "f  %ld//%ld %ld//%ld %ld//%ld\n",
			index[ 0 ] + this->m_lStartVert,
			index[ 0 ] + this->m_lStartNormal,
			index[ 1 ] + this->m_lStartVert,
			index[ 1 ] + this->m_lStartNormal,
			index[ 2 ] + this->m_lStartVert,
			index[ 2 ] + this->m_lStartNormal );
	}
	else				//---[ Has Vert ]---/
	{
		fprintf( m_pfOBJ_File, "f  %ld %ld %ld\n",
			index[ 0 ] + this->m_lStartVert,
			index[ 1 ] + this->m_lStartVert,
			index[ 2 ] + this->m_lStartVert );
	}

	return true;

}


void	OBJ_Exporter::AddComment( char *s, ...)
{
	char l_pcWriteBuf[4096];
	va_list l_vaArgs;

	//---[ Check To Ensure Value Was Passed ]---/
	if( ! s )
		return;

	//---[ Ensure OBJ File is Open ]---/
	if( !isOBJ_Open( ) )
	{
		LogFile( ERROR_LOG, "OBJ_Exporter::AddComment( ) - ERROR: Couldn't Add Comment '%s' - File not open",s);
		return;
	}

	//---[ Translate The Arguments Passed To Method ]---/
	va_start( l_vaArgs, s );
	vsprintf( l_pcWriteBuf, s, l_vaArgs );
	va_end( l_vaArgs );

	//---[ Write Out The Comment, Adding The Comment Character, And A Newline ]---/
	fprintf( m_pfOBJ_File, "# %s\n", l_pcWriteBuf );

}


void	OBJ_Exporter::StartNewVertexRange( )
{
	m_lStartVert		=	m_lCurVert;
	m_lStartTexcoord	=	m_lCurTexcoord;
	m_lStartNormal		=	m_lCurNormal;
}