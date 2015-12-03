
HEADERS      += src/podcast/UBPodcastController.h \
                src/podcast/UBAbstractVideoEncoder.h \
                src/podcast/UBPodcastRecordingPalette.h \
                src/podcast/youtube/UBYouTubePublisher.h \
                src/podcast/intranet/UBIntranetPodcastPublisher.h
                
SOURCES      += src/podcast/UBPodcastController.cpp \
                src/podcast/UBAbstractVideoEncoder.cpp \
                src/podcast/UBPodcastRecordingPalette.cpp \
                src/podcast/youtube/UBYouTubePublisher.cpp \
                src/podcast/intranet/UBIntranetPodcastPublisher.cpp

win32 {

#    SOURCES  += src/podcast/windowsmedia/UBWindowsMediaVideoEncoder.cpp \
#                src/podcast/windowsmedia/UBWindowsMediaFile.cpp \
#                src/podcast/windowsmedia/UBWaveRecorder.cpp
#
#    HEADERS  += src/podcast/windowsmedia/UBWindowsMediaVideoEncoder.h \
#                src/podcast/windowsmedia/UBWindowsMediaFile.h \
#                src/podcast/windowsmedia/UBWaveRecorder.h
}

macx {

    SOURCES  += src/podcast/quicktime/UBQuickTimeVideoEncoder.cpp \
                src/podcast/quicktime/UBAudioQueueRecorder.cpp

    HEADERS  += src/podcast/quicktime/UBQuickTimeVideoEncoder.h \
                src/podcast/quicktime/UBQuickTimeFile.h \
                src/podcast/quicktime/UBAudioQueueRecorder.h
                
    OBJECTIVE_SOURCES += src/podcast/quicktime/UBQuickTimeFile.mm
}
