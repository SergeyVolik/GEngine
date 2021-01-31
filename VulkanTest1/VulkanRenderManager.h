#ifndef VULKAN_RENDERER_MANAGER
#define VULKAN_RENDERER_MANAGER
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.hpp>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/hash.hpp>


#include <stb_image.h>
#include <tiny_obj_loader.h>

#include <iostream>
#include <fstream>
#include <stdexcept>
#include <algorithm>
#include <chrono>
#include <vector>
#include <cstring>
#include <cstdlib>
#include <cstdint>
#include <array>
#include <optional>
#include <set>
#include <unordered_map>

#include "EngineSystem.h"
#include "Vertex.h"
#include "Window.h"
#include "InputManager.h"
#include "VulkanHelper.h"
#include "Entity.h"
#include "Transform.h"
#include "Renderer.h"

const uint32_t WIDTH = 800;
const uint32_t HEIGHT = 600;

const std::string MODEL_PATH = "models/viking_room.obj";
const std::string TEXTURE_PATH = "textures/viking_room.png";

const int MAX_FRAMES_IN_FLIGHT = 2;

#ifdef NDEBUG
const bool enableValidationLayers = false;
#else
const bool enableValidationLayers = true;
#endif



const std::vector<const char*> validationLayers = {
    "VK_LAYER_KHRONOS_validation"
};

const std::vector<const char*> deviceExtensions = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

struct UniformBufferObject {
    alignas(16) glm::mat4 model;
    alignas(16) glm::mat4 view;
    alignas(16) glm::mat4 proj;
};

struct QueueFamilyIndices {
    std::optional<uint32_t> graphicsFamily;
    std::optional<uint32_t> presentFamily;

    bool isComplete() {
        return graphicsFamily.has_value() && presentFamily.has_value();
    }
};

struct SwapChainSupportDetails {
    vk::SurfaceCapabilitiesKHR capabilities;
    std::vector<vk::SurfaceFormatKHR> formats;
    std::vector<vk::PresentModeKHR> presentModes;
};

namespace te
{

    class VulkanRenderManager : public te::EngineSystem<VulkanRenderManager>
    {
    private:

       
        te::Window* window;

        vk::Instance instance;
        vk::DebugUtilsMessengerEXT debugMessenger;

        //VkSurfaceKHR surface;
        vk::SurfaceKHR surface;

        //VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
        vk::PhysicalDevice physicalDevice;
        Entity* gObject;
        Transform* gTransform;
        vk::Queue graphicsQueue;
        vk::Queue presentQueue;

        vk::SwapchainKHR swapChain;
        std::vector<vk::Image> swapChainImages;
        vk::Format swapChainImageFormat;
        vk::Extent2D swapChainExtent;
        std::vector<vk::ImageView> swapChainImageViews;
        std::vector<vk::Framebuffer> swapChainFramebuffers;

        vk::RenderPass renderPass;
        vk::DescriptorSetLayout descriptorSetLayout;
        vk::PipelineLayout pipelineLayout;
        vk::Pipeline graphicsPipeline;

        vk::CommandPool commandPool;

        vk::Image depthImage;
        vk::DeviceMemory depthImageMemory;
        vk::ImageView depthImageView;

        uint32_t mipLevels;
        vk::Image textureImage;
        vk::DeviceMemory textureImageMemory;
        vk::ImageView textureImageView;
        vk::Sampler textureSampler;

        std::vector<te::Vertex> vertices;
        std::vector<uint32_t> _indices;
        vk::Buffer vertexBuffer;
        vk::DeviceMemory vertexBufferMemory;
        vk::Buffer _indexBuffer;
        vk::DeviceMemory _indexBufferMemory;

        std::vector<vk::Buffer> uniformBuffers;
        std::vector<vk::DeviceMemory> uniformBuffersMemory;

        vk::DescriptorPool descriptorPool;
        std::vector<vk::DescriptorSet> descriptorSets;

        std::vector<vk::CommandBuffer> commandBuffers;

        std::vector<vk::Semaphore> imageAvailableSemaphores;
        std::vector<vk::Semaphore> renderFinishedSemaphores;
        std::vector<vk::Fence> inFlightFences;
        std::vector<vk::Fence> imagesInFlight;

        
        size_t currentFrame = 0;

       
        void createInstance();
        void setupDebugMessenger();
        void createSurface();
        void pickPhysicalDevice();
        void createLogicalDevice();
        void createSwapChain();
        void createSwapChainImageViews();
        void createRenderPass();
        void createDescriptorSetLayout();
        void createGraphicsPipeline();
        void createCommandPool();
        void createDepthResources();
        void createFramebuffers();
        void createTextureImage(vk::Image&, uint32_t&);
        void createTextureImageView(vk::ImageView& imgView, const vk::Image textureImage, const uint32_t mipLevels);
        void createTextureSampler();
        void loadModel();
       
        void createVulkanMemoryAllocator();

        void createUniformBuffers();
        void createDescriptorPool();
        void createDescriptorSets();
        void createCommandBuffers();
        void createSyncObjects();
       
        void updateUniformBuffer(uint32_t currentImage);
      

        void generateMipmaps(vk::Image image, vk::Format imageFormat, int32_t texWidth, int32_t texHeight, uint32_t mipLevels);
        void transitionImageLayout(vk::Image image, vk::Format format, vk::ImageLayout oldLayout, vk::ImageLayout newLayout, uint32_t mipLevels);
        
    
       
        vk::ImageView createImageView(vk::Image image, vk::Format format, vk::ImageAspectFlags aspectFlags, uint32_t mipLevels);
        void populateDebugMessengerCreateInfo(vk::DebugUtilsMessengerCreateInfoEXT& createInfo);
        std::vector<const char*> getRequiredExtensions();
        bool isDeviceSuitable(vk::PhysicalDevice device);
        QueueFamilyIndices findQueueFamilies(vk::PhysicalDevice device);
        SwapChainSupportDetails querySwapChainSupport(vk::PhysicalDevice device);
        vk::SurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<vk::SurfaceFormatKHR>& availableFormats);
        bool checkDeviceExtensionSupport(vk::PhysicalDevice device);
        vk::PresentModeKHR chooseSwapPresentMode(const std::vector<vk::PresentModeKHR>& availablePresentModes);
        vk::Extent2D chooseSwapExtent(const vk::SurfaceCapabilitiesKHR& capabilities);

        static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
            VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
            VkDebugUtilsMessageTypeFlagsEXT messageType,
            const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
            void* pUserData
        );
        vk::Format findDepthFormat();
        uint32_t findMemoryType(uint32_t typeFilter, vk::MemoryPropertyFlags properties);
        vk::Format findSupportedFormat(const std::vector<vk::Format>& candidates, vk::ImageTiling tiling, vk::FormatFeatureFlags features);
        void createImage(uint32_t width, uint32_t height, uint32_t mipLevels, vk::Format format, vk::ImageTiling tiling, vk::ImageUsageFlags usage, vk::MemoryPropertyFlags properties, vk::Image& image, vk::DeviceMemory& imageMemory);
        vk::ShaderModule createShaderModule(const std::vector<char>& code);
        bool checkValidationLayerSupport();

        
    public:
        std::list<Renderer*> renderers;
        vk::Device device;
        void createVertexBuffer(std::vector<te::Vertex> indices, vk::Buffer& vertexBuffer, vk::DeviceMemory& vertexBufferMemory);
        void createIndexBuffer(std::vector<uint32_t>, vk::Buffer&, vk::DeviceMemory&);
        void recreateSwapChain();
        void cleanupSwapChain();
        void drawFrame();
        static void initialize(te::Window* wnd) {


            _instance = new VulkanRenderManager();
            _instance->window = wnd;

            _instance->createInstance();
            _instance->setupDebugMessenger();
            _instance->createSurface();
            _instance->pickPhysicalDevice();
            _instance->createLogicalDevice();

            _instance->createVulkanMemoryAllocator();
            _instance->createSwapChain();
            _instance->createSwapChainImageViews();
            _instance->createRenderPass();
            _instance->createDescriptorSetLayout();
            _instance->createGraphicsPipeline();
            _instance->createCommandPool();
            _instance->createDepthResources();
            _instance->createFramebuffers();
            _instance->createTextureImage(_instance->textureImage, _instance->mipLevels);
            _instance->createTextureImageView(_instance->textureImageView,_instance->textureImage, _instance->mipLevels);
            _instance->createTextureSampler();
            _instance->loadModel();
            _instance->gObject = new Entity();
            _instance->gTransform = (Transform*)_instance->gObject->getComponent(typeid(Transform).hash_code());
            _instance->gTransform->onAwake();

            _instance->createVertexBuffer(
                _instance->vertices,
                _instance->vertexBuffer,
                _instance->vertexBufferMemory
            
            );
        
            _instance->createIndexBuffer(
                _instance->_indices,
                _instance->_indexBuffer,
                _instance->_indexBufferMemory
            );

            _instance->createUniformBuffers();
            _instance->createDescriptorPool();
            _instance->createDescriptorSets();
            _instance->createCommandBuffers();
            _instance->createSyncObjects();

        };

        static void terminate()
        {
            _instance->cleanupSwapChain();


            // �������� ����� � �������� �  GPU
            _instance->device.destroySampler(_instance->textureSampler, nullptr);
            _instance->device.destroyImageView(_instance->textureImageView, nullptr);
            _instance->device.destroyImage(_instance->textureImage, nullptr);
            _instance->device.freeMemory(_instance->textureImageMemory, nullptr);


            _instance->device.destroyDescriptorSetLayout(_instance->descriptorSetLayout, nullptr);

            _instance->device.destroyBuffer(_instance->_indexBuffer, nullptr);
            _instance->device.freeMemory(_instance->_indexBufferMemory, nullptr);

            _instance->device.destroyBuffer(_instance->vertexBuffer, nullptr);
            _instance->device.freeMemory(_instance->vertexBufferMemory, nullptr);

            for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
                _instance->device.destroySemaphore(_instance->renderFinishedSemaphores[i], nullptr);
                _instance->device.destroySemaphore(_instance->imageAvailableSemaphores[i], nullptr);
                _instance->device.destroyFence(_instance->inFlightFences[i], nullptr);
            }

            _instance->device.destroyCommandPool(_instance->commandPool, nullptr);

            _instance->device.destroy();

            if (enableValidationLayers) {
                _instance->instance.destroyDebugUtilsMessengerEXT(_instance->debugMessenger, nullptr);
            }

            
            _instance->instance.destroySurfaceKHR(_instance->surface, nullptr);          
            _instance->instance.destroy();

        }
    };

}
#endif // !VULKAN_RENDERER_MANAGER


