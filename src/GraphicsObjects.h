#pragma once

#include "Common.h"
#include <vulkan/vulkan.hpp>

class RenderWindow
{
public:
    RenderWindow(const vk::UniqueInstance& instance, uint32 width = 1024, uint32 height = 1024, const string& title = "Window");
    ~RenderWindow();

    const vk::UniqueSurfaceKHR& getVKSurface() const { return m_surface; }

    uint32 getWidth() const { return m_width; }
    uint32 getHeight() const { return m_height; }

private:
    static LRESULT CALLBACK WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

    HWND                  m_hwnd;
    vk::UniqueSurfaceKHR  m_surface;

    uint32                m_width;
    uint32                m_height;
};

class Device
{
public:
    Device(const RenderWindow& window, vk::PhysicalDevice physicalDevice);
    ~Device();

    const vk::PhysicalDevice& getPhysicalDevice() const { return m_physicalDevice; }
    const vk::UniqueDevice& getVKDevice() const { return m_device; }

    const vk::Queue& getPresentQueue() const { return m_presentQueue; }

    uint32 getGraphicsQueueFamilyIndex() const { return m_graphicsQueueFamilyIndex; }
    uint32 getPresentQueueFamilyIndex() const { return m_presentQueueFamilyIndex; }

    void updateDescriptorSets(const vk::UniqueDescriptorSet& descriptorSet,
                              const std::vector<std::tuple<vk::DescriptorType, const vk::UniqueBuffer&, const vk::UniqueBufferView&>>& bufferData,
                              const std::pair<Texture, const vk::UniqueSampler&>& sr,
                              uint32_t bindingOffset = 0);
    
    void updateDescriptorSets(const vk::UniqueDescriptorSet& descriptorSet,
                              const std::vector<std::tuple<vk::DescriptorType, const vk::UniqueBuffer&, const vk::UniqueBufferView&>>& bufferData,
                              const std::vector<Texture>& textureData, uint32_t bindingOffset = 0);

private:
    vk::PhysicalDevice    m_physicalDevice;
    vk::UniqueDevice      m_device;
    vk::Queue             m_graphicsQueue;
    vk::Queue             m_presentQueue;

    uint32                m_graphicsQueueFamilyIndex;
    uint32                m_presentQueueFamilyIndex;
};

class SwapChain
{
public:
    SwapChain(const Device& device, const RenderWindow& window, vk::ImageUsageFlags usage, const vk::UniqueSwapchainKHR& oldSwapChain);
    ~SwapChain();

    void Acquire();
    void Present();

    uint32 getCurrentImageIndex() const { return m_currentBufferIndex.value(); } 

private:
    const Device&                     m_device;
    vk::Format                        m_colorFormat;
    vk::UniqueSwapchainKHR            m_swapChain;
    std::vector<vk::Image>            m_images;
    std::vector<vk::UniqueImageView>  m_imageViews;

    vk::UniqueSemaphore               m_imageAcquiredSemaphore;

    std::optional<uint32>             m_currentBufferIndex;
};

class Buffer
{
public:
    Buffer(const Device& device, vk::DeviceSize size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags propertyFlags = vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);
    ~Buffer();

    void upload(const void* data, size_t size, size_t stride);

    template <typename DataType>
    void upload(const DataType& data) const
    {
        upload(&data, sizeof(data), 0);
    }

    template <typename DataType>
    void upload(const std::vector<DataType>& data, size_t stride = 0) const
    {
        assert(m_propertyFlags & vk::MemoryPropertyFlagBits::eHostVisible);

        size_t elementSize = stride ? stride : sizeof(DataType);
        assert(sizeof(DataType) <= elementSize);

        const vk::UniqueDevice& device = m_device.getVKDevice();

        copyToDevice(device, m_deviceMemory, data.data(), data.size(), elementSize);
    }

    template <typename DataType>
    void upload(const vk::UniqueCommandPool& commandPool, vk::Queue queue, const std::vector<DataType>& data, size_t stride) const
    {
        assert(m_usage & vk::BufferUsageFlagBits::eTransferDst);
        assert(m_propertyFlags & vk::MemoryPropertyFlagBits::eDeviceLocal);

        size_t elementSize = stride ? stride : sizeof(DataType);
        assert(sizeof(DataType) <= elementSize);

        size_t dataSize = data.size() * elementSize;
        assert(dataSize <= m_size);

        Buffer stagingBuffer(m_device, dataSize, vk::BufferUsageFlagBits::eTransferSrc);
        copyToDevice(device, stagingBuffer.m_deviceMemory, data.data(), data.size(), elementSize);

        vk::su::oneTimeSubmit(device, commandPool, queue, [&](const vk::UniqueCommandBuffer& commandBuffer) 
        {
            commandBuffer->copyBuffer(*stagingBuffer.m_buffer, *this->m_buffer, vk::BufferCopy(0, 0, dataSize)); 
        });
    }

private:
    const Device&           m_device;
    vk::UniqueBuffer        m_buffer;
    vk::UniqueDeviceMemory  m_deviceMemory;
    vk::DeviceSize          m_size;
    vk::BufferUsageFlags    m_usage;
    vk::MemoryPropertyFlags m_propertyFlags;

};

class Image
{
public:
    Image(const Device& device, vk::Format format, const vk::Extent2D& extent, vk::ImageTiling tiling, vk::ImageUsageFlags usage, 
          vk::ImageLayout initialLayout, vk::MemoryPropertyFlags memoryProperties, vk::ImageAspectFlags aspectMask);
    virtual ~Image();

private:
    vk::Format              m_format;
    vk::UniqueImage         m_image;
    vk::UniqueDeviceMemory  m_deviceMemory;
    vk::UniqueImageView     m_imageView;
};

class DepthBuffer : public Image
{
public:
    DepthBuffer(const Device& device, vk::Format format, const vk::Extent2D& extent);
};

class Texture
{
public:
    Texture(const Device& device, const vk::Extent2D& extent_ = { 256, 256 }, vk::ImageUsageFlags usageFlags = {},
            vk::FormatFeatureFlags formatFeatureFlags = {}, bool anisotropyEnable = false, bool forceStaging = false);
    ~Texture();

    template <typename ImageGenerator>
    void setImage(const Device& device, const vk::UniqueCommandBuffer& commandBuffer, const ImageGenerator& imageGenerator)
    {
        const vk::UniqueDevice& vkDevice = device.getVKDevice();

        void* data = m_needsStaging
            ? vkDevice->mapMemory(m_stagingBufferData->m_deviceMemory.get(), 0, vkDevice->getBufferMemoryRequirements(m_stagingBufferData->m_buffer.get()).size)
            : vkDevice->mapMemory(m_imageData->m_deviceMemory.get(), 0, vkDevice->getImageMemoryRequirements(m_imageData->m_image.get()).size);
        imageGenerator(data, m_extent);
        vkDevice->unmapMemory(m_needsStaging ? m_stagingBufferData->m_deviceMemory.get() : m_imageData->m_deviceMemory.get());

        if (m_needsStaging)
        {
            // Since we're going to blit to the texture image, set its layout to eTransferDstOptimal
            vk::su::setImageLayout(commandBuffer, m_imageData->m_image.get(), m_imageData->m_format, vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal);
            vk::BufferImageCopy copyRegion(0, m_extent.width, m_extent.height, vk::ImageSubresourceLayers(vk::ImageAspectFlagBits::eColor, 0, 0, 1), vk::Offset3D(0, 0, 0), vk::Extent3D(extent, 1));
            commandBuffer->copyBufferToImage(m_stagingBufferData->m_buffer.get(), m_imageData->m_image.get(), vk::ImageLayout::eTransferDstOptimal, copyRegion);
            // Set the layout for the texture image from eTransferDstOptimal to SHADER_READ_ONLY
            vk::su::setImageLayout(commandBuffer, m_imageData->m_image.get(), m_imageData->m_format, vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::eShaderReadOnlyOptimal);
        }
        else
        {
            // If we can use the linear tiled image as a texture, just do it
            vk::su::setImageLayout(commandBuffer, m_imageData->m_image.get(), m_imageData->m_format, vk::ImageLayout::ePreinitialized, vk::ImageLayout::eShaderReadOnlyOptimal);
        }
    }

private:

    vk::Format                  m_format;
    vk::Extent2D                m_extent;
    bool                        m_needsStaging;
    std::unique_ptr<Buffer>     m_stagingBufferData;
    std::unique_ptr<Image>      m_imageData;
};