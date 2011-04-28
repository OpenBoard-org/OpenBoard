CXX=g++
CC=gcc

CXXFLAGS=-gstabs+ -I../../.. -Wall -D_REENTRANT
LDFLAGS=-lpthread

OBJ_DIR=.
BIN_DIR=.

THREAD_SRC=linux_thread.cc
SHARE_SRC=../../minidump_file_writer.cc\
	  ../../../common/string_conversion.cc\
	  ../../../common/linux/file_id.cc\
	  minidump_generator.cc
HANDLER_SRC=exception_handler.cc\
	    ../../../common/linux/guid_creator.cc
SHARE_C_SRC=../../../common/convert_UTF.c

THREAD_TEST_SRC=linux_thread_test.cc
MINIDUMP_TEST_SRC=minidump_test.cc
EXCEPTION_TEST_SRC=exception_handler_test.cc

THREAD_OBJ=$(patsubst %.cc,$(OBJ_DIR)/%.o,$(THREAD_SRC))
SHARE_OBJ=$(patsubst %.cc,$(OBJ_DIR)/%.o,$(SHARE_SRC))
HANDLER_OBJ=$(patsubst %.cc,$(OBJ_DIR)/%.o,$(HANDLER_SRC))
SHARE_C_OBJ=$(patsubst %.c,$(OBJ_DIR)/%.o,$(SHARE_C_SRC)) md5.o
THREAD_TEST_OBJ=$(patsubst %.cc,$(OBJ_DIR)/%.o, $(THREAD_TEST_SRC))\
		$(THREAD_OBJ)
MINIDUMP_TEST_OBJ=$(patsubst %.cc,$(OBJ_DIR)/%.o, $(MINIDUMP_TEST_SRC))\
		  $(THREAD_OBJ) $(SHARE_OBJ) $(SHARE_C_OBJ)
EXCEPTION_TEST_OBJ=$(patsubst %.cc,$(OBJ_DIR)/%.o, $(EXCEPTION_TEST_SRC))\
		   $(THREAD_OBJ) $(SHARE_OBJ) $(SHARE_C_OBJ) $(HANDLER_OBJ)

BIN=$(BIN_DIR)/minidump_test\
    $(BIN_DIR)/linux_thread_test\
    $(BIN_DIR)/exception_handler_test

.PHONY:all clean

all:$(BIN)

$(BIN_DIR)/linux_thread_test:$(THREAD_TEST_OBJ)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $^ -o $@

$(BIN_DIR)/minidump_test:$(MINIDUMP_TEST_OBJ)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $^ -o $@

$(BIN_DIR)/exception_handler_test:$(EXCEPTION_TEST_OBJ)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $^ -o $@

md5.o:../../../common/md5.c
	$(CC) $(CXXFLAGS) -c $^

clean:
	rm -f $(BIN) *.o *.dmp
