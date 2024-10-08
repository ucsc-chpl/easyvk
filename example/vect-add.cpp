/*
   Copyright 2023 Reese	Levine,	Devon McKee, Sean Siddens

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
*/

#include <vector>
#include <iostream>
#include <easyvk.h>
#include <cassert>
#include <vector>

const int size = 1024 * 16;

int main() {
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
		auto a = easyvk::Buffer(device, size * sizeof(uint32_t));
		auto b = easyvk::Buffer(device, size * sizeof(float));
		auto c = easyvk::Buffer(device, size * sizeof(float));

		// Write initial values to the buffers.
		printf("Setting up host buffers...\n");
		std::vector<uint32_t> a_host;
		std::vector<float> b_host;
		for (int i = 0; i < size; i++) {
			// The buffer provides an untyped view of the memory, so you must specify
			// the type when using the load/store methods. 
			a_host.push_back(i);
			b_host.push_back(i + 1);
		}

		printf("Loading host buffers to device...\n");
		a.store(a_host.data(), size * sizeof(uint32_t));
		b.store(b_host.data(), size * sizeof(float));

		printf("Setting up program...\n");
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
		printf("Running program...\n");
		program.initialize("litmus_test");
		float runtime = program.runWithDispatchTiming();
		printf("Performed vector add in %.5fms\n", runtime / 1000000.0);

		// Check the output.
		printf("Loading results from device...\n");
		std::vector<float> c_host(size);
		c.load(c_host.data(), size * sizeof(float));
		printf("Checking results...\n");
		for (int i = 0; i < size; i++) {
			//printf("%d : %d + %f = %f\n", i, a_host[i], b_host[i], c_host[i]);
			assert(c_host[i] == a_host[i] + b_host[i]);
		}

		printf("Vector add completed successfully!\n");

		// Cleanup.
		a.teardown();
		b.teardown();
		c.teardown();
		program.teardown();
	}

	device.teardown();
	instance.teardown();
	return 0;
}
