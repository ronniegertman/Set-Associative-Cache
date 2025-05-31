
cacheSim: cacheSim.cpp cache.hpp
	g++ -o cacheSim cacheSim.cpp

.PHONY: clean
clean:
	rm -f *.o
	rm -f cacheSim
