#include "window/window_public.h"
#include "window/window_local.h"
#include "forward/cl_public.h"
#include "client/keycodes.h"
#include "sys/sys_public.h"

#include <cstring>
#include <cmath>

// FIXME: better joystick support would be nice?

cvar_t *in_joystick = NULL;
static cvar_t *in_joystickThreshold = NULL;
static cvar_t *in_joystickNo = NULL;
static cvar_t *in_joystickUseAnalog = NULL;

static SDL_Joystick *stick = NULL;

// We translate axes movement into keypresses
static int joy_keys[ 16 ] = {
	A_CURSOR_LEFT, A_CURSOR_RIGHT,
	A_CURSOR_UP, A_CURSOR_DOWN,
	A_JOY16, A_JOY17,
	A_JOY18, A_JOY19,
	A_JOY20, A_JOY21,
	A_JOY22, A_JOY23,
	A_JOY24, A_JOY25,
	A_JOY26, A_JOY27
};

// translate hat events into keypresses
// the 4 highest buttons are used for the first hat ...
static int hat_keys[ 16 ] = {
	A_JOY28, A_JOY29,
	A_JOY30, A_JOY31,
	A_JOY24, A_JOY25,
	A_JOY26, A_JOY27,
	A_JOY20, A_JOY21,
	A_JOY22, A_JOY23,
	A_JOY16, A_JOY17,
	A_JOY18, A_JOY19
};

struct stick_state_s
{
	qboolean buttons[ 16 ];  // !!! FIXME: these might be too many.
	unsigned int oldaxes;
	int oldaaxes[ MAX_JOYSTICK_AXIS ];
	unsigned int oldhats;
} stick_state;

/*
===============
IN_InitJoystick
===============
*/
void IN_InitJoystick( void )
{
	int i = 0;
	int total = 0;
	char buf[ 16384 ] = "";

	if( stick != NULL )
		SDL_JoystickClose( stick );

	// joystick variables

	in_joystick = Cvar_Get( "in_joystick", "0", CVAR_ARCHIVE | CVAR_LATCH );
	in_joystickThreshold = Cvar_Get( "joy_threshold", "0.15", CVAR_ARCHIVE );

	stick = NULL;
	memset( &stick_state, '\0', sizeof ( stick_state ) );

	total = SDL_NumJoysticks( );
	Com_DPrintf( "%d possible joysticks\n", total );

	// Print list and build cvar to allow ui to select joystick.
	for( i = 0; i < total; i++ )
	{
		Q_strcat( buf, sizeof( buf ), SDL_JoystickNameForIndex( i ) );
		Q_strcat( buf, sizeof( buf ), "\n" );
	}

	Cvar_Get( "in_availableJoysticks", buf, CVAR_ROM );

	if( !in_joystick->integer ) {
		Com_DPrintf( "Joystick is not active.\n" );
		return;
	}

	in_joystickNo = Cvar_Get( "in_joystickNo", "0", CVAR_ARCHIVE );
	if( in_joystickNo->integer < 0 || in_joystickNo->integer >= total )
		Cvar_Set( "in_joystickNo", "0" );

	in_joystickUseAnalog = Cvar_Get( "in_joystickUseAnalog", "0", CVAR_ARCHIVE );

	stick = SDL_JoystickOpen( in_joystickNo->integer );

	if( stick == NULL ) {
		Com_DPrintf( "No joystick opened.\n" );
		return;
	}

	Com_DPrintf( "Joystick %d opened\n", in_joystickNo->integer );
	Com_DPrintf( "Name:       %s\n", SDL_JoystickNameForIndex( in_joystickNo->integer ) );
	Com_DPrintf( "Axes:       %d\n", SDL_JoystickNumAxes( stick ) );
	Com_DPrintf( "Hats:       %d\n", SDL_JoystickNumHats( stick ) );
	Com_DPrintf( "Buttons:    %d\n", SDL_JoystickNumButtons( stick ) );
	Com_DPrintf( "Balls:      %d\n", SDL_JoystickNumBalls( stick ) );
	Com_DPrintf( "Use Analog: %s\n", in_joystickUseAnalog->integer ? "Yes" : "No" );

	SDL_JoystickEventState( SDL_QUERY );
}

/*
===============
IN_JoyMove
===============
*/
void IN_JoyMove( void )
{
	qboolean joy_pressed[ sizeof( joy_keys ) / sizeof( joy_keys[ 0 ] ) ];
	unsigned int axes = 0;
	unsigned int hats = 0;
	int total = 0;
	int i = 0;

	if( !stick )
		return;

	SDL_JoystickUpdate( );

	memset( joy_pressed, '\0', sizeof ( joy_pressed ) );

	// update the ball state.
	total = SDL_JoystickNumBalls( stick );
	if( total > 0 )
	{
		int balldx = 0;
		int balldy = 0;
		for( i = 0; i < total; i++ )
		{
			int dx = 0;
			int dy = 0;
			SDL_JoystickGetBall( stick, i, &dx, &dy );
			balldx += dx;
			balldy += dy;
		}
		if( balldx || balldy )
		{
			// !!! FIXME: is this good for stick balls, or just mice?
			// Scale like the mouse input...
			if( abs( balldx ) > 1 )
				balldx *= 2;
			if( abs( balldy ) > 1 )
				balldy *= 2;
			Sys_QueEvent( 0, SE_MOUSE, balldx, balldy, 0, NULL );
		}
	}

	// now query the stick buttons...
	total = SDL_JoystickNumButtons( stick );
	if( total > 0 )
	{
		int arrSize = ( int )( sizeof( stick_state.buttons ) / sizeof( stick_state.buttons[ 0 ] ) );
		if( total > arrSize )
			total = arrSize;
		for( i = 0; i < total; i++ )
		{
			qboolean pressed = ( qboolean )( SDL_JoystickGetButton( stick, i ) != 0 );
			if( pressed != stick_state.buttons[ i ] )
			{
				Sys_QueEvent( 0, SE_KEY, A_JOY1 + i, pressed, 0, NULL );
				stick_state.buttons[ i ] = pressed;
			}
		}
	}

	// look at the hats...
	total = SDL_JoystickNumHats( stick );
	if( total > 0 )
	{
		if( total > 4 ) total = 4;
		for( i = 0; i < total; i++ )
		{
			( ( Uint8 * )&hats )[ i ] = SDL_JoystickGetHat( stick, i );
		}
	}

	// update hat state
	if( hats != stick_state.oldhats )
	{
		for( i = 0; i < 4; i++ ) {
			if( ( ( Uint8 * )&hats )[ i ] != ( ( Uint8 * )&stick_state.oldhats )[ i ] ) {
				// release event
				switch( ( ( Uint8 * )&stick_state.oldhats )[ i ] ) {
				case SDL_HAT_UP:
					Sys_QueEvent( 0, SE_KEY, hat_keys[ 4 * i + 0 ], qfalse, 0, NULL );
					break;
				case SDL_HAT_RIGHT:
					Sys_QueEvent( 0, SE_KEY, hat_keys[ 4 * i + 1 ], qfalse, 0, NULL );
					break;
				case SDL_HAT_DOWN:
					Sys_QueEvent( 0, SE_KEY, hat_keys[ 4 * i + 2 ], qfalse, 0, NULL );
					break;
				case SDL_HAT_LEFT:
					Sys_QueEvent( 0, SE_KEY, hat_keys[ 4 * i + 3 ], qfalse, 0, NULL );
					break;
				case SDL_HAT_RIGHTUP:
					Sys_QueEvent( 0, SE_KEY, hat_keys[ 4 * i + 0 ], qfalse, 0, NULL );
					Sys_QueEvent( 0, SE_KEY, hat_keys[ 4 * i + 1 ], qfalse, 0, NULL );
					break;
				case SDL_HAT_RIGHTDOWN:
					Sys_QueEvent( 0, SE_KEY, hat_keys[ 4 * i + 2 ], qfalse, 0, NULL );
					Sys_QueEvent( 0, SE_KEY, hat_keys[ 4 * i + 1 ], qfalse, 0, NULL );
					break;
				case SDL_HAT_LEFTUP:
					Sys_QueEvent( 0, SE_KEY, hat_keys[ 4 * i + 0 ], qfalse, 0, NULL );
					Sys_QueEvent( 0, SE_KEY, hat_keys[ 4 * i + 3 ], qfalse, 0, NULL );
					break;
				case SDL_HAT_LEFTDOWN:
					Sys_QueEvent( 0, SE_KEY, hat_keys[ 4 * i + 2 ], qfalse, 0, NULL );
					Sys_QueEvent( 0, SE_KEY, hat_keys[ 4 * i + 3 ], qfalse, 0, NULL );
					break;
				default:
					break;
				}
				// press event
				switch( ( ( Uint8 * )&hats )[ i ] ) {
				case SDL_HAT_UP:
					Sys_QueEvent( 0, SE_KEY, hat_keys[ 4 * i + 0 ], qtrue, 0, NULL );
					break;
				case SDL_HAT_RIGHT:
					Sys_QueEvent( 0, SE_KEY, hat_keys[ 4 * i + 1 ], qtrue, 0, NULL );
					break;
				case SDL_HAT_DOWN:
					Sys_QueEvent( 0, SE_KEY, hat_keys[ 4 * i + 2 ], qtrue, 0, NULL );
					break;
				case SDL_HAT_LEFT:
					Sys_QueEvent( 0, SE_KEY, hat_keys[ 4 * i + 3 ], qtrue, 0, NULL );
					break;
				case SDL_HAT_RIGHTUP:
					Sys_QueEvent( 0, SE_KEY, hat_keys[ 4 * i + 0 ], qtrue, 0, NULL );
					Sys_QueEvent( 0, SE_KEY, hat_keys[ 4 * i + 1 ], qtrue, 0, NULL );
					break;
				case SDL_HAT_RIGHTDOWN:
					Sys_QueEvent( 0, SE_KEY, hat_keys[ 4 * i + 2 ], qtrue, 0, NULL );
					Sys_QueEvent( 0, SE_KEY, hat_keys[ 4 * i + 1 ], qtrue, 0, NULL );
					break;
				case SDL_HAT_LEFTUP:
					Sys_QueEvent( 0, SE_KEY, hat_keys[ 4 * i + 0 ], qtrue, 0, NULL );
					Sys_QueEvent( 0, SE_KEY, hat_keys[ 4 * i + 3 ], qtrue, 0, NULL );
					break;
				case SDL_HAT_LEFTDOWN:
					Sys_QueEvent( 0, SE_KEY, hat_keys[ 4 * i + 2 ], qtrue, 0, NULL );
					Sys_QueEvent( 0, SE_KEY, hat_keys[ 4 * i + 3 ], qtrue, 0, NULL );
					break;
				default:
					break;
				}
			}
		}
	}

	// save hat state
	stick_state.oldhats = hats;

	// finally, look at the axes...
	total = SDL_JoystickNumAxes( stick );
	if( total > 0 )
	{
		if( in_joystickUseAnalog->integer )
		{
			if( total > MAX_JOYSTICK_AXIS ) total = MAX_JOYSTICK_AXIS;
			for( i = 0; i < total; i++ )
			{
				Sint16 axis = SDL_JoystickGetAxis( stick, i );
				float f = ( ( float )abs( axis ) ) / 32767.0f;

				if( f < in_joystickThreshold->value ) axis = 0;

				if( axis != stick_state.oldaaxes[ i ] )
				{
					Sys_QueEvent( 0, SE_JOYSTICK_AXIS, i, axis, 0, NULL );
					stick_state.oldaaxes[ i ] = axis;
				}
			}
		}
		else
		{
			if( total > 16 ) total = 16;
			for( i = 0; i < total; i++ )
			{
				Sint16 axis = SDL_JoystickGetAxis( stick, i );
				float f = ( ( float )axis ) / 32767.0f;
				if( f < -in_joystickThreshold->value ) {
					axes |= ( 1 << ( i * 2 ) );
				}
				else if( f > in_joystickThreshold->value ) {
					axes |= ( 1 << ( ( i * 2 ) + 1 ) );
				}
			}
		}
	}

	/* Time to update axes state based on old vs. new. */
	if( axes != stick_state.oldaxes )
	{
		for( i = 0; i < 16; i++ ) {
			if( ( axes & ( 1 << i ) ) && !( stick_state.oldaxes & ( 1 << i ) ) ) {
				Sys_QueEvent( 0, SE_KEY, joy_keys[ i ], qtrue, 0, NULL );
			}

			if( !( axes & ( 1 << i ) ) && ( stick_state.oldaxes & ( 1 << i ) ) ) {
				Sys_QueEvent( 0, SE_KEY, joy_keys[ i ], qfalse, 0, NULL );
			}
		}
	}

	/* Save for future generations. */
	stick_state.oldaxes = axes;
}

/*
===============
IN_ShutdownJoystick
===============
*/
void IN_ShutdownJoystick( void )
{
	if( stick )
	{
		SDL_JoystickClose( stick );
		stick = NULL;
	}
}
