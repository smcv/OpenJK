#pragma once

// 

#if defined (_MSC_VER) && (_MSC_VER >= 1600)

#	include <stdint.h>

	// vsnprintf is ISO/IEC 9899:1999
	// abstracting this to make it portable
	int Q_vsnprintf( char *str, size_t size, const char *format, va_list args );

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

	// vsnprintf is ISO/IEC 9899:1999
	// abstracting this to make it portable
	int Q_vsnprintf( char *str, size_t size, const char *format, va_list args );
	
#else // not using MSVC

#	include <stdint.h>

#	include <stdio.h>
#	define Q_vsnprintf vsnprintf

#endif


typedef unsigned char byte;
typedef unsigned short word;
typedef unsigned long ulong;

typedef int32_t fileHandle_t;


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



#define	CVAR_NONE			(0x00000000u)	// can be set even when cheats are disabled, but is not archived
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
typedef struct cvar_s
{
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


typedef struct dma_s {
	int			channels;
	int			samples;				// mono samples in buffer
	int			submission_chunk;		// don't mix less than this #
	int			samplebits;
	int			speed;
	byte		*buffer;
} dma_t;

#define	MAX_QPATH			64		// max length of a quake game pathname
#ifdef PATH_MAX
#define MAX_OSPATH			PATH_MAX
#else
#define	MAX_OSPATH			256		// max length of a filesystem pathname
#endif

// the game guarantees that no string from the network will ever
// exceed MAX_STRING_CHARS
#define	MAX_STRING_CHARS	1024	// max length of a string passed to Cmd_TokenizeString

// Colors

#define Q_COLOR_ESCAPE	'^'
#define Q_COLOR_BITS 0xF // was 7

// you MUST have the last bit on here about colour strings being less than 7 or taiwanese strings register as colour!!!!
#define Q_IsColorString(p)	( p && *(p) == Q_COLOR_ESCAPE && *((p)+1) && *((p)+1) != Q_COLOR_ESCAPE && *((p)+1) <= '9' && *((p)+1) >= '0' )
// Correct version of the above for Q_StripColor
#define Q_IsColorStringExt(p)	((p) && *(p) == Q_COLOR_ESCAPE && *((p)+1) && *((p)+1) >= '0' && *((p)+1) <= '9') // ^[0-9]

#define COLOR_BLACK		'0'
#define COLOR_RED		'1'
#define COLOR_GREEN		'2'
#define COLOR_YELLOW	'3'
#define COLOR_BLUE		'4'
#define COLOR_CYAN		'5'
#define COLOR_MAGENTA	'6'
#define COLOR_WHITE		'7'
#define COLOR_ORANGE	'8'
#define COLOR_GREY		'9'
#define ColorIndex(c)	( ( (c) - '0' ) & Q_COLOR_BITS )

#define S_COLOR_BLACK	"^0"
#define S_COLOR_RED		"^1"
#define S_COLOR_GREEN	"^2"
#define S_COLOR_YELLOW	"^3"
#define S_COLOR_BLUE	"^4"
#define S_COLOR_CYAN	"^5"
#define S_COLOR_MAGENTA	"^6"
#define S_COLOR_WHITE	"^7"
#define S_COLOR_ORANGE	"^8"
#define S_COLOR_GREY	"^9"

// in order from highest priority to lowest
// if none of the catchers are active, bound key strings will be executed
#define KEYCATCH_CONSOLE		0x0001
#define	KEYCATCH_UI				0x0002
