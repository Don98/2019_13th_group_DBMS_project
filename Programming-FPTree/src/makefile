CXX = g++

HEADER_DIR = ../include

FPTREE_HEADER_DIR = $(HEADER_DIR)/fptree
UTILITY_HEADER_DIR = $(HEADER_DIR)/utility

CXXFLAGS = -std=c++11

CFLAGS = -fPIC -std=c99 -O3 -msse4.2 -mpclmul -march=native -funroll-loops -Wstrict-overflow -Wstrict-aliasing -Wall -Wextra -pedantic -Wshadow

LIB = lib

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

OBJECTS = $(LIB)/$(FPTREE) $(LIB)/$(PALLOCATOR) $(LIB)/$(UTILITY) $(LIB)/$(CLHASH)

ALL := $(LYCSB) $(YCSB) $(MAIN) $(FPTREE) $(UTILITY) $(PALLOCATOR) $(CLHASH) $(BIN)

#============================TODO: all below============================
$(BIN)/$(MAIN): $(BIN)/main.o $(OBJECTS)
	@mkdir -p bin
	$(CXX) $(CXXFLAGS) $^ -I$(HEADER_DIR) -lpmem -o $@

$(BIN)/main.o: main.cpp
	@mkdir -p bin
	$(CXX) $(CXXFLAGS) -c $< -I$(HEADER_DIR) -o $@

$(LIB)/$(FPTREE): fptree.cpp $(FPTREE_HEADER_DIR)/fptree.h
	@mkdir -p lib
	$(CXX) $(CXXFLAGS) -c $< -I$(HEADER_DIR) -o $@

$(LIB)/$(PALLOCATOR): p_allocator.cpp $(UTILITY_HEADER_DIR)/p_allocator.h
	@mkdir -p lib
	$(CXX) $(CXXFLAGS) -c $< -I$(HEADER_DIR) -o $@

$(LIB)/$(UTILITY): utility.cpp $(UTILITY_HEADER_DIR)/utility.h
	@mkdir -p lib
	$(CXX) $(CXXFLAGS) -c $< -I$(HEADER_DIR) -o $@

$(LIB)/$(CLHASH) : clhash.c $(UTILITY_HEADER_DIR)/clhash.h
	@mkdir -p lib
	$(CC) $(CFLAGS) -c clhash.c -I$(HEADER_DIR) -g -o $@

$(BIN)/$(LYCSB): lycsb.cpp
	@mkdir -p bin
	$(CXX) $(CXXFLAGS) -I $(HEADER_DIR) -I $(LEVELDB_HEADER_DIR) lycsb.cpp $(LEVELDB_LIB) -lpthread -o $@

$(BIN)/$(YCSB): ycsb.cpp $(OBJECTS)
	@mkdir -p bin
	$(CXX) $(CXXFLAGS) -I $(HEADER_DIR) -I $(LEVELDB_HEADER_DIR) $^ $(LEVELDB_LIB) -lpthread -lpmem -o $@


.PHONY: testleveldb testfptree check install

testleveldb: $(BIN)/$(LYCSB)
	@rm -rf /tmp/testdb
	@$(BIN)/$(LYCSB)
	@rm -rf /tmp/testdb

testfptree: $(BIN)/$(YCSB)
	@sudo rm -rf /mnt/mem/*
	@sudo $(BIN)/$(YCSB)

clean:
	rm -rf *.o ../data/* ./bin ./lib check

cleand:
	rm -rf ../data/*

install: $(OBJECTS)
	@mkdir -p $(LIB)
	$(CXX) $(CXXFLAGS) -fPIC -c fptree.cpp -I$(HEADER_DIR) -o fptree.o
	$(CXX) $(CXXFLAGS) -fPIC -c p_allocator.cpp -I$(HEADER_DIR) -o p_allocator.o
	$(CXX) $(CXXFLAGS) -fPIC -c utility.cpp -I$(HEADER_DIR) -o utility.o
	$(CC) $(CFLAGS) -fPIC -c clhash.c -I$(HEADER_DIR) -g -o clhash.o
	$(CXX) $(CXXFLAGS) -shared -o $(LIB)/libfptree.so fptree.o p_allocator.o utility.o clhash.o
	@ar crv $(LIB)/libfptree.a $(LIB)/$(FPTREE) $(LIB)/$(PALLOCATOR) $(LIB)/$(UTILITY) $(LIB)/$(CLHASH)
	@sudo cp $(LIB)/libfptree.a /usr/local/lib64/libfptree.a
	@sudo cp $(LIB)/libfptree.so /usr/lib
	@rm *.o
	@sudo ldconfig
	@sudo cp -r ../include/fptree /usr/include
	@sudo cp -r ../include/utility /usr/include

uninstall:
	@sudo rm -r /usr/include/fptree /usr/include/utility

check:
	@$(CXX) $(CXXFLAGS) -I$(HEADER_DIR) -o ../tools/check check.cpp -lpmem
	@sudo chmod +x ../tools/check.sh
	@sudo ../tools/check.sh
	@sudo ../tools/check

