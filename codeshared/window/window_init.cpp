#include "window/window_public.h"
#include "window/window_local.h"
#include "forward/cl_public.h"

cvar_t *vid_xpos = NULL;
cvar_t *vid_ypos = NULL;
cvar_t *r_mode = NULL;
cvar_t *r_fullscreen = NULL;
cvar_t *r_noborder = NULL;
cvar_t *r_centerWindow = NULL;
cvar_t *r_customwidth = NULL;
cvar_t *r_customheight = NULL;
cvar_t *r_display = NULL;
cvar_t *in_joystick = NULL;

static void Window_ReloadCvars( void )
{
	// Position of Window on Screen
	vid_xpos = Cvar_Get( "vid_xpos", "", 0 );
	vid_ypos = Cvar_Get( "vid_ypos", "", 0 );
	// Resolution
	r_mode = Cvar_Get( "r_mode", "4", CVAR_ARCHIVE | CVAR_LATCH );
	// custom resolution (if r_mode -1)
	r_customwidth = Cvar_Get( "r_customwidth", "1600", CVAR_ARCHIVE | CVAR_LATCH );
	r_customheight = Cvar_Get( "r_customheight", "1024", CVAR_ARCHIVE | CVAR_LATCH );
	// Fullscreen?
	r_fullscreen = Cvar_Get( "r_fullscreen", "0", CVAR_ARCHIVE | CVAR_LATCH );
	// Borderless Window (if r_fullscreen 0)
	r_noborder = Cvar_Get( "r_noborder", "0", CVAR_ARCHIVE | CVAR_LATCH );
	// Center the window on the display?
	r_centerWindow = Cvar_Get( "r_centerWindow", "0", CVAR_ARCHIVE | CVAR_LATCH );
	// which display to put the window on
	r_display = Cvar_Get( "r_display", "0", CVAR_ARCHIVE | CVAR_LATCH );

	// whether joystick input is enabled
	in_joystick = Cvar_Get( "in_joystick", "0", CVAR_ARCHIVE ); // can be changed on-the-fly.
}

static SDL_Window *s_window = NULL;

static void Window_ApplyWindowSettings()
{
	SDL_assert( s_window );

	int display = r_display->integer;
	if( display < 0 || display >= SDL_GetNumVideoDisplays() )
	{
		Com_Printf( S_COLOR_YELLOW "WARNING: Invalid display %i specified in r_display, must be at most %i! Defaulting to 0.\n", display, SDL_GetNumVideoDisplays() - 1 );
		display = 0;
	}

	Window_VideoMode_Init( display );
	SDL_DisplayMode mode = Window_VideoMode_GetDisplayMode();

	SDL_Rect displayBounds;
	SDL_assert( SDL_GetDisplayBounds( display, &displayBounds ) == 0 ); // should only error on invalid index

	int xpos = displayBounds.x;
	int ypos = displayBounds.y;

	if( !r_fullscreen->integer )
	{
		if( r_centerWindow->integer )
		{
			xpos = SDL_WINDOWPOS_CENTERED_DISPLAY( display );
			ypos = SDL_WINDOWPOS_CENTERED_DISPLAY( display );
		}
		else
		{
			xpos += vid_xpos->integer;
			ypos += vid_ypos->integer;
		}
	}

	SDL_SetWindowPosition( s_window, xpos, ypos );
	if( r_fullscreen->integer )
	{
		SDL_SetWindowDisplayMode( s_window, &mode );
	}
	else
	{
		SDL_SetWindowBordered( s_window, r_noborder->integer ? SDL_FALSE : SDL_TRUE );
		SDL_SetWindowSize( s_window, mode.w, mode.h );
	}
	SDL_SetWindowFullscreen( s_window, r_fullscreen->integer ? SDL_WINDOW_FULLSCREEN : 0 );
}

static void Window_ApplySettings( void )
{
	Window_ReloadCvars(); // first, update all the latched cvars
	Window_ApplyWindowSettings();
}

void *Window_Create( const char *title )
{
	SDL_assert( !s_window );

	if( SDL_Init( SDL_INIT_VIDEO | SDL_INIT_EVENTS | SDL_INIT_AUDIO | SDL_INIT_JOYSTICK ) != 0 )
	{
		Com_Error( ERR_FATAL, "SDL initialization failed! %s", SDL_GetError() );
	}

	// Window settings will be adjusted by ApplySettings()
	s_window = SDL_CreateWindow( title, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 320, 240, SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL );

	Window_ApplySettings();

	return s_window;
}

void Window_Shutdown( void )
{
	Window_VideoMode_Shutdown();
	if( !s_window )
	{
		return;
	}
	SDL_DestroyWindow( s_window );

	SDL_Quit();
	s_window = NULL;
}

void Window_Restart( void )
{
	Window_VideoMode_Shutdown( );
	// No need to destroy the window to change its settings :)
	Window_ApplySettings( );
}
