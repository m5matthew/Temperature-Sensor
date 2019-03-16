# NAME: Matthew Wang
# EMAIL: m5matthew@g.ucla.edu
# ID: 504984273

CC=gcc
CFLAGS=-Wall -Wextra -lmraa -lm
SOURCE=lab4c_tcp.c lab4c_tls.c tcp.c tls.c util.c
HEADER=lab4c_tcp.h lab4c_tls.h tcp.h tls.h util.h
MISC=README Makefile

.SILENT:

default: 
	$(CC) $(CFLAGS) -o lab4c_tcp lab4c_tcp.c util.c tcp.c
	$(CC) $(CFLAGS) -lssl -lcrypto -o lab4c_tls lab4c_tls.c util.c tls.c

clean:
	rm -f lab4c_tcp lab4c_tls lab4c-504984273.tar.gz

dist:
	tar -czf lab4c-504984273.tar.gz $(SOURCE) $(HEADER) $(MISC)
