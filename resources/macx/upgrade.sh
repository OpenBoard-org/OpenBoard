# --------------------------------------------------------------------
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.
# ---------------------------------------------------------------------

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
