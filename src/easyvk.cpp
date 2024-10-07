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

#include "easyvk.h"

// TODO: extend this to include ios logging lib
void evk_log(const char *fmt, ...) {
  va_list args;
  va_start(args, fmt);
#ifdef __ANDROID__
  __android_log_vprint(ANDROID_LOG_INFO, "EasyVK", fmt, args);
#else
  vprintf(fmt, args);
#endif
  va_end(args);
}

// Would use string_VkResult() for this but vk_enum_string_helper.h is no more...
inline const char* vkResultString(VkResult res) {
  switch (res) {
  // 1.0
  case VK_SUCCESS:
    return "VK_SUCCESS";
    break;
  case VK_NOT_READY:
    return "VK_NOT_READY";
    break;
  case VK_TIMEOUT:
    return "VK_TIMEOUT";
    break;
  case VK_EVENT_SET:
    return "VK_EVENT_SET";
    break;
  case VK_EVENT_RESET:
    return "VK_EVENT_RESET";
    break;
  case VK_INCOMPLETE:
    return "VK_INCOMPLETE";
    break;
  case VK_ERROR_OUT_OF_HOST_MEMORY:
    return "VK_ERROR_OUT_OF_HOST_MEMORY";
    break;
  case VK_ERROR_OUT_OF_DEVICE_MEMORY:
    return "VK_ERROR_OUT_OF_DEVICE_MEMORY";
    break;
  case VK_ERROR_INITIALIZATION_FAILED:
    return "VK_ERROR_INITIALIZATION_FAILED";
    break;
  case VK_ERROR_DEVICE_LOST:
    return "VK_ERROR_DEVICE_LOST";
    break;
  case VK_ERROR_MEMORY_MAP_FAILED:
    return "VK_ERROR_MEMORY_MAP_FAILED";
    break;
  case VK_ERROR_LAYER_NOT_PRESENT:
    return "VK_ERROR_LAYER_NOT_PRESENT";
    break;
  case VK_ERROR_EXTENSION_NOT_PRESENT:
    return "VK_ERROR_EXTENSION_NOT_PRESENT";
    break;
  case VK_ERROR_FEATURE_NOT_PRESENT:
    return "VK_ERROR_FEATURE_NOT_PRESENT";
    break;
  case VK_ERROR_INCOMPATIBLE_DRIVER:
    return "VK_ERROR_INCOMPATIBLE_DRIVER";
    break;
  case VK_ERROR_TOO_MANY_OBJECTS:
    return "VK_ERROR_TOO_MANY_OBJECTS";
    break;
  case VK_ERROR_FORMAT_NOT_SUPPORTED:
    return "VK_ERROR_FORMAT_NOT_SUPPORTED";
    break;
  case VK_ERROR_FRAGMENTED_POOL:
    return "VK_ERROR_FRAGMENTED_POOL";
    break;
  case VK_ERROR_UNKNOWN:
    return "VK_ERROR_UNKNOWN";
    break;
  // 1.1
  case VK_ERROR_OUT_OF_POOL_MEMORY:
    return "VK_ERROR_OUT_OF_POOL_MEMORY";
    break;
  case VK_ERROR_INVALID_EXTERNAL_HANDLE:
    return "VK_ERROR_INVALID_EXTERNAL_HANDLE";
    break;
  // 1.2
  case VK_ERROR_FRAGMENTATION:
    return "VK_ERROR_FRAGMENTATION";
    break;
  case VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS:
    return "VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS";
    break;
  // 1.3
  // case VK_PIPELINE_COMPILE_REQUIRED: return "VK_PIPELINE_COMPILE_REQUIRED"; break;
  default:
    return "UNKNOWN_ERROR";
    break;
  }
}

// Returns readable vendor name from vendorID, based on vulkan.gpuinfo.org entries
inline const char* vkVendorName(uint32_t vid) {
  switch(vid) {
    case 0x10DE:
      return "NVIDIA";
      break;
    case 0x1002:
      return "AMD";
      break;
    case 0x8086:
      return "Intel";
      break;
    case 0x106B:
      return "Apple";
      break;
    case 0x13B5:
      return "ARM";
      break;
    case 0x5143:
      return "Qualcomm";
      break;
    default:
      return "UNKNOWN";
      break;
  }
}

// Macro for checking Vulkan callbacks
inline void vkAssert(VkResult result, const char *file, int line, bool abort = true) {
  if (result != VK_SUCCESS) {
    evk_log("vkAssert: ERROR %s in '%s', line %d\n", vkResultString(result), file, line);
    if (abort) {
			exit(1);
		}
  }
}
#define vkCheck(result) { vkAssert((result), __FILE__, __LINE__); }

namespace easyvk {
  const char *vkDeviceType(VkPhysicalDeviceType type) {
    switch (type) {
    case VK_PHYSICAL_DEVICE_TYPE_OTHER:
      return "VK_PHYSICAL_DEVICE_TYPE_OTHER";
      break;
    case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
      return "VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU";
      break;
    case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
      return "VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU";
      break;
    case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU:
      return "VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU";
      break;
    case VK_PHYSICAL_DEVICE_TYPE_CPU:
      return "VK_PHYSICAL_DEVICE_TYPE_CPU";
      break;
    default:
      return "UNKNOWN_DEVICE_TYPE";
      break;
    }
  }

  static auto VKAPI_ATTR debugReporter(VkDebugReportFlagsEXT, VkDebugReportObjectTypeEXT, uint64_t, size_t, int32_t, const char *pLayerPrefix, const char *pMessage, void *pUserData) -> VkBool32 {
    std::cerr << "\x1B[31m[Vulkan:" << pLayerPrefix << "]\033[0m " << pMessage << "\n";
    return VK_FALSE;
  }

  Instance::Instance(bool _enableValidationLayers) {
    enableValidationLayers = _enableValidationLayers;
    std::vector<const char *> enabledLayers;
    std::vector<const char *> enabledExtensions;
    vkCheck(volkInitialize());
    if (enableValidationLayers) {
      enabledLayers.push_back("VK_LAYER_KHRONOS_validation");
      enabledExtensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
    }
#ifdef __APPLE__
    enabledExtensions.push_back("VK_KHR_portability_enumeration");
#endif

    // Define app information
    VkApplicationInfo appInfo{
      VK_STRUCTURE_TYPE_APPLICATION_INFO,
      nullptr,
      "EasyVK Application",
      0,
      "Heterogeneous Programming Group",
      0,
      VK_API_VERSION_1_3
    };

#ifdef __APPLE__
    VkInstanceCreateFlags instanceCreateFlags = VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
#else
    VkInstanceCreateFlags instanceCreateFlags = 0;
#endif

    // Define instance create info
    VkInstanceCreateInfo createInfo{
        VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        nullptr,
        instanceCreateFlags,
        &appInfo,
        (uint32_t)(enabledLayers.size()),
        enabledLayers.data(),
        (uint32_t)(enabledExtensions.size()),
        enabledExtensions.data()};

    // Create instance
    vkCheck(vkCreateInstance(&createInfo, nullptr, &instance));
    volkLoadInstance(instance);

    if (enableValidationLayers) {
      VkDebugReportCallbackCreateInfoEXT debugCreateInfo{
          VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT,
          nullptr,
          VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT | VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT,
          debugReporter};
      // Load debug report callback extension
      auto createFN = PFN_vkCreateDebugReportCallbackEXT(vkGetInstanceProcAddr(instance, "vkCreateDebugReportCallbackEXT"));
      if (createFN) {
        createFN(instance, &debugCreateInfo, nullptr, &debugReportCallback);
      }
    }

    // Print out vulkan's instance version
    uint32_t version;
    PFN_vkEnumerateInstanceVersion my_EnumerateInstanceVersion = (PFN_vkEnumerateInstanceVersion)vkGetInstanceProcAddr(VK_NULL_HANDLE, "vkEnumerateInstanceVersion");
    if (nullptr != my_EnumerateInstanceVersion) {
      my_EnumerateInstanceVersion(&version);
    }
  }

  std::vector<VkPhysicalDevice> Instance::physicalDevices() {
    // Get physical device count
    uint32_t deviceCount = 0;
    vkCheck(vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr));

    // Enumerate physical devices based on deviceCount
    std::vector<VkPhysicalDevice> physicalDevices(deviceCount);
    vkCheck(vkEnumeratePhysicalDevices(instance, &deviceCount, physicalDevices.data()));

    return physicalDevices;
  }

  void Instance::teardown() {
    // Destroy debug report callback extension
    if (enableValidationLayers) {
      auto destroyFn = PFN_vkDestroyDebugReportCallbackEXT(vkGetInstanceProcAddr(instance, "vkDestroyDebugReportCallbackEXT"));
      if (destroyFn) 
        destroyFn(instance, debugReportCallback, nullptr);
    }
    // Destroy instance
    vkDestroyInstance(instance, nullptr);
  }

  uint32_t getComputeFamilyId(VkPhysicalDevice physicalDevice) {
    // Get queue family count
    uint32_t queueFamilyPropertyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyPropertyCount, nullptr);

    std::vector<VkQueueFamilyProperties> familyProperties(queueFamilyPropertyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyPropertyCount, familyProperties.data());

    uint32_t i = 0;
    uint32_t computeFamilyId = -1;

    // Get compute family id based on size of family properties
    for (auto queueFamily : familyProperties) {
      if (queueFamily.queueCount > 0 && (queueFamily.queueFlags & VK_QUEUE_COMPUTE_BIT)) {
        computeFamilyId = i;
        break;
      }
      i++;
    }
    return computeFamilyId;
  }

  Device::Device(easyvk::Instance &_instance, VkPhysicalDevice _physicalDevice) : instance(_instance),
                                                                                  physicalDevice(_physicalDevice),
                                                                                  computeFamilyId(getComputeFamilyId(_physicalDevice)) {

    auto priority = float(1.0);
    auto queues = std::array<VkDeviceQueueCreateInfo, 1>{};

    // Define device queue info
    queues[0] = VkDeviceQueueCreateInfo{
      VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
      nullptr,
      VkDeviceQueueCreateFlags{},
      computeFamilyId,
      1,
      &priority
    };

    // check for support for extensions
    uint32_t pPropertyCount;
    vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &pPropertyCount, nullptr);
    std::vector<VkExtensionProperties> extensions(pPropertyCount);
    vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &pPropertyCount, extensions.data()); 
    supportsAMDShaderStats = false;
    
    std::vector<const char *> enabledExtensions{};
    for (const auto& extension : extensions) {
      if (strcmp(extension.extensionName, "VK_AMD_shader_info") == 0) {
        enabledExtensions.push_back(VK_AMD_SHADER_INFO_EXTENSION_NAME);
      } else if (strcmp(extension.extensionName, "VK_KHR_pipeline_executable_properties") == 0) {
        enabledExtensions.push_back(VK_KHR_PIPELINE_EXECUTABLE_PROPERTIES_EXTENSION_NAME);
      } else if (strcmp(extension.extensionName, "VK_KHR_portability_subset") == 0) {
        enabledExtensions.push_back("VK_KHR_portability_subset");
      } else if (strcmp(extension.extensionName, "VK_KHR_shader_non_semantic_info") == 0) {
        enabledExtensions.push_back("VK_KHR_shader_non_semantic_info");
      }
    }

    // enable support for computeFullSubgroups
    VkPhysicalDeviceVulkan13Features vulkan13Features = {};
    vulkan13Features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES;
    vulkan13Features.pNext = nullptr; 

    // enable pipeline executable properties reporting
    VkPhysicalDevicePipelineExecutablePropertiesFeaturesKHR pipelineProperties = {};
    pipelineProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PIPELINE_EXECUTABLE_PROPERTIES_FEATURES_KHR;
    pipelineProperties.pNext = &vulkan13Features;

    // mostly for enabling buffer device addresses
    VkPhysicalDeviceVulkan12Features vulkan12Features = {};
    vulkan12Features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES;
    vulkan12Features.pNext = &pipelineProperties;
    VkPhysicalDeviceFeatures2 features2 = {};
    features2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
    features2.pNext = &vulkan12Features;
    vkGetPhysicalDeviceFeatures2(physicalDevice, &features2);
    features2.features.robustBufferAccess = false;

    // Define device info
    VkDeviceCreateInfo deviceCreateInfo;
    deviceCreateInfo = {
        VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
        &vulkan12Features,
        VkDeviceCreateFlags{},
        1,
        queues.data(),
        0,
        nullptr,
        (uint32_t)enabledExtensions.size(),
        enabledExtensions.data(),
        &features2.features};

    // Create device
    vkCheck(vkCreateDevice(physicalDevice, &deviceCreateInfo, nullptr, &device));

    // Get queue handle.
    vkGetDeviceQueue(device, computeFamilyId, 0, &computeQueue);

    // Get device properties
    vkGetPhysicalDeviceProperties(physicalDevice, &properties);
  }

  uint32_t Device::selectMemory(uint32_t memoryTypeBits, VkMemoryPropertyFlags flags) {
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);

    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
      if ((memoryTypeBits & (1u << i)) && ((flags & memProperties.memoryTypes[i].propertyFlags) == flags)) {
        return i;
      }
    }
    return uint32_t(-1);
  }

  uint32_t Device::subgroupSize() {
    VkPhysicalDeviceSubgroupProperties subgroupProperties;
    subgroupProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SUBGROUP_PROPERTIES;
    subgroupProperties.pNext = NULL;

    VkPhysicalDeviceProperties2 physicalDeviceProperties;
    physicalDeviceProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
    physicalDeviceProperties.pNext = &subgroupProperties;

    vkGetPhysicalDeviceProperties2(physicalDevice, &physicalDeviceProperties);
    return subgroupProperties.subgroupSize;
  }

  const char* Device::vendorName() {
    return vkVendorName(properties.vendorID);
  }

  void Device::teardown() {
    vkDestroyDevice(device, nullptr);
  }


// -------------------------------------------------------------------------------

  Buffer::Buffer(Device &device, uint64_t sizeBytes, bool deviceLocal) : device(device), size(sizeBytes), deviceLocal(deviceLocal) {
    // Create VkBuffer    
    VkMemoryPropertyFlags memProp = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
    if (deviceLocal) {
      memProp = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;  
    }  
    VkBufferUsageFlags usage = 
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT 
      | VK_BUFFER_USAGE_TRANSFER_DST_BIT 
      | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
    _createVkBuffer(&buffer, &memory, sizeBytes, usage, memProp);
   
    // Create command pool for copy commands
    VkCommandPoolCreateInfo commandPoolCreateInfo {
      .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
      .pNext = nullptr,
      .flags = 0,
      .queueFamilyIndex = device.computeFamilyId
    };
    vkCheck(vkCreateCommandPool(device.device, &commandPoolCreateInfo, nullptr, &commandPool));

		// Allocate command buffer from command pool
		VkCommandBufferAllocateInfo commandBufferAllocInfo {
			.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
			.commandPool = commandPool,
			.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
			.commandBufferCount = 1
		};
		vkCheck(vkAllocateCommandBuffers(device.device, &commandBufferAllocInfo, &commandBuffer));
  }

  void Buffer::_createVkBuffer(VkBuffer* buf, VkDeviceMemory* mem, uint64_t size, VkBufferUsageFlags usage, VkMemoryPropertyFlags props) {
    // Creating VkBuffer    
    VkBufferCreateInfo bufferInfo {
      .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
      .size = size,
      .usage = usage,
      .sharingMode = VK_SHARING_MODE_EXCLUSIVE
    };
    vkCheck(vkCreateBuffer(device.device, &bufferInfo, nullptr, buf));
    
    // Allocating memory to it
    VkMemoryRequirements memReqs;
    vkGetBufferMemoryRequirements(device.device, *buf, &memReqs);
    VkMemoryAllocateInfo allocInfo {
      .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
      .allocationSize = memReqs.size,
      .memoryTypeIndex = device.selectMemory(memReqs.memoryTypeBits, props)
    }; 
    vkCheck(vkAllocateMemory(device.device, &allocInfo, nullptr, mem));
    vkCheck(vkBindBufferMemory(device.device, *buf, *mem, 0));
  }

  void Buffer::teardown() {
		vkFreeCommandBuffers(device.device, commandPool, 1, &commandBuffer);
		vkDestroyCommandPool(device.device, commandPool, nullptr);
		vkFreeMemory(device.device, memory, nullptr);
    vkDestroyBuffer(device.device, buffer, nullptr);
  }

  void Buffer::_copy(VkBuffer src, VkBuffer dst, uint64_t len, uint64_t srcOffset, uint64_t dstOffset) {
    // Begin recording command buffer, record command to copy buffer to buffer, end command buffer record
    VkCommandBufferBeginInfo beginInfo {
      .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
      .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT
    };
    vkCheck(vkBeginCommandBuffer(commandBuffer, &beginInfo));
    VkBufferCopy copyRegion {
      .srcOffset = srcOffset,
      .dstOffset = dstOffset,
      .size = len
    };
    vkCmdCopyBuffer(commandBuffer, src, dst, 1, &copyRegion);
    vkCheck(vkEndCommandBuffer(commandBuffer));

    // Submit command buffer to queue, wait for completion
    VkSubmitInfo submitInfo {
      .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
      .commandBufferCount = 1,
      .pCommandBuffers = &commandBuffer
    };
    vkCheck(vkQueueSubmit(device.computeQueue, 1, &submitInfo, VK_NULL_HANDLE));
    vkCheck(vkQueueWaitIdle(device.computeQueue));
		
		// Reset command pool (and all buffers in it) for next use
    vkCheck(vkResetCommandPool(device.device, commandPool, 0));
  }

  void Buffer::copy(Buffer dst, uint64_t len, uint64_t srcOffset, uint64_t dstOffset) {
    _copy(buffer, dst.buffer, len, srcOffset, dstOffset);
  }

  void Buffer::store(void* src, uint64_t len, uint64_t srcOffset, uint64_t dstOffset) {
    if (deviceLocal) {
      // Allocate staging buffer of copy size
      VkBuffer staging;
      VkDeviceMemory stagingMemory;
      _createVkBuffer(&staging, &stagingMemory, len, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT); 

      // Copy src region to staging buffer region
      void* stagingPtr;
      vkCheck(vkMapMemory(device.device, stagingMemory, 0, len, 0, &stagingPtr));
      memcpy(stagingPtr, (char*)src + srcOffset, len);
      vkUnmapMemory(device.device, stagingMemory);
      
      // Copy staging buffer region to device local buffer region
      _copy(staging, buffer, len, 0, dstOffset);
      
      // Free staging buffer
      vkFreeMemory(device.device, stagingMemory, nullptr);
      vkDestroyBuffer(device.device, staging, nullptr);
    } else {
      // Map host visible buffer, copy memory, unmap
      void* bufferPtr;
      vkCheck(vkMapMemory(device.device, memory, dstOffset, len, 0, &bufferPtr));
      memcpy(bufferPtr, (char*)src + srcOffset, len); 
      vkUnmapMemory(device.device, memory);
    }
  }

  void Buffer::load(void* dst, uint64_t len, uint64_t srcOffset, uint64_t dstOffset) {
    if (deviceLocal) {
      VkBuffer staging;
      VkDeviceMemory stagingMemory;
      _createVkBuffer(&staging, &stagingMemory, len, VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);      

      // Copy device local buffer region to staging buffer region
      _copy(buffer, staging, len, srcOffset, 0);
      
      // Copy staging buffer region to dst region (assumes memory allocated in dst)
      void* stagingPtr;
      vkCheck(vkMapMemory(device.device, stagingMemory, 0, len, 0, &stagingPtr));
      memcpy((char*)dst + dstOffset, (char*)stagingPtr, len);
      vkUnmapMemory(device.device, stagingMemory);
      
      // Free staging buffer
      vkFreeMemory(device.device, stagingMemory, nullptr);
      vkDestroyBuffer(device.device, staging, nullptr);
    } else {
      // Map host visible buffer, copy memory, unmap
      void* bufferPtr;
      vkCheck(vkMapMemory(device.device, memory, srcOffset, len, 0, &bufferPtr));
      memcpy((char*)dst + dstOffset, (char*)bufferPtr, len); 
    }
  }

  void Buffer::fill(uint32_t word, uint64_t offset) {
    // Begin command buffer, encode commands to fill buffer and staging buffer with word
    VkCommandBufferBeginInfo beginInfo {
      .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
      .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT
    };
    vkCheck(vkBeginCommandBuffer(commandBuffer, &beginInfo));
    vkCmdFillBuffer(commandBuffer, buffer, offset, size, word);
    vkCheck(vkEndCommandBuffer(commandBuffer));

    // Submit command buffer to queue, wait for completion
    VkSubmitInfo submitInfo {
      .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
      .commandBufferCount = 1,
      .pCommandBuffers = &commandBuffer
    };	
    vkCheck(vkQueueSubmit(device.computeQueue, 1, &submitInfo, VK_NULL_HANDLE));
    vkCheck(vkQueueWaitIdle(device.computeQueue));

    // Reset command pool (and all buffers in it) for next use
    vkCheck(vkResetCommandPool(device.device, commandPool, 0));
  }

  void Buffer::clear() {
    fill(0);	
  }

  // -------------------------------------------------------------------------------

  // Read spv shader files
  std::vector<uint32_t> read_spirv(const char *filename) {
    auto fin = std::ifstream(filename, std::ios::binary | std::ios::ate);
    if (!fin.is_open())
    {
      throw std::runtime_error(std::string("failed opening file ") + filename + " for reading");
    }
    const auto stream_size = unsigned(fin.tellg());
    fin.seekg(0);

    auto ret = std::vector<std::uint32_t>((stream_size + 3) / 4, 0);
    std::copy(std::istreambuf_iterator<char>(fin), std::istreambuf_iterator<char>(), reinterpret_cast<char *>(ret.data()));
    return ret;
  }

  VkShaderModule initShaderModule(easyvk::Device &device, std::vector<uint32_t> spvCode) {
    VkShaderModule shaderModule;
    vkCheck(vkCreateShaderModule(device.device, new VkShaderModuleCreateInfo{VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO, nullptr, 0, spvCode.size() * sizeof(uint32_t), spvCode.data()}, nullptr, &shaderModule));

    return shaderModule;
  }
  VkShaderModule initShaderModule(easyvk::Device &device, const char *filepath) {
    std::vector<uint32_t> code = read_spirv(filepath);
    // Create shader module with spv code
    return initShaderModule(device, code);
  }

  VkDescriptorSetLayout createDescriptorSetLayout(easyvk::Device &device, uint32_t size) {
    std::vector<VkDescriptorSetLayoutBinding> layouts;
    // Create descriptor set with binding
    for (uint32_t i = 0; i < size; i++) {
      layouts.push_back(VkDescriptorSetLayoutBinding{
        i,
        VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
        1,
        VK_SHADER_STAGE_COMPUTE_BIT
      });
    }
    // Define descriptor set layout info
    VkDescriptorSetLayoutCreateInfo createInfo {
      VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
      nullptr,
      VkDescriptorSetLayoutCreateFlags {},
      size,
      layouts.data()
    };
    VkDescriptorSetLayout descriptorSetLayout;
    vkCheck(vkCreateDescriptorSetLayout(device.device, &createInfo, nullptr, &descriptorSetLayout));
    return descriptorSetLayout;
  }

  // This function brings descriptorSet, buffers, and bufferInfo to create writeDescriptorSets,
  // which describes a descriptor set write operation
  void writeSets(VkDescriptorSet &descriptorSet,
      std::vector<easyvk::Buffer> &buffers,
      std::vector<VkWriteDescriptorSet> &writeDescriptorSets,
      std::vector<VkDescriptorBufferInfo> &bufferInfos) {

    // Define descriptor buffer info
    for (int i = 0; i < buffers.size(); i++) {
      bufferInfos.push_back(VkDescriptorBufferInfo {
        buffers[i].buffer,
        0,
        VK_WHOLE_SIZE
      });
    }

    // wow this bug sucked: https://medium.com/@arpytoth/the-dangerous-pointer-to-vector-a139cc42a192
    for (int i = 0; i < buffers.size(); i++) {
      writeDescriptorSets.push_back(VkWriteDescriptorSet {
        VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
        nullptr,
        descriptorSet,
        (uint32_t)i,
        0,
        1,
        VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
        nullptr,
        &bufferInfos[i],
        nullptr
      });
    }
  }

  void Program::initialize(const char *entry_point, VkPipelineShaderStageCreateFlags pipelineFlags) {
    descriptorSetLayout = createDescriptorSetLayout(device, buffers.size());

    // Define pipeline layout info
    VkPipelineLayoutCreateInfo createInfo {
      VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
      nullptr,
      VkPipelineLayoutCreateFlags {},
      1,
      &descriptorSetLayout,
      1,
      new VkPushConstantRange { VK_SHADER_STAGE_COMPUTE_BIT, 0, push_constant_size_bytes }
    };

    // Create a new pipeline layout object
    vkCheck(vkCreatePipelineLayout(device.device, &createInfo, nullptr, &pipelineLayout));

    // Define descriptor pool size
    VkDescriptorPoolSize poolSize {
      VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
      (uint32_t)buffers.size()
    };
    auto descriptorSizes = std::array<VkDescriptorPoolSize, 1>({poolSize});

    // Create a new descriptor pool object
    VkDescriptorPoolCreateInfo descriptorPoolCreateInfo {
      VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO, 
      nullptr, 
      VkDescriptorPoolCreateFlags{}, 
      1, 
      uint32_t(descriptorSizes.size()), 
      descriptorSizes.data()
    };
    vkCheck(vkCreateDescriptorPool(device.device, &descriptorPoolCreateInfo, nullptr, &descriptorPool));

    // Allocate descriptor set
    VkDescriptorSetAllocateInfo descriptorSetAllocateInfo {
      VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO, 
      nullptr, 
      descriptorPool, 
      1, 
      &descriptorSetLayout
    };
    vkCheck(vkAllocateDescriptorSets(device.device, &descriptorSetAllocateInfo, &descriptorSet));

    writeSets(descriptorSet, buffers, writeDescriptorSets, bufferInfos);

    // Update contents of descriptor set object
    vkUpdateDescriptorSets(device.device, writeDescriptorSets.size(), writeDescriptorSets.data(), 0, {});

    uint32_t numSpecConstants = 3 + workgroupMemoryLengths.size();
    std::vector<VkSpecializationMapEntry> specMap(numSpecConstants);
    std::vector<uint32_t> specMapContent(numSpecConstants);

    // first three specialization constants are the workgroup size
    specMap[0] = VkSpecializationMapEntry{0, 0, sizeof(uint32_t)};
    specMapContent[0] = workgroupSize;
    specMap[1] = VkSpecializationMapEntry{1, 4, sizeof(uint32_t)};
    specMapContent[1] = 1;
    specMap[2] = VkSpecializationMapEntry{2, 8, sizeof(uint32_t)};
    specMapContent[2] = 1;
    // key is index, value is length
    for (const auto &[key, value] : workgroupMemoryLengths) {
      specMap[3 + key] = VkSpecializationMapEntry{3 + key, (3 + key) * 4, sizeof(uint32_t)};
      specMapContent[3 + key] = value;
    }

    VkSpecializationInfo specInfo{numSpecConstants, specMap.data(), numSpecConstants * sizeof(uint32_t), specMapContent.data()};

    // Define shader stage create info
    VkPipelineShaderStageCreateInfo stageCI {
      VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
      nullptr,
      pipelineFlags,
      VK_SHADER_STAGE_COMPUTE_BIT,
      shaderModule,
      entry_point,
      &specInfo
    };

    // Define compute pipeline create info
    VkComputePipelineCreateInfo pipelineCI {
      VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO,
      nullptr,
      {},
      stageCI,
      pipelineLayout
    };

    // Create compute pipelines
    vkCheck(vkCreateComputePipelines(device.device, {}, 1, &pipelineCI, nullptr, &pipeline));

    // Create fence.
    vkCheck(vkCreateFence(
        device.device,
        new VkFenceCreateInfo {
            VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
            nullptr,
            0},
        nullptr,
        &fence));

    // Define command pool info
    VkCommandPoolCreateInfo commandPoolCreateInfo {
      VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
      nullptr,
      VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
      device.computeFamilyId
    };

    // Create command pool
    vkCheck(vkCreateCommandPool(device.device, &commandPoolCreateInfo, nullptr, &commandPool));

    // Define command buffer info
    VkCommandBufferAllocateInfo commandBufferAI {
        VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        nullptr,
        commandPool,
        VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        1
    };

    // Allocate command buffers
    vkCheck(vkAllocateCommandBuffers(device.device, &commandBufferAI, &commandBuffer));

    // Create timestamp query pool
    // TODO: Device support limits need to be queried.
    vkCheck(vkCreateQueryPool(
        device.device,
        new VkQueryPoolCreateInfo {
          VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO,
          VK_NULL_HANDLE,
          0,
          VK_QUERY_TYPE_TIMESTAMP,
          2
        },
        VK_NULL_HANDLE,
        &timestampQueryPool));
  }

  std::vector<ShaderStatistics> Program::getShaderStats() {
    std::vector<ShaderStatistics> stats;
    if (device.supportsAMDShaderStats) {
      VkShaderStatisticsInfoAMD statInfo = {};
      size_t infoSize = sizeof(statInfo);
      PFN_vkGetShaderInfoAMD pfnGetShaderInfoAMD = (PFN_vkGetShaderInfoAMD)vkGetDeviceProcAddr(
        device.device, "vkGetShaderInfoAMD");
      vkCheck(pfnGetShaderInfoAMD(
        device.device,
        pipeline,
        VK_SHADER_STAGE_COMPUTE_BIT,
        VK_SHADER_INFO_TYPE_STATISTICS_AMD,
        &infoSize,
        &statInfo));
      stats.push_back(ShaderStatistics{ "Physical Vgprs", "Physical vector general purpose registers", 2, statInfo.numPhysicalVgprs });
      stats.push_back(ShaderStatistics{ "Physical Sgprs", "Physical scalar general purpose registers", 2, statInfo.numPhysicalSgprs });
      stats.push_back(ShaderStatistics{ "Compiler Vgprs", "Compiler vector general purpose registers", 2, statInfo.numAvailableVgprs });
      stats.push_back(ShaderStatistics{ "Compiler Sgprs", "Compiler scalar general purpose registers", 2, statInfo.numAvailableSgprs });
    }

    // we assume there is only one executable (e.g. shader) associated with this pipeline, or at least, the first one is the one we want stats for
    VkPipelineExecutableInfoKHR pExecutableInfo = { VK_STRUCTURE_TYPE_PIPELINE_EXECUTABLE_INFO_KHR, nullptr, pipeline, 0 };
    PFN_vkGetPipelineExecutableStatisticsKHR pfnGetPipelineExecutableStatistics = (PFN_vkGetPipelineExecutableStatisticsKHR)vkGetDeviceProcAddr(
      device.device, "vkGetPipelineExecutableStatisticsKHR");
    uint32_t executableCount = 0;
    // get the count of statistics
    pfnGetPipelineExecutableStatistics(device.device, &pExecutableInfo, &executableCount, nullptr);
    std::vector<VkPipelineExecutableStatisticKHR> statistics(executableCount, { VK_STRUCTURE_TYPE_PIPELINE_EXECUTABLE_STATISTIC_KHR });
    // get the actual statistics
    pfnGetPipelineExecutableStatistics(device.device, &pExecutableInfo, &executableCount, statistics.data());
    // Output statistics
    for (const auto& stat : statistics) {
      switch (stat.format) {
      case VK_PIPELINE_EXECUTABLE_STATISTIC_FORMAT_BOOL32_KHR:
        stats.push_back(ShaderStatistics{ stat.name, stat.description, 0, stat.value.b32 });
        break;
      case VK_PIPELINE_EXECUTABLE_STATISTIC_FORMAT_INT64_KHR:
        stats.push_back(ShaderStatistics{ stat.name, stat.description, 1, (uint64_t) stat.value.i64 });
        break;
      case VK_PIPELINE_EXECUTABLE_STATISTIC_FORMAT_UINT64_KHR:
        stats.push_back(ShaderStatistics{ stat.name, stat.description, 2, stat.value.u64 });
        break;
      case VK_PIPELINE_EXECUTABLE_STATISTIC_FORMAT_FLOAT64_KHR:
        stats.push_back(ShaderStatistics{ stat.name, stat.description, 3, (uint64_t) stat.value.f64 });
        break;
      default:
        break;
      }
    }
    return stats;
  }

  void Program::run() {
    // Start recording command buffer
    vkCheck(vkBeginCommandBuffer(commandBuffer, new VkCommandBufferBeginInfo{VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO}));

    // Bind pipeline and descriptor sets
    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline);
    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE,
                            pipelineLayout, 0, 1, &descriptorSet, 0, 0);

    // Bind push constants
    uint32_t pValues[3] = {0, 0, 0};
    vkCmdPushConstants(commandBuffer, pipelineLayout, VK_SHADER_STAGE_COMPUTE_BIT, 0, push_constant_size_bytes, &pValues);

    vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_HOST_BIT, 0,
                         1, new VkMemoryBarrier{VK_STRUCTURE_TYPE_MEMORY_BARRIER, nullptr, VK_ACCESS_SHADER_WRITE_BIT, VK_ACCESS_HOST_READ_BIT}, 0, {}, 0, {});

    // Dispatch compute work items
    vkCmdDispatch(commandBuffer, numWorkgroups, 1, 1);

    vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_HOST_BIT, 0,
                         1, new VkMemoryBarrier{VK_STRUCTURE_TYPE_MEMORY_BARRIER, nullptr, VK_ACCESS_SHADER_WRITE_BIT, VK_ACCESS_HOST_READ_BIT}, 0, {}, 0, {});

    // End recording command buffer
    vkCheck(vkEndCommandBuffer(commandBuffer));
    
    // Define submit info
    VkSubmitInfo submitInfo {
      VK_STRUCTURE_TYPE_SUBMIT_INFO,
      nullptr,
      0,
      nullptr,
      nullptr,
      1,
      &commandBuffer,
      0,
      nullptr
    };

    auto queue = device.computeQueue;

    // Submit command buffer to queue, signals fence on completion.
    vkCheck(vkQueueSubmit(queue, 1, &submitInfo, fence));
    // Wait on fence.
    vkCheck(vkWaitForFences(device.device, 1, &fence, VK_TRUE, UINT64_MAX));
    // Reset fence signal.
    vkCheck(vkResetFences(device.device, 1, &fence));
  }

  float Program::runWithDispatchTiming() {
    // Start recording command buffer
    vkCheck(vkBeginCommandBuffer(commandBuffer, new VkCommandBufferBeginInfo{VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO}));

    // Reset query pool.
    vkCmdResetQueryPool(commandBuffer, timestampQueryPool, 0, 2);

    // Bind pipeline and descriptor sets
    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline);
    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE,
                            pipelineLayout, 0, 1, &descriptorSet, 0, 0);

    // Bind push constants
    uint32_t pValues[3] = {0, 0, 0};
    vkCmdPushConstants(commandBuffer, pipelineLayout, VK_SHADER_STAGE_COMPUTE_BIT, 0, push_constant_size_bytes, &pValues);

    vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_HOST_BIT, 0,
                         1, new VkMemoryBarrier{VK_STRUCTURE_TYPE_MEMORY_BARRIER, nullptr, VK_ACCESS_SHADER_WRITE_BIT, VK_ACCESS_HOST_READ_BIT}, 0, {}, 0, {});

    // Write first timestamp.
    vkCmdWriteTimestamp(commandBuffer, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, timestampQueryPool, 0);

    // Dispatch compute work items
    vkCmdDispatch(commandBuffer, numWorkgroups, 1, 1);

    vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_HOST_BIT, 0,
                         1, new VkMemoryBarrier{VK_STRUCTURE_TYPE_MEMORY_BARRIER, nullptr, VK_ACCESS_SHADER_WRITE_BIT, VK_ACCESS_HOST_READ_BIT}, 0, {}, 0, {});

    // Write second timestamp.
    vkCmdWriteTimestamp(commandBuffer, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, timestampQueryPool, 1);

    // End recording command buffer
    vkCheck(vkEndCommandBuffer(commandBuffer));

    // Define submit info
    VkSubmitInfo submitInfo{
        VK_STRUCTURE_TYPE_SUBMIT_INFO,
        nullptr,
        0,
        nullptr,
        nullptr,
        1,
        &commandBuffer,
        0,
        nullptr};

    auto queue = device.computeQueue;

    // Submit command buffer to queue, signals fence on completion.
    vkCheck(vkQueueSubmit(queue, 1, &submitInfo, fence));
    // Wait on fence.
    vkCheck(vkWaitForFences(device.device, 1, &fence, VK_TRUE, UINT64_MAX));
    // Reset fence signal.
    vkCheck(vkResetFences(device.device, 1, &fence));

    // Get timestamp query results.
    uint64_t queryResults[2] = {0, 0};
    vkCheck(vkGetQueryPoolResults(
        device.device,
        timestampQueryPool,
        0,
        2,
        sizeof(uint64_t) * 2,
        queryResults,
        sizeof(uint64_t),
        VK_QUERY_RESULT_64_BIT | VK_QUERY_RESULT_WAIT_BIT));

    return (queryResults[1] - queryResults[0]) * device.properties.limits.timestampPeriod;
  }

  void Program::setWorkgroups(uint32_t _numWorkgroups) {
    numWorkgroups = _numWorkgroups;
  }

  void Program::setWorkgroupSize(uint32_t _workgroupSize) {
    workgroupSize = _workgroupSize;
  }

  void Program::setWorkgroupMemoryLength(uint32_t length, uint32_t index) {
    workgroupMemoryLengths[index] = length;
  }

  Program::Program(Device &_device, std::vector<uint32_t> spvCode, std::vector<Buffer> &_buffers) : device(_device),
                                                                                                    shaderModule(initShaderModule(_device, spvCode)),
                                                                                                    buffers(_buffers) {}

  Program::Program(Device &_device, const char *filepath, std::vector<Buffer> &_buffers) : device(_device),
                                                                                           shaderModule(initShaderModule(_device, filepath)),
                                                                                           buffers(_buffers) {}

  void Program::teardown() {
    vkDestroyShaderModule(device.device, shaderModule, nullptr);
    vkDestroyDescriptorPool(device.device, descriptorPool, nullptr);
    vkDestroyDescriptorSetLayout(device.device, descriptorSetLayout, nullptr);
    vkDestroyPipelineLayout(device.device, pipelineLayout, nullptr);
    vkDestroyPipeline(device.device, pipeline, nullptr);
    vkDestroyFence(device.device, fence, nullptr);
    vkDestroyCommandPool(device.device, commandPool, nullptr);
    vkDestroyQueryPool(device.device, timestampQueryPool, VK_NULL_HANDLE);
  }
}
