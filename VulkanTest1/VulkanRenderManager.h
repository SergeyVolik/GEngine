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
#include "VulkanTexture.h"

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


        //���� �� ������� ����� ����������� ���������
        te::Window* window;

        vk::SurfaceKHR surface;

        vkGame::VulkanSwapChain* mySwapChain;
        te::vkh::VulkanDevice* vulkanDevice;
        ::vkh::Texture2D* texture;

        //���������� ����� ����������� ���������� ������
        vk::DebugUtilsMessengerEXT debugMessenger;   
        

        //-------------------------------------------- ������������� ������� �����������------------------------------

        //�������� ��� ������������� (�������� ���� ����������� �� ��������)

        struct {
            std::vector<vk::Semaphore> imageAvailableSemaphores;
            //�������� ��� ������������� (�������� ���� �������� ���������� �� ��������)
            std::vector<vk::Semaphore> renderFinishedSemaphores;


            std::vector<vk::Fence> inFlightFences;
            std::vector<vk::Fence> imagesInFlight;

        } synch;

       

        //-----------------------------------------------------�������----------------------------------------------
       
       
        //vk::RenderPass renderPass;
        vk::DescriptorSetLayout descriptorSetLayout;
        vk::PipelineLayout pipelineLayout;
        vk::Pipeline graphicsPipeline;
        vk::PipelineCache pipelineCache;     
        //vk::CommandPool transferCommandPool;
        //������� ������ ��� ���������� ��������� �������

        struct
        {
            vk::Queue graphicsQueue;
            vk::Queue presentQueue;
            vk::Queue transferQueue;

        } vulkanQueues;

      

        //��������������� ������� ��� ����������� �� �����������  SurfaceKHR
     
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


        size_t currentFrame = 0;     


        //std::list<Renderer*> renderers;
        Entity* gObject;
        Transform* gTransform;

        //������������� ���������� �����
        void createInstance();
        //��������� �������� ��� �������
        void setupDebugMessenger();
        //�������� ������� �� ������� ����� ������������� ����������
        void createSurface();

        //����� ����������� ���������� GPU
        void pickPhysicalDevice();

        //�������� ����������� ���������� ����� ��� �������������� � ��������� ����������
        void createLogicalDevice();

        //�������� �������� ������ � �������� (vertex, fragment)
        void createDescriptorSetLayout();

        //����������� ���� ������ ��������� �����������
        void createGraphicsPipeline();


        void createCommandPool();
        void createDepthResources();
        void createFramebuffers();

        ///�������� ������ ����� ������� � �������� ��������
        void loadModel();
       

        //
        void createVulkanMemoryAllocator();

        void createUniformBuffers();

        //�������� ���� ������������ ��� ��������
        void createDescriptorPool();
        void createDescriptorSets();
        void createCommandBuffers();
        void createSyncObjects();
        void createPipelineCache();
        void updateUniformBuffer(uint32_t currentImage);
      
       
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
     
        vk::Format findSupportedFormat(const std::vector<vk::Format>& candidates, vk::ImageTiling tiling, vk::FormatFeatureFlags features);
        
     
        bool checkValidationLayerSupport();

        //����������� ������� ���������� (�������-������� �����������) ����� ��������� �������� ����
        void recreateSwapChain();

    public:


        //��������� ���������� ������ � ��������� ����������� ������
        void createVertexBuffer(std::vector<te::Vertex> indices, vk::Buffer& vertexBuffer, vk::DeviceMemory& vertexBufferMemory);

        //��������� ��������� ������ � ��������� ����������� ������
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


