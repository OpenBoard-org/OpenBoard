
HEADERS      += src/adaptors/UBExportAdaptor.h\
                src/adaptors/UBExportPDF.h \
                src/adaptors/UBExportFullPDF.h \
                src/adaptors/UBExportDocument.h \
                src/adaptors/UBSvgSubsetAdaptor.h \
                src/adaptors/UBMetadataDcSubsetAdaptor.h \
                src/adaptors/UBImportAdaptor.h \
                src/adaptors/UBImportDocument.h \
                src/adaptors/UBThumbnailAdaptor.h \
                src/adaptors/UBImportPDF.h \
                src/adaptors/UBImportImage.h \
                src/adaptors/UBIniFileParser.h \
                src/adaptors/UBExportWeb.h \
                src/adaptors/UBWebPublisher.h \
                src/adaptors/UBImportCFF.h \
                src/adaptors/UBExportCFF.h \
                src/adaptors/UBCFFSubsetAdaptor.h

HEADERS      += src/adaptors/publishing/UBDocumentPublisher.h \
                src/adaptors/publishing/UBSvgSubsetRasterizer.h
               


SOURCES      += src/adaptors/UBExportAdaptor.cpp\
                src/adaptors/UBExportPDF.cpp \
                src/adaptors/UBExportFullPDF.cpp \
                src/adaptors/UBExportDocument.cpp \
                src/adaptors/UBSvgSubsetAdaptor.cpp \
                src/adaptors/UBMetadataDcSubsetAdaptor.cpp \
                src/adaptors/UBImportAdaptor.cpp \
                src/adaptors/UBImportDocument.cpp \
                src/adaptors/UBThumbnailAdaptor.cpp \
                src/adaptors/UBImportPDF.cpp \
                src/adaptors/UBImportImage.cpp \
                src/adaptors/UBIniFileParser.cpp \
                src/adaptors/UBExportWeb.cpp \
                src/adaptors/UBWebPublisher.cpp \
                src/adaptors/UBImportCFF.cpp \
                src/adaptors/UBExportCFF.cpp \
                src/adaptors/UBCFFSubsetAdaptor.cpp \
    src/adaptors/publishing/UBDocumentPublisher.cpp

SOURCES      += \
                src/adaptors/publishing/UBSvgSubsetRasterizer.cpp
