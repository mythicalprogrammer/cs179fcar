GCC=g++
GCCFLAGS= -W -Wall -Werror -pedantic -ansi
OBJECTS= strparse.o cntrl_db.o

all: $(OBJECTS)
	$(GCC) $(GCCFLAGS) -lpthread -lsqlite3 -o cntserv central.cpp $(OBJECTS)
	
strparse.o: ../server/strparse.h ../server/strparse.cpp
	$(GCC) $(GCCFLAGS) -c ../server/strparse.cpp
	
cntrl_db.o: ../db/cntrl_db.h ../db/cntrl_db.cpp
	$(GCC) $(GCCFLAGS) -c ../db/cntrl_db.cpp

clean:
	rm -rf *~ *.o cntserv cntrl.db
