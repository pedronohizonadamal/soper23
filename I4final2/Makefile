CC = gcc
LIB = -lrt -lm -lpthread
CFLAGS = -Wall -pedantic
MON = monitor
MIN = miner

.PHONY: all

all: $(MON) $(MIN)

clean:
	rm -f *.o $(MON) $(MIN) log_*



$(MON): monitor.o comprobador.o pow.o miner_rush.o
	$(CC) $(CFLAGS) $^ -o $@

$(MIN): miner.o register.o pow.o miner_rush.o
	$(CC) $(CFLAGS) $^ -o $@ $(LIB)


restore:
	rm /dev/shm/miner_network
	rm /dev/shm/sem.voting*
	rm /dev/mqueue/*
	kill -9 `jobs -p`

delog:
	rm log_*
	
miner.o: miner.c miner.h miner_register.h miner_rush.h pow.h
	$(CC) $(CFLAGS) -c $<
	
register.o: register.c register.h miner_register.h miner_rush.h pow.h
	$(CC) $(CFLAGS) -c $<

monitor.o: monitor.c monitor.h monitor_comprobador.h miner_rush.h pow.h
	$(CC) $(CFLAGS) -c $<
	
comprobador.o: comprobador.c comprobador.h monitor_comprobador.h miner_rush.h pow.h
	$(CC) $(CFLAGS) -c $<
	
miner_rush.o: miner_rush.c miner_rush.h
	$(CC) $(CFLAGS) -c $<

monitor_comprobador.o: monitor_comprobador.c monitor_comprobador.h
	$(CC) $(CFLAGS) -c $<

pow.o: pow.c pow.h
	$(CC) $(CFLAGS) -c $<
