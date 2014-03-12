#include "sys/sys_public.h"

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

void* Sys_LoadLibrary( const char *name )
{
	return ( void* )LoadLibrary( name );
}

void Sys_UnloadLibrary( void *handle )
{
	FreeLibrary( ( HMODULE )handle );
}

void* Sys_LoadFunction( void *handle, const char *name )
{
	return ( void* )GetProcAddress( ( HMODULE )handle, name );
}

const char* Sys_LibraryError()
{
	return "unknown";
}
