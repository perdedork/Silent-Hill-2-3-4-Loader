#include <windows.h>
#include <stdio.h>
#include <mvector.h>
#include <errno.h>
#include <limits.h>
#include <gl\gl.h>

#include "SH3_Loader.h"
#include "SH3_SDB_Loader.h"

extern int errno;

static long SH3_FindArcEntrySizeByOffset(FILE* inFile, long l_lOffset)
{
	long l_lCurPos;
	long l_lHeader[4];
	long l_lNumIndex;
	long k;

	if (!inFile || l_lOffset < 0)
		return 0;

	l_lCurPos = ftell(inFile);
	fseek(inFile, 0, SEEK_SET);

	if (fread(l_lHeader, 1, sizeof(l_lHeader), inFile) != sizeof(l_lHeader))
	{
		fseek(inFile, l_lCurPos, SEEK_SET);
		return 0;
	}

	if (l_lHeader[0] != 537068807)
	{
		fseek(inFile, l_lCurPos, SEEK_SET);
		return 0;
	}

	l_lNumIndex = l_lHeader[1];
	if (l_lNumIndex <= 0 || l_lNumIndex > 0x100000)
	{
		fseek(inFile, l_lCurPos, SEEK_SET);
		return 0;
	}

	for (k = 0; k < l_lNumIndex; k++)
	{
		index_record l_sIndex;

		if (fread(&l_sIndex, 1, sizeof(l_sIndex), inFile) != sizeof(l_sIndex))
		{
			fseek(inFile, l_lCurPos, SEEK_SET);
			return 0;
		}

		if (l_sIndex.offset == l_lOffset && l_sIndex.size > 0)
		{
			fseek(inFile, l_lCurPos, SEEK_SET);
			return l_sIndex.size;
		}
	}

	fseek(inFile, l_lCurPos, SEEK_SET);
	return 0;
}


long SH3_SdbRecord::LoadData(FILE* inFile)
{
	long	l_lRes;

	if (!inFile)
	{
		LogFile(ERROR_LOG, "SH3_SdbRecord::LoadData( ) - ERROR: No file pointer was passed in");
		return 0;
	}

	DeleteData();

	l_lRes = _loadBlock((void*)&m_sRecord, sizeof(m_sRecord), inFile,
		"SH3_SdbRecord::LoadData( ) - ERROR: Could not load SDB record data", ERROR_LOG);

	if (l_lRes == -1)
		return 0;

	return l_lRes;
}


long SH3_Sdb::Load(char* filename, long _offset)
{
	FILE* inFile;
	long			l_lRes;
	long			l_lTotalRead = 0;
	long			l_lFileSize;
	long			l_lDataSize;
	long			l_lNumRecords;
	long			l_lArcEntrySize;
	long			k;
	SH3_SdbRecord	l_cTempRecord;

	if (!filename)
	{
		LogFile(ERROR_LOG, "SH3_Sdb::Load( ) - ERROR: No filename was passed in");
		return 0;
	}

	if (_offset < 0)
	{
		LogFile(ERROR_LOG, "SH3_Sdb::Load( ) - ERROR: Invalid offset %ld", _offset);
		return 0;
	}

	DeleteData();

	if ((inFile = fopen(filename, "rb")) == NULL)
	{
		LogFile(ERROR_LOG, "SH3_Sdb::Load( ) - ERROR: Could not load filename '%s' - Reason: %s", filename, strerror(errno));
		return 0;
	}

	fseek(inFile, 0, SEEK_END);
	l_lFileSize = ftell(inFile);

	if (l_lFileSize <= _offset)
	{
		LogFile(ERROR_LOG, "SH3_Sdb::Load( ) - ERROR: Invalid offset %ld for file size %ld", _offset, l_lFileSize);
		fclose(inFile);
		return 0;
	}

	l_lArcEntrySize = SH3_FindArcEntrySizeByOffset(inFile, _offset);
	if (l_lArcEntrySize > 0)
		l_lDataSize = l_lArcEntrySize;
	else
		l_lDataSize = l_lFileSize - _offset;

	// Current best-known format:
	//   96-byte header
	//   N * 96-byte records
	//   96-byte footer
	if (l_lDataSize < (long)(sizeof(sh3_sdb_header) + sizeof(sh3_sdb_footer)))
	{
		LogFile(ERROR_LOG, "SH3_Sdb::Load( ) - ERROR: File '%s' is too small to be a valid SDB (%ld bytes from offset)", filename, l_lDataSize);
		fclose(inFile);
		return 0;
	}

	if ((l_lDataSize - (long)sizeof(sh3_sdb_header) - (long)sizeof(sh3_sdb_footer)) % (long)sizeof(sh3_sdb_record))
	{
		LogFile(ERROR_LOG, "SH3_Sdb::Load( ) - WARNING: SDB '%s' has unexpected size %ld from offset %ld; record area is not a clean multiple of %u",
			filename, l_lDataSize, _offset, (unsigned)sizeof(sh3_sdb_record));
	}

	l_lNumRecords = (l_lDataSize - (long)sizeof(sh3_sdb_header) - (long)sizeof(sh3_sdb_footer)) / (long)sizeof(sh3_sdb_record);

	if (l_lNumRecords < 0 || l_lNumRecords > 0x100000)
	{
		LogFile(ERROR_LOG, "SH3_Sdb::Load( ) - ERROR: Invalid record count %ld", l_lNumRecords);
		fclose(inFile);
		return 0;
	}

	fseek(inFile, _offset, SEEK_SET);

	l_lRes = _loadBlock((void*)&m_sHeader, sizeof(m_sHeader), inFile,
		"SH3_Sdb::Load( ) - ERROR: Could not load SDB header", ERROR_LOG);

	if (l_lRes == -1)
	{
		fclose(inFile);
		return 0;
	}

	l_lTotalRead += l_lRes;

	LogFile(ERROR_LOG, "SH3_Sdb::Load( ) - Loading file '%s:%ld'", filename, _offset);
	if (l_lArcEntrySize > 0)
		LogFile(ERROR_LOG, "SH3_Sdb::Load( ) - Using archive entry size %ld bytes", l_lArcEntrySize);
	LogFile(ERROR_LOG, "SH3_Sdb::Load( ) - CHECK: Header raw bounds/extents are:");
	debugLongArray(m_sHeader.s_lBoundsOrExtents, 6);

	m_vRecords.clear();

	for (k = 0; k < l_lNumRecords; k++)
	{
		if (!(l_lRes = l_cTempRecord.LoadData(inFile)))
		{
			LogFile(ERROR_LOG, "SH3_Sdb::Load( ) - ERROR: Could not load record %ld of %ld", k + 1, l_lNumRecords);
			fclose(inFile);
			return l_lTotalRead;
		}

		l_lTotalRead += l_lRes;
		m_vRecords.push_back(l_cTempRecord);

		LogFile(ERROR_LOG, "SH3_Sdb::Load( ) - Record %ld:", k);
		debugFloat(l_cTempRecord.m_sRecord.s_fX1);
		debugFloat(l_cTempRecord.m_sRecord.s_fZ1);
		debugFloat(l_cTempRecord.m_sRecord.s_fX2);
		debugFloat(l_cTempRecord.m_sRecord.s_fZ2);
		debugFloat(l_cTempRecord.m_sRecord.s_fHeightOrRadius);
		debugFloat(l_cTempRecord.m_sRecord.s_fYOrFloor);
		debugLong(l_cTempRecord.m_sRecord.s_lMapExtOrID);
		debugLong(l_cTempRecord.m_sRecord.s_lZero);
	}

	l_lRes = _loadBlock((void*)&m_sFooter, sizeof(m_sFooter), inFile,
		"SH3_Sdb::Load( ) - ERROR: Could not load SDB footer", ERROR_LOG);

	if (l_lRes == -1)
	{
		fclose(inFile);
		return l_lTotalRead;
	}

	l_lTotalRead += l_lRes;
	m_lNumRecords = (long)m_vRecords.size();

	LogFile(ERROR_LOG, "SH3_Sdb::Load( ) - Footer name is '%s'", m_sFooter.s_caName);
	LogFile(ERROR_LOG, "SH3_Sdb::Load( ) - Loaded %ld records", m_lNumRecords);

	fclose(inFile);
	return l_lTotalRead;
}


void SH3_Sdb::Draw( )
{
	long k;

	if( m_vRecords.empty( ) )
		return;

	glDisable(GL_TEXTURE_2D);
	glDisable(GL_CULL_FACE);
	glDisable(GL_BLEND);
	glDisable(GL_DEPTH_TEST);
	glLineWidth(2.0f);
	glColor3f(0.2f, 0.35f, 1.0f);

	for( k = 0; k < (long)m_vRecords.size( ); k++ )
	{
		float l_fMinX = m_vRecords[ k ].m_sRecord.s_fX1;
		float l_fMinZ = m_vRecords[ k ].m_sRecord.s_fZ1;
		float l_fMaxX = m_vRecords[ k ].m_sRecord.s_fX2;
		float l_fMaxZ = m_vRecords[ k ].m_sRecord.s_fZ2;
		float l_fMinY = m_vRecords[ k ].m_sRecord.s_fYOrFloor;
		float l_fMaxY = m_vRecords[ k ].m_sRecord.s_fHeightOrRadius;
		float l_fTmp;

		if( l_fMinX > l_fMaxX )
		{
			l_fTmp = l_fMinX;
			l_fMinX = l_fMaxX;
			l_fMaxX = l_fTmp;
		}
		if( l_fMinY > l_fMaxY )
		{
			l_fTmp = l_fMinY;
			l_fMinY = l_fMaxY;
			l_fMaxY = l_fTmp;
		}
		if( l_fMinZ > l_fMaxZ )
		{
			l_fTmp = l_fMinZ;
			l_fMinZ = l_fMaxZ;
			l_fMaxZ = l_fTmp;
		}

		glBegin(GL_LINE_LOOP);
			glVertex3f(l_fMinX, l_fMinY, l_fMinZ);
			glVertex3f(l_fMaxX, l_fMinY, l_fMinZ);
			glVertex3f(l_fMaxX, l_fMinY, l_fMaxZ);
			glVertex3f(l_fMinX, l_fMinY, l_fMaxZ);
		glEnd();

		glBegin(GL_LINE_LOOP);
			glVertex3f(l_fMinX, l_fMaxY, l_fMinZ);
			glVertex3f(l_fMaxX, l_fMaxY, l_fMinZ);
			glVertex3f(l_fMaxX, l_fMaxY, l_fMaxZ);
			glVertex3f(l_fMinX, l_fMaxY, l_fMaxZ);
		glEnd();

		glBegin(GL_LINES);
			glVertex3f(l_fMinX, l_fMinY, l_fMinZ); glVertex3f(l_fMinX, l_fMaxY, l_fMinZ);
			glVertex3f(l_fMaxX, l_fMinY, l_fMinZ); glVertex3f(l_fMaxX, l_fMaxY, l_fMinZ);
			glVertex3f(l_fMaxX, l_fMinY, l_fMaxZ); glVertex3f(l_fMaxX, l_fMaxY, l_fMaxZ);
			glVertex3f(l_fMinX, l_fMinY, l_fMaxZ); glVertex3f(l_fMinX, l_fMaxY, l_fMaxZ);
		glEnd();
	}

	glEnable(GL_DEPTH_TEST);
}


bool SdbRecordHasNonZeroPad(SH3_SdbRecord* h)
{
	long k;

	if (!h)
		return false;

	for (k = 0; k < (long)sizeof(h->m_sRecord.s_caPad); k++)
	{
		if (h->m_sRecord.s_caPad[k])
			return true;
	}

	return false;
}


bool SdbHeaderHasNonZeroPad(SH3_Sdb* h)
{
	long k;

	if (!h)
		return false;

	for (k = 0; k < (long)sizeof(h->m_sHeader.s_caPad); k++)
	{
		if (h->m_sHeader.s_caPad[k])
			return true;
	}

	return false;
}


bool SdbFooterHasNonZeroPad(SH3_Sdb* h)
{
	long k;

	if (!h)
		return false;

	for (k = 0; k < (long)sizeof(h->m_sFooter.s_caPad); k++)
	{
		if (h->m_sFooter.s_caPad[k])
			return true;
	}

	return false;
}


void DebugSdbRecord(SH3_SdbRecord* h, long _lIndex)
{
	if (!h)
	{
		LogFile(ERROR_LOG, "DebugSdbRecord( ) - ERROR: No SDB record pointer was passed in");
		return;
	}

	LogFile(ERROR_LOG,
		"DebugSdbRecord( ) - Record %ld: Rect[(%.3f, %.3f) -> (%.3f, %.3f)] H=%.3f Y=%.3f ID=%ld (0x%02lx) Zero=%ld",
		_lIndex,
		h->m_sRecord.s_fX1,
		h->m_sRecord.s_fZ1,
		h->m_sRecord.s_fX2,
		h->m_sRecord.s_fZ2,
		h->m_sRecord.s_fHeightOrRadius,
		h->m_sRecord.s_fYOrFloor,
		h->m_sRecord.s_lMapExtOrID,
		(unsigned long)(h->m_sRecord.s_lMapExtOrID & 0xff),
		h->m_sRecord.s_lZero
	);

	if (SdbRecordHasNonZeroPad(h))
	{
		LogFile(ERROR_LOG, "DebugSdbRecord( ) - NOTICE: Record %ld has non-zero padding bytes", _lIndex);
	}
}


void DebugSdb(SH3_Sdb* h)
{
	long k;

	if (!h)
	{
		LogFile(ERROR_LOG, "DebugSdb( ) - ERROR: No SDB pointer was passed in");
		return;
	}

	LogFile(ERROR_LOG, "DebugSdb( ) - Header bounds/extents are:");
	debugLongArray(h->m_sHeader.s_lBoundsOrExtents, 6);

	if (SdbHeaderHasNonZeroPad(h))
		LogFile(ERROR_LOG, "DebugSdb( ) - NOTICE: Header padding contains non-zero data");

	LogFile(ERROR_LOG, "DebugSdb( ) - Num Records: %ld", h->m_lNumRecords);

	for (k = 0; k < h->m_lNumRecords; k++)
	{
		DebugSdbRecord(&(h->m_vRecords[k]), k);
	}

	LogFile(ERROR_LOG, "DebugSdb( ) - Footer name: '%s'", h->m_sFooter.s_caName);

	if (SdbFooterHasNonZeroPad(h))
		LogFile(ERROR_LOG, "DebugSdb( ) - NOTICE: Footer padding contains non-zero data");
}


void DebugSdbSummaryByID(SH3_Sdb* h)
{
	long k;
	long counts[256];

	if (!h)
	{
		LogFile(ERROR_LOG, "DebugSdbSummaryByID( ) - ERROR: No SDB pointer was passed in");
		return;
	}

	memset(counts, 0, sizeof(counts));

	for (k = 0; k < h->m_lNumRecords; k++)
	{
		counts[h->m_vRecords[k].m_sRecord.s_lMapExtOrID & 0xff]++;
	}

	LogFile(ERROR_LOG, "DebugSdbSummaryByID( ) - Record counts by low-byte ID:");

	for (k = 0; k < 256; k++)
	{
		if (counts[k])
		{
			LogFile(ERROR_LOG, "  0x%02lx (%ld): %ld record(s)", (unsigned long)k, k, counts[k]);
		}
	}
}
