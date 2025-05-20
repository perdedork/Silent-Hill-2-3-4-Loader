#include "stdafx.h"
//#include <windows.h>
#include <lzexpand.h>
#include <stdio.h>
#include <string>
#include <zlib.h>
#include "SH3_ArcFilenames.h"
//#include "LogFile.h"
#include "typedefs.h"
using namespace std;

sh3_arc_section_header & sh3_arc_section_header::operator=( const sh3_arc_section_header & rhs )
{ 
	if( &rhs != this )
	{
		structType = rhs.structType;
		headerSize = rhs.headerSize;
		nameCount = rhs.nameCount;
		if( rhs.sectionName )
		{
			delete[] sectionName;
			sectionName = new char[ headerSize - sizeof(sh3_arc_section_loader) ];
			strcpy( sectionName, rhs.sectionName );
		}

		if( rhs.m_pcArcFilenames )
		{
			long k;
			delete [] m_pcArcFilenames;
			m_pcArcFilenames = new sh3_arc_filename[ nameCount ];
			for( k = 0; k < nameCount; k++ )
				m_pcArcFilenames[ k ] = rhs.m_pcArcFilenames[ k ];
		}
	}
	return *this ;
}

int sh3_arc_section_header::Load( gzFile iHandle )
{
	sh3_arc_section_loader	l_sSectLoader;
	long					res;
	long					totalRead;
	long					k;

	if( iHandle == NULL )
		return 0;

	if( (res = gzread(iHandle,(voidp)&l_sSectLoader,sizeof( sh3_arc_section_loader ) )) != sizeof( sh3_arc_section_loader ))
	{
		LogFile( ERROR_LOG, "sh3_arc_section_header::Load( ) - ERROR: Unable to load arc section header (%ld of %ld bytes read) - REASON: %s",res,sizeof( sh3_arc_section_loader ), strerror( errno ));
		return -res;
	}

	delete [] sectionName;
	sectionName = new char[ l_sSectLoader.headerSize - sizeof( sh3_arc_section_loader ) ];
	
	totalRead = res;

	if( (res = gzread(iHandle, sectionName, l_sSectLoader.headerSize - sizeof( sh3_arc_section_loader ) )) != l_sSectLoader.headerSize - sizeof( sh3_arc_section_loader ))
	{
		LogFile( ERROR_LOG, "sh3_arc_section_header::Load( ) - ERROR: Unable to load arc section name (%ld of %ld bytes read) - REASON: %s",res,l_sSectLoader.headerSize - sizeof( sh3_arc_section_loader ), strerror( errno ));
		return -(res + totalRead);
	}

	totalRead += res;
	headerSize = l_sSectLoader.headerSize;
	structType = l_sSectLoader.structType;
	nameCount = l_sSectLoader.nameCount;

	//---[ LOAD THE FILENAMES FOR THIS SECTION ]---/
	m_pcArcFilenames = new sh3_arc_filename[ nameCount ];

	for( k = 0; k < nameCount; k++ )
	{
		res = m_pcArcFilenames[ k ].Load( iHandle );

		if( res < 1 )
		{
			LogFile( ERROR_LOG, "sh3_arc_section_header::Load( ) - ERROR: Unable to load complete filename %ld of %ld - %ld bytes read",k,nameCount,-res);
			return -(totalRead - res);
		}
		totalRead += res;
	}

	return totalRead;
}


long sh3_arc_section_header::GetCountFileType( char *pcFileExt )
{
	long k;
	long fileCount = 0;

	for( k = 0; k < nameCount; k++ )
	{
		if( strcmp( &(m_pcArcFilenames[ k ].fileName[ strlen(m_pcArcFilenames[ k ].fileName) - strlen( pcFileExt ) ]), pcFileExt ) == 0 )
			fileCount ++;
	}

	return fileCount;
}



sh3_arc_filename & sh3_arc_filename::operator=( const sh3_arc_filename & rhs )
{ 
	if( &rhs != this )
	{ 
		if( rhs.fileName )
		{
			if( fileName )
				delete [] fileName;
			fileName = new char[ rhs.dataSize - sizeof(sh3_arc_file_loader) ];
			strcpy( fileName, rhs.fileName );
		}
		structType = rhs.structType;
		dataSize = rhs.dataSize;
		fileIndex = rhs.fileIndex;
		sectIndex = rhs.sectIndex;

	}
	return *this ;
}

int sh3_arc_filename::Load( gzFile iHandle )
{
	sh3_arc_file_loader	l_sFileLoader;
	long				res;
	long				totalRead;

	if( iHandle == NULL  )
		return 0;

	if( (res = gzread(iHandle,(voidp)&l_sFileLoader,sizeof( sh3_arc_file_loader ) )) != sizeof( sh3_arc_file_loader ))
	{
		LogFile( ERROR_LOG, "sh3_arc_filename::Load( ) - ERROR: Unable to load arc filename data (%ld of %ld bytes read) - REASON: %s",res,sizeof( sh3_arc_file_loader ), strerror( errno ));
		return -res;
	}

	delete [] fileName;
	fileName = new char[ l_sFileLoader.dataSize - sizeof( sh3_arc_file_loader ) ];
		
	totalRead = res;

	if( (res = gzread(iHandle, fileName, l_sFileLoader.dataSize - sizeof( sh3_arc_file_loader ) )) != l_sFileLoader.dataSize - sizeof( sh3_arc_file_loader ))
	{
		LogFile( ERROR_LOG, "sh3_arc_filename::Load( ) - ERROR: Unable to load arc filename string (%ld of %ld bytes read) - REASON: %s",res,l_sFileLoader.dataSize - sizeof( sh3_arc_file_loader ), strerror( errno ));
		return -(res + totalRead);
	}

	totalRead += res;
	dataSize = l_sFileLoader.dataSize;
	structType = l_sFileLoader.structType;
	fileIndex = l_sFileLoader.fileIndex;
	sectIndex = l_sFileLoader.sectIndex;

	return totalRead;
}


bool sh3_arc_filename::IsFileType( char *pcFileExt )
{
	LogFile(ERROR_LOG,"TEST: name to check is: %s, ext is %s and is at %s",fileName,pcFileExt,(strstr( fileName, pcFileExt ))?strstr( fileName, pcFileExt ):"NULL");
	return (strcmp( &(fileName[ strlen(fileName) - strlen( pcFileExt ) ]), pcFileExt ) == 0 );
}


sh3_arcfile & sh3_arcfile::operator=( const sh3_arcfile & rhs )
{
	if( &rhs != this )
	{
		memcpy( (void*)&m_sArcData, (void *)&rhs.m_sArcData, sizeof( sh3_arc_data_header ));
		m_sArcFilename = rhs.m_sArcFilename;
		if( rhs.m_pcArcSections )
		{
			long k;
			delete [] m_pcArcSections;
			m_pcArcSections = new sh3_arc_section_header[ m_sArcData.arcFileCount ];
			for( k = 0; k < m_sArcData.arcFileCount; k++ )
				m_pcArcSections[ k ] = rhs.m_pcArcSections[ k ];
		}
	}
	return *this;
}

int sh3_arcfile::Load( char *filename )
{
	long totalRead;
	long res;
	long k;
	gzFile		iHandle;
	//OFSTRUCT	ofStruct;
	sh3_arc_file_header	l_sArcFileHeader;

	if( !filename )
		return 0;

	if( (iHandle = gzopen( filename, "rb")) == NULL )	//LZOpenFile( filename, &ofStruct, OF_READ )) < 0 )
	{
		LogFile( ERROR_LOG,"sh3_arcfile::Load( %s ) - ERROR: Unable to open compressed file for reading",filename);
		return 0;
	}


	if( (res = gzread( iHandle, (voidp)&l_sArcFileHeader, sizeof( l_sArcFileHeader ))) != sizeof( l_sArcFileHeader ))
	{
		LogFile( ERROR_LOG, "sh3_arcfile::Load( %s ) - ERROR: Unable to read arc file header (%ld of %ld bytes read) - %s",
			filename,res,sizeof(l_sArcFileHeader),strerror( errno ));
		gzclose( iHandle );
		return -res;
	}

	totalRead = res;

	if( l_sArcFileHeader.fileMarker != 0x20030417 )
	{
		LogFile( ERROR_LOG, "sh3_arcfile::Load( %s ) - ERROR: Does not appear to be the arc file - expected marker of 0x20030417, got 0x%08x",
			filename, l_sArcFileHeader.fileMarker);
		gzclose( iHandle );
		return -totalRead;
	}


	if( (res = gzread( iHandle, (voidp)&m_sArcData, sizeof( m_sArcData ))) != sizeof( m_sArcData ))
	{
		LogFile( ERROR_LOG, "sh3_arcfile::Load( %s ) - ERROR: Unable to read arc data header (%ld of %ld bytes read) - %s",
			filename,res,sizeof(m_sArcData),strerror( errno ));
		gzclose( iHandle );
		return -(totalRead + res);
	}

	delete [] m_pcArcSections;
	m_pcArcSections = new sh3_arc_section_header[ m_sArcData.arcFileCount ];

	for( k = 0; k < m_sArcData.arcFileCount; k++ )
	{
		if( (res = m_pcArcSections[ k ].Load( iHandle )) < 1 )
		{
			LogFile( ERROR_LOG, "sh3_arcfile::Load( %s ) - ERROR: Didn't load %ld of %ld arc section(%ld)",
				filename,k,m_sArcData.arcFileCount,res);
			gzclose( iHandle );
			return -(totalRead - res );
		}
	}

	m_sArcFilename = filename;

	gzclose( iHandle );
	return totalRead;
}



bool sh3_dropList::buildList( sh3_arc_section_header *pcArcSection, char *pFileExt )
{
	long k;
	long lCountFound = 0;
LogFile(ERROR_LOG,"START - sh3_dropList::buildList - EXT [%s]",pFileExt);
	deleteData( );
LogFile( ERROR_LOG, "Deleted Data - Arc section addy is 0x%08x", pcArcSection );
LogFile( ERROR_LOG, "Section name of file we are loading is %s",pcArcSection->sectionName);
	m_lNumFilenames = pcArcSection->GetCountFileType( pFileExt );
LogFile(ERROR_LOG,"START - sh3_dropList::buildList - Num Filenames: %ld",m_lNumFilenames);
	m_pcFilename = new sh3_arc_filename[ m_lNumFilenames ];
	if( m_pcFilename == NULL )
		LogFile( ERROR_LOG,"OUT OF FUCKING EMEMOROSAIDFJSDJFSLKDJLKFJSLKDSJFKDKKKKKKKKKKKKKKKKKKKKK");

	for( k = 0; k < pcArcSection->nameCount; k++ )
	{
		if( pcArcSection->m_pcArcFilenames[ k ].IsFileType( pFileExt ) )
			m_pcFilename[ lCountFound++ ] = pcArcSection->m_pcArcFilenames[ k ];
LogFile(ERROR_LOG,"START - sh3_dropList::buildList - Found: %ld\tChecking [%s]",lCountFound,pcArcSection->m_pcArcFilenames[ k ].fileName);
	}

	return true;
}