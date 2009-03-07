GCC=g++
GCCFLAGS= -W -Wall -Werror -pedantic -ansi
OBJECTS= strparse.o messagehandle.o database.o

all: $(OBJECTS)
	$(GCC) $(GCCFLAGS) -lpthread -lsqlite3 -o server server.cpp $(OBJECTS)
	
messagehandle.o: messagehandle.h messagehandle.cpp
	$(GCC) $(GCCFLAGS) -c messagehandle.cpp

strparse.o: strparse.h strparse.cpp
	$(GCC) $(GCCFLAGS) -c strparse.cpp
	
database.o: ../db/database.h ../db/database.cpp
	$(GCC) $(GCCFLAGS) -c ../db/database.cpp

clean:
	rm -rf *~ *.o server
