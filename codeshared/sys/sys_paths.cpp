/**
	\file
	Common path-related functions.
**/

#include "sys/sys_local.h"
#include "sys/sys_public.h"
#include "forward/cl_public.h"

static char binaryPath[ MAX_OSPATH ] = { 0 };

void Sys_SetBinaryPath( const char *path )
{
	Q_strncpyz( binaryPath, path, sizeof( binaryPath ) );
}

const char *Sys_BinaryPath( void )
{
	return binaryPath;
}