#include "VulkanHelper.h"
#include <stdexcept>
#include "Vertex.h"

void te::vkh::VulkanHelper::copyBuffer(
    vk::Buffer srcBuffer,
    vk::Buffer dstBuffer,
    vk::DeviceSize size,
    vk::Queue graphicsQueue,
    vk::CommandPool commandPool,
    vk::Device device
)
{
    vk::CommandBuffer commandBuffer = beginSingleTimeCommands(commandPool, device);

    vk::BufferCopy copyRegion{};
    copyRegion.size = size;
  
    commandBuffer.copyBuffer(srcBuffer, dstBuffer, 1, &copyRegion);
 
    endSingleTimeCommands(commandBuffer, graphicsQueue, commandPool, device);
}

void te::vkh::VulkanHelper::endSingleTimeCommands(
    vk::CommandBuffer commandBuffer,
    vk::Queue graphicsQueue,
    vk::CommandPool commandPool,
    vk::Device device
)
{
    commandBuffer.end();
   
    vk::SubmitInfo submitInfo{};
    
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    graphicsQueue.submit(1, &submitInfo, nullptr);
  
    graphicsQueue.waitIdle();  
   
    device.freeCommandBuffers(commandPool, 1, &commandBuffer);
}

vk::CommandBuffer te::vkh::VulkanHelper::beginSingleTimeCommands(vk::CommandPool commandPool, vk::Device device)
{
    vk::CommandBufferAllocateInfo allocInfo{};
    allocInfo.level = vk::CommandBufferLevel::ePrimary;
    allocInfo.commandPool = commandPool;
    allocInfo.commandBufferCount = 1;

    vk::CommandBuffer commandBuffer;
    
    device.allocateCommandBuffers(&allocInfo, &commandBuffer);

    vk::CommandBufferBeginInfo beginInfo{};
    beginInfo.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit;

    commandBuffer.begin(&beginInfo);

    return commandBuffer;
}

std::vector<vk::PhysicalDevice> te::vkh::VulkanHelper::getPhysicalDevices(vk::Instance instance)
{

    std::vector<vk::PhysicalDevice> devices = instance.enumeratePhysicalDevices();

    if (devices.size() == 0) {
        throw std::runtime_error("failed to find GPUs with Vulkan support!");
    }

    return devices;
}

void te::vkh::VulkanHelper::createVertexBuffer(
    std::vector<te::Vertex> vertices,
    vk::Buffer& vertexBuffer,
    vk::DeviceMemory& vertexBufferMemory,
    vk::CommandPool commandPool,
    vk::Queue graphicsQueue,
    vk::PhysicalDevice phisycalDevice,
    vk::Device device
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
        stagingBuffer, stagingBufferMemory,
        phisycalDevice, device
    );

    void* data;
   
    device.mapMemory(stagingBufferMemory, 0, bufferSize, {}, &data);
   
    memcpy(data, vertices.data(), (size_t)bufferSize);

    device.unmapMemory(stagingBufferMemory);


    createBuffer(
        bufferSize,
        vk::BufferUsageFlagBits::eTransferDst |
        vk::BufferUsageFlagBits::eVertexBuffer,
        vk::MemoryPropertyFlagBits::eDeviceLocal,
        vertexBuffer, vertexBufferMemory, phisycalDevice, device);

    te::vkh::VulkanHelper::copyBuffer(stagingBuffer, vertexBuffer, bufferSize, graphicsQueue, commandPool, device);

    vkDestroyBuffer(device, stagingBuffer, nullptr);
    vkFreeMemory(device, stagingBufferMemory, nullptr);
}

void te::vkh::VulkanHelper::createIndexBuffer(
    std::vector<uint32_t> indices,
    vk::Buffer& indexBuffer, 
    vk::DeviceMemory& indexBufferMemory,
    vk::CommandPool commandPool,
    vk::Queue graphicsQueue,
    vk::PhysicalDevice phisycalDevice,
    vk::Device device
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
        stagingBuffer, stagingBufferMemory,
        phisycalDevice, device
    );

    void* data;
    vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, indices.data(), (size_t)bufferSize);
    vkUnmapMemory(device, stagingBufferMemory);

    createBuffer(
        bufferSize,
        vk::BufferUsageFlagBits::eTransferDst |
        vk::BufferUsageFlagBits::eIndexBuffer,
        vk::MemoryPropertyFlagBits::eDeviceLocal,
        indexBuffer, indexBufferMemory,
        phisycalDevice, device
    );

    te::vkh::VulkanHelper::copyBuffer(stagingBuffer, indexBuffer, bufferSize, graphicsQueue, commandPool, device);

    vkDestroyBuffer(device, stagingBuffer, nullptr);
    vkFreeMemory(device, stagingBufferMemory, nullptr);
}




void te::vkh::VulkanHelper::createBuffer(
    vk::DeviceSize size,
    vk::BufferUsageFlags usage,
    vk::MemoryPropertyFlags properties,
    vk::Buffer& buffer,
    vk::DeviceMemory& bufferMemory,
    vk::PhysicalDevice phisycalDevice,
    vk::Device device)
{
    vk::BufferCreateInfo bufferInfo{};
    
    bufferInfo.size = size;
    bufferInfo.usage = usage;
    bufferInfo.sharingMode = vk::SharingMode::eExclusive;

    if (device.createBuffer(&bufferInfo, nullptr, &buffer) != vk::Result::eSuccess) {
        throw std::runtime_error("failed to create buffer!");
    }

    vk::MemoryRequirements memRequirements;
    device.getBufferMemoryRequirements(buffer, &memRequirements);

    vk::MemoryAllocateInfo allocInfo{};
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = te::vkh::VulkanHelper::findMemoryType(memRequirements.memoryTypeBits, properties, phisycalDevice);

    if (device.allocateMemory(&allocInfo, nullptr, &bufferMemory) != vk::Result::eSuccess) {
        throw std::runtime_error("failed to allocate buffer memory!");
    }

    vkBindBufferMemory(device, buffer, bufferMemory, 0);
}

 uint32_t te::vkh::VulkanHelper::findMemoryType(uint32_t typeFilter, vk::MemoryPropertyFlags properties, vk::PhysicalDevice physicalDevice)
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

 void te::vkh::VulkanHelper::copyBufferToImage(vk::Buffer buffer, vk::Image image, uint32_t width, uint32_t height, vk::CommandPool commandPool, vk::Queue graphicsQueue, vk::Device device)
 {
     vk::CommandBuffer commandBuffer = te::vkh::VulkanHelper::beginSingleTimeCommands(commandPool, device);

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

     te::vkh::VulkanHelper::endSingleTimeCommands(commandBuffer, graphicsQueue, commandPool, device);
 }
