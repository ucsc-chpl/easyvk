#include <array>
#include <cstdint>
#include <fstream>
#include <set>
#include <stdarg.h>
#include <vector>

#include <vulkan/vulkan.h>
#ifdef __ANDROID__
	#include <android/log.h>
#endif

namespace easyvk {

	const uint32_t push_constant_size_bytes = 20;

	class Device;
	class Buffer;

	class Instance {
		public:
			Instance(bool = false);
			std::vector<VkPhysicalDevice> physicalDevices();
			void teardown();
		private:
			bool enableValidationLayers;
			VkInstance instance;
			VkDebugReportCallbackEXT debugReportCallback;
	};

	class Device {
		public:
			Device(Instance &_instance, VkPhysicalDevice _physicalDevice);
			VkDevice device;
			VkPhysicalDeviceProperties properties;
			uint32_t selectMemory(VkBuffer buffer, VkMemoryPropertyFlags flags);
			uint32_t computeFamilyId = uint32_t(-1);
			VkQueue computeQueue; 
			void teardown();
		private:
			Instance &instance;
			VkPhysicalDevice physicalDevice;
	};

	/**
	 * @brief Represents a buffer for storing data elements in device memory.
	 * 
	 * The Buffer class provides a convenient interface for allocating and interacting with 
	 * VKBuffers tied to device memory. The buffer is implicitly un-typed and the load/store 
	 * methods provide templated views to the underlying buffer.
	 * 
	 * NOTE: The correctness of this implementation relies on whether the OpenCL data types are 
	 * interepreted and represented the same way as on the host device. For example, you 
	 * define a buffer of 256 longs like this:
	 * 
	 *     auto myBuf = Buffer(device, 256, sizeof(long));
	 * 
	 * And you would use myBuf.store<long>(...) and myBuf.load<long>(...) to write/read to the 
	 * buffer from the host. However, if your host device has a specification for that what a 
	 * long is that doesn't match OpenCL's spec, then you are going to get unexpected behavior.
	 * I think the OpenCL spec should match the spec for most modern devices, but you should 
	 * verify to be safe. See http://man.opencl.org/scalarDataTypes.html for how OpenCL 
	 * specifies it's types.
	 */
	class Buffer {
		public:
			Buffer(Device &device, size_t numElements, size_t elementSize);
			VkBuffer buffer;


			// The below load and store implementations use a type template which dictates
			// how the underlying buffer should be interpreted. 
			template <typename T>
			void store(size_t i, T value) {
				*(reinterpret_cast<T*>(data) + i) = value;
			}

			template <typename T>
			T load(size_t i) {
				return *(reinterpret_cast<T*>(data) + i);
			}

			void clear() {
				for (uint32_t i = 0; i < size; i++)
					store(i, 0);
			}

			void teardown();
		private:
			easyvk::Device &device;
			VkDeviceMemory memory;
			uint32_t size;
            uint32_t* data;
	};

	class Program {
		public:
			Program(Device &_device, const char* filepath, std::vector<easyvk::Buffer> &buffers);
			Program(Device &_device, std::vector<uint32_t> spvCode, std::vector<easyvk::Buffer> &buffers);
			void initialize(const char* entry_point);
			void run();
			float runWithDispatchTiming();
			void setWorkgroups(uint32_t _numWorkgroups);
			void setWorkgroupSize(uint32_t _workgroupSize);
			void teardown();
		private:
			std::vector<easyvk::Buffer> &buffers;
			VkShaderModule shaderModule;
			easyvk::Device &device;
			VkDescriptorSetLayout descriptorSetLayout;
			VkDescriptorPool descriptorPool;
			VkDescriptorSet descriptorSet;
			std::vector<VkWriteDescriptorSet> writeDescriptorSets;
			std::vector<VkDescriptorBufferInfo> bufferInfos;
			VkPipelineLayout pipelineLayout;
			VkPipeline pipeline;
			uint32_t numWorkgroups;
			uint32_t workgroupSize;
			VkFence fence;
			VkCommandBuffer commandBuffer;
			VkCommandPool commandPool;
			VkQueryPool timestampQueryPool;
	};

	const char* vkDeviceType(VkPhysicalDeviceType type);
}
