CXX=g++

CXXFLAGS=-gstabs -I../../.. -Wall -D_REENTRANT

.PHONY:all clean

BIN=minidump_upload sym_upload

all:$(BIN)

DUMP_UPLOAD_OBJ=minidump_upload.o http_upload.o
SYM_UPLOAD_OBJ=sym_upload.o http_upload.o

minidump_upload:$(DUMP_UPLOAD_OBJ)
	$(CXX) $(CXXFLAGS) -ldl -o $@ $^

sym_upload:$(SYM_UPLOAD_OBJ)
	$(CXX) $(CXXFLAGS) -ldl -o $@ $^

http_upload.o:../../../common/linux/http_upload.cc
	$(CXX) $(CXXFLAGS) `curl-config --cflags` -c $^

clean:
	rm *.o $(BIN)
