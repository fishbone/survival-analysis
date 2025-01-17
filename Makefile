CXX=g++
CFLAGS=-std=c++11  -g -ggdb -fopenmp  -O3
LDFLAGS=-lboost_date_time -lboost_system -lboost_filesystem -lboost_program_options -lboost_thread
#LDFLAGS=-lglog -lprotobuf -lprotoc -lzmq -llz4 -lcityhash 
# test-model: models/test-model.cpp $(PS_LIB) $(PS_MAIN)
# 	$(CXX) $(CFLAGS) $(LDFLAGS) -fopenmp $^ -o $@
INC=-I./include -I./models
obj = $(patsubst %.cpp,%.o,$(wildcard models/*.cpp) $(wildcard src/*.cpp))

main: $(obj)
	$(CXX) $(CFLAGS) $(LDFLAGS) $(INC) $(notdir $(obj)) -o $@
$(obj): %.o: %.cpp
	$(CXX) $(CFLAGS) $(LDFLAGS) $(INC) -c $^ 



.PHONY: clean
clean:
	rm *.o main
