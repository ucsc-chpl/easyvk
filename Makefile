CXXFLAGS = -std=c++17

.PHONY: all clean easyvk

all: build easyvk

build:
	mkdir -p build
	mkdir -p build/android

easyvk: build src/easyvk.cpp src/easyvk.h
	$(CXX) $(CXXFLAGS) -Isrc -c src/easyvk.cpp -o build/easyvk.o

android: build
	ndk-build APP_BUILD_SCRIPT=./Android.mk  NDK_PROJECT_PATH=. NDK_APPLICATION_MK=./Application.mk NDK_LIBS_OUT=./build/android/libs NDK_OUT=./build/android/obj
	
clean:
	rm -rf build
