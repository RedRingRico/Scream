#include <FileSystem.h>
#include <Log.h>

#define GDROM_MAX_OPEN_FILES 20
#define GDROM_BUFFER_COUNT 512
#define GDROM_RETRY_COUNT 10

static Uint8 g_GDFSWork[ GDFS_WORK_SIZE( GDROM_MAX_OPEN_FILES ) + 32 ];
static Uint8 g_GDFSHandleTable[ GDFS_DIRREC_SIZE( GDROM_BUFFER_COUNT ) + 32 ];
static GDFS_DIRREC g_RootDirectory;

static void GDFSErrorCallback( void *p_pObject, long p_Error );

Sint32 FS_Initialise( void )
{
	Uint8 *pWork = NULL, *pDirectory = NULL;
	Sint32 Error = 0, Itr = 0;
	Uint32 RootDirectoryBuffer[ GDFS_DIRREC_SIZE( 64 ) ];

	pWork =
		( Uint8 * )( ( ( Uint32 )g_GDFSWork & 0xFFFFFFE0 ) + 0x20 );
	
	pDirectory =
		( Uint8 * )( ( ( Uint32 )g_GDFSHandleTable & 0xFFFFFFE0 ) + 0x20 );
	
	for( Itr = GDROM_RETRY_COUNT; Itr > 0; --Itr )
	{
		Error = gdFsInit( GDROM_MAX_OPEN_FILES, pWork, GDROM_BUFFER_COUNT,
			pDirectory );

		if( ( Error == GDD_ERR_TRAYOPEND ) ||
			( Error == GDD_ERR_UNITATTENT ) )
		{
#if defined ( SCREAM_BUILD_DEBUG )
			LOG_Debug( "[FS_Initialise] <WARNING> GD-ROM drive door open" );
#else
			return FS_FATALERROR;
#endif /* SCREAM_BUILD_DEBUG */
		}
		else if( Error == GDD_ERR_OK )
		{
			break;
		}
		else if( Error == GDD_ERR_NOTREADY )
		{
			LOG_Debug( "[FS_Initialise] <WARNING> GD-ROM drive not ready" );
		}
		else
		{
			LOG_Debug( "[FS_Initialise] <INFO> Unknown GD-ROM error code: %d",
				Error );
		}
	}

	if( Itr == 0 )
	{
		LOG_Debug( "[FS_Initialise] <ERROR> Unable to initialise GD-ROM "
			"drive" );

		return FS_DRIVEERROR;
	}

	gdFsEntryErrFuncAll( GDFSErrorCallback, NULL );

	g_RootDirectory = gdFsCreateDirhn( RootDirectoryBuffer, 64 );
	gdFsLoadDir( ".", g_RootDirectory );

#if defined ( SCREAM_BUILD_DEBUG )
	LOG_Debug( "[FS_Initialise] GD-ROM Initialised" );
	LOG_Debug( "\tMaximum open files: %d", GDROM_MAX_OPEN_FILES );
	LOG_Debug( "\tOpen buffer count: %d", GDROM_BUFFER_COUNT );
	LOG_Debug( "\t%s", GDD_VERSION_STR );
#endif /* SCREAM_BUILD_DEBUG */

	return FS_OK;
}

void FS_Terminate( void )
{
	gdFsFinish( );
}

static void GDFSErrorCallback( void *p_pObject, long p_Error )
{
}

