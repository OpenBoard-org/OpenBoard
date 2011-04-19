INCLUDEPATH  += "$$PWD/../../zlib/1.2.3/include"

HEADERS += \
	AnnotsHandler.h \
	ASCII85Decode.h \
	ASCIIHexDecode.h \
	CCITTFaxDecode.h \
	Config.h \
	ContentHandler.h \
	DCTDecode.h \
	Decoder.h \
	Document.h \
	Exception.h \
	FileIsAbsentException.h \
	Filter.h \
	FilterPredictor.h \
	FlateDecode.h \
	JBIG2Decode.h \
	LZWDecode.h \
	MediaBoxElementHandler.h \
	MergePageDescription.h \
	Merger.h \
	Object.h \
	Page.h \
	PageElementHandler.h \
	PageParser.h \
	Parser.h \
	Rectangle.h \
	RemoveHimSelfHandler.h \
	RunLengthDecode.h \
	Transformation.h \
	TypeElementHandler.h \
	Utils.h \
	AbstractBoxElementHandler.h \
	CropBoxElementHandler.h \
	OverlayDocumentParser.h \
	RotationHandler.h

SOURCES += \
	AnnotsHandler.cpp \
	ASCII85Decode.cpp \
	ASCIIHexDecode.cpp \
	ContentHandler.cpp \
	Document.cpp \
	Filter.cpp \
	FilterPredictor.cpp \
	FlateDecode.cpp \
	LZWDecode.cpp \
	Merger.cpp \
	Object.cpp \
	Page.cpp \
	PageElementHandler.cpp \
	Parser.cpp \
	Rectangle.cpp \
	RemoveHimselfHandler.cpp \
	RunLengthDecode.cpp \
	Utils.cpp \
	OverlayDocumentParser.cpp
	

macx {

   LIBS += -lz
}

win32 {
    
    LIBS     += "-L$$PWD/../../zlib/1.2.3/lib" "-lzlib"
   
}

linux-g++ {

   LIBS += -lz

}