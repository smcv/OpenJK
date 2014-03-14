#include "window/window_public.h"
#include "window/window_local.h"
#include "forward/cl_public.h"




void Window_Create( void )
{
	vid_xpos = ri->Cvar_Get( "vid_xpos", "", 0 );
	vid_ypos = ri->Cvar_Get( "vid_ypos", "", 0 );
	r_mode = ri->Cvar_Get( "r_mode", "4", CVAR_ARCHIVE | CVAR_LATCH );
	r_fullscreen = ri->Cvar_Get( "r_fullscreen", "0", CVAR_ARCHIVE | CVAR_LATCH );
	r_noborder = ri->Cvar_Get( "r_noborder", "0", CVAR_ARCHIVE | CVAR_LATCH );
	r_centerWindow = ri->Cvar_Get( "r_centerWindow", "0", CVAR_ARCHIVE | CVAR_LATCH );
	r_customwidth = ri->Cvar_Get( "r_customwidth", "1600", CVAR_ARCHIVE | CVAR_LATCH );
	r_customheight = ri->Cvar_Get( "r_customheight", "1024", CVAR_ARCHIVE | CVAR_LATCH );

	SDL_GetNumDisplayModes
	SDL_GetDisplayMode
}

void Window_Shutdown( void )
{

}

void Window_ApplySettings( void )
{

}
