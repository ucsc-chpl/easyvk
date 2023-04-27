CXXFLAGS = -std=c++17

all: build easyvk

easyvk: build src/easyvk.cpp src/easyvk.h
	$(CXX) $(CXXFLAGS) -Isrc -c src/easyvk.cpp -o build/easyvk.o
	
build:
	mkdir -p build
