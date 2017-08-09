# Makefile for suftest

# options
CC						= gcc
#CXX						= g++
#OUTPUT_OPTION	= -o $@
CFLAGS				= -O3 -fomit-frame-pointer
#CXXFLAGS			= -O3 -fomit-frame-pointer
CPPFLAGS			= -Wall -DNDEBUG
LDFLAGS				= 
LDLIBS				= 
#TARGET_ARCH		=

# targets
.PHONY: all
all: suftest
suftest: qsufsort.o suftest.o
test:
	$(CC) -O -g -Wall test.c qsufsort.c -o test
	./test
	$(RM) test test.exe

distclean: clean
clean:
	$(RM) suftest suftest.exe test test.exe qsufsort.o suftest.o

# dependencies
qsufsort.o suftest.o: Makefile
