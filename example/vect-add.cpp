#include <vector>
#include <iostream>
#include <easyvk.h>
#include <cassert>
#include <vector>

const int size = 4;

int main(int argc, char* argv[]) {
	auto instance = easyvk::Instance(true);
	auto device = instance.devices().at(0);
	// std::cout << "Using device: " << device.properties().deviceName << "\n";

	auto a = easyvk::Buffer(device, size);
	auto b = easyvk::Buffer(device, size);
	auto c = easyvk::Buffer(device, size);

	for (int i = 0; i < size; i++) {
		a.store(i, i);
		b.store(i, i + 1);
		c.store(i, 0);
	}
	std::vector<easyvk::Buffer> bufs = {a, b, c};
	const char* testFile = "build/vect-add.spv";
	std::vector<uint32_t> spvCode =
    #include "vect-add.cinit"
    ;	
	auto program = easyvk::Program(device, spvCode, bufs);
	program.setWorkgroups(size);
	program.setWorkgroupSize(1);
	program.prepare();
	program.run();
	for (int i = 0; i < size; i++) {
		std::cout << "c[" << i << "]: " << c.load(i) << "\n";
		assert(c.load(i) == a.load(i) + b.load(i));
	}
	program.teardown();
	a.teardown();
	b.teardown();
	c.teardown();
	device.teardown();
	instance.teardown();
	return 0;
}
