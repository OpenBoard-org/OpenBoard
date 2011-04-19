#!/bin/bash

#
# Usage: upgrade.sh "Uniboard 4.3.5" "/Users/Name/Downloads/Uniboard 4.3.5.dmg" "/Applications/Uniboard.app" 
#  

DMG_NAME=$1
DMG_PATH=$2
TARGET_PATH=$3

umount "$DMG_NAME" 2> /dev/null
hdiutil attach "$DMG_PATH" -nobrowse

rm -rf "$TARGET_PATH"
cp -R "/Volumes/$DMG_NAME/Uniboard.app" "$TARGET_PATH"

hdiutil detach "/Volumes/$DMG_NAME"

open $TARGET_PATH