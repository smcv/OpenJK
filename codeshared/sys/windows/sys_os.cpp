#include "sys/sys_local.h"
#include "sys/sys_public.h"
#include "sys/windows/windows_local.h"

void OS_Init( void )
{
	SET_CRT_DEBUG_FIELD( _CRTDBG_LEAK_CHECK_DF );
	//_CrtSetBreakAlloc(34804);

	// no abort/retry/fail errors
	SetErrorMode( SEM_FAILCRITICALERRORS );
}
