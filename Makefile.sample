CXX=g++
CFLAGS=-std=c++11 -msse2 -Wall -finline-functions
LDFLAGS=-lboost_date_time -lboost_system -lboost_filesystem
#LDFLAGS=-lglog -lprotobuf -lprotoc -lzmq -llz4 -lcityhash 
# test-model: models/test-model.cpp $(PS_LIB) $(PS_MAIN)
# 	$(CXX) $(CFLAGS) $(LDFLAGS) -fopenmp $^ -o $@
INC=-I./include
main: src/main.cpp src/data_io.cpp
	$(CXX) $(CFLAGS) $(LDFLAGS) $(INC) $^ -o $@
