#ifndef G_VULKAN_HELPER
#define G_VULKAN_HELPER

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vector>
#include "Vertex.h"
namespace te
{
	namespace vk
	{
		class VulkanHelper
		{


		public:
			static VkCommandBuffer beginSingleTimeCommands(
				VkCommandPool commandPool,
				VkDevice device
			);

			static void endSingleTimeCommands(
				VkCommandBuffer commandBuffer,
				VkQueue graphicsQueue,
				VkCommandPool commandPool,
				VkDevice device
			);

			static void copyBuffer(
				VkBuffer srcBuffer,
				VkBuffer dstBuffer,
				VkDeviceSize size,
				VkQueue graphicsQueue,
				VkCommandPool commandPool,
				VkDevice device
			);

			//static void pickPhysicalDevice();
			static std::vector<VkPhysicalDevice> getPhysicalDevices(
				VkInstance instance
			);

			static void createVertexBuffer(
				std::vector<te::Vertex> vertices,
				VkBuffer& vertexBuffer,
				VkDeviceMemory& vertexBufferMemory,
				VkCommandPool commandPool,
				VkQueue graphicsQueue,
				VkPhysicalDevice phisycalDevice,
				VkDevice device
			);

			static void createIndexBuffer(
				std::vector<uint32_t> indices,
				VkBuffer& indexBuffer,
				VkDeviceMemory& indexBufferMemory,
				VkCommandPool commandPool,
				VkQueue graphicsQueue,
				VkPhysicalDevice phisycalDevice,
				VkDevice device
			);



			static void createBuffer(
				VkDeviceSize size,
				VkBufferUsageFlags usage,
				VkMemoryPropertyFlags properties,
				VkBuffer& buffer,
				VkDeviceMemory& bufferMemory,
				VkPhysicalDevice phisycalDevice,
				VkDevice device
			);



			static uint32_t findMemoryType(
				uint32_t typeFilter,
				VkMemoryPropertyFlags properties,
				VkPhysicalDevice physicalDevice
			);
		};
	}
	
		
}

#endif // !G_VULKAN_HELPER
