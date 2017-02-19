#include <Hardware.h>
#include <Log.h>
#include <FileSystem.h>

static KMVOID PALExtCallback( PKMVOID p_pArgs );

Sint32 HW_Initialise( KMBPPMODE p_BPP,
	PNATIVE_MEMORY_FREESTAT p_pMemoryFreeStat )
{
	KMDISPLAYMODE DisplayMode;

	switch( syCblCheck( ) )
	{
		case SYE_CBL_NTSC:
		{
			DisplayMode = KM_DSPMODE_NTSCNI640x480;
			break;
		}
		case SYE_CBL_PAL:
		{
			DisplayMode = KM_DSPMODE_PALNI640x480EXT;
			break;
		}
		case SYE_CBL_VGA:
		{
			DisplayMode = KM_DSPMODE_VGA;
			break;
		}
		default:
		{
			HW_Reboot( );
		}
	}

	set_imask( 15 );

	/* Initialise the CPU, G1 bus, interrupt controller, cache, and timer */
	syHwInit( );
	MEM_Initialise( p_pMemoryFreeStat );
	syStartGlobalConstructor( );
	kmInitDevice( KM_DREAMCAST );

	kmSetDisplayMode( DisplayMode, p_BPP, KM_DITHER, KM_AAMODE );
	kmSetWaitVsyncCount( 1 );

	/* Initialise the interrupt controller and libraries required for after the
	 * graphics library is initialised */
	syHwInit2( );

	/* Start the real-time clock */
	syRtcInit( );

	set_imask( 0 );

	if( FS_Initialise( ) != FS_OK )
	{
		LOG_Debug( "[HW_Initialise] <ERROR> GD-ROM initialisation error" );

		return HW_GDROMERROR;
	}

	if( syCblCheck( ) == SYE_CBL_PAL )
	{
		kmSetPALEXTCallback( PALExtCallback, NULL );
		kmSetDisplayMode( DisplayMode, p_BPP, KM_DITHER, KM_AAMODE );
	}

	return HW_OK;
}

void HW_Terminate( void )
{
	FS_Terminate( );
	syRtcFinish( );
	kmUnloadDevice( );
	syStartGlobalDestructor( );
	MEM_Terminate( );
	syHwFinish( );
	set_imask( 15 );
}

void HW_Reboot( void )
{
	syBtExit( );
}

Sint32 HW_GetConsoleID( PHW_CONSOLEID p_pConsoleID )
{
	if( syCfgGetIndividualID( p_pConsoleID->ConsoleID ) != SYD_CFG_IID_OK )
	{
		LOG_Debug( "Failed to acquire the console's ID" );

		memset( p_pConsoleID->ConsoleID, 0,
			sizeof( p_pConsoleID->ConsoleID ) );
		memset( p_pConsoleID->ConsoleIDString, '\0',
			sizeof( p_pConsoleID->ConsoleIDString ) );

		return HW_FATALERROR;
	}

	sprintf( p_pConsoleID->ConsoleIDString, "%02X%02X%02X%02X%02X%02X",
		( unsigned char )p_pConsoleID->ConsoleID[ 0 ],
		( unsigned char )p_pConsoleID->ConsoleID[ 1 ],
		( unsigned char )p_pConsoleID->ConsoleID[ 2 ],
		( unsigned char )p_pConsoleID->ConsoleID[ 3 ],
		( unsigned char )p_pConsoleID->ConsoleID[ 4 ],
		( unsigned char )p_pConsoleID->ConsoleID[ 5 ] );

	p_pConsoleID->ConsoleIDString[ ( SYD_CFG_IID_SIZE * 2 ) ] = '\0';

	return HW_OK;
}

static KMVOID PALExtCallback( PKMVOID p_pArgs )
{
	PKMPALEXTINFO pPALInfo;

	pPALInfo = ( PKMPALEXTINFO )p_pArgs;
	pPALInfo->nPALExtMode = KM_PALEXT_HEIGHT_RATIO_1_166;
}

