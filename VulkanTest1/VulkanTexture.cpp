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

        device->logicalDevice.destroyBuffer(stagingBuffer, nullptr);
        device->logicalDevice.freeMemory(stagingBufferMemory, nullptr);

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


    
    void TextureCubeMap::createTextureImage()
    {
        int texWidth, texHeight, texChannels;
       

        std::array<stbi_uc*, 6> imagesData;

        //6 картинок скайбокса
        for (int i = 0; i < pathsOfImages.size(); i++)
        {
            int texWidthElem, texHeightElem, texChannelsElem;
            imagesData[i] = stbi_load(pathsOfImages[i].c_str(), &texWidthElem, &texHeightElem, &texChannelsElem, STBI_rgb_alpha);

            if (i != 0)
                if (texWidthElem != texWidth || texHeightElem != texHeight || texChannels != texChannelsElem)
                    throw std::runtime_error("failed to cubmap images (images have different width, height or chanels)");

            if (!imagesData[i]) {
                throw std::runtime_error("failed to load texture image from file!");
            }

            texWidth = texWidthElem; 
            texHeight = texHeightElem;
            texChannels = texChannelsElem;
         
        }


        //размера буфера будет ширина * высоту * кол каналов * кол картинок скайбокса
        vk::DeviceSize imageSize = texWidth * texHeight * 4 * pathsOfImages.size();

       
        mipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(texWidth, texHeight)))) + 1;

        vk::Buffer stagingBuffer;
        vk::DeviceMemory stagingBufferMemory;

        device->createBuffer(
            imageSize,
            vk::BufferUsageFlagBits::eTransferSrc,
            vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent,
            stagingBuffer, stagingBufferMemory);

        //теперь необходимо привязать пискели картинок к буферу GPU

        vk::DeviceSize sizeOffset = texWidth * texHeight * 4;
        vk::DeviceSize currentOffset = 0;

        void* data;
        device->logicalDevice.mapMemory(stagingBufferMemory, 0, imageSize, {}, &data);

        char* bytes = reinterpret_cast<char*>(data);

        for (int i = 0; i < imagesData.size(); i++)
        {
            
            memcpy(bytes + (i * sizeOffset), imagesData[i], sizeOffset);           

            stbi_image_free(imagesData[i]);
        }
     
        device->logicalDevice.unmapMemory(stagingBufferMemory);

        device->createImage(
            texWidth, texHeight, 1,
            format,
            vk::ImageTiling::eOptimal,
            vk::ImageUsageFlagBits::eTransferSrc |
            vk::ImageUsageFlagBits::eTransferDst |
            vk::ImageUsageFlagBits::eSampled,
            vk::MemoryPropertyFlagBits::eDeviceLocal,
            textureImage, deviceMemory, 6, vk::ImageCreateFlagBits::eCubeCompatible
        );

        vk::CommandBuffer commandBuffer = device->beginSingleTimeCommands(device->commandPool);

        device->setImageLayout(
            commandBuffer, textureImage, 1,
            vk::ImageAspectFlagBits::eColor, vk::ImageLayout::eUndefined,
            vk::ImageLayout::eTransferDstOptimal, 6);

        device->endSingleTimeCommands(commandBuffer, copyQueue, device->commandPool);

        // Setup buffer copy regions for each face including all of its mip levels
        std::vector<vk::BufferImageCopy> bufferCopyRegions;

        sizeOffset = texWidth * texHeight * 4;
        currentOffset = 0;
        for (uint32_t face = 0; face < 6; face++)
        {   
            vk::BufferImageCopy bufferCopyRegion = {};
            bufferCopyRegion.imageSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
            bufferCopyRegion.imageSubresource.mipLevel = 0;
            bufferCopyRegion.imageSubresource.baseArrayLayer = face;
            bufferCopyRegion.imageSubresource.layerCount = 1;
            bufferCopyRegion.imageExtent.width = texWidth;
            bufferCopyRegion.imageExtent.height = texHeight;
            bufferCopyRegion.imageExtent.depth = 1;
            bufferCopyRegion.bufferOffset = currentOffset;

            bufferCopyRegions.push_back(bufferCopyRegion);
            currentOffset += sizeOffset;
            
        }

        commandBuffer = device->beginSingleTimeCommands(device->commandPool);
        commandBuffer.copyBufferToImage(stagingBuffer,
            textureImage,
            vk::ImageLayout::eTransferDstOptimal,
            static_cast<uint32_t>(bufferCopyRegions.size()),
            bufferCopyRegions.data());
        device->endSingleTimeCommands(commandBuffer, copyQueue, device->commandPool);


        commandBuffer = device->beginSingleTimeCommands(device->commandPool);

        device->setImageLayout(
            commandBuffer, textureImage, 1,
            vk::ImageAspectFlagBits::eColor, vk::ImageLayout::eTransferDstOptimal,
            vk::ImageLayout::eShaderReadOnlyOptimal, 6);

        device->endSingleTimeCommands(commandBuffer, copyQueue, device->commandPool);

        device->logicalDevice.destroyBuffer(stagingBuffer, nullptr);
        device->logicalDevice.freeMemory(stagingBufferMemory, nullptr);
    }

    void TextureCubeMap::createTextureImageView()
    {
        // Create image view
        vk::ImageViewCreateInfo viewCreateInfo = {};
        viewCreateInfo.viewType = vk::ImageViewType::eCube;
        viewCreateInfo.format = format;
        viewCreateInfo.components = { vk::ComponentSwizzle::eR, vk::ComponentSwizzle::eG,vk::ComponentSwizzle::eB,vk::ComponentSwizzle::eA };
        viewCreateInfo.subresourceRange = { vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1 };
        viewCreateInfo.subresourceRange.layerCount = 6;
        viewCreateInfo.subresourceRange.levelCount = 1;
        viewCreateInfo.image = textureImage;
        

        if (device->logicalDevice.createImageView(&viewCreateInfo, nullptr, &textureImageView) != vk::Result::eSuccess) {
            throw std::runtime_error("failed to create texture image view!");
        }
      
    }

    void TextureCubeMap::createTextureSampler()
    {
        vk::PhysicalDeviceProperties properties = device->info.properties;


        vk::SamplerCreateInfo samplerInfo{};

        samplerInfo.magFilter = vk::Filter::eLinear;
        samplerInfo.minFilter = vk::Filter::eLinear;
        samplerInfo.addressModeU = vk::SamplerAddressMode::eClampToEdge;
        samplerInfo.addressModeV = vk::SamplerAddressMode::eClampToEdge;
        samplerInfo.addressModeW = vk::SamplerAddressMode::eClampToEdge;
        samplerInfo.anisotropyEnable = device->info.features.samplerAnisotropy;
        samplerInfo.maxAnisotropy = device->info.features.samplerAnisotropy ? device->info.properties.limits.maxSamplerAnisotropy : 1.0f;
        samplerInfo.borderColor = vk::BorderColor::eFloatOpaqueWhite;
        samplerInfo.unnormalizedCoordinates = VK_FALSE;
        samplerInfo.compareEnable = VK_FALSE;
        samplerInfo.compareOp = vk::CompareOp::eNever;
        samplerInfo.mipmapMode = vk::SamplerMipmapMode::eLinear;
        samplerInfo.minLod = 0.0f;
        samplerInfo.maxLod = static_cast<float>(1);
        samplerInfo.mipLodBias = 0.0f;

        if (device->logicalDevice.createSampler(&samplerInfo, nullptr, &textureSampler) != vk::Result::eSuccess) {
            throw std::runtime_error("failed to create texture sampler!");
        }
    }


    void TextureCubeMap::loadFromFile(std::vector<std::string> pathsOfImages, vk::Format format, vk::Queue copyQueue, vk::ImageUsageFlags imageUsageFlags, vk::ImageLayout imageLayout)
    {
        this->pathsOfImages = pathsOfImages;
        this->format = format;
        this->imageUsageFlags = imageUsageFlags;
        this->imageLayout = imageLayout;
        this->forceLinear = forceLinear;
        this->copyQueue = copyQueue;

        createTexture();

       
    }

}
