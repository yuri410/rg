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
