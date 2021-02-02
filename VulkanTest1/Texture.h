#ifndef GTEXTURE
#define GTEXTURE


#include <GLFW/glfw3.h>
#include <vulkan/vulkan.hpp>
namespace te
{
	struct Texture {

	public:
		vk::Device memoryHolderDevice;

		vk::ImageView textureView;
		vk::Image texture;
		vk::DeviceMemory textureDeviceMemory;

		Texture(char* path)
		{

		}

		~Texture()
		{
			
			vkDestroyImageView(memoryHolderDevice, textureView, nullptr);
			vkDestroyImage(memoryHolderDevice, texture, nullptr);
			vkFreeMemory(memoryHolderDevice, textureDeviceMemory, nullptr);
		}
	};
}

#endif // !GTEXTURE

