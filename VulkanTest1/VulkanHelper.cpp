#include "VulkanHelper.h"
#include <stdexcept>
#include "Vertex.h"

void te::VulkanHelper::copyBuffer(
    VkBuffer srcBuffer,
    VkBuffer dstBuffer,
    VkDeviceSize size,
    VkQueue graphicsQueue,
    VkCommandPool commandPool,
    VkDevice device
)
{
    VkCommandBuffer commandBuffer = beginSingleTimeCommands(commandPool, device);

    VkBufferCopy copyRegion{};
    copyRegion.size = size;
    vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

    endSingleTimeCommands(commandBuffer, graphicsQueue, commandPool, device);
}

void te::VulkanHelper::endSingleTimeCommands(
    VkCommandBuffer commandBuffer,
    VkQueue graphicsQueue,
    VkCommandPool commandPool,
    VkDevice device
)
{
    vkEndCommandBuffer(commandBuffer);

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    vkQueueSubmit(graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(graphicsQueue);

    vkFreeCommandBuffers(device, commandPool, 1, &commandBuffer);
}

VkCommandBuffer te::VulkanHelper::beginSingleTimeCommands(VkCommandPool commandPool, VkDevice device)
{
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = commandPool;
    allocInfo.commandBufferCount = 1;

    VkCommandBuffer commandBuffer;
    vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer);

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(commandBuffer, &beginInfo);

    return commandBuffer;
}

std::vector<VkPhysicalDevice> te::VulkanHelper::getPhysicalDevices(VkInstance instance)
{
    uint32_t deviceCount = 0;

    vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);

    if (deviceCount == 0) {
        throw std::runtime_error("failed to find GPUs with Vulkan support!");
    }

    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

    return devices;
}
//void te::VulkanHelper::createVertexBuffer(std::vector<te::Vertex> vertices, VkBuffer& vertexBuffer, VkDeviceMemory& vertexBufferMemory)
//{
//    VkDeviceSize bufferSize = sizeof(vertices[0]) * vertices.size();
//
//    VkBuffer stagingBuffer;
//    VkDeviceMemory stagingBufferMemory;
//
//    createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);
//
//    void* data;
//    vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &data);
//    memcpy(data, vertices.data(), (size_t)bufferSize);
//    vkUnmapMemory(device, stagingBufferMemory);
//
//    createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, vertexBuffer, vertexBufferMemory);
//
//    te::VulkanHelper::copyBuffer(stagingBuffer, vertexBuffer, bufferSize, graphicsQueue, commandPool, device);
//    //copyBuffer(stagingBuffer, vertexBuffer, bufferSize);
//
//    vkDestroyBuffer(device, stagingBuffer, nullptr);
//    vkFreeMemory(device, stagingBufferMemory, nullptr);
//}
