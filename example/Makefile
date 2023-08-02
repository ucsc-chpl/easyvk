CXXFLAGS = -std=c++17
CLSPVFLAGS = --cl-std=CL2.0 --spv-version=1.3 --inline-entry-points

SHADERS = $(wildcard *.cl)
SPVS = $(patsubst %.cl,build/%.spv,$(SHADERS))
CINITS = $(patsubst %.cl,build/%.cinit,$(SHADERS))

.PHONY: all build clean easyvk example

all: build easyvk example

build:
	mkdir -p build
	mkdir -p build/android

easyvk:
	make -C ../

example: easyvk vect-add.cpp $(SPVS) $(CINITS)
	$(CXX) $(CXXFLAGS) -I../src ../build/easyvk.o  vect-add.cpp -lvulkan -o build/vect-add.run

android: build easyvk
	ndk-build APP_BUILD_SCRIPT=./Android.mk  NDK_PROJECT_PATH=. NDK_APPLICATION_MK=./Application.mk NDK_LIBS_OUT=./build/android/libs NDK_OUT=./build/android/obj

build/%.spv: %.cl
	clspv $(CLSPVFLAGS) $< -o $@

build/%.cinit: %.cl
	clspv $(CLSPVFLAGS) --output-format=c  $< -o $@

clean:
	rm -rf build
