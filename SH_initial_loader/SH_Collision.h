#ifndef  __SH_COLLISION_H__
#define  __SH_COLLISION_H__


#include <windows.h>
#include <vector>
#include "utils.h"

using std::vector;

typedef struct
{
	float	s_fXPos;
	float	s_fZPos;
	long	s_lSectDataSize[ 5 ];		//It's 80 for empty for the first 4
//	long	s_lQ1_SectDataSize;			//It's 48 for empty
	long	f_sh3_ch;
}
sh3_cld_header;

typedef struct
{
	long	s_laIndexOffsets[ 16 ];	//If there is no data(just -1) at index location, then the diff between each offset is 4
}
sh3_cld_index_offsets;

typedef struct
{
	long	s_lOffsetSectData[ 5 ];		//Offset to sect 1-5's vert data.  If empty, the diff to the next is 80 for sect 0-3, and 48 for 4
//	long	s_lQ1_OffsetSectData;		//Offset to data w/ empty size of 48.  I don't know what this is...
}
sh3_cld_data_offsets;


typedef struct
{
	long	s_lVertType;			//So far seems to always be 257
	long	s_lNumVerts;			//So far it's always 4.  This could change tho...
	long	q1_sh3_cvh;				//Varies - But only ever seems to be 1 bit set 
	long	f1_sh3_cvh;				//Filler
}
sh3_cld_vert_header;



class SH3_CldIndex
{
public:
	SH3_CldIndex( ){ SetNullAll( ); }
	SH3_CldIndex( const SH3_CldIndex & rhs ){ SetNullAll( ); operator=( rhs ); }
	~SH3_CldIndex( ){ DeleteData( ); }
	SH3_CldIndex & operator=( const SH3_CldIndex & rhs ){ if( &rhs != this ){ DeleteData( ); dynamicDataCopy( m_plIndices, long, m_lNumIndex );} return *this; }

	void SetNullAll( ){ m_lNumIndex = 0; m_plIndices = NULL; }
	void DeleteData( ){ delete [] m_plIndices; SetNullAll( ); }

	long LoadData( FILE *inFile, long _lDataSize );

	long	m_lNumIndex;
	long	*m_plIndices;
};


class SH3_CldPrim
{
public:
	SH3_CldPrim( ){ SetNullAll( ); }
	SH3_CldPrim( const SH3_CldPrim & rhs ){ SetNullAll( ); operator=( rhs ); }
	~SH3_CldPrim( ){ DeleteData( ); }
	SH3_CldPrim & operator=( const SH3_CldPrim & rhs ){ if( &rhs != this ){ DeleteData( ); structMemcpy( m_sVertHeader ); dynamicDataCopy( m_pcVerts, vertex4f, m_sVertHeader.s_lNumVerts ); } return *this; }

	void SetNullAll( ){ structMemClear( m_sVertHeader ); m_pcVerts = NULL; }
	void DeleteData( ){ delete [] m_pcVerts; SetNullAll( ); }

	long LoadData( FILE *inFile );

	sh3_cld_vert_header	m_sVertHeader;
	vertex4f			*m_pcVerts;
};


class SH3_CldSet
{
public:
	SH3_CldSet( ){ SetNullAll( ); }
	SH3_CldSet( const SH3_CldSet & rhs ){ SetNullAll( ); operator=( rhs ); }
	~SH3_CldSet( ){ DeleteData( ); }
	SH3_CldSet & operator=( const SH3_CldSet & rhs ){ if( &rhs != this ){ DeleteData( ); fieldCopy( m_vIndexData ); fieldCopy( m_vPrimData ); } return *this; }

	void SetNullAll( ){ }
	void DeleteData( ){ m_vIndexData.clear( ); m_vPrimData.clear( ); }

	long LoadIndex( FILE *inFile, sh3_cld_index_offsets *_psOffsets );
	long LoadVerts( FILE *inFile, long _lOffset, long _lDataSize );

	vector<SH3_CldIndex>	m_vIndexData;
	vector<SH3_CldPrim>		m_vPrimData;
};


class SH3_Collision
{
public:
	SH3_Collision( ){ SetNullAll( ); }
	SH3_Collision( const SH3_Collision & rhs ){ SetNullAll( ); operator=( rhs ); }
	~SH3_Collision( ){ DeleteData( ); }
	SH3_Collision & operator=( const SH3_Collision & rhs ){ if( &rhs != this ){ DeleteData( ); structMemcpy( m_sHeader ); staticDataCopy( m_caCldData, 5 ); } return *this; }

	void SetNullAll( ){ }
	void DeleteData( ){ }

	long Load( char *filename, long _offset );

	sh3_cld_header	m_sHeader;			//Collision File Header
	SH3_CldSet		m_caCldData[ 5 ];	//All collision data (indexes and verts)
};


#endif /*__SH_COLLISION_H__*/