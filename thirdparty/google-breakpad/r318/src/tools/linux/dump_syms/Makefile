CXX=g++
CC=gcc

CXXFLAGS=-gstabs -I../../.. -DNDEBUG -Wall -D_REENTRANT

.PHONY:all clean

BIN=dump_syms

all:$(BIN)

DUMP_OBJ=dump_symbols.o guid_creator.o dump_syms.o file_id.o md5.o

dump_syms:$(DUMP_OBJ)
	$(CXX) $(CXXFLAGS) -o $@ $^

dump_symbols.o:../../../common/linux/dump_symbols.cc
	$(CXX) $(CXXFLAGS) -c $^

guid_creator.o:../../../common/linux/guid_creator.cc
	$(CXX) $(CXXFLAGS) -c $^

file_id.o:../../../common/linux/file_id.cc
	$(CXX) $(CXXFLAGS) -c $^

md5.o:../../../common/md5.c
	$(CC) $(CXXFLAGS) -c $^

clean:
	rm -f $(BIN) $(DUMP_OBJ)
