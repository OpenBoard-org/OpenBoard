/*
 * vms_unlink.c
 *
 * A UNIX-style unlink() function for VMS.
 *
 * Thanks to Patrick Moreau (pmoreau@cena.dgac.fr).
 */

#include <descrip.h>
#include <string.h>
#include <lib$routines.h>

int unlink(char *filename) {
  static struct dsc$descriptor_s file_desc;

  file_desc.dsc$w_length = strlen(filename);
  file_desc.dsc$b_dtype  = DSC$K_DTYPE_T;
  file_desc.dsc$b_class  = DSC$K_CLASS_S;
  file_desc.dsc$a_pointer= filename;

  return (lib$delete_file(&file_desc));
}
