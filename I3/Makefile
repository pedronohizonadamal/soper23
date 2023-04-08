CC = gcc
CFLAGS = -Wall -pedantic
COM = comprobador
MON = monitor
.PHONY: all

clean:
	rm -f *.o $(MON) $(COM)

$(COM): comprobador.o
	$(CC) $(CFLAGS) $^ -o $@

$(MON): monitor.o
	$(CC) $(CFLAGS) $^ -o $@

all: $(MON) $(COM)

monitor.o: monitor.c monitor.h
	$(CC) $(CFLAGS) -c $<

comprobador.o: comprobador.c monitor.h
	$(CC) $(CFLAGS) -c $<
