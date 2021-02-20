///*
//* Vulkan texture loader
//*
//* Copyright(C) by Sascha Willems - www.saschawillems.de
//*
//* This code is licensed under the MIT license(MIT) (http://opensource.org/licenses/MIT)
//*/
//
#include "VulkanTexture.h"
#include "VulkanValidate.h"
#include "VulkanHelper.h"
#include <algorithm>
#include <stb_image.h>
namespace vkh
{
	void Texture::updateDescriptor()
	{
		descriptor.sampler = textureSampler;
		descriptor.imageView = textureImageView;
		descriptor.imageLayout = imageLayout;
	}

	Texture::Texture(te::vkh::VulkanDevice* device): device(device) {}




    Texture::~Texture()
	{
		device->logicalDevice.destroyImageView(textureImageView, nullptr);
		device->logicalDevice.destroyImage(textureImage, nullptr);
		if (textureSampler)
		{
			device->logicalDevice.destroySampler(textureSampler, nullptr);
			
		}
		device->logicalDevice.freeMemory(deviceMemory);

	}

	void Texture2D::createTextureImage() {

        int texWidth, texHeight, texChannels;
        stbi_uc* pixels = stbi_load(filePath.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
        vk::DeviceSize imageSize = texWidth * texHeight * 4;
        mipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(texWidth, texHeight)))) + 1;

        if (!pixels) {
            throw std::runtime_error("failed to load texture image!");
        }

        vk::Buffer stagingBuffer;
        vk::DeviceMemory stagingBufferMemory;
        device->createBuffer(
            imageSize,
            vk::BufferUsageFlagBits::eTransferSrc,
            vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent,
            stagingBuffer, stagingBufferMemory);

        void* data;
        device->logicalDevice.mapMemory(stagingBufferMemory, 0, imageSize, {}, &data);
        memcpy(data, pixels, static_cast<size_t>(imageSize));
        device->logicalDevice.unmapMemory(stagingBufferMemory);

        stbi_image_free(pixels);

        device->createImage(
            texWidth, texHeight, mipLevels,
            format,
            vk::ImageTiling::eOptimal,
            vk::ImageUsageFlagBits::eTransferSrc |
            vk::ImageUsageFlagBits::eTransferDst |
            vk::ImageUsageFlagBits::eSampled,
            vk::MemoryPropertyFlagBits::eDeviceLocal,
            textureImage, deviceMemory
        );

        vk::CommandBuffer commandBuffer = device->beginSingleTimeCommands(device->commandPool);

        device->setImageLayout(
            commandBuffer, textureImage, mipLevels,
            vk::ImageAspectFlagBits::eColor, vk::ImageLayout::eUndefined,
            vk::ImageLayout::eTransferDstOptimal);

        device->endSingleTimeCommands(commandBuffer, copyQueue, device->commandPool);


        device->copyBufferToImage(
            stagingBuffer,
            textureImage,
            static_cast<uint32_t>(texWidth),
            static_cast<uint32_t>(texHeight),
            device->commandPool,
            copyQueue
        );

        vkDestroyBuffer(device->logicalDevice, stagingBuffer, nullptr);
        vkFreeMemory(device->logicalDevice, stagingBufferMemory, nullptr);

        device->generateMipmaps(
            textureImage,
            format,
            texWidth,
            texHeight,
            mipLevels, copyQueue
        );
	};
	void Texture2D::createTextureImageView() {
        textureImageView = device->createImageView(
            textureImage,
            format,
            vk::ImageAspectFlagBits::eColor,
            mipLevels
        );
    };
	void Texture2D::createTextureSampler() {

        vk::PhysicalDeviceProperties properties = device->info.properties;


        vk::SamplerCreateInfo samplerInfo{};

        samplerInfo.magFilter = vk::Filter::eLinear;
        samplerInfo.minFilter = vk::Filter::eLinear;
        samplerInfo.addressModeU = vk::SamplerAddressMode::eRepeat;
        samplerInfo.addressModeV = vk::SamplerAddressMode::eRepeat;
        samplerInfo.addressModeW = vk::SamplerAddressMode::eRepeat;
        samplerInfo.anisotropyEnable = VK_TRUE;
        samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;
        samplerInfo.borderColor = vk::BorderColor::eIntOpaqueBlack;;
        samplerInfo.unnormalizedCoordinates = VK_FALSE;
        samplerInfo.compareEnable = VK_FALSE;
        samplerInfo.compareOp = vk::CompareOp::eAlways;;
        samplerInfo.mipmapMode = vk::SamplerMipmapMode::eLinear;;
        samplerInfo.minLod = 0.0f;
        samplerInfo.maxLod = static_cast<float>(mipLevels);
        samplerInfo.mipLodBias = 0.0f;

        if (device->logicalDevice.createSampler(&samplerInfo, nullptr, &textureSampler) != vk::Result::eSuccess) {
            throw std::runtime_error("failed to create texture sampler!");
        }
    };

	void Texture2D::loadFromFile(std::string filename, vk::Format format, vk::Queue copyQueue, vk::ImageUsageFlags imageUsageFlags, vk::ImageLayout imageLayout, bool forceLinear)
	{
        this->filePath = filename;
        this->format = format;
        this->imageUsageFlags = imageUsageFlags;
        this->imageLayout = imageLayout;
        this->forceLinear = forceLinear;
        this->copyQueue = copyQueue;

		createTexture();
	}

	


	//
//	/**
//	* Load a 2D texture including all mip levels
//	*
//	* @param filename File to load (supports .ktx)
//	* @param format Vulkan format of the image data stored in the file
//	* @param device Vulkan device to create the texture on
//	* @param copyQueue Queue used for the texture staging copy commands (must support transfer)
//	* @param (Optional) imageUsageFlags Usage flags for the texture's image (defaults to VK_IMAGE_USAGE_SAMPLED_BIT)
//	* @param (Optional) imageLayout Usage layout for the texture (defaults VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
//	* @param (Optional) forceLinear Force linear tiling (not advised, defaults to false)
//	*
//	*/
//	void Texture2D::loadFromFile(std::string filename, vk::Format format, te::vkh::VulkanDevice* device, vk::Queue copyQueue, vk::ImageUsageFlags imageUsageFlags, vk::ImageLayout imageLayout, bool forceLinear)
//	{
//		ktxTexture* ktxTexture;
//		ktxResult result = loadKTXFile(filename, &ktxTexture);
//		assert(result == KTX_SUCCESS);
//
//		this->device = device;
//		width = ktxTexture->baseWidth;
//		height = ktxTexture->baseHeight;
//		mipLevels = ktxTexture->numLevels;
//
//		ktx_uint8_t* ktxTextureData = ktxTexture_GetData(ktxTexture);
//		ktx_size_t ktxTextureSize = ktxTexture_GetSize(ktxTexture);
//
//		// Get device properties for the requested texture format
//		vk::FormatProperties formatProperties;	
//		
//		device->physicalDevice.getFormatProperties(format, &formatProperties);
//		// Only use linear tiling if requested (and supported by the device)
//		// Support for linear tiling is mostly limited, so prefer to use
//		// optimal tiling instead
//		// On most implementations linear tiling will only support a very
//		// limited amount of formats and features (mip maps, cubemaps, arrays, etc.)
//		vk::Bool32 useStaging = !forceLinear;
//
//		vk::MemoryAllocateInfo memAllocInfo = {};
//		vk::MemoryRequirements memReqs;
//
//		// Use a separate command buffer for texture loading
//		vk::CommandBuffer copyCmd = device->createCommandBuffer(vk::CommandBufferLevel::ePrimary, true);
//
//		if (useStaging)
//		{
//			// Create a host-visible staging buffer that contains the raw image data
//			vk::Buffer stagingBuffer;
//			vk::DeviceMemory stagingMemory;
//
//			vk::BufferCreateInfo bufferCreateInfo = {};
//			bufferCreateInfo.size = ktxTextureSize;
//			// This buffer is used as a transfer source for the buffer copy
//			bufferCreateInfo.usage = vk::BufferUsageFlagBits::eTransferSrc;
//			bufferCreateInfo.sharingMode = vk::SharingMode::eExclusive;
//
//			VK_CHECK_RESULT(device->logicalDevice.createBuffer(&bufferCreateInfo, nullptr, &stagingBuffer));
//
//			// Get memory requirements for the staging buffer (alignment, memory type bits)
//			
//			
//			device->logicalDevice.getBufferMemoryRequirements(stagingBuffer, &memReqs);
//
//			memAllocInfo.allocationSize = memReqs.size;
//			// Get memory type index for a host visible buffer
//			memAllocInfo.memoryTypeIndex = device->getMemoryType(memReqs.memoryTypeBits, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);
//
//			VK_CHECK_RESULT(device->logicalDevice.allocateMemory(&memAllocInfo, nullptr, &stagingMemory));
//			/*VK_CHECK_RESULT(vkBindBufferMemory(device->logicalDevice, stagingBuffer, stagingMemory, 0));*/
//			device->logicalDevice.bindBufferMemory(stagingBuffer, stagingMemory, 0);
//
//			// Copy texture data into staging buffer
//			uint8_t* data;
//			VK_CHECK_RESULT(device->logicalDevice.mapMemory(stagingMemory, 0, memReqs.size, {}, (void**)&data));
//			memcpy(data, ktxTextureData, ktxTextureSize);
//			vkUnmapMemory(device->logicalDevice, stagingMemory);
//
//			// Setup buffer copy regions for each mip level
//			std::vector<vk::BufferImageCopy> bufferCopyRegions;
//
//			for (uint32_t i = 0; i < mipLevels; i++)
//			{
//				ktx_size_t offset;
//				KTX_error_code result = ktxTexture_GetImageOffset(ktxTexture, i, 0, 0, &offset);
//				assert(result == KTX_SUCCESS);
//
//				vk::BufferImageCopy bufferCopyRegion = {};
//				bufferCopyRegion.imageSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
//				bufferCopyRegion.imageSubresource.mipLevel = i;
//				bufferCopyRegion.imageSubresource.baseArrayLayer = 0;
//				bufferCopyRegion.imageSubresource.layerCount = 1;
//				bufferCopyRegion.imageExtent.width = std::max(1u, ktxTexture->baseWidth >> i);
//				bufferCopyRegion.imageExtent.height = std::max(1u, ktxTexture->baseHeight >> i);
//				bufferCopyRegion.imageExtent.depth = 1;
//				bufferCopyRegion.bufferOffset = offset;
//
//				bufferCopyRegions.push_back(bufferCopyRegion);
//			}
//
//			// Create optimal tiled target image
//			vk::ImageCreateInfo imageCreateInfo = {};
//
//
//			imageCreateInfo.imageType = vk::ImageType::e2D;
//			imageCreateInfo.format = format;
//			imageCreateInfo.mipLevels = mipLevels;
//			imageCreateInfo.arrayLayers = 1;
//			imageCreateInfo.samples = vk::SampleCountFlagBits::e1;
//			imageCreateInfo.tiling = vk::ImageTiling::eOptimal;
//			imageCreateInfo.sharingMode = vk::SharingMode::eExclusive;
//			imageCreateInfo.initialLayout = vk::ImageLayout::eUndefined;
//			imageCreateInfo.extent = vk::Extent3D{ width, height, 1 };
//			imageCreateInfo.usage = imageUsageFlags;
//			// Ensure that the TRANSFER_DST bit is set for staging
//			if (!(imageCreateInfo.usage & vk::ImageUsageFlagBits::eTransferDst))
//			{
//				imageCreateInfo.usage |= vk::ImageUsageFlagBits::eTransferDst;
//			}
//			VK_CHECK_RESULT(device->logicalDevice.createImage(&imageCreateInfo, nullptr, &image));
//
//			device->logicalDevice.getImageMemoryRequirements(image, &memReqs);
//			
//			memAllocInfo.allocationSize = memReqs.size;
//
//			memAllocInfo.memoryTypeIndex = device->getMemoryType(memReqs.memoryTypeBits, vk::MemoryPropertyFlagBits::eDeviceLocal);
//			VK_CHECK_RESULT(device->logicalDevice.allocateMemory(&memAllocInfo, nullptr, &deviceMemory));
//			//VK_CHECK_RESULT(device->logicalDevice.bindImageMemory(image, deviceMemory, 0));
//			device->logicalDevice.bindImageMemory(image, deviceMemory, 0);
//
//			vk::ImageSubresourceRange subresourceRange = {};
//			subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
//			subresourceRange.baseMipLevel = 0;
//			subresourceRange.levelCount = mipLevels;
//			subresourceRange.layerCount = 1;
//
//			// Image barrier for optimal image (target)
//			// Optimal image will be used as destination for the copy
//			te::vkh::VulkanHelper::setImageLayout(
//				copyCmd,
//				image,
//				vk::ImageLayout::eUndefined,
//				vk::ImageLayout::eTransferDstOptimal,
//				subresourceRange);
//
//			// Copy mip levels from staging buffer
//			copyCmd.copyBufferToImage(				
//				stagingBuffer,
//				image,
//				vk::ImageLayout::eTransferDstOptimal,
//				static_cast<uint32_t>(bufferCopyRegions.size()),
//				bufferCopyRegions.data()
//			);
//
//			// Change texture image layout to shader read after all mip levels have been copied
//			this->imageLayout = imageLayout;
//			te::vkh::VulkanHelper::setImageLayout(
//				copyCmd,
//				image,
//				vk::ImageLayout::eTransferDstOptimal,
//				imageLayout,
//				subresourceRange);
//
//			device->flushCommandBuffer(copyCmd, copyQueue);
//
//			// Clean up staging resources
//			vkFreeMemory(device->logicalDevice, stagingMemory, nullptr);
//			vkDestroyBuffer(device->logicalDevice, stagingBuffer, nullptr);
//		}
//		else
//		{
//			// Prefer using optimal tiling, as linear tiling 
//			// may support only a small set of features 
//			// depending on implementation (e.g. no mip maps, only one layer, etc.)
//
//			// Check if this support is supported for linear tiling
//			assert(formatProperties.linearTilingFeatures &vk::FormatFeatureFlagBits::eSampledImage);
//
//			vk::Image mappableImage;
//			vk::DeviceMemory mappableMemory;
//
//			vk::ImageCreateInfo imageCreateInfo = {};
//			imageCreateInfo.imageType = vk::ImageType::e2D;
//			imageCreateInfo.format = format;
//			imageCreateInfo.extent = vk::Extent3D{ width, height, 1 };
//			imageCreateInfo.mipLevels = 1;
//			imageCreateInfo.arrayLayers = 1;
//			imageCreateInfo.samples = vk::SampleCountFlagBits::e1;
//			imageCreateInfo.tiling = vk::ImageTiling::eLinear;
//			imageCreateInfo.usage = imageUsageFlags;
//			imageCreateInfo.sharingMode = vk::SharingMode::eExclusive;
//			imageCreateInfo.initialLayout = vk::ImageLayout::eUndefined;
//
//			// Load mip map level 0 to linear tiling image
//			VK_CHECK_RESULT(device->logicalDevice.createImage(&imageCreateInfo, nullptr, &mappableImage));
//
//			// Get memory requirements for this image 
//			// like size and alignment			
//			device->logicalDevice.getImageMemoryRequirements(mappableImage, &memReqs);
//			// Set memory allocation size to required memory size
//			memAllocInfo.allocationSize = memReqs.size;
//
//			// Get memory type that can be mapped to host memory
//			memAllocInfo.memoryTypeIndex = device->getMemoryType(memReqs.memoryTypeBits, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);
//
//			// Allocate host memory
//			VK_CHECK_RESULT(device->logicalDevice.allocateMemory(&memAllocInfo, nullptr, &mappableMemory));
//			
//			//VK_CHECK_RESULT(vkBindImageMemory(device->logicalDevice, mappableImage, mappableMemory, 0));
//			device->logicalDevice.bindImageMemory(mappableImage, mappableMemory, 0);
//			// Bind allocated image for use
//			
//
//			// Get sub resource layout
//			// Mip map count, array layer, etc.
//			vk::ImageSubresource subRes = {};
//			subRes.aspectMask = vk::ImageAspectFlagBits::eColor;
//			subRes.mipLevel = 0;
//
//			vk::SubresourceLayout subResLayout;
//			void* data;
//
//			// Get sub resources layout 
//			// Includes row pitch, size offsets, etc.
//			device->logicalDevice.getImageSubresourceLayout(mappableImage, &subRes, &subResLayout);
//
//			// Map image memory
//			VK_CHECK_RESULT(device->logicalDevice.mapMemory(mappableMemory, 0, memReqs.size, {}, & data));
//
//			// Copy image data into memory
//			memcpy(data, ktxTextureData, memReqs.size);
//
//			vkUnmapMemory(device->logicalDevice, mappableMemory);
//
//			// Linear tiled images don't need to be staged
//			// and can be directly used as textures
//			image = mappableImage;
//			deviceMemory = mappableMemory;
//			this->imageLayout = imageLayout;
//
//			// Setup image memory barrier
//			te::vkh::VulkanHelper::setImageLayout(copyCmd, image, vk::ImageAspectFlagBits::eColor, vk::ImageLayout::eUndefined, imageLayout);
//
//			device->flushCommandBuffer(copyCmd, copyQueue);
//		}
//
//		ktxTexture_Destroy(ktxTexture);
//
//		// Create a default sampler
//		vk::SamplerCreateInfo samplerCreateInfo = {};
//		samplerCreateInfo.maxAnisotropy = 1.0f;
//		samplerCreateInfo.magFilter = vk::Filter::eLinear;
//		samplerCreateInfo.minFilter = vk::Filter::eLinear;
//		samplerCreateInfo.mipmapMode = vk::SamplerMipmapMode::eLinear;
//		samplerCreateInfo.addressModeU = vk::SamplerAddressMode::eRepeat;
//		samplerCreateInfo.addressModeV = vk::SamplerAddressMode::eRepeat;
//		samplerCreateInfo.addressModeW = vk::SamplerAddressMode::eRepeat;
//
//		samplerCreateInfo.mipLodBias = 0.0f;
//		samplerCreateInfo.compareOp = vk::CompareOp::eNever;
//		samplerCreateInfo.minLod = 0.0f;
//		// Max level-of-detail should match mip level count
//		samplerCreateInfo.maxLod = (useStaging) ? (float)mipLevels : 0.0f;
//		// Only enable anisotropic filtering if enabled on the device
//		samplerCreateInfo.maxAnisotropy = device->enabledFeatures.samplerAnisotropy ? device->properties.limits.maxSamplerAnisotropy : 1.0f;
//		samplerCreateInfo.anisotropyEnable = device->enabledFeatures.samplerAnisotropy;
//		samplerCreateInfo.borderColor = vk::BorderColor::eFloatOpaqueWhite;
//		VK_CHECK_RESULT(device->logicalDevice.createSampler(&samplerCreateInfo, nullptr, &sampler));
//
//		// Create image view
//		// Textures are not directly accessed by the shaders and
//		// are abstracted by image views containing additional
//		// information and sub resource ranges
//		vk::ImageViewCreateInfo viewCreateInfo = {};
//		
//		viewCreateInfo.viewType = vk::ImageViewType::e2D;
//		viewCreateInfo.format = format;
//		viewCreateInfo.components = { vk::ComponentSwizzle::eR,  vk::ComponentSwizzle::eG,  vk::ComponentSwizzle::eB,  vk::ComponentSwizzle::eA };
//		viewCreateInfo.subresourceRange = { vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1 };
//		// Linear tiling usually won't support mip maps
//		// Only set mip map count if optimal tiling is used
//		viewCreateInfo.subresourceRange.levelCount = (useStaging) ? mipLevels : 1;
//		viewCreateInfo.image = image;
//		VK_CHECK_RESULT(device->logicalDevice.createImageView(&viewCreateInfo, nullptr, &view));
//
//		// Update descriptor image info member that can be used for setting up descriptor sets
//		updateDescriptor();
//	}
//
//	/**
//	* Creates a 2D texture from a buffer
//	*
//	* @param buffer Buffer containing texture data to upload
//	* @param bufferSize Size of the buffer in machine units
//	* @param width Width of the texture to create
//	* @param height Height of the texture to create
//	* @param format Vulkan format of the image data stored in the file
//	* @param device Vulkan device to create the texture on
//	* @param copyQueue Queue used for the texture staging copy commands (must support transfer)
//	* @param (Optional) filter Texture filtering for the sampler (defaults to VK_FILTER_LINEAR)
//	* @param (Optional) imageUsageFlags Usage flags for the texture's image (defaults to VK_IMAGE_USAGE_SAMPLED_BIT)
//	* @param (Optional) imageLayout Usage layout for the texture (defaults VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
//	*/
//	void Texture2D::fromBuffer(void* buffer, vk::DeviceSize bufferSize, vk::Format format, uint32_t texWidth, uint32_t texHeight, te::vkh::VulkanDevice* device, vk::Queue copyQueue, vk::Filter filter, vk::ImageUsageFlags imageUsageFlags, vk::ImageLayout imageLayout)
//	{
//		assert(buffer);
//
//		this->device = device;
//		width = texWidth;
//		height = texHeight;
//		mipLevels = 1;
//
//		vk::MemoryAllocateInfo memAllocInfo = {};
//		vk::MemoryRequirements memReqs;
//
//		// Use a separate command buffer for texture loading
//		vk::CommandBuffer copyCmd = device->createCommandBuffer(vk::CommandBufferLevel::ePrimary , true);
//
//		// Create a host-visible staging buffer that contains the raw image data
//		vk::Buffer stagingBuffer;
//		vk::DeviceMemory stagingMemory;
//
//		vk::BufferCreateInfo bufferCreateInfo = {};
//		bufferCreateInfo.size = bufferSize;
//		// This buffer is used as a transfer source for the buffer copy
//		bufferCreateInfo.usage = vk::BufferUsageFlagBits::eTransferSrc;
//		bufferCreateInfo.sharingMode = vk::SharingMode::eExclusive;
//
//		VK_CHECK_RESULT(device->logicalDevice.createBuffer(&bufferCreateInfo, nullptr, &stagingBuffer));
//
//		// Get memory requirements for the staging buffer (alignment, memory type bits)
//		
//		
//		device->logicalDevice.getBufferMemoryRequirements(stagingBuffer, &memReqs);
//
//		memAllocInfo.allocationSize = memReqs.size;
//		// Get memory type index for a host visible buffer
//		memAllocInfo.memoryTypeIndex = device->getMemoryType(memReqs.memoryTypeBits, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);
//
//		VK_CHECK_RESULT(device->logicalDevice.allocateMemory(&memAllocInfo, nullptr, &stagingMemory));
//	
//		device->logicalDevice.bindBufferMemory(stagingBuffer, stagingMemory, 0);
//		// Copy texture data into staging buffer
//		uint8_t* data;
//		VK_CHECK_RESULT(device->logicalDevice.mapMemory(stagingMemory, 0, memReqs.size, {}, (void**)&data));
//		memcpy(data, buffer, bufferSize);
//		vkUnmapMemory(device->logicalDevice, stagingMemory);
//
//		vk::BufferImageCopy bufferCopyRegion = {};
//		bufferCopyRegion.imageSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
//		bufferCopyRegion.imageSubresource.mipLevel = 0;
//		bufferCopyRegion.imageSubresource.baseArrayLayer = 0;
//		bufferCopyRegion.imageSubresource.layerCount = 1;
//		bufferCopyRegion.imageExtent.width = width;
//		bufferCopyRegion.imageExtent.height = height;
//		bufferCopyRegion.imageExtent.depth = 1;
//		bufferCopyRegion.bufferOffset = 0;
//
//		// Create optimal tiled target image
//		vk::ImageCreateInfo imageCreateInfo = {};
//		imageCreateInfo.imageType = vk::ImageType::e2D;
//		imageCreateInfo.format = format;
//		imageCreateInfo.mipLevels = mipLevels;
//		imageCreateInfo.arrayLayers = 1;
//		imageCreateInfo.samples = vk::SampleCountFlagBits::e1;
//		imageCreateInfo.tiling = vk::ImageTiling::eOptimal;
//		imageCreateInfo.sharingMode = vk::SharingMode::eExclusive;
//		imageCreateInfo.initialLayout = vk::ImageLayout::eUndefined;
//		imageCreateInfo.extent = vk::Extent3D{ width, height, 1 };
//		imageCreateInfo.usage = imageUsageFlags;
//		// Ensure that the TRANSFER_DST bit is set for staging
//		if (!(imageCreateInfo.usage & vk::ImageUsageFlagBits::eTransferDst))
//		{
//			imageCreateInfo.usage |= vk::ImageUsageFlagBits::eTransferDst;
//		}
//		VK_CHECK_RESULT(device->logicalDevice.createImage(&imageCreateInfo, nullptr, &image));
//
//		device->logicalDevice.getImageMemoryRequirements(image, &memReqs);
//	
//		memAllocInfo.allocationSize = memReqs.size;
//
//		memAllocInfo.memoryTypeIndex = device->getMemoryType(memReqs.memoryTypeBits, vk::MemoryPropertyFlagBits::eDeviceLocal);
//		VK_CHECK_RESULT(device->logicalDevice.allocateMemory(&memAllocInfo, nullptr, &deviceMemory));
//		//VK_CHECK_RESULT(vkBindImageMemory(device->logicalDevice, image, deviceMemory, 0));
//		device->logicalDevice.bindImageMemory(image, deviceMemory, 0);
//
//		vk::ImageSubresourceRange subresourceRange = {};
//		subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
//		subresourceRange.baseMipLevel = 0;
//		subresourceRange.levelCount = mipLevels;
//		subresourceRange.layerCount = 1;
//
//		// Image barrier for optimal image (target)
//		// Optimal image will be used as destination for the copy
//		te::vkh::VulkanHelper::setImageLayout(
//			copyCmd,
//			image,
//			vk::ImageLayout::eUndefined,
//			vk::ImageLayout::eTransferDstOptimal,
//			subresourceRange);
//
//		// Copy mip levels from staging buffer
//		copyCmd.copyBufferToImage(			
//			stagingBuffer,
//			image,
//			vk::ImageLayout::eTransferDstOptimal,
//			1,
//			&bufferCopyRegion);
//
//		
//
//		// Change texture image layout to shader read after all mip levels have been copied
//		this->imageLayout = imageLayout;
//
//		te::vkh::VulkanHelper::setImageLayout(
//			copyCmd,
//			image,
//			vk::ImageLayout::eTransferDstOptimal,
//			imageLayout,
//			subresourceRange);
//
//		device->flushCommandBuffer(copyCmd, copyQueue);
//
//		// Clean up staging resources
//		device->logicalDevice.freeMemory(stagingMemory, nullptr);
//		device->logicalDevice.destroyBuffer(stagingBuffer, nullptr);
//
//		// Create sampler
//		vk::SamplerCreateInfo samplerCreateInfo = {};
//		samplerCreateInfo.maxAnisotropy = 1.0f;
//		samplerCreateInfo.magFilter = filter;
//		samplerCreateInfo.minFilter = filter;
//		samplerCreateInfo.mipmapMode = vk::SamplerMipmapMode::eLinear;
//		samplerCreateInfo.addressModeU = vk::SamplerAddressMode::eRepeat;
//		samplerCreateInfo.addressModeV = vk::SamplerAddressMode::eRepeat;
//		samplerCreateInfo.addressModeW = vk::SamplerAddressMode::eRepeat;
//		samplerCreateInfo.mipLodBias = 0.0f;
//		samplerCreateInfo.compareOp = vk::CompareOp::eNever;
//		samplerCreateInfo.minLod = 0.0f;
//		samplerCreateInfo.maxLod = 0.0f;
//		samplerCreateInfo.maxAnisotropy = 1.0f;
//		VK_CHECK_RESULT(device->logicalDevice.createSampler(&samplerCreateInfo, nullptr, &sampler));
//
//		// Create image view
//		vk::ImageViewCreateInfo viewCreateInfo = {};
//	
//		viewCreateInfo.pNext = NULL;
//		viewCreateInfo.viewType = vk::ImageViewType::e2D;
//		viewCreateInfo.format = format;
//		viewCreateInfo.components = { vk::ComponentSwizzle::eR,  vk::ComponentSwizzle::eG,  vk::ComponentSwizzle::eB,  vk::ComponentSwizzle::eA };
//		viewCreateInfo.subresourceRange = { vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1 };
//		viewCreateInfo.subresourceRange.levelCount = 1;
//		viewCreateInfo.image = image;
//		VK_CHECK_RESULT(device->logicalDevice.createImageView(&viewCreateInfo, nullptr, &view));
//
//		// Update descriptor image info member that can be used for setting up descriptor sets
//		updateDescriptor();
//	}
//
//	/**
//	* Load a 2D texture array including all mip levels
//	*
//	* @param filename File to load (supports .ktx)
//	* @param format Vulkan format of the image data stored in the file
//	* @param device Vulkan device to create the texture on
//	* @param copyQueue Queue used for the texture staging copy commands (must support transfer)
//	* @param (Optional) imageUsageFlags Usage flags for the texture's image (defaults to VK_IMAGE_USAGE_SAMPLED_BIT)
//	* @param (Optional) imageLayout Usage layout for the texture (defaults VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
//	*
//	*/
//	void Texture2DArray::loadFromFile(std::string filename, vk::Format format,te::vkh::VulkanDevice* device, vk::Queue copyQueue, vk::ImageUsageFlags imageUsageFlags, vk::ImageLayout imageLayout)
//	{
//		ktxTexture* ktxTexture;
//		ktxResult result = loadKTXFile(filename, &ktxTexture);
//		assert(result == KTX_SUCCESS);
//
//		this->device = device;
//		width = ktxTexture->baseWidth;
//		height = ktxTexture->baseHeight;
//		layerCount = ktxTexture->numLayers;
//		mipLevels = ktxTexture->numLevels;
//
//		ktx_uint8_t* ktxTextureData = ktxTexture_GetData(ktxTexture);
//		ktx_size_t ktxTextureSize = ktxTexture_GetSize(ktxTexture);
//
//		vk::MemoryAllocateInfo memAllocInfo = {};
//		vk::MemoryRequirements memReqs;
//
//		// Create a host-visible staging buffer that contains the raw image data
//		vk::Buffer stagingBuffer;
//		vk::DeviceMemory stagingMemory;
//
//		vk::BufferCreateInfo bufferCreateInfo = {};
//		bufferCreateInfo.size = ktxTextureSize;
//		// This buffer is used as a transfer source for the buffer copy
//		bufferCreateInfo.usage = vk::BufferUsageFlagBits::eTransferSrc;
//		bufferCreateInfo.sharingMode = vk::SharingMode::eExclusive;
//
//		VK_CHECK_RESULT(device->logicalDevice.createBuffer(&bufferCreateInfo, nullptr, &stagingBuffer));
//
//		// Get memory requirements for the staging buffer (alignment, memory type bits)
//		
//		device->logicalDevice.getBufferMemoryRequirements(stagingBuffer, &memReqs);
//
//		memAllocInfo.allocationSize = memReqs.size;
//		// Get memory type index for a host visible buffer
//		memAllocInfo.memoryTypeIndex = device->getMemoryType(memReqs.memoryTypeBits, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);
//
//		VK_CHECK_RESULT(device->logicalDevice.allocateMemory(&memAllocInfo, nullptr, &stagingMemory));
//		
//		device->logicalDevice.bindBufferMemory(stagingBuffer, stagingMemory, 0);
//		//VK_CHECK_RESULT(vkBindBufferMemory(device->logicalDevice, stagingBuffer, stagingMemory, 0));
//
//		// Copy texture data into staging buffer
//		uint8_t* data;
//		VK_CHECK_RESULT(device->logicalDevice.mapMemory(stagingMemory, 0, memReqs.size, {}, (void**)&data));
//		memcpy(data, ktxTextureData, ktxTextureSize);
//		vkUnmapMemory(device->logicalDevice, stagingMemory);
//
//		// Setup buffer copy regions for each layer including all of its miplevels
//		std::vector<vk::BufferImageCopy> bufferCopyRegions;
//
//		for (uint32_t layer = 0; layer < layerCount; layer++)
//		{
//			for (uint32_t level = 0; level < mipLevels; level++)
//			{
//				ktx_size_t offset;
//				KTX_error_code result = ktxTexture_GetImageOffset(ktxTexture, level, layer, 0, &offset);
//				assert(result == KTX_SUCCESS);
//
//				vk::BufferImageCopy bufferCopyRegion = {};
//				bufferCopyRegion.imageSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
//				bufferCopyRegion.imageSubresource.mipLevel = level;
//				bufferCopyRegion.imageSubresource.baseArrayLayer = layer;
//				bufferCopyRegion.imageSubresource.layerCount = 1;
//				bufferCopyRegion.imageExtent.width = ktxTexture->baseWidth >> level;
//				bufferCopyRegion.imageExtent.height = ktxTexture->baseHeight >> level;
//				bufferCopyRegion.imageExtent.depth = 1;
//				bufferCopyRegion.bufferOffset = offset;
//
//				bufferCopyRegions.push_back(bufferCopyRegion);
//			}
//		}
//
//		// Create optimal tiled target image
//		vk::ImageCreateInfo imageCreateInfo = {};
//		imageCreateInfo.imageType = vk::ImageType::e2D;
//		imageCreateInfo.format = format;
//		imageCreateInfo.samples = vk::SampleCountFlagBits::e1;
//		imageCreateInfo.tiling = vk::ImageTiling::eOptimal;
//		imageCreateInfo.sharingMode = vk::SharingMode::eExclusive;
//		imageCreateInfo.initialLayout = vk::ImageLayout::eUndefined;
//		imageCreateInfo.extent = vk::Extent3D{ width, height, 1 };
//		imageCreateInfo.usage = imageUsageFlags;
//		// Ensure that the TRANSFER_DST bit is set for staging
//		if (!(imageCreateInfo.usage & vk::ImageUsageFlagBits::eTransferDst))
//		{
//			imageCreateInfo.usage |= vk::ImageUsageFlagBits::eTransferDst;
//		}
//		imageCreateInfo.arrayLayers = layerCount;
//		imageCreateInfo.mipLevels = mipLevels;
//
//		VK_CHECK_RESULT(device->logicalDevice.createImage(&imageCreateInfo, nullptr, &image));
//
//		device->logicalDevice.getImageMemoryRequirements(image, &memReqs);
//	
//		memAllocInfo.allocationSize = memReqs.size;
//		memAllocInfo.memoryTypeIndex = device->getMemoryType(memReqs.memoryTypeBits, vk::MemoryPropertyFlagBits::eDeviceLocal);
//
//
//		VK_CHECK_RESULT(device->logicalDevice.allocateMemory(&memAllocInfo, nullptr, &deviceMemory));
//		device->logicalDevice.bindImageMemory(image, deviceMemory, 0);
//		//VK_CHECK_RESULT(vkBindImageMemory(device->logicalDevice, image, deviceMemory, 0));
//
//		// Use a separate command buffer for texture loading
//		vk::CommandBuffer copyCmd = device->createCommandBuffer(vk::CommandBufferLevel::ePrimary, true);
//
//		// Image barrier for optimal image (target)
//		// Set initial layout for all array layers (faces) of the optimal (target) tiled texture
//		vk::ImageSubresourceRange subresourceRange = {};
//		subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
//		subresourceRange.baseMipLevel = 0;
//		subresourceRange.levelCount = mipLevels;
//		subresourceRange.layerCount = layerCount;
//
//		te::vkh::VulkanHelper::setImageLayout(
//			copyCmd,
//			image,
//			vk::ImageLayout::eUndefined,
//			vk::ImageLayout::eTransferDstOptimal,
//			subresourceRange);
//
//		// Copy the layers and mip levels from the staging buffer to the optimal tiled image
//		copyCmd.copyBufferToImage(		
//			stagingBuffer,
//			image,
//			vk::ImageLayout::eTransferDstOptimal,
//			static_cast<uint32_t>(bufferCopyRegions.size()),
//			bufferCopyRegions.data());
//	
//		// Change texture image layout to shader read after all faces have been copied
//		this->imageLayout = imageLayout;
//		te::vkh::VulkanHelper::setImageLayout(
//			copyCmd,
//			image,
//			vk::ImageLayout::eTransferDstOptimal,
//			imageLayout,
//			subresourceRange);
//
//		device->flushCommandBuffer(copyCmd, copyQueue);
//
//		// Create sampler
//		vk::SamplerCreateInfo samplerCreateInfo = {};
//		samplerCreateInfo.maxAnisotropy = 1.0f;
//		samplerCreateInfo.magFilter = vk::Filter::eLinear;
//		samplerCreateInfo.minFilter = vk::Filter::eLinear;
//		samplerCreateInfo.mipmapMode = vk::SamplerMipmapMode::eLinear;
//		samplerCreateInfo.addressModeU = vk::SamplerAddressMode::eClampToEdge;
//		samplerCreateInfo.addressModeV = samplerCreateInfo.addressModeU;
//		samplerCreateInfo.addressModeW = samplerCreateInfo.addressModeU;
//		samplerCreateInfo.mipLodBias = 0.0f;
//		samplerCreateInfo.maxAnisotropy = device->enabledFeatures.samplerAnisotropy ? device->properties.limits.maxSamplerAnisotropy : 1.0f;
//		samplerCreateInfo.anisotropyEnable = device->enabledFeatures.samplerAnisotropy;
//		samplerCreateInfo.compareOp = vk::CompareOp::eNever;
//		samplerCreateInfo.minLod = 0.0f;
//		samplerCreateInfo.maxLod = (float)mipLevels;
//		samplerCreateInfo.borderColor = vk::BorderColor::eFloatOpaqueWhite;
//		VK_CHECK_RESULT(device->logicalDevice.createSampler(&samplerCreateInfo, nullptr, &sampler));
//
//		// Create image view
//		vk::ImageViewCreateInfo viewCreateInfo = {};
//		viewCreateInfo.viewType = vk::ImageViewType::e2DArray;
//		viewCreateInfo.format = format;
//		viewCreateInfo.components = { vk::ComponentSwizzle::eR,  vk::ComponentSwizzle::eG,  vk::ComponentSwizzle::eB,  vk::ComponentSwizzle::eA };
//		viewCreateInfo.subresourceRange = { vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1 };
//		viewCreateInfo.subresourceRange.layerCount = layerCount;
//		viewCreateInfo.subresourceRange.levelCount = mipLevels;
//		viewCreateInfo.image = image;
//		VK_CHECK_RESULT(device->logicalDevice.createImageView(&viewCreateInfo, nullptr, &view));
//
//		// Clean up staging resources
//		ktxTexture_Destroy(ktxTexture);
//		vkFreeMemory(device->logicalDevice, stagingMemory, nullptr);
//		vkDestroyBuffer(device->logicalDevice, stagingBuffer, nullptr);
//
//		// Update descriptor image info member that can be used for setting up descriptor sets
//		updateDescriptor();
//	}
//
//	/**
//	* Load a cubemap texture including all mip levels from a single file
//	*
//	* @param filename File to load (supports .ktx)
//	* @param format Vulkan format of the image data stored in the file
//	* @param device Vulkan device to create the texture on
//	* @param copyQueue Queue used for the texture staging copy commands (must support transfer)
//	* @param (Optional) imageUsageFlags Usage flags for the texture's image (defaults to VK_IMAGE_USAGE_SAMPLED_BIT)
//	* @param (Optional) imageLayout Usage layout for the texture (defaults VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
//	*
//	*/
//	void TextureCubeMap::loadFromFile(std::string filename, vk::Format format, te::vkh::VulkanDevice* device, vk::Queue copyQueue, vk::ImageUsageFlags imageUsageFlags, vk::ImageLayout imageLayout)
//	{
//		ktxTexture* ktxTexture;
//		ktxResult result = loadKTXFile(filename, &ktxTexture);
//		assert(result == KTX_SUCCESS);
//
//		this->device = device;
//		width = ktxTexture->baseWidth;
//		height = ktxTexture->baseHeight;
//		mipLevels = ktxTexture->numLevels;
//
//		ktx_uint8_t* ktxTextureData = ktxTexture_GetData(ktxTexture);
//		ktx_size_t ktxTextureSize = ktxTexture_GetSize(ktxTexture);
//
//		vk::MemoryAllocateInfo memAllocInfo = {};
//		vk::MemoryRequirements memReqs;
//
//		// Create a host-visible staging buffer that contains the raw image data
//		vk::Buffer stagingBuffer;
//		vk::DeviceMemory stagingMemory;
//
//		vk::BufferCreateInfo bufferCreateInfo = {};
//		bufferCreateInfo.size = ktxTextureSize;
//		// This buffer is used as a transfer source for the buffer copy
//		bufferCreateInfo.usage = vk::BufferUsageFlagBits::eTransferSrc;
//		bufferCreateInfo.sharingMode = vk::SharingMode::eExclusive;
//
//		VK_CHECK_RESULT(device->logicalDevice.createBuffer(&bufferCreateInfo, nullptr, &stagingBuffer));
//
//		// Get memory requirements for the staging buffer (alignment, memory type bits)
//		device->logicalDevice.getBufferMemoryRequirements(stagingBuffer, &memReqs);
//
//		memAllocInfo.allocationSize = memReqs.size;
//		// Get memory type index for a host visible buffer
//		memAllocInfo.memoryTypeIndex = device->getMemoryType(memReqs.memoryTypeBits, vk::MemoryPropertyFlagBits::eHostVisible  | vk::MemoryPropertyFlagBits::eHostCoherent);
//
//		VK_CHECK_RESULT(device->logicalDevice.allocateMemory(&memAllocInfo, nullptr, &stagingMemory));
//		//VK_CHECK_RESULT(vkBindBufferMemory(device->logicalDevice, stagingBuffer, stagingMemory, 0));
//		device->logicalDevice.bindBufferMemory(stagingBuffer, stagingMemory, 0);
//		// Copy texture data into staging buffer
//		uint8_t* data;
//		VK_CHECK_RESULT(device->logicalDevice.mapMemory(stagingMemory, 0, memReqs.size, {}, (void**)&data));
//		memcpy(data, ktxTextureData, ktxTextureSize);
//		vkUnmapMemory(device->logicalDevice, stagingMemory);
//
//		// Setup buffer copy regions for each face including all of its mip levels
//		std::vector<vk::BufferImageCopy> bufferCopyRegions;
//
//		for (uint32_t face = 0; face < 6; face++)
//		{
//			for (uint32_t level = 0; level < mipLevels; level++)
//			{
//				ktx_size_t offset;
//				KTX_error_code result = ktxTexture_GetImageOffset(ktxTexture, level, 0, face, &offset);
//				assert(result == KTX_SUCCESS);
//
//				vk::BufferImageCopy bufferCopyRegion = {};
//				bufferCopyRegion.imageSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
//				bufferCopyRegion.imageSubresource.mipLevel = level;
//				bufferCopyRegion.imageSubresource.baseArrayLayer = face;
//				bufferCopyRegion.imageSubresource.layerCount = 1;
//				bufferCopyRegion.imageExtent.width = ktxTexture->baseWidth >> level;
//				bufferCopyRegion.imageExtent.height = ktxTexture->baseHeight >> level;
//				bufferCopyRegion.imageExtent.depth = 1;
//				bufferCopyRegion.bufferOffset = offset;
//
//				bufferCopyRegions.push_back(bufferCopyRegion);
//			}
//		}
//
//		// Create optimal tiled target image
//		vk::ImageCreateInfo imageCreateInfo = {};
//		imageCreateInfo.imageType = vk::ImageType::e2D;
//		imageCreateInfo.format = format;
//		imageCreateInfo.mipLevels = mipLevels;
//		imageCreateInfo.samples = vk::SampleCountFlagBits::e1;
//		imageCreateInfo.tiling = vk::ImageTiling::eOptimal;
//		imageCreateInfo.sharingMode = vk::SharingMode::eExclusive;
//		imageCreateInfo.initialLayout = vk::ImageLayout::eUndefined;
//		imageCreateInfo.extent = vk::Extent3D{ width, height, 1 };
//		imageCreateInfo.usage = imageUsageFlags;
//		// Ensure that the TRANSFER_DST bit is set for staging
//		if (!(imageCreateInfo.usage & vk::ImageUsageFlagBits::eTransferDst))
//		{
//			imageCreateInfo.usage |= vk::ImageUsageFlagBits::eTransferDst;
//		}
//		// Cube faces count as array layers in Vulkan
//		imageCreateInfo.arrayLayers = 6;
//		// This flag is required for cube map images
//		imageCreateInfo.flags = vk::ImageCreateFlagBits::eCubeCompatible;
//
//
//		VK_CHECK_RESULT(device->logicalDevice.createImage(&imageCreateInfo, nullptr, &image));
//
//		device->logicalDevice.getImageMemoryRequirements(image, &memReqs);
//
//		memAllocInfo.allocationSize = memReqs.size;
//		memAllocInfo.memoryTypeIndex = device->getMemoryType(memReqs.memoryTypeBits, vk::MemoryPropertyFlagBits::eDeviceLocal);
//
//		VK_CHECK_RESULT(device->logicalDevice.allocateMemory(&memAllocInfo, nullptr, &deviceMemory));
//
//		//VK_CHECK_RESULT(vkBindImageMemory(device->logicalDevice, image, deviceMemory, 0));
//		device->logicalDevice.bindImageMemory(image, deviceMemory, 0);
//
//		// Use a separate command buffer for texture loading
//		vk::CommandBuffer copyCmd = device->createCommandBuffer(vk::CommandBufferLevel::ePrimary, true);
//
//		// Image barrier for optimal image (target)
//		// Set initial layout for all array layers (faces) of the optimal (target) tiled texture
//		vk::ImageSubresourceRange subresourceRange = {};
//
//		subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
//		subresourceRange.baseMipLevel = 0;
//		subresourceRange.levelCount = mipLevels;
//		subresourceRange.layerCount = 6;
//
//		te::vkh::VulkanHelper::setImageLayout(
//			copyCmd,
//			image,
//			vk::ImageLayout::eUndefined,
//			vk::ImageLayout::eTransferDstOptimal,
//			subresourceRange);
//
//		// Copy the cube map faces from the staging buffer to the optimal tiled image
//		copyCmd.copyBufferToImage(			
//			stagingBuffer,
//			image,
//			vk::ImageLayout::eTransferDstOptimal,
//			static_cast<uint32_t>(bufferCopyRegions.size()),
//			bufferCopyRegions.data());
//
//		// Change texture image layout to shader read after all faces have been copied
//		this->imageLayout = imageLayout;
//
//		te::vkh::VulkanHelper::setImageLayout(
//			copyCmd,
//			image,
//			vk::ImageLayout::eTransferDstOptimal,
//			imageLayout,
//			subresourceRange);
//
//		device->flushCommandBuffer(copyCmd, copyQueue);
//
//		// Create sampler
//		vk::SamplerCreateInfo samplerCreateInfo = {};
//		samplerCreateInfo.maxAnisotropy = 1.0f;
//		samplerCreateInfo.magFilter = vk::Filter::eLinear;
//		samplerCreateInfo.minFilter = vk::Filter::eLinear;
//		samplerCreateInfo.mipmapMode = vk::SamplerMipmapMode::eLinear;
//		samplerCreateInfo.addressModeU = vk::SamplerAddressMode::eClampToEdge;
//		samplerCreateInfo.addressModeV = samplerCreateInfo.addressModeU;
//		samplerCreateInfo.addressModeW = samplerCreateInfo.addressModeU;
//		samplerCreateInfo.mipLodBias = 0.0f;
//		samplerCreateInfo.maxAnisotropy = device->enabledFeatures.samplerAnisotropy ? device->properties.limits.maxSamplerAnisotropy : 1.0f;
//		samplerCreateInfo.anisotropyEnable = device->enabledFeatures.samplerAnisotropy;
//		samplerCreateInfo.compareOp = vk::CompareOp::eNever;
//		samplerCreateInfo.minLod = 0.0f;
//		samplerCreateInfo.maxLod = (float)mipLevels;
//		samplerCreateInfo.borderColor = vk::BorderColor::eFloatOpaqueWhite;
//		VK_CHECK_RESULT(device->logicalDevice.createSampler(&samplerCreateInfo, nullptr, &sampler));
//
//		// Create image view
//		vk::ImageViewCreateInfo viewCreateInfo = {};
//
//		viewCreateInfo.viewType = vk::ImageViewType::eCube;
//		viewCreateInfo.format = format;
//		viewCreateInfo.components = { vk::ComponentSwizzle::eR,  vk::ComponentSwizzle::eG,  vk::ComponentSwizzle::eB,  vk::ComponentSwizzle::eA };
//		viewCreateInfo.subresourceRange = { vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1 };
//		viewCreateInfo.subresourceRange.layerCount = 6;
//		viewCreateInfo.subresourceRange.levelCount = mipLevels;
//		viewCreateInfo.image = image;
//		VK_CHECK_RESULT(device->logicalDevice.createImageView(&viewCreateInfo, nullptr, &view));
//
//		// Clean up staging resources
//		ktxTexture_Destroy(ktxTexture);
//		device->logicalDevice.freeMemory(stagingMemory, nullptr);
//		device->logicalDevice.destroyBuffer(stagingBuffer, nullptr);
//
//		// Update descriptor image info member that can be used for setting up descriptor sets
//		updateDescriptor();
//	}
//
}
