CXXFLAGS = -std=c++17

SHADERS = $(wildcard example/*.cl)
SPVS = $(patsubst example/%.cl,build/%.spv,$(SHADERS))
CINITS = $(patsubst example/%.cl,build/%.cinit,$(SHADERS))

.PHONY: all clean easyvk example

all: build easyvk

build:
	mkdir -p build

easyvk: build src/easyvk.cpp src/easyvk.h
	$(CXX) $(CXXFLAGS) -Isrc -c src/easyvk.cpp -o build/easyvk.o

example: build easyvk $(SPVS) $(CINITS)
	$(CXX) $(CXXFLAGS) -Isrc -Ibuild -c example/vect-add.cpp -o build/vect-add.o
	$(CXX) $(CXXFLAGS) build/easyvk.o build/vect-add.o -lvulkan -o build/vect-add

build/%.spv: example/%.cl
	clspv -cl-std=CL2.0 -inline-entry-points $< -o $@

build/%.cinit: example/%.cl
	clspv -cl-std=CL2.0 -inline-entry-points -output-format=c $< -o $@
	
clean:
	rm -rf build
