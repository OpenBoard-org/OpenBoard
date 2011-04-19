$!========================================================================
$!
$! Goo library compile script for VMS.
$!
$! Written by Patrick Moreau, Martin P.J. Zinser.
$!
$! Copyright 1996-2003 Glyph & Cog, LLC
$!
$!========================================================================
$!
$ GOO_CXXOBJS = "GString.obj,gmempp.obj,gfile.obj,ghash.obj,glist.obj"
$ GOO_CCOBJS  = "gmem.obj,parseargs.obj,vms_directory.obj,vms_unix_times.obj"
$!
$ if f$extract(1,3,f$getsyi("Version")) .lts. "7.0"
$  then
$   GOO_CCOBJS = GOO_CCOBJS + ",vms_unlink.obj"
$ endif
$!
$ i = 0
$ j = 0
$COMPILE_CXX_LOOP:
$ file = f$element(i, ",",GOO_CXXOBJS)
$ if file .eqs. "," then goto COMPILE_CC_LOOP
$ i = i + 1
$ name = f$parse(file,,,"NAME")
$ call make 'file "CXXCOMP ''name'.cc" - 
       'name'.cc
$ goto COMPILE_CXX_LOOP
$!
$COMPILE_CC_LOOP:
$ file = f$element(j, ",",GOO_CCOBJS)
$ if file .eqs. "," then goto COMPILE_END
$ j = j + 1
$ name = f$parse(file,,,"NAME")
$ call make 'file "CCOMP ''name'.c" - 
       'name'.c
$ goto COMPILE_CC_LOOP
$!
$COMPILE_END:
$ call make libgoo.olb "lib/cre libgoo.olb ''GOO_CXXOBJS',''GOO_CCOBJS'" *.obj
$!
$ exit
$!
$MAKE: SUBROUTINE   !SUBROUTINE TO CHECK DEPENDENCIES
$ V = 'F$Verify(0)
$! P1 = What we are trying to make
$! P2 = Command to make it
$! P3 - P8  What it depends on
$
$ If F$Search(P1) .Eqs. "" Then Goto Makeit
$ Time = F$CvTime(F$File(P1,"RDT"))
$arg=3
$Loop:
$       Argument = P'arg
$       If Argument .Eqs. "" Then Goto Exit
$       El=0
$Loop2:
$       File = F$Element(El," ",Argument)
$       If File .Eqs. " " Then Goto Endl
$       AFile = ""
$Loop3:
$       OFile = AFile
$       AFile = F$Search(File)
$       If AFile .Eqs. "" .Or. AFile .Eqs. OFile Then Goto NextEl
$       If F$CvTime(F$File(AFile,"RDT")) .Ges. Time Then Goto Makeit
$       Goto Loop3
$NextEL:
$       El = El + 1
$       Goto Loop2
$EndL:
$ arg=arg+1
$ If arg .Le. 8 Then Goto Loop
$ Goto Exit
$
$Makeit:
$ VV=F$VERIFY(0)
$ write sys$output P2
$ 'P2
$ VV='F$Verify(VV)
$Exit:
$ If V Then Set Verify
$ENDSUBROUTINE
