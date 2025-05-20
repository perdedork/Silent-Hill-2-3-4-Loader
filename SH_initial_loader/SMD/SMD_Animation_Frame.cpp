#include <windows.h>
#include <matrix.h>
#include <mvector.h>
#include <quat.h>
#include <iomanip>
#include "SMD_Vertex.h"
#include "SMD_Triangle.h"
#include "SMD_Animation_Frame.h"

namespace SMD
{

//-------------------------/
//-                       -/
//-                       -/
//-------------------------/
SMD_Animation_Frame & SMD_Animation_Frame::operator=( const SMD_Animation_Frame & rhs )
{
	if( & rhs != this )
	{
		m_Positions		=	rhs.m_Positions;
		m_Angles		=	rhs.m_Angles;

	}
	return *this;
}


//-------------------------/
//-                       -/
//-                       -/
//-------------------------/
void	SMD_Animation_Frame::SetNullAll( )
{
	
}


//-------------------------/
//-                       -/
//-                       -/
//-------------------------/
void	SMD_Animation_Frame::DeleteData( )
{
	m_Positions.clear( );
	m_Angles.clear( );

	SetNullAll( ); 
}


ostream &	operator<<( ostream & _os, SMD_Animation_Frame & _anim )
{
	for( long k = 0; k < _anim.m_Angles.size(); k++ )
	{
		_os	<<	k	<<	" "	<<	std::fixed	<<	std::setprecision( 6 )	<<	_anim.m_Positions[ k ]	<<	" "	<< _anim.m_Angles[ k ]	<<	std::endl;
	}
	return _os;
}


bool		SMD_Animation_Frame::Load( FileCrawler & _is )
{
	mstring	l_curLine = _is.readLine().toLower();

	this->DeleteData();

	while( l_curLine != "end" && l_curLine.substr( 0, 4 ) != "time" && ( !_is.isAtEnd() ) )
	{
		if( l_curLine == "end" )
		{
			return false;
		}
		else if( l_curLine[ 0 ] == '/' || l_curLine[ 0 ] == ';' || l_curLine[ 0 ] == '#' )
		{
			continue;
		}
		else
		{
			mvector<mstring>	l_lineVals = getTokenVector( l_curLine," \t",false, false);

			if( l_lineVals.size() != 7 )
			{
				LogFile( ERROR_LOG, "%s() - Syntax Error:  Expected 7 tokens,  got %ld", __FUNCTION__, l_lineVals.size() );
				continue;
			}

			this->AddPositionAndAngle(	vertex( static_cast<float>(atof( l_lineVals[ 1 ].c_str() )), 
												static_cast<float>(atof( l_lineVals[ 2 ].c_str() )), 
												static_cast<float>(atof( l_lineVals[ 3 ].c_str() )) ),
										vertex( static_cast<float>(atof( l_lineVals[ 4 ].c_str() )), 
												static_cast<float>(atof( l_lineVals[ 5 ].c_str() )), 
												static_cast<float>(atof( l_lineVals[ 6 ].c_str() )) ) );
		}

		l_curLine = _is.readLine().toLower();
	}
	return true;
}

void		SMD_Animation_Frame::AddTransform( matrix & _mat )
{
	quat l_TempQuat;
	float l_XAxis;
	float l_YAxis;
	float l_ZAxis;

	l_TempQuat.matToQuat( _mat );

	l_TempQuat.getAxisAngles( &l_XAxis, &l_YAxis, &l_ZAxis );

	this->m_Angles.push_back( vertex( l_XAxis, l_YAxis, l_ZAxis ) );
	this->m_Positions.push_back( vertex( _mat[ 3 ], _mat[ 7 ], _mat[ 11 ] ) );
}


void		SMD_Animation_Frame::AddPositionAndAngle( vertex & _position, vertex & _angles )
{
	this->m_Positions.push_back( _position );
	this->m_Angles.push_back( _angles );
}

long		SMD_Animation_Frame::getNumTransform( )
{
	return this->m_Angles.size();
}

};	//End Namespace SMD