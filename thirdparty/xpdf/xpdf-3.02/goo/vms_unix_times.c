/*
 *	UNIX-style Time Functions
 *
 */
#include <stdio.h>
#include <signal.h>
#include <time.h>
#include "vms_unix_time.h"

/*
 *	gettimeofday(2) - Returns the current time
 *
 *	NOTE: The timezone portion is useless on VMS.
 *	Even on UNIX, it is only provided for backwards
 *	compatibilty and is not guaranteed to be correct.
 */

#if (__VMS_VER < 70000000)
int gettimeofday(tv, tz)
struct timeval  *tv;
struct timezone *tz;
{
    timeb_t tmp_time;

    ftime(&tmp_time);

    if (tv != NULL)
    {
	tv->tv_sec  = tmp_time.time;
	tv->tv_usec = tmp_time.millitm * 1000;
    }

    if (tz != NULL)
    {
	tz->tz_minuteswest = tmp_time.timezone;
	tz->tz_dsttime = tmp_time.dstflag;
    }

    return (0);

} /*** End gettimeofday() ***/
#endif
