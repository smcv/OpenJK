#include "console/con_public.h"

#include "sys/sys_public.h"

#include "forward/cl_public.h"

// TODO: look at former win32/win_main_ded.cpp for how to do console input on windows better?

#include <cstdio>
#include <conio.h> // Windows-only, sadly.
#include <cctype>
#include <cstring>

void Con_CreateConsole( const char *message )
{
	// Nothing to do
}

void Con_ShowConsole( int level, qboolean quitOnClose )
{
	// Nothing do do
}

void Con_Print( const char *message )
{
	// FIXME: interrupts input, make it not print to the lowest line but keep the cursor there for input
	fputs( message, stdout );
}

void Con_ShowError( const char *message )
{
	fputs( message, stderr );
}

void Con_Frame( void )
{
	static char str[ MAX_STRING_CHARS ];
	static int len = 0;
	while( _kbhit() )
	{
		int ch = _getch();
		if( ch == 0 || ch == 0xE0 ) // function or arrow key?
		{
			_getch(); // then there's a nother one. we ignore those though.
		}
		// backspace?
		else if( ch == '\b' )
		{
			_putch( '\b' );
			_putch( ' ' );
			_putch( '\b' );
			if( len > 0 )
			{
				--len;
			}
		}
		// return?
		else if( ch == '\r' )
		{
			_putch( '\r' );
			_putch( '\n' );
			
			++len;
			str[ len ] = '\0';
			char *buf = ( char* )Z_Malloc( len, TAG_EVENT, qfalse );
			Q_strncpyz( buf, str, len );
			Sys_QueEvent( 0, SE_CONSOLE, 0, 0, len, buf );
			len = 0;
		}
		else if( isprint( ch ) && len < MAX_STRING_CHARS - 1 )
		{
			// FIXME: Linewrap will break deletion and thus display
			_putch( ch );
			str[ len ] = ch;
			++len;
		}
	}
}

void Con_DestroyConsole( void )
{
	// Nothing to do here
}
