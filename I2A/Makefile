CC = gcc
CFLAGS = -Wall -pedantic
PRIN = voting
VOT = votante
.PHONY: all

clean:
	rm -f *.o $(VOT)

$(VOT): votante.o
	$(CC) $(CFLAGS) $^ -o $@

$(PRIN): voting.o
	$(CC) $(CFLAGS) $^ -o $@

all: $(VOT) $(PRIN)

votante.o: votante.c votante.h
	$(CC) $(CFLAGS) -c $<

voting.o: voting.c votante.h votante.c
	$(CC) $(CFLAGS) -c $<
