#include <windows.h>
#include <vertex.h>
#include <mvector.h>
#include <mstring.h>
#include "SMD_Vertex.h"
#include "SMD_Triangle.h"


namespace SMD
{

//-------------------------/
//-                       -/
//-                       -/
//-------------------------/
SMD_Triangle & SMD_Triangle::operator=( const SMD_Triangle & rhs )
{
	if( & rhs != this )
	{
		m_Verts		=	rhs.m_Verts;
		m_Material		=	rhs.m_Material;

	}
	return *this;
}


//-------------------------/
//-                       -/
//-                       -/
//-------------------------/
void	SMD_Triangle::SetNullAll( )
{
	m_Material = "";
}


//-------------------------/
//-                       -/
//-                       -/
//-------------------------/
void	SMD_Triangle::DeleteData( )
{
	m_Verts.clear( );
	
	SetNullAll( ); 
}



ostream &		operator<<( ostream & _os, SMD_Triangle & _t  )
{
	_os	<<	_t.m_Material	<<	std::endl;

	for( long k = 0; k < _t.m_Verts.size(); k++ )
		_os	<<	_t.m_Verts[ k ]	<<	std::endl;
	for( long k = _t.m_Verts.size(); k < 3;	k++ )
	{
		SMD_Vertex	l_Dummy;
		_os	<<	l_Dummy	<<	std::endl;
	}

	return	_os;
}

bool			SMD_Triangle::Load( FileCrawler & _is )
{
	return false;
}


void			SMD_Triangle::AddVertex( SMD_Vertex & _vert )
{
	if( this->m_Verts.size() > 3 )
		return;

	this->m_Verts.push_back( _vert );
}


};	//END namespace SMD