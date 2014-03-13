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


/**
	\brief System initialization to be done once vvars are available

	Unlike OS_Init(), which is done before any game code is called, this is called after the common systems (cvars, files, etc) are initialized
**/
void Sys_Init( void );

/**
	\brief Exits the programs.
	
	This is the sole exit point, don't call exit() anywhere else!

	Calls Com_Shutdown() for clean shutdown.
**/
void Sys_Quit( int returnCode ) __attribute__( ( noreturn ) );

/**
	\brief Displays a formatted error and quits.

	Format as in printf.

	May display an error window and wait for the user to accept depending on the implementation. Should thus not be called before Con_CreateConsole().
**/
void QDECL Sys_Error( const char *error, ... ) __attribute__( ( noreturn ) );

/**
\brief Returns the current user's name.
**/
const char *Sys_GetCurrentUser( void );

/**
	\brief Snaps the components of a vector to the nearest whole number.
**/
#ifdef __cplusplus
extern "C"
#endif
void Sys_SnapVector( float *v );

/**
	\brief Generates random bytes
	\return Success
**/
qboolean Sys_RandomBytes( byte *string, int len );

/**
	\brief Whether the system has little memory.

	On Windows, low is 128MB and below. On Unix, there is no such thing as little memory as far as we're concerned.
**/
qboolean Sys_LowPhysicalMemory();

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

/**
	\brief Retrieves the name of the directory/file in the given path

	/foo/bar/ will yield bar, /foo/bar/blub.txt yields blub.txt

	\note Returns a static variable that /will/ change on subsequent calls!
**/
const char *Sys_Basename( char *path );


/**
	\brief Resolves path names and determines if they are the same

	For use with full OS paths not quake paths

	\return qtrue if resulting paths are valid and the same, otherwise qfalse
**/
qboolean Sys_PathCmp( const char *path1, const char *path2 );
/**
	\brief Lists the files in a given directory.
	\param directory Directory to enumerate.
	\param extension Only list files with the given extension. "/" lists directories.
	\param filter Only results matching this filter will be returned. Overrules wantsubs.
	\param numFiles Number of files returned is written here.
	\param wantsubs Whether to include subdirectories in the results.
	\return Null-terminated dynamically allocated array of files, to be freed via Sys_FreeFileList().
**/
char **Sys_ListFiles( const char *directory, const char *extension, char *filter, int *numfiles, qboolean wantsubs );
/**
	\brief Frees a file list received from Sys_ListFiles().
**/
void Sys_FreeFileList( char **filelist );


//    Shared Library handling

/**
	\brief General development dll loading for virtual machine testing.

	Used mainly for loading the renderer.
**/
void* QDECL Sys_LoadDll( const char *name, qboolean useSystemLib );

/**
Searches for a DLL at the various places it could be.

\return Opened handle if found, NULL otherwise.
**/
void *Sys_FindDLL( const char *game, const char *name );

/**
	\brief Closes a loaded dll/so.
**/
void Sys_UnloadDll( void *dllHandle );

/**
	\brief Unloading a library
**/
void Sys_UnloadLibrary( void *handle );

/**
	\brief Retrieves a function from a loaded dll/so.
**/
void* Sys_LoadFunction( void *handle, const char *name );

/**
	\brief Retrieves the reason for the last dll/so-related failure.
**/
const char* Sys_LibraryError();

/**
	\brief Main function.
**/
int ojk_main( int argc, char **argv );

// end of Doxygen Group
/// \}
