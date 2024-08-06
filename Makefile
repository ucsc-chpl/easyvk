CXXFLAGS = -std=c++20
.PHONY: all clean easyvk

all: build easyvk

build:
ifeq ($(OS),Windows_NT)
	mkdir build\android
else 
	mkdir -p build/android
endif

easyvk: build src/easyvk.cpp src/easyvk.h
	$(CXX) $(CXXFLAGS) -Isrc -c src/easyvk.cpp -o build/easyvk.o 

android: build
	ndk-build APP_BUILD_SCRIPT=./Android.mk  NDK_PROJECT_PATH=. NDK_APPLICATION_MK=./Application.mk NDK_LIBS_OUT=./build/android/libs NDK_OUT=./build/android/obj
	
clean:
	rm -rf build
