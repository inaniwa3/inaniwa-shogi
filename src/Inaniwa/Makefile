### Executable name

EXE = Inaniwa

### Object files

OBJS = Lesserkai.o Globals.o Joseki.o KomaMoves.o \
	Kyokumen.o Sikou.o Te.o USIUtil.o WinAPI.o \
	Inaniwa.o

### Compiler

CXX = g++

### General compiler settings

CXXFLAGS = -g -Wall

### General linker settings

LDFLAGS = -lpthread

### Debugging

CXXFLAGS += -DNDEBUG

### Optimization

CXXFLAGS += -O3

### Targets

all: $(EXE) .depend

clean:
	$(RM) $(EXE) *.o .depend

$(EXE): $(OBJS)
	$(CXX) -o $@ $(OBJS) $(LDFLAGS)

.depend:
	-@$(CXX) -MM $(OBJS:.o=.cpp) > $@

-include .depend

