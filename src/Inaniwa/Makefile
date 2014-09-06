###
### Files
###

EXE = Inaniwa

OBJS = Lesserkai.o Globals.o Joseki.o KomaMoves.o Kyokumen.o Sikou.o Te.o USIUtil.o WinAPI.o Inaniwa.o

###
### Rules
###

all: $(EXE) .depend

clean: 
	$(RM) *.o .depend Inaniwa

###
### Compiler:
###

CXX = g++

###
### Dependencies
###

$(EXE): $(OBJS)
	$(CXX) $(LDFLAGS) -o $@ $(OBJS)

.depend:
	$(CXX) -MM $(OBJS:.o=.cpp) > $@

include .depend

###
### Compiler and linker switches
###

# Enable/disable debugging:

CXXFLAGS += -DNDEBUG

# Compile with full warnings, and symbol names

CXXFLAGS += -Wall -g

# General optimization flags.  Note that -O2 might be faster than -O3 on some
# systems; this requires testing.

CXXFLAGS += -O3

# General linker flags

LDFLAGS += -lpthread
