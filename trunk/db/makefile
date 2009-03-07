GCC= g++
GCCFLAGS= -W -Wall -Werror -pedantic -ansi
OBJECTS= database.o

all: $(OBJECTS)
	$(GCC) $(GCCFLAGS) main.cpp -lsqlite3 $(OBJECTS)

database.o: database.h
	$(GCC) $(GCCFLAGS) -c database.cpp 

cntrl_db.o: cntrl_db.h
	$(GCC) $(GCCFLAGS) -c cntrl_db.cpp 
	
clean:
	rm -rf *~ a.out *.o cardb.db
