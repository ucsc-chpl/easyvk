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

#include <array>
#include <cstdint>
#include <fstream>
#include <set>
#include <string>
#include <stdarg.h>
#include <vector>
#include <map>

#include <vulkan/vulkan.h>
#ifdef __ANDROID__
#include <android/log.h>
#endif

namespace easyvk
{

  const uint32_t push_constant_size_bytes = 20;

  class Device;
  class Buffer;

  class Instance
  {
  public:
    Instance(bool = false);
    std::vector<VkPhysicalDevice> physicalDevices();
    void teardown();

  private:
    bool enableValidationLayers;
    VkInstance instance;
    VkDebugReportCallbackEXT debugReportCallback;
  };

  class Device
  {
  public:
    Device(Instance &_instance, VkPhysicalDevice _physicalDevice);
    VkDevice device;
    VkPhysicalDeviceProperties properties;
    uint32_t selectMemory(VkBuffer buffer, VkMemoryPropertyFlags flags);
    uint32_t computeFamilyId = uint32_t(-1);
    uint32_t subgroupSize();
    VkQueue computeQueue;
    // AMD shader info extension gives more register info than the portable stats extension
    bool supportsAMDShaderStats;
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

  typedef struct BufferParams
  {
    void* HostBuffer;
    size_t numElements;
    size_t elementSize;
    bool deviceLocal;
  } BufferParams;

  class Buffer
  {
  public:
    // HostBuffer would be a pointer to a <vector> type so i.e. vector.data()
    Buffer(Device &device, void* HostBuffer, size_t numElements, size_t elementSize); // I don't know if the easyvk paradigm doesn't want program
    Buffer(Device &device, BufferParams params);                                      // as part of the class Buffer.

    //void load(easyvk::Device &_device, void* HostBuffer, uint32_t size);
    VkBuffer buffer;
    VkBuffer staging;

    /**
     * Returns the total size of the underlying buffer (in bytes).
     */
    size_t size() const
    {
      return _numElements * _elementSize;
    }

    /** Returns the device address of this buffer. */
    //uint64_t device_addr();

    void teardown();
    void load(easyvk::Device &_device, std::vector<uint> &HostBuffer);



  private:
    easyvk::Device &device;
    VkDeviceMemory memory;
    VkBuffer getNewBuffer(easyvk::Device &_device, void* HostBuffer, uint32_t size);
    void CopyBuffer(easyvk::Device &_device, VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
    void createBuffer(easyvk::Device &_device, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory); 
    bool deviceLocal; // specifies whether the memory can be mapped on the host
    size_t _numElements;
    size_t _elementSize;
    void *HostBuffer;
    //void *data;
  };

  typedef struct ShaderStatistics {
    std::string name;
    std::string description;
    size_t format; // 0 -> bool, 1 -> int64, 2 -> uint64, 3 -> float64
    uint64_t value; // may need to cast this to get the right value based on the format
  } ShaderStatistics;

  /**
   * A program consists of shader code and the buffers/inputs to the shader
   * Buffers should be passed in according to their argument order in the shader.
   * Workgroup memory buffers are indexed from 0.
   */
  class Program
  {
  public:
    Program(Device &_device, const char *filepath, std::vector<easyvk::Buffer> &buffers);
    Program(Device &_device, std::vector<uint32_t> spvCode, std::vector<easyvk::Buffer> &buffers);
    void initialize(const char *entry_point);
    std::vector<ShaderStatistics> getShaderStats();
    void run();
    float runWithDispatchTiming();
    void setWorkgroups(uint32_t _numWorkgroups);
    void setWorkgroupSize(uint32_t _workgroupSize);
    void setWorkgroupMemoryLength(uint32_t length, uint32_t index);
    void teardown();

  private:
    std::vector<easyvk::Buffer> &buffers;
    std::map<uint32_t, uint32_t> workgroupMemoryLengths;
    VkShaderModule shaderModule;
    easyvk::Device &device;
    VkDescriptorSetLayout descriptorSetLayout;
    VkDescriptorPool descriptorPool;
    VkDescriptorSet descriptorSet;
    std::vector<VkWriteDescriptorSet> writeDescriptorSets;
    std::vector<VkDescriptorBufferInfo> bufferInfos;
    VkPipelineLayout pipelineLayout;
    VkPipeline pipeline;
    VkCommandPool commandPool; 
    uint32_t numWorkgroups;
    uint32_t workgroupSize;
    VkFence fence;
    VkCommandBuffer commandBuffer;
    VkQueryPool timestampQueryPool;
  };

  const char *vkDeviceType(VkPhysicalDeviceType type);
}
