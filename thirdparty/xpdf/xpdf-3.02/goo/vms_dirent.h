/*	@(#)dirent.h 1.7 89/06/25 SMI 	*/

/*
 * Filesystem-independent directory information.
 */

#ifndef	__dirent_h
#define	__dirent_h

#ifdef __cplusplus
extern "C" {
#endif

/* Make sure we don't get the V7 RTL dirent functions. These are broken. */

#ifndef __CRTL_VER
#   define __CRTL_VER __VMS_VER
#endif
#if __CRTL_VER >= 70000000
#include <dirent.h>
#endif

#include <types.h>

#define opendir   goo_opendir
#define readdir   goo_readdir
#define closedir  goo_closedir
#define seekdir   goo_seekdir
#define telldir   goo_telldir
#define rewinddir goo_rewindir
#define DIR       GOO_DIR

#ifndef	_POSIX_SOURCE
#define	d_ino	d_fileno	/* compatability */
#ifndef	NULL
#define	NULL	0
#endif
#endif	/* !_POSIX_SOURCE */

/*
 * Definitions for library routines operating on directories.
 */
typedef	struct __dirdesc {
	unsigned long dd_fd;	/* file descriptor */
	long	dd_loc;		/* buf offset of entry from last readddir() */
	long	dd_size;	/* amount of valid data in buffer */
	long	dd_bsize;	/* amount of entries read at a time */
	long	dd_off;		/* Current offset in dir (for telldir) */
	char	*dd_buf;	/* directory data buffer */
} DIR;

#include "vms_sys_dirent.h"

extern	DIR *opendir(char *dirname);
extern	struct dirent *readdir(DIR *dirp);
extern	void closedir(DIR *dirp);
#ifndef	_POSIX_SOURCE
extern	void seekdir(DIR *dirp, int loc);
extern	long telldir(DIR *dirp);
#endif	/* POSIX_SOURCE */
extern	void rewinddir(DIR *dirp);

#ifdef __cplusplus
}
#endif

#endif	/* !__dirent_h */
