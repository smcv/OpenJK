/**
	\file
	Unix filesystem-related functions
**/

#include "sys/sys_local.h"
#include "sys/sys_public.h"
#include "sys/unix/unix_local.h"
#include "forward/cl_public.h"

#include <sys/stat.h> // mkdir
#include <unistd.h> // getcwd
#include <libgen.h> // basename, dirname
#include <dirent.h> // opendir, readdir, closedir
#include <errno.h> // errno etc.
#include <cstring> // strcmp etc.
#include <cstdlib> // getenv

qboolean Sys_Mkdir( const char *path )
{
	int result = mkdir( path, 0750 );

	if( result != 0 )
		return ToQBoolean(errno == EEXIST);

	return qtrue;
}

const char *Sys_Cwd( void )
{
	static char cwd[MAX_OSPATH];

	getcwd( cwd, sizeof( cwd ) - 1 );
	cwd[MAX_OSPATH-1] = 0;

	return cwd;
}

#	ifdef MACOS_X
/**
	 Discovers if passed dir is suffixed with the directory structure of a Mac OS X
	.app bundle. If it is, the .app directory structure is stripped off the end and
	the result is returned. If not, dir is returned untouched.
**/
static const char *Sys_StripAppBundle( const char *dir )
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
#endif

const char *Sys_DefaultInstallPath( void )
{
#	ifdef MACOS_X
		return Sys_StripAppBundle( Sys_BinaryPath() );
#	else
		return Sys_BinaryPath();
#	endif
}

const char *Sys_DefaultHomePath( const char *subDir )
{
#ifdef _PORTABLE_VERSION
	Com_Printf( "Portable install requested, skipping homepath support\n" );
	return NULL;
#else
	static char homePath[ MAX_OSPATH ] = { 0 };
	const char *path = NULL;

#ifndef MACOS_X
	if( !path )
	{
		path = getenv( "XDG_DATA_HOME" );
	}
#endif
	if( !path )
	{
		path = getenv( "HOME" );
	}
	
	if( path )
	{
		Com_sprintf(homePath, sizeof(homePath), "%s%c%s", path, PATH_SEP, subDir );
		return homePath;
	}
	else
	{
		return "";
	}
}
#endif

const char *Sys_Basename( const char *path )
{
	return basename( ( char* )path );
}

const char *Sys_Dirname( const char *path )
{
	return dirname( ( char* )path );
}

#define MAX_FOUND_FILES 0x1000

static void Sys_ListFilteredFiles( const char *basedir, const char *subdirs, const char *filter, char **psList, int *numfiles )
{
	char		search[MAX_OSPATH], newsubdirs[MAX_OSPATH];
	char		filename[MAX_OSPATH];
	DIR			*fdir;
	struct dirent *d;
	struct stat st;

	if( *numfiles >= MAX_FOUND_FILES - 1 ) {
		return;
	}

	if( strlen( subdirs ) ) {
		Com_sprintf( search, sizeof( search ), "%s/%s", basedir, subdirs );
	}
	else {
		Com_sprintf( search, sizeof( search ), "%s", basedir );
	}

	if ((fdir = opendir(search)) == NULL) {
		return;
	}

	while ((d = readdir(fdir)) != NULL) {
		Com_sprintf(filename, sizeof(filename), "%s/%s", search, d->d_name);
		if (stat(filename, &st) == -1)
			continue;

		if (st.st_mode & S_IFDIR) {
			if (Q_stricmp(d->d_name, ".") && Q_stricmp(d->d_name, "..")) {
				if (strlen(subdirs)) {
					Com_sprintf( newsubdirs, sizeof(newsubdirs), "%s/%s", subdirs, d->d_name);
				}
				else {
					Com_sprintf( newsubdirs, sizeof(newsubdirs), "%s", d->d_name);
				}
				Sys_ListFilteredFiles( basedir, newsubdirs, filter, psList, numfiles );
			}
		}
		if ( *numfiles >= MAX_FOUND_FILES - 1 ) {
			break;
		}
		Com_sprintf( filename, sizeof(filename), "%s/%s", subdirs, d->d_name );
		if (!Com_FilterPath( filter, filename, qfalse ))
			continue;
		psList[ *numfiles ] = CopyString( filename );
		(*numfiles)++;
	}

	closedir(fdir);
}

char **Sys_ListFiles( const char *directory, const char *extension, const char *filter, int *numfiles, qboolean wantsubs )
{
	char		*list[ MAX_FOUND_FILES ];
	int			nfiles;

	if( filter ) {
		nfiles = 0;
		Sys_ListFilteredFiles( directory, "", filter, list, &nfiles );
	}
	else {
		char		search[ MAX_OSPATH ];
		struct dirent *d;
		DIR		*fdir;
		qboolean dironly = wantsubs;
		struct stat st;
		
		if( !extension ) {
			extension = "";
		}

		// passing a slash as extension will find directories
		if( extension[ 0 ] == '/' && extension[ 1 ] == 0 ) {
			extension = "";
			dironly = qtrue;
		}

		size_t extLen = strlen( extension );

		// search
		nfiles = 0;

		if ((fdir = opendir(directory)) == NULL) {
			*numfiles = 0;
			return NULL;
		}

		while ((d = readdir(fdir)) != NULL) {
			Com_sprintf(search, sizeof(search), "%s/%s", directory, d->d_name);
			if (stat(search, &st) == -1)
				continue;
			if ((dironly && !(st.st_mode & S_IFDIR)) ||
				(!dironly && (st.st_mode & S_IFDIR)))
				continue;

			if (*extension) {
				if ( strlen( d->d_name ) < extLen ||
					Q_stricmp(
						d->d_name + strlen( d->d_name ) - extLen,
						extension ) ) {
					continue; // didn't match
				}
			}

			if ( nfiles == MAX_FOUND_FILES - 1 )
				break;
			list[ nfiles ] = CopyString( d->d_name );
			nfiles++;
		}

		closedir(fdir);
	}

	list[ nfiles ] = 0;

	// return a copy of the list
	*numfiles = nfiles;

	if( !nfiles ) {
		return NULL;
	}

	
	char **listCopy = ( char ** )Z_Malloc( ( nfiles + 1 ) * sizeof( *listCopy ), TAG_FILESYS );
	for( int i = 0; i < nfiles; i++ ) {
		listCopy[ i ] = list[ i ];
	}
	listCopy[ nfiles ] = NULL;

	return listCopy;
}

void Sys_FreeFileList( char **psList ) {
	int i;

	if( !psList ) {
		return;
	}

	for( i = 0; psList[ i ]; i++ ) {
		Z_Free( psList[ i ] );
	}

	Z_Free( psList );
}
