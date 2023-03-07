CC = gcc
CFLAGS = -Wall -pedantic
LIBS = -lm -lpthread
MIN = minero
PRIN = principal
MON = monitor

.PHONY: all clear clean

clean:
	rm -f *.o $(PRIN) $(MIN) $(MON)

$(MIN): minero.o pow.o monitor.o
	$(CC) $(CFLAGS) $^ -o $@ -lm -lpthread

$(PRIN): principal.o
	$(CC) $(CFLAGS) $^ -o $@

$(MON): monitor.o pow.o
	$(CC) $(CFLAGS) $^ -o $@

all: clean $(MIN) $(PRIN)

pow.o: pow.c pow.h
	$(CC) $(CFLAGS) -c $<

minero.o: minero.c minero.h
	$(CC) $(CFLAGS) -c $<

principal.o: principal.c
	$(CC) $(CFLAGS) -c $<

monitor.o: monitor.c monitor.h
	$(CC) $(CFLAGS) -c $<

#gcc minero.c minero.h pow.c pow.h -o minero -lm