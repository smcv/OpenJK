/**
	\file
	Contains the main() function
**/

#include "sys/sys_local.h"
#include "sys/sys_public.h"
#include "forward/cl_public.h"

#include <cstring>

int main( int argc, char **argv )
{
	char	commandLine[ MAX_STRING_CHARS ] = { 0 };

	OS_Init();

	// Set the initial time base
	Sys_Milliseconds( qfalse );

	Sys_SetBinaryPath( Sys_Dirname( argv[ 0 ] ) );

	// Concatenate the command line for passing to Com_Init
	for( int i = 1; i < argc; i++ )
	{
		const bool containsSpaces = ( strchr( argv[ i ], ' ' ) != NULL );
		if( containsSpaces )
			Q_strcat( commandLine, sizeof( commandLine ), "\"" );

		Q_strcat( commandLine, sizeof( commandLine ), argv[ i ] );

		if( containsSpaces )
			Q_strcat( commandLine, sizeof( commandLine ), "\"" );

		Q_strcat( commandLine, sizeof( commandLine ), " " );
	}

	Com_Init( commandLine );

	while( 1 )
	{
		Com_Frame();
	}

	return 0;
}
