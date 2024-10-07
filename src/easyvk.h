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

#ifndef EASYVK_H
#define EASYVK_H

#include <array>
#include <cstdint>
#include <fstream>
#include <set>
#include <string>
#include <cstring>
#include <stdarg.h>
#include <vector>
#include <map>
#include <iostream>
#include <stdlib.h>

#include <../volk/volk.h>
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
    uint32_t selectMemory(uint32_t memoryTypeBits, VkMemoryPropertyFlags flags);
    uint32_t computeFamilyId = uint32_t(-1);
    uint32_t subgroupSize();
    const char* vendorName();
    VkQueue computeQueue;
    // AMD shader info extension gives more register info than the portable stats extension
    bool supportsAMDShaderStats;
    void teardown();
  private:
    Instance &instance;
    VkPhysicalDevice physicalDevice;
  };

  class Buffer {
  public:
    Buffer(Device &device, uint64_t sizeBytes, bool deviceLocal = false);
    void teardown();
    void copy(Buffer dst, uint64_t len, uint64_t srcOffset = 0, uint64_t dstOffset = 0);  
    void store(void* src, uint64_t len, uint64_t srcOffset = 0, uint64_t dstOffset = 0);
    void load(void* dst, uint64_t len, uint64_t srcOffset = 0, uint64_t dstOffset = 0);
    void clear();
    void fill(uint32_t word, uint64_t offset = 0);
    void _copy(VkBuffer src, VkBuffer dst, uint64_t len, uint64_t srcOffset = 0, uint64_t dstOffset = 0);
    void _createVkBuffer(VkBuffer* buf, VkDeviceMemory* mem, uint64_t size, VkBufferUsageFlags usage, VkMemoryPropertyFlags props);

    easyvk::Device &device;
    VkCommandPool commandPool;
    VkCommandBuffer commandBuffer;
    VkDeviceMemory memory;
    VkBuffer buffer;
    uint64_t size;
    bool deviceLocal;
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
  class Program {
  public:
    Program(Device &_device, const char *filepath, std::vector<easyvk::Buffer> &buffers);
    Program(Device &_device, std::vector<uint32_t> spvCode, std::vector<easyvk::Buffer> &buffers);
    void initialize(const char *entry_point, VkPipelineShaderStageCreateFlags pipelineFlags = 0);
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

#endif
