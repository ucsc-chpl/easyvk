#include <vector>
#include <iostream>
#include <easyvk.h>
#include <cassert>
#include <vector>

const int size = 4;

int main(int argc, char* argv[]) {
	// Initialize 
	auto instance = easyvk::Instance(true);
	auto device = instance.devices().at(0);
	std::cout << "Using device: " << device.properties.deviceName << "\n";

	// Create GPU buffers.
	auto a = easyvk::Buffer(device, size);
	auto b = easyvk::Buffer(device, size);
	auto c = easyvk::Buffer(device, size);

	// Write initial values to the buffers.
	for (int i = 0; i < size; i++) {
		a.store(i, i);
		b.store(i, i + 1);
		c.store(i, 0);
	}
	std::vector<easyvk::Buffer> bufs = {a, b, c};

	// Kernel source code can be loaded in two ways: 
	// 1. .spv binary read from file at runtime.
	const char* testFile = "build/vect-add.spv";
	// 2. .spv binary loaded into the executable at compile time.
	std::vector<uint32_t> spvCode =
	#include "vect-add.cinit"
	;	
	auto program = easyvk::Program(device, spvCode, bufs);
	// auto program = easyvk::Program(device, testFile, bufs);

	// Dispatch 4 work groups of size 1 to carry out the work.
	program.setWorkgroups(size);
	program.setWorkgroupSize(1);

	// Run the kernel.
	program.prepare();
	program.run();

	// Print and check the output.
	for (int i = 0; i < size; i++) {
		std::cout << "c[" << i << "]: " << c.load(i) << "\n";
		assert(c.load(i) == a.load(i) + b.load(i));
	}

	// Cleanup.
	program.teardown();
	a.teardown();
	b.teardown();
	c.teardown();
	device.teardown();
	instance.teardown();
	return 0;
}
