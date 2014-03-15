#include "window/window_public.h"
#include "window/window_local.h"
#include "forward/cl_public.h"
#include "client/keycodes.h"
#include "sys/sys_public.h"

#include <cstring>

extern cvar_t *cl_consoleKeys;

static cvar_t *in_mouse = NULL;
static cvar_t *in_nograb = NULL;

static qboolean mouseAvailable = qfalse;
static qboolean mouseActive = qfalse;

/*
===============
IN_IsConsoleKey

TODO: If the SDL_Scancode situation improves, use it instead of
both of these methods
===============
*/

#define MAX_CONSOLE_KEYS 16

static qboolean IN_IsConsoleKey( fakeAscii_t key, int character )
{
	typedef struct consoleKey_s
	{
		enum
		{
			QUAKE_KEY,
			CHARACTER
		} type;

		union
		{
			fakeAscii_t key;
			int character;
		} u;
	} consoleKey_t;

	static consoleKey_t consoleKeys[ MAX_CONSOLE_KEYS ];
	static int numConsoleKeys = 0;
	int i;

	// Only parse the variable when it changes
	if( cl_consoleKeys->modified )
	{
		const char *text_p;
		char *token;

		cl_consoleKeys->modified = qfalse;
		text_p = cl_consoleKeys->string;
		numConsoleKeys = 0;

		COM_BeginParseSession( "cl_consoleKeys" );
		while( numConsoleKeys < MAX_CONSOLE_KEYS )
		{
			consoleKey_t *c = &consoleKeys[ numConsoleKeys ];
			int charCode = 0;

			token = COM_Parse( &text_p );
			if( !token[ 0 ] )
				break;

			if( strlen( token ) == 4 )
				charCode = Com_HexStrToInt( token );

			if( charCode > 0 )
			{
				c->type = consoleKey_t::CHARACTER;
				c->u.character = charCode;
			}
			else
			{
				c->type = consoleKey_t::QUAKE_KEY;
				c->u.key = ( fakeAscii_t )Key_StringToKeynum( token );

				// 0 isn't a key
				if( c->u.key <= 0 )
					continue;
			}

			numConsoleKeys++;
		}
		COM_EndParseSession();
	}

	// If the character is the same as the key, prefer the character
	if( key == character )
		key = A_NULL;

	for( i = 0; i < numConsoleKeys; i++ )
	{
		consoleKey_t *c = &consoleKeys[ i ];

		switch( c->type )
		{
		case consoleKey_t::QUAKE_KEY:
			if( key && c->u.key == key )
				return qtrue;
			break;

		case consoleKey_t::CHARACTER:
			if( c->u.character == character )
				return qtrue;
			break;
		}
	}

	return qfalse;
}

/*
===============
IN_TranslateSDLToJKKey
===============
*/
static const char *IN_TranslateSDLToJKKey( SDL_Keysym *keysym, fakeAscii_t *key, qboolean down )
{
	static unsigned char buf[ 2 ] = { '\0', '\0' };

	*buf = '\0';
	*key = A_NULL;

	if( keysym->sym >= SDLK_SPACE && keysym->sym < SDLK_DELETE )
	{
		// These happen to match the ASCII chars
		*key = ( fakeAscii_t )keysym->sym;
	}
	else
	{
		switch( keysym->sym )
		{
		case SDLK_PAGEUP:       *key = A_PAGE_UP;       break;
		case SDLK_KP_9:         *key = A_KP_9;          break;
		case SDLK_PAGEDOWN:     *key = A_PAGE_DOWN;     break;
		case SDLK_KP_3:         *key = A_KP_3;          break;
		case SDLK_KP_7:         *key = A_KP_7;          break;
		case SDLK_HOME:         *key = A_HOME;          break;
		case SDLK_KP_1:         *key = A_KP_1;          break;
		case SDLK_END:          *key = A_END;           break;
		case SDLK_KP_4:         *key = A_KP_4;          break;
		case SDLK_LEFT:         *key = A_CURSOR_LEFT;   break;
		case SDLK_KP_6:         *key = A_KP_6;          break;
		case SDLK_RIGHT:        *key = A_CURSOR_RIGHT;  break;
		case SDLK_KP_2:         *key = A_KP_2;          break;
		case SDLK_DOWN:         *key = A_CURSOR_DOWN;   break;
		case SDLK_KP_8:         *key = A_KP_8;          break;
		case SDLK_UP:           *key = A_CURSOR_UP;     break;
		case SDLK_ESCAPE:       *key = A_ESCAPE;        break;
		case SDLK_KP_ENTER:     *key = A_KP_ENTER;      break;
		case SDLK_RETURN:       *key = A_ENTER;         break;
		case SDLK_TAB:          *key = A_TAB;           break;
		case SDLK_F1:           *key = A_F1;            break;
		case SDLK_F2:           *key = A_F2;            break;
		case SDLK_F3:           *key = A_F3;            break;
		case SDLK_F4:           *key = A_F4;            break;
		case SDLK_F5:           *key = A_F5;            break;
		case SDLK_F6:           *key = A_F6;            break;
		case SDLK_F7:           *key = A_F7;            break;
		case SDLK_F8:           *key = A_F8;            break;
		case SDLK_F9:           *key = A_F9;            break;
		case SDLK_F10:          *key = A_F10;           break;
		case SDLK_F11:          *key = A_F11;           break;
		case SDLK_F12:          *key = A_F12;           break;

		case SDLK_BACKSPACE:    *key = A_BACKSPACE;     break;
		case SDLK_KP_PERIOD:    *key = A_KP_PERIOD;     break;
		case SDLK_DELETE:       *key = A_DELETE;        break;
		case SDLK_PAUSE:        *key = A_PAUSE;         break;

		case SDLK_LSHIFT:
		case SDLK_RSHIFT:       *key = A_SHIFT;         break;

		case SDLK_LCTRL:
		case SDLK_RCTRL:        *key = A_CTRL;          break;

		case SDLK_RALT:
		case SDLK_LALT:         *key = A_ALT;           break;

		case SDLK_KP_5:         *key = A_KP_5;          break;
		case SDLK_INSERT:       *key = A_INSERT;        break;
		case SDLK_KP_0:         *key = A_KP_0;          break;
		case SDLK_KP_MULTIPLY:  *key = A_STAR;          break;
		case SDLK_KP_PLUS:      *key = A_KP_PLUS;       break;
		case SDLK_KP_MINUS:     *key = A_KP_MINUS;      break;
		case SDLK_KP_DIVIDE:    *key = A_FORWARD_SLASH; break;

		case SDLK_SCROLLLOCK:   *key = A_SCROLLLOCK;    break;
		case SDLK_NUMLOCKCLEAR: *key = A_NUMLOCK;       break;
		case SDLK_CAPSLOCK:     *key = A_CAPSLOCK;      break;

		default:
			break;
		}
	}

	if( *key == A_BACKSPACE )
		*buf = '\b'; // Full-hack ahead!

	if( IN_IsConsoleKey( *key, 0 ) )
	{
		// Console keys can't be bound or generate characters
		*key = A_CONSOLE;
		*buf = '\0';
	}

	return ( const char * )buf;
}

/*
===============
IN_GobbleMouseMotionEvents
===============
*/
static void IN_GobbleMouseMotionEvents( void )
{
	SDL_Event dummy[ 1 ];
	int val = 0;

	// Gobble any mouse motion events
	SDL_PumpEvents( );
	while( ( val = SDL_PeepEvents( dummy, 1, SDL_GETEVENT,
		SDL_MOUSEMOTION, SDL_MOUSEMOTION ) ) > 0 ) {
	}

	if( val < 0 )
		Com_Printf( "IN_GobbleMotionEvents failed: %s\n", SDL_GetError( ) );
}

/*
===============
IN_ActivateMouse
===============
*/
static void IN_ActivateMouse( void )
{
	if( !mouseAvailable || !SDL_WasInit( SDL_INIT_VIDEO ) )
		return;

	if( !mouseActive )
	{
		SDL_SetRelativeMouseMode( SDL_TRUE );
		SDL_SetWindowGrab( Window_GetWindow(), SDL_TRUE );

		IN_GobbleMouseMotionEvents( );
	}

	// in_nograb makes no sense in fullscreen mode
	if( !Cvar_VariableIntegerValue( "r_fullscreen" ) )
	{
		if( in_nograb->modified || !mouseActive )
		{
			if( in_nograb->integer )
				SDL_SetWindowGrab( Window_GetWindow( ), SDL_FALSE );
			else
				SDL_SetWindowGrab( Window_GetWindow( ), SDL_TRUE );

			in_nograb->modified = qfalse;
		}
	}

	mouseActive = qtrue;
}

/*
===============
IN_DeactivateMouse
===============
*/
static void IN_DeactivateMouse( void )
{
	if( !SDL_WasInit( SDL_INIT_VIDEO ) )
		return;

	// Always show the cursor when the mouse is disabled,
	// but not when fullscreen
	if( !Cvar_VariableIntegerValue( "r_fullscreen" ) )
		SDL_ShowCursor( 1 );

	if( !mouseAvailable )
		return;

	if( mouseActive )
	{
		IN_GobbleMouseMotionEvents( );

		SDL_SetWindowGrab( Window_GetWindow(), SDL_FALSE );
		SDL_SetRelativeMouseMode( SDL_FALSE );

		// Don't warp the mouse unless the cursor is within the window
		if( SDL_GetWindowFlags( Window_GetWindow() ) & SDL_WINDOW_MOUSE_FOCUS )
		{
			int w, h;
			SDL_GetWindowSize( Window_GetWindow(), &w, &h );
			SDL_WarpMouseInWindow( Window_GetWindow(), w / 2, h / 2 );
		}

		mouseActive = qfalse;
	}
}

void IN_Init()
{
	int appState;

	if( !SDL_WasInit( SDL_INIT_VIDEO ) )
	{
		Com_Error( ERR_FATAL, "IN_Init called before SDL_Init( SDL_INIT_VIDEO )" );
		return;
	}

	Com_DPrintf( "\n------- Input Initialization -------\n" );

	// mouse variables
	in_mouse = Cvar_Get( "in_mouse", "1", CVAR_ARCHIVE );
	in_nograb = Cvar_Get( "in_nograb", "0", CVAR_ARCHIVE );

	SDL_StartTextInput( );

	mouseAvailable = ( qboolean )( in_mouse->value != 0 );
	IN_DeactivateMouse( );

	appState = SDL_GetWindowFlags( Window_GetWindow() );
	Cvar_Set( "com_unfocused", !( appState & SDL_WINDOW_INPUT_FOCUS ) ? "1" : "0" );
	Cvar_Set( "com_minimized", appState & SDL_WINDOW_MINIMIZED ? "1" : "0" );

	IN_InitJoystick();
	Com_DPrintf( "------------------------------------\n" );
}



/*
===============
IN_ProcessEvents
===============
*/
static void IN_ProcessEvents( void )
{
	SDL_Event e;
	fakeAscii_t key = A_NULL;
	const char *character = NULL;
	static fakeAscii_t lastKeyDown = A_NULL;

	if( !SDL_WasInit( SDL_INIT_VIDEO ) )
		return;

	while( SDL_PollEvent( &e ) )
	{
		switch( e.type )
		{
		case SDL_KEYDOWN:
			character = IN_TranslateSDLToJKKey( &e.key.keysym, &key, qtrue );
			if( key != A_NULL )
				Sys_QueEvent( 0, SE_KEY, key, qtrue, 0, NULL );

			if( character )
				Sys_QueEvent( 0, SE_CHAR, *character, qfalse, 0, NULL );

			lastKeyDown = key;
			break;

		case SDL_KEYUP:
			IN_TranslateSDLToJKKey( &e.key.keysym, &key, qfalse );
			if( key != A_NULL )
				Sys_QueEvent( 0, SE_KEY, key, qfalse, 0, NULL );

			lastKeyDown = A_NULL;
			break;

		case SDL_TEXTINPUT:
			if( lastKeyDown != A_CONSOLE )
			{
				char *c = e.text.text;

				// Quick and dirty UTF-8 to UTF-32 conversion
				while( *c )
				{
					int utf32 = 0;

					if( ( *c & 0x80 ) == 0 )
						utf32 = *c++;
					else if( ( *c & 0xE0 ) == 0xC0 ) // 110x xxxx
					{
						utf32 |= ( *c++ & 0x1F ) << 6;
						utf32 |= ( *c++ & 0x3F );
					}
					else if( ( *c & 0xF0 ) == 0xE0 ) // 1110 xxxx
					{
						utf32 |= ( *c++ & 0x0F ) << 12;
						utf32 |= ( *c++ & 0x3F ) << 6;
						utf32 |= ( *c++ & 0x3F );
					}
					else if( ( *c & 0xF8 ) == 0xF0 ) // 1111 0xxx
					{
						utf32 |= ( *c++ & 0x07 ) << 18;
						utf32 |= ( *c++ & 0x3F ) << 6;
						utf32 |= ( *c++ & 0x3F ) << 6;
						utf32 |= ( *c++ & 0x3F );
					}
					else
					{
						Com_DPrintf( "Unrecognised UTF-8 lead byte: 0x%x\n", ( unsigned int )*c );
						c++;
					}

					if( utf32 != 0 )
					{
						if( IN_IsConsoleKey( A_NULL, utf32 ) )
						{
							Sys_QueEvent( 0, SE_KEY, A_CONSOLE, qtrue, 0, NULL );
							Sys_QueEvent( 0, SE_KEY, A_CONSOLE, qfalse, 0, NULL );
						}
						else
							Sys_QueEvent( 0, SE_CHAR, utf32, 0, 0, NULL );
					}
				}
			}
			break;

		case SDL_MOUSEMOTION:
			if( mouseActive )
				Sys_QueEvent( 0, SE_MOUSE, e.motion.xrel, e.motion.yrel, 0, NULL );
			break;

		case SDL_MOUSEBUTTONDOWN:
		case SDL_MOUSEBUTTONUP:
		{
								  unsigned short b;
								  switch( e.button.button )
								  {
								  case SDL_BUTTON_LEFT:	b = A_MOUSE1;     break;
								  case SDL_BUTTON_MIDDLE:	b = A_MOUSE3;     break;
								  case SDL_BUTTON_RIGHT:	b = A_MOUSE2;     break;
								  case SDL_BUTTON_X1:		b = A_MOUSE4;     break;
								  case SDL_BUTTON_X2:		b = A_MOUSE5;     break;
								  default: b = A_AUX0 + ( e.button.button - 6 ) % 32; break;
								  }
								  Sys_QueEvent( 0, SE_KEY, b,
									  ( e.type == SDL_MOUSEBUTTONDOWN ? qtrue : qfalse ), 0, NULL );
		}
			break;

		case SDL_MOUSEWHEEL:
			if( e.wheel.y > 0 )
			{
				Sys_QueEvent( 0, SE_KEY, A_MWHEELUP, qtrue, 0, NULL );
				Sys_QueEvent( 0, SE_KEY, A_MWHEELUP, qfalse, 0, NULL );
			}
			else
			{
				Sys_QueEvent( 0, SE_KEY, A_MWHEELDOWN, qtrue, 0, NULL );
				Sys_QueEvent( 0, SE_KEY, A_MWHEELDOWN, qfalse, 0, NULL );
			}
			break;

		case SDL_QUIT:
			Cbuf_ExecuteText( EXEC_NOW, "quit Closed window\n" );
			break;

		case SDL_WINDOWEVENT:
			switch( e.window.event )
			{
			case SDL_WINDOWEVENT_RESIZED:      Com_Printf( S_COLOR_YELLOW "WARNING: Window got resized, which should be impossible!\n" );  break; // shouldn't happen
			case SDL_WINDOWEVENT_MINIMIZED:    Cvar_Set( "com_minimized", "1" ); break;
			case SDL_WINDOWEVENT_RESTORED:
			case SDL_WINDOWEVENT_MAXIMIZED:    Cvar_Set( "com_minimized", "0" ); break;
			case SDL_WINDOWEVENT_FOCUS_LOST:   Cvar_Set( "com_unfocused", "1" ); break;
			case SDL_WINDOWEVENT_FOCUS_GAINED: Cvar_Set( "com_unfocused", "0" ); break;
			}
			break;

		default:
			break;
		}
	}
}

void Window_Frame( void )
{
	IN_JoyMove( );
	IN_ProcessEvents( );

	// If not DISCONNECTED (main menu) or ACTIVE (in game), we're loading
	qboolean loading = Com_IsLoading();
	bool fullscreen = SDL_GetWindowFlags( Window_GetWindow() ) & SDL_WINDOW_FULLSCREEN;


	if( !fullscreen && ( Key_GetCatcher( ) & KEYCATCH_CONSOLE ) )
	{
		// Console is down in windowed mode
		IN_DeactivateMouse( );
	}
	else if( !fullscreen && loading )
	{
		// Loading in windowed mode
		IN_DeactivateMouse( );
	}
	else if( !( SDL_GetWindowFlags( Window_GetWindow() ) & SDL_WINDOW_INPUT_FOCUS ) )
	{
		// Window not got focus
		IN_DeactivateMouse( );
	}
	else
		IN_ActivateMouse( );
}

void IN_Shutdown( void ) {
	SDL_StopTextInput( );

	IN_DeactivateMouse( );
	mouseAvailable = qfalse;

	IN_ShutdownJoystick();
}
