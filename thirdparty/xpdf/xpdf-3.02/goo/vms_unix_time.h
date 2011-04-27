/*	@(#)time.h 2.9 87/01/17 SMI; from UCB 7.1 6/4/86	*/

/*
	Definitions of various structures used on UNIX for
	time-related syscalls.
*/

/*
 * Copyright (c) 1982, 1986 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */

#ifndef _UNIX_TIME_
#define _UNIX_TIME_

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Structure returned by gettimeofday(2) system call,
 * and used in other calls.
 */
#ifndef __DECC
struct timeval
{
	long	tv_sec;		/* seconds */
	long	tv_usec;	/* and microseconds */
};
#else
#if (__DECC_VER < 50200000) && (__VMS_VER < 70000000)
struct timeval
{
	long	tv_sec;		/* seconds */
	long	tv_usec;	/* and microseconds */
};
#endif /* __DECC_VER */
#endif /* __DECC */
struct timezone
{
	int	tz_minuteswest;	/* minutes west of Greenwich */
	int	tz_dsttime;	/* type of dst correction */
};

#define	DST_NONE	0	/* not on dst */
#define	DST_USA		1	/* USA style dst */
#define	DST_AUST	2	/* Australian style dst */
#define	DST_WET		3	/* Western European dst */
#define	DST_MET		4	/* Middle European dst */
#define	DST_EET		5	/* Eastern European dst */
#define	DST_CAN		6	/* Canada */
#define	DST_GB		7	/* Great Britain and Eire */
#define	DST_RUM		8	/* Rumania */
#define	DST_TUR		9	/* Turkey */
#define	DST_AUSTALT	10	/* Australian style with shift in 1986 */

/*
 * Operations on timevals.
 *
 * NB: timercmp does not work for >= or <=.
 */
#define	timerisset(tvp)		((tvp)->tv_sec || (tvp)->tv_usec)
#define	timercmp(tvp, uvp, cmp)	\
	((tvp)->tv_sec cmp (uvp)->tv_sec || \
	 (tvp)->tv_sec == (uvp)->tv_sec && (tvp)->tv_usec cmp (uvp)->tv_usec)
#define	timerclear(tvp)		(tvp)->tv_sec = (tvp)->tv_usec = 0

/*
 * Names of the interval timers, and structure
 * defining a timer setting.
 */
#define	ITIMER_REAL	0
#define	ITIMER_VIRTUAL	1
#define	ITIMER_PROF	2

#ifndef __DECC
struct	itimerval
{
	struct	timeval it_interval;	/* timer interval */
	struct	timeval it_value;	/* current value */
};
#else
#if (__DECC_VER < 50200000) && (__VMS_VER < 70000000)
struct	itimerval
{
	struct	timeval it_interval;	/* timer interval */
	struct	timeval it_value;	/* current value */
};
#endif /* __DECC_VER */
#endif /* __DECC */

#ifndef KERNEL
#include <time.h>
#endif

#ifdef __cplusplus
}
#endif

#endif /*!_UNIX_TIME_*/

