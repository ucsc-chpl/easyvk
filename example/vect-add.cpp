#include <vector>
#include <iostream>
#include <easyvk.h>
#include <cassert>
#include <vector>

const int size = 1024 * 16;

int main(int argc, char* argv[]) {
	// Initialize instance.
	auto instance = easyvk::Instance(true);
	// Get list of available physical devices.
	auto physicalDevices = instance.physicalDevices();
	// Create device from first physical device.
	auto device = easyvk::Device(instance, physicalDevices.at(0));
	std::cout << "Using device: " << device.properties.deviceName << "\n";

	auto numIters = 1;
	for (int n = 0; n < numIters; n++) {
		// Define the buffers to use in the kernel. 
		auto a = easyvk::Buffer(device, size, sizeof(uint32_t));
		auto b = easyvk::Buffer(device, size, sizeof(double));
		auto c = easyvk::Buffer(device, size, sizeof(double));

		// Write initial values to the buffers.
		for (int i = 0; i < size; i++) {
			// The buffer provides an untyped view of the memory, so you must specify
			// the type when using the load/store methods. 
			a.store<uint32_t>(i, i);
			b.store<double>(i, i + 1);
		}
		c.clear();
		std::vector<easyvk::Buffer> bufs = {a, b, c};

		// Kernel source code can be loaded in two ways: 
		// 1. .spv binary read from file at runtime.
		const char* testFile = "build/vect-add.spv";
		// 2. .spv binary loaded into the executable at compile time.
		std::vector<uint32_t> spvCode =
		#include "build/vect-add.cinit"
		;	
		auto program = easyvk::Program(device, spvCode, bufs);

		program.setWorkgroups(size);
		program.setWorkgroupSize(1);

		// Run the kernel.
		program.initialize("litmus_test");

		program.run();

		// Check the output.
		for (int i = 0; i < size; i++) {
			// std::cout << "c[" << i << "]: " << c.load(i) << "\n";
			assert(c.load<double>(i) == a.load<uint32_t>(i) + b.load<double>(i));
		}

		// Cleanup.
		program.teardown();
		a.teardown();
		b.teardown();
		c.teardown();
	}

	device.teardown();
	instance.teardown();
	return 0;
}
