
HEADERS      += src/podcast/UBPodcastController.h \
                src/podcast/UBAbstractVideoEncoder.h \
                src/podcast/UBPodcastRecordingPalette.h \
                src/podcast/youtube/UBYouTubePublisher.h \
                src/podcast/intranet/UBIntranetPodcastPublisher.h \
    $$PWD/ffmpeg/UBMicrophoneInput.h
                
SOURCES      += src/podcast/UBPodcastController.cpp \
                src/podcast/UBAbstractVideoEncoder.cpp \
                src/podcast/UBPodcastRecordingPalette.cpp \
                src/podcast/youtube/UBYouTubePublisher.cpp \
                src/podcast/intranet/UBIntranetPodcastPublisher.cpp \
    $$PWD/ffmpeg/UBMicrophoneInput.cpp

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
    HEADERS  += src/podcast/ffmpeg/UBFFmpegVideoEncoder.h

    SOURCES  += src/podcast/ffmpeg/UBFFmpegVideoEncoder.cpp


    FFMPEG = /opt/ffmpeg

    INCLUDEPATH += $${FFMPEG}/include
    DEPENDPATH += /usr/lib/x86_64-linux-gnu

    LIBS += -L $${FFMPEG}/lib -lavformat \
            -L $${FFMPEG}/lib -lavcodec \
            -L $${FFMPEG}/lib -lswscale \
            -L $${FFMPEG}/lib -lavutil \
            -lva-x11 \
            -lva \
            -lxcb-shm \
            -lxcb-xfixes \
            -lxcb-render -lxcb-shape -lxcb -lX11 -lasound -lSDL -lx264 -lpthread -lvpx -lvorbisenc -lvorbis -ltheoraenc -ltheoradec -logg -lopus -lmp3lame -lfreetype -lfdk-aac -lass -llzma -lbz2 -lz -ldl -lswresample -lswscale -lavutil -lm


    QMAKE_CXXFLAGS += -std=c++11 # move this to OpenBoard.pro when we can use C++11 on all platforms
}
