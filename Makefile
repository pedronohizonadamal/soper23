CC = gcc
CFLAGS = -Wall -pedantic
LIBS = -lm -lpthread
EXE = minero

.PHONY: all clear clean

clean:
	rm -f *.o $(EXE)

$(EXE): minero.o pow.o
	$(CC) $(CFLAGS) -o $@ $^ $(LIBFLAGS)

all: clean $(EXE)

pow.o: pow.c pow.h
	$(CC) $(CFLAGS) -c $<

minero.o: minero.c minero.h
	$(CC) $(CFLAGS) -c $<

#gcc minero.c minero.h pow.c pow.h -o minero -lm