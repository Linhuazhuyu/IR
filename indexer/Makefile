CC = g++
LIBS = -lboost_filesystem -L. -lz
CPPFLAGS = -g -I.

all: indexer 

indexer: gzstream.o zfilebuf.o indexer.o ocurrences.o utils.o egamma.o myFileHandler.o vector_model.o tuples.o httpserver.o
	$(CC) $(CPPFLAGS) $(LIBS) -o $@ gzstream.o zfilebuf.o indexer.o myFileHandler.o ocurrences.o utils.o egamma.o vector_model.o tuples.o httpserver.o

extractor: extractor.o gzstream.o zfilebuf.o entities.o utils.o
	$(CC) $(CPPFLAGS) $(LIBS) -o $@ gzstream.o zfilebuf.o extractor.o entities.o utils.o

prank: pagerank.o parser.o utils.o zfilebuf.o gzstream.o graph.o
	$(CC) $(CPPFLAGS) $(LIBS) -o $@ pagerank.o parser.o utils.o zfilebuf.o gzstream.o graph.o
	
parser: parser.cpp parser.h
	$(CC) $(CPPFLAGS) -c parser.cpp

pagerank: pagerank.cpp
	$(CC) $(CPPFLAGS) -c pagerank.cpp

extract: extractor.h extractor.cpp
	$(CC) $(CPPFLAGS) -c extractor.cpp

gzstream: gzstream.h gzstream.C
	$(CC) $(CPPFLAGS) -c gzstream.C

zfilebuf: zfilebuf.h zfilebuf.cpp
	$(CC) $(CPPFLAGS) -c zfilebuf.cpp

entities: entities.h entities.cpp
	$(CC) $(CPPFLAGS) -c entities.cpp

index: indexer.h indexer.cpp
	$(CC) $(CPPFLAGS) -c indexer.cpp

myFileHandler: myFileHandler.h myFileHandler.cpp
	$(CC) $(CPPFLAGS) -c myFileHandler.cpp

ocurrences: ocurrences.h ocurrences.cpp
	$(CC) $(CPPFLAGS) -c ocurrences.cpp

utils: utils.h utils.cpp
	$(CC) $(CPPFLAGS) -c utils.cpp

egamma: egamma.h egamma.cpp
	$(CC) $(CPPFLAGS) -c egamma.cpp

vector_model: vector_model.h vector_model.cpp
	$(CC) $(CPPFLAGS) -c vector_model.cpp

tuples: tuples.h tuples.cpp
	$(CC) $(CPPFLAGS) -c tuples.cpp

httpserver: httpserver.h httpserver.cpp
	$(CC) $(CPPFLAGS) -c httpserver.cpp

graph: graph.h graph.cpp
	$(CC) $(CPPFLAGS) -c graph.cpp
clear-pages:
	rm -rf stripped/*

clean:
	rm -rf indexer *.o 
