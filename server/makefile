GCC=g++
GCCFLAGS= -W -Wall -Werror -pedantic -ansi
OBJECTS= strparse.o messagehandle.o

all: $(OBJECTS)
	$(GCC) $(GCCFLAGS) -lpthread -o server server.cpp $(OBJECTS)
	
messagehandle.o: messagehandle.h
	$(GCC) $(GCCFLAGS) -c messagehandle.cpp

strparse.o: strparse.h
	$(GCC) $(GCCFLAGS) -c strparse.cpp

clean:
	rm -rf *~ *.o server
