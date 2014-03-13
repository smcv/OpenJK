#include "sys/sys_local.h"
#include "sys/sys_public.h"

#include <pwd.h>
#include <cstdio>

void OS_Init( void )
{
}

void OS_Shutdown( int returnCode )
{
}

qboolean Sys_LowPhysicalMemory()
{
	static cvar_t* sys_lowmem = Cvar_Get( "sys_lowmem", "0", 0 );
	return ToQBoolean( sys_lowmem->integer );
}

void Sys_Init() // common systems are up
{
	Cvar_Set( "arch", OS_STRING " " ARCH_STRING );
	Cvar_Set( "username", Sys_GetCurrentUser( ) );
}

const char *Sys_GetCurrentUser( void )
{
	struct passwd *p;

	if ( (p = getpwuid( getuid() )) == NULL ) {
		return "player";
	}
	return p->pw_name[0] ? p->pw_name : "player";
}

qboolean Sys_RandomBytes( byte *string, int len )
{
	FILE *fp;

	fp = fopen( "/dev/urandom", "r" );
	if( !fp )
		return qfalse;

	if( !fread( string, sizeof( byte ), len, fp ) )
	{
		fclose( fp );
		return qfalse;
	}

	fclose( fp );
	return qtrue;
}
