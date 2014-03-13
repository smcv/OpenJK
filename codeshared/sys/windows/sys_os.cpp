#include "sys/sys_local.h"
#include "sys/sys_public.h"

#include "forward/cl_public.h"

#include <Windows.h>
#include <crtdbg.h>

// The following macros set and clear, respectively, given bits
// of the C runtime library debug flag, as specified by a bitmask.

#ifdef   _DEBUG
#define  SET_CRT_DEBUG_FIELD(a) \
	_CrtSetDbgFlag( ( a ) | _CrtSetDbgFlag( _CRTDBG_REPORT_FLAG ) )
#define  CLEAR_CRT_DEBUG_FIELD(a) \
	_CrtSetDbgFlag( ~( a )& _CrtSetDbgFlag( _CRTDBG_REPORT_FLAG ) )
#else
#define  SET_CRT_DEBUG_FIELD(a)   ((void) 0)
#define  CLEAR_CRT_DEBUG_FIELD(a) ((void) 0)
#endif

#define MEM_THRESHOLD 128*1024*1024

// pre-common system init
void OS_Init( void )
{
	// Perform automatic leak checking at program exit through a call to _CrtDumpMemoryLeaks and generate an error report if the application failed to free all the memory it allocated
	SET_CRT_DEBUG_FIELD( _CRTDBG_LEAK_CHECK_DF );
	//_CrtSetBreakAlloc(34804);

	// no abort/retry/fail errors
	SetErrorMode( SEM_FAILCRITICALERRORS );

	// make sure the timer is high precision, otherwise
	// NT gets 18ms resolution
	timeBeginPeriod( 1 );
}

void OS_Shutdown( void )
{
	timeEndPeriod( 1 );
}

qboolean Sys_LowPhysicalMemory( )
{
	static MEMORYSTATUSEX stat;
	static qboolean bAsked = qfalse;
	static cvar_t* sys_lowmem = Cvar_Get( "sys_lowmem", "0", 0 );

	if( !bAsked )	// just in case it takes a little time for GlobalMemoryStatus() to gather stats on
	{				//	stuff we don't care about such as virtual mem etc.
		bAsked = qtrue;
		GlobalMemoryStatusEx( &stat );
	}
	if( sys_lowmem->integer )
	{
		return qtrue;
	}
	return ( stat.ullTotalPhys <= MEM_THRESHOLD ) ? qtrue : qfalse;
}

// post-common system init
void Sys_Init( void )
{
	OSVERSIONINFO	osversion;
	osversion.dwOSVersionInfoSize = sizeof( osversion );

	if( !GetVersionEx( &osversion ) )
		Sys_Error( "Couldn't get OS info" );

	if( osversion.dwMajorVersion < 4 )
		Sys_Error( "This game requires Windows version 4 or greater" );
	if( osversion.dwPlatformId == VER_PLATFORM_WIN32s )
		Sys_Error( "This game doesn't run on Win32s" );

	Cvar_Set( "arch", OS_STRING " " ARCH_STRING );

	Cvar_Set( "username", Sys_GetCurrentUser( ) );
}

const char *Sys_GetCurrentUser( void )
{
	static char s_userName[ 1024 ];
	unsigned long size = sizeof( s_userName );


	if( !GetUserName( s_userName, &size ) )
		return "player";

	if( !s_userName[ 0 ] )
	{
		return "player";
	}

	return s_userName;
}

qboolean Sys_RandomBytes( byte *string, int len )
{
	HCRYPTPROV  prov;

	if( !CryptAcquireContext( &prov, NULL, NULL,
		PROV_RSA_FULL, CRYPT_VERIFYCONTEXT ) )  {

		return qfalse;
	}

	if( !CryptGenRandom( prov, len, ( BYTE * )string ) )  {
		CryptReleaseContext( prov, 0 );
		return qfalse;
	}
	CryptReleaseContext( prov, 0 );
	return qtrue;
}
