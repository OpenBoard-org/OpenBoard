# Directories
PRODUCT_DIR="$BUILD_DIR/product"

function notify {
    GROWLNOTIFY=`which growlnotify`
    if [ -x "$GROWLNOTIFY" ]; then
        $GROWLNOTIFY --name OpenBoard-build --iconpath /Developer/Applications/Xcode.app --message "$1" "OpenBoard"
    fi
    printf "\033[32m--->\033[0m $1\n"
}

function abort {
    printf "\033[31merror:\033[0m $1\n"
    exit 1
}

function warn {
    abort "$1"
}

function checkExecutable {
    if [ ! -x "$1" ]; then
        abort "$1 not found"
    fi
}

APP="$PRODUCT_DIR/$APPLICATION_NAME.app"
# make sure libs installed via homebrew 2.0 refer to in-app libs
notify "relinking libs ..."
# libavformat
install_name_tool "$APP/Contents/Frameworks/libavformat.58.dylib" -change /usr/local/Cellar/ffmpeg/4.1.4_2/lib/libavcodec.58.dylib @executable_path/../Frameworks/libavcodec.58.dylib
install_name_tool "$APP/Contents/Frameworks/libavformat.58.dylib" -change /usr/local/Cellar/ffmpeg/4.1.4_2/lib/libswresample.3.dylib @executable_path/../Frameworks/libswresample.3.dylib
install_name_tool "$APP/Contents/Frameworks/libavformat.58.dylib" -change /usr/local/Cellar/ffmpeg/4.1.4_2/lib/libavutil.56.dylib @executable_path/../Frameworks/libavutil.56.dylib

# libavcodec
install_name_tool "$APP/Contents/Frameworks/libavcodec.58.dylib" -change /usr/local/Cellar/ffmpeg/4.1.4_2/lib/libswresample.3.dylib  @executable_path/../Frameworks/libswresample.3.dylib
install_name_tool "$APP/Contents/Frameworks/libavcodec.58.dylib" -change /usr/local/Cellar/ffmpeg/4.1.4_2/lib/libavutil.56.dylib @executable_path/../Frameworks/libavutil.56.dylib

#libswresample
install_name_tool "$APP/Contents/Frameworks/libswresample.3.dylib" -change /usr/local/Cellar/ffmpeg/4.1.4_2/lib/libavutil.56.dylib @executable_path/../Frameworks/libavutil.56.dylib

#libswscale
install_name_tool "$APP/Contents/Frameworks/libswscale.5.dylib" -change /usr/local/Cellar/ffmpeg/4.1.4_2/lib/libavutil.56.dylib @executable_path/../Frameworks/libavutil.56.dylib

# libhogweed
install_name_tool "$APP/Contents/Frameworks/libhogweed.4.dylib" -change /usr/local/Cellar/nettle/3.4.1/lib/libnettle.6.dylib @executable_path/../Frameworks/libnettle.6.dylib

# libssl
install_name_tool "$APP/Contents/Frameworks/libssl.1.1.dylib" -change /usr/local/Cellar/openssl@1.1/1.1.1c/lib/libcrypto.1.1.dylib @executable_path/../Frameworks/libcrypto.1.1.dylib

# libvorbis
install_name_tool "$APP/Contents/Frameworks/libvorbisenc.2.dylib" -change /usr/local/Cellar/libvorbis/1.3.6/lib/libvorbis.0.dylib @executable_path/../Frameworks/libvorbis.0.dylib
