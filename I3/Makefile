CC = gcc
CFLAGS = -Wall -pedantic
PROMON = procesomonitor
MON = monitor

clean:
	rm -f *.o $(MON) $(PROMON)

.PHONY: all

$(PROMON): procesomonitor.o
	$(CC) $(CFLAGS) $^ -o $@

$(MON): procesocomprobador.o
	$(CC) $(CFLAGS) $^ -o $@

all: $(MON) $(PROMON)

procesomonitor.o: procesomonitor.c monitor.h
	$(CC) $(CFLAGS) -c $<

procesocomprobador.o: procesocomprobador.c monitor.h
	$(CC) $(CFLAGS) -c $<
