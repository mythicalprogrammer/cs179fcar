GCC=g++
GCCFLAGS= -W -Wall -Werror -pedantic -ansi
OBJECTS=strparse.o -lpthread

all:$(OBJECTS)
	$(GCC) $(GCCFLAGS) -o client client.cpp $(OBJECTS)
strparse.o: strparse.h strparse.cpp
	$(GCC) $(GCCFLAGS) -c strparse.cpp
clean:
	rm -rf *~ *.o client 
