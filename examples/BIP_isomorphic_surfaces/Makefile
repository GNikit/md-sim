SHELL = /bin/bash
include ../../Makefile.variable
LIB := ../../lib
BIN := ../../bin
SRC := ../../src
INC := -I ../../include
LN  := -L ../../lib

LINKER = $(INC) $(LN) -lmd

all: isomorph_line

isomorph_line: *.o $(LIB)/libmd.a
	$(CXX) $(CXXFLAGS) *.o $(LINKER) -o isomorph_line -lstdc++fs
	cp isomorph_line ../examplebin/

%.o: %.cpp
	$(CXX) -c $(CXXFLAGS) -I$(SRC) $<

clean: clean_keep_data
	$(RM) *.dat *.txt *.log

clean_keep_data:
	$(RM) isomorph_line *.o