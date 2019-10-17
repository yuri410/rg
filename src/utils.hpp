#pragma once

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

#include "vulkan/vulkan.hpp"
#include <iostream>
#include <map>

namespace vk
{
  namespace su
  {
    const uint64_t FenceTimeout = 100000000;

    class CheckerboardImageGenerator
    {
    public:
      CheckerboardImageGenerator(std::array<uint8_t, 3> const& rgb0 = {0, 0, 0}, std::array<uint8_t, 3> const& rgb1 = {255, 255, 255});

      void operator()(void* data, vk::Extent2D &extent) const;

    private:
      std::array<uint8_t, 3> const& m_rgb0;
      std::array<uint8_t, 3> const& m_rgb1;
    };

    class MonochromeImageGenerator
    {
      public:
      MonochromeImageGenerator(std::array<unsigned char, 3> const& rgb);

      void operator()(void* data, vk::Extent2D &extent) const;

      private:
      std::array<unsigned char, 3> const& m_rgb;
    };

    class PixelsImageGenerator
    {
      public:
      PixelsImageGenerator(vk::Extent2D const& extent, size_t channels, unsigned char const* pixels);

      void operator()(void* data, vk::Extent2D & extent) const;

      private:
      vk::Extent2D          m_extent;
      size_t                m_channels;
      unsigned char const*  m_pixels;
    };

    struct UUID
    {
      public:
      UUID(uint8_t data[VK_UUID_SIZE]);

      uint8_t m_data[VK_UUID_SIZE];
    };


    template <typename TargetType, typename SourceType>
    VULKAN_HPP_INLINE TargetType checked_cast(SourceType value)
    {
      static_assert(sizeof(TargetType) <= sizeof(SourceType), "No need to cast from smaller to larger type!");
      static_assert(!std::numeric_limits<TargetType>::is_signed, "Only unsigned types supported!");
      static_assert(!std::numeric_limits<SourceType>::is_signed, "Only unsigned types supported!");
      assert(value <= std::numeric_limits<TargetType>::max());
      return static_cast<TargetType>(value);
    }

    template <class T>
    void copyToDevice(vk::UniqueDevice const& device, vk::UniqueDeviceMemory const& memory, T const* pData, size_t count, size_t stride = sizeof(T))
    {
      assert(sizeof(T) <= stride);
      uint8_t* deviceData = static_cast<uint8_t*>(device->mapMemory(memory.get(), 0, count * stride));
      if (stride == sizeof(T))
      {
        memcpy(deviceData, pData, count * sizeof(T));
      }
      else
      {
        for (size_t i = 0; i < count; i++)
        {
          memcpy(deviceData, &pData[i], sizeof(T));
          deviceData += stride;
        }
      }
      device->unmapMemory(memory.get());
    }

    template <class T>
    void copyToDevice(vk::UniqueDevice const& device, vk::UniqueDeviceMemory const& memory, T const& data)
    {
      copyToDevice<T>(device, memory, &data, 1);
    }

    template<class T>
    VULKAN_HPP_INLINE constexpr const T& clamp(const T& v, const T& lo, const T& hi)
    {
      return v < lo ? lo : hi < v ? hi : v;
    }

    template <typename Func>
    void oneTimeSubmit(vk::UniqueCommandBuffer const& commandBuffer, vk::Queue const& queue, Func const& func)
    {
      commandBuffer->begin(vk::CommandBufferBeginInfo(vk::CommandBufferUsageFlagBits::eOneTimeSubmit));
      func(commandBuffer);
      commandBuffer->end();
      queue.submit(vk::SubmitInfo(0, nullptr, nullptr, 1, &(*commandBuffer)), nullptr);
      queue.waitIdle();
    }

    template <typename Func>
    void oneTimeSubmit(vk::UniqueDevice const& device, vk::UniqueCommandPool const& commandPool, vk::Queue const& queue, Func const& func)
    {
      vk::UniqueCommandBuffer commandBuffer = std::move(device->allocateCommandBuffersUnique(vk::CommandBufferAllocateInfo(*commandPool, vk::CommandBufferLevel::ePrimary, 1)).front());
      oneTimeSubmit(commandBuffer, queue, func);
    }

    vk::UniqueDeviceMemory allocateMemory(vk::UniqueDevice const& device, vk::PhysicalDeviceMemoryProperties const& memoryProperties, vk::MemoryRequirements const& memoryRequirements,
                                          vk::MemoryPropertyFlags memoryPropertyFlags);
    vk::UniqueCommandPool createCommandPool(vk::UniqueDevice &device, uint32_t queueFamilyIndex);
    vk::UniqueDebugUtilsMessengerEXT createDebugUtilsMessenger(vk::UniqueInstance &instance);
    vk::UniqueDescriptorPool createDescriptorPool(vk::UniqueDevice &device, std::vector<vk::DescriptorPoolSize> const& poolSizes);
    vk::UniqueDescriptorSetLayout createDescriptorSetLayout(vk::UniqueDevice const& device, std::vector<std::tuple<vk::DescriptorType, uint32_t, vk::ShaderStageFlags>> const& bindingData,
                                                            vk::DescriptorSetLayoutCreateFlags flags = {});
    std::vector<vk::UniqueFramebuffer> createFramebuffers(vk::UniqueDevice &device, vk::UniqueRenderPass &renderPass, std::vector<vk::UniqueImageView> const& imageViews, vk::UniqueImageView const& depthImageView, vk::Extent2D const& extent);
    vk::UniquePipeline createGraphicsPipeline(vk::UniqueDevice const& device, vk::UniquePipelineCache const& pipelineCache,
                                              std::pair<vk::ShaderModule, vk::SpecializationInfo const*> const& vertexShaderData,
                                              std::pair<vk::ShaderModule, vk::SpecializationInfo const*> const& fragmentShaderData, uint32_t vertexStride,
                                              std::vector<std::pair<vk::Format, uint32_t>> const& vertexInputAttributeFormatOffset, vk::FrontFace frontFace, bool depthBuffered,
                                              vk::UniquePipelineLayout const& pipelineLayout, vk::UniqueRenderPass const& renderPass);
    
    vk::UniqueRenderPass createRenderPass(vk::UniqueDevice &device, vk::Format colorFormat, vk::Format depthFormat, vk::AttachmentLoadOp loadOp = vk::AttachmentLoadOp::eClear, vk::ImageLayout colorFinalLayout = vk::ImageLayout::ePresentSrcKHR);
    VkBool32 debugUtilsMessengerCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageTypes, VkDebugUtilsMessengerCallbackDataEXT const * pCallbackData, void * /*pUserData*/);
    uint32_t findMemoryType(vk::PhysicalDeviceMemoryProperties const& memoryProperties, uint32_t typeBits, vk::MemoryPropertyFlags requirementsMask);
    std::vector<std::string> getInstanceExtensions();
    vk::Format pickDepthFormat(vk::PhysicalDevice const& physicalDevice);
    void setImageLayout(vk::UniqueCommandBuffer const& commandBuffer, vk::Image image, vk::Format format, vk::ImageLayout oldImageLayout, vk::ImageLayout newImageLayout);
    void submitAndWait(vk::UniqueDevice &device, vk::Queue queue, vk::UniqueCommandBuffer &commandBuffer);

  }
}

std::ostream& operator<<(std::ostream& os, vk::su::UUID const& uuid);