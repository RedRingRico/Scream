#include <Hardware.h>
#include <Memory.h>
#include <Log.h>
#include <GitVersion.h>

void main( void )
{
	size_t SystemMemorySize, GraphicsMemorySize, AudioMemorySize;
	void *pSystemMemory = NULL, *pGraphicsMemory = NULL, *pAudioMemory = NULL;
	MEMORY_BLOCK SystemMemoryBlock, GraphicsMemoryBlock, AudioMemoryBlock;
	NATIVE_MEMORY_FREESTAT MemoryFree;

	if( HW_Initialise( KM_DSPBPP_RGB888, &MemoryFree ) != HW_OK )
	{
		goto MainCleanup;
	}

	LOG_Initialise( NULL );
	LOG_Debug( "SCREAM" );
	LOG_Debug( "Version: %s", GIT_VERSION );

	SystemMemorySize = MEM_MIB( 4 );
	GraphicsMemorySize = MEM_MIB( 4 );
	AudioMemorySize = MEM_MIB( 1 );

	if( ( pSystemMemory = syMalloc( SystemMemorySize ) ) == NULL )
	{
		LOG_Debug( "Could not allocate system memory of size: %ld",
			SystemMemorySize );

		goto MainCleanup;
	}

	if( MEM_InitialiseMemoryBlock( &SystemMemoryBlock, pSystemMemory,
		SystemMemorySize, 32, "System memory" ) != MEM_OK )
	{
		LOG_Debug( "Failed to initialise the system memory block fo size: %ld",
			SystemMemorySize );

		goto MainCleanup;
	}

	/*while( 1 )
	{
	}*/

MainCleanup:
	HW_Terminate( );
	HW_Reboot( );
}

