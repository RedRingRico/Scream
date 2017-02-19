#include <Memory.h>
#include <Log.h>
#include <shinobi.h>
#include <sg_syhw.h>

extern Uint8 *_BSG_END;

#define MEM_P1_AREA 0x80000000
#define MEM_WORK_END ( ( ( Uint32 ) _BSG_END ) & 0xE0000000 | 0x0D000000 )
#define MEM_HEAP_AREA ( ( void * )( ( ( ( Uint32 ) _BSG_END | MEM_P1_AREA ) & \
	0xFFFFFFE0 ) + 0x20 ) )
#define MEM_HEAP_SIZE ( MEM_WORK_END - ( Uint32 ) MEM_HEAP_AREA )
#define MEM_STRUCT_ALIGN ( sizeof( size_t ) )
#define MEM_MAGIC8 0xAC

#define MEM_BLOCK_FLAG_LOCKED 0x0001
#define MEM_BLOCK_FLAG_FREE 0x0002

#define MEM_Align( pPointer, Alignment ) \
	( ( ( size_t )( pPointer ) + ( Alignment ) - 1 ) & \
		( ~( ( Alignment ) -1 ) ) )
#define MEM_Align32( pPointer, Alignment ) \
	( MEM_Align( pPointer, Alignment ) + Alignment )
#define MEM_IsAligned( pPointer, Alignment ) \
	( ( ( size_t )( pPointer ) & ( ( Alignment ) - 1 ) ) == 0 )

Sint32 MEM_Initialise( PNATIVE_MEMORY_FREESTAT p_pNativeMemoryFreeStat )
{
	syMallocInit( MEM_HEAP_AREA, MEM_HEAP_SIZE );
	syMallocStat( &p_pNativeMemoryFreeStat->Free,
		&p_pNativeMemoryFreeStat->LargestFree );

#if defined ( SCREAM_BUILD_DEBUG )
	LOG_Debug( "Memory initialised with a heap of %ld bytes",
		p_pNativeMemoryFreeStat->Free );
#endif /* SCREAM_BUILD_DEBUG */

	return MEM_OK;
}

void MEM_Terminate( void )
{
	syMallocFinish( );
}

Sint32 MEM_InitialiseMemoryBlock( PMEMORY_BLOCK p_pMemoryBlock,
	void *p_pMemoryPointer, size_t p_Size, Uint8 p_Alignment,
	const char *p_pName )
{
	if( p_pMemoryPointer == NULL )
	{
		LOG_Debug( "NULL memory pointer" );
		return MEM_FATALERROR;
	}

	if( p_pMemoryBlock == NULL )
	{
		LOG_Debug( "NULL memory block" );
		return MEM_FATALERROR;
	}

	p_pMemoryBlock->Alignment = p_Alignment;
	p_pMemoryBlock->StructAlignment = MEM_STRUCT_ALIGN;
	p_pMemoryBlock->PaddedHeaderSize = MEM_Align(
		sizeof( MEMORY_BLOCK_HEADER ), MEM_STRUCT_ALIGN );
	p_pMemoryBlock->pAllocatedBlock = p_pMemoryPointer;
	p_pMemoryBlock->AllocatedSize = p_Size;

	p_pMemoryBlock->pFirstBlock =
		( PMEMORY_BLOCK_HEADER )p_pMemoryBlock->pAllocatedBlock;
	p_pMemoryBlock->pFirstBlock->Size = p_pMemoryBlock->AllocatedSize;
	p_pMemoryBlock->pFirstBlock->Flags = MEM_BLOCK_FLAG_FREE;
	p_pMemoryBlock->pFirstBlock->pNext = NULL;

#if defined ( SCREAM_BUILD_DEBUG )
	memset( p_pMemoryBlock->pFirstBlock->Name, '\0',
		sizeof( p_pMemoryBlock->pFirstBlock->Name ) );
	
	if( strlen( p_pName ) >=
		( sizeof( p_pMemoryBlock->pFirstBlock->Name ) -1 ) )
	{
		memcpy( p_pMemoryBlock->pFirstBlock->Name, p_pName,
			sizeof( p_pMemoryBlock->pFirstBlock->Name ) -1 );
	}
	else
	{
		memcpy( p_pMemoryBlock->pFirstBlock->Name, p_pName,
			strlen( p_pName ) );
	}

#endif /* SCREAM_BUILD_DEBUG */

	return MEM_OK;
}

bool MEM_CreateMemoryBlockHeader( PMEMORY_BLOCK_HEADER p_pMemoryBlockHeader,
	bool p_Free, size_t p_TotalSize, size_t p_DataSize )
{
	Uint8 *pPadding = ( Uint8 * )p_pMemoryBlockHeader;
	PMEMORY_BLOCK_FOOTER pFooter = NULL;

	if( p_DataSize > p_TotalSize )
	{
		LOG_Debug( "Data size greater than total size" );
		return false;
	}

	p_pMemoryBlockHeader->Flags = 0;

	if( p_Free == true )
	{
		p_pMemoryBlockHeader->Flags |= MEM_BLOCK_FLAG_FREE;
	}

	p_pMemoryBlockHeader->Size = p_TotalSize;
	p_pMemoryBlockHeader->DataOffset = p_TotalSize - p_DataSize;

	pPadding += p_pMemoryBlockHeader->DataOffset;
	pPadding -= sizeof( MEMORY_BLOCK_FOOTER );

	pFooter = ( PMEMORY_BLOCK_FOOTER )pPadding;
	pFooter->Padding = ( Uint8 )( p_pMemoryBlockHeader->DataOffset -
		( sizeof( MEMORY_BLOCK_HEADER ) + sizeof( MEMORY_BLOCK_FOOTER ) ) );
	pFooter->Magic = MEM_MAGIC8;

#if defined ( SCREAM_BUILD_DEBUG )
	p_pMemoryBlockHeader->CRC = 0;
#endif /* SCREAM_BUILD_DEBUG */

	return true;
}

size_t MEM_CalculateMemoryBlockHeaderDataOffset(
	PMEMORY_BLOCK_HEADER p_pMemoryBlockHeader, Uint8 p_Alignment )
{
	size_t Position = 0, Start = 0;

	Start = Position = ( size_t )p_pMemoryBlockHeader;

	Position += sizeof( MEMORY_BLOCK_HEADER );
	Position += sizeof( MEMORY_BLOCK_FOOTER );
	Position = MEM_Align( Position, p_Alignment );

	return Position - Start;
}

PMEMORY_BLOCK_HEADER MEM_GetFreeMemoryBlock( PMEMORY_BLOCK p_pMemoryBlock,
	size_t p_Size )
{
	PMEMORY_BLOCK_HEADER pHeader = p_pMemoryBlock->pFirstBlock;
	PMEMORY_BLOCK_HEADER pNewBlock = NULL;
	size_t TotalSize = 0;

	while( pHeader != NULL )
	{
		TotalSize = MEM_GetMemoryBlockSize( pHeader, p_Size,
			p_pMemoryBlock->Alignment, p_pMemoryBlock->StructAlignment );

		if( ( pHeader->Flags & MEM_BLOCK_FLAG_FREE ) &&
			( TotalSize >= p_Size ) )
		{
			size_t FreeSize, FreeTotalSize, FreeOffset;

			pNewBlock = ( PMEMORY_BLOCK_HEADER )(
				( ( Uint8 * )pHeader ) + TotalSize );

			FreeTotalSize = pHeader->Size - TotalSize;
			FreeOffset = MEM_CalculateMemoryBlockHeaderDataOffset( pNewBlock,
				p_pMemoryBlock->Alignment );

			FreeSize = FreeTotalSize - FreeOffset;

			/* Is there enough space to split the block? */
			if( FreeTotalSize > FreeOffset )
			{
				bool MemoryBlockHeaderCreated = false;
				MemoryBlockHeaderCreated = MEM_CreateMemoryBlockHeader(
					pNewBlock, true, FreeTotalSize, FreeSize );

#if defined ( SCREAM_BUILD_DEBUG )
				if( MemoryBlockHeaderCreated == false )
				{
					LOG_Debug( "[MEM_GetFreeMemoryBlock] Failed to cretae a "
						"memory block for the first half (free)" );
				}
#endif /* SCREAM_BUILD_DEBUG */

				/* Verify the magic number is correct */
				pNewBlock->pNext = NULL;

				if( pHeader->pNext != NULL )
				{
					/* Count out any extremely bad values */
					if( pHeader->pNext->DataOffset < 255 )
					{
						PMEMORY_BLOCK_FOOTER pFooter;

						pFooter = ( PMEMORY_BLOCK_FOOTER )(
							( ( Uint8 * )pHeader->pNext +
								pHeader->pNext->DataOffset ) -
							sizeof( MEMORY_BLOCK_FOOTER ) );

						if( pFooter->Magic == MEM_MAGIC8 )
						{
							pNewBlock->pNext = pHeader->pNext;
						}
					}
				}

#if defined ( SCREAM_BUILD_DEBUG )
				if( ( pNewBlock->Name == NULL ) ||
					( strlen( pNewBlock->Name ) == 0 ) )
				{
					memcpy( pNewBlock->Name, pHeader->Name,
						sizeof( pHeader->Name ) );
				}
#endif /* SCREAM_BUILD_DEBUG */

				MemoryBlockHeaderCreated = MEM_CreateMemoryBlockHeader(
					pHeader, false, TotalSize, p_Size );

#if defined ( SCREAM_BUILD_DEBUG )
				if( MemoryBlockHeaderCreated == false )
				{
					LOG_Debug( "[MEM_GetFreeMemoryBlock] Failed to cretae a "
						"memory block for the second half (used)" );
				}
#endif /* SCREAM_BUILD_DEBUG */
				pHeader->pNext = pNewBlock;
			}
			else
			{
				/* No room left to split the block */
				pHeader->Flags &= ~( MEM_BLOCK_FLAG_FREE );
			}

			return pHeader;
		}

		pHeader = pHeader->pNext;
	}

	return NULL;
}

size_t MEM_GetMemoryBlockSize( PMEMORY_BLOCK_HEADER p_pMemoryBlockHeader,
	size_t p_Size, Uint8 p_Alignment, Uint8 p_StructAlign )
{
	size_t Start = 0, End = 0;

	if( p_Size > p_pMemoryBlockHeader->Size )
	{
		/* Unable to align the size requested, return the current block size */
		return p_pMemoryBlockHeader->Size;
	}

	Start = End = ( size_t )p_pMemoryBlockHeader;

	End += MEM_CalculateMemoryBlockHeaderDataOffset( p_pMemoryBlockHeader,
		p_Alignment );
	End += p_Size;
	End = MEM_Align( End, p_StructAlign );

	return End - Start;
}

void *MEM_GetPointerFromMemoryBlockHeader(
	PMEMORY_BLOCK_HEADER p_pMemoryBlockHeader )
{
	Uint8 *pData = ( Uint8 * )p_pMemoryBlockHeader;

	pData += p_pMemoryBlockHeader->DataOffset;

	return ( void * )pData;
}

PMEMORY_BLOCK_HEADER MEM_GetMemoryBlockHeaderFromPointer( void *p_pPointer )
{
	PMEMORY_BLOCK_FOOTER pFooter = NULL;
	Uint8 *pData = ( Uint8 * )p_pPointer;

	pFooter =
		( PMEMORY_BLOCK_FOOTER )( pData - sizeof( MEMORY_BLOCK_FOOTER ) );
	
	pData -= sizeof( MEMORY_BLOCK_HEADER ) + sizeof( MEMORY_BLOCK_FOOTER );
	pData -= pFooter->Padding;

	return ( PMEMORY_BLOCK_HEADER )pData;
}

void MEM_GarbageCollectMemoryBlock( PMEMORY_BLOCK p_pMemoryBlock )
{
	PMEMORY_BLOCK_HEADER pHeader = NULL, pNext = NULL;

	pHeader = p_pMemoryBlock->pFirstBlock;

	while( pHeader )
	{
		pNext = pHeader->pNext;

		if( ( pHeader->Flags & MEM_BLOCK_FLAG_FREE ) &&
			( pNext->Flags & MEM_BLOCK_FLAG_FREE ) )
		{
#if defined ( SCREAM_BUILD_DEBUG )
			/* When freeing, copy the next block's name into this one */
			memcpy( pHeader->Name, pNext->Name, sizeof( pHeader->Name ) );
#endif /* SCREAM_BUILD_DEBUG */
			pHeader->Size += pNext->Size;
			pHeader->pNext = pNext->pNext;
		}
		else
		{
			pHeader = pHeader->pNext;
		}
	}
}

void *MEM_AllocateFromMemoryBlock( PMEMORY_BLOCK p_pMemoryBlock, size_t p_Size,
	const char *p_pName )
{
	PMEMORY_BLOCK_HEADER pNewBlock = NULL;

	if( ( pNewBlock = MEM_GetFreeMemoryBlock( p_pMemoryBlock, p_Size ) ) ==
		NULL )
	{
		MEM_GarbageCollectMemoryBlock( p_pMemoryBlock );
		pNewBlock = MEM_GetFreeMemoryBlock( p_pMemoryBlock, p_Size );
	}

	if( pNewBlock != NULL )
	{
#if defined ( SCREAM_BUILD_DEBUG )
		/* Copy the previous block's name, if necessary */
		if( pNewBlock->pNext->Flags & MEM_BLOCK_FLAG_FREE )
		{
			memcpy( pNewBlock->pNext->Name, pNewBlock->Name,
				sizeof( pNewBlock->Name ) );
		}

		/* Set the new block's name */
		memset( pNewBlock->Name, '\0', sizeof( pNewBlock->Name ) );

		if( strlen( p_pName ) >= ( sizeof( pNewBlock->Name ) - 1 ) )
		{
			memcpy( pNewBlock->Name, p_pName, sizeof( pNewBlock->Name ) - 1 );
		}
		else
		{
			memcpy( pNewBlock->Name, p_pName, strlen( p_pName ) );
		}
#endif /* SCREAM_BUILD_DEBUG */

		return MEM_GetPointerFromMemoryBlockHeader( pNewBlock );
	}

	return NULL;
}

void MEM_FreeFromMemoryBlock( PMEMORY_BLOCK p_pMemoryBlock, void *p_pPointer )
{
	PMEMORY_BLOCK_HEADER pFree =
		MEM_GetMemoryBlockHeaderFromPointer( p_pPointer );
	
	pFree->Flags |= MEM_BLOCK_FLAG_FREE;
}

void *MEM_ReallocateFromMemoryBlock( PMEMORY_BLOCK p_pMemoryBlock,
	size_t p_NewSize, void *p_pOriginalPointer )
{
	PMEMORY_BLOCK_HEADER pHeader = NULL;
	size_t DesiredSize = 0, PointerSize = 0;

	pHeader = MEM_GetMemoryBlockHeaderFromPointer( p_pOriginalPointer );

	PointerSize = pHeader->Size - pHeader->DataOffset;

	if( p_NewSize < PointerSize )
	{
		/* Resize downward, joing with the next memory block if possible */
		if( pHeader->pNext->Flags & MEM_BLOCK_FLAG_FREE )
		{
			PMEMORY_BLOCK_HEADER pNewBlock = NULL;
			size_t NewSize = 0, FreeTotalSize = 0, FreeOffset = 0;

			NewSize = MEM_GetMemoryBlockSize( pHeader, p_NewSize,
				p_pMemoryBlock->Alignment, p_pMemoryBlock->StructAlignment );

			pNewBlock = ( PMEMORY_BLOCK_HEADER )(
				( ( Uint8 * )pHeader ) - NewSize );

			FreeTotalSize = ( pHeader->Size + pHeader->pNext->Size ) - NewSize;
			FreeOffset = MEM_CalculateMemoryBlockHeaderDataOffset( pNewBlock,
				p_pMemoryBlock->Alignment );

			MEM_CreateMemoryBlockHeader( pNewBlock, true, FreeTotalSize,
				FreeTotalSize - FreeOffset );
			pNewBlock->pNext = pHeader->pNext->pNext;

#if defined ( SCREAM_BUILD_DEBUG )
			memcpy( pNewBlock->Name, pHeader->pNext->Name,
				sizeof( pHeader->pNext->Name ) );
#endif /* SCREAM_BUILD_DEBUG */

			MEM_CreateMemoryBlockHeader( pHeader, false, NewSize, p_NewSize );
			pHeader->pNext = pNewBlock;
		}
		else
		{
			/* Reallocate and see if this chunk can be split */
			PMEMORY_BLOCK_HEADER pNewBlock = NULL;
			size_t NewSize = 0, FreeTotalSize = 0, FreeOffset = 0;

			NewSize = MEM_GetMemoryBlockSize( pHeader, p_NewSize,
				p_pMemoryBlock->Alignment, p_pMemoryBlock->StructAlignment );

			pNewBlock = ( PMEMORY_BLOCK_HEADER )(
				( ( Uint8 * )pHeader - NewSize ) );

			FreeTotalSize = pHeader->Size - NewSize;
			FreeOffset = MEM_CalculateMemoryBlockHeaderDataOffset( pNewBlock,
				p_pMemoryBlock->Alignment );

			/* Split possible */
			if( FreeTotalSize > FreeOffset )
			{
				MEM_CreateMemoryBlockHeader( pNewBlock, true, FreeTotalSize,
					FreeTotalSize - FreeOffset );
				pNewBlock->pNext = pHeader->pNext;

#if defined ( SCREAM_BUILD_DEBUG )
				strncpy( pNewBlock->Name, "RESIZED", strlen( "RESIZED" ) );
#endif /* SCREAM_BUILD_DEBUG */

				MEM_CreateMemoryBlockHeader( pHeader, false, NewSize,
					p_NewSize );
				pHeader->pNext = pNewBlock;
			}
		}

		return MEM_GetPointerFromMemoryBlockHeader( pHeader );
	}
	else
	{
		/* Is there enough memory in the next block (plus the next if possible)
		 * to just resize the current chunk of memory? */
		bool MemoryContiguous = false;
		PMEMORY_BLOCK_HEADER pNextBlock = pHeader->pNext;

		DesiredSize = p_NewSize - PointerSize;

		while( pNextBlock != NULL )
		{
			/* Join the blocks if they are both free */
			if( ( pNextBlock->Flags & MEM_BLOCK_FLAG_FREE ) &&
				( pNextBlock->pNext != NULL ) )
			{
				if( pNextBlock->pNext->Flags & MEM_BLOCK_FLAG_FREE )
				{
#if defined ( SCREAM_BUILD_DEBUG )
					memcpy( pNextBlock->Name, pNextBlock->pNext->Name,
						sizeof( pNextBlock->Name ) );
#endif /* SCREAM_BUILD_DEBUG */
					pNextBlock->Size += pNextBlock->pNext->Size;
					pNextBlock->pNext = pNextBlock->pNext->pNext;
				}
			}

			if( pNextBlock->Flags & MEM_BLOCK_FLAG_FREE )
			{
				if( pNextBlock->Size >= DesiredSize )
				{
					MemoryContiguous = true;
					break;
				}

				pNextBlock = pNextBlock->pNext;
			}
			else
			{
				break;
			}
		}

		if( ( pNextBlock == NULL ) || ( MemoryContiguous == false ) )
		{
			PMEMORY_BLOCK_HEADER pNewBlock = NULL;
			void *pNewMemory = NULL;

			MEM_GarbageCollectMemoryBlock( p_pMemoryBlock );

			pNewBlock = MEM_GetFreeMemoryBlock( p_pMemoryBlock, p_NewSize );

			if( pNewBlock == NULL )
			{
				return NULL;
			}

			pNewMemory = MEM_GetPointerFromMemoryBlockHeader( pNewBlock );

			/* Copy the memory contents to the new block */
			memcpy( pNewMemory, p_pOriginalPointer, pHeader->Size );

#if defined ( SCREAM_BUILD_DEBUG )
			memcpy( pNewBlock->Name, pHeader->Name, sizeof( pHeader->Name ) );
#endif /* SCREAM_BUILD_DEBUG */

			/* Free the old memory */
			pHeader->Flags |= MEM_BLOCK_FLAG_FREE;

			return pNewMemory;
		}
		else
		{
			PMEMORY_BLOCK_HEADER pNewBlock = NULL;
			size_t TotalSize = 0, FreeTotalSize = 0, FreeOffset = 0;

			/* Join with the next chunk(s) */
			pHeader->Size += pNextBlock->Size;

			TotalSize = MEM_GetMemoryBlockSize( pHeader, p_NewSize,
				p_pMemoryBlock->Alignment, p_pMemoryBlock->StructAlignment );

			/* split it if possible */
			pNewBlock = ( PMEMORY_BLOCK_HEADER )(
				( ( Uint8 * )pHeader ) - TotalSize );

			FreeTotalSize = pHeader->Size - TotalSize;
			FreeOffset = MEM_CalculateMemoryBlockHeaderDataOffset( pNewBlock,
				p_pMemoryBlock->Alignment );

			if( FreeTotalSize > FreeOffset )
			{
#if defined ( SCREAM_BUILD_DEBUG )
				/* The name can get overwritten by the new block's information
				 * if the next memory block is too close */
				char Name[ 64 ];
				memcpy( Name, pNextBlock->Name, sizeof( Name ) );
#endif /* SCREAM_BUILD_DEBUG */

				MEM_CreateMemoryBlockHeader( pNewBlock, true, FreeTotalSize,
					FreeTotalSize - FreeOffset );
				pNewBlock->pNext = pNextBlock->pNext;

#if defined ( SCREAM_BUILD_DEBUG )
				if( ( pNewBlock->Name == NULL ) ||
					( strlen( pNewBlock->Name ) == 0 ) )
				{
					if( pNextBlock != NULL )
					{
						memcpy( pNewBlock->Name, Name, sizeof( Name ) );
					}
					else
					{
						memcpy( pNewBlock->Name, pHeader->Name,
							sizeof( pNewBlock->Name ) );
					}
				}
#endif /* SCREAM_BUILD_DEBUG */
				
				MEM_CreateMemoryBlockHeader( pHeader, false, TotalSize,
					p_NewSize );
				pHeader->pNext = pNewBlock;
			}
			else
			{
				/* Cannot split it */
				pHeader->Flags &= ~( MEM_BLOCK_FLAG_FREE );
			}

			return MEM_GetPointerFromMemoryBlockHeader( pHeader );
		}
	}

	return NULL;
}

size_t MEM_GetFreeMemoryBlockSize( PMEMORY_BLOCK p_pMemoryBlock )
{
	size_t FreeMemory = 0;
	PMEMORY_BLOCK_HEADER pMemoryBlockHeader = p_pMemoryBlock->pFirstBlock;

	while( pMemoryBlockHeader )
	{
		if( pMemoryBlockHeader->Flags & MEM_BLOCK_FLAG_FREE )
		{
			FreeMemory += pMemoryBlockHeader->Size;
		}

		pMemoryBlockHeader = pMemoryBlockHeader->pNext;
	}

	return FreeMemory;
}

size_t MEM_GetUsedMemoryBlockSize( PMEMORY_BLOCK p_pMemoryBlock )
{
	size_t UsedMemory = 0;
	PMEMORY_BLOCK_HEADER pMemoryBlockHeader = p_pMemoryBlock->pFirstBlock;

	while( pMemoryBlockHeader )
	{
		if( ( pMemoryBlockHeader->Flags & MEM_BLOCK_FLAG_FREE ) == 0 )
		{
			UsedMemory += pMemoryBlockHeader->Size;
		}

		pMemoryBlockHeader = pMemoryBlockHeader->pNext;
	}

	return UsedMemory;
}

void MEM_ListMemoryBlocks( PMEMORY_BLOCK p_pMemoryBlock )
{
#if defined ( SCREAM_BUILD_DEBUG ) || defined ( SCREAM_BUILD_DEVELOPMENT )
	PMEMORY_BLOCK_HEADER pMemoryBlockHeader = p_pMemoryBlock->pFirstBlock;

	LOG_Debug( "Memory block dump" );
	LOG_Debug( "\tFree: %ld", MEM_GetFreeMemoryBlockSize( p_pMemoryBlock ) );
	LOG_Debug( "\tUsed: %ld", MEM_GetUsedMemoryBlockSize( p_pMemoryBlock ) );

	while( pMemoryBlockHeader )
	{
#if defined ( SCREAM_BUILD_DEBUG )
		LOG_Debug( "\t%s | %s: %ld", pMemoryBlockHeader->Name,
			( pMemoryBlockHeader->Flags & MEM_BLOCK_FLAG_FREE ) ? "FREE" :
				"USED", pMemoryBlockHeader->Size );
#elif defined ( SCREAM_BUILD_DEVELOPMENT )
		LOG_Debug( "\t%s: %ld",
			( pMemoryBlockHeader->Flags & MEM_BLOCK_FLAG_FREE ) ? "FREE" :
				"USED", pMemoryBlockHeader->Size );
#endif /* SCREAM_BUILD_DEBUG */
		pMemoryBlockHeader = pMemoryBlockHeader->pNext;
	}
#endif /* SCREAM_BUILD_DEBUG || SCREAM_BUILD_DEVELOPMENT */
}

