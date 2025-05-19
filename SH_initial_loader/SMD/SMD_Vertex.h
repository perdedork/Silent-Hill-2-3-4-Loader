#ifndef  __SMD_VERTEX_H__
#define  __SMD_VERTEX_H__

#include <windows.h>
#include <vertex.h>
#include <mvector.h>
#include <fileCrawler.h>
#include <RVertex.h>

using mstd::mvector;
using std::ostream;

namespace SMD
{

class SMD_Vertex
{
public:
	SMD_Vertex( ){ SetNullAll( ); }
	SMD_Vertex( const SMD_Vertex & rhs ){ SetNullAll( ); operator=( rhs ); }
	~SMD_Vertex( ){ DeleteData( ); }
	SMD_Vertex & operator=( const SMD_Vertex & rhs );


	void	SetNullAll( );
	void	DeleteData( );

	//---[ Member Methods ]---/
	friend	ostream &	operator<<( ostream & _os, SMD_Vertex & _v );
	bool		Load( FileCrawler & _is );
	void		AddWeight( float _weight, long _node );

	//---[ Get / Set Member ]---/
	vertex		getPosition( ){ return m_Position; }
	void		setPosition( vertex & _Position ){ m_Position = _Position; }

	vertex		getNormal( ){ return m_Normal; }
	void		setNormal( vertex & _Normal ){ m_Normal = _Normal; }

	texcoord	getTexcoord( ){ return m_Texcoord; }
	void		setTexcoord( texcoord & _Texcoord ){ m_Texcoord = _Texcoord; }

	mvector< float >	getWeights( ){ return m_Weights; }
	void		setWeights( mvector< float > & _Weights ){ m_Weights = _Weights; }

	mvector< long >	getNodes( ){ return m_Nodes; }
	void		setNodes( mvector< long > & _Nodes ){ m_Nodes = _Nodes; }

	long		getParent( ){ return m_Parent; }
	void		setParent( long _Parent ){ m_Parent = _Parent; }


	//---[ Member Variables ]---/
	
	
	vertex				m_Position;
	vertex				m_Normal;
	texcoord			m_Texcoord;
	mvector< float >	m_Weights;
	mvector< long >		m_Nodes;
	long				m_Parent;
};



};	//END namespace SMD

#endif /*__SMD_VERTEX_H__*/
