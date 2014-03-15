#include "console/con_public.h"

#include "forward/cl_public.h"
#include "sys/sys_public.h"
#include "win32/resource.h"

#include <Windows.h>

extern int g_console_field_width;
extern cvar_t *com_viewlog;

enum {
	COPY_ID = 1,
	QUIT_ID,
	CLEAR_ID,

	ERRORBOX_ID = 10,
	ERRORTEXT_ID,

	EDIT_ID = 100,
	INPUT_ID
};

typedef struct WinConData_s {
	HWND		hWnd;
	HWND		hwndBuffer;

	HWND		hwndButtonClear;
	HWND		hwndButtonCopy;
	HWND		hwndButtonQuit;

	HWND		hwndErrorBox;
	HWND		hwndErrorText;

	HBITMAP		hbmLogo;
	HBITMAP		hbmClearBitmap;

	HBRUSH		hbrEditBackground;
	HBRUSH		hbrErrorBackground;

	HFONT		hfBufferFont;
	HFONT		hfButtonFont;

	HWND		hwndInputLine;

	char		errorString[ 80 ];

	char		consoleText[ 512 ], returnedText[ 512 ];
	int			visLevel;
	qboolean	quitOnClose;
	int			windowWidth, windowHeight;

	WNDPROC		SysInputLineWndProc;

	// console
	field_t		g_consoleField;
	int			nextHistoryLine;	// the last line in the history buffer, not masked
	int			historyLine;		// the line being displayed from history buffer will be <= nextHistoryLine
	field_t		historyEditLines[ COMMAND_HISTORY ];

} WinConData;

static WinConData s_wcd;

#define	MAX_VA_STRING	32000
#define MAX_VA_BUFFERS 4
static char * QDECL con_va( const char *format, ... )
{
	va_list		argptr;
	static char	string[ MAX_VA_BUFFERS ][ MAX_VA_STRING ];	// in case va is called by nested functions
	static int	index = 0;
	char		*buf;

	va_start( argptr, format );
	buf = ( char * )&string[ index++ & 3 ];
	Q_vsnprintf( buf, sizeof( *string ), format, argptr );
	va_end( argptr );
	return buf;
}

static void Conbuf_AppendText( const char *pMsg )
{
#define CONSOLE_BUFFER_SIZE		16384
	if( !s_wcd.hWnd ) {
		return;
	}
	char buffer[ CONSOLE_BUFFER_SIZE * 4 ];
	char *b = buffer;
	const char *msg;
	int bufLen;
	int i = 0;
	static unsigned long s_totalChars;

	//
	// if the message is REALLY long, use just the last portion of it
	//
	if( strlen( pMsg ) > CONSOLE_BUFFER_SIZE - 1 )
	{
		msg = pMsg + strlen( pMsg ) - CONSOLE_BUFFER_SIZE + 1;
	}
	else
	{
		msg = pMsg;
	}

	//
	// copy into an intermediate buffer
	//
	while( msg[ i ] && ( ( b - buffer ) < sizeof( buffer )-1 ) )
	{
		if( msg[ i ] == '\n' && msg[ i + 1 ] == '\r' )
		{
			b[ 0 ] = '\r';
			b[ 1 ] = '\n';
			b += 2;
			i++;
		}
		else if( msg[ i ] == '\r' )
		{
			b[ 0 ] = '\r';
			b[ 1 ] = '\n';
			b += 2;
		}
		else if( msg[ i ] == '\n' )
		{
			b[ 0 ] = '\r';
			b[ 1 ] = '\n';
			b += 2;
		}
		else if( Q_IsColorStringExt( &msg[ i ] ) )
		{
			i++;
		}
		else
		{
			*b = msg[ i ];
			b++;
		}
		i++;
	}
	*b = 0;
	bufLen = b - buffer;

	s_totalChars += bufLen;

	//
	// replace selection instead of appending if we're overflowing
	//
	if( s_totalChars > 0x7fff )
	{
		SendMessage( s_wcd.hwndBuffer, EM_SETSEL, 0, -1 );
		s_totalChars = bufLen;
	}

	//
	// put this text into the windows console
	//
	SendMessage( s_wcd.hwndBuffer, EM_LINESCROLL, 0, 0xffff );
	SendMessage( s_wcd.hwndBuffer, EM_SCROLLCARET, 0, 0 );
	SendMessage( s_wcd.hwndBuffer, EM_REPLACESEL, 0, ( LPARAM )buffer );
}

static LRESULT CALLBACK ConWndProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	const char *cmdString;
	static qboolean s_timePolarity;

	switch( uMsg )
	{
	case WM_ACTIVATE:
		if( LOWORD( wParam ) != WA_INACTIVE )
		{
			SetFocus( s_wcd.hwndInputLine );
		}

		if( com_viewlog && !Cvar_VariableIntegerValue( "com_dedicated" ) )
		{
			// if the viewlog is open, check to see if it's being minimized
			if( com_viewlog->integer == 1 )
			{
				if( HIWORD( wParam ) )		// minimized flag
				{
					Cvar_Set( "viewlog", "2" );
				}
			}
			else if( com_viewlog->integer == 2 )
			{
				if( !HIWORD( wParam ) )		// minimized flag
				{
					Cvar_Set( "viewlog", "1" );
				}
			}
		}
		break;

	case WM_CLOSE:
		if( Cvar_VariableIntegerValue( "com_dedicated" ) )
		{
			cmdString = CopyString( "quit" );
			Sys_QueEvent( 0, SE_CONSOLE, 0, 0, strlen( cmdString ) + 1, ( void * )cmdString );
		}
		else if( s_wcd.quitOnClose )
		{
			PostQuitMessage( 0 );
		}
		else
		{
			Con_ShowConsole( 0, qfalse );
			Cvar_Set( "viewlog", "0" );
		}
		return 0;
	case WM_CTLCOLORSTATIC:
		if( ( HWND )lParam == s_wcd.hwndBuffer )
		{
			SetBkColor( ( HDC )wParam, RGB( 0, 0, 0 ) );
			SetTextColor( ( HDC )wParam, RGB( 249, 249, 000 ) );
			return ( long )s_wcd.hbrEditBackground;
		}
		else if( ( HWND )lParam == s_wcd.hwndErrorBox )
		{
			if( s_timePolarity & 1 )
			{
				SetBkColor( ( HDC )wParam, RGB( 0x80, 0x80, 0x80 ) );
				SetTextColor( ( HDC )wParam, RGB( 0xff, 0x00, 0x00 ) );
			}
			else
			{
				SetBkColor( ( HDC )wParam, RGB( 0x80, 0x80, 0x80 ) );
				SetTextColor( ( HDC )wParam, RGB( 0x00, 0x00, 0x00 ) );
			}
			return ( long )s_wcd.hbrErrorBackground;
		}
		return FALSE;
		break;

	case WM_COMMAND:
		if( wParam == COPY_ID )
		{
			SendMessage( s_wcd.hwndBuffer, EM_SETSEL, 0, -1 );
			SendMessage( s_wcd.hwndBuffer, WM_COPY, 0, 0 );
		}
		else if( wParam == QUIT_ID )
		{
			if( s_wcd.quitOnClose )
			{
				PostQuitMessage( 0 );
			}
			else
			{
				cmdString = CopyString( "quit" );
				Sys_QueEvent( 0, SE_CONSOLE, 0, 0, strlen( cmdString ) + 1, ( void * )cmdString );
			}
		}
		else if( wParam == CLEAR_ID )
		{
			SendMessage( s_wcd.hwndBuffer, EM_SETSEL, 0, -1 );
			SendMessage( s_wcd.hwndBuffer, EM_REPLACESEL, FALSE, ( LPARAM ) "" );
			UpdateWindow( s_wcd.hwndBuffer );
		}
		break;
	case WM_CREATE:
		s_wcd.hbrEditBackground = CreateSolidBrush( RGB( 0x00, 0x00, 0x00 ) );
		s_wcd.hbrErrorBackground = CreateSolidBrush( RGB( 0x80, 0x80, 0x80 ) );
		SetTimer( hWnd, 1, 1000, NULL );
		break;
	case WM_ERASEBKGND:
		return DefWindowProc( hWnd, uMsg, wParam, lParam );
	case WM_TIMER:
		if( wParam == 1 )
		{
			s_timePolarity = ( qboolean )!s_timePolarity;
			if( s_wcd.hwndErrorBox )
			{
				InvalidateRect( s_wcd.hwndErrorBox, NULL, FALSE );
			}
		}
		break;
	}

	return DefWindowProc( hWnd, uMsg, wParam, lParam );
}

static LRESULT CALLBACK InputLineWndProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	switch( uMsg )
	{
	case WM_KILLFOCUS:
		if( ( HWND )wParam == s_wcd.hWnd ||
			( HWND )wParam == s_wcd.hwndErrorBox )
		{
			SetFocus( hWnd );
			return 0;
		}
		break;

	case WM_CHAR:
		GetWindowText( s_wcd.hwndInputLine, s_wcd.g_consoleField.buffer, sizeof( s_wcd.g_consoleField.buffer ) );
		SendMessage( s_wcd.hwndInputLine, EM_GETSEL, ( WPARAM )NULL, ( LPARAM )&s_wcd.g_consoleField.cursor );

		if( wParam == VK_RETURN )
		{
			strncat( s_wcd.consoleText, s_wcd.g_consoleField.buffer, sizeof( s_wcd.consoleText ) - strlen( s_wcd.consoleText ) - 5 );
			strcat( s_wcd.consoleText, "\n" );
			SetWindowText( s_wcd.hwndInputLine, "" );

			Con_Print( con_va( "%c%s\n", CONSOLE_PROMPT_CHAR, s_wcd.g_consoleField.buffer ) );

			// empty lines just scroll the console without adding to history
			if( !s_wcd.g_consoleField.buffer[ 0 ] )
				return 0;

			// copy line to history buffer
			s_wcd.historyEditLines[ s_wcd.nextHistoryLine % COMMAND_HISTORY ] = s_wcd.g_consoleField;
			s_wcd.nextHistoryLine++;
			s_wcd.historyLine = s_wcd.nextHistoryLine;
			Field_Clear( &s_wcd.g_consoleField );
			s_wcd.g_consoleField.widthInChars = g_console_field_width;

			return 0;
		}

		if( wParam == VK_TAB )
		{
			Field_AutoComplete( &s_wcd.g_consoleField );
			SetWindowText( s_wcd.hwndInputLine, s_wcd.g_consoleField.buffer );
			SendMessage( s_wcd.hwndInputLine, EM_SETSEL, s_wcd.g_consoleField.cursor, s_wcd.g_consoleField.cursor );
			return 0;
		}
		break;
	case WM_KEYDOWN:
		// history scrolling
		if( wParam == VK_UP )
		{// scroll up: arrow-up
			if( s_wcd.nextHistoryLine - s_wcd.historyLine < COMMAND_HISTORY && s_wcd.historyLine > 0 )
				s_wcd.historyLine--;
			s_wcd.g_consoleField = s_wcd.historyEditLines[ s_wcd.historyLine % COMMAND_HISTORY ];
			SetWindowText( s_wcd.hwndInputLine, s_wcd.g_consoleField.buffer );
			SendMessage( s_wcd.hwndInputLine, EM_SETSEL, s_wcd.g_consoleField.cursor, s_wcd.g_consoleField.cursor );

			return 0;
		}

		if( wParam == VK_DOWN )
		{// scroll down: arrow-down
			s_wcd.historyLine++;
			if( s_wcd.historyLine >= s_wcd.nextHistoryLine ) {
				s_wcd.historyLine = s_wcd.nextHistoryLine;
				Field_Clear( &s_wcd.g_consoleField );
				s_wcd.g_consoleField.widthInChars = g_console_field_width;
				SetWindowText( s_wcd.hwndInputLine, s_wcd.g_consoleField.buffer );
				SendMessage( s_wcd.hwndInputLine, EM_SETSEL, s_wcd.g_consoleField.cursor, s_wcd.g_consoleField.cursor );
				return 0;
			}
			s_wcd.g_consoleField = s_wcd.historyEditLines[ s_wcd.historyLine % COMMAND_HISTORY ];
			SetWindowText( s_wcd.hwndInputLine, s_wcd.g_consoleField.buffer );
			SendMessage( s_wcd.hwndInputLine, EM_SETSEL, s_wcd.g_consoleField.cursor, s_wcd.g_consoleField.cursor );

			return 0;
		}
		break;
	}

	return CallWindowProc( s_wcd.SysInputLineWndProc, hWnd, uMsg, wParam, lParam );
}

void Con_CreateConsole( const char *title )
{
	// This would usually be the hInstance passed into WinMain(), but since we don't have that...
	HINSTANCE hInstance = GetModuleHandle( NULL );

	HDC hDC;
	WNDCLASS wc;
	RECT rect;
	const char *DEDCLASS = "OpenJK WinConsole";
	int nHeight;
	int swidth, sheight;
	int DEDSTYLE = WS_POPUPWINDOW | WS_CAPTION | WS_MINIMIZEBOX;

	memset( &wc, 0, sizeof( wc ) );

	wc.style = 0;
	wc.lpfnWndProc = ( WNDPROC )ConWndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = hInstance;
	wc.hIcon = LoadIcon( hInstance, MAKEINTRESOURCE( IDI_ICON1 ) );
	wc.hCursor = LoadCursor( NULL, IDC_ARROW );
	wc.hbrBackground = ( HBRUSH__ * )COLOR_INACTIVEBORDER;
	wc.lpszMenuName = 0;
	wc.lpszClassName = DEDCLASS;

	if( !RegisterClass( &wc ) )	{
		return;
	}

	rect.left = 0;
	rect.right = 600;
	rect.top = 0;
	rect.bottom = 450;
	AdjustWindowRect( &rect, DEDSTYLE, FALSE );

	hDC = GetDC( GetDesktopWindow( ) );
	swidth = GetDeviceCaps( hDC, HORZRES );
	sheight = GetDeviceCaps( hDC, VERTRES );
	ReleaseDC( GetDesktopWindow( ), hDC );

	s_wcd.windowWidth = rect.right - rect.left + 1;
	s_wcd.windowHeight = rect.bottom - rect.top + 1;

	s_wcd.hWnd = CreateWindowEx( 0,
		DEDCLASS,
		title,
		DEDSTYLE,
		( swidth - 600 ) / 2, ( sheight - 450 ) / 2, rect.right - rect.left + 1, rect.bottom - rect.top + 1,
		NULL,
		NULL,
		hInstance,
		NULL );

	if( s_wcd.hWnd == NULL )
	{
		return;
	}

	//
	// create fonts
	//
	hDC = GetDC( s_wcd.hWnd );
	nHeight = -MulDiv( 8, GetDeviceCaps( hDC, LOGPIXELSY ), 72 );

	s_wcd.hfBufferFont = CreateFont( nHeight,
		0,
		0,
		0,
		FW_LIGHT,
		0,
		0,
		0,
		DEFAULT_CHARSET,
		OUT_DEFAULT_PRECIS,
		CLIP_DEFAULT_PRECIS,
		DEFAULT_QUALITY,
		FF_MODERN | FIXED_PITCH,
		"Courier New" );

	ReleaseDC( s_wcd.hWnd, hDC );

	//
	// create the input line
	//
	s_wcd.hwndInputLine = CreateWindow( "edit", NULL, WS_CHILD | WS_VISIBLE | WS_BORDER |
		ES_LEFT | ES_AUTOHSCROLL | WS_TABSTOP,
		6, 400, s_wcd.windowWidth - 20, 20,
		s_wcd.hWnd,
		( HMENU )INPUT_ID,	// child window ID
		hInstance, NULL );

	//
	// create the buttons
	//
	s_wcd.hwndButtonCopy = CreateWindow( "button", NULL, BS_PUSHBUTTON | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON | WS_TABSTOP,
		5, 425, 72, 24,
		s_wcd.hWnd,
		( HMENU )COPY_ID,	// child window ID
		hInstance, NULL );
	SendMessage( s_wcd.hwndButtonCopy, WM_SETTEXT, 0, ( LPARAM ) "Copy" );

	s_wcd.hwndButtonClear = CreateWindow( "button", NULL, BS_PUSHBUTTON | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON | WS_TABSTOP,
		82, 425, 72, 24,
		s_wcd.hWnd,
		( HMENU )CLEAR_ID,	// child window ID
		hInstance, NULL );
	SendMessage( s_wcd.hwndButtonClear, WM_SETTEXT, 0, ( LPARAM ) "Clear" );

	s_wcd.hwndButtonQuit = CreateWindow( "button", NULL, BS_PUSHBUTTON | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON | WS_TABSTOP,
		s_wcd.windowWidth - 92, 425, 72, 24,
		s_wcd.hWnd,
		( HMENU )QUIT_ID,	// child window ID
		hInstance, NULL );
	SendMessage( s_wcd.hwndButtonQuit, WM_SETTEXT, 0, ( LPARAM ) "Quit" );


	//
	// create the scrollbuffer
	//
	s_wcd.hwndBuffer = CreateWindow( "edit", NULL, WS_CHILD | WS_VISIBLE | WS_VSCROLL | WS_BORDER |
		ES_LEFT | ES_MULTILINE | ES_AUTOVSCROLL | ES_READONLY | WS_TABSTOP,
		6, 40, s_wcd.windowWidth - 20, 354,
		s_wcd.hWnd,
		( HMENU )EDIT_ID,	// child window ID
		hInstance, NULL );
	SendMessage( s_wcd.hwndBuffer, WM_SETFONT, ( WPARAM )s_wcd.hfBufferFont, 0 );

	s_wcd.SysInputLineWndProc = ( WNDPROC )SetWindowLongPtr( s_wcd.hwndInputLine, GWLP_WNDPROC, ( long )InputLineWndProc );
	SendMessage( s_wcd.hwndInputLine, WM_SETFONT, ( WPARAM )s_wcd.hfBufferFont, 0 );
	SendMessage( s_wcd.hwndBuffer, EM_LIMITTEXT, ( WPARAM )0x7fff, 0 );

	ShowWindow( s_wcd.hWnd, SW_SHOWDEFAULT );
	UpdateWindow( s_wcd.hWnd );
	SetForegroundWindow( s_wcd.hWnd );
	SetFocus( s_wcd.hwndInputLine );

	Field_Clear( &s_wcd.g_consoleField );
	s_wcd.g_consoleField.widthInChars = g_console_field_width;
	for( int i = 0; i < COMMAND_HISTORY; i++ ) {
		Field_Clear( &s_wcd.historyEditLines[ i ] );
		s_wcd.historyEditLines[ i ].widthInChars = g_console_field_width;
	}

	s_wcd.visLevel = 1;
}

void Con_ShowConsole( int visLevel, qboolean quitOnClose )
{
	s_wcd.quitOnClose = quitOnClose;

	if( visLevel == s_wcd.visLevel )
	{
		return;
	}

	s_wcd.visLevel = visLevel;

	if( !s_wcd.hWnd )
	{
		return;
	}

	switch( visLevel )
	{
	case 0:
		ShowWindow( s_wcd.hWnd, SW_HIDE );
		break;
	case 1:
		ShowWindow( s_wcd.hWnd, SW_SHOWNORMAL );
		SendMessage( s_wcd.hwndBuffer, EM_LINESCROLL, 0, 0xffff );
		break;
	case 2:
		ShowWindow( s_wcd.hWnd, SW_MINIMIZE );
		break;
	default:
		Sys_Error( "Invalid visLevel %d sent to Sys_ShowConsole\n", visLevel );
		break;
	}
}

void Con_Print( const char *message )
{
	// TTimo - prefix for text that shows up in console but not in notify
	// backported from RTCW
	if( !Q_strncmp( message, "[skipnotify]", 12 ) ) {
		message += 12;
	}
	if( message[ 0 ] == '*' ) {
		message += 1;
	}
	Conbuf_AppendText( message );
}

static void Con_SetErrorText( const char *message )
{
	// This would usually be the hInstance passed into WinMain(), but since we don't have that...
	HINSTANCE hInstance = GetModuleHandle( NULL );

	Q_strncpyz( s_wcd.errorString, message, sizeof( s_wcd.errorString ) );

	if( !s_wcd.hwndErrorBox )
	{
		s_wcd.hwndErrorBox = CreateWindow( "static", NULL, WS_CHILD | WS_VISIBLE | SS_SUNKEN,
			6, 5, s_wcd.windowWidth - 20, 30,
			s_wcd.hWnd,
			( HMENU )ERRORBOX_ID,	// child window ID
			hInstance, NULL );
		SendMessage( s_wcd.hwndErrorBox, WM_SETFONT, ( WPARAM )s_wcd.hfBufferFont, 0 );
		SetWindowText( s_wcd.hwndErrorBox, s_wcd.errorString );

		DestroyWindow( s_wcd.hwndInputLine );
		s_wcd.hwndInputLine = NULL;
	}
}

static qboolean s_showingError = qfalse;

void Con_ShowError( const char *message )
{
	Conbuf_AppendText( message );
	Conbuf_AppendText( "\n" );
	
	Con_SetErrorText( message );
	Con_ShowConsole( 1, qtrue );

	// Not waiting here so Com_Shutdown() can commence, which will in turn call Con_DestroyConsole(), where we'll wait.
	s_showingError = qtrue;
}

void Con_Frame( void )
{
	if ( s_wcd.consoleText[0] != 0 )
	{
		int len = strlen( s_wcd.consoleText ) + 1;
		char *buf = (char *)Z_Malloc( len, TAG_EVENT );
		Q_strncpyz( buf, s_wcd.consoleText, len );
		Sys_QueEvent( 0, SE_CONSOLE, 0, 0, len, buf );
		s_wcd.consoleText[0] = 0;
	}
}

void Con_DestroyConsole( void )
{
	if( s_showingError )
	{
		// We're still showing an error, wait for user to quit.
		MSG msg;
		while( GetMessage( &msg, NULL, 0, 0 ) ) {
			TranslateMessage( &msg );
			DispatchMessage( &msg );
		}
		s_showingError = qfalse;
	}

	if( s_wcd.hWnd )
	{
		DeleteObject( s_wcd.hbrEditBackground );
		DeleteObject( s_wcd.hbrErrorBackground );
		DeleteObject( s_wcd.hfBufferFont );

		ShowWindow( s_wcd.hWnd, SW_HIDE );
		CloseWindow( s_wcd.hWnd );
		DestroyWindow( s_wcd.hWnd );
		s_wcd.hWnd = 0;
	}
}
