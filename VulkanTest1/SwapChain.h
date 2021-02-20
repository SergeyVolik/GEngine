#ifndef MY_VULKAN_SWAPCHAIN
#define MY_VULKAN_SWAPCHAIN

#define VULKAN_HPP_DISPATCH_LOADER_DYNAMIC 1


#include<vulkan/vulkan.hpp>
#include <optional>
#include <vector>

#include "VulkanHelper.h"
#include "VulkanFrameBuffer.h"
namespace vkGame {

   

	class VulkanSwapChain
	{
    public:
        //буфер изображения двойной-тройной буферизации
        std::vector <vkGame::VulkanFrameBuffer*> framebuffers;
    private:
        te::vkh::VulkanDevice* device;
        //поверхность для выводна на екран
      
        vk::SurfaceKHR surface;

        te::vkh::SwapChainSupportDetails details;
       
        // дескриптов для дувойной-тройной буферизации
        vk::SwapchainKHR swapChain;

        //список изображений для каждого кадра последовательности буферзации изображений SwapchainKHR
        std::vector<vk::Image> swapChainImages;

        //вью изображения для прямого обрашению к изображению
        std::vector<vk::ImageView> swapChainImageViews;

        

        //цветовой формат отображенного изображения
        vk::Format colorFormat;

        //разрежение екрана отображения
        vk::Extent2D swapChainExtent;

        int width;
        int height;

        

        void createSwapChainImageViews()
        {
            swapChainImageViews.resize(swapChainImages.size());

            for (uint32_t i = 0; i < swapChainImages.size(); i++) {
                swapChainImageViews[i] = device->createImageView(swapChainImages[i], colorFormat, vk::ImageAspectFlagBits::eColor, 1);
            }
        }

        vk::SurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<vk::SurfaceFormatKHR>& availableFormats)
        {
            for (const auto& availableFormat : availableFormats) {
                if (availableFormat.format == vk::Format::eB8G8R8A8Srgb &&
                    availableFormat.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear) {
                    return availableFormat;
                }
            }

            return availableFormats[0];
        }

        vk::PresentModeKHR chooseSwapPresentMode(const std::vector<vk::PresentModeKHR>& availablePresentModes)
        {
            for (const auto& availablePresentMode : availablePresentModes) {
                if (availablePresentMode == vk::PresentModeKHR::eMailbox) {
                    return availablePresentMode;
                }
            }

            return vk::PresentModeKHR::eFifo;
        }

        vk::Extent2D chooseSwapExtent(const vk::SurfaceCapabilitiesKHR& capabilities)
        {
            if (capabilities.currentExtent.width != UINT32_MAX) {
                return capabilities.currentExtent;
            }
            else {
                              

                vk::Extent2D actualExtent = {
                    static_cast<uint32_t>(width),
                    static_cast<uint32_t>(height)
                };

                actualExtent.width = std::max(capabilities.minImageExtent.width, std::min(capabilities.maxImageExtent.width, actualExtent.width));
                actualExtent.height = std::max(capabilities.minImageExtent.height, std::min(capabilities.maxImageExtent.height, actualExtent.height));

                return actualExtent;
            }
        }
    public:
        VulkanSwapChain(vk::SurfaceKHR surface, te::vkh::VulkanDevice* device) : surface(surface), device(device), width(0), height(0), details({})
        {

        }

        ~VulkanSwapChain()
        {
           device->instance.destroySurfaceKHR(surface, nullptr);
        }

        void createSwapChain(int newWidth, int newHeight)
        {

            width = newWidth;
            height = newHeight;

            te::vkh::SwapChainSupportDetails swapChainSupport = te::vkh::querySwapChainSupport(surface, device->physicalDevice);

            //querySwapChainSupport(physicalDevice)
            vk::SurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
            vk::PresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
            vk::Extent2D extent = chooseSwapExtent(swapChainSupport.capabilities);

            uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
            if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount) {
                imageCount = swapChainSupport.capabilities.maxImageCount;
            }

            vk::SwapchainCreateInfoKHR createInfo{};

            createInfo.surface = surface;

            createInfo.minImageCount = imageCount;
            createInfo.imageFormat = surfaceFormat.format;
            createInfo.imageColorSpace = surfaceFormat.colorSpace;
            createInfo.imageExtent = extent;
            createInfo.imageArrayLayers = 1;
            createInfo.imageUsage = vk::ImageUsageFlagBits::eColorAttachment;

            te::vkh::QueueFamilyIndices indices = te::vkh::findQueueFamilies(surface, device->physicalDevice);
            uint32_t queueFamilyIndices[] = { indices.graphicsFamily.value(), indices.presentFamily.value() };

            if (indices.graphicsFamily != indices.presentFamily) {
                createInfo.imageSharingMode = vk::SharingMode::eConcurrent;
                createInfo.queueFamilyIndexCount = 2;
                createInfo.pQueueFamilyIndices = queueFamilyIndices;
            }
            else {
                createInfo.imageSharingMode = vk::SharingMode::eExclusive;
            }

            createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
            createInfo.compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque;
            createInfo.presentMode = presentMode;
            createInfo.clipped = VK_TRUE;

            if (device->logicalDevice.createSwapchainKHR(&createInfo, nullptr, &swapChain) != vk::Result::eSuccess) {
                throw std::runtime_error("failed to create swap chain!");
            }

            swapChainImages = device->logicalDevice.getSwapchainImagesKHR(swapChain);

            colorFormat = surfaceFormat.format;
            swapChainExtent = extent;

            createSwapChainImageViews();


        }
        void destroySwapchain()
        {
            for (auto imageView : swapChainImageViews) {
                device->logicalDevice.destroyImageView(imageView, nullptr);
            }

            device->logicalDevice.destroySwapchainKHR(swapChain, nullptr);
        }
        int getChainsCount()
        {
            return swapChainImageViews.size();
        }
        vk::Format getImageFormat()
        {
            return colorFormat;
        }

        vk::Extent2D getExtent()
        {
            return swapChainExtent;
        }

        

        void destroyFramebuffers()
        {          
            for (int i = 0; i < framebuffers[0]->attachments.size(); i++)
            {
                if (framebuffers[0]->attachments[i].uniqueAttachment)
                {
                    const auto& attachment = framebuffers[0]->attachments[i];

                        
                    device->logicalDevice.destroyImage(attachment.image);
                    device->logicalDevice.destroyImageView(attachment.view);
                    device->logicalDevice.freeMemory(attachment.memory);
                }

            }

           
                device->logicalDevice.destroyRenderPass(framebuffers[0]->renderPass);

          
                device->logicalDevice.destroySampler(framebuffers[0]->sampler);

            for (int i = 0; i < framebuffers.size(); i++)
            {
                delete framebuffers[i];
            }
        }
    
       
        vk::RenderPass getRenderPass()
        {
            return framebuffers[0]->renderPass;
        }

        vk::Framebuffer getFrameBufferByIndex(int i)
        {
            return framebuffers[i]->framebuffer;
        }

        vk::SwapchainKHR getSwapchain()
        {
            return swapChain;
        }

        void initFrameBuffers()
        {
            framebuffers.resize(swapChainImageViews.size());

            for (size_t i = 0; i < swapChainImageViews.size(); i++) {
                framebuffers[i] = new vkGame::VulkanFrameBuffer(device);
            }
        }
        void createFramebuffers()
        {
      
            for (size_t i = 0; i < swapChainImageViews.size(); i++) {


                framebuffers[i]->width = swapChainExtent.width;
                framebuffers[i]->height = swapChainExtent.height;
            
                framebuffers[i]->createFramebuffer(swapChainImageViews[i]);

            }
        }


	};
}

#endif // !MY_VULKAN_SWAPCHAIN
