#ifndef RENDERING_VULKAN_FRAMEBUFFER
#define RENDERING_VULKAN_FRAMEBUFFER

#include "vulkan/vulkan.hpp"
#include <algorithm>
#include "VulkanHelper.h"
#include <assert.h>
namespace vkGame
{
	struct FramebufferAttachment
	{
		vk::Image image;
		vk::DeviceMemory memory;
		vk::ImageView view;
		vk::Format format;
		vk::ImageSubresourceRange subresourceRange;
		vk::AttachmentDescription description;

		/**
		* @brief Returns true if the attachment has a depth component
		*/
		bool hasDepth()
		{
			std::vector<vk::Format> formats =
			{				
				vk::Format::eD16Unorm,
				vk::Format::eX8D24UnormPack32,
				vk::Format::eD32Sfloat,
				vk::Format::eD16Unorm,
				vk::Format::eD24UnormS8Uint,
				vk::Format::eD32SfloatS8Uint
			};
			return std::find(formats.begin(), formats.end(), format) != std::end(formats);
		}

		/**
		* @brief Returns true if the attachment has a stencil component
		*/
		bool hasStencil()
		{
			std::vector<vk::Format> formats =
			{				
				vk::Format::eS8Uint,
				vk::Format::eD16UnormS8Uint,
				vk::Format::eD24UnormS8Uint,
				vk::Format::eD32SfloatS8Uint
			};

			return std::find(formats.begin(), formats.end(), format) != std::end(formats);
		}

		/**
		* @brief Returns true if the attachment is a depth and/or stencil attachment
		*/
		bool isDepthStencil()
		{
			return(hasDepth() || hasStencil());
		}

	};

	/**
	* @brief Describes the attributes of an attachment to be created
	*/
	struct AttachmentCreateInfo
	{
		uint32_t width, height;
		uint32_t layerCount;
		vk::Format format;
		vk::ImageUsageFlags usage;
		vk::SampleCountFlagBits imageSampleCount = vk::SampleCountFlagBits::e1;
	};

	class VulkanFrameBuffer
	{
	private:
		te::vkh::VulkanDevice* vulkanDevice;
	public:

		/**
		* Default constructor
		*
		* @param vulkanDevice Pointer to a valid VulkanDevice
		*/
		VulkanFrameBuffer(te::vkh::VulkanDevice* vulkanDevice)
		{
			assert(vulkanDevice);
			this->vulkanDevice = vulkanDevice;
		}

	};
}


#endif // !RENDERING_VULKAN_FRAMEBUFFER

