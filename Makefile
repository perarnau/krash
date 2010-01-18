CFLAGS= -ansi -pedantic -Wall -Werror -g -O2
LDFLAGS=-lcgroup

.PHONY: clean


all: ksetup 

ksetup: ksetup.c
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^

clean:
	rm *.o ksetup
