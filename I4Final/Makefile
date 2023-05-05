CC = gcc
LIB = -lrt -lm -lpthread
CFLAGS = -Wall -pedantic
MON = monitor
MIN = miner

.PHONY: all

all: $(MON) $(MIN)

clean:
	rm -f *.o $(MON) $(MIN) log_*



$(MON): monitor.o pow.o
	$(CC) $(CFLAGS) $^ -o $@

$(MIN): miner.o pow.o
	$(CC) $(CFLAGS) $^ -o $@ $(LIB)


restore:
	rm log_*
	rm /dev/shm/miner_network
	rm /dev/shm/sem.voting*
	rm /dev/mqueue/*
	kill -9 `jobs -p`

delog:
	rm log_*
	
miner.o: miner.c registrador.c miner.h register.h miner_register.h miner_rush.h pow.h
	$(CC) $(CFLAGS) -c $<

monitor.o: monitor.c comprobador.c monitor.h comprobador.h monitor_comprobador.h miner_rush.h pow.h
	$(CC) $(CFLAGS) -c $<

monitor_comprobador.o:



pow.o: pow.c pow.h
	$(CC) $(CFLAGS) -c $<
