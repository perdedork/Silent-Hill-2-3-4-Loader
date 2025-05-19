#ifndef  __SMD_ANIMATION_FRAME_H__
#define  __SMD_ANIMATION_FRAME_H__


#include <windows.h>
#include <matrix.h>
#include <mvector.h>
#include "SMD_Vertex.h"
#include "SMD_Triangle.h"

using mstd::mvector;
using std::ostream;

namespace SMD
{

class SMD_Animation_Frame
{
public:
	SMD_Animation_Frame( ){ SetNullAll( ); }
	SMD_Animation_Frame( const SMD_Animation_Frame & rhs ){ SetNullAll( ); operator=( rhs ); }
	~SMD_Animation_Frame( ){ DeleteData( ); }
	SMD_Animation_Frame & operator=( const SMD_Animation_Frame & rhs );


	void	SetNullAll( );
	void	DeleteData( );

	//---[ Member Methods ]---/
	friend	ostream &	operator<<( ostream & _os, SMD_Animation_Frame & _anim );
	bool		Load( FileCrawler & _is );
	void		AddTransform( matrix & _mat );
	void		AddPositionAndAngle( vertex & _position, vertex & _angles );

	//---[ Get / Set Member ]---/
	 
	long				getNumTransform( );
	mvector< vertex >	getPositions( ){ return m_Positions; }
	void				setPositions( mvector< vertex > & _Positions ){ m_Positions = _Positions; }

	mvector< vertex >	getAngles( ){ return m_Angles; }
	void				setAngles( mvector< vertex > & _Angles ){ m_Angles = _Angles; }


	//---[ Member Variables ]---/
	mvector< vertex >		m_Positions;
	mvector< vertex >		m_Angles;


};



};	//END namespace SMD

#endif /*__SMD_ANIMATION_FRAME_H__*/
