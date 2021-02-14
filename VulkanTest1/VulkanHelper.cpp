#include "VulkanHelper.h"
#include <stdexcept>
#include "Vertex.h"

void te::vkh::VulkanDevice::copyBuffer(
    vk::Buffer srcBuffer,
    vk::Buffer dstBuffer,
    vk::DeviceSize size,
    vk::Queue graphicsQueue,
    vk::CommandPool commandPool
)
{
    vk::CommandBuffer commandBuffer = beginSingleTimeCommands(commandPool);

    vk::BufferCopy copyRegion{};
    copyRegion.size = size;
  
    commandBuffer.copyBuffer(srcBuffer, dstBuffer, 1, &copyRegion);
 
    endSingleTimeCommands(commandBuffer, graphicsQueue, commandPool);
}

vk::Bool32 te::vkh::VulkanDevice::formatIsFilterable(vk::Format format, vk::ImageTiling tiling)
{
    vk::FormatProperties formatProps;
   
   
    physicalDevice.getFormatProperties(format, &formatProps);

    if (tiling == vk::ImageTiling::eOptimal)
        return static_cast<vk::Bool32>(formatProps.optimalTilingFeatures & vk::FormatFeatureFlagBits::eSampledImageFilterLinear);

    if (tiling == vk::ImageTiling::eLinear)
        return static_cast<vk::Bool32>(formatProps.linearTilingFeatures & vk::FormatFeatureFlagBits::eSampledImageFilterLinear);

    return false;
}

void te::vkh::VulkanDevice::insertImageMemoryBarrier(
    vk::CommandBuffer cmdbuffer,
    vk::Image image,
    vk::AccessFlags srcAccessMask,
    vk::AccessFlags dstAccessMask,
    vk::ImageLayout oldImageLayout,
    vk::ImageLayout newImageLayout,
    vk::PipelineStageFlags srcStageMask,
    vk::PipelineStageFlags dstStageMask,
    vk::ImageSubresourceRange subresourceRange)
{
    vk::ImageMemoryBarrier imageMemoryBarrier = {};

    imageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    imageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

    imageMemoryBarrier.srcAccessMask = srcAccessMask;
    imageMemoryBarrier.dstAccessMask = dstAccessMask;
    imageMemoryBarrier.oldLayout = oldImageLayout;
    imageMemoryBarrier.newLayout = newImageLayout;
    imageMemoryBarrier.image = image;
    imageMemoryBarrier.subresourceRange = subresourceRange;


    cmdbuffer.pipelineBarrier(
        srcStageMask,
        dstStageMask,
        {},
        0, nullptr,
        0, nullptr,
        1, &imageMemoryBarrier
    );

}

void te::vkh::VulkanDevice::setImageLayout(
    vk::CommandBuffer cmdbuffer,
    vk::Image image,
    vk::ImageLayout oldImageLayout,
    vk::ImageLayout newImageLayout,
    vk::ImageSubresourceRange subresourceRange,
    vk::PipelineStageFlags srcStageMask,
    vk::PipelineStageFlags dstStageMask)
{
    // Create an image barrier object
    vk::ImageMemoryBarrier imageMemoryBarrier = {};

    imageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    imageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

    imageMemoryBarrier.oldLayout = oldImageLayout;
    imageMemoryBarrier.newLayout = newImageLayout;
    imageMemoryBarrier.image = image;
    imageMemoryBarrier.subresourceRange = subresourceRange;

    // Source layouts (old)
    // Source access mask controls actions that have to be finished on the old layout
    // before it will be transitioned to the new layout
    switch (oldImageLayout)
    {
    case vk::ImageLayout::eUndefined:
        // Image layout is undefined (or does not matter)
        // Only valid as initial layout
        // No flags required, listed only for completeness
        imageMemoryBarrier.srcAccessMask = {};
        break;

    case vk::ImageLayout::ePreinitialized:
        // Image is preinitialized
        // Only valid as initial layout for linear images, preserves memory contents
        // Make sure host writes have been finished
        imageMemoryBarrier.srcAccessMask = vk::AccessFlagBits::eHostWrite;
        break;

    case vk::ImageLayout::eColorAttachmentOptimal:
        // Image is a color attachment
        // Make sure any writes to the color buffer have been finished
        imageMemoryBarrier.srcAccessMask = vk::AccessFlagBits::eColorAttachmentWrite;
        break;

    case vk::ImageLayout::eDepthStencilAttachmentOptimal:
        // Image is a depth/stencil attachment
        // Make sure any writes to the depth/stencil buffer have been finished
        imageMemoryBarrier.srcAccessMask = vk::AccessFlagBits::eDepthStencilAttachmentWrite;
        break;

    case vk::ImageLayout::eTransferSrcOptimal:
        // Image is a transfer source
        // Make sure any reads from the image have been finished
        imageMemoryBarrier.srcAccessMask = vk::AccessFlagBits::eTransferRead;
        break;

    case vk::ImageLayout::eTransferDstOptimal:
        // Image is a transfer destination
        // Make sure any writes to the image have been finished
        imageMemoryBarrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
        break;

    case vk::ImageLayout::eShaderReadOnlyOptimal:
        // Image is read by a shader
        // Make sure any shader reads from the image have been finished
        imageMemoryBarrier.srcAccessMask = vk::AccessFlagBits::eShaderRead;
        break;
    default:
        // Other source layouts aren't handled (yet)
        break;
    }

    // Target layouts (new)
    // Destination access mask controls the dependency for the new image layout
    switch (newImageLayout)
    {
    case vk::ImageLayout::eTransferDstOptimal:
        // Image will be used as a transfer destination
        // Make sure any writes to the image have been finished
        imageMemoryBarrier.dstAccessMask = vk::AccessFlagBits::eTransferWrite;
        break;

    case vk::ImageLayout::eTransferSrcOptimal:
        // Image will be used as a transfer source
        // Make sure any reads from the image have been finished
        imageMemoryBarrier.dstAccessMask = vk::AccessFlagBits::eTransferRead;
        break;

    case vk::ImageLayout::eColorAttachmentOptimal:
        // Image will be used as a color attachment
        // Make sure any writes to the color buffer have been finished
        imageMemoryBarrier.dstAccessMask = vk::AccessFlagBits::eColorAttachmentWrite;
        break;

    case vk::ImageLayout::eDepthStencilAttachmentOptimal:
        // Image layout will be used as a depth/stencil attachment
        // Make sure any writes to depth/stencil buffer have been finished
        imageMemoryBarrier.dstAccessMask = imageMemoryBarrier.dstAccessMask | vk::AccessFlagBits::eDepthStencilAttachmentWrite;
        break;

    case vk::ImageLayout::eShaderReadOnlyOptimal:
        // Image will be read in a shader (sampler, input attachment)
        // Make sure any writes to the image have been finished
        if (imageMemoryBarrier.srcAccessMask == static_cast<vk::AccessFlags>(0))
        {
            imageMemoryBarrier.srcAccessMask = vk::AccessFlagBits::eHostWrite | vk::AccessFlagBits::eTransferWrite;
        }
        imageMemoryBarrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;
        break;
    default:
        // Other source layouts aren't handled (yet)
        break;
    }

    // Put barrier inside setup command buffer
    cmdbuffer.pipelineBarrier(
        
        srcStageMask,
        dstStageMask,
        {},
        0, nullptr,
        0, nullptr,
        1, &imageMemoryBarrier);
}

void te::vkh::VulkanDevice::setImageLayout(
    vk::CommandBuffer cmdbuffer,
    vk::Image image,
    vk::ImageAspectFlags aspectMask,
    vk::ImageLayout oldImageLayout,
    vk::ImageLayout newImageLayout,
    vk::PipelineStageFlags srcStageMask,
    vk::PipelineStageFlags dstStageMask)
{
    vk::ImageSubresourceRange subresourceRange = {};
    subresourceRange.aspectMask = aspectMask;
    subresourceRange.baseMipLevel = 0;
    subresourceRange.levelCount = 1;
    subresourceRange.layerCount = 1;
    setImageLayout(cmdbuffer, image, oldImageLayout, newImageLayout, subresourceRange, srcStageMask, dstStageMask);
}

void te::vkh::VulkanDevice::endSingleTimeCommands(
    vk::CommandBuffer commandBuffer,
    vk::Queue graphicsQueue,
    vk::CommandPool commandPool
)
{
    commandBuffer.end();
   
    vk::SubmitInfo submitInfo{};
    
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    graphicsQueue.submit(1, &submitInfo, nullptr);
  
    graphicsQueue.waitIdle();  
   
    logicalDevice.freeCommandBuffers(commandPool, 1, &commandBuffer);
}

vk::ShaderModule te::vkh::VulkanDevice::createShaderModule(const std::vector<char>& code)
{
    vk::ShaderModuleCreateInfo createInfo{};

    createInfo.codeSize = code.size();
    createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

    vk::ShaderModule shaderModule;
    if (logicalDevice.createShaderModule(&createInfo, nullptr, &shaderModule) != vk::Result::eSuccess) {
        throw std::runtime_error("failed to create shader module!");
    }

    return shaderModule;
}

vk::CommandBuffer te::vkh::VulkanDevice::beginSingleTimeCommands(vk::CommandPool commandPool)
{
    vk::CommandBufferAllocateInfo allocInfo{};
    allocInfo.level = vk::CommandBufferLevel::ePrimary;
    allocInfo.commandPool = commandPool;
    allocInfo.commandBufferCount = 1;

    vk::CommandBuffer commandBuffer;
    
    logicalDevice.allocateCommandBuffers(&allocInfo, &commandBuffer);

    vk::CommandBufferBeginInfo beginInfo{};
    beginInfo.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit;

    commandBuffer.begin(&beginInfo);

    return commandBuffer;
}


void te::vkh::VulkanDevice::createVertexBuffer(
    std::vector<te::Vertex> vertices,
    vk::Buffer& vertexBuffer,
    vk::DeviceMemory& vertexBufferMemory,
    vk::CommandPool commandPool,
    vk::Queue graphicsQueue
)
{
    vk::DeviceSize bufferSize = sizeof(vertices[0]) * vertices.size();

    vk::Buffer stagingBuffer;
    vk::DeviceMemory stagingBufferMemory;

    createBuffer(
        bufferSize,
        vk::BufferUsageFlagBits::eTransferSrc,
        vk::MemoryPropertyFlagBits::eHostVisible |
        vk::MemoryPropertyFlagBits::eHostCoherent,
        stagingBuffer, stagingBufferMemory
    );

    void* data;
   
    logicalDevice.mapMemory(stagingBufferMemory, 0, bufferSize, {}, &data);
   
    memcpy(data, vertices.data(), (size_t)bufferSize);

    logicalDevice.unmapMemory(stagingBufferMemory);


    createBuffer(
        bufferSize,
        vk::BufferUsageFlagBits::eTransferDst |
        vk::BufferUsageFlagBits::eVertexBuffer,
        vk::MemoryPropertyFlagBits::eDeviceLocal,
        vertexBuffer, vertexBufferMemory);

    te::vkh::VulkanDevice::copyBuffer(stagingBuffer, vertexBuffer, bufferSize, graphicsQueue, commandPool);

    logicalDevice.destroyBuffer(stagingBuffer, nullptr);
    logicalDevice.freeMemory(stagingBufferMemory, nullptr);
}

void te::vkh::VulkanDevice::createIndexBuffer(
    std::vector<uint32_t> indices,
    vk::Buffer& indexBuffer, 
    vk::DeviceMemory& indexBufferMemory,
    vk::CommandPool commandPool,
    vk::Queue graphicsQueue
)
{
    vk::DeviceSize bufferSize = sizeof(indices[0]) * indices.size();

    vk::Buffer stagingBuffer;
    vk::DeviceMemory stagingBufferMemory;
    createBuffer(
        bufferSize,
        vk::BufferUsageFlagBits::eTransferSrc,
        vk::MemoryPropertyFlagBits::eHostVisible |
        vk::MemoryPropertyFlagBits::eHostCoherent,
        stagingBuffer, stagingBufferMemory   
    );

    void* data;
   
    logicalDevice.mapMemory(stagingBufferMemory, 0, bufferSize, {}, &data);
    memcpy(data, indices.data(), (size_t)bufferSize);
    logicalDevice.unmapMemory(stagingBufferMemory);

    createBuffer(
        bufferSize,
        vk::BufferUsageFlagBits::eTransferDst |
        vk::BufferUsageFlagBits::eIndexBuffer,
        vk::MemoryPropertyFlagBits::eDeviceLocal,
        indexBuffer, indexBufferMemory
    );

    copyBuffer(stagingBuffer, indexBuffer, bufferSize, graphicsQueue, commandPool);

    logicalDevice.destroyBuffer(stagingBuffer, nullptr);
    logicalDevice.freeMemory(stagingBufferMemory, nullptr);
}

vk::ImageView  te::vkh::VulkanDevice::createImageView(vk::Image image, vk::Format format, vk::ImageAspectFlags aspectFlags, uint32_t mipLevels)
{
    vk::ImageViewCreateInfo viewInfo{};

    viewInfo.image = image;
    viewInfo.viewType = vk::ImageViewType::e2D;
    viewInfo.format = format;
    viewInfo.subresourceRange.aspectMask = aspectFlags;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.levelCount = mipLevels;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount = 1;

    vk::ImageView imageView;
    if (logicalDevice.createImageView(&viewInfo, nullptr, &imageView) != vk::Result::eSuccess) {
        throw std::runtime_error("failed to create texture image view!");
    }

    return imageView;
}

te::vkh::QueueFamilyIndices te::vkh::findQueueFamilies(vk::SurfaceKHR surface, vk::PhysicalDevice physicalDevice)
{
    te::vkh::QueueFamilyIndices indices;


    std::vector<vk::QueueFamilyProperties> queueFamilies = physicalDevice.getQueueFamilyProperties();
    int i = 0;
    for (const auto& queueFamily : queueFamilies) {
        if (queueFamily.queueFlags & vk::QueueFlagBits::eGraphics) {
            indices.graphicsFamily = i;
        }

        if (queueFamily.queueFlags & vk::QueueFlagBits::eTransfer)
        {
            indices.transferFamily = i;
        }

        vk::Bool32 presentSupport = false;
        physicalDevice.getSurfaceSupportKHR(i, surface, &presentSupport);

        if (presentSupport) {
            indices.presentFamily = i;
        }

        if (indices.isComplete()) {
            break;
        }

        i++;
    }

    return indices;
}

 te::vkh::SwapChainSupportDetails te::vkh::querySwapChainSupport(
    vk::SurfaceKHR surface, vk::PhysicalDevice physicalDevice)
{
     te::vkh::SwapChainSupportDetails details = {};

    physicalDevice.getSurfaceCapabilitiesKHR(surface, &details.capabilities);

    details.formats = physicalDevice.getSurfaceFormatsKHR(surface);

    details.presentModes = physicalDevice.getSurfacePresentModesKHR(surface);

    return details;
}

void te::vkh::VulkanDevice::createBuffer(
    vk::DeviceSize size,
    vk::BufferUsageFlags usage,
    vk::MemoryPropertyFlags properties,
    vk::Buffer& buffer,
    vk::DeviceMemory& bufferMemory)
{
    vk::BufferCreateInfo bufferInfo{};
    
    bufferInfo.size = size;
    bufferInfo.usage = usage;
    bufferInfo.sharingMode = vk::SharingMode::eExclusive;

    if (logicalDevice.createBuffer(&bufferInfo, nullptr, &buffer) != vk::Result::eSuccess) {
        throw std::runtime_error("failed to create buffer!");
    }

    vk::MemoryRequirements memRequirements;
    logicalDevice.getBufferMemoryRequirements(buffer, &memRequirements);

    vk::MemoryAllocateInfo allocInfo{};
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = te::vkh::VulkanDevice::findMemoryType(memRequirements.memoryTypeBits, properties);

    if (logicalDevice.allocateMemory(&allocInfo, nullptr, &bufferMemory) != vk::Result::eSuccess) {
        throw std::runtime_error("failed to allocate buffer memory!");
    }

   
    logicalDevice.bindBufferMemory(buffer, bufferMemory, 0);
}

 uint32_t te::vkh::VulkanDevice::findMemoryType(uint32_t typeFilter, vk::MemoryPropertyFlags properties)
{
    vk::PhysicalDeviceMemoryProperties memProperties;

    physicalDevice.getMemoryProperties(&memProperties);

    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
        if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
            return i;
        }
    }

    throw std::runtime_error("failed to find suitable memory type!");
}

 void te::vkh::VulkanDevice::copyBufferToImage(vk::Buffer buffer, vk::Image image, uint32_t width, uint32_t height, vk::CommandPool commandPool, vk::Queue graphicsQueue)
 {
     vk::CommandBuffer commandBuffer = te::vkh::VulkanDevice::beginSingleTimeCommands(commandPool);

     vk::BufferImageCopy region{};
     region.bufferOffset = 0;
     region.bufferRowLength = 0;
     region.bufferImageHeight = 0;
     region.imageSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
     region.imageSubresource.mipLevel = 0;
     region.imageSubresource.baseArrayLayer = 0;
     region.imageSubresource.layerCount = 1;
     region.imageOffset = vk::Offset3D{ 0, 0, 0 };
     region.imageExtent = vk::Extent3D{
         width,
         height,
         1
     };
    
     commandBuffer.copyBufferToImage(buffer, image, vk::ImageLayout::eTransferDstOptimal, 1, &region);

     te::vkh::VulkanDevice::endSingleTimeCommands(commandBuffer, graphicsQueue, commandPool);
 }
