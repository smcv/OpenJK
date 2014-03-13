#include "sys/sys_public.h"
#include "sys/sys_local.h"
#include <dlfcn.h>

void* Sys_LoadLibrary( const char *name )
{
	return dlopen( name, RTLD_NOW );
}

void Sys_UnloadLibrary( void *handle )
{
	dlclose( handle );
}

void* Sys_LoadFunction( void *handle, const char *name )
{
	return dlsym( handle, name );
}

const char* Sys_LibraryError()
{
	return dlerror();
}
