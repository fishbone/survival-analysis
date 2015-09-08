CXX=g++
CFLAGS=-std=c++11 -msse2 -Wall -finline-functions
LDFLAGS=-lglog
test-model: models/test-model.cpp $(PS_LIB) $(PS_MAIN)
	$(CXX) $(CFLAGS) $(LDFLAGS) -fopenmp $^ -o $@
