#include "sys/sys_public.h"
#include "sys/sys_local.h"
#include "console/con_public.h"
#include "forward/cl_public.h"

#include <cstdlib> // exit
#include <cstdarg> // vararg
#include <vector>

void Sys_Error( const char *error, ... )
{
	va_list argptr;
	char string[ 4096 ];

	va_start( argptr, error );
	Q_vsnprintf( string, sizeof(string), error, argptr );
	va_end( argptr );
	
	Con_ShowError( string );
	
	Sys_Quit( 1 );
}

void Sys_Quit( int returnCode )
{
	Com_Shutdown();
	OS_Shutdown();
	exit( returnCode );
}

void* QDECL Sys_LoadDll( const char *name, qboolean useSystemLib )
{
	void *dllhandle = NULL;

	if( useSystemLib )
		Com_Printf( "Trying to load \"%s\"...\n", name );
	
	if( !useSystemLib || !( dllhandle = Sys_LoadLibrary( name ) ) )
	{
		const char *topDir;
		char libPath[ MAX_OSPATH ];
        
		topDir = Sys_BinaryPath();
        
		if( !*topDir )
			topDir = ".";
        
		Com_Printf( "Trying to load \"%s\" from \"%s\"...\n", name, topDir );
		Com_sprintf( libPath, sizeof( libPath ), "%s%c%s", topDir, PATH_SEP, name );
        
		if( !( dllhandle = Sys_LoadLibrary( libPath ) ) )
		{
			const char *basePath = Cvar_VariableString( "fs_basepath" );
			
			if( !basePath || !*basePath )
				basePath = ".";
			
			if( FS_FilenameCompare( topDir, basePath ) )
			{
				Com_Printf( "Trying to load \"%s\" from \"%s\"...\n", name, basePath );
				Com_sprintf( libPath, sizeof( libPath ), "%s%c%s", basePath, PATH_SEP, name );
				dllhandle = Sys_LoadLibrary( libPath );
			}
			
			if(!dllhandle)
			{
				Com_Printf("Loading \"%s\" failed\n", name);
			}
		}
	}
	
	return dllhandle;
}

void Sys_UnloadDll( void *dllHandle )
{
	Sys_UnloadLibrary( dllHandle );
}
