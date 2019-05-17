CXX = g++

HEADER_DIR = ../include

FPTREE_HEADER_DIR = $(HEADER_DIR)/fptree
UTILITY_HEADER_DIR = $(HEADER_DIR)/utility

CXXFLAGS = -std=c++11

CFLAGS = -fPIC -std=c99 -O3 -msse4.2 -mpclmul -march=native -funroll-loops -Wstrict-overflow -Wstrict-aliasing -Wall -Wextra -pedantic -Wshadow

BIN = bin

MAIN = main

YCSB = ycsb

LYCSB = lycsb

FPTREE = fptree.o

UTILITY = utility.o

PALLOCATOR = p_allocator.o

CLHASH = clhash.o

LEVELDB_HEADER_DIR = ../leveldb/include

LEVELDB_LIB = ../leveldb/build/libleveldb.a

OBJECTS = $(BIN)/$(FPTREE) $(BIN)/$(PALLOCATOR) $(BIN)/$(UTILITY) $(BIN)/$(CLHASH)

ALL := $(LYCSB) $(YCSB) $(MAIN) $(FPTREE) $(UTILITY) $(PALLOCATOR) $(CLHASH) $(BIN)

#============================TODO: all below============================
$(BIN)/$(MAIN): $(BIN)/main.o $(OBJECTS)
	@mkdir -p bin
	$(CXX) $(CXXFLAGS) $^ -I$(HEADER_DIR) -lpmem -o $@

$(BIN)/main.o: main.cpp
	@mkdir -p bin
	$(CXX) $(CXXFLAGS) -c $< -I$(HEADER_DIR) -o $@

$(BIN)/$(FPTREE): fptree.cpp $(FPTREE_HEADER_DIR)/fptree.h
	@mkdir -p bin
	$(CXX) $(CXXFLAGS) -c $< -I$(HEADER_DIR) -o $@

$(BIN)/$(PALLOCATOR): p_allocator.cpp $(UTILITY_HEADER_DIR)/p_allocator.h
	@mkdir -p bin
	$(CXX) $(CXXFLAGS) -c $< -I$(HEADER_DIR) -o $@

$(BIN)/$(UTILITY): utility.cpp $(UTILITY_HEADER_DIR)/utility.h
	@mkdir -p bin
	$(CXX) $(CXXFLAGS) -c $< -I$(HEADER_DIR) -o $@

$(BIN)/$(CLHASH) : clhash.c $(UTILITY_HEADER_DIR)/clhash.h
	@mkdir -p bin
	$(CC) $(CFLAGS) -c clhash.c -I$(HEADER_DIR) -g -o $@

$(BIN)/$(LYCSB): lycsb.cpp
	@mkdir -p bin
	$(CXX) $(CXXFLAGS) -I $(HEADER_DIR) -I $(LEVELDB_HEADER_DIR) lycsb.cpp $(LEVELDB_LIB) -lpthread -o $@

$(BIN)/$(YCSB): ycsb.cpp $(OBJECTS)
	@mkdir -p bin
	$(CXX) $(CXXFLAGS) -I $(HEADER_DIR) -I $(LEVELDB_HEADER_DIR) $^ $(LEVELDB_LIB) -lpthread -lpmem -o $@


.PHONY: $(BIN)
$(BIN):
	@mkdir -p bin

.PHONY: all testleveldb testfptree
all : $(ALL)

testleveldb: $(BIN)/$(LYCSB)
	@rm -rf /tmp/testdb
	@$(BIN)/$(LYCSB)
	@rm -rf /tmp/testdb

testfptree: $(BIN)/$(YCSB)
	@sudo rm -rf /mnt/mem/*
	@sudo $(BIN)/$(YCSB)

clean :
	rm -rf *.o ./bin/* ../data/*

cleand :
	rm -rf ../data/*