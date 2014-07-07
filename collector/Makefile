CC = g++
LIBS = -lcurl -lboost_filesystem -L. -lz
CPPFLAGS = -g -I.

all: crazy_collector

crazy_collector: collector.o parser.o mycurl.o gatherer.o beholder.o gzstream.o DataTypes.o robots.o
	$(CC) $(CPPFLAGS) $(LIBS) -o $@ collector.o parser.o mycurl.o gatherer.o beholder.o gzstream.o DataTypes.o robots.o

curl: DataTypes.h mycurl.cpp mycurl.h
	$(CC) $(CPPFLAGS) -c mycurl.cpp

parser: DataTypes.h parser.cpp parser.h
	$(CC) $(CPPFLAGS) -c parser.cpp

collector: DataTypes.h collector.cpp collector.h Domain.h
	$(CC) $(CPPFLAGS) -c collector.cpp 

gatherer: DataTypes.h gatherer.cpp gatherer.h
	$(CC) $(CPPFLAGS) -c gatherer.cpp

gzstream: gzstream.h gzstream.C
	$(CC) $(CPPFLAGS) -c gzstream.C

datatypes: DataTypes.cpp DataTypes.h
	$(CC) $(CPPFLAGS) -c DataTypes.cpp
domain: Domain.h

beholder: beholder.cpp beholder.h
	$(CC) $(CFLAGS) -c beholder.cpp

robots: robots.cpp robots.h
	$(CC) $(CFLAGS) -c robots.cpp

clear-pages:
	rm -rf collected/*

clean:
	rm -rf crazy_collector  *.o statistics.txt error_converting.txt
