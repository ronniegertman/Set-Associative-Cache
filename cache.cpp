//create a queue:
#include <iostream>
#include <fstream>
#include <queue>
#include <sstream>
using std::queue;
class Way{
    public:
        unsigned tag;
        bool valid;
        bool dirty;
        Way(unsigned tag, bool valid, bool dirty) : tag(tag), valid(valid), dirty(dirty) {}
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
    public:
        int ways; // number of ways
        int size;
        int cyc;
        int sets; // number of sets
        Set* setsArray;
        Cache(int size, int assoc, int cyc) : size(size),cyc(cyc), ways(1 << assoc), sets(size / ways) {
            // assoc is number of bits for ways
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
    int blockSize; // size of cache line in bytes
    int memCyc;
 
    public:
    //l1 assoc is log2(ways)
        CacheSimulator(int l1Size, int l1Assoc, int l1Cyc, int l2Size, int l2Assoc, int l2Cyc, bool wrAlloc, int blockSize, int memCyc)
            : l1(l1Size, l1Assoc, l1Cyc), l2(l2Size, l2Assoc, l2Cyc), wrAlloc(wrAlloc), blockSize(blockSize), memCyc(memCyc)  {}


        void read(unsigned address) {
            unsigned set_l1 = (address / blockSize) % l1.sets; 
            unsigned set_l2 = (address / blockSize) % l2.sets; 
            unsigned tag_l1 = (address / blockSize) / l1.sets; 
            unsigned tag_l2 = (address / blockSize) / l2.sets; 
            bool foundInL1 = find(tag_l1, set_l1, l1);
            if (foundInL1) {
                // Hit in L1
                std::cout << "Hit in L1" << std::endl;
                return;
            }
            bool foundInL2 = find(tag_l2, set_l2, l2);
            if (foundInL2) {
                // Hit in L2
                std::cout << "Hit in L2" << std::endl;
                // Load to L1
                load(tag_l1, set_l1, l1, 1);
                // Implement logic to load the block from L2 to L1
                return;
            }
            // Miss in both caches
            load(tag_l1, set_l1, l1, 1);
            load(tag_l2, set_l2, l2, 2);
        }

        bool find(unsigned tag, unsigned set, Cache& cache) {
            queue<Way> temp;
            queue<Way>& ways_queue = cache.setsArray[set].waysQueue;
            bool found = false;
            bool isdirty;
            while (!ways_queue.empty()) {
                found = false;
                Way way = ways_queue.front();
                ways_queue.pop();
                if (way.valid && way.tag == tag) {
                    // Found the block in cache
                    found = true;
                    isdirty = way.dirty; 
                } else{
                        temp.push(way); // Keep the way in a temporary queue

                }
            }

            temp.push(Way(tag, true, isdirty)); // Add the new way to the end of the queue
            while (!temp.empty()) {
                ways_queue.push(temp.front()); // Restore the original queue
                temp.pop();
            }
            return found; 
        }

        void load(unsigned tag, unsigned set, Cache& cache, int cacheIndex = 1) {
            // Check if the set is full
            if (cache.setsArray[set].waysQueue.size() >= cache.ways) {
                // Evict the least recently used way
                Way evicted = cache.setsArray[set].waysQueue.front();
                cache.setsArray[set].waysQueue.pop();
                if (evicted.dirty) {
                    // Write back to memory if dirty
                    if (cacheIndex == 1) {
                        Cache& cache = l2;
                        CacheSimulator::write(evicted.tag * cache.size + set * cache.ways, cache);
                    } else {
                        std::cout << "Writing back dirty block from L2 to memory" << std::endl;
                    }
                }
             } 
            // Add the new block
            Way newWay(cache.setsArray[set].waysQueue.size(), tag, true, false);
            cache.setsArray[set].waysQueue.push(newWay);
            cache.setsArray[set].size++;
    }   

        void write(unsigned address, Cache& cache) {
            // Implement write logic
        }

        void stats() {
            // Implement stats logic
        }
};
