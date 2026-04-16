#ifndef __SH_SDB_H__
#define __SH_SDB_H__

#include <windows.h>
#include <gl\gl.h>
#include <mvector.h>
#include "utils.h"

using mstd::mvector;

#pragma pack(push, 1)

typedef struct
{
	long	s_lBoundsOrExtents[6];	// Raw values from file; do not appear to be normal floats
	char	s_caPad[72];				// Unknown / currently zero in sample
}
sh3_sdb_header;

typedef struct
{
	float	s_fX1;
	float	s_fZ1;
	float	s_fX2;
	float	s_fZ2;
	float	s_fHeightOrRadius;
	float	s_fYOrFloor;
	long	s_lMapExtOrID;
	long	s_lZero;					// Usually 0
	char	s_caPad[64];				// Unknown / currently zero in sample
}
sh3_sdb_record;

typedef struct
{
	char	s_caName[16];				// Example: "apart.sdb"
	char	s_caPad[80];				// Unknown footer data
}
sh3_sdb_footer;

#pragma pack(pop)


class SH3_SdbRecord
{
public:
	SH3_SdbRecord() { SetNullAll(); }
	SH3_SdbRecord(const SH3_SdbRecord& rhs) { SetNullAll(); operator=(rhs); }
	~SH3_SdbRecord() { DeleteData(); }
	SH3_SdbRecord& operator=(const SH3_SdbRecord& rhs)
	{
		if (&rhs != this)
		{
			DeleteData();
			structMemcpy(m_sRecord);
		}
		return *this;
	}

	void SetNullAll() { structMemClear(m_sRecord); }
	void DeleteData() { SetNullAll(); }

	long LoadData(FILE* inFile);

	sh3_sdb_record	m_sRecord;
};


class SH3_Sdb
{
public:
	SH3_Sdb() { SetNullAll(); }
	SH3_Sdb(const SH3_Sdb& rhs) { SetNullAll(); operator=(rhs); }
	~SH3_Sdb() { DeleteData(); }
	SH3_Sdb& operator=(const SH3_Sdb& rhs)
	{
		if (&rhs != this)
		{
			DeleteData();
			structMemcpy(m_sHeader);
			fieldCopy(m_vRecords);
			structMemcpy(m_sFooter);
			m_lNumRecords = rhs.m_lNumRecords;
		}
		return *this;
	}

	void SetNullAll()
	{
		structMemClear(m_sHeader);
		structMemClear(m_sFooter);
		m_vRecords.clear();
		m_lNumRecords = 0;
	}

	void DeleteData()
	{
		m_vRecords.clear();
		structMemClear(m_sHeader);
		structMemClear(m_sFooter);
		m_lNumRecords = 0;
	}

	long Load(char* filename, long _offset = 0);
	void Draw( );

	sh3_sdb_header			m_sHeader;
	mvector<SH3_SdbRecord>	m_vRecords;
	sh3_sdb_footer			m_sFooter;
	long					m_lNumRecords;
};



//--------------------------------------------------------/
//- Debug Functions ---------------------------------------------/
//--------------------------------------------------------/
void DebugSdb(SH3_Sdb* h);
void DebugSdbRecord(SH3_SdbRecord* h, long _lIndex);
void DebugSdbSummaryByID(SH3_Sdb* h);
bool SdbRecordHasNonZeroPad(SH3_SdbRecord* h);
bool SdbHeaderHasNonZeroPad(SH3_Sdb* h);
bool SdbFooterHasNonZeroPad(SH3_Sdb* h);

#endif /*__SH_SDB_H__*/
