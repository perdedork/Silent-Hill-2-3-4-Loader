#ifndef  __SMD_TRIANGLE_H__
#define  __SMD_TRIANGLE_H__

#include <windows.h>
#include <vertex.h>
#include <mvector.h>
#include <mstring.h>
#include "SMD_Vertex.h"

using mstd::mvector;
using mstd::mstring;
using std::ostream;

namespace SMD
{

class SMD_Triangle
{
public:
	SMD_Triangle( ){ SetNullAll( ); }
	SMD_Triangle( const SMD_Triangle & rhs ){ SetNullAll( ); operator=( rhs ); }
	~SMD_Triangle( ){ DeleteData( ); }
	SMD_Triangle & operator=( const SMD_Triangle & rhs );


	void	SetNullAll( );
	void	DeleteData( );

	//---[ Member Methods ]---/
	friend	ostream &	operator<<( ostream & _os, SMD_Triangle & _t );
	bool			Load( FileCrawler & _is );
	void			AddVertex( SMD_Vertex & _vert );
	

	//---[ Get / Set Member ]---/
	mvector< SMD_Vertex >	getVerts( ){ return m_Verts; }
	void			setVerts( mvector< SMD_Vertex > & _Verts ){ m_Verts = _Verts; }

	mstring			getMaterial( ){ return m_Material; }
	void			setMaterial( mstring & _Material ){ m_Material = _Material; }


	//---[ Member Variables ]---/
	mvector< SMD_Vertex >	m_Verts;
	mstring							m_Material;


};


};	//END namespace SMD

#endif /*__SMD_TRIANGLE_H__*/
