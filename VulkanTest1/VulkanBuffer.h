//#ifndef GVULKAN_BUFFER
//#define GVULKAN_BUFFER
//#include <vulkan/vulkan.hpp>
//
//namespace te
//{
//	namespace vkh
//	{
//		/**
//		* @brief Encapsulates access to a Vulkan buffer backed up by device memory
//		* @note To be filled by an external source like the VulkanDevice
//		*/
//		struct Buffer
//		{
//			vk::Device device;
//			vk::Buffer buffer = nullptr;
//			vk::DeviceMemory memory = nullptr;
//			vk::DescriptorBufferInfo descriptor;
//			vk::DeviceSize size = 0;
//			vk::DeviceSize alignment = 0;
//			void* mapped = nullptr;
//			/** @brief Usage flags to be filled by external source at buffer creation (to query at some later point) */
//			vk::BufferUsageFlags usageFlags;
//			/** @brief Memory property flags to be filled by external source at buffer creation (to query at some later point) */
//			vk::MemoryPropertyFlags memoryPropertyFlags;
//			vk::Result map(vk::DeviceSize size = VK_WHOLE_SIZE, vk::DeviceSize offset = 0);
//			void unmap();
//			void bind(vk::DeviceSize offset = 0);
//			void setupDescriptor(vk::DeviceSize size = VK_WHOLE_SIZE, vk::DeviceSize offset = 0);
//			void copyTo(void* data, vk::DeviceSize size);
//			vk::Result flush(vk::DeviceSize size = VK_WHOLE_SIZE, vk::DeviceSize offset = 0);
//			vk::Result invalidate(vk::DeviceSize size = VK_WHOLE_SIZE, vk::DeviceSize offset = 0);
//			void destroy();
//		};
//	}
//}
//#endif // !GVULKAN_BUFFER
