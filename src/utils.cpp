// Copyright(c) 2019, NVIDIA CORPORATION. All rights reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//

#include "utils.hpp"
#include "vulkan/vulkan.hpp"
#include "Common.h"
#include <iomanip>
#include <numeric>

#if (VULKAN_HPP_DISPATCH_LOADER_DYNAMIC == 1)
VULKAN_HPP_DEFAULT_DISPATCH_LOADER_DYNAMIC_STORAGE
#elif !defined(NDEBUG)
PFN_vkCreateDebugUtilsMessengerEXT pfnVkCreateDebugUtilsMessengerEXT;
PFN_vkDestroyDebugUtilsMessengerEXT pfnVkDestroyDebugUtilsMessengerEXT;

VKAPI_ATTR VkResult VKAPI_CALL vkCreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pMessenger)
{
  return pfnVkCreateDebugUtilsMessengerEXT(instance, pCreateInfo, pAllocator, pMessenger);
}

VKAPI_ATTR void VKAPI_CALL vkDestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT messenger, VkAllocationCallbacks const * pAllocator)
{
  return pfnVkDestroyDebugUtilsMessengerEXT(instance, messenger, pAllocator);
}
#endif

namespace vk
{
  namespace su
  {
    vk::UniqueDeviceMemory allocateMemory(vk::UniqueDevice const& device, vk::PhysicalDeviceMemoryProperties const& memoryProperties, vk::MemoryRequirements const& memoryRequirements,
                                          vk::MemoryPropertyFlags memoryPropertyFlags)
    {
      uint32_t memoryTypeIndex = findMemoryType(memoryProperties, memoryRequirements.memoryTypeBits, memoryPropertyFlags);

      return device->allocateMemoryUnique(vk::MemoryAllocateInfo(memoryRequirements.size, memoryTypeIndex));
    }

    vk::UniqueCommandPool createCommandPool(vk::UniqueDevice &device, uint32_t queueFamilyIndex)
    {
      vk::CommandPoolCreateInfo commandPoolCreateInfo(vk::CommandPoolCreateFlagBits::eResetCommandBuffer, queueFamilyIndex);
      return device->createCommandPoolUnique(commandPoolCreateInfo);
    }

    vk::UniqueDebugUtilsMessengerEXT createDebugUtilsMessenger(vk::UniqueInstance &instance)
    {
      vk::DebugUtilsMessageSeverityFlagsEXT severityFlags(vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning | vk::DebugUtilsMessageSeverityFlagBitsEXT::eError);
      vk::DebugUtilsMessageTypeFlagsEXT messageTypeFlags(vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral | vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance | vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation);
      return instance->createDebugUtilsMessengerEXTUnique(vk::DebugUtilsMessengerCreateInfoEXT({}, severityFlags, messageTypeFlags, &vk::su::debugUtilsMessengerCallback));
    }

    vk::UniqueDescriptorPool createDescriptorPool(vk::UniqueDevice &device, std::vector<vk::DescriptorPoolSize> const& poolSizes)
    {
      assert(!poolSizes.empty());
      uint32_t maxSets = std::accumulate(poolSizes.begin(), poolSizes.end(), 0, [](uint32_t sum, vk::DescriptorPoolSize const& dps) { return sum + dps.descriptorCount; });
      assert(0 < maxSets);

      vk::DescriptorPoolCreateInfo descriptorPoolCreateInfo(vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet, maxSets, checked_cast<uint32_t>(poolSizes.size()), poolSizes.data());
      return device->createDescriptorPoolUnique(descriptorPoolCreateInfo);
    }

    vk::UniqueDescriptorSetLayout createDescriptorSetLayout(vk::UniqueDevice const& device, std::vector<std::tuple<vk::DescriptorType, uint32_t, vk::ShaderStageFlags>> const& bindingData,
                                                            vk::DescriptorSetLayoutCreateFlags flags)
    {
      std::vector<vk::DescriptorSetLayoutBinding> bindings(bindingData.size());
      for (size_t i = 0; i < bindingData.size(); i++)
      {
        bindings[i] = vk::DescriptorSetLayoutBinding(checked_cast<uint32_t>(i), std::get<0>(bindingData[i]), std::get<1>(bindingData[i]), std::get<2>(bindingData[i]));
      }
      return device->createDescriptorSetLayoutUnique(vk::DescriptorSetLayoutCreateInfo(flags, checked_cast<uint32_t>(bindings.size()), bindings.data()));
    }


    std::vector<vk::UniqueFramebuffer> createFramebuffers(vk::UniqueDevice &device, vk::UniqueRenderPass &renderPass, std::vector<vk::UniqueImageView> const& imageViews, vk::UniqueImageView const& depthImageView, vk::Extent2D const& extent)
    {
      vk::ImageView attachments[2];
      attachments[1] = depthImageView.get();

      vk::FramebufferCreateInfo framebufferCreateInfo(vk::FramebufferCreateFlags(), *renderPass, depthImageView ? 2 : 1, attachments, extent.width, extent.height, 1);
      std::vector<vk::UniqueFramebuffer> framebuffers;
      framebuffers.reserve(imageViews.size());
      for (auto const& view : imageViews)
      {
        attachments[0] = view.get();
        framebuffers.push_back(device->createFramebufferUnique(framebufferCreateInfo));
      }

      return framebuffers;
    }

    vk::UniquePipeline createGraphicsPipeline(vk::UniqueDevice const& device, vk::UniquePipelineCache const& pipelineCache,
                                              std::pair<vk::ShaderModule, vk::SpecializationInfo const*> const& vertexShaderData,
                                              std::pair<vk::ShaderModule, vk::SpecializationInfo const*> const& fragmentShaderData, uint32_t vertexStride,
                                              std::vector<std::pair<vk::Format, uint32_t>> const& vertexInputAttributeFormatOffset, vk::FrontFace frontFace, bool depthBuffered
                                              , vk::UniquePipelineLayout const& pipelineLayout, vk::UniqueRenderPass const& renderPass)
    {
      vk::PipelineShaderStageCreateInfo pipelineShaderStageCreateInfos[2] =
      {
        vk::PipelineShaderStageCreateInfo(vk::PipelineShaderStageCreateFlags(), vk::ShaderStageFlagBits::eVertex, vertexShaderData.first, "main", vertexShaderData.second),
        vk::PipelineShaderStageCreateInfo(vk::PipelineShaderStageCreateFlags(), vk::ShaderStageFlagBits::eFragment, fragmentShaderData.first, "main", fragmentShaderData.second)
      };

      std::vector<vk::VertexInputAttributeDescription> vertexInputAttributeDescriptions;
      vk::PipelineVertexInputStateCreateInfo pipelineVertexInputStateCreateInfo;
      if (0 < vertexStride)
      {
        vk::VertexInputBindingDescription vertexInputBindingDescription(0, vertexStride);
        vertexInputAttributeDescriptions.reserve(vertexInputAttributeFormatOffset.size());
        for (uint32_t i=0 ; i<vertexInputAttributeFormatOffset.size() ; i++)
        {
          vertexInputAttributeDescriptions.push_back(vk::VertexInputAttributeDescription(i, 0, vertexInputAttributeFormatOffset[i].first, vertexInputAttributeFormatOffset[i].second));
        }
        pipelineVertexInputStateCreateInfo.vertexBindingDescriptionCount = 1;
        pipelineVertexInputStateCreateInfo.pVertexBindingDescriptions = &vertexInputBindingDescription;
        pipelineVertexInputStateCreateInfo.vertexAttributeDescriptionCount = vk::su::checked_cast<uint32_t>(vertexInputAttributeDescriptions.size());
        pipelineVertexInputStateCreateInfo.pVertexAttributeDescriptions = vertexInputAttributeDescriptions.data();
      }

      vk::PipelineInputAssemblyStateCreateInfo pipelineInputAssemblyStateCreateInfo(vk::PipelineInputAssemblyStateCreateFlags(), vk::PrimitiveTopology::eTriangleList);

      vk::PipelineViewportStateCreateInfo pipelineViewportStateCreateInfo(vk::PipelineViewportStateCreateFlags(), 1, nullptr, 1, nullptr);

      vk::PipelineRasterizationStateCreateInfo pipelineRasterizationStateCreateInfo(vk::PipelineRasterizationStateCreateFlags(), false, false, vk::PolygonMode::eFill, vk::CullModeFlagBits::eBack,
                                                                                    frontFace, false, 0.0f, 0.0f, 0.0f, 1.0f);

      vk::PipelineMultisampleStateCreateInfo pipelineMultisampleStateCreateInfo;

      vk::StencilOpState stencilOpState(vk::StencilOp::eKeep, vk::StencilOp::eKeep, vk::StencilOp::eKeep, vk::CompareOp::eAlways);
      vk::PipelineDepthStencilStateCreateInfo pipelineDepthStencilStateCreateInfo(vk::PipelineDepthStencilStateCreateFlags(), depthBuffered, depthBuffered, vk::CompareOp::eLessOrEqual, false,
                                                                                  false, stencilOpState, stencilOpState);

      vk::ColorComponentFlags colorComponentFlags(vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA);
      vk::PipelineColorBlendAttachmentState pipelineColorBlendAttachmentState(false, vk::BlendFactor::eZero, vk::BlendFactor::eZero, vk::BlendOp::eAdd, vk::BlendFactor::eZero,
                                                                              vk::BlendFactor::eZero, vk::BlendOp::eAdd, colorComponentFlags);
      vk::PipelineColorBlendStateCreateInfo pipelineColorBlendStateCreateInfo(vk::PipelineColorBlendStateCreateFlags(), false, vk::LogicOp::eNoOp, 1, &pipelineColorBlendAttachmentState,
                                                                              { { (1.0f, 1.0f, 1.0f, 1.0f) } });

      vk::DynamicState dynamicStates[2] = { vk::DynamicState::eViewport, vk::DynamicState::eScissor };
      vk::PipelineDynamicStateCreateInfo pipelineDynamicStateCreateInfo(vk::PipelineDynamicStateCreateFlags(), 2, dynamicStates);

      vk::GraphicsPipelineCreateInfo graphicsPipelineCreateInfo(vk::PipelineCreateFlags(), 2, pipelineShaderStageCreateInfos, &pipelineVertexInputStateCreateInfo,
                                                                &pipelineInputAssemblyStateCreateInfo, nullptr, &pipelineViewportStateCreateInfo, &pipelineRasterizationStateCreateInfo,
                                                                &pipelineMultisampleStateCreateInfo, &pipelineDepthStencilStateCreateInfo, &pipelineColorBlendStateCreateInfo,
                                                                &pipelineDynamicStateCreateInfo, pipelineLayout.get(), renderPass.get());
      
      return device->createGraphicsPipelineUnique(pipelineCache.get(), graphicsPipelineCreateInfo);
    }

    vk::UniqueInstance createInstance(std::string const& appName, std::string const& engineName, std::vector<std::string> const& layers, std::vector<std::string> const& extensions,
                                      uint32_t apiVersion)
    {
#if (VULKAN_HPP_DISPATCH_LOADER_DYNAMIC == 1)
      static vk::DynamicLoader dl;
      PFN_vkGetInstanceProcAddr vkGetInstanceProcAddr = dl.getProcAddress<PFN_vkGetInstanceProcAddr>("vkGetInstanceProcAddr");
      VULKAN_HPP_DEFAULT_DISPATCHER.init(vkGetInstanceProcAddr);
#endif

      std::vector<char const*> enabledLayers;
      enabledLayers.reserve(layers.size());
      for (auto const& layer : layers)
      {
        enabledLayers.push_back(layer.data());
      }
#if !defined(NDEBUG)
      std::vector<vk::LayerProperties> availableLayers = vk::enumerateInstanceLayerProperties();
      
      // Enable standard validation layer to find as much errors as possible!
      if (!contains(layers, "VK_LAYER_LUNARG_standard_validation") && 
          contains_if(availableLayers, [](auto e) { return e.layerName == "VK_LAYER_LUNARG_standard_validation"; } ))
      {
          enabledLayers.push_back("VK_LAYER_LUNARG_standard_validation");
      }
      else if (!contains(layers, "VK_LAYER_KHRONOS_validation") && 
               contains_if(availableLayers, [](auto e) { return e.layerName == "VK_LAYER_KHRONOS_validation"; } ))
      {
          enabledLayers.push_back("VK_LAYER_KHRONOS_validation");
      }
#endif

      std::vector<char const*> enabledExtensions;
      enabledExtensions.reserve(extensions.size());
      for (auto const& ext : extensions)
      {
        enabledExtensions.push_back(ext.data());
      }
#if !defined(NDEBUG)
      if (std::find(extensions.begin(), extensions.end(), VK_EXT_DEBUG_UTILS_EXTENSION_NAME) == extensions.end())
      {
        enabledExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
      }
#endif

      // create a UniqueInstance
      vk::ApplicationInfo applicationInfo(appName.c_str(), 1, engineName.c_str(), 1, apiVersion);
      vk::UniqueInstance instance = vk::createInstanceUnique(vk::InstanceCreateInfo({}, &applicationInfo, checked_cast<uint32_t>(enabledLayers.size()), enabledLayers.data(),
                                                                                    checked_cast<uint32_t>(enabledExtensions.size()), enabledExtensions.data()));

#if (VULKAN_HPP_DISPATCH_LOADER_DYNAMIC == 1)
      // initialize function pointers for instance
      VULKAN_HPP_DEFAULT_DISPATCHER.init(*instance);
#else
# if !defined(NDEBUG)
      static bool initialized = false;
      if (!initialized)
      {
        pfnVkCreateDebugUtilsMessengerEXT = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(instance->getProcAddr("vkCreateDebugUtilsMessengerEXT"));
        pfnVkDestroyDebugUtilsMessengerEXT = reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(instance->getProcAddr("vkDestroyDebugUtilsMessengerEXT"));
        assert(pfnVkCreateDebugUtilsMessengerEXT && pfnVkDestroyDebugUtilsMessengerEXT);
        initialized = true;
      }
# endif
#endif

      return instance;
    }

    vk::UniqueRenderPass createRenderPass(vk::UniqueDevice &device, vk::Format colorFormat, vk::Format depthFormat, vk::AttachmentLoadOp loadOp, vk::ImageLayout colorFinalLayout)
    {
      std::vector<vk::AttachmentDescription> attachmentDescriptions;
      assert(colorFormat != vk::Format::eUndefined);
      attachmentDescriptions.push_back(vk::AttachmentDescription(vk::AttachmentDescriptionFlags(), colorFormat, vk::SampleCountFlagBits::e1, loadOp, vk::AttachmentStoreOp::eStore,
                                                                 vk::AttachmentLoadOp::eDontCare, vk::AttachmentStoreOp::eDontCare, vk::ImageLayout::eUndefined, colorFinalLayout));
      if (depthFormat != vk::Format::eUndefined)
      {
        attachmentDescriptions.push_back(vk::AttachmentDescription(vk::AttachmentDescriptionFlags(), depthFormat, vk::SampleCountFlagBits::e1, loadOp, vk::AttachmentStoreOp::eDontCare,
                                                                   vk::AttachmentLoadOp::eDontCare, vk::AttachmentStoreOp::eDontCare, vk::ImageLayout::eUndefined,
                                                                   vk::ImageLayout::eDepthStencilAttachmentOptimal));
      }
      vk::AttachmentReference colorAttachment(0, vk::ImageLayout::eColorAttachmentOptimal);
      vk::AttachmentReference depthAttachment(1, vk::ImageLayout::eDepthStencilAttachmentOptimal);
      vk::SubpassDescription subpassDescription(vk::SubpassDescriptionFlags(), vk::PipelineBindPoint::eGraphics, 0, nullptr, 1, &colorAttachment, nullptr,
        (depthFormat != vk::Format::eUndefined) ? &depthAttachment : nullptr);
      return device->createRenderPassUnique(vk::RenderPassCreateInfo(vk::RenderPassCreateFlags(), static_cast<uint32_t>(attachmentDescriptions.size()), attachmentDescriptions.data(), 1,
                                                                     &subpassDescription));
    }

    VkBool32 debugUtilsMessengerCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageTypes,
                                         VkDebugUtilsMessengerCallbackDataEXT const * pCallbackData, void * /*pUserData*/)
    {
      std::cerr << vk::to_string(static_cast<vk::DebugUtilsMessageSeverityFlagBitsEXT>(messageSeverity)) << ": " << vk::to_string(static_cast<vk::DebugUtilsMessageTypeFlagsEXT>(messageTypes)) << ":\n";
      std::cerr << "\t" << "messageIDName   = <" << pCallbackData->pMessageIdName << ">\n";
      std::cerr << "\t" << "messageIdNumber = " << pCallbackData->messageIdNumber << "\n";
      std::cerr << "\t" << "message         = <" << pCallbackData->pMessage << ">\n";
      if (0 < pCallbackData->queueLabelCount)
      {
        std::cerr << "\t" << "Queue Labels:\n";
        for (uint8_t i = 0; i < pCallbackData->queueLabelCount; i++)
        {
          std::cerr << "\t\t" << "lableName = <" << pCallbackData->pQueueLabels[i].pLabelName << ">\n";
        }
      }
      if (0 < pCallbackData->cmdBufLabelCount)
      {
        std::cerr << "\t" << "CommandBuffer Labels:\n";
        for (uint8_t i = 0; i < pCallbackData->cmdBufLabelCount; i++)
        {
          std::cerr << "\t\t" << "labelName = <" << pCallbackData->pCmdBufLabels[i].pLabelName << ">\n";
        }
      }
      if (0 < pCallbackData->objectCount)
      {
        std::cerr << "\t" << "Objects:\n";
        for (uint8_t i = 0; i < pCallbackData->objectCount; i++)
        {
          std::cerr << "\t\t" << "Object " << i << "\n";
          std::cerr << "\t\t\t" << "objectType   = " << vk::to_string(static_cast<vk::ObjectType>(pCallbackData->pObjects[i].objectType)) << "\n";
          std::cerr << "\t\t\t" << "objectHandle = " << pCallbackData->pObjects[i].objectHandle << "\n";
          if (pCallbackData->pObjects[i].pObjectName)
          {
            std::cerr << "\t\t\t" << "objectName   = <" << pCallbackData->pObjects[i].pObjectName << ">\n";
          }
        }
      }
      return VK_TRUE;
    }

    uint32_t findMemoryType(vk::PhysicalDeviceMemoryProperties const& memoryProperties, uint32_t typeBits, vk::MemoryPropertyFlags requirementsMask)
    {
      uint32_t typeIndex = uint32_t(~0);
      for (uint32_t i = 0; i < memoryProperties.memoryTypeCount; i++)
      {
        if ((typeBits & 1) && ((memoryProperties.memoryTypes[i].propertyFlags & requirementsMask) == requirementsMask))
        {
          typeIndex = i;
          break;
        }
        typeBits >>= 1;
      }
      assert(typeIndex != ~0);
      return typeIndex;
    }

    std::vector<std::string> getInstanceExtensions()
    {
      std::vector<std::string> extensions;
      extensions.push_back(VK_KHR_SURFACE_EXTENSION_NAME);
#if  defined(VK_USE_PLATFORM_ANDROID_KHR)
      extensions.push_back(VK_KHR_ANDROID_SURFACE_EXTENSION_NAME);
#elif defined(VK_USE_PLATFORM_IOS_MVK)
      extensions.push_back(VK_MVK_IOS_SURFACE_EXTENSION_NAME);
#elif defined(VK_USE_PLATFORM_MACOS_MVK)
      extensions.push_back(VK_MVK_MACOS_SURFACE_EXTENSION_NAME);
#elif defined(VK_USE_PLATFORM_MIR_KHR)
      extensions.push_back(VK_KHR_MIR_SURFACE_EXTENSION_NAME);
#elif defined(VK_USE_PLATFORM_VI_NN)
      extensions.push_back(VK_NN_VI_SURFACE_EXTENSION_NAME);
#elif defined(VK_USE_PLATFORM_WAYLAND_KHR)
      extensions.push_back(VK_KHR_WAYLAND_SURFACE_EXTENSION_NAME);
#elif defined(VK_USE_PLATFORM_WIN32_KHR)
      extensions.push_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
#elif defined(VK_USE_PLATFORM_XCB_KHR)
      extensions.push_back(VK_KHR_XCB_SURFACE_EXTENSION_NAME);
#elif defined(VK_USE_PLATFORM_XLIB_KHR)
      extensions.push_back(VK_KHR_XLIB_SURFACE_EXTENSION_NAME);
#elif defined(VK_USE_PLATFORM_XLIB_XRANDR_EXT)
      extensions.push_back(VK_EXT_ACQUIRE_XLIB_DISPLAY_EXTENSION_NAME);
#endif
      return extensions;
    }

    vk::Format pickDepthFormat(vk::PhysicalDevice const& physicalDevice)
    {
      std::vector<vk::Format> candidates = {vk::Format::eD32Sfloat, vk::Format::eD32SfloatS8Uint, vk::Format::eD24UnormS8Uint};
      for (vk::Format format : candidates)
      {
        vk::FormatProperties props = physicalDevice.getFormatProperties(format);

        if (props.optimalTilingFeatures & vk::FormatFeatureFlagBits::eDepthStencilAttachment)
        {
          return format;
        }
      }
      throw std::runtime_error("failed to find supported format!");
    }

    void setImageLayout(vk::UniqueCommandBuffer const& commandBuffer, vk::Image image, vk::Format format, vk::ImageLayout oldImageLayout, vk::ImageLayout newImageLayout)
    {
      vk::AccessFlags sourceAccessMask;
      switch (oldImageLayout)
      {
        case vk::ImageLayout::eTransferDstOptimal:
          sourceAccessMask = vk::AccessFlagBits::eTransferWrite;
          break;
        case vk::ImageLayout::ePreinitialized:
          sourceAccessMask = vk::AccessFlagBits::eHostWrite;
          break;
        case vk::ImageLayout::eGeneral:     // sourceAccessMask is empty
        case vk::ImageLayout::eUndefined:
          break;
        default:
          assert(false);
          break;
      }

      vk::PipelineStageFlags sourceStage;
      switch (oldImageLayout)
      {
        case vk::ImageLayout::eGeneral:
        case vk::ImageLayout::ePreinitialized:
          sourceStage = vk::PipelineStageFlagBits::eHost;
          break;
        case vk::ImageLayout::eTransferDstOptimal:
          sourceStage = vk::PipelineStageFlagBits::eTransfer;
          break;
        case vk::ImageLayout::eUndefined:
          sourceStage = vk::PipelineStageFlagBits::eTopOfPipe;
          break;
        default:
          assert(false);
          break;
      }

      vk::AccessFlags destinationAccessMask;
      switch (newImageLayout)
      {
        case vk::ImageLayout::eColorAttachmentOptimal:
          destinationAccessMask = vk::AccessFlagBits::eColorAttachmentWrite;
          break;
        case vk::ImageLayout::eDepthStencilAttachmentOptimal:
          destinationAccessMask = vk::AccessFlagBits::eDepthStencilAttachmentRead | vk::AccessFlagBits::eDepthStencilAttachmentWrite;
          break;
        case vk::ImageLayout::eGeneral:   // empty destinationAccessMask
          break;
        case vk::ImageLayout::eShaderReadOnlyOptimal:
          destinationAccessMask = vk::AccessFlagBits::eShaderRead;
          break;
        case vk::ImageLayout::eTransferSrcOptimal:
          destinationAccessMask = vk::AccessFlagBits::eTransferRead;
          break;
        case vk::ImageLayout::eTransferDstOptimal:
          destinationAccessMask = vk::AccessFlagBits::eTransferWrite;
          break;
        default:
          assert(false);
          break;
      }

      vk::PipelineStageFlags destinationStage;
      switch (newImageLayout)
      {
        case vk::ImageLayout::eColorAttachmentOptimal:
          destinationStage = vk::PipelineStageFlagBits::eColorAttachmentOutput;
          break;
        case vk::ImageLayout::eDepthStencilAttachmentOptimal:
          destinationStage = vk::PipelineStageFlagBits::eEarlyFragmentTests;
          break;
        case vk::ImageLayout::eGeneral:
          destinationStage = vk::PipelineStageFlagBits::eHost;
          break;
        case vk::ImageLayout::eShaderReadOnlyOptimal:
          destinationStage = vk::PipelineStageFlagBits::eFragmentShader;
          break;
        case vk::ImageLayout::eTransferDstOptimal:
        case vk::ImageLayout::eTransferSrcOptimal:
          destinationStage = vk::PipelineStageFlagBits::eTransfer;
          break;
        default:
          assert(false);
          break;
      }

      vk::ImageAspectFlags aspectMask;
      if (newImageLayout == vk::ImageLayout::eDepthStencilAttachmentOptimal)
      {
        aspectMask = vk::ImageAspectFlagBits::eDepth;
        if (format == vk::Format::eD32SfloatS8Uint || format == vk::Format::eD24UnormS8Uint)
        {
          aspectMask |= vk::ImageAspectFlagBits::eStencil;
        }
      }
      else
      {
        aspectMask = vk::ImageAspectFlagBits::eColor;
      }

      vk::ImageSubresourceRange imageSubresourceRange(aspectMask, 0, 1, 0, 1);
      vk::ImageMemoryBarrier imageMemoryBarrier(sourceAccessMask, destinationAccessMask, oldImageLayout, newImageLayout, VK_QUEUE_FAMILY_IGNORED, VK_QUEUE_FAMILY_IGNORED, image, imageSubresourceRange);
      return commandBuffer->pipelineBarrier(sourceStage, destinationStage, {}, nullptr, nullptr, imageMemoryBarrier);
    }

    void submitAndWait(vk::UniqueDevice &device, vk::Queue queue, vk::UniqueCommandBuffer &commandBuffer)
    {
      vk::UniqueFence fence = device->createFenceUnique(vk::FenceCreateInfo());
      vk::PipelineStageFlags pipelineStageFlags = vk::PipelineStageFlagBits::eColorAttachmentOutput;
      queue.submit(vk::SubmitInfo(0, nullptr, &pipelineStageFlags, 1, &commandBuffer.get()), fence.get());
      while (vk::Result::eTimeout == device->waitForFences(fence.get(), VK_TRUE, vk::su::FenceTimeout))
        ;
    }


    CheckerboardImageGenerator::CheckerboardImageGenerator(std::array<uint8_t, 3> const& rgb0, std::array<uint8_t, 3> const& rgb1)
      : m_rgb0(rgb0)
      , m_rgb1(rgb1)
    {}

    void CheckerboardImageGenerator::operator()(void* data, vk::Extent2D &extent) const
    {
      // Checkerboard of 16x16 pixel squares
      uint8_t *pImageMemory = static_cast<uint8_t *>(data);
      for (uint32_t row = 0; row < extent.height; row++)
      {
        for (uint32_t col = 0; col < extent.width; col++)
        {
          std::array<uint8_t, 3> const& rgb = (((row & 0x10) == 0) ^ ((col & 0x10) == 0)) ? m_rgb1 : m_rgb0;
          pImageMemory[0] = rgb[0];
          pImageMemory[1] = rgb[1];
          pImageMemory[2] = rgb[2];
          pImageMemory[3] = 255;
          pImageMemory += 4;
        }
      }
    }

    MonochromeImageGenerator::MonochromeImageGenerator(std::array<unsigned char, 3> const& rgb)
      : m_rgb(rgb)
    {}

    void MonochromeImageGenerator::operator()(void* data, vk::Extent2D &extent) const
    {
      // fill in with the monochrome color
      unsigned char *pImageMemory = static_cast<unsigned char*>(data);
      for (uint32_t row = 0; row < extent.height; row++)
      {
        for (uint32_t col = 0; col < extent.width; col++)
        {
          pImageMemory[0] = m_rgb[0];
          pImageMemory[1] = m_rgb[1];
          pImageMemory[2] = m_rgb[2];
          pImageMemory[3] = 255;
          pImageMemory += 4;
        }
      }
    }

    PixelsImageGenerator::PixelsImageGenerator(vk::Extent2D const& extent, size_t channels, unsigned char const* pixels)
      : m_extent(extent)
      , m_channels(channels)
      , m_pixels(pixels)
    {
      assert(m_channels == 4);
    }

    void PixelsImageGenerator::operator()(void* data, vk::Extent2D & extent) const
    {
      assert(extent == m_extent);
      memcpy(data, m_pixels, m_extent.width * m_extent.height * m_channels);
    }


    UUID::UUID(uint8_t data[VK_UUID_SIZE])
    {
      memcpy(m_data, data, VK_UUID_SIZE * sizeof(uint8_t));
    }
  }
}

std::ostream& operator<<(std::ostream& os, vk::su::UUID const& uuid)
{
  os << std::setfill('0') << std::hex;
  for (int j = 0; j < VK_UUID_SIZE; ++j)
  {
    os << std::setw(2) << static_cast<uint32_t>(uuid.m_data[j]);
    if (j == 3 || j == 5 || j == 7 || j == 9)
    {
      std::cout << '-';
    }
  }
  os << std::setfill(' ') << std::dec;
  return os;
}