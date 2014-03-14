#pragma once

// Edit fields and command line history/completion

#define CONSOLE_PROMPT_CHAR ']'
#define	MAX_EDIT_LINE		256
#define COMMAND_HISTORY		32

typedef struct field_s {
	int		cursor;
	int		scroll;
	int		widthInChars;
	char	buffer[ MAX_EDIT_LINE ];
} field_t;
