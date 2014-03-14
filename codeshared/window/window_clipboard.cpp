#include "window/window_public.h"
#include "forward/cl_public.h"

#include "SDL.h"


const char *Window_GetClipboardData( void ) {
	char *cb = SDL_GetClipboardText();
	if( !cb )
	{
		return NULL;
	}

	unsigned int size = SDL_strlen( cb ) + 1;
	char *data = ( char* )Z_Malloc( size, TAG_CLIPBOARD );
	Q_strncpyz( data, cb, size );
	SDL_free( cb );
	return data;
}
