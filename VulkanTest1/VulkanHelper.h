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

			VulkanDevice(vk::Device logicalDevice,
				vk::PhysicalDevice physicalDevice,
				vk::Instance instance) : logicalDevice(logicalDevice), physicalDevice(physicalDevice), instance(instance)
			{

			}
			VulkanDevice() : logicalDevice(), physicalDevice(), instance() {}
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
