HEADERS += \
	src/pdf-merger/AnnotsHandler.h \
	src/pdf-merger/ASCII85Decode.h \
	src/pdf-merger/ASCIIHexDecode.h \
	src/pdf-merger/CCITTFaxDecode.h \
	src/pdf-merger/Config.h \
	src/pdf-merger/ContentHandler.h \
	src/pdf-merger/DCTDecode.h \
	src/pdf-merger/Decoder.h \
	src/pdf-merger/Document.h \
	src/pdf-merger/Exception.h \
	src/pdf-merger/FileIsAbsentException.h \
	src/pdf-merger/Filter.h \
	src/pdf-merger/FilterPredictor.h \
	src/pdf-merger/FlateDecode.h \
	src/pdf-merger/JBIG2Decode.h \
	src/pdf-merger/LZWDecode.h \
	src/pdf-merger/MediaBoxElementHandler.h \
	src/pdf-merger/MergePageDescription.h \
	src/pdf-merger/Merger.h \
	src/pdf-merger/Object.h \
	src/pdf-merger/Page.h \
	src/pdf-merger/PageElementHandler.h \
	src/pdf-merger/PageParser.h \
	src/pdf-merger/Parser.h \
	src/pdf-merger/Rectangle.h \
	src/pdf-merger/RemoveHimSelfHandler.h \
	src/pdf-merger/RunLengthDecode.h \
	src/pdf-merger/Transformation.h \
	src/pdf-merger/TypeElementHandler.h \
	src/pdf-merger/Utils.h \
	src/pdf-merger/AbstractBoxElementHandler.h \
	src/pdf-merger/CropBoxElementHandler.h \
	src/pdf-merger/OverlayDocumentParser.h \
	src/pdf-merger/RotationHandler.h

SOURCES += \
	src/pdf-merger/AnnotsHandler.cpp \
	src/pdf-merger/ASCII85Decode.cpp \
	src/pdf-merger/ASCIIHexDecode.cpp \
	src/pdf-merger/ContentHandler.cpp \
	src/pdf-merger/Document.cpp \
	src/pdf-merger/Filter.cpp \
	src/pdf-merger/FilterPredictor.cpp \
	src/pdf-merger/FlateDecode.cpp \
	src/pdf-merger/LZWDecode.cpp \
	src/pdf-merger/Merger.cpp \
	src/pdf-merger/Object.cpp \
	src/pdf-merger/Page.cpp \
	src/pdf-merger/PageElementHandler.cpp \
	src/pdf-merger/Parser.cpp \
	src/pdf-merger/Rectangle.cpp \
	src/pdf-merger/RemoveHimselfHandler.cpp \
	src/pdf-merger/RunLengthDecode.cpp \
	src/pdf-merger/Utils.cpp \
	src/pdf-merger/OverlayDocumentParser.cpp \
    src/pdf-merger/CCITTFaxDecode.cpp \
    src/pdf-merger/JBIG2Decode.cpp \
    src/pdf-merger/DCTDecode.cpp
	

macx {
   LIBS += -lz
}

win32 {
    
    LIBS     += "-L$$PWD/../../zlib/1.2.3/lib" "-lzlib"
   
}

linux-g++ {

   LIBS += -lz

}
linux-g++-32 {

   LIBS += -lz

}
linux-g++-64 {

   LIBS += -lz

}



