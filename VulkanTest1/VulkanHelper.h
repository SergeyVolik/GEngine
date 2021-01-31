#ifndef G_VULKAN_HELPER
#define G_VULKAN_HELPER

#include <GLFW/glfw3.h>
#include <vulkan/vulkan.hpp>
#include <vector>
#include "Vertex.h"
namespace te
{
	namespace vkh
	{
		class VulkanHelper
		{


		public:
			static vk::CommandBuffer beginSingleTimeCommands(
				vk::CommandPool commandPool,
				vk::Device device
			);

			static void endSingleTimeCommands(
				vk::CommandBuffer commandBuffer,
				vk::Queue graphicsQueue,
				vk::CommandPool commandPool,
				vk::Device device
			);

			static void copyBuffer(
				vk::Buffer srcBuffer,
				vk::Buffer dstBuffer,
				vk::DeviceSize size,
				vk::Queue graphicsQueue,
				vk::CommandPool commandPool,
				vk::Device device
			);

			//static void pickPhysicalDevice();
			static std::vector<vk::PhysicalDevice> getPhysicalDevices(
				vk::Instance instance
			);

			static void createVertexBuffer(
				std::vector<te::Vertex> vertices,
				vk::Buffer& vertexBuffer,
				vk::DeviceMemory& vertexBufferMemory,
				vk::CommandPool commandPool,
				vk::Queue graphicsQueue,
				vk::PhysicalDevice phisycalDevice,
				vk::Device device
			);

			static void createIndexBuffer(
				std::vector<uint32_t> indices,
				vk::Buffer& indexBuffer,
				vk::DeviceMemory& indexBufferMemory,
				vk::CommandPool commandPool,
				vk::Queue graphicsQueue,
				vk::PhysicalDevice phisycalDevice,
				vk::Device device
			);



			static void createBuffer(
				vk::DeviceSize size,
				vk::BufferUsageFlags usage,
				vk::MemoryPropertyFlags properties,
				vk::Buffer& buffer,
				vk::DeviceMemory& bufferMemory,
				vk::PhysicalDevice phisycalDevice,
				vk::Device device
			);



			static uint32_t findMemoryType(
				uint32_t typeFilter,
				vk::MemoryPropertyFlags properties,
				vk::PhysicalDevice physicalDevice
			);

			static void copyBufferToImage(
				vk::Buffer buffer,
				vk::Image image,
				uint32_t width,
				uint32_t height,
				vk::CommandPool commandPool,
				vk::Queue graphicsQueue,
				vk::Device device
			);
		};
	}
	
		
}

#endif // !G_VULKAN_HELPER
