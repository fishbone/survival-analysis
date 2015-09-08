CXX=g++
CFLAGS=-std=c++11 -msse2 -Wall -finline-functions
LDFLAGS=-lglog -lprotobuf -lprotoc -lzmq -llz4 -lcityhash 
test-model: models/test-model.cpp $(PS_LIB) $(PS_MAIN)
	$(CXX) $(CFLAGS) $(LDFLAGS) -fopenmp $^ -o $@
