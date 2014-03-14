/**
	\file
	Windows-specific path-related functions.
**/
#include "sys/sys_public.h"
#include "sys/sys_local.h"
#include "forward/cl_public.h"

#include <Windows.h>
#include <direct.h> // _getcwd() 	
#include <ShlObj.h> // CSIDL_PERSONAL
#include <io.h> // _finddata_t

extern cvar_t *com_homepath;
extern cvar_t *com_developer;
// Used to determine where to store user-specific files
static char homePath[ MAX_OSPATH ] = { 0 };

qboolean Sys_Mkdir( const char *path )
{
	if( !CreateDirectory( path, NULL ) )
	{
		// Already existed is not an error we care about.
		if( GetLastError( ) != ERROR_ALREADY_EXISTS )
			return qfalse;
	}
	return qtrue;
}

const char *Sys_Cwd( void )
{
	static char cwd[ MAX_OSPATH ];

	_getcwd( cwd, sizeof( cwd )-1 );
	cwd[ MAX_OSPATH - 1 ] = 0;

	return cwd;
}

const char *Sys_DefaultInstallPath( void )
{
	return Sys_BinaryPath( );
}

const char	*Sys_DefaultHomePath( const char *subDir ) {
#ifdef _PORTABLE_VERSION
	Com_Printf( "Portable install requested, skipping homepath support\n" );
	return NULL;
#else
	typedef HRESULT( __stdcall * GETFOLDERPATH )( HWND, int, HANDLE, DWORD, LPSTR );

	TCHAR szPath[ MAX_PATH ];
	GETFOLDERPATH qSHGetFolderPath;
	HMODULE shfolder = LoadLibrary( "shfolder.dll" );

	if( shfolder == NULL )
	{
		Com_Printf( "Unable to load SHFolder.dll\n" );
		return NULL;
	}

	if( !*homePath && com_homepath )
	{
		qSHGetFolderPath = ( GETFOLDERPATH )GetProcAddress( shfolder, "SHGetFolderPathA" );
		if( qSHGetFolderPath == NULL )
		{
			Com_Printf( "Unable to find SHGetFolderPath in SHFolder.dll\n" );
			FreeLibrary( shfolder );
			return NULL;
		}

		if( !SUCCEEDED( qSHGetFolderPath( NULL, CSIDL_PERSONAL,
			NULL, 0, szPath ) ) )
		{
			Com_Printf( "Unable to detect CSIDL_PERSONAL\n" );
			FreeLibrary( shfolder );
			return NULL;
		}

		Com_sprintf( homePath, sizeof( homePath ), "%s%cMy Games%c", szPath, PATH_SEP, PATH_SEP );

		if( com_homepath->string[ 0 ] )
			Q_strcat( homePath, sizeof( homePath ), com_homepath->string );
		else
			Q_strcat( homePath, sizeof( homePath ), subDir );
	}

	FreeLibrary( shfolder );
	return homePath;
#endif
}

const char *Sys_Basename( char *path )
{
	static char base[ MAX_OSPATH ] = { 0 };
	int length;

	length = strlen( path ) - 1;

	// Skip trailing slashes
	while( length > 0 && path[ length ] == '\\' )
		length--;

	while( length > 0 && path[ length - 1 ] != '\\' )
		length--;

	Q_strncpyz( base, &path[ length ], sizeof( base ) );

	length = strlen( base ) - 1;

	// Strip trailing slashes
	while( length > 0 && base[ length ] == '\\' )
		base[ length-- ] = '\0';

	return base;
}

const char *Sys_Dirname( const char *path )
{
	static char dir[ MAX_OSPATH ] = { 0 };
	int length;

	Q_strncpyz( dir, path, sizeof( dir ) );
	length = strlen( dir ) - 1;

	while( length > 0 && dir[ length ] != '\\' )
		length--;

	if( length == 0 )
		return ".";

	dir[ length ] = '\0';

	return dir;
}

qboolean Sys_PathCmp( const char *path1, const char *path2 ) {
	char *r1, *r2;

	r1 = _fullpath( NULL, path1, MAX_OSPATH );
	r2 = _fullpath( NULL, path2, MAX_OSPATH );

	if( r1 && r2 && !Q_stricmp( r1, r2 ) )
	{
		free( r1 );
		free( r2 );
		return qtrue;
	}

	free( r1 );
	free( r2 );
	return qfalse;
}

#define	MAX_FOUND_FILES	0x1000

static void Sys_ListFilteredFiles( const char *basedir, const char *subdirs, const char *filter, char **psList, int *numfiles ) {
	char		search[ MAX_OSPATH ], newsubdirs[ MAX_OSPATH ];
	char		filename[ MAX_OSPATH ];
	intptr_t	findhandle;
	struct _finddata_t findinfo;

	if( *numfiles >= MAX_FOUND_FILES - 1 ) {
		return;
	}

	if( strlen( subdirs ) ) {
		Com_sprintf( search, sizeof( search ), "%s\\%s\\*", basedir, subdirs );
	}
	else {
		Com_sprintf( search, sizeof( search ), "%s\\*", basedir );
	}

	findhandle = _findfirst( search, &findinfo );
	if( findhandle == -1 ) {
		return;
	}

	do {
		if( findinfo.attrib & _A_SUBDIR ) {
			if( Q_stricmp( findinfo.name, "." ) && Q_stricmp( findinfo.name, ".." ) ) {
				if( strlen( subdirs ) ) {
					Com_sprintf( newsubdirs, sizeof( newsubdirs ), "%s\\%s", subdirs, findinfo.name );
				}
				else {
					Com_sprintf( newsubdirs, sizeof( newsubdirs ), "%s", findinfo.name );
				}
				Sys_ListFilteredFiles( basedir, newsubdirs, filter, psList, numfiles );
			}
		}
		if( *numfiles >= MAX_FOUND_FILES - 1 ) {
			break;
		}
		Com_sprintf( filename, sizeof( filename ), "%s\\%s", subdirs, findinfo.name );
		if( !Com_FilterPath( filter, filename, qfalse ) )
			continue;
		psList[ *numfiles ] = CopyString( filename );
		( *numfiles )++;
	} while( _findnext( findhandle, &findinfo ) != -1 );

	_findclose( findhandle );
}

static qboolean strgtr( const char *s0, const char *s1 ) {
	int l0, l1, i;

	l0 = strlen( s0 );
	l1 = strlen( s1 );

	if( l1<l0 ) {
		l0 = l1;
	}

	for( i = 0; i < l0; i++ ) {
		if( s1[ i ] > s0[ i ] ) {
			return qtrue;
		}
		if( s1[ i ] < s0[ i ] ) {
			return qfalse;
		}
	}
	return qfalse;
}

static void SortFileList( char **listCopy, int nfiles )
{
	int flag;
	do {
		flag = 0;
		for( int i = 1; i < nfiles; i++ ) {
			if( strgtr( listCopy[ i - 1 ], listCopy[ i ] ) ) {
				char *temp = listCopy[ i ];
				listCopy[ i ] = listCopy[ i - 1 ];
				listCopy[ i - 1 ] = temp;
				flag = 1;
			}
		}
	} while( flag );
}

char **Sys_ListFiles( const char *directory, const char *extension, const char *filter, int *numfiles, qboolean wantsubs ) {
	char		search[ MAX_OSPATH ];
	int			nfiles;
	char		**listCopy;
	char		*list[ MAX_FOUND_FILES ];
	struct _finddata_t findinfo;
	intptr_t	findhandle;
	int			flag;
	int			i;

	if( filter ) {

		nfiles = 0;
		Sys_ListFilteredFiles( directory, "", filter, list, &nfiles );

		list[ nfiles ] = 0;
		*numfiles = nfiles;

		if( !nfiles )
			return NULL;

		listCopy = ( char ** )Z_Malloc( ( nfiles + 1 ) * sizeof( *listCopy ), TAG_FILESYS );
		for( i = 0; i < nfiles; i++ ) {
			listCopy[ i ] = list[ i ];
		}
		listCopy[ i ] = NULL;

		return listCopy;
	}

	if( !extension ) {
		extension = "";
	}

	// passing a slash as extension will find directories
	if( extension[ 0 ] == '/' && extension[ 1 ] == 0 ) {
		extension = "";
		flag = 0;
	}
	else {
		flag = _A_SUBDIR;
	}

	Com_sprintf( search, sizeof( search ), "%s\\*%s", directory, extension );

	// search
	nfiles = 0;

	findhandle = _findfirst( search, &findinfo );
	if( findhandle == -1 ) {
		*numfiles = 0;
		return NULL;
	}

	do {
		if( ( !wantsubs && flag ^ ( findinfo.attrib & _A_SUBDIR ) ) || ( wantsubs && findinfo.attrib & _A_SUBDIR ) ) {
			if( nfiles == MAX_FOUND_FILES - 1 ) {
				break;
			}
			list[ nfiles ] = CopyString( findinfo.name );
			nfiles++;
		}
	} while( _findnext( findhandle, &findinfo ) != -1 );

	list[ nfiles ] = 0;

	_findclose( findhandle );

	// return a copy of the list
	*numfiles = nfiles;

	if( !nfiles ) {
		return NULL;
	}

	listCopy = ( char ** )Z_Malloc( ( nfiles + 1 ) * sizeof( *listCopy ), TAG_FILESYS );
	for( i = 0; i < nfiles; i++ ) {
		listCopy[ i ] = list[ i ];
	}
	listCopy[ i ] = NULL;

	SortFileList( listCopy, nfiles );

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


static qboolean Sys_GetFileTime( LPCSTR psFileName, FILETIME &ft )
{
	qboolean bSuccess = qfalse;
	HANDLE hFile = INVALID_HANDLE_VALUE;

	hFile = CreateFile( psFileName,	// LPCTSTR lpFileName,          // pointer to name of the file
		GENERIC_READ,			// DWORD dwDesiredAccess,       // access (read-write) mode
		FILE_SHARE_READ,		// DWORD dwShareMode,           // share mode
		NULL,					// LPSECURITY_ATTRIBUTES lpSecurityAttributes,	// pointer to security attributes
		OPEN_EXISTING,			// DWORD dwCreationDisposition,  // how to create
		FILE_FLAG_NO_BUFFERING,// DWORD dwFlagsAndAttributes,   // file attributes
		NULL					// HANDLE hTemplateFile          // handle to file with attributes to 
		);

	if( hFile != INVALID_HANDLE_VALUE )
	{
		if( GetFileTime( hFile,	// handle to file
			NULL,	// LPFILETIME lpCreationTime
			NULL,	// LPFILETIME lpLastAccessTime
			&ft		// LPFILETIME lpLastWriteTime
			)
			)
		{
			bSuccess = qtrue;
		}

		CloseHandle( hFile );
	}

	return bSuccess;
}


qboolean Sys_FileOutOfDate( const char *dest, const char * src )
{
	FILETIME ftFinalFile, ftDataFile;

	if( Sys_GetFileTime( dest, ftFinalFile ) && Sys_GetFileTime( src, ftDataFile ) )
	{
		// timer res only accurate to within 2 seconds on FAT, so can't do exact compare...
		//
		//LONG l = CompareFileTime( &ftFinalFile, &ftDataFile );
		if( ( abs( long( ftFinalFile.dwLowDateTime - ftDataFile.dwLowDateTime ) ) <= 20000000 ) &&
			ftFinalFile.dwHighDateTime == ftDataFile.dwHighDateTime
			)
		{
			return qfalse;	// file not out of date, ie use it.
		}
		return qtrue;	// flag return code to copy over a replacement version of this file
	}


	// extra error check, report as suspicious if you find a file locally but not out on the net.,.
	//
	if( com_developer->integer )
	{
		if( !Sys_GetFileTime( dest, ftDataFile ) )
		{
			Com_Printf( "Sys_FileOutOfDate: reading %s but it's not on the net!\n", src );
		}
	}

	return qfalse;
}

qboolean Sys_CopyFile( const char *lpExistingFileName, const char *lpNewFileName, qboolean bOverWrite )
{
	qboolean bOk = qtrue;
	if( !CopyFile( lpExistingFileName, lpNewFileName, !bOverWrite ) && bOverWrite )
	{
		DWORD dwAttrs = GetFileAttributes( lpNewFileName );
		SetFileAttributes( lpNewFileName, dwAttrs & ~FILE_ATTRIBUTE_READONLY );
		bOk = ToQBoolean( CopyFile( lpExistingFileName, lpNewFileName, FALSE ) );
	}
	return bOk;
}
