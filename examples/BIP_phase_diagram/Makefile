SHELL = /bin/bash
include ../../Makefile.variable
LIB := ../../lib
BIN := ../../bin
SRC := ../../src
INC := -I ../../include
LN  := -L ../../lib

LINKER = $(INC) $(LN) -lmd

all: bip_compression

bip_compression: *.o $(LIB)/libmd.a
	$(CXX) $(CXXFLAGS) *.o $(LINKER) -o bip_compression -lstdc++fs
	cp bip_compression ../examplebin/


%.o: %.cpp
	$(CXX) -c $(CXXFLAGS) -I$(SRC) $<

clean: clean_keep_data
	$(RM) *.dat *.txt *.log

clean_keep_data:
	$(RM) bip_compression *.o
