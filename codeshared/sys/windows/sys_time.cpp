#include "sys/sys_public.h"

#include <Windows.h> // Don't define WIN32_MEAN_AND_LEAN or timeGetTime won't be included

int Sys_Milliseconds( qboolean baseTime )
{
	static int sys_timeBase = timeGetTime(); // requires winmm.lib
	int			sys_curtime;

	sys_curtime = timeGetTime( );
	if( !baseTime )
	{
		sys_curtime -= sys_timeBase;
	}

	return sys_curtime;
}
