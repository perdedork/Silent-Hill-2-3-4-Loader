#include <windows.h>
#include <matrix.h>
#include <mvector.h>
#include <mstring.h>
#include <fileCrawler.h>
#include <errno.h>
#include "SMD_Triangle.h"
#include "SMD_Animation_Frame.h"
#include "SMD_Model.h"

extern int errno;


namespace SMD
{

//-------------------------/
//-                       -/
//-                       -/
//-------------------------/
SMD_Model & SMD_Model::operator=( const SMD_Model & rhs )
{
	if( & rhs != this )
	{
		m_Faces			=	rhs.m_Faces;
		m_Animations	=	rhs.m_Animations;
		m_Nodes			=	rhs.m_Nodes;
	}
	return *this;
}


//-------------------------/
//-                       -/
//-                       -/
//-------------------------/
void	SMD_Model::SetNullAll( )
{
	
}


//-------------------------/
//-                       -/
//-                       -/
//-------------------------/
void	SMD_Model::DeleteData( )
{
	m_Faces.clear( );
	m_Animations.clear( );
	m_Nodes.clear( );

	SetNullAll( ); 
}

ostream &	operator<<( ostream & _os, SMD_Model & _m )
{
	_os	<<	"version 1"	<<	std::endl;
	_os	<<	"nodes"	<<	std::endl;

	for( long k = 0; k < _m.m_Nodes.size(); k++ )
	{
		_os	<<	k	<<	" \""	<<	k	<<	"\" "	<<	_m.m_Nodes[ k ]	<<	std::endl;
	}

	_os	<<	"end"	<<	std::endl;
	_os	<<	"skeleton"	<<	std::endl;
	
	for( long k = 0; k < _m.m_Animations.size(); k++ )
	{
		_os	<<	"time "	<<	k	<<	std::endl;
		_os	<<	_m.m_Animations[ k ];
	}

	_os	<<	"end"	<<	std::endl;


	if( _m.m_Faces.size() > 0 )
	{
		_os	<<	"triangles"	<<	std::endl;
	
		for( long k = 0; k < _m.m_Faces.size(); k++ )
		{
			_os	<<	_m.m_Faces[ k ];
		}

		_os	<<	"end"	<<	std::endl;
	}

	return _os;
}

bool		SMD_Model::Load( FileCrawler & _is )
{
	mstring l_curLine;

	this->DeleteData();

	while( ! _is.isAtEnd() )
	{
		//---[ Get The Current Line ]---/
		l_curLine = _is.readLine( ).toLower();

		//---[ Use Tag To Determine Action ]---/
		if( l_curLine.substr( 0, 7 ) == "version" )
		{
			continue;
		}
		else if( l_curLine == "nodes" )
		{

		}
		else if( l_curLine == "skeleton" )
		{
			SMD::SMD_Animation_Frame l_curFrame;

			l_curLine = _is.peekLine().toLower();
	
			while( l_curFrame.Load( _is ) )
			{
				this->AddAnimationFrame( l_curFrame );
			}
		}
		else if( l_curLine == "triangles" )
		{
			SMD_Triangle l_curTriangle;

			while( l_curTriangle.Load( _is ) )
			{
				this->AddTriangle( l_curTriangle );
			}
		}
		else if( l_curLine == "vertexanimation" )
		{
			while( l_curLine != "end" )
			{
				l_curLine = _is.readLine().toLower();
			}
		}
		else if( l_curLine[ 0 ] == '/' || l_curLine[ 0 ] == ';' || l_curLine[ 0 ] == '#' )
		{
			continue;
		}
		else
		{
			LogFile( ERROR_LOG, "%s( ) - Unknown Line Tag [%s] in file '%s'",__FUNCTION__, l_curLine.c_str(), _is.m_sFilename.c_str() );
			continue;
		}

	}		//---[ End of While Loop ]---/

	return true;
}


bool		SMD_Model::Load( mstring & _filename )
{
	FileCrawler	l_inFile;

	if( ! l_inFile.Open( _filename.c_str() ) )
	{
		LogFile( ERROR_LOG, "%s() - Unable to open file '%s' - %s",__FUNCTION__,_filename.c_str(), strerror( errno ) );
		return false;
	}

	return Load( l_inFile );
}

void		SMD_Model::AddAnimationFrame( SMD_Animation_Frame & _frame )
{
	this->m_Animations.push_back( _frame );
}

void		SMD_Model::AddTriangle( SMD_Triangle & _tri )
{
	this->m_Faces.push_back( _tri );
}

void		SMD_Model::AddNode( long _parent )
{
	this->m_Nodes.push_back( _parent );
}

void		SMD_Model::AddNode( char _parent )
{
	this->m_Nodes.push_back( static_cast<long>(_parent) );
}

};	//END namespace SMD