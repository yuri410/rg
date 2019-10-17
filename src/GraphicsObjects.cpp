#include "GraphicsObjects.h"

const uint64 FenceTimeout = 100000000;

//////////////////////////////////////////////////////////////////////////

RenderWindow::RenderWindow(const vk::UniqueInstance& vkInstance, uint32 width, uint32 height, const string& title)
    : m_width(width)
    , m_height(height)
{
    WNDCLASSEX windowClass;
    memset(&windowClass, 0, sizeof(WNDCLASSEX));

    HINSTANCE hInstance = GetModuleHandle(nullptr);
    windowClass.cbSize = sizeof(WNDCLASSEX);
    windowClass.style = CS_HREDRAW | CS_VREDRAW;
    windowClass.lpfnWndProc = WindowProc;
    windowClass.hInstance = hInstance;
    windowClass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    windowClass.hCursor = LoadCursor(NULL, IDC_ARROW);
    windowClass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
    windowClass.lpszClassName = "STATIC";
    windowClass.hIconSm = LoadIcon(NULL, IDI_WINLOGO);

    if (!RegisterClassEx(&windowClass))
    {
        assert(0);
    }

    RECT windowRect = { 0, 0, width, height };
    AdjustWindowRect(&windowRect, WS_OVERLAPPEDWINDOW, FALSE);

    m_hwnd = CreateWindowEx(0, windowClass.lpszClassName, title.c_str(), WS_OVERLAPPEDWINDOW | WS_VISIBLE | WS_SYSMENU, 100, 100, windowRect.right - windowRect.left,
                            windowRect.bottom - windowRect.top, nullptr, nullptr, hInstance, nullptr);

    m_surface = vkInstance->createWin32SurfaceKHRUnique(vk::Win32SurfaceCreateInfoKHR(vk::Win32SurfaceCreateFlagsKHR(), hInstance, m_hwnd));
}

RenderWindow::~RenderWindow()
{
    DestroyWindow(m_hwnd);
}

LRESULT RenderWindow::WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
        case WM_CLOSE:
            PostQuitMessage(0);
            break;
        default:
            break;
    }
    return (DefWindowProc(hWnd, uMsg, wParam, lParam));
}

//////////////////////////////////////////////////////////////////////////

uint32_t findGraphicsQueueFamilyIndex(const std::vector<vk::QueueFamilyProperties>& queueFamilyProperties)
{
    // get the first index into queueFamiliyProperties which supports graphics
    size_t graphicsQueueFamilyIndex = std::distance(queueFamilyProperties.begin(), std::find_if(queueFamilyProperties.begin(), queueFamilyProperties.end(),
                                                                                                [](const vk::QueueFamilyProperties& qfp) { return qfp.queueFlags & vk::QueueFlagBits::eGraphics; }));
    assert(graphicsQueueFamilyIndex < queueFamilyProperties.size());

    return (uint32_t)graphicsQueueFamilyIndex;
}

std::pair<uint32_t, uint32_t> findGraphicsAndPresentQueueFamilyIndex(vk::PhysicalDevice physicalDevice, const vk::SurfaceKHR& surface)
{
    std::vector<vk::QueueFamilyProperties> queueFamilyProperties = physicalDevice.getQueueFamilyProperties();
    assert(queueFamilyProperties.size() < std::numeric_limits<uint32_t>::max());

    uint32_t graphicsQueueFamilyIndex = findGraphicsQueueFamilyIndex(queueFamilyProperties);
    if (physicalDevice.getSurfaceSupportKHR(graphicsQueueFamilyIndex, surface))
    {
        return std::make_pair(graphicsQueueFamilyIndex, graphicsQueueFamilyIndex);    // the first graphicsQueueFamilyIndex does also support presents
    }

    // the graphicsQueueFamilyIndex doesn't support present -> look for an other family index that supports both graphics and present
    for (size_t i = 0; i < queueFamilyProperties.size(); i++)
    {
        if ((queueFamilyProperties[i].queueFlags & vk::QueueFlagBits::eGraphics) && physicalDevice.getSurfaceSupportKHR(static_cast<uint32_t>(i), surface))
        {
            return std::make_pair(static_cast<uint32_t>(i), static_cast<uint32_t>(i));
        }
    }

    // there's nothing like a single family index that supports both graphics and present -> look for an other family index that supports present
    for (size_t i = 0; i < queueFamilyProperties.size(); i++)
    {
        if (physicalDevice.getSurfaceSupportKHR(static_cast<uint32_t>(i), surface))
        {
            return std::make_pair(graphicsQueueFamilyIndex, static_cast<uint32_t>(i));
        }
    }

    throw std::runtime_error("Could not find queues for both graphics or present -> terminating");
}

std::vector<std::string> getDeviceExtensions()
{
    return{ VK_KHR_SWAPCHAIN_EXTENSION_NAME };
}

Device::Device(const RenderWindow& window, vk::PhysicalDevice physicalDevice)
    : m_physicalDevice(physicalDevice)
{
    auto queueIndices = findGraphicsAndPresentQueueFamilyIndex(m_physicalDevice, window.getVKSurface().get());

    m_graphicsQueueFamilyIndex = queueIndices.first;
    m_presentQueueFamilyIndex = queueIndices.second;

    std::vector<const char*> enabledExtensions;
    for (const auto& ext : getDeviceExtensions())
    {
        enabledExtensions.push_back(ext.data());
    }

    // create a UniqueDevice
    float queuePriority = 0.0f;
    vk::DeviceQueueCreateInfo deviceQueueCreateInfo(vk::DeviceQueueCreateFlags(), m_graphicsQueueFamilyIndex, 1, &queuePriority);
    vk::DeviceCreateInfo deviceCreateInfo(vk::DeviceCreateFlags(), 1, &deviceQueueCreateInfo, 0, nullptr, (uint32_t)enabledExtensions.size(), enabledExtensions.data(), nullptr);
    deviceCreateInfo.pNext = nullptr;
    m_device = physicalDevice.createDeviceUnique(deviceCreateInfo);

    m_graphicsQueue = m_device->getQueue(m_graphicsQueueFamilyIndex, 0);
    m_presentQueue = m_device->getQueue(m_presentQueueFamilyIndex, 0);
}

Device::~Device()
{

}

void Device::updateDescriptorSets(const vk::UniqueDescriptorSet& descriptorSet,
                                  const std::vector<std::tuple<vk::DescriptorType, const vk::UniqueBuffer&, const vk::UniqueBufferView&>>& bufferData, 
                                  const Texture& textureData, uint32_t bindingOffset)
{
    std::vector<vk::DescriptorBufferInfo> bufferInfos;
    bufferInfos.reserve(bufferData.size());

    std::vector<vk::WriteDescriptorSet> writeDescriptorSets;
    writeDescriptorSets.reserve(bufferData.size() + 1);
    uint32_t dstBinding = bindingOffset;
    for (auto const& bd : bufferData)
    {
        bufferInfos.push_back(vk::DescriptorBufferInfo(*std::get<1>(bd), 0, VK_WHOLE_SIZE));
        writeDescriptorSets.push_back(vk::WriteDescriptorSet(*descriptorSet, dstBinding++, 0, 1, std::get<0>(bd), nullptr, &bufferInfos.back(), std::get<2>(bd) ? &*std::get<2>(bd) : nullptr));
    }

    vk::DescriptorImageInfo imageInfo(*textureData.m_textureSampler, *textureData.m_imageData->m_imageView, vk::ImageLayout::eShaderReadOnlyOptimal);
    writeDescriptorSets.push_back(vk::WriteDescriptorSet(*descriptorSet, dstBinding, 0, 1, vk::DescriptorType::eCombinedImageSampler, &imageInfo, nullptr, nullptr));

    m_device->updateDescriptorSets(writeDescriptorSets, nullptr);
}

void Device::updateDescriptorSets(const vk::UniqueDescriptorSet& descriptorSet,
                                  const std::vector<std::tuple<vk::DescriptorType, const vk::UniqueBuffer&, const vk::UniqueBufferView&>>& bufferData,
                                  const std::vector<Texture>& textureData, uint32_t bindingOffset)
{
    std::vector<vk::DescriptorBufferInfo> bufferInfos;
    bufferInfos.reserve(bufferData.size());

    std::vector<vk::WriteDescriptorSet> writeDescriptorSets;
    writeDescriptorSets.reserve(bufferData.size() + textureData.empty() ? 0 : 1);
    uint32_t dstBinding = bindingOffset;
    for (auto const& bd : bufferData)
    {
        bufferInfos.push_back(vk::DescriptorBufferInfo(*std::get<1>(bd), 0, VK_WHOLE_SIZE));
        writeDescriptorSets.push_back(vk::WriteDescriptorSet(*descriptorSet, dstBinding++, 0, 1, std::get<0>(bd), nullptr, &bufferInfos.back(), std::get<2>(bd) ? &*std::get<2>(bd) : nullptr));
    }

    std::vector<vk::DescriptorImageInfo> imageInfos;
    if (!textureData.empty())
    {
        imageInfos.reserve(textureData.size());
        for (auto const& td : textureData)
        {
            imageInfos.push_back(vk::DescriptorImageInfo(*td.m_textureSampler, *td.m_imageData->m_imageView, vk::ImageLayout::eShaderReadOnlyOptimal));
        }
        writeDescriptorSets.push_back(vk::WriteDescriptorSet(*descriptorSet, dstBinding, 0, (uint32_t)imageInfos.size(), vk::DescriptorType::eCombinedImageSampler, imageInfos.data(),
                                                             nullptr, nullptr));
    }

    m_device->updateDescriptorSets(writeDescriptorSets, nullptr);
}


/////////////////////////////////////////////////////////////////////////

vk::PresentModeKHR pickPresentMode(std::vector<vk::PresentModeKHR> const& presentModes)
{
    vk::PresentModeKHR pickedMode = vk::PresentModeKHR::eFifo;
    for (const auto& presentMode : presentModes)
    {
        if (presentMode == vk::PresentModeKHR::eMailbox)
        {
            pickedMode = presentMode;
            break;
        }

        if (presentMode == vk::PresentModeKHR::eImmediate)
        {
            pickedMode = presentMode;
        }
    }
    return pickedMode;
}

vk::SurfaceFormatKHR pickSurfaceFormat(std::vector<vk::SurfaceFormatKHR> const& formats)
{
    assert(!formats.empty());
    vk::SurfaceFormatKHR pickedFormat = formats[0];
    if (formats.size() == 1)
    {
        if (formats[0].format == vk::Format::eUndefined)
        {
            pickedFormat.format = vk::Format::eB8G8R8A8Unorm;
            pickedFormat.colorSpace = vk::ColorSpaceKHR::eSrgbNonlinear;
        }
    }
    else
    {
        // request several formats, the first found will be used
        vk::Format        requestedFormats[] = { vk::Format::eB8G8R8A8Unorm, vk::Format::eR8G8B8A8Unorm, vk::Format::eB8G8R8Unorm, vk::Format::eR8G8B8Unorm };
        vk::ColorSpaceKHR requestedColorSpace = vk::ColorSpaceKHR::eSrgbNonlinear;
        for (size_t i = 0; i < sizeof(requestedFormats) / sizeof(requestedFormats[0]); i++)
        {
            vk::Format requestedFormat = requestedFormats[i];
            auto it = std::find_if(formats.begin(), formats.end(), [requestedFormat, requestedColorSpace](auto const& f) { return (f.format == requestedFormat) && (f.colorSpace == requestedColorSpace); });
            if (it != formats.end())
            {
                pickedFormat = *it;
                break;
            }
        }
    }
    assert(pickedFormat.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear);
    return pickedFormat;
}

SwapChain::SwapChain(const Device& device, const RenderWindow& window, vk::ImageUsageFlags usage, const vk::UniqueSwapchainKHR& oldSwapChain)
    : m_device(device)
{
    const vk::PhysicalDevice& physicalDevice = device.getPhysicalDevice();
    const vk::UniqueDevice& vkdevice = device.getVKDevice();
    const vk::SurfaceKHR& surface = window.getVKSurface().get();
    const uint32 graphicsQueueFamilyIndex = device.getGraphicsQueueFamilyIndex();
    const uint32 presentQueueFamilyIndex = device.getPresentQueueFamilyIndex();

    const uint32 surfaceWidth = window.getWidth();
    const uint32 surfaceHeight = window.getHeight();

    vk::SurfaceFormatKHR surfaceFormat = pickSurfaceFormat(physicalDevice.getSurfaceFormatsKHR(surface));
    m_colorFormat = surfaceFormat.format;

    vk::SurfaceCapabilitiesKHR surfaceCapabilities = physicalDevice.getSurfaceCapabilitiesKHR(surface);
    VkExtent2D swapchainExtent;
    if (surfaceCapabilities.currentExtent.width == std::numeric_limits<uint32_t>::max())
    {
        // If the surface size is undefined, the size is set to the size of the images requested.
        swapchainExtent.width = clamp(surfaceWidth, surfaceCapabilities.minImageExtent.width, surfaceCapabilities.maxImageExtent.width);
        swapchainExtent.height = clamp(surfaceHeight, surfaceCapabilities.minImageExtent.height, surfaceCapabilities.maxImageExtent.height);
    }
    else
    {
        // If the surface size is defined, the swap chain size must match
        swapchainExtent = surfaceCapabilities.currentExtent;
    }
    vk::SurfaceTransformFlagBitsKHR preTransform = (surfaceCapabilities.supportedTransforms & vk::SurfaceTransformFlagBitsKHR::eIdentity) ? vk::SurfaceTransformFlagBitsKHR::eIdentity : surfaceCapabilities.currentTransform;
    vk::CompositeAlphaFlagBitsKHR compositeAlpha =
        (surfaceCapabilities.supportedCompositeAlpha & vk::CompositeAlphaFlagBitsKHR::ePreMultiplied) ? vk::CompositeAlphaFlagBitsKHR::ePreMultiplied :
        (surfaceCapabilities.supportedCompositeAlpha & vk::CompositeAlphaFlagBitsKHR::ePostMultiplied) ? vk::CompositeAlphaFlagBitsKHR::ePostMultiplied :
        (surfaceCapabilities.supportedCompositeAlpha & vk::CompositeAlphaFlagBitsKHR::eInherit) ? vk::CompositeAlphaFlagBitsKHR::eInherit : vk::CompositeAlphaFlagBitsKHR::eOpaque;
    vk::PresentModeKHR presentMode = pickPresentMode(physicalDevice.getSurfacePresentModesKHR(surface));
    vk::SwapchainCreateInfoKHR swapChainCreateInfo({}, surface, surfaceCapabilities.minImageCount, m_colorFormat, surfaceFormat.colorSpace, swapchainExtent, 1, usage, vk::SharingMode::eExclusive,
                                                   0, nullptr, preTransform, compositeAlpha, presentMode, true, *oldSwapChain);
    if (graphicsQueueFamilyIndex != presentQueueFamilyIndex)
    {
        uint32_t queueFamilyIndices[2] = { graphicsQueueFamilyIndex, presentQueueFamilyIndex };
        // If the graphics and present queues are from different queue families, we either have to explicitly transfer ownership of images between
        // the queues, or we have to create the swapchain with imageSharingMode as vk::SharingMode::eConcurrent
        swapChainCreateInfo.imageSharingMode = vk::SharingMode::eConcurrent;
        swapChainCreateInfo.queueFamilyIndexCount = 2;
        swapChainCreateInfo.pQueueFamilyIndices = queueFamilyIndices;
    }
    m_swapChain = vkdevice->createSwapchainKHRUnique(swapChainCreateInfo);
    m_images = vkdevice->getSwapchainImagesKHR(m_swapChain.get());

    m_imageViews.reserve(m_images.size());
    vk::ComponentMapping componentMapping(vk::ComponentSwizzle::eR, vk::ComponentSwizzle::eG, vk::ComponentSwizzle::eB, vk::ComponentSwizzle::eA);
    vk::ImageSubresourceRange subResourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1);
    for (auto image : m_images)
    {
        vk::ImageViewCreateInfo imageViewCreateInfo(vk::ImageViewCreateFlags(), image, vk::ImageViewType::e2D, m_colorFormat, componentMapping, subResourceRange);
        m_imageViews.push_back(vkdevice->createImageViewUnique(imageViewCreateInfo));
    }

    m_imageAcquiredSemaphore = vkdevice->createSemaphoreUnique(vk::SemaphoreCreateInfo());
}

SwapChain::~SwapChain()
{

}

void SwapChain::Acquire()
{
    vk::ResultValue<uint32_t> currentBufferIndex = m_device.getVKDevice()->acquireNextImageKHR(m_swapChain.get(), FenceTimeout, m_imageAcquiredSemaphore.get(), nullptr);

    //m_currentBufferIndex = m_device.getVKDevice()->acquireNextImageKHR(m_swapChain.get(), vk::su::FenceTimeout, m_imageAcquiredSemaphore.get(), nullptr);
    assert(currentBufferIndex.result == vk::Result::eSuccess);
    assert(currentBufferIndex.value < m_imageViews.size());

    m_currentBufferIndex = currentBufferIndex.value;
}

void SwapChain::Present()
{
    const vk::Queue& presentQueue = m_device.getPresentQueue();
    presentQueue.presentKHR(vk::PresentInfoKHR(0, nullptr, 1, &m_swapChain.get(), &m_currentBufferIndex.value));
}

/////////////////////////////////////////////////////////////////////////

Buffer::Buffer(const Device& device, vk::DeviceSize size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags propertyFlags)
    : m_device(device)
    , m_size(size)
    , m_usage(usage)
    , m_propertyFlags(propertyFlags)
{
    const vk::UniqueDevice& vkDevice = device.getVKDevice();

    m_buffer = vkDevice->createBufferUnique(vk::BufferCreateInfo(vk::BufferCreateFlags(), size, usage));
    m_deviceMemory = vk::su::allocateMemory(vkDevice, device.getPhysicalDevice().getMemoryProperties(), vkDevice->getBufferMemoryRequirements(m_buffer.get()), propertyFlags);
    vkDevice->bindBufferMemory(m_buffer.get(), m_deviceMemory.get(), 0);
}

Buffer::~Buffer()
{

}

void Buffer::upload(const void* data, size_t size, size_t stride)
{
    assert((m_propertyFlags & vk::MemoryPropertyFlagBits::eHostCoherent) && (m_propertyFlags & vk::MemoryPropertyFlagBits::eHostVisible));
    assert(size <= m_size);

    const vk::UniqueDevice& device = m_device.getVKDevice();

    void* dataPtr = device->mapMemory(*m_deviceMemory, 0, size);
    memcpy(dataPtr, &data, size);
    device->unmapMemory(*m_deviceMemory);
}

/////////////////////////////////////////////////////////////////////////

Image::Image(const Device& device, vk::Format format_, const vk::Extent2D& extent, vk::ImageTiling tiling, vk::ImageUsageFlags usage, vk::ImageLayout initialLayout, vk::MemoryPropertyFlags memoryProperties, vk::ImageAspectFlags aspectMask)
    : m_format(format_)
{
    const vk::UniqueDevice& vkDevice = device.getVKDevice();
    const vk::PhysicalDevice& physicalDevice = device.getPhysicalDevice();

    vk::ImageCreateInfo imageCreateInfo(vk::ImageCreateFlags(), vk::ImageType::e2D, m_format, vk::Extent3D(extent, 1), 1, 1,
                                        vk::SampleCountFlagBits::e1, tiling, usage | vk::ImageUsageFlagBits::eSampled, vk::SharingMode::eExclusive, 0, nullptr, initialLayout);
    m_image = vkDevice->createImageUnique(imageCreateInfo);

    m_deviceMemory = vk::su::allocateMemory(vkDevice, physicalDevice.getMemoryProperties(), vkDevice->getImageMemoryRequirements(m_image.get()), memoryProperties);

    vkDevice->bindImageMemory(m_image.get(), m_deviceMemory.get(), 0);

    vk::ComponentMapping componentMapping(vk::ComponentSwizzle::eR, vk::ComponentSwizzle::eG, vk::ComponentSwizzle::eB, vk::ComponentSwizzle::eA);
    vk::ImageViewCreateInfo imageViewCreateInfo(vk::ImageViewCreateFlags(), m_image.get(), vk::ImageViewType::e2D, m_format, componentMapping, vk::ImageSubresourceRange(aspectMask, 0, 1, 0, 1));
    m_imageView = vkDevice->createImageViewUnique(imageViewCreateInfo);
}

Image::~Image()
{

}

/////////////////////////////////////////////////////////////////////////

DepthBuffer::DepthBuffer(const Device& device, vk::Format format, const vk::Extent2D& extent)
    : Image(device, format, extent, vk::ImageTiling::eOptimal, vk::ImageUsageFlagBits::eDepthStencilAttachment, vk::ImageLayout::eUndefined,
                vk::MemoryPropertyFlagBits::eDeviceLocal, vk::ImageAspectFlagBits::eDepth)
{
}

/////////////////////////////////////////////////////////////////////////

Texture::Texture(const Device& device, const vk::Extent2D& extent_, vk::ImageUsageFlags usageFlags, vk::FormatFeatureFlags formatFeatureFlags, bool anisotropyEnable, bool forceStaging)
    : m_format(vk::Format::eR8G8B8A8Unorm)
    , m_extent(extent_)
{
    const vk::UniqueDevice& vkDevice = device.getVKDevice();
    const vk::PhysicalDevice& physicalDevice = device.getPhysicalDevice();

    vk::PhysicalDeviceMemoryProperties memoryProperties = physicalDevice.getMemoryProperties();
    vk::FormatProperties formatProperties = physicalDevice.getFormatProperties(m_format);

    formatFeatureFlags |= vk::FormatFeatureFlagBits::eSampledImage;
    m_needsStaging = forceStaging || ((formatProperties.linearTilingFeatures & formatFeatureFlags) != formatFeatureFlags);
    vk::ImageTiling imageTiling;
    vk::ImageLayout initialLayout;
    vk::MemoryPropertyFlags requirements;
    if (m_needsStaging)
    {
        assert((formatProperties.optimalTilingFeatures & formatFeatureFlags) == formatFeatureFlags);
        m_stagingBufferData = std::make_unique<Buffer>(physicalDevice, device, m_extent.width * m_extent.height * 4, vk::BufferUsageFlagBits::eTransferSrc);
        imageTiling = vk::ImageTiling::eOptimal;
        usageFlags |= vk::ImageUsageFlagBits::eTransferDst;
        initialLayout = vk::ImageLayout::eUndefined;
    }
    else
    {
        imageTiling = vk::ImageTiling::eLinear;
        initialLayout = vk::ImageLayout::ePreinitialized;
        requirements = vk::MemoryPropertyFlagBits::eHostCoherent | vk::MemoryPropertyFlagBits::eHostVisible;
    }
    m_imageData = std::make_unique<Image>(physicalDevice, device, m_format, m_extent, imageTiling, usageFlags | vk::ImageUsageFlagBits::eSampled, initialLayout, requirements,
                                            vk::ImageAspectFlagBits::eColor);

    //textureSampler = device->createSamplerUnique(vk::SamplerCreateInfo(vk::SamplerCreateFlags(), vk::Filter::eLinear, vk::Filter::eLinear, vk::SamplerMipmapMode::eLinear,
    //                                                                   vk::SamplerAddressMode::eRepeat, vk::SamplerAddressMode::eRepeat, vk::SamplerAddressMode::eRepeat, 0.0f, anisotropyEnable,
    //                                                                   16.0f, false, vk::CompareOp::eNever, 0.0f, 0.0f, vk::BorderColor::eFloatOpaqueBlack));
}

Texture::~Texture()
{

}
