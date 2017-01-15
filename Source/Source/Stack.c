#include <Stack.h>
#include <Log.h>

Sint32 STK_Initialise( PSTACK p_pStack, PMEMORY_BLOCK p_pMemoryBlock,
	size_t p_Capacity, size_t p_ItemSize, size_t p_GrowableAmount,
	const char *p_pName )
{
	p_pStack->pStack = MEM_AllocateFromMemoryBlock( p_pMemoryBlock,
		p_Capacity * p_ItemSize, p_pName );

	if( p_pStack->pStack == NULL )
	{
		LOG_Debug( "STK_Initialise <ERROR> Failed to allocate a stack of %ld "
			"item of size %ld", p_Capacity, p_ItemSize );

		return STK_OUTOFMEMORY;
	}

	p_pStack->Top = 0;
	p_pStack->Capacity = p_Capacity * p_ItemSize;
	p_pStack->ItemSize = p_ItemSize;
	p_pStack->GrowableAmount = p_GrowableAmount;
	p_pStack->pMemoryBlock = p_pMemoryBlock;

	return STK_OK;
}

void STK_Terminate( PSTACK p_pStack )
{
	MEM_FreeFromMemoryBlock( p_pStack->pMemoryBlock, p_pStack->pStack );
	MEM_GarbageCollectMemoryBlock( p_pStack->pMemoryBlock );

	memset( p_pStack, 0, sizeof( STACK ) );
}

Sint32 STK_Push( PSTACK p_pStack, void *p_pItem )
{
	size_t StackTop = ( size_t )p_pStack->pStack + p_pStack->Top;

	if( p_pStack->Top == p_pStack->Capacity )
	{
		/* Overflow */
		LOG_Debug( "STK_Push <ERROR> Cannot grow stack, yet.  Failing" );

		return STK_OVERFLOW;
	}

	memcpy( ( void * )StackTop, p_pItem, p_pStack->ItemSize );

	p_pStack->Top += p_pStack->ItemSize;

	return STK_OK;
}

Sint32 STK_Pop( PSTACK p_pStack, void *p_pItem )
{
	size_t StackTop;

	if( p_pStack->Top == 0 )
	{
		/* Cannot get anything below zero! */
		LOG_Debug( "STK_Pop <ERROR> Unable to pop item off the stack" );

		return STK_UNDERFLOW;
	}

	/* top always points to the start of the next item in the stack, decrease
	 * the top by one item to get the correct position */
	StackTop = ( ( size_t )p_pStack->pStack + p_pStack->Top ) -
		p_pStack->ItemSize;
	
	if( p_pItem != NULL )
	{
		memcpy( p_pItem, ( void * )StackTop, p_pStack->ItemSize );
	}

	p_pStack->Top -= p_pStack->ItemSize;

	return STK_OK;
}

void *STK_GetTopItem( PSTACK p_pStack )
{
	size_t ItemPosition = 0;

	if( p_pStack->Top == 0 )
	{
		LOG_Debug( "STK_GetTopItem <WARN> No items left on the stack" );

		return NULL;
	}

	ItemPosition = ( ( size_t )p_pStack->pStack + p_pStack->Top ) -
		p_pStack->ItemSize;

	return ( void * )ItemPosition;
}

size_t STK_GetCount( PSTACK p_pStack )
{
#if defined ( SCREAM_BUILD_DEBUG )
	if( ( p_pStack->Top % p_pStack->ItemSize ) != 0 )
	{
		LOG_Debug( "STK_GetCount <WARN> Stack top may be misaligned" );
	}
#endif /* SCREAM_BUILD_DEBUG */

	return p_pStack->Top / p_pStack->ItemSize;
}

bool STK_IsFull( PSTACK p_pStack )
{
	if( p_pStack->Top == p_pStack->Capacity )
	{
		return true;
	}

	return false;
}

bool STK_IsEmpty( PSTACK p_pStack )
{
	if( p_pStack->Top == 0 )
	{
		return true;
	}

	return false;
}

