/**
	\file
	Windows-specific path-related functions.
**/
#include "sys/sys_public.h"
#include "sys/sys_local.h"
#include "sys/windows/windows_local.h"
#include "forward/cl_public.h"

#include <direct.h> // _getcwd() 	
#include <ShlObj.h> // CSIDL_PERSONAL

extern cvar_t *com_homepath;
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
