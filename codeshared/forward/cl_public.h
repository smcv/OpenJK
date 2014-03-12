/**
	\file
	
	Client-specific functions used by shared code.
	i.e. these functions differ in implementation between SP and MP.
	
	They are thus not implemented in the CommonLib & co, they'll be linked in when the libraries are linked into the client.
**/

#pragma once

#include "qcommon/q_platform.h"
#include "qcommon/q_sharedtypes.h"

// This is a common subset of the SP and MP tags that is safe to use in shared code.
typedef enum
{
	TAG_FILESYS = 3,
	TAG_EVENT,
	TAG_CLIPBOARD
} memtag_t;

/**
	\brief Print a formatted string to the console. Format like printf.
**/
void	QDECL Com_Printf( const char *format, ... );

int	QDECL Com_sprintf( char *dest, int size, const char *fmt, ... );

void Q_strcat( char *dest, int size, const char *src );

#ifdef __cplusplus
void	Q_strncpyz( char *dest, const char *src, int destsize, qboolean bBarfIfTooLong = qfalse );
#else
void	Q_strncpyz( char *dest, const char *src, int destsize, qboolean bBarfIfTooLong );
#endif

void Com_Init( char *commandLine );

void Com_Frame( void );

int Z_Free( void *pvAddress );

void *Z_Malloc( int iSize, memtag_t eTag, qboolean bZeroit = qfalse, int iUnusedAlign = 4 );

//void *Z_Malloc( int iSize, memtag_t eTag, qboolean bZeroit, int unusedAlign );


//    Console Commands

typedef void( *xcommand_t ) ( void );

void	Cmd_AddCommand( const char *cmd_name, xcommand_t function );

void	Cmd_RemoveCommand( const char *cmd_name );

int		Cmd_Argc( void );

char	*Cmd_Argv( int arg );


//    CVar

cvar_t *Cvar_Get( const char *var_name, const char *value, int flags );

void 	Cvar_Set( const char *var_name, const char *value );
