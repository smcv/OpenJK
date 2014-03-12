/**
	\ingroup common
	\{
	
	\file
	
	Shared System API.
	
	This file provides access to System-specific functionality to the Clients (SP & MP), which use the SharedLib.
**/

#pragma once

#include "../qcommon/q_sharedtypes.h"
#include "../qcommon/q_platform.h"


//    General System functionality

void Sys_Init( void );

void Sys_Quit( void );

char *Sys_GetCurrentUser( void );

void QDECL Sys_Error( const char *error, ...) __attribute__((noreturn));

#ifdef __cplusplus
extern "C"
#endif
void Sys_SnapVector( float *v );

qboolean Sys_RandomBytes( byte *string, int len );

qboolean Sys_LowPhysicalMemory( );

/**
	\param baseTime When qtrue, returns milliseconds since some system-specific point in time.
	                When qfalse, returns milliseconds since first call to Sys_Milliseconds().
	\note Sys_Milliseconds should only be used for profiling purposes,
	      any game related timing information should come from event timestamps
**/
int		Sys_Milliseconds( qboolean baseTime );

//    Events

/**
	\brief Joystick axes
	\note To be removed in the future once they're no longer hardcoded.
**/
typedef enum {
	AXIS_SIDE,
	AXIS_FORWARD,
	AXIS_UP,
	AXIS_ROLL,
	AXIS_YAW,
	AXIS_PITCH,
	MAX_JOYSTICK_AXIS
} joystickAxis_t;


/**
	\brief System Event types
**/
typedef enum {
  // bk001129 - make sure SE_NONE is zero
	SE_NONE = 0,      ///< evTime is still valid
	SE_KEY,           ///< evValue is a key code, evValue2 is the down flag
	SE_CHAR,          ///< evValue is an ascii char
	SE_MOUSE,         ///< evValue and evValue2 are reletive signed x / y moves
	SE_JOYSTICK_AXIS, ///< evValue is an axis number and evValue2 is the current state (-127 to 127)
	SE_CONSOLE,       ///< evPtr is a char*
	SE_PACKET         ///< evPtr is a netadr_t followed by data bytes to evPtrLength
} sysEventType_t;

/**
	\brief System Event
**/
typedef struct sysEvent_s {
	int				evTime;
	sysEventType_t	evType;
	int				evValue;
	int				evValue2;
	int				evPtrLength;	// bytes of data pointed to by evPtr, for journaling
	void			*evPtr;			// this must be manually freed if not NULL
} sysEvent_t;

/**
	\brief Retrieves an event from the queue.
	
	\return The first event in the Queue, or an SE_NONE event if there are none. (Whose time will still be correct.)
	
	\see Sys_AddEventSource()
**/
sysEvent_t Sys_GetEvent( void );

/**
	Pushes an event to the event queue.
	
	A time of 0 will use the current time.
	ptr should be null or a block of data that can be freed with Z_Free().
**/
void Sys_QueEvent( int time, sysEventType_t type, int value, int value2, int ptrLength, void *ptr );


//    File System

/**
	\brief Tries creating a directory.
	\return Success. The directory already existing is treated as success.
**/
qboolean Sys_Mkdir( const char *path );

/**
	\brief Returns the current working directory.
**/
const char *Sys_Cwd( void );

/**
	\brief Install Path is where the Base/ folder with the original assets is.
**/
const char *Sys_DefaultInstallPath( void );

#ifdef MACOS_X
/**
	\brief App Path on OSX contains assets as well.
**/
const char *Sys_DefaultAppPath( void );
#endif

/**
	\brief Home Path is where files are written - config, safegames, screenshots and the like.
	\param homepathName name 
**/
const char *Sys_DefaultHomePath( const char *subDir );
const char *Sys_Basename( char *path );

qboolean Sys_PathCmp( const char *path1, const char *path2 );
char **Sys_ListFiles( const char *directory, const char *extension, char *filter, int *numfiles, qboolean wantsubs );
void Sys_FreeFileList( char **filelist );
qboolean Sys_FileOutOfDate( const char *psFinalFileName /* dest */, const char *psDataFileName /* src */ );
qboolean Sys_CopyFile(const char *lpExistingFileName, const char *lpNewFileName, qboolean bOverwrite);


//    Shared Library handling

// general development dll loading for virtual machine testing
void* QDECL Sys_LoadDll(const char *name, qboolean useSystemLib);
void* QDECL Sys_LoadLegacyGameDll( const char *name, intptr_t (QDECL **vmMain)(int, ...), intptr_t (QDECL *systemcalls)(intptr_t, ...) );
void* QDECL Sys_LoadGameDll( const char *name, void *(QDECL **moduleAPI)(int, ...) );
void Sys_UnloadDll( void *dllHandle );

void* Sys_LoadLibrary( const char *name );
void Sys_UnloadLibrary( void *handle );
void* Sys_LoadFunction( void *handle, const char *name );
const char* Sys_LibraryError();

// end of Doxygen Group
/// \}
