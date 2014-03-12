#include "sys_public.h"
#include "forward/cl_public.h"

#include <cstring> // memset

enum
{
	MAX_QUED_EVENTS = 1 << 8, // = 256. Must be a power of 2 so we can mask the queue indices instead of doing modulo. [Gee, should we still worry about such things? -mrw]
	MASK_QUED_EVENTS = ( MAX_QUED_EVENTS - 1 )
};

sysEvent_t g_eventQue[MAX_QUED_EVENTS];
int g_eventHead = 0;
int g_eventTail = 0;

static void Sys_QueEvent( int time, sysEventType_t type, int value, int value2, int ptrLength, void *ptr )
{
	sysEvent_t	*ev;

	ev = &g_eventQue[ g_eventHead & MASK_QUED_EVENTS ];

	// bk000305 - was missing
	if( g_eventHead - g_eventTail >= MAX_QUED_EVENTS )
	{
		Com_Printf("Sys_QueEvent: overflow\n");
		// we are discarding an event, but don't leak memory
		if( ev->evPtr )
		{
			Z_Free( ev->evPtr );
		}
		g_eventTail++;
	}

	g_eventHead++;

	if( time == 0 )
	{
		time = Sys_Milliseconds( qfalse );
	}

	ev->evTime = time;
	ev->evType = type;
	ev->evValue = value;
	ev->evValue2 = value2;
	ev->evPtrLength = ptrLength;
	ev->evPtr = ptr;
}

sysEvent_t Sys_GetEvent( void )
{
	// return if we have data
	if( g_eventHead > g_eventTail )
	{
		return g_eventQue[ ( g_eventTail++ ) & MASK_QUED_EVENTS ];
	}
	// return if we have data now
	if( g_eventHead > g_eventTail )
	{
		return g_eventQue[ ( g_eventTail++ ) & MASK_QUED_EVENTS ];
	}

	// create an empty event to return
	sysEvent_t ev;
	memset( &ev, 0, sizeof( ev ) );
	ev.evTime = Sys_Milliseconds( qfalse );

	return ev;
}
