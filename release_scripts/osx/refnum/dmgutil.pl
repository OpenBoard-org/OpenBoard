#!/usr/bin/perl -w
#============================================================================
#	NAME:
#		dmgutil.pl
#
#	DESCRIPTION:
#		Disk image creation utility.
#	
#	COPYRIGHT:
#		Copyright (c) 2006-2008, refNum Software
#		<http://www.refnum.com/>
#
#		All rights reserved.
#
#		Redistribution and use in source and binary forms, with or without
#		modification, are permitted provided that the following conditions
#		are met:
#
#			o Redistributions of source code must retain the above
#			copyright notice, this list of conditions and the following
#			disclaimer.
#
#			o Redistributions in binary form must reproduce the above
#			copyright notice, this list of conditions and the following
#			disclaimer in the documentation and/or other materials
#			provided with the distribution.
#
#			o Neither the name of refNum Software nor the names of its
#			contributors may be used to endorse or promote products derived
#			from this software without specific prior written permission.
#
#		THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
#		"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
#		LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
#		A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
#		OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
#		SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
#		LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
#		DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
#		THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
#		(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
#		OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#============================================================================
#		Imports
#----------------------------------------------------------------------------
use strict;
use Getopt::Long;





#============================================================================
#		Constants
#----------------------------------------------------------------------------
my $kLogging = "-quiet";

my $Rez     = "/Developer/Tools/Rez";
my $SetFile = "/Developer/Tools/SetFile";

my $kManPage = <<MANPAGE;
NAME
     dmgutil -- create, adjust, and compress a distribution disk image

SYNOPSIS
     dmgutil --help
     
     dmgutil --open --volume=name file
     
     dmgutil --set [--x=integer]        [--y=integer]
                   [--width=integer]    [--height=integer]
                   [--iconsize=integer] [--icon=file]
                   [--background=file]  [--bgcol=r,g,b]
                   [--toolbar=boolean]  file

     dmgutil --close --volume=name file

DESCRIPTION
    dmgutil is used to create distribution disk images, and to adjust
    the Finder view settings of these volumes or their contents.

    It can be invoked in three modes: open, set, and close.

   OPEN MODE
     Open Mode has the following options:
     --open                  Select open mode.

     --volume=name           The volume name for the disk image.

     file                    The output path for the .dmg file.

   SET MODE
     Set Mode has the following options:
     --set                   Select set mode.
     
     --x=integer             The x coordinate of the item.

     --y=integer             The y coordinate of the item.

     --width=integer         The width for a Finder window.

     --height=integer        The height for a Finder window.

     --iconsize=integer      The icon size for a Finder window.
     
     --icon=file.icns        The icon to apply to the item.
     
     --background=file       The background picture for a Finder window.
     
     --bgcol=r,g,b           The background color for a Finder window.
     
     --toolbar=boolean       The toolbar state for a Finder window.

     file                    The file or folder to be set.

   CLOSE MODE
     Close Mode has the following options:
     --close                 Select close mode.

     --volume=name           The volume name for the disk image.

     file                    The output path for the .dmg file.

EXAMPLES
   OPEN MODE
     To create a new disk image for "MyApp 1.0":
     
           dmgutil.pl --open --volume="MyApp 1.0" myapp_1.0.dmg

   SET MODE
     To set the position of a file or folder:
     
           dmgutil.pl --set --x=100 --y=100 "/Volumes/MyApp 1.0/Read Me.rtf"
           dmgutil.pl --set --x=200 --y=100 "/Volumes/MyApp 1.0/MyApp.app"

     To set the window size for the volume:

           dmgutil.pl --set --width=300 --height=200 "/Volumes/MyApp 1.0"

     To set the icon size for the volume:

           dmgutil.pl --set --iconsize=128 "/Volumes/MyApp 1.0"

     To set a custom icon for a volume, folder, or file:
     
           dmgutil.pl --set --icon=volume.icns "/Volumes/MyApp 1.0"
           dmgutil.pl --set --icon=folder.icns "/Volumes/MyApp 1.0/Extras"
           dmgutil.pl --set --icon=readme.icns "/Volumes/MyApp 1.0/Read Me.rtf"

     To set the background picture for the volume:

           dmgutil.pl --set --background=flowers.jpg "/Volumes/MyApp 1.0"

     To set the background color for the volume:

           dmgutil.pl --set --bgcol=0,65535,0 "/Volumes/MyApp 1.0"

     To hide the toolbar for the volume:

           dmgutil.pl --set --toolbar=false "/Volumes/MyApp 1.0"

     Multiple flags may be combined, to set all of the properties of an
     item simultaneously.

   CLOSE MODE
     To unmount and compress the disk image created for "MyApp 1.0":
     
           dmgutil.pl --close --volume="MyApp 1.0" myapp_1.0.dmg

VERSION
   dmgutil 1.1

COPYRIGHT
   Copyright (c) refNum Software                   http://www.refnum.com/
MANPAGE





#============================================================================
#		appleScript : Execute an AppleScript.
#----------------------------------------------------------------------------
sub appleScript
{


	# Retrieve our parameters
	my ($theScript) = @_;



	# Save the script
	my $theFile = "/tmp/dmgutil_applescript.txt";

	open( OUTPUT, ">$theFile") or die "Can't open $theFile for writing: $!\n";
	print OUTPUT $theScript;
	close(OUTPUT);



	# And execute it
	system("osascript", $theFile);
	unlink($theFile);
}





#============================================================================
#		isVolume : Is a path to the root of a volume?
#----------------------------------------------------------------------------
sub isVolume
{


	# Retrieve our parameters
	my ($thePath) = @_;



	# Check the state
	#
	# After stripping out the leading /Volumes, any further slashes
	# indicate we have a folder rather than a volume.
	$thePath =~ s/\/Volumes\///;

	my $isVolume = ($thePath =~ /.*\/.*/) ? 0 : 1;

	return($isVolume);
}





#============================================================================
#		setFolderState : Set the state for a folder.
#----------------------------------------------------------------------------
sub setFolderState
{


	# Retrieve our parameters
	my ($thePath, $iconSize, $flagToolbar, $bgImage, $bgColor) = @_;



	# Initialise ourselves
	my $cmdBackground = "";
	my $cmdIconSize   = "";
	my $cmdToolbar    = "";



	# Prepare the background
	#
	# As of 10.5, the Finder refuses to manipulate files whose names start with
	# a period (rdar://5582578). As such we need to use an underscore for the
	# image, then make it invisible using SetFile.
	if (-f $bgImage)
		{
		   $bgImage  =~ /.*\.(\w+)/;
		my $dstImage = "$thePath/_Background.$1";

		`cp "$bgImage" "$dstImage"`;

		$cmdBackground .= "set theImage to posix file \"$dstImage\"\n";
		$cmdBackground .= "    set background picture of theOptions to theImage\n";
		$cmdBackground .= "    do shell script \"/Developer/Tools/SetFile -a V '$dstImage'\"\n";
		}
	
	elsif ($bgColor ne "")
		{
		$cmdBackground = "set background color of theOptions to {$bgColor} as RGB color";
		}



	# Prepare the icon size
	if ($iconSize != 0)
		{
		$cmdIconSize = "set icon size of theOptions to $iconSize";
		}



	# Prepare the toolbar
	#
	# The window must be made visible in order to change the toolbar state.
	if ($flagToolbar ne "")
		{
		$cmdToolbar  = "    open theWindow                                   \n";
		$cmdToolbar .= "    set toolbar visible of theWindow to $flagToolbar \n";
		$cmdToolbar .= "    close theWindow                                  \n";
		}



	# Identify the target
	#
	# AppleScript requires the correct nomenclature for the target item.
	my $theTarget = "folder \"$thePath\"";
	
	if (isVolume($thePath))
		{
		$theTarget =~ s/folder "\/Volumes\//disk "/;
		}



	# Set the folder state
	#
	# Once a change has been made, it must be flushed to disk with update.
	my $theScript = "";
	
	$theScript .= "tell application \"Finder\"\n";
	$theScript .= "    set theTarget to $theTarget                       \n";
	$theScript .= "    set theWindow to window of theTarget              \n";
	$theScript .= "\n";
	$theScript .= "    set current view of theWindow to icon view        \n";
	$theScript .= "    set theOptions to icon view options of theWindow  \n";
	$theScript .= "    set arrangement of theOptions to not arranged     \n";
	$theScript .= "\n";
	$theScript .= "    $cmdBackground\n";
	$theScript .= "    $cmdIconSize  \n";
	$theScript .= "    $cmdToolbar   \n";
	$theScript .= "\n";
	$theScript .= "    update theTarget\n";
	$theScript .= "end tell";

	appleScript($theScript);
}





#============================================================================
#		setCustomIcon : Set a custom icon.
#----------------------------------------------------------------------------
sub setCustomIcon
{


	# Retrieve our parameters
	my ($thePath, $theIcon) = @_;



	# Validate our state
	#
	# We require several tools inside /Developer/Tools.
	die("Setting an icon requires $Rez")     if (! -e $Rez);
	die("Setting an icon requires $SetFile") if (! -e $SetFile);



	# Prepare the flags
	#
	# Prior to Mac OS X 10.4, SetFile can only set an attribute if it is
	# first cleared (rdar://3738867).
	my $sysVers   = `uname -r`;
	my $setHidden = ($sysVers =~ /^[0-7]\./) ? "vV" : "V";
	my $setIcon   = ($sysVers =~ /^[0-7]\./) ? "cC" : "C";



	# Set a volume icon
	#
	# Volume custom icons are contained in a .VolumeIcon.icns file.
	if (isVolume($thePath))
		{
		my $iconFile = "$thePath/.VolumeIcon.icns";

		`cp "$theIcon" "$iconFile"`;

		`$SetFile -a $setHidden "$iconFile"`;
		`$SetFile -a $setIcon   "$thePath"`;
		}


	# Set a folder icon
	#
	# Folder custom icons are contained in an ('icns', -16455) resource,
	# placed in an invisible "Icon\r" file inside the folder.
	elsif (-d $thePath)
		{
		my $iconFile = "$thePath/Icon\r";
		my $tmpR     = "/tmp/dmgutil.r";
		
		`echo "read 'icns' (-16455) \\"$theIcon\\";\n" > $tmpR`;
		`cd /tmp; $Rez dmgutil.r -append -o "$iconFile"`;

		`$SetFile -a $setHidden "$iconFile"`;
		`$SetFile -a $setIcon   "$thePath"`;
		
		unlink($tmpR);
		}


	# Set a file icon
	#
	# File custom icons are contained in an ('icns', -16455) resource.
	else
		{
		my $tmpR = "/tmp/dmgutil.r";
		
		`echo "read 'icns' (-16455) \\"$theIcon\\";\n" > $tmpR`;
		`cd /tmp; $Rez dmgutil.r -append -o "$thePath"`;

		`$SetFile -a $setIcon "$thePath"`;
		
		unlink($tmpR);
		}
}





#============================================================================
#		setWindowPos : Set the position of a window.
#----------------------------------------------------------------------------
sub setWindowPos
{


	# Retrieve our parameters
	my ($thePath, $posX, $posY, $theWidth, $theHeight) = @_;



	# Initialise ourselves
	my $bottom = $posY + $theHeight;
	my $right  = $posX + $theWidth;



	# Identify the target
	#
	# AppleScript requires the correct nomenclature for the target item.
	my $theTarget = "folder \"$thePath\"";
	
	if (isVolume($thePath))
		{
		$theTarget =~ s/folder "\/Volumes\//disk "/;
		}



	# Set the window position
	#
	# In theory, the "set bounds" command is all that should be necessary
	# to set the bounds of a Finder window.
	#
	# Unfortunately, under 10.4 this will result in a window that will be
	# taller than the specified size when the window is next opened.
	#
	# To reliably set the bounds of a window we must open the window, show
	# the status bar, and set the window bounds to be 20 pixels taller (the
	# height of the status bar) than necessary.
	#
	# The status bar can then be hidden, the window closed, and the bounds
	# bounds will be the desired size when the window is next opened.
	my $theScript = "";
	
	$theScript .= "tell application \"Finder\"\n";
	$theScript .= "    set theTarget to $theTarget                   \n";
	$theScript .= "    set theWindow to window of theTarget          \n";
	$theScript .= "\n";
	$theScript .= "    open theWindow                                \n";
	$theScript .= "    set statusbar visible of theWindow to true    \n";
	$theScript .= "    set bounds            of theWindow to {$posX, $posY, $right, $bottom+20} \n";
	$theScript .= "    set statusbar visible of theWindow to false   \n";
	$theScript .= "    close theWindow                               \n";
	$theScript .= "\n";
	$theScript .= "end tell";

	appleScript($theScript);
}





#============================================================================
#		setIconPos : Set the position of an icon.
#----------------------------------------------------------------------------
sub setIconPos
{


	# Retrieve our parameters
	my ($theFile, $posX, $posY) = @_;



	# Identify the target
	#
	# Since the 'posix file' command follows symlinks, in order to set the
	# position of a symlink (vs its target) we need to use an HFS path and
	# reference it as a file rather than an alias.
	my $theTarget = "alias (posix file \"$theFile\")";

	if (-l $theFile)
		{
		$theTarget = $theFile;
		
		$theTarget =~ s/\/Volumes\///;
		$theTarget =~ s/\//:/g;
		$theTarget = "file \"$theTarget\"";
		}



	# Set the icon position
	#
	# Once a change has been made, it must be flushed to disk with update.
	my $theScript = "";
	
	$theScript .= "tell application \"Finder\"\n";
	$theScript .= "    set theTarget to $theTarget                 \n";
	$theScript .= "\n";
	$theScript .= "    set position of theTarget to {$posX, $posY} \n";
	$theScript .= "    update theTarget\n";
	$theScript .= "end tell";

	appleScript($theScript);
}





#============================================================================
#		doOpen : Open a new disk image.
#----------------------------------------------------------------------------
sub doOpen
{


	# Retrieve our parameters
	my ($dmgFile, $volName) = @_;



	# Clean up any previous image
	system("rm", "-f", "$dmgFile.sparseimage");
	system("rm", "-f", "$dmgFile");



	# Create the image
	#
	# A large sparse disk image is created, which will be shrunk down
	# and compressed when the disk image is finally closed.
	print "  creating $dmgFile\n" if ($kLogging eq "-quiet");

	system("hdiutil",	"create",		$dmgFile,
						"-volname",		$volName,
						"-megabytes",	"1000",
						"-type",		"SPARSE",
						"-fs",			"HFS+",
						$kLogging);

	system("hdiutil", "mount", $kLogging, "$dmgFile.sparseimage");
}





#============================================================================
#		doClose : Close a disk image.
#----------------------------------------------------------------------------
sub doClose
{


	# Retrieve our parameters
	my ($dmgFile, $volName) = @_;



	# Bless the volume
	#
	# Blessing the volume ensures that the volume always opens in the current
	# view, overriding the user's "Open new windows in column view" preference.
	system("bless", "--openfolder", "/Volumes/$volName");



	# Compress the image
	#
	# On 10.5, the disk image must be ejected rather than unmounted to allow
	# it to be converted from a sparse image to a compressed image.
	print "  compressing $dmgFile\n" if ($kLogging eq "-quiet");

	system("hdiutil", "eject", $kLogging, "/Volumes/$volName");

	system("hdiutil",	"convert",		"$dmgFile.sparseimage",
						"-format",		"UDZO",
						"-o",			$dmgFile,
						"-imagekey",	"zlib-level=9",
						$kLogging);



	# Clean up
	system("rm", "-f", "$dmgFile.sparseimage");
}





#============================================================================
#		doSet : Set a file/folder state.
#----------------------------------------------------------------------------
sub doSet
{


	# Retrieve our parameters
	my ($thePath, $posX, $posY, $theWidth, $theHeight, $iconSize, $theIcon, $bgImage, $bgColor, $flagToolbar) = @_;



	# Set the custom icon
	if ($theIcon ne "")
		{
		setCustomIcon($thePath, $theIcon);
		}



	# Set the folder state
	if ($iconSize != 0 || $bgImage ne "" || $bgColor ne "" || $flagToolbar ne "")
		{
		setFolderState($thePath, $iconSize, $flagToolbar, $bgImage, $bgColor);
		}



	# Set the position
	#
	# Window position must be set after applying the folder state.
	if ($posX != 0 && $posY != 0)
		{
		if ($theWidth != 0 && $theHeight != 0)
			{
			setWindowPos($thePath, $posX, $posY, $theWidth, $theHeight);
			}
		else
			{
			setIconPos($thePath, $posX, $posY);
			}
		}
}





#============================================================================
#		dmgUtil : Manipulate a disk image.
#----------------------------------------------------------------------------
sub dmgUtil
{


	# Retrieve our parameters
	my ($doOpen,  $doClose,  $doSet)							= (0, 0, 0);
	my ($posX, $posY, $theWidth, $theHeight, $iconSize)			= (0, 0, 0, 0, 0);
	my ($volName, $theIcon, $bgImage, $bgColor, $flagToolbar)	= ("", "", "", "", "");

	GetOptions(	"--open+",			=> \$doOpen,
				"--close+",			=> \$doClose,
				"--set+",			=> \$doSet,
				"--volume=s",		=> \$volName,
				"--x=i",			=> \$posX,
				"--y=i",			=> \$posY,
				"--width=i",		=> \$theWidth,
				"--height=i",		=> \$theHeight,
				"--iconsize=i",		=> \$iconSize,
				"--icon=s",			=> \$theIcon,
				"--background=s",	=> \$bgImage,
				"--bgcol=s",		=> \$bgColor,
				"--toolbar=s",		=> \$flagToolbar);

	my ($thePath) = @ARGV;

	$thePath = "" if (!defined($thePath));



	# Perform the action
	if ($doOpen     && $thePath ne "" && $volName ne "")
		{
		doOpen($thePath, $volName);
		}
	
	elsif ($doClose && $thePath ne "" && $volName ne "")
		{
		doClose($thePath, $volName);
		}
	
	elsif ($doSet   && $thePath ne "")
		{
		doSet($thePath, $posX, $posY, $theWidth, $theHeight, $iconSize, $theIcon, $bgImage, $bgColor, $flagToolbar);
		}
	
	else
		{
		print $kManPage;
		}
}





#============================================================================
#		Script entry point
#----------------------------------------------------------------------------
dmgUtil();
