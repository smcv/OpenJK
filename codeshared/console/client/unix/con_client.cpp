#include "console/con_public.h"

#include "forward/cl_public.h"
#include "sys/sys_public.h"

// TODO: Use ioq3's con_tty

void Con_CreateConsole( void )
{
	// Nothing to do here
}

void Con_ShowConsole( int level, qboolean quitOnClose )
{
	// Nothing to do here
}

void Con_Print( const char *message )
{
	fputs( message, stdout );
}

void Con_ShowError( const char *message )
{
	fputs( message, stderr );
}

void Con_Frame( void )
{
	// Nothing to do here
}

void Con_Shutdown( void )
{
	// Nothing to do here
}
