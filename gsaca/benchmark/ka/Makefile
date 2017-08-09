# Makefile for suftest

# sources
OBJS					= BooleanString.o LinearSuffixSort.o suftest.o
TARGET				= suftest
MAKEFILE			= Makefile

# options
CC						= g++
LD						= g++
#OUTPUT_OPTION	= -o $@
CFLAGS				= -O2 -g
CXXFLAGS			= -O2 -g
CPPFLAGS			= -Wall #-DNDEBUG
LDFLAGS				= 
LDLIBS				= 
#TARGET_ARCH		=


# targets
.PHONY: all
all: $(TARGET)
$(TARGET): $(OBJS)

distclean: clean
clean:
	$(RM) $(TARGET) $(OBJS)

# dependencies
$(OBJS): BooleanString.h LinearSuffixSort.h $(MAKEFILE)

