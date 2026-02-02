target_sources(${PROJECT_NAME} PRIVATE
    UBAbstractVideoEncoder.cpp
    UBAbstractVideoEncoder.h
    UBPodcastController.cpp
    UBPodcastController.h
    UBPodcastRecordingPalette.cpp
    UBPodcastRecordingPalette.h
    intranet/UBIntranetPodcastPublisher.cpp
    intranet/UBIntranetPodcastPublisher.h
    youtube/UBYouTubePublisher.cpp
    youtube/UBYouTubePublisher.h
)

if(WIN32)
    target_sources(${PROJECT_NAME} PRIVATE
        windowsmedia/UBWindowsMediaVideoEncoder.cpp
        windowsmedia/UBWindowsMediaVideoEncoder.h
        windowsmedia/UBWindowsMediaFile.cpp
        windowsmedia/UBWindowsMediaFile.h
        windowsmedia/UBWaveRecorder.cpp
        windowsmedia/UBWaveRecorder.h
    )
else()
    target_sources(${PROJECT_NAME} PRIVATE
        ffmpeg/UBFFmpegVideoEncoder.cpp
        ffmpeg/UBFFmpegVideoEncoder.h
        ffmpeg/UBMicrophoneInput.cpp
        ffmpeg/UBMicrophoneInput.h
    )
endif()
