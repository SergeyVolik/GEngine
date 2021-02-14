#ifndef VULKAN_RENDERER_MANAGER
#define VULKAN_RENDERER_MANAGER
//#include <GLFW/glfw3.h

#define VULKAN_HPP_DISPATCH_LOADER_DYNAMIC 1
//#define VULKAN_HPP_DEFAULT_DISPATCH_LOADER_DYNAMIC_STORAGE
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan.h>

VULKAN_HPP_DEFAULT_DISPATCH_LOADER_DYNAMIC_STORAGE

//#define VULKAN_HPP_DEFAULT_DISPATCH_LOADER_DYNAMIC_STORAGE
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
#include "VulkanHelper.h"
#include "SwapChain.h"

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


namespace te
{

    class VulkanRenderManager : public te::EngineSystem<VulkanRenderManager>
    {
    private:


        //окно на котором будет происходить рендеринг
        te::Window* window;

        vk::SurfaceKHR surface;

        vkGame::VulkanSwapChain* mySwapChain;
        te::vkh::VulkanDevice* vulkanDevice;

        vk::PhysicalDeviceProperties deviceProperties;
        // Stores the features available on the selected physical device (for e.g. checking if a feature is available)
        vk::PhysicalDeviceFeatures deviceFeatures;
        // Stores all available memory (type) properties for the physical device
        vk::PhysicalDeviceMemoryProperties deviceMemoryProperties;


        //дескриптор дебаг мессенджера библиотеки вулкан
        vk::DebugUtilsMessengerEXT debugMessenger;   
        

        //-------------------------------------------- синхронизация двойной буферизации------------------------------

        //семафора для синхронизации (ожидания пока изображение не доступно)

        struct {
            std::vector<vk::Semaphore> imageAvailableSemaphores;
            //семафора для синхронизации (ожидания пока операции рендеринга не окончены)
            std::vector<vk::Semaphore> renderFinishedSemaphores;


            std::vector<vk::Fence> inFlightFences;
            std::vector<vk::Fence> imagesInFlight;

        } synch;

       

        //-----------------------------------------------------графика----------------------------------------------
       
       
        //vk::RenderPass renderPass;
        vk::DescriptorSetLayout descriptorSetLayout;
        vk::PipelineLayout pipelineLayout;
        vk::Pipeline graphicsPipeline;
        vk::PipelineCache pipelineCache;
        vk::CommandPool commandPool;
        //vk::CommandPool transferCommandPool;
        //очередь команд для вычисления елементов графики

        struct
        {
            vk::Queue graphicsQueue;
            vk::Queue presentQueue;
            vk::Queue transferQueue;

        } vulkanQueues;

      

        //презентационная очередь для отображения на поверхность  SurfaceKHR
     
       /* vk::Image depthImage;
        vk::DeviceMemory depthImageMemory;
        vk::ImageView depthImageView;*/

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

        size_t currentFrame = 0;     


        //std::list<Renderer*> renderers;
        Entity* gObject;
        Transform* gTransform;

        //инициализация библиотеки вулка
        void createInstance();
        //установка дебагера для вулкана
        void setupDebugMessenger();
        //создание полотна на котором будет производиться рендеринга
        void createSurface();

        //выбор физитеского устройства GPU
        void pickPhysicalDevice();

        //создание логического устройсива вулка для взаимодействия с драйвером устройства
        void createLogicalDevice();

        //создание привязки данных к шейдерам (vertex, fragment)
        void createDescriptorSetLayout();

        //определение всех этапов отрисовки изображения
        void createGraphicsPipeline();


        void createCommandPool();
        void createDepthResources();
        void createFramebuffers();
        void createTextureImage(vk::Image&, uint32_t&);
        void createTextureImageView(vk::ImageView& imgView, const vk::Image textureImage, const uint32_t mipLevels);
        void createTextureSampler();

        ///загрузку модели нужно вынести в загрузку ресурсов
        void loadModel();
       

        //
        void createVulkanMemoryAllocator();

        void createUniformBuffers();

        //создание пула дескрипторов для пайплана
        void createDescriptorPool();
        void createDescriptorSets();
        void createCommandBuffers();
        void createSyncObjects();
        void createPipelineCache();
        void updateUniformBuffer(uint32_t currentImage);
      

        void generateMipmaps(vk::Image image, vk::Format imageFormat, int32_t texWidth, int32_t texHeight, uint32_t mipLevels);
        void transitionImageLayout(vk::Image image, vk::Format format, vk::ImageLayout oldLayout, vk::ImageLayout newLayout, uint32_t mipLevels);
        
    
       
       
        void populateDebugMessengerCreateInfo(vk::DebugUtilsMessengerCreateInfoEXT& createInfo);
        std::vector<const char*> getRequiredExtensions();
        bool isDeviceSuitable(vk::PhysicalDevice device);
       

        bool checkDeviceExtensionSupport(vk::PhysicalDevice device);

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
     
        bool checkValidationLayerSupport();

        //перестройка ципочки обновлений (двойная-тройная буферизация) после изменения размеров окна
        void recreateSwapChain();

    public:


        //выделение вертесного буфера и получение дискриптора паняти
        void createVertexBuffer(std::vector<te::Vertex> indices, vk::Buffer& vertexBuffer, vk::DeviceMemory& vertexBufferMemory);

        //выделение индесного буфера и получение дискриптора паняти
        void createIndexBuffer(std::vector<uint32_t>, vk::Buffer&, vk::DeviceMemory&);
        
        void deviceWaitIdle() { vulkanDevice->logicalDevice.waitIdle(); }

      
        void cleanupSwapChain();
        void drawFrame();
        void createSwapchain();
        static void initialize(te::Window* wnd);

        static void terminate();
        
           
    };

}
#endif // !VULKAN_RENDERER_MANAGER


