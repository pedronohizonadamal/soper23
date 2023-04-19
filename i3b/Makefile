CC = gcc
LIB = -lrt
CFLAGS = -Wall -pedantic
MON = monitor
MIN = minero

clean:
	rm -f *.o $(MON)

.PHONY: all

$(MON): monitor.o pow.o
	$(CC) $(CFLAGS) $^ -o $@

$(MIN): miner.o pow.o
	$(CC) $(CFLAGS) $^ -o $@

all: $(MON) $(MIN)

miner.o: miner.c monitor.h pow.h
	$(CC) $(CFLAGS) -c $< $(LIB)

monitor.o: monitor.c monitor.h pow.h
	$(CC) $(CFLAGS) -c $< $(LIB)

pow.o: pow.c pow.h
	$(CC) $(CFLAGS) -c $<