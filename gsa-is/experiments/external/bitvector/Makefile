CCLIB=-lsdsl -ldivsufsort -ldivsufsort64 -Wno-comment -Wunused-variable
VLIB= -g -O0

LIB_DIR = ${HOME}/lib
INC_DIR = ${HOME}/include
MY_CXX_FLAGS= -std=c++11 -Wall -Wextra  -DNDEBUG $(CODE_COVER)
MY_CXX_OPT_FLAGS= -O3 -ffast-math -funroll-loops -m64 -fomit-frame-pointer -D_FILE_OFFSET_BITS=64
MY_CXX=/usr/bin/c++

LFLAGS = -lm -lrt -ldl

LIBOBJ = ../malloc_count/malloc_count.o\
	 lib/utils.o\
	 lib/document_array.o

CXX_FLAGS=$(MY_CXX_FLAGS) $(MY_CXX_OPT_FLAGS) -I$(INC_DIR) -L$(LIB_DIR) $(LFLAGS)

CLAGS= -DSYMBOLBYTES=1

####
DIR = ../../dataset/
INPUT = input-10000.txt

K	= 10000
MODE 	= 6
CHECK	= 1
OUTPUT	= 0

DEBUG	= 0
SDV	= 0
DEFINES = -DDEBUG=$(DEBUG) -DSDV=$(SDV)

####

all: compile 

utils: 
	$(MY_CXX) $(CXX_FLAGS) $(DEFINES) -c lib/utils.cpp $(CCLIB) -o lib/utils.o 

document_array: 
	$(MY_CXX) $(CXX_FLAGS) $(DEFINES) -c lib/document_array.cpp $(CCLIB) -o lib/document_array.o 

compile: document_array utils main.cpp ${LIBOBJ}
	$(MY_CXX) $(CXX_FLAGS) $(LIBOBJ) $(DEFINES) main.cpp $(CCLIB) -o main

run: 
	../../gsais $(DIR) $(INPUT) $(K) 6 0 0 $(CHECK) 1
	./main $(DIR) $(INPUT) $(K) $(CHECK) $(OUTPUT)

clean:
	\rm lib/*.o src/*.o external/*.o main -f 

