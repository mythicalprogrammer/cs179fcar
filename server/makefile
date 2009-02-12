GCC=g++
GCCFLAGS= -W -Wall -Werror -pedantic -ansi
OBJECTS= messagehandle.o

all: $(OBJECTS)
	$(GCC) $(GCCFLAGS) -lpthread -o server server.cpp $(OBJECTS)
	
messagehandle.o: messagehandle.h
	$(GCC) $(GCCFLAGS) -c messagehandle.cpp

clean:
	rm -rf *~ *.o server
