#include "sys/sys_public.h"

#include <sys/time.h>

static int Sys_Milliseconds2()
{
	struct timeval tp;

	gettimeofday(&tp, NULL);
	
	return tp.tv_usec/1000 + tp.tv_sec * 1000;
}

int Sys_Milliseconds( qboolean baseTime )
{
	static int timeBase = Sys_Milliseconds2();
	if (baseTime)
	{
		return Sys_Milliseconds2();
	}
	else
	{
		return Sys_Milliseconds2() - timeBase;
	}
}
