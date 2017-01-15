#ifndef __SCREAM_HARDWARE_H__
#define __SCREAM_HARDWARE_H__

#include <Memory.h>
#include <kamui2.h>
#include <shinobi.h>

#define HW_OK 0
#define HW_ERROR 1
#define HW_FATALERROR -1

#define KM_DITHER TRUE
#define KM_AAMODE FALSE

typedef struct _tagHW_CONSOLEID
{
	Sint8	ConsoleID[ SYD_CFG_IID_SIZE ];
	char	ConsoleIDString[ ( SYD_CFG_IID_SIZE * 2 ) + 1 ];
}HW_CONSOLEID, *PHW_CONSOLEID;

Sint32 HW_Initialise( KMBPPMODE p_BPP,
	PNATIVE_MEMORY_FREESTAT p_pMemoryFreeStat );
void HW_Terminate( void );

void HW_Reboot( void );

Sint32 HW_GetConsoleID( PHW_CONSOLEID p_pConsoleID );

#endif /* __SCREAM_HARDWARE_H__ */

