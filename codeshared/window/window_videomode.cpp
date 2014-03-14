/**
	\file
	Extended r_mode support.

	Values -2 to 12 stay as before (for compat), values above are filled with additional available modes.
**/

#include "window/window_public.h"
#include "window/window_local.h"
#include "forward/cl_public.h"

#include <vector>

extern cvar_t *r_fullscreen;
extern cvar_t *r_customwidth;
extern cvar_t *r_customheight;
extern cvar_t *r_customformat;
extern cvar_t *r_customrate;
extern cvar_t *r_mode;

typedef struct vidmode_s
{
	const char *description;
	int         width, height;
} vidmode_t;

// What should Window_VideoMode_GetDisplayMode use when supplied with an invalid mode?
enum
{
	R_MODE_FALLBACK = 4,
};

// These have to stay for base compat (referenced in menu files etc.)
static const vidmode_t s_fixedVideoModes[] = {
	{ "Mode  0: 320x240", 320, 240 },
	{ "Mode  1: 400x300", 400, 300 },
	{ "Mode  2: 512x384", 512, 384 },
	{ "Mode  3: 640x480", 640, 480 },
	{ "Mode  4: 800x600", 800, 600 },
	{ "Mode  5: 960x720", 960, 720 },
	{ "Mode  6: 1024x768", 1024, 768 },
	{ "Mode  7: 1152x864", 1152, 864 },
	{ "Mode  8: 1280x1024", 1280, 1024 },
	{ "Mode  9: 1600x1200", 1600, 1200 },
	{ "Mode 10: 2048x1536", 2048, 1536 },
	{ "Mode 11: 856x480 (wide)", 856, 480 },
	{ "Mode 12: 2400x600(surround)", 2400, 600 }
};
static const unsigned int s_numFixedVideoModes = ( sizeof( s_fixedVideoModes ) / sizeof( s_fixedVideoModes[ 0 ] ) );

// which r_mode corresponds to which SDL VideoMode index? (unsupported = -1)
static std::vector< int > s_modeTranslationTable;

static int s_displayIndex = -1;

/*
	R_ModeList_f

	\brief Lists available r_mode values in console.
*/
static void VideoMode_ModeList_f( void )
{
	SDL_assert( s_displayIndex >= 0 );
	Com_Printf( "\n" );
	Com_Printf( "r_mode values for display %i:", s_displayIndex );
	Com_Printf( "Mode -2: Use desktop resolution\n" );
	Com_Printf( "Mode -1: Use r_customWidth and r_customHeight variables\n" );
	// List fixed modes
	for( unsigned int i = 0; i < s_numFixedVideoModes; ++i )
	{
		if( s_modeTranslationTable[ i ] < 0 )
		{
			Com_Printf( "%s (unsupported)\n", s_fixedVideoModes[ i ].description );
		}
		else
		{
			SDL_DisplayMode sdlMode;
			// This previously worked, it better work again
			int res = SDL_GetDisplayMode( s_displayIndex, s_modeTranslationTable[ i ], &sdlMode );
			SDL_assert( res >= 0 );
			Com_Printf( "%s %iBPP @%iHz\n", s_fixedVideoModes[ i ].description, sdlMode.format, sdlMode.refresh_rate );
		}
	}
	// List dynamic modes
	for( unsigned int i = s_numFixedVideoModes; i < s_modeTranslationTable.size(); ++i )
	{
		int mode = s_modeTranslationTable[ i ];
		SDL_DisplayMode sdlMode;
		// This previously worked, it better work again
		int res = SDL_GetDisplayMode( s_displayIndex, mode, &sdlMode );
		SDL_assert( res >= 0 );
		Com_Printf( "Mode %i: %ix%i %iBPP @%iHz", i, sdlMode.w, sdlMode.h, sdlMode.format, sdlMode.refresh_rate );
	}
	Com_Printf( "\n" );
}

/**
	\brief Finds the fixed mode with the given width and height.
	\return Index into s_fixedVideoModes or -1
**/
static int FindMatchingFixedMode( int width, int height )
{
	for( int i = 0; i < s_numFixedVideoModes; ++i )
	{
		if( s_fixedVideoModes[ i ].width == width && s_fixedVideoModes[ i ].height == height )
		{
			return i;
		}
	}
	return -1;
}

static void SetupTranslationTable()
{
	int numSDLModes = SDL_GetNumDisplayModes( s_displayIndex );

	s_modeTranslationTable.clear();
	s_modeTranslationTable.reserve( numSDLModes + s_numFixedVideoModes ); // upper limit, there's likely an intersection

	// Initialize fixed values to -1
	for( int i = 0; i < s_numFixedVideoModes; ++i )
	{
		s_modeTranslationTable.push_back( -1 );
	}

	for( int i = 0; i < numSDLModes; ++i )
	{
		SDL_DisplayMode mode;
		if( SDL_GetDisplayMode( s_displayIndex, i, &mode ) < 0 )
		{
			Com_Printf( S_COLOR_YELLOW "WARNING: Failed to query DisplayMode %i: %s\n", i, SDL_GetError() );
		}
		else
		{
			int matchingIndex = FindMatchingFixedMode( mode.w, mode.h );
			// If this is the first match
			if( matchingIndex >= 0 && s_modeTranslationTable[ matchingIndex ] == -1 )
			{
				s_modeTranslationTable[ matchingIndex ] = i;
			}
			else
			{
				s_modeTranslationTable.push_back( i );
			}
		}
	}
}

void Window_VideoMode_Init( int displayIndex )
{
	SDL_assert( displayIndex >= 0 && displayIndex < SDL_GetNumVideoDisplays() );
	s_displayIndex = displayIndex;

	SetupTranslationTable();

	Cmd_AddCommand( "modelist", VideoMode_ModeList_f );
}

void Window_VideoMode_Shutdown( void )
{
	Cmd_RemoveCommand( "modelist" );
	s_modeTranslationTable.clear();
	s_displayIndex = -1;
}

SDL_DisplayMode Window_VideoMode_GetDisplayMode()
{
	// Window_VideoMode_Init must be called first!
	SDL_assert( s_modeTranslationTable.size() >= s_numFixedVideoModes );
	int mode = r_mode->integer;
	if( mode < -2 || (unsigned int)mode >= s_modeTranslationTable.size() )
	{
		Com_Printf( S_COLOR_YELLOW "WARNING: r_mode must be between -2 and %i. Falling back to %s.\n", s_modeTranslationTable.size( ), s_fixedVideoModes[ R_MODE_FALLBACK ].description );
		mode = R_MODE_FALLBACK;
	}
	if( mode == -1 && ( r_customheight->integer <= 0 || r_customheight->integer <= 0 ) )
	{
		Com_Printf( S_COLOR_YELLOW "WARNING: r_customwidth and r_customheight must be at least 1. Falling back to %s.\n", s_fixedVideoModes[ R_MODE_FALLBACK ].description );
		mode = R_MODE_FALLBACK;
	}
	SDL_DisplayMode desiredMode;

	// Initialize to Desktop values
	if( SDL_GetDesktopDisplayMode( s_displayIndex, &desiredMode ) < 0 )
	{
		// SDL will probably choose something reasonable when these are 0
		desiredMode.format = 0;
		desiredMode.refresh_rate = 0;

		// -2 means: use desktop size
		if( mode == -2 )
		{
			Com_Printf( S_COLOR_YELLOW "WARNING: Could not retrieve desktop resolution: %s\nFalling back to %s\n", SDL_GetError( ), s_fixedVideoModes[ R_MODE_FALLBACK ] );
			mode = R_MODE_FALLBACK;
		}
	}
	// -1: supply values yourself
	if( mode == -1 )
	{
		desiredMode.w = r_customwidth->integer;
		desiredMode.h = r_customheight->integer;
	}
	else if( mode >= 0 )
	{
		int displayMode = s_modeTranslationTable[ mode ];
		if( displayMode == -1 ) // unavailable fixed mode?
		{
			SDL_assert( mode >= 0 && mode < s_numFixedVideoModes );
			desiredMode.w = s_fixedVideoModes[ mode ].width;
			desiredMode.h = s_fixedVideoModes[ mode ].height;
		}
		else // fixed mode with SDL equivalent
		{
			int err = SDL_GetDisplayMode( s_displayIndex, s_modeTranslationTable[ mode ], &desiredMode );
			// This previously worked so it should work again.
			SDL_assert( err >= 0 );
		}
	}
	// If we are in r_fullscreen, we *must* use one of the available modes. I think.
	if( r_fullscreen->integer )
	{
		SDL_DisplayMode availableMode;
		if( SDL_GetClosestDisplayMode( s_displayIndex, &desiredMode, &availableMode ) )
		{
			return availableMode;
		}
		else
		{
			// This is already the fallback
			if( mode == R_MODE_FALLBACK )
			{
				Com_Error( ERR_FATAL, "ERROR: No display mode close to %s (the fallback) available!", s_fixedVideoModes[ R_MODE_FALLBACK ].description );
			}
			// Desired mode is not available. Try fallback.
			int displayMode = s_modeTranslationTable[ R_MODE_FALLBACK ];
			if( displayMode == -1 ) // no direct equivalent?
			{
				desiredMode.w = s_fixedVideoModes[ R_MODE_FALLBACK ].width;
				desiredMode.h = s_fixedVideoModes[ R_MODE_FALLBACK ].height;
				desiredMode.format = 0;
				desiredMode.refresh_rate = 0;
				if( !SDL_GetClosestDisplayMode( s_displayIndex, &desiredMode, &availableMode ) )
				{
					Com_Error( ERR_FATAL, "ERROR: No display mode close to r_mode %i or %s (the fallback) available!", mode, s_fixedVideoModes[ R_MODE_FALLBACK ].description );
				}
			}
			else
			{
				int err = SDL_GetDisplayMode( s_displayIndex, s_modeTranslationTable[ R_MODE_FALLBACK ], &availableMode );
				// This previously worked so it should work again.
				SDL_assert( err >= 0 );
			}
			return availableMode;
		}
	}
	// Windowed however has more freedom, especially with regards to size.
	else
	{
		return desiredMode;
	}
}
