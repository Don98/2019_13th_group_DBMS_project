CXX = g++

HEADER_DIR = ../include

FPTREE_HEADER_DIR = $(HEADER_DIR)/fptree

CXXFLAGS = -std=c++11

CFLAGS = -fPIC -std=c99 -O3 -msse4.2 -mpclmul -march=native -funroll-loops -Wstrict-overflow -Wstrict-aliasing -Wall -Wextra -pedantic -Wshadow

MAIN = ./bin/main

YCSB = ./bin/ycsb

LYCSB = ./bin/lycsb

BIN = bin

FPTREE = fptree.o

UTILITY = utility.o

PALLOCATOR = pallocator.o

CLHASH = clhash.o

LEVELDB_HEADER_DIR = ../leveldb/include

LEVELDB_LIB = ../leveldb/build/libleveldb.a

ALL := $(LYCSB) $(YCSB) $(MAIN) $(FPTREE) $(UTILITY) $(PALLOCATOR) $(CLHASH)

#============================TODO: all below============================
$(MAIN) :

$(FPTREE) : 

$(UTILITY) :

$(PALLOCATOR) : 

$(CLHASH) : clhash.c $(HEADER_DIR)/utility/clhash.h
	$(CC) $(CFLAGS) -c clhash.c -I $(HEADER_DIR) -g -o $@

$(BIN):
	mkdir -p bin

$(LYCSB): lycsb.cpp
	$(CXX) $(CXXFLAGS) -I $(HEADER_DIR) -I $(LEVELDB_HEADER_DIR) lycsb.cpp $(LEVELDB_LIB) -lpthread -o $@

all : $(ALL)

lycsbTest: $(BIN) $(LYCSB)
	@rm -rf /tmp/testdb
	@$(LYCSB)

clean :
	rm -rf *.o ./bin/* ../data/*

cleand : 
	rm -rf ../data/*