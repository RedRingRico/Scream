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

	if( ( pGraphicsMemory = syMalloc( GraphicsMemorySize ) ) == NULL )
	{
		LOG_Debug( "Could not allocate graphics memory of size: %ld",
			GraphicsMemorySize );

		goto MainCleanup;
	}

	if( ( pAudioMemory = syMalloc( AudioMemorySize ) ) == NULL )
	{
		LOG_Debug( "Could not allocate audio memory of size: 5ld",
			AudioMemorySize );

		goto MainCleanup;
	}

	if( MEM_InitialiseMemoryBlock( &SystemMemoryBlock, pSystemMemory,
		SystemMemorySize, 32, "System memory" ) != MEM_OK )
	{
		LOG_Debug( "Failed to initialise the system memory block of size: %ld",
			SystemMemorySize );

		goto MainCleanup;
	}

	if( MEM_InitialiseMemoryBlock( &GraphicsMemoryBlock, pGraphicsMemory,
		GraphicsMemorySize, 32, "Graphics memory" ) != MEM_OK )
	{
		LOG_Debug( "Failed to initialise the graphics memory block of size: "
			"%ld", GraphicsMemorySize );

		goto MainCleanup;
	}

	if( MEM_InitialiseMemoryBlock( &AudioMemoryBlock, pAudioMemory,
		AudioMemorySize, 32, "Audio memory" ) != MEM_OK )
	{
		LOG_Debug( "Failed to initialise the audio memory block of size %ld",
			AudioMemorySize );

		goto MainCleanup;
	}

MainCleanup:
	
	if( pAudioMemory != NULL )
	{
		MEM_GarbageCollectMemoryBlock( &AudioMemoryBlock );
		MEM_ListMemoryBlocks( &AudioMemoryBlock );
		syFree( pAudioMemory );
	}

	if( pGraphicsMemory != NULL )
	{
		MEM_GarbageCollectMemoryBlock( &GraphicsMemoryBlock );
		MEM_ListMemoryBlocks( &GraphicsMemoryBlock );
		syFree( pGraphicsMemory );
	}

	if( pSystemMemory != NULL )
	{
		MEM_GarbageCollectMemoryBlock( &SystemMemoryBlock );
		MEM_ListMemoryBlocks( &SystemMemoryBlock );
		syFree( pSystemMemory );
	}

	LOG_Terminate( );
	HW_Terminate( );
	HW_Reboot( );
}

