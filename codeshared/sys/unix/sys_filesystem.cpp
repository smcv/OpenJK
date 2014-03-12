/**
	\file
	Unix filesystem-related functions
**/

/*
#include <dirent.h>
#include <stdarg.h>
#include <stdlib.h>
#include <sys/time.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <pwd.h>
#include <libgen.h>
*/

#include "sys/sys_local.h"
#include "sys/sys_public.h"
#include "sys/unix/unix_local.h"

const char *Sys_Dirname( const char *path )
{
	return dirname( path );
}

/**
	 Discovers if passed dir is suffixed with the directory structure of a Mac OS X
	.app bundle. If it is, the .app directory structure is stripped off the end and
	the result is returned. If not, dir is returned untouched.
**/
static const char *Sys_StripAppBundle( char *dir )
{
	static char cwd[MAX_OSPATH];

	Q_strncpyz(cwd, dir, sizeof(cwd));
	if(strcmp(Sys_Basename(cwd), "MacOS"))
		return dir;
	Q_strncpyz(cwd, Sys_Dirname(cwd), sizeof(cwd));
	if(strcmp(Sys_Basename(cwd), "Contents"))
		return dir;
	Q_strncpyz(cwd, Sys_Dirname(cwd), sizeof(cwd));
	if(!strstr(Sys_Basename(cwd), ".app"))
		return dir;
	Q_strncpyz(cwd, Sys_Dirname(cwd), sizeof(cwd));
	return cwd;
}

const char *Sys_DefaultInstallPath( void )
{
#	ifdef MACOS_X
		return Sys_StripAppBundle( Sys_BinaryPath() );
#	else
		return Sys_BinaryPath();
#	endif
}