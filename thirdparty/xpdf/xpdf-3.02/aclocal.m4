# <<< smr.m4 from smr_macros 0.2.4 >>>

dnl ####################### -*- Mode: M4 -*- ###########################
dnl smr.m4 -- 
dnl 
dnl Copyright (C) 1999 Matthew D. Langston <langston@SLAC.Stanford.EDU>
dnl Copyright (C) 1998 Steve Robbins <stever@cs.mcgill.ca>
dnl
dnl This file is free software; you can redistribute it and/or modify it
dnl under the terms of the GNU General Public License as published by
dnl the Free Software Foundation; either version 2 of the License, or
dnl (at your option) any later version.
dnl
dnl This file is distributed in the hope that it will be useful, but
dnl WITHOUT ANY WARRANTY; without even the implied warranty of
dnl MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
dnl General Public License for more details.
dnl
dnl You should have received a copy of the GNU General Public License
dnl along with this file; if not, write to:
dnl
dnl   Free Software Foundation, Inc.
dnl   Suite 330
dnl   59 Temple Place
dnl   Boston, MA 02111-1307, USA.
dnl ####################################################################


dnl NOTE: The macros in this file are extensively documented in the
dnl       accompanying `smr_macros.texi' Texinfo file.  Please see the
dnl       Texinfo documentation for the definitive specification of how
dnl       these macros are supposed to work.  If the macros work
dnl       differently than the Texinfo documentation says they should,
dnl       then the macros (and not the Texinfo documentation) has the
dnl       bug(s).

dnl This is a convenient macro which translates illegal characters for
dnl bourne shell variables into legal characters.  It has the same
dnl functionality as sed 'y%./+-:%__p__%'.
AC_DEFUN([smr_safe_translation], [patsubst(patsubst([$1], [+], [p]), [./-:], [_])])

AC_DEFUN(smr_SWITCH,
[
  dnl Define convenient aliases for the arguments since there are so
  dnl many of them and I keep confusing myself whenever I have to edit
  dnl this macro.
  pushdef([smr_name],        $1)
  pushdef([smr_help_string], $2)
  pushdef([smr_default],     $3)
  pushdef([smr_yes_define],  $4)
  pushdef([smr_no_define],   $5)

  dnl Do some sanity checking of the arguments.
  ifelse([regexp(smr_default, [^\(yes\|no\)$])], -1, [AC_MSG_ERROR($0: third arg must be either yes or no)])

  dnl Create the help string
  pushdef([smr_lhs], [--ifelse(smr_default, yes, disable, enable)-smr_name])dnl
  pushdef([smr_rhs], [ifelse(smr_default, yes, disable, enable) smr_help_string (default is smr_default)])dnl

  dnl Add the option to `configure --help'.  We don't need to supply the
  dnl 4th argument to AC_ARG_ENABLE (i.e. the code to set the default
  dnl value) because that is done below by AC_CACHE_CHECK.
  AC_ARG_ENABLE([smr_name],
                AC_HELP_STRING([smr_lhs], [smr_rhs]),
                smr_cv_enable_[]smr_name=$enableval)

  dnl We cache the result so that the user doesn't have to remember
  dnl which flags they passed to `configure'.
  AC_CACHE_CHECK([whether to enable smr_help_string],
                 smr_cv_enable_[]smr_name,
                 smr_cv_enable_[]smr_name=smr_default)

    ifelse(smr_yes_define, , , test x"[$]smr_cv_enable_[]smr_name" = xyes && AC_DEFINE(smr_yes_define))
    ifelse(smr_no_define, , ,  test x"[$]smr_cv_enable_[]smr_name" = xno  && AC_DEFINE(smr_no_define))

  dnl Sanity check the value assigned to smr_cv_enable_$1 to force it to
  dnl be either `yes' or `no'.
  if test ! x"[$]smr_cv_enable_[]smr_name" = xyes; then
    if test ! x"[$]smr_cv_enable_[]smr_name" = xno; then
      AC_MSG_ERROR([smr_lhs must be either yes or no])
    fi
  fi

  popdef([smr_name])
  popdef([smr_help_string])
  popdef([smr_default])
  popdef([smr_yes_define])
  popdef([smr_no_define])
  popdef([smr_lhs])
  popdef([smr_rhs])
])


AC_DEFUN(smr_ARG_WITHLIB,
[
  dnl Define convenient aliases for the arguments since there are so
  dnl many of them and I keep confusing myself whenever I have to edit
  dnl this macro.
  pushdef([smr_name],        $1)
  pushdef([smr_libname],     ifelse($2, , smr_name, $2))
  pushdef([smr_help_string], $3)
  pushdef([smr_safe_name],   smr_safe_translation(smr_name))

  dnl Add the option to `configure --help'.  We don't need to supply the
  dnl 4th argument to AC_ARG_WITH (i.e. the code to set the default
  dnl value) because that is done below by AC_CACHE_CHECK.
  AC_ARG_WITH(smr_safe_name-library,
              AC_HELP_STRING([--with-smr_safe_name-library[[=PATH]]],
                             [use smr_name library ifelse(smr_help_string, , , (smr_help_string))]),
              smr_cv_with_[]smr_safe_name[]_library=$withval)

  dnl We cache the result so that the user doesn't have to remember
  dnl which flags they passed to `configure'.
  AC_CACHE_CHECK([whether to use smr_name library],
                 smr_cv_with_[]smr_safe_name[]_library,
                 smr_cv_with_[]smr_safe_name[]_library=maybe)


  case x"[$]smr_cv_with_[]smr_safe_name[]_library" in
      xyes | xmaybe)
          smr_safe_name[]_LIBS="-l[]smr_libname"
          with_[]smr_safe_name=[$]smr_cv_with_[]smr_safe_name[]_library
          ;;
      xno)
          smr_safe_name[]_LIBS=
          with_[]smr_safe_name=no
          ;;
      *)
          if test -f "[$]smr_cv_with_[]smr_safe_name[]_library"; then
            smr_safe_name[]_LIBS=[$]smr_cv_with_[]smr_safe_name[]_library
          elif test -d "[$]smr_cv_with_[]smr_safe_name[]_library"; then
            smr_safe_name[]_LIBS="-L[$]smr_cv_with_[]smr_safe_name[]_library -l[]smr_libname"
          else
            AC_MSG_ERROR([argument must be boolean, file, or directory])
          fi
          with_[]smr_safe_name=yes
          ;;
  esac

  popdef([smr_name])
  popdef([smr_libname])
  popdef([smr_help_string])
  popdef([smr_safe_name])
])


AC_DEFUN(smr_ARG_WITHINCLUDES,
[
  dnl Define convenient aliases for the arguments since there are so
  dnl many of them and I keep confusing myself whenever I have to edit
  dnl this macro.
  pushdef([smr_name],        $1)
  pushdef([smr_header],      $2)
  pushdef([smr_extra_flags], $3)
  pushdef([smr_safe_name],   smr_safe_translation(smr_name))

  dnl Add the option to `configure --help'.  We don't need to supply the
  dnl 4th argument to AC_ARG_WITH (i.e. the code to set the default
  dnl value) because that is done below by AC_CACHE_CHECK.
  AC_ARG_WITH(smr_safe_name-includes,
              AC_HELP_STRING([--with-smr_safe_name-includes[[=DIR]]],
                             [set directory for smr_name headers]),
              smr_cv_with_[]smr_safe_name[]_includes=$withval)

  dnl We cache the result so that the user doesn't have to remember
  dnl which flags they passed to `configure'.
  AC_CACHE_CHECK([where to find the smr_name header files],
                 smr_cv_with_[]smr_safe_name[]_includes,
                 smr_cv_with_[]smr_safe_name[]_includes=)

  if test ! x"[$]smr_cv_with_[]smr_safe_name[]_includes" = x; then
    if test -d "[$]smr_cv_with_[]smr_safe_name[]_includes"; then
      smr_safe_name[]_CFLAGS="-I[$]smr_cv_with_[]smr_safe_name[]_includes"
    else
      AC_MSG_ERROR([argument must be a directory])
    fi
  else
    smr_safe_name[]_CFLAGS=
  fi

  dnl This bit of logic comes from the autoconf AC_PROG_CC macro.  We
  dnl need to put the given include directory into CPPFLAGS temporarily,
  dnl but then restore CPPFLAGS to its old value.
  smr_test_CPPFLAGS="${CPPFLAGS+set}"
  smr_save_CPPFLAGS="$CPPFLAGS"
  CPPFLAGS="$CPPFLAGS [$]smr_safe_name[]_CFLAGS smr_extra_flags"

  dnl If the header file smr_header exists, then define
  dnl HAVE_[]smr_header (in all capitals).
  AC_CHECK_HEADERS([smr_header],
                   smr_have_[]smr_safe_name[]_header=yes,
                   smr_have_[]smr_safe_name[]_header=no)

  if test x"$smr_test_CPPFLAGS" = xset; then
    CPPFLAGS=$smr_save_CPPFLAGS
  else
    unset CPPFLAGS
  fi

  popdef([smr_name])
  popdef([smr_header])
  popdef([smr_extra_flags])
  popdef([smr_safe_name])
])


AC_DEFUN(smr_CHECK_LIB,
[
  dnl Define convenient aliases for the arguments since there are so
  dnl many of them and I keep confusing myself whenever I have to edit
  dnl this macro.
  pushdef([smr_name],        $1)
  pushdef([smr_libname],     ifelse($2, , smr_name, $2))
  pushdef([smr_help_string], $3)
  pushdef([smr_function],    $4)
  pushdef([smr_header],      $5)
  pushdef([smr_extra_libs],  $6)
  pushdef([smr_extra_flags], $7)
  pushdef([smr_prototype],   $8)
  pushdef([smr_safe_name],   smr_safe_translation(smr_name))

  dnl Give the user (via "configure --help") an interface to specify
  dnl whether we should use the library or not, and possibly where we
  dnl should find it.
  smr_ARG_WITHLIB([smr_name], [smr_libname], [smr_help_string])

  if test ! x"$with_[]smr_safe_name" = xno; then

    # If we got this far, then the user didn't explicitly ask not to use
    # the library.

    dnl If the caller of smr_CHECK_LIB specified a header file for this
    dnl library, then give the user (via "configure --help") an
    dnl interface to specify where this header file can be found (if it
    dnl isn't found by the compiler by default).
    ifelse(smr_header, , , [smr_ARG_WITHINCLUDES(smr_name, smr_header, smr_extra_flags)])

    # We need only look for the library if the header has been found
    # (or no header is needed).
    if test [$]smr_have_[]smr_safe_name[]_header != no; then

       AC_CHECK_LIB(smr_libname,
                    smr_function,
                    smr_have_[]smr_safe_name[]_library=yes,
                    smr_have_[]smr_safe_name[]_library=no,
                    [$]smr_safe_name[]_CFLAGS [smr_extra_flags] [$]smr_safe_name[]_LIBS [smr_extra_libs],
                    [ifelse(smr_prototype, , , [[#]include <smr_header>])],
                    smr_prototype)
    fi

    if test x"[$]smr_have_[]smr_safe_name[]_library" = xyes; then
      AC_MSG_RESULT([using smr_name library])
    else
      smr_safe_name[]_LIBS=
      smr_safe_name[]_CFLAGS=

      if test x"$with_[]smr_safe_name" = xmaybe; then
        AC_MSG_RESULT([not using smr_name library])
      else
        AC_MSG_WARN([requested smr_name library not found!])
      fi
    fi
  fi

  popdef([smr_name])
  popdef([smr_libname])
  popdef([smr_help_string])
  popdef([smr_function])
  popdef([smr_header])
  popdef([smr_extra_libs])
  popdef([smr_extra_flags])
  popdef([smr_prototype])
  popdef([smr_safe_name])
])
