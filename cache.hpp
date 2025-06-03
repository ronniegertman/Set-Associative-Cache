//create a queue:
#include <iostream>
#include <fstream>
#include <queue>
#include <sstream>
#include <iomanip>
#include <stdlib.h>

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
        int blockSize;
        int ways; // number of ways
        int size;
        int cyc;
        int sets; // number of sets
        int hits = 0;
        int misses = 0;
        Set* setsArray;
        Cache(int size, int assoc, int cyc, int blockSize) 
            : blockSize(blockSize),
              size(1 << size),
              cyc(cyc),
              ways(1 << assoc),
              sets((1 << size) / (blockSize * ways))
        {
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
    public:
    Cache l1;
    Cache l2;
    bool wrAlloc;
    int blockSize; // size of cache line in bytes
    int memCyc;
    //l1 assoc is log2(ways)
        CacheSimulator(int l1Size, int l1Assoc, int l1Cyc, int l2Size, int l2Assoc, int l2Cyc, bool wrAlloc, int blockSize, int memCyc)
            : l1(l1Size, l1Assoc, l1Cyc, (1<<blockSize)), l2(l2Size, l2Assoc, l2Cyc, (1<<blockSize)), wrAlloc(wrAlloc), blockSize(blockSize), memCyc(memCyc)  {}

        
        void printCacheTable(const Cache& cache, const std::string& cacheName) {
            std::cout << "Cache Table for " << cacheName << ":\n";
            for (int setIndex = 0; setIndex < cache.sets; ++setIndex) {
                std::cout << "Set " << setIndex << ": ";
                queue<Way> tempQueue = cache.setsArray[setIndex].waysQueue;
                while (!tempQueue.empty()) {
                    Way way = tempQueue.front();
                    tempQueue.pop();
                    if (way.valid) {
                        std::cout << way.tag << " ";
                    } else {
                        std::cout << "Empty ";
                    }
                }
                std::cout << "\n";
            }
            std::cout << std::endl;
        }

        void read(unsigned address) {
            unsigned set_l1 = (address >> blockSize) % l1.sets;
            unsigned set_l2 = (address >> blockSize) % l2.sets;
            unsigned tag_l1 = (address >> blockSize) / l1.sets;
            unsigned tag_l2 = (address >> blockSize) / l2.sets;
            std::cout << "l1 set " << set_l1 << "l1 tag" << tag_l1 <<std::endl;
            std::cout << "l2 set " << set_l2 << "l2 tag" << tag_l2 <<std::endl;
            bool foundInL1 = find(tag_l1, set_l1, l1);
            if (foundInL1) {
                // Hit in L1
                l1.hits++;
                std::cout << "Hit in L1" << std::endl;
                printCacheTable(l1, "L1");
                printCacheTable(l2, "L2");
                return;
            }
            l1.misses++;
            bool foundInL2 = find(tag_l2, set_l2, l2);
            if (foundInL2) {
                // Hit in L2
                l2.hits++;
                std::cout << "Hit in L2" << std::endl;
                // Load to L1
                load(address, l1, 1);
                // Implement logic to load the block from L2 to L1
                printCacheTable(l1, "L1");
                printCacheTable(l2, "L2");
                return;
            }
            // Miss in both caches
            std::cout << "read miss in both caches" << std::endl;

            l2.misses++;
            load(address, l1, 1);
            load(address, l2, 2);
            printCacheTable(l1, "L1");
            printCacheTable(l2, "L2");
        }

        bool find(unsigned tag, unsigned set, Cache& cache) {
            std::cout << "Finding tag: " << tag << " in set: " << set << std::endl;
            queue<Way> temp;
            queue<Way>& ways_queue = cache.setsArray[set].waysQueue;

            bool found = false;
            bool isdirty;
            while (!ways_queue.empty()) {
                // std::cout << "removing way with tag: " << ways_queue.front().tag << std::endl;
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
            if (found) { 
                temp.push(Way(tag, true, isdirty)); // Add the new way to the end of the queue
            }
            // Restore the original queue
            while (!temp.empty()) {
                ways_queue.push(temp.front()); // Restore the original queue
                temp.pop();
            }
            return found; 
        }
        // ...existing code...

        void evictAndSnoop(unsigned address, Cache& cache, int cacheIndex) {
        unsigned set = (address >> blockSize) % cache.sets;
        unsigned tag = (address >> blockSize) / cache.sets;
        // Evict the block from the given cache
        queue<Way>& waysQueue = cache.setsArray[set].waysQueue;
        queue<Way> tempQueue;

        bool found = false;
        while (!waysQueue.empty()) {
            Way way = waysQueue.front();
            waysQueue.pop();
            if (way.valid && way.tag == tag) {
                std::cout << "Evicting block with tag " << way.tag << " from cache index " << cacheIndex << std::endl; // Debug print
                found = true; // Block to evict found
            } else {
                tempQueue.push(way); // Keep other blocks
            }
        }

        // Restore the queue without the evicted block
        while (!tempQueue.empty()) {
            waysQueue.push(tempQueue.front());
            tempQueue.pop();
        }

        if (found) {
            std::cout << "Evicted block with tag " << tag << " from cache index " << cacheIndex << std::endl; // Debug print

            // If evicting from L2, snoop in L1
            if (cacheIndex == 2) {
                std::cout << "Snooping L1 for address " << address << std::endl; // Debug print
                evictAndSnoop(address, l1, 1); // Recursively evict from L1 if needed
            }
        }
        }

        void load(unsigned address, Cache& cache, int cacheIndex = 1) {
        std::cout << "Loading address " << address << " to cache index " << cacheIndex << std::endl; // Debug print
        unsigned set = (address >> blockSize) % cache.sets;
        unsigned tag = (address >> blockSize) / cache.sets;

        // Check if the set is full
        if (cache.setsArray[set].waysQueue.size() >= cache.ways) {
            std::cout << "Set is full, evicting a way" << std::endl; // Debug print
            Way evicted = cache.setsArray[set].waysQueue.front();
            cache.setsArray[set].waysQueue.pop();
            if( evicted.valid && evicted.dirty && cacheIndex == 1) {
                unsigned evictedAddress = (evicted.tag * cache.sets + set) << blockSize;
                unsigned evicted_set_l2 = (evictedAddress >> blockSize) % l2.sets;
                unsigned evicted_tag_l2 = (evictedAddress >> blockSize) / l2.sets;
                find(evicted_tag_l2 ,evicted_set_l2 ,l2); 
            }
            // If evicted block is valid, handle eviction and snooping
            if (evicted.valid && cacheIndex == 2) {
                unsigned evictedAddress = (evicted.tag * cache.sets + set) << blockSize;
                std::cout << "Evicting block with tag " << evicted.tag << " from set " << set << std::endl; // Debug print
                std::cout << "Evicted address: " << evictedAddress << std::endl; // Debug print
                std::cout << "evicted in cacheIndex" << cacheIndex << std::endl; // Debug print
                evictAndSnoop(evictedAddress, l1, 1);
            }
        }

        // Add the new block
        Way newWay(tag, true, false);
        cache.setsArray[set].waysQueue.push(newWay);
        }

        void write(unsigned address) {
            // Implement write logic

            unsigned set_l1 = (address >> blockSize) % l1.sets; 
            unsigned set_l2 = (address >> blockSize) % l2.sets; 
            unsigned tag_l1 = (address >> blockSize) / l1.sets; 
            unsigned tag_l2 = (address >> blockSize) / l2.sets; 

            std::cout << "l1 set " << set_l1 << "l1 tag" << tag_l1 <<std::endl;
            std::cout << "l2 set " << set_l2 << "l2 tag" << tag_l2 <<std::endl;
            // Check L1 cache
            bool found = find(tag_l1, set_l1, l1);

            if (found) {
                l1.hits++;
                // Hit in L1
                std::cout << "Hit in L1 for write" << std::endl;
                // Mark the block as dirty
                Way& way = l1.setsArray[set_l1].waysQueue.back();
                way.dirty = true;
                printCacheTable(l1, "L1");
                printCacheTable(l2, "L2");
                return;
            }
            l1.misses++;
            // Check L2 cache
            found = find(tag_l2, set_l2, l2);
            if (found) {
                l2.hits++;
                // Hit in L2
                std::cout << "Hit in L2 for write" << std::endl;
                // Load to L1 and mark as dirty
                if (wrAlloc) {
                    // with a grain of salt, we change the block only in L1
                    load(address, l1, 1);
                    Way& way = l1.setsArray[set_l1].waysQueue.back();
                    way.dirty = true;
                    printCacheTable(l1, "L1");
                    printCacheTable(l2, "L2");
                    return;
                } else {
                    Way& way = l2.setsArray[set_l2].waysQueue.back();
                    way.dirty = true;
                    printCacheTable(l1, "L1");
                    printCacheTable(l2, "L2");
                    return;
                }              
            }
            // Miss in both caches
            l2.misses++;
            if(wrAlloc) {
                // Write allocate
                load(address, l1, 1);
                Way& way = l1.setsArray[set_l1].waysQueue.back();
                way.dirty = true;
                load(address, l2, 2);
                printCacheTable(l1, "L1");
                printCacheTable(l2, "L2");
            }  
        }

        void stats() {
            // Implement stats logic
            float l1_miss_rate = static_cast<float>(l1.misses) / (l1.hits + l1.misses);
            float l2_miss_rate = static_cast<float>(l2.misses) / (l2.hits + l2.misses);
            int mem_accesss = l2.misses;

            int instructions_counter = l1.hits + l1.misses;
            int l1_time = (l1.hits + l1.misses) * l1.cyc;
            int l2_time = (l2.hits + l2.misses) * l2.cyc;

            int mem_accesss_time = mem_accesss * memCyc;
            int total_time = l1_time + l2_time + mem_accesss_time;
            float avg_time = static_cast<float>(total_time) / instructions_counter;

            // print with 3 digit accuracy
            // std::cout << std::fixed << std::setprecision(3);
            // std::cout << "L1miss=" << l1_miss_rate << " L2miss=" << l2_miss_rate 
            // << " AccTimeAvg=" << avg_time << std::endl;

            printf("L1miss=%.03f ", l1_miss_rate);
	        printf("L2miss=%.03f ", l2_miss_rate);
	        printf("AccTimeAvg=%.03f\n", avg_time);
        }
};
