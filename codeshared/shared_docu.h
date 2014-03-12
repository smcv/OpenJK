// This file is for documentation only.

/**
	\defgroup common CommonLib
	Common code that is shared between the SP and MP client, including dedicated server.
	
	
	\defgroup window WindowLib
	Code for Window creation & Input. Shared between SP and MP client, but not the dedicated server.
	
	
	\defgroup console ConsoleLib
	Code for the different Consoles
	
	
	\dir codeshared/con
	Contains code for the console that's displayed prior to start/when running a dedicated server.
	
	There are two versions, one for dedicated servers and one for normal clients.
	
	
	\dir codeshared/forward
	Contains forward declarations of functions with client-specific implementation. They have to be provided by the clients linking SharedLib or WindowLib.
	
	
	\dir codeshared/qcommon
	Contains types & platform specific defines required by the shared code and everything else.
	
	
	\dir codeshared/ghoul2
	Contains the Ghoul 2 code for models & animations.
	
	
	\dir codeshared/sys
	Contains platform specific code.

	Includes network code, which while not used by the SP client is still platform specific, so it's nice to keep it out of the main code base.
	
	Does not include window creation & input code.
	
	
	\dir codeshared/window
	Contains window, input and general event code.
**/
