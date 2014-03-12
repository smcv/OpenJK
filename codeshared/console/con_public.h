/**
	\ingroup console
	\{
	
	\file
	
	Console API.
	
	Common API for the different console types (client & dedicated)
**/

#include "qcommon/q_sharedtypes.h" // qboolean

/**
	\brief Creates and displays the console.

	On Windows client, this creates a new window.
	On Windows dedicated, this uses the ordinary console.
	On Unix client, it does nothing.
	On Unix dedicated, this sets up stdin for nonblocking reading.
**/
void Con_CreateConsole( void );

/**
	\brief Changes visibility of the console window.
	
	On Windows Dedicated and Unix the console is visible if and only if the game was started from there. There's no showing or hiding it.
	
	\param level 0 -> invisible, 1 -> visible, 2 -> visible and minimized
	\param quitOnClose Whether to Quit the game when the console window is closed. Only makes a difference in Windows Client.
*/
void Con_ShowConsole( int level, qboolean quitOnClose );

/**
	\brief Writes to the console
*/
void Con_Print( const char *message );

/**
	\brief Polls the console for input.
**/
void Con_Frame( void );

/**
	\brief Shuts the console down.
**/
void Con_ShutdownConsole( void );

// end of Doxygen Group
/// \}
