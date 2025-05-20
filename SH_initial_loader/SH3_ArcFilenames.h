#ifndef  __READARCLIST_H__
#define  __READARCLIST_H__

//#include <windows.h>
#include <lzexpand.h>
#include <string>
#include <zlib.h>

//----[ STRUCTURES TO LOAD arc.arc FILE ]----/

typedef struct
{
	long	fileMarker;		//Value is 0x20030417
	long	f1_af;
	long	f2_af;
	long	f3_af;
}sh3_arc_file_header;


typedef struct
{
	short	structType;		//It's 1 for file data header
	short	headerSize;		//Total size of this header
	long	arcFileCount;	//Count of arc file extensions
	long	filenameCount;	//Count of filenames in this file
}sh3_arc_data_header;


typedef struct
{
	short	structType;		//It's 3 for arc filename
	short	dataSize;		//Total size of this filename in bytes
	short	fileIndex;		//index for this filename for the current section (starts at 0)
	short	sectIndex;		//Index of current section (starts at 0)
}sh3_arc_file_loader;


class sh3_arc_filename
{
public:
	short	structType;		//It's 3 for arc filename
	short	dataSize;		//Total size of this filename in bytes
	short	fileIndex;		//index for this filename for the current section (starts at 0)
	short	sectIndex;		//Index of current section (starts at 0)
	char	*fileName;		//Name of file (ending with a null)

	sh3_arc_filename( gzFile iHandle = NULL ){ fileName = NULL; Load( iHandle ); }
	sh3_arc_filename( const sh3_arc_filename & rhs ){ fileName = NULL; operator=( rhs ); }
	~sh3_arc_filename( ){ delete [] fileName; }
	sh3_arc_filename & operator=( const sh3_arc_filename & rhs );

	int Load( gzFile iHandle );
	bool IsFileType( char *pcFileExt );
};


typedef struct
{
	short	structType;		//It's 2 for arc section header
	short	headerSize;		//Total size of this header in bytes
	long	nameCount;		//Count of names in this section
}sh3_arc_section_loader;

class sh3_arc_section_header
{
public:
	short				structType;			//It's 2 for arc section header
	short				headerSize;			//Total size of this header in bytes
	long				nameCount;			//Count of names in this section
	char				*sectionName;		//Name of section (matches arc file base name)
	sh3_arc_filename	*m_pcArcFilenames;	//Set of filenames for this section

	sh3_arc_section_header( gzFile iHandle = NULL ){ sectionName = NULL; m_pcArcFilenames = NULL; Load( iHandle ); }
	sh3_arc_section_header( const sh3_arc_section_header & rhs ){ sectionName = NULL; m_pcArcFilenames = NULL; operator=( rhs ); }
	~sh3_arc_section_header( ){ delete [] sectionName; delete [] m_pcArcFilenames; }
	sh3_arc_section_header & operator=( const sh3_arc_section_header & rhs );

	int Load( gzFile iHandle );

	long GetCountFileType( char *pcFileExt );	//Returns count of a particular file extension
};




class sh3_arcfile
{
public:
	sh3_arc_data_header		m_sArcData;			//Contains arc data header, with count of sections
	sh3_arc_section_header	*m_pcArcSections;	//Contains section header, and all filenames in section
	std::string				m_sArcFilename;		//Name and path of arc.arc

	sh3_arcfile( char *filename = NULL ):m_sArcFilename( "" ){ m_pcArcSections = NULL; if( filename )Load( filename ); }
	sh3_arcfile( const sh3_arcfile & rhs ){ operator=( rhs ); }
	~sh3_arcfile( ){ delete [] m_pcArcSections; }
	sh3_arcfile & operator=( const sh3_arcfile & rhs );

	int Load( char *filename );
};


class sh3_dropList
{
public:
	sh3_arc_filename	*m_pcFilename;
	long				m_lNumFilenames;

	sh3_dropList( ){ m_pcFilename = NULL; m_lNumFilenames = 0; }
	sh3_dropList( const sh3_dropList & rhs ){ m_pcFilename = NULL; m_lNumFilenames = 0; operator=( rhs ); }
	~sh3_dropList( ){ deleteData( ); }
	sh3_dropList & operator=( const sh3_dropList & rhs ){ if( &rhs != this ){ long k; deleteData( ); m_lNumFilenames = rhs.m_lNumFilenames; m_pcFilename = new sh3_arc_filename[ m_lNumFilenames ]; for( k = 0; k < m_lNumFilenames; k++ ) m_pcFilename[ k ] = rhs.m_pcFilename[ k ]; } return *this; }

	void deleteData( ){ delete [] m_pcFilename; m_lNumFilenames = 0; }

	bool buildList( sh3_arc_section_header *pcArcSection, char *pFileExt );
	bool buildListMDL( sh3_arc_section_header *pcArcSection ){ return buildList( pcArcSection, ".mdl" ); }
	bool buildListMAP( sh3_arc_section_header *pcArcSection ){ return buildList( pcArcSection, ".map" ); }
};





#endif/*__READARCLIST_H__*/