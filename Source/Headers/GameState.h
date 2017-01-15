#ifndef __SCREAM_GAMESTATE_H__
#define __SCREAM_GAMESTATE_H__

#include <sg_xpt.h>

/* Game state functions can accept a parameter of any type.  Use a struct for
 * multiple parameters */
typedef Sint32 ( *GS_Function )( void * );

typedef struct _tagGAMESTATE
{
	GS_Function	Load;
	GS_Function Initialise;
	GS_Function Update;
	GS_Function Render;
	GS_Function Terminate;
	GS_Function Unload;
	bool		Paused;
	Uint32		ElapsedGameTime;
}GAMESTATE, *PGAMESTATE;

void GS_Copy( PGAMESTATE p_pCopy, PGAMESTATE p_pOriginal );

void GS_Pause( PGAMESTATE p_pGameState );
void GS_Resume( PGAMESTATE p_pGameState );

#endif /* __SCREAM_GAMESTATE_H__ */

