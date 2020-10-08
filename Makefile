### Build information
# asteroid 

# Flags that can be passed during compilation

#OPTS = -O3 -lpthread # compiler options
OPTS = 

INCLUDE_FLAGS += -I. -I../
CPPFLAGS += $(INCLUDE_FLAGS) -c
#LDFLAGS += $(OPTS) -lc -lstdc++
LDFLAGS = 
LDLIBS = 

CC = /usr/bin/g++
DEBUGDIR = debug
RELEASEDIR = release

DFLAGS = -g -DDEBUG
RFLAGS = -O3  

target : Debug

Release: CPPFLAGS += $(RFLAGS)
Release: OUTDIR = $(RELEASEDIR)
Release: TARGET = $(RELEASEDIR)/obgen

Debug: CPPFLAGS += $(DFLAGS) 
Debug: OUTDIR = $(DEBUGDIR)
Debug: TARGET = $(DEBUGDIR)/obgen

OBJECTS = $(OUTDIR)/main.o $(OUTDIR)/ObjInput.o

Release: $(OBJECTS)
	$(CC) -v -o $(TARGET) $(LDFLAGS) $(OBJECTS) $(LDLIBS)

Debug: $(OBJECTS)
	$(CC) -v -o $(TARGET) $(LDFLAGS) $(OBJECTS) $(LDLIBS)


$(OUTDIR)/main.o : main.cpp ObjInput.h
	$(CC) $(CPPFLAGS) main.cpp -o  $(OUTDIR)/main.o 

$(OUTDIR)/ObjInput.o : ObjInput.cpp ObjInput.h 
	$(CC) $(CPPFLAGS) ObjInput.cpp -o  $(OUTDIR)/ObjInput.o 

PHONY : clean

clean : 
	rm $(DEBUGDIR)/*.o
	rm $(DEBUGDIR)/obgen 
	rm $(RELEASEDIR)/*.o 
	rm $(RELEASEDIR)/obgen
