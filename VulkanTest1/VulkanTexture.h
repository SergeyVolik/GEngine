//#ifndef GTEXTURE
//#define GTEXTURE
//
//#include <ktx.h>
//#include <ktxvulkan.h>
//
//#include <GLFW/glfw3.h>
//#include <vulkan/vulkan.hpp>
//#include "VulkanDevice.h"
//#include "FileReader.h"
//#include "GameEngine.h"
//
//namespace vkh
//{
//	class Texture
//	{
//	public:
//		te::vkh::VulkanDevice* device;
//		vk::Image               image;
//		vk::ImageLayout         imageLayout;
//		vk::DeviceMemory        deviceMemory;
//		vk::ImageView           view;
//		uint32_t              width, height;
//		uint32_t              mipLevels;
//		uint32_t              layerCount;
//		vk::DescriptorImageInfo descriptor;
//		vk::Sampler             sampler;
//
//		void      updateDescriptor();
//		void      destroy();
//		ktxResult loadKTXFile(std::string filename, ktxTexture** target);
//	};
//
//	class Texture2D : public Texture
//	{
//	public:
//		void loadFromFile(
//			std::string        filename,
//			vk::Format           format,
//			te::vkh::VulkanDevice* device,
//			vk::Queue            copyQueue,
//			vk::ImageUsageFlags  imageUsageFlags = vk::ImageUsageFlagBits::eSampled,
//			vk::ImageLayout      imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal,
//			bool               forceLinear = false);
//		void fromBuffer(
//			void* buffer,
//			vk::DeviceSize       bufferSize,
//			vk::Format           format,
//			uint32_t           texWidth,
//			uint32_t           texHeight,
//			te::vkh::VulkanDevice* device,
//			vk::Queue            copyQueue,
//			vk::Filter           filter = vk::Filter::eLinear,
//			vk::ImageUsageFlags  imageUsageFlags = vk::ImageUsageFlagBits::eSampled,
//			vk::ImageLayout      imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal);
//	};
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
//#endif // !GTEXTURE
//
