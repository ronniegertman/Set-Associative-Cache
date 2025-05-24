//create a queue:
#include <iostream>
#include <fstream>
#include <queue>
#include <sstream>
using std::queue;
class Way{
    public:
        int id;
        unsigned tag;
        bool valid;
        bool dirty;
        Way() : tag(0), valid(false), dirty(false), id(id) {}
};

class Set{
    public:
        int size;
        queue<Way> waysQueue;
    Set() : size(0) {}
    Set(int size) : size(size) {}
    ~Set() {
        while (!waysQueue.empty()) {
            waysQueue.pop();
        }
    }
};

class Cache{
    int ways; // number of ways
    int size;
    int cyc;
    int sets; // number of sets
    Set* setsArray;
    public:
        Cache(int size, int assoc, int cyc) : size(size),cyc(cyc), ways(1 << assoc), sets(size / ways) {
            setsArray = new Set[sets];
            for (int i = 0; i < sets; ++i) {
                setsArray[i].size = ways;
            }
        }
        ~Cache() {
            delete[] setsArray;
        }
};

class CacheSimulator {
 Cache l1;
 Cache l2;
 bool wrAlloc;
 int blockSize;
 int memCyc;
 
 public:
    CacheSimulator(int l1Size, int l1Assoc, int l1Cyc, int l2Size, int l2Assoc, int l2Cyc, bool wrAlloc, int blockSize, int memCyc)
        : l1(l1Size, l1Assoc, l1Cyc), l2(l2Size, l2Assoc, l2Cyc), wrAlloc(wrAlloc), blockSize(blockSize), memCyc(memCyc) {
            this->l1 = Cache(l1Size, l1Assoc, l1Cyc);
            this->l2 = Cache(l2Size, l2Assoc, l2Cyc);

        }

    void read(unsigned address) {
        // Implement read logic
    }

    void write(unsigned address) {
        // Implement write logic
    }

    void stats() {
        // Implement stats logic
    }
};
