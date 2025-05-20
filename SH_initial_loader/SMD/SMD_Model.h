#ifndef  __SMD_MODEL_H__
#define  __SMD_MODEL_H__

#include <windows.h>
#include <matrix.h>
#include <mvector.h>
#include <mstring.h>
#include <fileCrawler.h>
#include "SMD_Triangle.h"
#include "SMD_Animation_Frame.h"

using mstd::mstring;
using mstd::mvector;
using std::ostream;

namespace SMD
{

class SMD_Model
{
public:
	SMD_Model( ){ SetNullAll( ); }
	SMD_Model( const SMD_Model & rhs ){ SetNullAll( ); operator=( rhs ); }
	~SMD_Model( ){ DeleteData( ); }
	SMD_Model & operator=( const SMD_Model & rhs );


	void	SetNullAll( );
	void	DeleteData( );

	//---[ Member Methods ]---/
	friend	ostream &	operator<<( ostream & _os, SMD_Model & _m );
	bool		Load( FileCrawler & _is );
	bool		Load( mstring & _filename );
	void		AddAnimationFrame( SMD_Animation_Frame & _frame );
	void		AddTriangle( SMD_Triangle & _tri );
	void		AddNode( long _parent );
	void		AddNode( char _parent );
	


	//---[ Get / Set Member ]---/
	mvector< SMD_Triangle >	getFaces( ){ return m_Faces; }
	void				setFaces( mvector< SMD_Triangle > & _Faces ){ m_Faces = _Faces; }

	mvector< SMD_Animation_Frame >	getAnimations( ){ return m_Animations; }
	void				setAnimations( mvector< SMD_Animation_Frame > & _Animations ){ m_Animations = _Animations; }

	mvector< long >		getNodes( ){ return m_Nodes; }
	void				setNodes( mvector< long > & _Nodes ){ m_Nodes = _Nodes; }


	//---[ Member Variables ]---/
	mvector< SMD_Triangle >		m_Faces;
	mvector< SMD_Animation_Frame >		m_Animations;
	mvector< long >		m_Nodes;


};


};	//END naemspace SMD

#endif /*__SMD_MODEL_H__*/
