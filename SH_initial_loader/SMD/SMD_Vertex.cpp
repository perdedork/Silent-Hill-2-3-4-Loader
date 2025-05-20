#include <windows.h>
#include <vertex.h>
#include <mvector.h>
#include <iomanip>
#include "SMD_Vertex.h"

namespace SMD
{

//-------------------------/
//-                       -/
//-                       -/
//-------------------------/
SMD_Vertex & SMD_Vertex::operator=( const SMD_Vertex & rhs )
{
	if( & rhs != this )
	{
		m_Position		=	rhs.m_Position;
		m_Normal		=	rhs.m_Normal;
		m_Texcoord		=	rhs.m_Texcoord;
		m_Weights		=	rhs.m_Weights;
		m_Nodes			=	rhs.m_Nodes;
		m_Parent		=	rhs.m_Parent;
	}
	return *this;
}


//-------------------------/
//-                       -/
//-                       -/
//-------------------------/
void	SMD_Vertex::SetNullAll( )
{
	m_Position		=	vertex( 0.0f, 0.0f, 0.0f );
	m_Normal		=	vertex( 0.0f, 0.0f, 0.0f );
	m_Texcoord		=	texcoord( 0.0f, 0.0f );
	m_Parent		=	0;

}


//-------------------------/
//-                       -/
//-                       -/
//-------------------------/
void	SMD_Vertex::DeleteData( )
{
	//m_Position.DeleteData( );
	//m_Normal.DeleteData( );
	//m_Texcoord.DeleteData( );
	m_Weights.clear( );
	m_Nodes.clear( );
	//m_Parentn
	SetNullAll( ); 
}


ostream &	operator<<( ostream & _os, SMD_Vertex & _v )
{
	_os	<<	_v.m_Parent	<<	" ";
	_os	<<	std::setprecision( 6 )	<<	_v.m_Position.x	<<	" "	<<	_v.m_Position.y	<<	" "	<<	_v.m_Position.z	<<	" ";
	_os	<<	std::setprecision( 6 )	<<	_v.m_Normal.x	<<	" "	<<	_v.m_Normal.y	<<	" "	<<	_v.m_Normal.z	<<	" ";
	_os	<<	std::setprecision( 6 )	<<	_v.m_Texcoord.s	<<	" "	<<	_v.m_Texcoord.t	<<	" ";
	_os	<<	_v.m_Nodes.size( );

	for( long k = 0; k < _v.m_Nodes.size( ); k++ )
	{
		_os		<< " "	<<	_v.m_Nodes[ k ];
		_os		<< " "	<<	_v.m_Weights[ k ];
	}

	return _os;
}

bool	SMD_Vertex::Load( FileCrawler & _is )
{
	mstring	l_CurLine;

	return true;
}


void	SMD_Vertex::AddWeight( float _weight, long _node )
{
	if( _weight < 0.0f || _node < 0 )
		return;

	this->m_Nodes.push_back( _node );
	this->m_Weights.push_back( _weight );
}
	

};	//END namespace SMD