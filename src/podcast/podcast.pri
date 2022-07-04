
HEADERS      += src/podcast/UBPodcastController.h \
                src/podcast/UBAbstractVideoEncoder.h \
                src/podcast/UBPodcastRecordingPalette.h \
                src/podcast/youtube/UBYouTubePublisher.h \
                src/podcast/intranet/UBIntranetPodcastPublisher.h \
                
SOURCES      += src/podcast/UBPodcastController.cpp \
                src/podcast/UBAbstractVideoEncoder.cpp \
                src/podcast/UBPodcastRecordingPalette.cpp \
                src/podcast/youtube/UBYouTubePublisher.cpp \
                src/podcast/intranet/UBIntranetPodcastPublisher.cpp \

win32 {

    SOURCES  += src/podcast/windowsmedia/UBWindowsMediaVideoEncoder.cpp \
                src/podcast/windowsmedia/UBWindowsMediaFile.cpp \
                src/podcast/windowsmedia/UBWaveRecorder.cpp

    HEADERS  += src/podcast/windowsmedia/UBWindowsMediaVideoEncoder.h \
                src/podcast/windowsmedia/UBWindowsMediaFile.h \
                src/podcast/windowsmedia/UBWaveRecorder.h
}

macx {
    CONFIG += c++11

    SOURCES  += src/podcast/ffmpeg/UBFFmpegVideoEncoder.cpp \
                src/podcast/ffmpeg/UBMicrophoneInput.cpp

    HEADERS  += src/podcast/ffmpeg/UBFFmpegVideoEncoder.h \
                src/podcast/ffmpeg/UBMicrophoneInput.h

    LIBS += -lavformat -lavcodec -lswscale  -lswresample -lavutil \
        -lpthread -lvpx -lvorbisenc -llzma -lbz2 -lz -ldl -lavutil -lm

    # (ffmpeg-4.0 with all options (to clean))
    # brew install ffmpeg --with-chromaprint --with-fdk-aac --with-libass --with-librsvg --with-libsoxr --with-libssh --with-tesseract
    #--with-libvidstab --with-opencore-amr --with-openh264 --with-openjpeg --with-openssl --with-rtmpdump --with-rubberband --with-sdl2
    #--with-snappy --with-tools --with-webp --with-x265 --with-xz --with-zeromq --with-zimg
    # brew install opus

    LIBS += -L/usr/local/opt/x264/lib
    LIBS += -L/usr/local/opt/libvorbis/lib
    LIBS += -L/usr/local/opt/libvpx/lib
    LIBS += -L/usr/local/opt/theora/lib
    LIBS += -L/usr/local/opt/libogg/lib
    LIBS += -L/usr/local/opt/opus/lib
    LIBS += -L/usr/local/opt/lame/lib
    LIBS += -L/usr/local/opt/libass/lib
}

linux {
    HEADERS  += src/podcast/ffmpeg/UBFFmpegVideoEncoder.h \
                src/podcast/ffmpeg/UBMicrophoneInput.h

    SOURCES  += src/podcast/ffmpeg/UBFFmpegVideoEncoder.cpp \
                src/podcast/ffmpeg/UBMicrophoneInput.cpp

    PKGCONFIG += libavformat libavcodec libavutil libswresample libswscale
}
