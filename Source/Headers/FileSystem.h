#ifndef __SCREAM_FILESYSTEM_H__
#define __SCREAM_FILESYSTEM_H__

#include <shinobi.h>
#include <Stack.h>
#include <Memory.h>
#include <Queue.h>

#define FS_OK 0
#define FS_ERROR 1
#define FS_FATALERROR -1
#define FS_DRIVEERROR -10

/* File work queue modes */
enum FS_WORK_QUEUE_MODE
{
	FS_WORK_QUEUE_MODE_IDLE = 0,
	FS_WORK_QUEUE_MODE_ACTIVE
};

typedef struct _tagFILE_CHUNK
{
	Uint32	ID;
	Uint32	Size;
}FILE_CHUNK, *PFILE_CHUNK;

typedef struct _tagFS_READ_REQUEST
{
	GDFS	Handle;
	Sint32	Size;
	void	*pBuffer;
}FS_READ_REQUEST, *PFS_READ_REQUEST;

typedef struct _tagFS_WORK_QUEUE
{
	Sint32			Mode;
	GDFS			CurrentFileHandle;
	QUEUE			Queue;
}FS_WORK_QUEUE, *PFS_WORK_QUEUE;

Sint32 FS_Initialise( void );
void FS_Terminate( void );

/* Read data from the queue of file handles, if necessary */
void FS_Process( void );

#endif /* __SCREAM_FILESYSTEM_H__ */

