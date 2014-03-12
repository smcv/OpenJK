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

void IN_Init( void );

// end of Doxygen Group
/// \}
