CC = gcc
CFLAGS = -Wall -pedantic
LIBS = -lpthread
EXE = minero

.PHONY: all clear clean

clean:
	rm -f *.o $(EXE)

$(EXE): minero.c pow.o
	$(CC) $(CFLAGS) -o $@ $^ $(LIBFLAGS)

all: clean $(EXE)

pow.o: pow.c pow.h
	$(CC) $(CFLAGS) -c $<
	

