#ifndef __SCREAM_LOG_H__
#define __SCREAM_LOG_H__

#include <sg_xpt.h>

#define LOG_OK 0
#define LOG_ERROR 1
#define LOG_FATALERROR -1

#if defined ( SCREAM_BUILD_DEBUG ) || defined ( SCREAM_BUILD_DEVELOPMENT )
#define LOG_Initialise LOG_Initialise_Internal
#define LOG_Terminate LOG_Terminate_Internal
#define LOG_Debug LOG_Debug_Internal
#else
#define LOG_Initialise sizeof
#define LOG_Terminate( )
#define LOG_Debug sizeof
#endif /* SCREAM_BUILD_DEBUG || SCREAM_BUILD_DEVELOPMENT */

Sint32 LOG_Initialise_Internal( const char *p_pLogFileName );
void LOG_Terminate_Internal( void );
void LOG_Debug_Internal( const char *p_pMessage, ... );

#endif /* __SCREAM_LOG_H__ */

