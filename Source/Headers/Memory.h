#ifndef __SCREAM_MEMORY_H__
#define __SCREAM_MEMORY_H__

#include <sg_xpt.h>
#include <sg_maloc.h>

#define MEM_SH4_P2NonCachedMemory( pAddress ) \
	( ( ( ( long ) pAddress ) & 0x0FFFFFFF ) | 0xA0000000 )

#define MEM_KIB( Amount )( Amount << 10 )
#define MEM_MIB( Amount )( Amount << 20 )

#define MEM_OK 0
#define MEM_ERROR 1
#define MEM_FATALERROR -1

typedef struct _tagMEMORY_BLOCK_HEADER
{
	struct _tagMEMORY_BLOCK_HEADER	*pNext;
	size_t							DataOffset;
	size_t							Size;
#if defined ( SCREAM_BUILD_DEBUG )
	char							Name[ 64 ];
	Uint32							CRC;
#endif /* SCREAM_BUILD_DEBUG */
	Uint16							Flags;
}MEMORY_BLOCK_HEADER, *PMEMORY_BLOCK_HEADER;

typedef struct _tagMEMORY_BLOCK_FOOTER
{
	Uint8	Magic;
	Uint8	Padding;
}MEMORY_BLOCK_FOOTER, *PMEMORY_BLOCK_FOOTER;

/* Memory blocks are limited to 256-byte boundaries */
typedef struct _tagMEMORY_BLOCK
{
	PMEMORY_BLOCK_HEADER	pFirstBlock;
	size_t					AllocatedSize;
	size_t					PaddedHeaderSize;
	void 					*pAllocatedBlock;
	Uint8					Alignment;
	Uint8					StructAlignment;
}MEMORY_BLOCK, *PMEMORY_BLOCK;

typedef struct _tagNATIVE_MEMORY_FREESTAT
{
	Uint32	Free;
	Uint32	LargestFree;
}NATIVE_MEMORY_FREESTAT, *PNATIVE_MEMORY_FREESTAT;

Sint32 MEM_Initialise( PNATIVE_MEMORY_FREESTAT p_pNativeMemoryFreeStat );
void MEM_Terminate( void );

Sint32 MEM_InitialiseMemoryBlock( PMEMORY_BLOCK p_pMemoryBlock,
	void *p_pMemoryPointer, size_t p_Size, Uint8 p_Alignment,
	const char *p_pName );
bool MEM_CreateMemoryBlockHeader( PMEMORY_BLOCK_HEADER p_pMemoryBlockHeader,
	bool p_Free, size_t p_TotalSize, size_t p_DataSize );
size_t MEM_CalculateMemoryBlockHeaderDataOffset(
	PMEMORY_BLOCK_HEADER p_pMemoryBlockHeader, Uint8 p_Alignment );
PMEMORY_BLOCK_HEADER MEM_GetFreeMemoryBlock( PMEMORY_BLOCK p_pMemoryBlock,
	size_t p_Size );
size_t MEM_GetMemoryBlockSize( PMEMORY_BLOCK_HEADER p_pMemoryBlockHeader,
	size_t p_Size, Uint8 p_Alignment, Uint8 p_StructAlign );
void *MEM_GetPointerFromMemoryBlockHeader(
	PMEMORY_BLOCK_HEADER p_pMemoryBlockHeader );
PMEMORY_BLOCK_HEADER MEM_GetMemoryBlockHeaderFromPointer( void *p_pPointer );
void MEM_GarbageCollectMemoryBlock( PMEMORY_BLOCK p_pMemoryBlock );

void *MEM_AllocateFromMemoryBlock( PMEMORY_BLOCK p_pMemoryBlock, size_t p_Size,
	const char *p_pName );
void MEM_FreeFromMemoryBlock( PMEMORY_BLOCK p_pMemoryBlock, void *p_pPointer );
void *MEM_ReallocateFromMemoryBlock( PMEMORY_BLOCK p_pMemoryBlock,
	size_t p_NewSize, void *p_pOriginalPointer );

size_t MEM_GetFreeMemoryBlockSize( PMEMORY_BLOCK p_pMemoryBlock );
size_t MEM_GetUsedMemoryBlockSize( PMEMORY_BLOCK p_pMemoryBlock );

void MEM_ListMemoryBlocks( PMEMORY_BLOCK p_pMemoryBlock );

#endif /* __SCREAM_MEMORY_H__ */

