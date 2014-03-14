/**
	\ingroup window
	\{
	
	\file
	
	Window & Input API.
	
	SDL-based window & input API shared across MP and SP, but not used in Dedicated Server.
**/

#pragma once

#include "../qcommon/q_sharedtypes.h"
#include "../qcommon/q_platform.h"

/**
	\brief Retrieve the system's clipboard contents.
	
	\return Clipboard contents, or NULL if none.
	\note Returns static memory that may be overwritten on the next call.
**/
char *Window_GetClipboardData( void );

/**
	\brief Shuts the window, event & input system down cleanly.
**/
void Window_Shutdown( void );

/**
	\brief Creates a window and sets it up for Input & Sound
	\param title Window title
	\return Resulting SDL_Window* as void* (to be passed on to the Renderer)
**/
void *Window_Create( const char *title );

/**
	\brief Applies any setting changes, such as resolution or fullscreen.
**/
void Window_Restart( void );

/**
	\brief Processes events, mostly forwarding them to Sys_QueEvent().
**/
void Window_Frame( void );



// initializes cycling through a DMA buffer and returns information on it
qboolean SNDDMA_Init( void );

// gets the current DMA position
int		SNDDMA_GetDMAPos( void );

// shutdown the DMA xfer.
void	SNDDMA_Shutdown( void );

void	SNDDMA_BeginPainting( void );

void	SNDDMA_Submit( void );

// end of Doxygen Group
/// \}
