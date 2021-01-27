#ifndef VULKAN_RENDERER_MANAGER
#define VULKAN_RENDERER_MANAGER

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

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
//#include "Renderer.h"


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

inline VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger) {
    auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
    if (func != nullptr) {
        return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
    }
    else {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
}

namespace std {
    template<> struct hash<te::Vertex> {
        size_t operator()(te::Vertex const& vertex) const {
            return ((hash<glm::vec3>()(vertex.pos) ^ (hash<glm::vec3>()(vertex.color) << 1)) >> 1) ^ (hash<glm::vec2>()(vertex.texCoord) << 1);
        }
    };
}
inline void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator) {
    auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
    if (func != nullptr) {
        func(instance, debugMessenger, pAllocator);
    }
}

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
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
};

namespace te
{

    

    class VulkanRenderManager : public te::EngineSystem<VulkanRenderManager>
    {
    private:

       
        te::Window* window;

        VkInstance instance;
        VkDebugUtilsMessengerEXT debugMessenger;
        VkSurfaceKHR surface;

        VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;


        VkQueue graphicsQueue;
        VkQueue presentQueue;

        VkSwapchainKHR swapChain;
        std::vector<VkImage> swapChainImages;
        VkFormat swapChainImageFormat;
        VkExtent2D swapChainExtent;
        std::vector<VkImageView> swapChainImageViews;
        std::vector<VkFramebuffer> swapChainFramebuffers;

        VkRenderPass renderPass;
        VkDescriptorSetLayout descriptorSetLayout;
        VkPipelineLayout pipelineLayout;
        VkPipeline graphicsPipeline;

        VkCommandPool commandPool;

        VkImage depthImage;
        VkDeviceMemory depthImageMemory;
        VkImageView depthImageView;

        uint32_t mipLevels;
        VkImage textureImage;
        VkDeviceMemory textureImageMemory;
        VkImageView textureImageView;
        VkSampler textureSampler;

        std::vector<te::Vertex> vertices;
        std::vector<uint32_t> _indices;
        VkBuffer vertexBuffer;
        VkDeviceMemory vertexBufferMemory;
        VkBuffer _indexBuffer;
        VkDeviceMemory _indexBufferMemory;

        std::vector<VkBuffer> uniformBuffers;
        std::vector<VkDeviceMemory> uniformBuffersMemory;

        VkDescriptorPool descriptorPool;
        std::vector<VkDescriptorSet> descriptorSets;

        std::vector<VkCommandBuffer> commandBuffers;

        std::vector<VkSemaphore> imageAvailableSemaphores;
        std::vector<VkSemaphore> renderFinishedSemaphores;
        std::vector<VkFence> inFlightFences;
        std::vector<VkFence> imagesInFlight;
        size_t currentFrame = 0;

        void createInstance();
        void setupDebugMessenger();
        void createSurface();
        void pickPhysicalDevice();
        void createLogicalDevice();
        void createSwapChain();
        void createImageViews();
        void createRenderPass();
        void createDescriptorSetLayout();
        void createGraphicsPipeline();
        void createCommandPool();
        void createDepthResources();
        void createFramebuffers();
        void createTextureImage();
        void createTextureImageView();
        void createTextureSampler();
        void loadModel();
        //void createVertexBuffer();
        //void createIndexBuffer();
       
        void createUniformBuffers();
        void createDescriptorPool();
        void createDescriptorSets();
        void createCommandBuffers();
        void createSyncObjects();
        void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);
        void updateUniformBuffer(uint32_t currentImage);
      
        void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);
        void generateMipmaps(VkImage image, VkFormat imageFormat, int32_t texWidth, int32_t texHeight, uint32_t mipLevels);
        void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t mipLevels);
        
    
       
        VkImageView createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, uint32_t mipLevels);
        void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);
        std::vector<const char*> getRequiredExtensions();
        bool isDeviceSuitable(VkPhysicalDevice device);
        QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);
        SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device);
        VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
        bool checkDeviceExtensionSupport(VkPhysicalDevice device);
        VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
        VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);
        static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
            VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
            VkDebugUtilsMessageTypeFlagsEXT messageType,
            const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
            void* pUserData
        );
        VkFormat findDepthFormat();
        uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
        VkFormat findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);
        void createImage(uint32_t width, uint32_t height, uint32_t mipLevels, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory);
        VkShaderModule createShaderModule(const std::vector<char>& code);
        bool checkValidationLayerSupport();

        
    public:
        VkDevice device;
        void createVertexBuffer(std::vector<te::Vertex> indices, VkBuffer& vertexBuffer, VkDeviceMemory& vertexBufferMemory);
        void createIndexBuffer(std::vector<uint32_t>, VkBuffer&, VkDeviceMemory&);
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
            _instance->createSwapChain();
            _instance->createImageViews();
            _instance->createRenderPass();
            _instance->createDescriptorSetLayout();
            _instance->createGraphicsPipeline();
            _instance->createCommandPool();
            _instance->createDepthResources();
            _instance->createFramebuffers();
            _instance->createTextureImage();
            _instance->createTextureImageView();
            _instance->createTextureSampler();
            _instance->loadModel();

            _instance->createVertexBuffer(
                _instance->vertices,
                _instance->vertexBuffer,
                _instance->vertexBufferMemory);

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

            vkDestroySampler(_instance->device, _instance->textureSampler, nullptr);
            vkDestroyImageView(_instance->device, _instance->textureImageView, nullptr);

            vkDestroyImage(_instance->device, _instance->textureImage, nullptr);
            vkFreeMemory(_instance->device, _instance->textureImageMemory, nullptr);

            vkDestroyDescriptorSetLayout(_instance->device, _instance->descriptorSetLayout, nullptr);

            vkDestroyBuffer(_instance->device, _instance->_indexBuffer, nullptr);
            vkFreeMemory(_instance->device, _instance->_indexBufferMemory, nullptr);

            vkDestroyBuffer(_instance->device, _instance->vertexBuffer, nullptr);
            vkFreeMemory(_instance->device, _instance->vertexBufferMemory, nullptr);

            for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
                vkDestroySemaphore(_instance->device, _instance->renderFinishedSemaphores[i], nullptr);
                vkDestroySemaphore(_instance->device, _instance->imageAvailableSemaphores[i], nullptr);
                vkDestroyFence(_instance->device, _instance->inFlightFences[i], nullptr);
            }

            vkDestroyCommandPool(_instance->device, _instance->commandPool, nullptr);

            vkDestroyDevice(_instance->device, nullptr);

            if (enableValidationLayers) {
                DestroyDebugUtilsMessengerEXT(_instance->instance, _instance->debugMessenger, nullptr);
            }

            vkDestroySurfaceKHR(_instance->instance, _instance->surface, nullptr);
            vkDestroyInstance(_instance->instance, nullptr);

        }
    };

}
#endif // !VULKAN_RENDERER_MANAGER


