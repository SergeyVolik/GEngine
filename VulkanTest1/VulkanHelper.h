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
			

		
		public:
			vk::Device logicalDevice;
			vk::PhysicalDevice physicalDevice;
			vk::Instance instance;
			vk::CommandPool commandPool;
			std::vector<vk::CommandBuffer> commandBuffers;

			struct {
				vk::PhysicalDeviceProperties properties;
				/** @brief Features of the physical device that an application can use to check if a feature is supported */
				vk::PhysicalDeviceFeatures features;
				/** @brief Features that have been enabled for use on the physical device */
				vk::PhysicalDeviceFeatures enabledFeatures;
				/** @brief Memory types and heaps of the physical device */
				vk::PhysicalDeviceMemoryProperties memoryProperties;

				/** @brief List of extensions supported by the device */
				std::vector<std::string> supportedExtensions;
			} info;

			VulkanDevice(vk::Device logicalDevice,
				vk::PhysicalDevice physicalDevice,
				vk::Instance instance) : logicalDevice(logicalDevice), physicalDevice(physicalDevice), instance(instance)
			{
				setupPhysicalDevice(physicalDevice);
			
				
			}
			VulkanDevice() : logicalDevice(), physicalDevice(), instance() {}

			void setupPhysicalDevice(vk::PhysicalDevice physicalDevice)
			{
				this->physicalDevice = physicalDevice;
				// Store Properties features, limits and properties of the physical device for later use
				// Device properties also contain limits and sparse properties
				physicalDevice.getProperties(&info.properties);
				// Features should be checked by the examples before using them
				physicalDevice.getFeatures(&info.features);
				// Memory properties are used regularly for creating all kinds of buffers

				physicalDevice.getMemoryProperties(&info.memoryProperties);

				uint32_t extCount = 0;

				auto extentions = physicalDevice.enumerateDeviceExtensionProperties();

				for (const auto& ext : extentions)
					info.supportedExtensions.push_back(ext.extensionName);
			}
			uint32_t getMemoryType(uint32_t typeBits, vk::MemoryPropertyFlags properties, vk::Bool32* memTypeFound) const
			{
				for (uint32_t i = 0; i < info.memoryProperties.memoryTypeCount; i++)
				{
					if ((typeBits & 1) == 1)
					{
						if ((info.memoryProperties.memoryTypes[i].propertyFlags & properties) == properties)
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

			void createImage(uint32_t width, uint32_t height, uint32_t mipLevels, vk::Format format, vk::ImageTiling tiling, vk::ImageUsageFlags usage, vk::MemoryPropertyFlags properties, vk::Image& image, vk::DeviceMemory& imageMemory, uint32_t arrayLayers = 1, vk::ImageCreateFlags imageCreateFlags = {})
			{
				vk::ImageCreateInfo imageInfo{};

				imageInfo.imageType = vk::ImageType::e2D;
				imageInfo.extent.width = width;
				imageInfo.extent.height = height;
				imageInfo.extent.depth = 1;
				imageInfo.mipLevels = mipLevels;
				imageInfo.arrayLayers = arrayLayers;
				imageInfo.format = format;
				imageInfo.tiling = tiling;
				imageInfo.initialLayout = vk::ImageLayout::eUndefined;
				imageInfo.usage = usage;

				imageInfo.flags = imageCreateFlags;

				imageInfo.samples = vk::SampleCountFlagBits::e1;
				imageInfo.sharingMode = vk::SharingMode::eExclusive;

				if (logicalDevice.createImage(&imageInfo, nullptr, &image) != vk::Result::eSuccess) {
					throw std::runtime_error("failed to create image!");
				}

				vk::MemoryRequirements memRequirements = logicalDevice.getImageMemoryRequirements(image);


				vk::MemoryAllocateInfo allocInfo{};

				allocInfo.allocationSize = memRequirements.size;
				vk::Bool32 result;
				allocInfo.memoryTypeIndex = getMemoryType(memRequirements.memoryTypeBits, properties, &result);


				if (logicalDevice.allocateMemory(&allocInfo, nullptr, &imageMemory) != vk::Result::eSuccess) {
					throw std::runtime_error("failed to allocate image memory!");
				}
				logicalDevice.bindImageMemory(image, imageMemory, 0);

			}
			vk::ShaderModule createShaderModule(const std::vector<char>& code);

			vk::CommandBuffer beginSingleTimeCommands(
				vk::CommandPool commandPool
				
			);

			void endSingleTimeCommands(
				vk::CommandBuffer commandBuffer,
				vk::Queue graphicsQueue,
				vk::CommandPool commandPool
				
			);

			void copyBuffer(
				vk::Buffer srcBuffer,
				vk::Buffer dstBuffer,
				vk::DeviceSize size,
				vk::Queue graphicsQueue,
				vk::CommandPool commandPool
				
			);

			vk::Bool32 formatIsFilterable(vk::Format format, vk::ImageTiling tiling);

			void insertImageMemoryBarrier(
				vk::CommandBuffer cmdbuffer,
				vk::Image image,
				vk::AccessFlags srcAccessMask,
				vk::AccessFlags dstAccessMask,
				vk::ImageLayout oldImageLayout,
				vk::ImageLayout newImageLayout,
				vk::PipelineStageFlags srcStageMask,
				vk::PipelineStageFlags dstStageMask,
				vk::ImageSubresourceRange subresourceRange
			);

			void setImageLayout(vk::CommandBuffer cmdbuffer,
				vk::Image image, vk::ImageLayout oldImageLayout,
				vk::ImageLayout newImageLayout, vk::ImageSubresourceRange subresourceRange,
				vk::PipelineStageFlags srcStageMask = vk::PipelineStageFlagBits::eAllCommands,
				vk::PipelineStageFlags dstStageMask = vk::PipelineStageFlagBits::eAllCommands);

			void setImageLayout(vk::CommandBuffer cmdbuffer,
				vk::Image image, uint32_t mipLevels, vk::ImageAspectFlags aspectMask, vk::ImageLayout oldImageLayout,
				vk::ImageLayout newImageLayout, uint32_t layerCount = 1, vk::PipelineStageFlags srcStageMask = vk::PipelineStageFlagBits::eAllCommands,
				vk::PipelineStageFlags dstStageMask = vk::PipelineStageFlagBits::eAllCommands);

			void generateMipmaps(vk::Image image, vk::Format imageFormat, int32_t texWidth, int32_t texHeight, uint32_t mipLevels, vk::Queue graphicQueue);

			void generateMipmapsForCubmap(vk::Image image, vk::Format imageFormat, int32_t texWidth, int32_t texHeight, uint32_t mipLevels, vk::Queue graphicsQueue);


			void createVertexBuffer(
				std::vector<te::Vertex> vertices,
				vk::Buffer& vertexBuffer,
				vk::DeviceMemory& vertexBufferMemory,
				vk::CommandPool commandPool,
				vk::Queue graphicsQueue

			);

			void createIndexBuffer(
				std::vector<uint32_t> indices,
				vk::Buffer& indexBuffer,
				vk::DeviceMemory& indexBufferMemory,
				vk::CommandPool commandPool,
				vk::Queue graphicsQueue

			);



			void createBuffer(
				vk::DeviceSize size,
				vk::BufferUsageFlags usage,
				vk::MemoryPropertyFlags properties,
				vk::Buffer& buffer,
				vk::DeviceMemory& bufferMemory
				
			);



			uint32_t findMemoryType(
				uint32_t typeFilter,
				vk::MemoryPropertyFlags properties
			);

			void copyBufferToImage(
				vk::Buffer buffer,
				vk::Image image,
				uint32_t width,
				uint32_t height,
				vk::CommandPool commandPool,
				vk::Queue graphicsQueue
			);

			vk::ImageView createImageView(
				vk::Image image,
				vk::Format format,
				vk::ImageAspectFlags aspectFlags,
				uint32_t mipLevels
			);

			
		};

		
		

		struct QueueFamilyIndices {
			std::optional<uint32_t> graphicsFamily;
			std::optional<uint32_t> presentFamily;
			std::optional<uint32_t> transferFamily;
			bool isComplete() {
				return graphicsFamily.has_value() && presentFamily.has_value() && transferFamily.has_value();
			}
		};

		te::vkh::QueueFamilyIndices findQueueFamilies(
			vk::SurfaceKHR surface, vk::PhysicalDevice physicalDevice
		);

		te::vkh::SwapChainSupportDetails querySwapChainSupport(
			vk::SurfaceKHR surface, vk::PhysicalDevice physicalDevice
		);

	}
	
		
}

#endif // !G_VULKAN_HELPER
