$!========================================================================
$!
$! Xpdf compile script for VMS.
$!
$! Written by Patrick Moreau, Martin P.J. Zinser.
$!
$! Copyright 1996-2003 Glyph & Cog, LLC
$!
$!========================================================================
$!
$ i = 0
$ j = 0
$ APPS = "XPDF,PDFTOPS,PDFTOTEXT,PDFINFO,PDFTOPBM,PDFIMAGES,PDFFONTS"
$ if f$search("COMMON.OLB").eqs."" then lib/create common.olb
$!
$ COMMON_OBJS = "Annot.obj,Array.obj,BuiltinFont.obj," + - 
                "BuiltinFontTables.obj,Catalog.obj,CharCodeToUnicode.obj," + - 
                "CMap.obj,Decrypt.obj,Dict.obj,Error.obj," + -
                "FontEncodingTables.obj,FontFile.obj," + -
                "Function.obj,Gfx.obj,GfxFont.obj,GfxState.obj,"+ - 
                "GlobalParams.obj,JArithmeticDecoder.obj,JBIG2Stream.obj,"+ -
                "Lexer.obj,Link.obj,NameToCharCode.obj,Object.obj,"+ -
                "Outline.obj,OutputDev.obj,Page.obj,Parser.obj,PDFdoc.obj," + -
                "PDFDocEncoding.obj,PSTokenizer.obj,Stream.obj," + -
                "UnicodeMap.obj,UnicodeTypeTable.obj,XRef.obj"
$ COMMON_LIBS = "[]common.olb/lib,[-.goo]libgoo.olb/lib"
$!
$ XPDF_OBJS = "xpdf.obj,FTFont.obj,PSOutputDev.obj," + -
              "SFont.obj,T1Font.obj,TextOutputDev.obj,TTFont.obj," + -
              "XOutputDev.obj,XPDFApp.obj,XPDFCore.obj,XPDFTree.obj," + -
              "XPDFViewer.obj,XPixmapOutputDev.obj"
$ XPDF_LIBS = ""
$!
$ PDFTOPS_OBJS   = "pdftops.obj,PSOutputDev.obj" 
$ PDFTOPS_LIBS   = ""
$!
$ PDFTOTEXT_OBJS = "pdftotext.obj,TextOutputDev.obj"
$ PDFTOTEXT_LIBS = ""
$!
$ PDFINFO_OBJS   =  "pdfinfo.obj"
$ PDFINFO_LIBS   = ""
$!
$ PDFTOPBM_OBJS  = "pdftopbm.obj,FTFont.obj,PBMOutputDev.obj,SFont.obj," + -
                   "T1Font.obj,TextOutputDev.obj,TTFont.obj,XOutputDev.obj"
$ PDFTOPBM_LIBS  = ""
$!
$ PDFIMAGES_OBJS = "pdfimages.obj,ImageOutputDev.obj"
$ PDFIMAGES_LIBS = ""
$!
$ PDFFONTS_OBJS  = "pdffonts.obj"
$ PDFFONTS_LIBS  = ""
$!
$COMPILE_CXX_LOOP:
$ file = f$element(i, ",",COMMON_OBJS)
$ if file .eqs. "," then goto BUILD_APPS
$ i = i + 1
$ name = f$parse(file,,,"NAME")
$ call make 'file "CXXCOMP ''name'.cc" - 
       'name'.cc
$ call make common.olb "lib/replace common.olb ''name'.obj" - 
       'name'.obj
$ goto COMPILE_CXX_LOOP
$!
$BUILD_APPS:
$ curr_app = f$element(j,",",APPS)
$ if curr_app .eqs. "," then exit
$ j = j + 1
$ i = 0
$COMPILE_APP:
$ file = f$element(i,",",'curr_app'_OBJS)
$ if file .eqs. "," then goto LINK_APP
$ i = i + 1
$ name = f$parse(file,,,"NAME")
$ call make 'file "CXXCOMP ''name'.cc" - 
       'name'.cc
$ goto COMPILE_APP
$LINK_APP:
$ if 'curr_app'_LIBS .nes. "" 
$ then 
$   LIBS = 'curr_app'_LIBS + "," + COMMON_LIBS
$ else
$   LIBS = COMMON_LIBS 
$ endif
$ OBJS = 'curr_app'_OBJS
$ write sys$output "Linking ''curr_app'..."
$ xpdf_link/exe='curr_app'.exe 'OBJS','libs',[-]xpdf.opt/opt
$!  
$ goto BUILD_APPS
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
