#ifndef RENDERING_VULKAN_FRAMEBUFFER
#define RENDERING_VULKAN_FRAMEBUFFER

#include "vulkan/vulkan.hpp"
#include <algorithm>
#include "VulkanHelper.h"
#include <assert.h>
#include "VulkanValidate.h"
namespace vkGame
{
	struct FramebufferAttachment
	{
		bool uniqueAttachment;

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
		bool uniqueAttachment;
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

		uint32_t width, height;
		vk::Framebuffer framebuffer;
		vk::RenderPass renderPass;
		vk::Sampler sampler;

		std::vector<FramebufferAttachment> attachments;

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

		~VulkanFrameBuffer()
		{


			for (int i = 0; i < attachments.size(); i++)
			{   
				if (!attachments[i].uniqueAttachment)
				{
					
					vulkanDevice->logicalDevice.destroyImage(attachments[i].image);					
					vulkanDevice->logicalDevice.destroyImageView(attachments[i].view);
					vulkanDevice->logicalDevice.freeMemory(attachments[i].memory);
				}

			}
			

			vulkanDevice->logicalDevice.destroyFramebuffer(framebuffer, nullptr);
		}

		void createFramebuffer(vk::ImageView swapChainImageView)
		{
			vk::FramebufferCreateInfo framebufferInfo{};

			framebufferInfo.renderPass = renderPass;
			framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size() + 1);

			std::vector<vk::ImageView> attachmentsView;


			attachmentsView.push_back(swapChainImageView);

			for (int i = 0; i < attachments.size(); i++)
			{
				attachmentsView.push_back(attachments[i].view);
			}

			

			framebufferInfo.pAttachments = attachmentsView.data();
			framebufferInfo.width = width;
			framebufferInfo.height = height;
			framebufferInfo.layers = 1;

			if (vulkanDevice->logicalDevice.createFramebuffer(&framebufferInfo, nullptr, &framebuffer) != vk::Result::eSuccess) {
				throw std::runtime_error("failed to create framebuffer!");
			}
			
		}
		/**
		* Add a new attachment described by createinfo to the framebuffer's attachment list
		*
		* @param createinfo Structure that specifies the framebuffer to be constructed
		*
		* @return Index of the new attachment
		*/
		uint32_t addAttachment(AttachmentCreateInfo createinfo)
		{
			FramebufferAttachment attachment;

			attachment.format = createinfo.format;
			attachment.uniqueAttachment = createinfo.uniqueAttachment;
			vk::ImageAspectFlags aspectMask{};

			// Select aspect mask and layout depending on usage

			// Color attachment
			if (createinfo.usage & vk::ImageUsageFlagBits::eColorAttachment)
			{
				aspectMask = vk::ImageAspectFlagBits::eColor;
			}

			// Depth (and/or stencil) attachment
			if (createinfo.usage & vk::ImageUsageFlagBits::eDepthStencilAttachment)
			{
				if (attachment.hasDepth())
				{
					aspectMask = vk::ImageAspectFlagBits::eDepth;
				}
				if (attachment.hasStencil())
				{
					aspectMask = aspectMask | vk::ImageAspectFlagBits::eStencil;
				}
			}

			assert(aspectMask > static_cast<vk::ImageAspectFlagBits>(0));

			vk::ImageCreateInfo image{};
			image.imageType = vk::ImageType::e2D;
			image.format = createinfo.format;
			image.extent.width = createinfo.width;
			image.extent.height = createinfo.height;
			image.extent.depth = 1;
			image.mipLevels = 1;
			image.arrayLayers = createinfo.layerCount;
			image.samples = createinfo.imageSampleCount;
			image.tiling = vk::ImageTiling::eOptimal;
			image.usage = createinfo.usage;

			vk::MemoryAllocateInfo memAlloc{ };
			vk::MemoryRequirements memReqs;

			// Create image for this attachment
			VK_CHECK_RESULT(vulkanDevice->logicalDevice.createImage(&image, nullptr, &attachment.image));
		
			vulkanDevice->logicalDevice.getImageMemoryRequirements(attachment.image, &memReqs);

			memAlloc.allocationSize = memReqs.size;

			vk::Bool32 result = false;
			memAlloc.memoryTypeIndex = vulkanDevice->getMemoryType(memReqs.memoryTypeBits, vk::MemoryPropertyFlagBits::eDeviceLocal, &result);

			VK_CHECK_RESULT(vulkanDevice->logicalDevice.allocateMemory(&memAlloc, nullptr, &attachment.memory));
			
			vulkanDevice->logicalDevice.bindImageMemory(attachment.image, attachment.memory, 0);

			attachment.subresourceRange = vk::ImageSubresourceRange();
			attachment.subresourceRange.aspectMask = aspectMask;
			attachment.subresourceRange.levelCount = 1;
			attachment.subresourceRange.layerCount = createinfo.layerCount;

			vk::ImageViewCreateInfo imageView{};
			imageView.viewType = (createinfo.layerCount == 1) ? vk::ImageViewType::e2D  : vk::ImageViewType::e2DArray;
			imageView.format = createinfo.format;
			imageView.subresourceRange = attachment.subresourceRange;
			//todo: workaround for depth+stencil attachments
			imageView.subresourceRange.aspectMask = (attachment.hasDepth()) ? vk::ImageAspectFlagBits::eDepth : aspectMask;
			imageView.image = attachment.image;
			VK_CHECK_RESULT(vulkanDevice->logicalDevice.createImageView(&imageView, nullptr, &attachment.view));

			// Fill attachment description
			attachment.description = vk::AttachmentDescription();
			attachment.description.samples = createinfo.imageSampleCount;
			attachment.description.loadOp = vk::AttachmentLoadOp::eClear;
			attachment.description.storeOp = (createinfo.usage & vk::ImageUsageFlagBits::eSampled) ? vk::AttachmentStoreOp::eStore : vk::AttachmentStoreOp::eDontCare;
			attachment.description.stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
			attachment.description.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
			attachment.description.format = createinfo.format;
			attachment.description.initialLayout = vk::ImageLayout::eUndefined;
			// Final layout
			// If not, final layout depends on attachment type
			if (attachment.hasDepth() || attachment.hasStencil())
			{
				attachment.description.finalLayout = vk::ImageLayout::eDepthStencilReadOnlyOptimal;
			}
			else
			{
				attachment.description.finalLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
			}

			attachments.push_back(attachment);

			return static_cast<uint32_t>(attachments.size() - 1);

		}

		/**
		* Creates a default sampler for sampling from any of the framebuffer attachments
		* Applications are free to create their own samplers for different use cases
		*
		* @param magFilter Magnification filter for lookups
		* @param minFilter Minification filter for lookups
		* @param adressMode Addressing mode for the U,V and W coordinates
		*
		* @return VkResult for the sampler creation
		*/
		vk::Result createSampler(vk::Filter magFilter, vk::Filter minFilter, vk::SamplerAddressMode adressMode)
		{
			vk::SamplerCreateInfo samplerInfo{};
			samplerInfo.maxAnisotropy = 1.0f;
			samplerInfo.magFilter = magFilter;
			samplerInfo.minFilter = minFilter;
			samplerInfo.mipmapMode = vk::SamplerMipmapMode::eLinear;
			samplerInfo.addressModeU = adressMode;
			samplerInfo.addressModeV = adressMode;
			samplerInfo.addressModeW = adressMode;
			samplerInfo.mipLodBias = 0.0f;
			samplerInfo.maxAnisotropy = 1.0f;
			samplerInfo.minLod = 0.0f;
			samplerInfo.maxLod = 1.0f;
			samplerInfo.borderColor = vk::BorderColor::eFloatOpaqueWhite;;
			return vulkanDevice->logicalDevice.createSampler(&samplerInfo, nullptr, &sampler);
		}

		void createRenderPass(vk::Format swapchainImageFormat)
		{
			vk::AttachmentDescription colorAttachment{};
			colorAttachment.format = swapchainImageFormat;
			colorAttachment.samples = vk::SampleCountFlagBits::e1;
			colorAttachment.loadOp = vk::AttachmentLoadOp::eClear;
			colorAttachment.storeOp = vk::AttachmentStoreOp::eStore;
			colorAttachment.stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
			colorAttachment.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
			colorAttachment.initialLayout = vk::ImageLayout::eUndefined;
			colorAttachment.finalLayout = vk::ImageLayout::ePresentSrcKHR;
		
			std::vector<vk::AttachmentDescription> attachmentsDescriptions = { colorAttachment };

			for (auto& attachment : attachments)
			{
				attachmentsDescriptions.push_back(attachment.description);
			};

			

			vk::AttachmentReference colorAttachmentRef{};

			colorAttachmentRef.attachment = 0;
			colorAttachmentRef.layout = vk::ImageLayout::eColorAttachmentOptimal;

			vk::AttachmentReference depthAttachmentRef{};
			depthAttachmentRef.attachment = 1;
			depthAttachmentRef.layout = vk::ImageLayout::eDepthStencilAttachmentOptimal;


			vk::SubpassDescription subpass{};
			subpass.pipelineBindPoint = vk::PipelineBindPoint::eGraphics;
			subpass.colorAttachmentCount = 1;
			subpass.pColorAttachments = &colorAttachmentRef;
			subpass.pDepthStencilAttachment = &depthAttachmentRef;

			vk::SubpassDependency dependency{};
			dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
			dependency.dstSubpass = 0;
			dependency.srcStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput | vk::PipelineStageFlagBits::eEarlyFragmentTests;
			dependency.srcAccessMask = {};
			dependency.dstStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput | vk::PipelineStageFlagBits::eEarlyFragmentTests;
			dependency.dstAccessMask = vk::AccessFlagBits::eColorAttachmentWrite | vk::AccessFlagBits::eDepthStencilAttachmentWrite;

			
			vk::RenderPassCreateInfo renderPassInfo{};

			renderPassInfo.attachmentCount = static_cast<uint32_t>(attachmentsDescriptions.size());
			renderPassInfo.pAttachments = attachmentsDescriptions.data();
			renderPassInfo.subpassCount = 1;
			renderPassInfo.pSubpasses = &subpass;
			renderPassInfo.dependencyCount = 1;
			renderPassInfo.pDependencies = &dependency;

			if (vulkanDevice->logicalDevice.createRenderPass(&renderPassInfo, nullptr, &renderPass) != vk::Result::eSuccess) {
				throw std::runtime_error("failed to create render pass!");
			}
		}

	};
}


#endif // !RENDERING_VULKAN_FRAMEBUFFER

