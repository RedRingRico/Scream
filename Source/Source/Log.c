#if defined ( SCREAM_BUILD_DEBUG ) || defined ( SCREAM_BUILD_DEVELOPMENT )
#include <Log.h>
#include <shinobi.h>
#include <sn_fcntl.h>
#include <usrsnasm.h>
#include <SHC/errno.h>
#include <stdarg.h>

static int g_FileHandle = -1;

static void DebugOut( const char *p_pString )
{
	if( ( p_pString != NULL ) && ( strlen( p_pString ) > 0 ) )
	{
		debug_write( SNASM_STDOUT, p_pString, strlen( p_pString ) );
	}
}

Sint32 LOG_Initialise_Internal( const char *p_pLogFileName )
{
	Sint32 InitialiseReturn = LOG_OK;

	/* Logging to a file is only available for debug builds as development
	 * builds need the network to be free of traffic */
#if defined ( SCREAM_BUILD_DEBUG )
	char *pLogFile = NULL;

	DebugOut( "Initialising log file" );

	if( ( p_pLogFileName != NULL ) && ( strlen( p_pLogFileName ) > 0 ) )
	{
		/* As the memory manager may not be initialised, use the built-in
		 * memory allocation functions */
		pLogFile = syMalloc( ( strlen( p_pLogFileName ) ) + 1 );
		strncpy( pLogFile, p_pLogFileName, strlen( p_pLogFileName ) );
		pLogFile[ strlen( p_pLogFileName ) ] = '\0';
	}
	else
	{
		SYS_RTC_DATE Date;
		char DateFile[ 24 ];

		DateFile[ 23 ] = '\0';
		syRtcGetDate( &Date );

		/* Format YYY-MM-DD_HH-MM-SS */
		sprintf( DateFile, "%04d-%02d-%02d_%02d-%02d-%02d.log", Date.year,
			Date.month, Date.day, Date.hour, Date.minute, Date.second );

		pLogFile = syMalloc( strlen( DateFile ) + 1 );
		strncpy( pLogFile, DateFile, strlen( DateFile ) );
		pLogFile[ strlen( DateFile ) ] = '\0';
	}

	if( ( g_FileHandle = debug_open( pLogFile,
		SNASM_O_WRONLY | SNASM_O_CREAT | SNASM_O_TRUNC | SNASM_O_TEXT,
		SNASM_S_IREAD | SNASM_S_IWRITE ) ) == -1 )
	{
		DebugOut( "[ERROR] Failed to open log file for writing" );

		switch( errno )
		{
			case SNASM_EACCESS:
			{
				DebugOut( "ERROR: Access" );
				break;
			}
			default:
			{
				DebugOut( "ERROR: Unknown" );
			}
		}

		InitialiseReturn = LOG_ERROR;
	}

#endif /* SCREAM_BUILD_DEBUG */

	return InitialiseReturn;
}

void LOG_Terminate_Internal( void )
{
#if defined ( SCREAM_BUILD_DEBUG )
	if( g_FileHandle != -1 )
	{
		debug_close( g_FileHandle );
		g_FileHandle = -1;
	}
#endif /* SCREAM_BUILD_DEBUG */
}

void LOG_Debug_Internal( const char *p_pMessage, ... )
{
	if( ( strlen( p_pMessage ) > 0 ) && ( p_pMessage != NULL ) )
	{
		static char Buffer[ 256 ];
		va_list Args;
		
		sprintf( Buffer, "[DEBUG] " );

		va_start( Args, p_pMessage );
		vsprintf( Buffer + strlen( "[DEBUG] " ), p_pMessage, Args );
		va_end( Args );

#if defined ( SCREAM_BUILD_DEBUG )
		debug_write( g_FileHandle, Buffer, strlen( Buffer ) );
#endif /* SCREAM_BUILD_DEBUG */
		debug_write( SNASM_STDOUT, Buffer, strlen( Buffer ) );
	}
}

#else
#include <Log.h>
#endif /* SCREAM_BUILD_DEBUG || SCREAM_BUILD_DEVELOPMENT */

