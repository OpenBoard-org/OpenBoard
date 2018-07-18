
HEADERS      += src/core/UB.h \
                src/core/UBApplication.h \
                src/core/UBSettings.h \
                src/core/UBSetting.h \
                src/core/UBPersistenceManager.h \
                src/core/UBSceneCache.h \
                src/core/UBPreferencesController.h \
                src/core/UBMimeData.h \
                src/core/UBIdleTimer.h \
                src/core/UBDisplayManager.h \
                src/core/UBDocumentManager.h \
                src/core/UBApplicationController.h \
                src/core/UBDownloadManager.h \
                src/core/UBDownloadThread.h \
                src/core/UBOpenSankoreImporter.h \
                src/core/UBTextTools.h \
    src/core/UBPersistenceWorker.h \
    $$PWD/UBForeignObjectsHandler.h

SOURCES      += src/core/main.cpp \
                src/core/UBApplication.cpp \
                src/core/UBSettings.cpp \
                src/core/UBSetting.cpp \
                src/core/UBPersistenceManager.cpp \
                src/core/UBSceneCache.cpp \
                src/core/UBPreferencesController.cpp \
                src/core/UBMimeData.cpp \
                src/core/UBIdleTimer.cpp \
                src/core/UBDisplayManager.cpp \
                src/core/UBDocumentManager.cpp \
                src/core/UBApplicationController.cpp \
                src/core/UBDownloadManager.cpp \
                src/core/UBDownloadThread.cpp \
                src/core/UBOpenSankoreImporter.cpp \
                src/core/UBTextTools.cpp \
    src/core/UBPersistenceWorker.cpp \
    $$PWD/UBForeignObjectsHandler.cpp
