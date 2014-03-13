#include "sys/sys_public.h"
#include "sys/sys_local.h"
#include "forward/cl_public.h"

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

void* Sys_LoadLibrary( const char *name )
{
	return ( void* )LoadLibrary( name );
}

void Sys_UnloadLibrary( void *handle )
{
	FreeLibrary( ( HMODULE )handle );
	if( !handle ) {
		return;
	}
	if( !FreeLibrary( ( struct HINSTANCE__ * )handle ) ) {
		Com_Error( ERR_FATAL, "Sys_UnloadDll FreeLibrary failed" );
	}
}

void* Sys_LoadFunction( void *handle, const char *name )
{
	return ( void* )GetProcAddress( ( HMODULE )handle, name );
}

const char* Sys_LibraryError()
{
	DWORD errorCode = GetLastError();
	static char error[ MAX_STRING_CHARS ];
	DWORD size = FormatMessage(
		FORMAT_MESSAGE_FROM_SYSTEM,					// It's a system error
		NULL,										// No string to be formatted needed
		errorCode,
		MAKELANGID( LANG_NEUTRAL, SUBLANG_DEFAULT ),// Do it in the standard language
		error,										// Put the message here
		sizeof( error ) - 1,						// Number of bytes to store the message
		NULL );
	error[ size ] = '\0'; // is this necessary?
	return error;
}
