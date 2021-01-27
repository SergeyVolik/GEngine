#ifndef G_VULKAN_HELPER
#define G_VULKAN_HELPER

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vector>
namespace te
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
		static std::vector<VkPhysicalDevice> getPhysicalDevices(VkInstance instance);
	};

	
		
}

#endif // !G_VULKAN_HELPER
