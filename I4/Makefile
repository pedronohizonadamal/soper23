CC = gcc
LIB = -lrt -lm -lpthread
CFLAGS = -Wall -pedantic
MON = monitor
MIN = miner

clean:
	rm -f *.o $(MON) $(MIN)

.PHONY: all

$(MON): monitor.o pow.o
	$(CC) $(CFLAGS) $^ -o $@

$(MIN): miner.o pow.o
	$(CC) $(CFLAGS) $^ -o $@ $(LIB)

all: $(MON) $(MIN)

restore:
	rm /dev/shm/miner_network
	rm /dev/shm/sem.voting*
	rm /dev/mqueue/*
	kill -9 `jobs -p`
	
miner.o: miner.c monitor.h pow.h
	$(CC) $(CFLAGS) -c $<

monitor.o: monitor.c monitor.h pow.h
	$(CC) $(CFLAGS) -c $<

pow.o: pow.c pow.h
	$(CC) $(CFLAGS) -c $<
