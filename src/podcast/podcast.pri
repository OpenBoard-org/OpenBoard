
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

    SOURCES  += src/podcast/quicktime/UBQuickTimeVideoEncoder.cpp \
                src/podcast/quicktime/UBAudioQueueRecorder.cpp

    HEADERS  += src/podcast/quicktime/UBQuickTimeVideoEncoder.h \
                src/podcast/quicktime/UBQuickTimeFile.h \
                src/podcast/quicktime/UBAudioQueueRecorder.h
                
    OBJECTIVE_SOURCES += src/podcast/quicktime/UBQuickTimeFile.mm
}

linux-g++* {
    HEADERS  += src/podcast/ffmpeg/UBFFmpegVideoEncoder.h \
                src/podcast/ffmpeg/UBMicrophoneInput.h

    SOURCES  += src/podcast/ffmpeg/UBFFmpegVideoEncoder.cpp \
                src/podcast/ffmpeg/UBMicrophoneInput.cpp


    DEPENDPATH += /usr/lib/x86_64-linux-gnu

    LIBS += -lavformat -lavcodec -lswscale -lavutil \
            -l:libva-x11.so.1 \
            -l:libva.so.1 \
            -l:libxcb-shm.so.0 \
            -lxcb-xfixes \
            -lxcb-render -lxcb-shape -lxcb -lX11 -l:libasound.so.2 -l:libSDL-1.2.so.0 -l:libx264.so.148 -lpthread -l:libvpx.so.3 -l:libvorbisenc.so.2 -l:libvorbis.so.0 -l:libtheoraenc.so.1 -l:libtheoradec.so.1 -l:libogg.so.0 -l:libopus.so.0 -l:libmp3lame.so.0 -lfreetype -l:libfdk-aac.so.0 -l:libass.so.5 -l:liblzma.so.5 -l:libbz2.so.1 -lz -ldl -lswresample -lswscale -lavutil -lm

    UBUNTU_VERSION = $$system(lsb_release -irs)
    equals(UBUNTU_VERSION, Ubuntu 14.04) {
        LIBS -= -lswresample
        LIBS += -lavresample
    }


    QMAKE_CXXFLAGS += -std=c++11 # move this to OpenBoard.pro when we can use C++11 on all platforms
}
