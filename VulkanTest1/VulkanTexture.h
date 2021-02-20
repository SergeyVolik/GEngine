#ifndef GTEXTURE
#define GTEXTURE

//#include <ktx.h>
//#include <ktxvulkan.h>
//
//#include <GLFW/glfw3.h>
//#include <vulkan/vulkan.hpp>
//#include "VulkanDevice.h"
//#include "FileReader.h"
//#include "GameEngine.h"
//
#include "VulkanHelper.h"

namespace vkh
{

	class Texture
	{
	protected:
		vk::Format format;
		te::vkh::VulkanDevice* device;
		vk::Queue copyQueue;
		vk::ImageUsageFlags imageUsageFlags;
		vk::ImageLayout imageLayout;
		bool forceLinear;

	public:
		
		vk::Image               textureImage;
		vk::ImageLayout         textureImageLoyout;
		vk::Sampler             textureSampler;
		vk::DeviceMemory        deviceMemory;
		vk::ImageView           textureImageView;
		uint32_t              width, height;
		uint32_t              mipLevels;
		uint32_t              layerCount;
		vk::DescriptorImageInfo descriptor;
		

		void  updateDescriptor();

		Texture(te::vkh::VulkanDevice* device);

		
		~Texture();

		
	protected:

		virtual void createTextureImage() = 0;
		virtual void createTextureImageView() = 0;
		virtual void createTextureSampler() = 0;

		void createTexture() {
			createTextureImage();
			createTextureImageView();
			createTextureSampler();
		};

	};

	class Texture2D : public Texture
	{
	private:
		std::string filePath;
		
	protected:
		void createTextureImage() override;
		void createTextureImageView() override;
		void createTextureSampler() override;
	public:
		Texture2D(te::vkh::VulkanDevice* device) : Texture(device) {};
		void loadFromFile(
			std::string        filename,
			vk::Format           format,		
			vk::Queue            copyQueue,
			vk::ImageUsageFlags  imageUsageFlags = vk::ImageUsageFlagBits::eSampled,
			vk::ImageLayout      imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal,
			bool               forceLinear = false);
		
	};

		/*class TextureCubeMap : public Texture
	{
	public:
		void loadFromFile(
			std::string        filename,
			vk::Format           format,
			te::vkh::VulkanDevice* device,
			vk::Queue            copyQueue,
			vk::ImageUsageFlags  imageUsageFlags = vk::ImageUsageFlagBits::eSampled,
			vk::ImageLayout      imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal);
	};*/
}


	
//
//	class Texture2DArray : public Texture
//	{
//	public:
//		void loadFromFile(
//			std::string        filename,
//			vk::Format           format,
//			te::vkh::VulkanDevice* device,
//			vk::Queue            copyQueue,
//			vk::ImageUsageFlags  imageUsageFlags = vk::ImageUsageFlagBits::eSampled,
//			vk::ImageLayout      imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal);
//	};
//
//	class TextureCubeMap : public Texture
//	{
//	public:
//		void loadFromFile(
//			std::string        filename,
//			vk::Format           format,
//			te::vkh::VulkanDevice* device,
//			vk::Queue            copyQueue,
//			vk::ImageUsageFlags  imageUsageFlags = vk::ImageUsageFlagBits::eSampled,
//			vk::ImageLayout      imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal);
//	};
//}
//
#endif // !GTEXTURE
//
