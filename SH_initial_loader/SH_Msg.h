#ifndef  __SH_MSG_H__
#define  __SH_MSG_H__

#include <windows.h>
#include "typedefs.h"

//---[ Message Types ]---/
#define MSG_LANG_ENGLISH	0x00000001
#define MSG_LANG_GERMAN		0x00000002
#define MSG_LANG_FRENCH		0x00000004
#define MSG_LANG_ITALIAN	0x00000008
#define MSG_LANG_CHINESE	0x00000010
#define MSG_LANG_JAPANESE	0x00000020
#define MSG_LANG_SPANISH	0x00000040
#define MSG_LANG_KOREAN		0x00000080

#define MSG_CONVERT_SH3_TO_ASCII( x )		((x) + 32 )
#define MSG_CONVERT_ASCII_TO_SH3( x )		((x) - 32 )

class SH3_Msg
{
public:
	SH3_Msg( char *_msg = NULL, long _len = 0, short _id = 0, short _alt_id = 0 ){ SetNullAll( ); if( _msg )SetMsg( _msg, _len, _id, _alt_id ); }
	SH3_Msg( const SH3_Msg & rhs ){ SetNullAll( ); operator=( rhs ); }
	~SH3_Msg( ){ ReleaseAllData( ); }
	SH3_Msg & operator=( const SH3_Msg & rhs ){ if( &rhs != this ){ SetMsg( rhs.m_pcMsg, rhs.m_lLength, rhs.m_sID, rhs.m_sAltID ); }return *this; }

	void SetNullAll( ){ m_pcMsg = NULL; m_lLength = 0; m_sID = 0; m_sAltID = 0; }
	void ReleaseAllData( ){ if( m_pcMsg )delete [] m_pcMsg; SetNullAll( ); }

	void SetMsg( char *_msg, long _len, short _id, short _alt_id )
	{
		long k;
		ReleaseAllData( );
		if( !_msg || !_len )
			return;
		if( _msg[ _len - 1 ] == 0 )
			m_lLength = _len;
		else
			m_lLength = _len + 1;

		m_pcMsg = new char[ m_lLength ];
		for( k = 0; k < m_lLength; k++ )
			m_pcMsg[ k ] = _msg[ k ];
		m_pcMsg[ m_lLength - 1 ] = '\0';

		m_sID = _id;
		m_sAltID = _alt_id;
	}


	char	*GetMsg( char *_msg, short _id )
	{
		long k = 0;
		char *l_pcPos = _msg;
		short *l_psControl;
LogFile( ERROR_LOG, "SH3_Msg::GetMsg( ) - Start: addy of _msg = 0x%08x\t_id = %d",_msg, _id );
		if( !_msg )
			return NULL;

		ReleaseAllData( );

		while( !(_msg[ k ] == -1 && _msg[ k + 1 ] == -1) )		//Endpoint for msg is 0xff, 0xff
			l_pcPos = &(_msg[ k++ ]);

	//	if( _msg[ k + 1 ] == -1  )
	//	k++;
LogFile( ERROR_LOG, "SH3_Msg::GetMsg( ) - Endpoint of message is %ld",k );
		m_lLength = k + 1;
		m_pcMsg = new char[ m_lLength + 1 ];

		//---[ Get the message, and convert to a usable format ]---/
		for( k = 0; k < m_lLength; k++ )
		{
			m_pcMsg[ k ] = _msg[ k ];
			if( m_pcMsg[ k ] < 0 || m_pcMsg[ k ] > 95 )
			{
				if( m_pcMsg[ k ] == -1 )	// && _msg[ k + 1 ] == -3 )
				{
					m_pcMsg[ k + 1 ] = _msg[ k + 1 ];
					if( m_pcMsg[ k + 1 ] == -3 )		//Newline
					{
						m_pcMsg[ k++ ] = ' ';
						m_pcMsg[ k ] = '\n';
					}
					else if( m_pcMsg[ k + 1 ] == -6 )	//3 lines of black bars across screen
					{
						m_pcMsg[ k++ ] = '&';
						m_pcMsg[ k ] = 'B';
					}
					else if( m_pcMsg[ k + 1 ] == -5 )	//Flashing Red (when highlighted)
					{
						m_pcMsg[ k++ ] = '&';
						m_pcMsg[ k ] = 'R';
					}
					else if( m_pcMsg[ k + 1 ] == 0 )	//White, or clear formatting
					{
						m_pcMsg[ k++ ] = '&';
						m_pcMsg[ k ] = 'w';
					}
					else if( m_pcMsg[ k + 1 ] == 2 )	//Red font (maybe)
					{
						m_pcMsg[ k++ ] = '&';
						m_pcMsg[ k ] = 'r';
					}
					else if( m_pcMsg[ k + 1 ] == 3 )	//Green font
					{
						m_pcMsg[ k++ ] = '&';
						m_pcMsg[ k ] = 'g';
					}
					else if( m_pcMsg[ k + 1 ] == 4 )	//Blue
					{
						m_pcMsg[ k++ ] = '&';
						m_pcMsg[ k ] = 'b';
					}
					else if( m_pcMsg[ k + 1 ] == 1 )	//cyan font
					{
						m_pcMsg[ k++ ] = '&';
						m_pcMsg[ k ] = 'c';
					}
					else if( m_pcMsg[ k + 1 ] == -10 )	//Yes or no Prompt
					{
						m_pcMsg[ k++ ] = '&';
						m_pcMsg[ k ] = 'P';
					}
					else if( m_pcMsg[ k + 1 ] == -7 )	//Option (each option is preceded by this code)
					{
						m_pcMsg[ k++ ] = '&';
						m_pcMsg[ k ] = 'O';
					}
					else if( m_pcMsg[ k + 1 ] == -32 )	//Argument (I think)
					{
						m_pcMsg[ k++ ] = '%';
						m_pcMsg[ k ] = '1';
					}
					else if( m_pcMsg[ k + 1 ] == -31 )	//2nd Argument (I think)
					{
						m_pcMsg[ k++ ] = '%';
						m_pcMsg[ k ] = '2';
					}
					else if( m_pcMsg[ k + 1 ] == -30 )	//3rd Argument (I think)
					{
						m_pcMsg[ k++ ] = '%';
						m_pcMsg[ k ] = '3';
					}
					else
					{
						if( k + 1 == m_lLength )
							m_pcMsg[ k ] = '\0';
						else
							LogFile( ERROR_LOG, "SH3_Msg::GetMsg( ) - WARNING: Unknown control code: %d or %d (0x%.2x) found at position %ld of %ld",m_pcMsg[ k+1 ], (unsigned char)m_pcMsg[ k+1 ], (unsigned char)m_pcMsg[ k+1 ], k+1, m_lLength  );
					}
				}
				else if( m_pcMsg[ k ] == -3 )
					m_pcMsg[ k ] = '\n';
				else if( m_pcMsg[ k ] == -32 )
					m_pcMsg[ k ] = '%';
				else if( m_pcMsg[ k ] == -1 )
					m_pcMsg[ k ] = '\0';
				else if( m_pcMsg[ k ] == -47 )	//The enya (tilde-n)
					m_pcMsg[ k ] = 241;
				else if( m_pcMsg[ k ] == -114 )	//Trademark
					m_pcMsg[ k ] = 153;
				else if( m_pcMsg[ k ] == -55 )	//The accented 'e' in cafe
					m_pcMsg[ k ] = 233;
				else if( m_pcMsg[ k ] == -8 )	//Text Position Control (H is top of screen)
					m_pcMsg[ k ] = '#';
				else
					LogFile( ERROR_LOG, "SH3_Msg::GetMsg( ) - WARNING: Value of %d or %d (0x%2.2x) found at position %ld of %ld",m_pcMsg[ k ],(unsigned char)m_pcMsg[ k ], m_pcMsg[ k ], k+1, m_lLength );
			}
			else
				m_pcMsg[ k ] = MSG_CONVERT_SH3_TO_ASCII( m_pcMsg[ k ] );
		}
		
		m_pcMsg[ m_lLength - 1 ] = '\0';

		k = m_lLength + 1;

		if( k % 2 )
			k++;

		l_psControl = (short *)&(_msg[ k ] );

		m_sID = _id;
		m_sAltID = l_psControl[ 0 ];

		LogFile( ERROR_LOG, "SH3_Msg::GetMsg( ) - DEBUG: The message set is %d : %d or %d, the length is %ld, and the message is:\n\t\t[%s]",m_sID,m_sAltID,(unsigned short)m_sAltID, m_lLength, m_pcMsg );
		//LogFile( ERROR_LOG, "Check: The value of the alt is %d or %d at address 0x%08x.  The val to return is 0x%08x",l_psControl[0],(unsigned short)l_psControl[0],l_psControl,l_psControl + 1 );
//		if( m_sAltID == -32768 )
//			return (char *)&(l_psControl[ 0 ]);
		return (char *)&(l_psControl[ 1 ]);
	}

	char	*m_pcMsg;
	long	m_lLength;
	short	m_sID;
	short	m_sAltID;
};

class SH3_MsgMgr
{
public:
	SH3_MsgMgr( char *_pcFilename = NULL, long _lIndexNum = 0 ){ SetNullAll( ); if( _pcFilename )m_lNumMessages = LoadMsgs( _pcFilename, _lIndexNum ); }
	SH3_MsgMgr( const SH3_MsgMgr & rhs ){ SetNullAll( ); operator=( rhs ); }
	~SH3_MsgMgr( ){ ReleaseAllData( ); }
	SH3_MsgMgr & operator=( const SH3_MsgMgr & rhs ){ if( &rhs != this ){ ReleaseAllData( ); m_lNumMessages = rhs.m_lNumMessages; m_pcMessages = new SH3_Msg[ m_lNumMessages ]; for( long k = 0; k < m_lNumMessages; k++ )m_pcMessages[ k ] = rhs.m_pcMessages[ k ]; } return *this; }

	void SetNullAll( ){ m_pcMessages = NULL; m_lNumMessages = 0; }
	void ReleaseAllData( ){ delete [] m_pcMessages; SetNullAll( ); }

//	SH3_Msg &operator[](int index);                // Mutator
//	SH3_Msg  operator[](int index) const;          // Accessor

	long LoadMsgs( char *_pcFilename, long _lIndexNum );

	SH3_Msg		*m_pcMessages;
	long		m_lNumMessages;
};


class SH3_AllMsg
{
public:
	SH3_AllMsg( long _lLangFlag = 0 ){ SetNullAll( ); }
	SH3_AllMsg( const SH3_AllMsg & rhs ){ SetNullAll( ); operator=( rhs ); }
	~SH3_AllMsg( ){ ReleaseAllData( ); }
	SH3_AllMsg & operator=( const SH3_AllMsg & rhs ){ if( &rhs != this ){ ReleaseAllData( ); m_lNumAll = rhs.m_lNumAll; m_pcAllMsg = new SH3_MsgMgr[ m_lNumAll ]; for( long k = 0; k < m_lNumAll; k++ )m_pcAllMsg[ k ] = rhs.m_pcAllMsg[ k ]; } return *this; }

	void SetNullAll( ){ m_lNumAll = 0; m_pcAllMsg = NULL; }
	void ReleaseAllData( ){ delete [] m_pcAllMsg; SetNullAll( ); }

	long LoadAll( char *_baseSH3dir, long _lLangFlag );

	SH3_MsgMgr	*m_pcAllMsg;
	long		m_lNumAll;
};

#endif /*__SH_MSG_H__*/