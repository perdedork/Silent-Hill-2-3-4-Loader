#ifndef  __MVECTOR_H__
#define  __MVECTOR_H__

//###NOTES: In order to restore original logging, replace [##::##] with [","], and [LogFile] with [RecordError]
//#include <utility/Logging.h>
//#include <utility/ErrTracker.h>
#include <windows.h>
#include "typedefs.h"

namespace mstd
{

template < class tObj >
class mvector
{
public:
	typedef tObj **iterator;

	tObj	m_tEmpty;		//Empty Element Placeholder.  Used to let users check if operator[] returned crap.

	mvector( long _lCapacity = 10):m_lCapacity( _lCapacity ){ fill_empty( ); _allocate( _lCapacity ); }
	mvector( const mvector & rhs ){ fill_empty( ); mvector::operator=( rhs ); }
	~mvector( ){ mvector::release( ); }

	const mvector & operator=( const mvector & rhs )
	{
		if( & rhs != this )
		{
			long k;
			mvector::release( );
			m_lCapacity = rhs.m_lCapacity;
			_allocate( m_lCapacity );
			m_lSize = rhs.m_lSize;

			for( k = 0; k < m_lSize; k++ )
			{
				tObj *tPtr = new tObj;
				tPtr[0] = *(rhs.m_tElements[k]);
				m_tElements[k] = tPtr;
			}
		}
		return *this;
	}


	tObj & operator[]( long _lIndex ){ if( _lIndex == -1 || !m_lSize ) return m_tEmpty; return *(m_tElements[ (abs(_lIndex))%m_lSize ]); }
	const tObj & operator[]( long _lIndex ) const { if( _lIndex == -1 || !m_lSize ) return m_tEmpty; return *(m_tElements[ (abs(_lIndex))%m_lSize ]); }

	bool is_empty_element( const tObj & obj ){ return &obj == &m_tEmpty; }

	//---[ Clear And Release all elements ]---/
	void	release( )				// Release the entire vector
	{
		if( m_tElements )
		{
			for( long k = 0; k < m_lSize; k++ )
			{
				if( m_tElements[ k ] )
					delete m_tElements[ k ];
			}
			delete [] m_tElements;
		}
		fill_empty( );
	}

	//---[ Pointer Related Methods ]---/
	long	push_back_orig( tObj * pObj )
	{
		if( !pObj )
		{
			LogFile( ERROR_LOG, "mvector::push_back_orig( ) - ERROR: Null pointer passed in");
			return -1;
		}

		if( m_lSize == m_lCapacity ) 
			_allocate( m_lCapacity + ( m_lCapacity >> 1 ) + 10 ); 
		m_tElements[ m_lSize ++ ] = pObj; 
		return m_lSize - 1;
	}

	long	index_by_pointer( tObj * pObj )
	{
		if( !pObj )
		{
			LogFile( ERROR_LOG, "mvector::index_by_pointer( ) - ERROR: Null pointer passed in");
			return -1;
		}

		for( long k = 0; k < m_lSize; k++ )
		{
			if( m_tElements[ k ] == pObj )
				return k;
		}

		return -1;
	}
	
	//---[ General Vector Methods ]---/
	void	resize( long _lCapacity ){ _allocate( _lCapacity ); }
	void	clear( long _lStart = 0, long lEnd = -1 )
	{
		long _lEnd = (lEnd < 0 || lEnd > m_lSize )?m_lSize:lEnd;
		if( validIndex( _lStart ) && validIndex( _lEnd ) )
		{
			if( _lStart < _lEnd )
			{
				for( long k = _lEnd; k > _lStart; k-- )
					_delete( k-1 );
			}
			else
			{
				for( long k = _lStart; k > _lEnd; k-- )
					_delete( k-1 );
			}
			//m_lSize = 0;
		}
	}

	void	copy( const mvector & rhs, long _lNum = -1, long _lSrcStart = 0, long _lDestStart = 0 )
	{
		long len = ( _lNum < 0 )?rhs.size():_lNum;
		long dStart = ( _lDestStart > m_lSize )?m_lSize:_lDestStart;

		if( _lSrcStart >= 0 && _lSrcStart < rhs.size() && _lDestStart >= 0 )
		{
			if( _lSrcStart + len > rhs.size() )
				len = rhs.size() - _lSrcStart;

			//if( dStart < m_lSize && len + dStart > m_lSize )
			//	clear( dStart, dStart + len );
			if( len + dStart > m_lCapacity )
				_allocate( len + dStart + 10 );
			for( long k = 0; k < len; k++ )
				_replace( dStart + k, rhs[ _lSrcStart + k ] );
			if( len + dStart > m_lSize )
				m_lSize = len + dStart;
		}
	}
	long	size( ) const { return m_lSize; }
	bool	empty( ) const { return m_lSize == 0; }
	long	push_back( const tObj & obj )
	{ 
		tObj *pObj = new tObj( obj ); 

		return push_back_orig( pObj );
	}
	void	push_front( const tObj & obj ){ tObj *pObj = new tObj( obj ); shift_right( 0 ); m_tElements[ 0 ] = pObj; }
	tObj	pop_back( ){ if( !empty( ) ){ tObj *tPtr = m_tElements[ m_lSize-1 ]; m_tElements[ --m_lSize ] = NULL; return *tPtr;} return tObj(); }
	tObj	pop_front( ){ if( !empty( ) ){ tObj *tPtr = m_tElements[ 0 ]; shift_left( 0 ); return *tPtr; } return tObj(); }
	long	front( ){ return 0; }
	long	back( ){ return m_lSize - 1; }
	bool	validIndex( long _lIndex ){ return !((_lIndex > m_lSize + 1 )||(_lIndex < 0 )); }
	void	swap( mvector & rhs ){ tObj **tElements = rhs.m_tElements; long _lSize =  rhs.m_lSize; long _lCapacity = rhs.m_lCapacity; rhs.m_tElements = m_tElements; rhs.m_lSize = m_lSize; rhs.m_lCapacity = m_lCapacity; m_tElements = tElements; m_lSize = _lSize; m_lCapacity =  _lCapacity; }
	bool	insert( long _lIndex, const tObj & obj )
	{
		if( validIndex( _lIndex ) )
			_insert( _lIndex, obj );
		else
		{
			LogFile( ERROR_LOG, "Index Range Error##::##Index out of range (%ld for range [0,%ld{+ 1}]) in mvector::insert - putting at end",_lIndex,m_lSize);
			push_back( obj );
			return false;
		}
		return true;
	}

	bool	insert_orig( long _lIndex, tObj * obj )
	{
		if( validIndex( _lIndex ) )
			_insert_orig( _lIndex, obj );
		else
		{
			LogFile( ERROR_LOG, "Index Range Error##::##Index out of range (%ld for range [0,%ld{+ 1}]) in mvector::insert - putting at end",_lIndex,m_lSize);
			push_back_orig( obj );
			return false;
		}
		return true;
	}

	bool	replace( long _lIndex, const tObj & obj )
	{
		if( validIndex( _lIndex ) )
		{
			_replace( _lIndex, obj );
			return true;
		}
		else
			LogFile( ERROR_LOG, "Index Range Error##::##Index out of range (%ld for range [0,%ld{+ 1}]) in mvector::replace - ignoring",_lIndex,m_lSize);
		return false;
	}

	bool	erase( long _lIndex, long _lCount = 1 )
	{
		clear( _lIndex, _lIndex + _lCount );
		//long k;
		//for( k = _lIndex; k < _lIndex + _lCount && validIndex( k ); k++ )
		//	shift_left( k );
		return true;
	}

private:
	void	shift_right( long _lIndex )
	{
		if( m_lSize == m_lCapacity )
			_allocate( m_lCapacity + ( m_lCapacity >> 1 ) + 10 );
		for( long k = m_lSize; k > _lIndex; k-- )
			m_tElements[ k ] = m_tElements[ k-1 ];
		++m_lSize;
		m_tElements[ _lIndex ] = NULL;
	}

	void	shift_left( long _lIndex )
	{
		if( m_tElements[ _lIndex ] )
			delete m_tElements[ _lIndex ];
		--m_lSize;
		for( long k = _lIndex; k < m_lSize; k++ )
			m_tElements[ k ] = m_tElements[ k+1 ];
		m_tElements[ m_lSize ] = NULL;
	}

	void	_insert( long _lIndex, const tObj & obj )
	{
		tObj *pObj = new tObj( obj );
		
		shift_right( _lIndex );
		m_tElements[ _lIndex ] = pObj;
	}

	void	_insert_orig( long _lIndex, tObj * obj )
	{
		shift_right( _lIndex );
		m_tElements[ _lIndex ] = obj;
	}


	void	_replace( long _lIndex, const tObj & obj )
	{
		tObj *pObj = new tObj( obj );
		delete m_tElements[ _lIndex ];
		m_tElements[ _lIndex ] = pObj;
	}

	void	_delete( long _lIndex ){ if( m_tElements[ _lIndex ] ) delete m_tElements[ _lIndex ]; m_tElements[ _lIndex ] = NULL; shift_left( _lIndex );}

	//---[ Change the size of the vector ]---/
	void	_allocate( unsigned long _Capacity )
	{
		long k;
		tObj **tElements;
		long _lCapacity = _Capacity;
		
		if( _lCapacity )
		{
			tElements = new tObj*[ _lCapacity ];

			if( ! tElements )
				LogFile( ERROR_LOG, "Mem Alloc Failure##::##Couldn't allocate memory in mvector::allocate");

			if( m_tElements )
			{
				if( m_lSize > _lCapacity )
				{
					for( k = _lCapacity; k < m_lSize; k++ )
						delete m_tElements[ k ];
					m_lSize = _lCapacity;
				}

				for( k = 0; k < m_lSize; k++ )
				{
					tElements[ k ] = m_tElements[ k ];
					m_tElements[ k ] = NULL;
				}
				delete [] m_tElements;
			}
			else
				m_lSize = 0;

			for( k = m_lSize; k < _lCapacity; k++ )
				tElements[ k ] = NULL;
		}
		else
		{
			if( m_tElements )
			{
				clear( );
				delete [] m_tElements;
				m_lSize = 0;
			}
			m_tElements = NULL; 
		}

		m_tElements = tElements;
		m_lCapacity = _lCapacity;
	}


	void	fill_empty( ){ m_tElements = NULL; m_lSize = 0; m_lCapacity = 0; }


	//---[ Members ]---/
	tObj	**m_tElements;			// Underlying representation
	long	m_lSize;				// Current number of tObj in m_tElements
	long	m_lCapacity;			// Maximum size at the time
};

};	/* NAMESPACE mstd END */

#endif /*__MVECTOR_H__*/