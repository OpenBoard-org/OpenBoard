/*
 *	DIRECTORY.C - VMS emulation routines for UNIX Directory
 *	              callable routines
 *
 *	Author:		Patrick L. Mahan
 *	Location:	TGV, Inc
 *	Date:		19-November-1991
 *
 *	Purpose:	Provides emulation of the BSD directory routines
 *			which are used by some of the X11 R4 release
 *			software.
 *
 *	Side effects:	This is only a partial emulation.  Not all of
 *			the required information is passed to the user.
 *
 *	Modification History
 *
 *	Date        | Who	| Version	| History
 *	------------+-----------+---------------+----------------------------
 *	19-Nov-1991 | PLM	| 1.0		| First Write
 *	20-Apr-1992 | PLM	| 1.1		| Added validation check for
 *	            |		|		| for the directory
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <rmsdef.h>
#include <descrip.h>
#include <lib$routines.h>
#include "vms_dirent.h"

#define	NOWILD		0x00000001
#define MULTIPLE	0x00000002

static unsigned long context = 0;

static struct dsc$descriptor_s *create_descriptor ( name )
char *name;
{
   struct dsc$descriptor_s *retdescrip;

   retdescrip = (struct dsc$descriptor_s *)calloc(1, sizeof(struct dsc$descriptor_s));

   if (retdescrip == NULL) return ((struct dsc$descriptor_s *)NULL);

   retdescrip->dsc$b_dtype	= DSC$K_DTYPE_T;
   retdescrip->dsc$b_class	= DSC$K_CLASS_S;
   retdescrip->dsc$w_length	= strlen(name);
   retdescrip->dsc$a_pointer	= name;

   return (retdescrip);
}

static int Check_Directory( dirname )
char *dirname;
{
	static char *tmpdir, *cp;
	FILE *tfp;
	int status;

	status = 1;

	tmpdir = calloc(strlen(dirname)+15,sizeof(char));

	strcpy(tmpdir, dirname);

	cp = strrchr(tmpdir, '.');

	if (cp != NULL) {
		*cp = ']';
		cp = strrchr(tmpdir, ']');
		*cp = '.';
		strcat(tmpdir, "dir");
	}
	else {
		char *tmp1;
		tmp1 = calloc(strlen(dirname)+1,sizeof(char));
		cp = strchr(tmpdir, '[');
		cp++;
		strcpy(tmp1, cp);
		cp = strrchr(tmp1, ']');
		*cp = '\0';
		cp = strchr(tmpdir, '[');
		cp++;
		*cp = '\0';
		strcat(tmpdir, "000000]");
		strcat(tmpdir, tmp1);
		strcat(tmpdir, ".dir");
	}

	tfp = fopen(tmpdir, "r");

	if (tfp == NULL) status = 0;

	fclose(tfp);

	return (status);
}

DIR *opendir( dirname )
char *dirname;
{
   DIR *retdir;
   struct dsc$descriptor_s filedescriptor;
   char *filepathname;

   retdir = (DIR *) calloc(1, sizeof(DIR));

   if (retdir == NULL) return ((DIR *)NULL);

   if (!Check_Directory(dirname)) return ((DIR *)NULL);

   filepathname = (char *)calloc(256, sizeof(char));

   strcpy(filepathname, dirname);
   strcat(filepathname, "*.*.*");

   retdir->dd_fd = (unsigned long) create_descriptor(filepathname);
   retdir->dd_loc = 0;
   retdir->dd_size = strlen(filepathname);
   retdir->dd_bsize = 0;
   retdir->dd_off = 0;
   retdir->dd_buf = filepathname;

   return (retdir);
}

struct dirent *readdir( dirp )
DIR *dirp;
{
   static struct dirent *retdirent;
   struct dsc$descriptor_s retfilenamedesc;
   struct dsc$descriptor_s searchpathdesc = *((struct dsc$descriptor_s *)dirp->dd_fd);
   char retfilename[256];
   char *sp;
   unsigned long istatus;
   unsigned long rms_status;
   unsigned long flags;

   retdirent = (struct dirent *)NULL;

   flags = MULTIPLE;

   retfilenamedesc.dsc$b_dtype	= DSC$K_DTYPE_T;
   retfilenamedesc.dsc$b_class	= DSC$K_CLASS_S;
   retfilenamedesc.dsc$w_length	= 255;
   retfilenamedesc.dsc$a_pointer= retfilename;

   istatus = lib$find_file (&searchpathdesc,
                            &retfilenamedesc,
                            &dirp->dd_loc,
                            0, 0,
                            &rms_status,
                            &flags);

   if (!(istatus & 1) && (istatus != RMS$_NMF) && (istatus != RMS$_FNF))
   {
      lib$signal (istatus);
      return (retdirent);
   }
   else if ((istatus == RMS$_NMF) || (istatus == RMS$_FNF))
      return (retdirent);

   retfilename[retfilenamedesc.dsc$w_length] = '\0';

   sp = strchr(retfilename, ' ');
   if (sp != NULL) *sp = '\0';

   sp = strrchr(retfilename, ']');
   if (sp != NULL)
      sp++;
   else
      sp = retfilename;

   retdirent = (struct dirent *)calloc(1, sizeof(struct dirent));

   strcpy(retdirent->d_name, sp);
   retdirent->d_namlen = strlen(sp);
   retdirent->d_fileno = 0;
   retdirent->d_off = 0;
   retdirent->d_reclen = DIRSIZ(retdirent);

   return (retdirent);
}

long telldir( dirp )
DIR *dirp;
{
   return(0);
}

void seekdir( dirp, loc )
DIR *dirp;
int loc;
{
   return;
}

void rewinddir( dirp )
DIR *dirp;
{
   lib$find_file_end (&dirp->dd_loc);
}

void closedir( dirp )
DIR *dirp;
{
   lib$find_file_end (&dirp->dd_loc);

   cfree ((void *) dirp->dd_fd);
   cfree (dirp->dd_buf);
   cfree (dirp);
}
