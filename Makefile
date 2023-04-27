CXXFLAGS = -std=c++17

all: build easyvk

build:
	mkdir -p build

easyvk: build src/easyvk.cpp src/easyvk.h
	$(CXX) $(CXXFLAGS) -Isrc -c src/easyvk.cpp -o build/easyvk.o
	
clean:
	rm -r build
