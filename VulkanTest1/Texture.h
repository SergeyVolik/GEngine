#ifndef GTEXTURE
#define GTEXTURE

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

namespace te
{
	struct Texture {

	public:
		VkDevice memoryHolderDevice;

		VkSampler textureSampler;
		VkImageView textureView;
		VkImage texture;
		VkDeviceMemory textureDeviceMemory;

		Texture(char* path)
		{

		}

		~Texture()
		{
			vkDestroySampler(memoryHolderDevice, textureSampler, nullptr);
			vkDestroyImageView(memoryHolderDevice, textureView, nullptr);
			vkDestroyImage(memoryHolderDevice, texture, nullptr);
			vkFreeMemory(memoryHolderDevice, textureDeviceMemory, nullptr);
		}
	};
}

#endif // !GTEXTURE

