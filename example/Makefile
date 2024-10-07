CXXFLAGS = -std=c++20
CLSPVFLAGS = --cl-std=CL2.0 --spv-version=1.3 --inline-entry-points

SHADERS = $(wildcard *.cl)
SPVS = $(patsubst %.cl,build/%.spv,$(SHADERS))
CINITS = $(patsubst %.cl,build/%.cinit,$(SHADERS))

RUN_EXT = run
ifeq ($(OS), Windows_NT)
	RUN_EXT = exe
endif

.PHONY: all build clean easyvk example

all: build easyvk example

build:
ifeq ($(OS), Windows_NT)
	if not exist "build" mkdir build
	if not exist "build\android" mkdir build\android
else
	mkdir -p build
	mkdir -p build/android
endif

easyvk:
	make -C ../

example: build easyvk vect-add.cpp $(SPVS) $(CINITS)
	$(CXX) $(CXXFLAGS) -I../src ../volk/volk.c ../build/easyvk.o vect-add.cpp -L$(VULKAN_SDK)/Lib -lvulkan -o build/vect-add.$(RUN_EXT)

android: build easyvk
	ndk-build APP_BUILD_SCRIPT=./Android.mk  NDK_PROJECT_PATH=. NDK_APPLICATION_MK=./Application.mk NDK_LIBS_OUT=./build/android/libs NDK_OUT=./build/android/obj

build/%.spv: %.cl
	clspv $(CLSPVFLAGS) $< -o $@

build/%.cinit: %.cl
	clspv $(CLSPVFLAGS) --output-format=c  $< -o $@

clean:
	rm -rf build
