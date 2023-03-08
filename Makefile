CC = gcc
CFLAGS = -Wall -pedantic
LIBS = -lm -lpthread
MIN = minero
PRIN = principal

.PHONY: all clear clean

clean:
	rm -f *.o $(PRIN) $(MIN)

$(MIN): minero.o pow.o
	$(CC) $(CFLAGS) $^ -o $@ -lm -lpthread

$(PRIN): principal.o
	$(CC) $(CFLAGS) $^ -o $@


all: clean $(MIN) $(PRIN)

pow.o: pow.c pow.h
	$(CC) $(CFLAGS) -c $<

minero.o: minero.c minero.h
	$(CC) $(CFLAGS) -c $<

principal.o: principal.c
	$(CC) $(CFLAGS) -c $<


#gcc minero.c minero.h pow.c pow.h -o minero -lm