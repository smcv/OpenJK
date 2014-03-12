#pragma once

// 

#if defined (_MSC_VER) && (_MSC_VER >= 1600)

#	include <stdint.h>

#elif defined (_MSC_VER)

#	include <io.h>

	typedef signed __int64 int64_t;
	typedef signed __int32 int32_t;
	typedef signed __int16 int16_t;
	typedef signed __int8  int8_t;
	typedef unsigned __int64 uint64_t;
	typedef unsigned __int32 uint32_t;
	typedef unsigned __int16 uint16_t;
	typedef unsigned __int8  uint8_t;
#else // not using MSVC

#	include <stdint.h>

#endif


typedef unsigned char byte;
typedef unsigned short word;
typedef unsigned long ulong;


typedef enum { qfalse=0, qtrue } qboolean;
#define ToQBoolean(x) ((x) ? qtrue : qfalse)
#ifdef __cplusplus
	inline qboolean operator!( const qboolean b )
	{
		return b ? qfalse : qtrue;
	}

	inline qboolean operator||( const qboolean lhs, const qboolean rhs )
	{
		return ToQBoolean( int(lhs) || int(rhs) );
	}

	inline qboolean operator|( const qboolean lhs, const qboolean rhs )
	{
		return lhs || rhs;
	}

	inline qboolean operator&&( const qboolean lhs, const qboolean rhs )
	{
		return ToQBoolean( int(lhs) && int(rhs) );
	}

	inline qboolean operator&( const qboolean lhs, const qboolean rhs )
	{
		return lhs && rhs;
	}
	
	inline qboolean operator^( const qboolean lhs, const qboolean rhs )
	{
		return ToQBoolean( lhs != rhs );
	}

	inline qboolean operator |=( qboolean& lhs, const qboolean rhs )
	{
		lhs = lhs | rhs;
		return lhs;
	}

	inline qboolean operator ^=( qboolean& lhs, const qboolean rhs )
	{
		lhs = lhs ^ rhs;
		return lhs;
	}

	inline qboolean operator &=( qboolean& lhs, const qboolean rhs )
	{
		lhs = lhs & rhs;
		return lhs;
	}
#endif



#define	CVAR_ARCHIVE		(0x00000001u)	// set to cause it to be saved to configuration file. used for system variables,
											//	not for player specific configurations
#define	CVAR_USERINFO		(0x00000002u)	// sent to server on connect or change
#define	CVAR_SERVERINFO		(0x00000004u)	// sent in response to front end requests
#define	CVAR_SYSTEMINFO		(0x00000008u)	// these cvars will be duplicated on all clients
#define	CVAR_INIT			(0x00000010u)	// don't allow change from console at all, but can be set from the command line
#define	CVAR_LATCH			(0x00000020u)	// will only change when C code next does a Cvar_Get(), so it can't be changed
											//	without proper initialization. modified will be set, even though the value
											//	hasn't changed yet
#define	CVAR_ROM			(0x00000040u)	// display only, cannot be set by user at all (can be set by code)
#define	CVAR_USER_CREATED	(0x00000080u)	// created by a set command
#define CVAR_CHEAT			(0x00000200u)	// can not be changed if cheats are disabled
#define CVAR_NORESTART		(0x00000400u)	// do not clear when a cvar_restart is issued

// These flags are only returned by the Cvar_Flags() function
#define CVAR_MODIFIED		(0x40000000u)	// Cvar was modified
#define CVAR_NONEXISTENT	(0x80000000u)	// Cvar doesn't exist.

// nothing outside the Cvar_*() functions should modify these fields!
typedef struct cvar_s {
	char			*name;
	char			*string;
	char			*resetString;		// cvar_restart will reset to this value
	char			*latchedString;		// for CVAR_LATCH vars
	uint32_t		flags;
	qboolean		modified;			// set each time the cvar is changed
	int				modificationCount;	// incremented each time the cvar is changed
	float			value;				// atof( string )
	int				integer;			// atoi( string )
	qboolean		validate;
	qboolean		integral;
	float			min, max;

	struct cvar_s	*next, *prev;
	struct cvar_s	*hashNext, *hashPrev;
	int				hashIndex;
} cvar_t;