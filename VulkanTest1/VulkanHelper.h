#ifndef G_VULKAN_HELPER
#define G_VULKAN_HELPER

#include <vulkan/vulkan.hpp>
#include <vector>
#include "Vertex.h"
#include <optional>
namespace te
{
	namespace vkh
	{
		struct SwapChainSupportDetails {


			vk::SurfaceCapabilitiesKHR capabilities;
			std::vector<vk::SurfaceFormatKHR> formats;
			std::vector<vk::PresentModeKHR> presentModes;
		};

		class VulkanDevice
		{
			vk::PhysicalDeviceProperties properties;
			/** @brief Features of the physical device that an application can use to check if a feature is supported */
			vk::PhysicalDeviceFeatures features;
			/** @brief Features that have been enabled for use on the physical device */
			vk::PhysicalDeviceFeatures enabledFeatures;
			/** @brief Memory types and heaps of the physical device */
			vk::PhysicalDeviceMemoryProperties memoryProperties;

			/** @brief List of extensions supported by the device */
			std::vector<std::string> supportedExtensions;
		public:
			vk::Device logicalDevice;
			vk::PhysicalDevice physicalDevice;
			vk::Instance instance;

			VulkanDevice(vk::Device logicalDevice,
				vk::PhysicalDevice physicalDevice,
				vk::Instance instance) : logicalDevice(logicalDevice), physicalDevice(physicalDevice), instance(instance)
			{
				// Store Properties features, limits and properties of the physical device for later use
		        // Device properties also contain limits and sparse properties
				physicalDevice.getProperties(&properties);
				// Features should be checked by the examples before using them
				physicalDevice.getFeatures(&features);
				// Memory properties are used regularly for creating all kinds of buffers
			
				physicalDevice.getMemoryProperties(&memoryProperties);

				uint32_t extCount = 0;

				 auto extentions = physicalDevice.enumerateDeviceExtensionProperties();

				 for (auto const ext : extentions)
					 supportedExtensions.push_back(ext.extensionName);
				
			}
			VulkanDevice() : logicalDevice(), physicalDevice(), instance() {}


			uint32_t getMemoryType(uint32_t typeBits, vk::MemoryPropertyFlags properties, vk::Bool32* memTypeFound) const
			{
				for (uint32_t i = 0; i < memoryProperties.memoryTypeCount; i++)
				{
					if ((typeBits & 1) == 1)
					{
						if ((memoryProperties.memoryTypes[i].propertyFlags & properties) == properties)
						{
							if (memTypeFound)
							{
								*memTypeFound = true;
							}
							return i;
						}
					}
					typeBits >>= 1;
				}

				if (memTypeFound)
				{
					*memTypeFound = false;
					return 0;
				}
				else
				{
					throw std::runtime_error("Could not find a matching memory type");
				}
			}
		};

		struct QueueFamilyIndices {
			std::optional<uint32_t> graphicsFamily;
			std::optional<uint32_t> presentFamily;
			std::optional<uint32_t> transferFamily;
			bool isComplete() {
				return graphicsFamily.has_value() && presentFamily.has_value() && transferFamily.has_value();
			}
		};

		class VulkanHelper
		{


		public:

			static vk::ShaderModule createShaderModule(vk::Device device, const std::vector<char>& code);

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

			static vk::Bool32 formatIsFilterable(vk::PhysicalDevice physicalDevice, vk::Format format, vk::ImageTiling tiling);

			static void insertImageMemoryBarrier(vk::CommandBuffer cmdbuffer, vk::Image image, vk::AccessFlags srcAccessMask, vk::AccessFlags dstAccessMask, vk::ImageLayout oldImageLayout, vk::ImageLayout newImageLayout, vk::PipelineStageFlags srcStageMask, vk::PipelineStageFlags dstStageMask, vk::ImageSubresourceRange subresourceRange);

			static void setImageLayout(vk::CommandBuffer cmdbuffer,
				vk::Image image, vk::ImageLayout oldImageLayout,
				vk::ImageLayout newImageLayout, vk::ImageSubresourceRange subresourceRange,
				vk::PipelineStageFlags srcStageMask = vk::PipelineStageFlagBits::eAllCommands,
				vk::PipelineStageFlags dstStageMask = vk::PipelineStageFlagBits::eAllCommands);

			static void setImageLayout(vk::CommandBuffer cmdbuffer,
				vk::Image image, vk::ImageAspectFlags aspectMask, vk::ImageLayout oldImageLayout,
				vk::ImageLayout newImageLayout, vk::PipelineStageFlags srcStageMask = vk::PipelineStageFlagBits::eAllCommands,
				vk::PipelineStageFlags dstStageMask = vk::PipelineStageFlagBits::eAllCommands);


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

			static vk::ImageView createImageView(
				vk::Image image,
				vk::Format format,
				vk::ImageAspectFlags aspectFlags,
				uint32_t mipLevels,
				vk::Device device
			);

			static QueueFamilyIndices findQueueFamilies(
				vk::PhysicalDevice device,
				vk::SurfaceKHR surface
			);

			static te::vkh::SwapChainSupportDetails querySwapChainSupport(
				vk::PhysicalDevice physicalDevice,
				vk::SurfaceKHR surface
			);
			

			
		};
	}
	
		
}

#endif // !G_VULKAN_HELPER
