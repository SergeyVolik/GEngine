//#ifndef GVULKAN_VALIDATE
//#define GVULKAN_VALIDATE
//
//#include "vulkan/vulkan.hpp"
//
//#include <math.h>
//#include <stdlib.h>
//#include <string>
//#include <cstring>
//#include <fstream>
//#include <assert.h>
//#include <stdio.h>
//#include <vector>
//#include <iostream>
//#include <stdexcept>
//#include <fstream>
//#if defined(_WIN32)
////#include <windows.h>
////#include <fcntl.h>
////#include <io.h>
//#endif
//
//// Custom define for better code readability
//#define VK_FLAGS_NONE 0
//// Default fence timeout in nanoseconds
//#define DEFAULT_FENCE_TIMEOUT 100000000000
//
//#define VK_CHECK_RESULT(f)																				\
//{																										\
//	vk::Result res = (f);																					\
//	if (res != vk::Result::eSuccess)																				\
//	{																									\
//		std::cout << "Fatal : VkResult is \"" << te::vkh::errorString(res) << "\" in " << __FILE__ << " at line " << __LINE__ << "\n"; \
//		assert(res == vk::Result::eSuccess);																		\
//	}																									\
//}
//
//namespace te
//{
//	namespace vkh
//	{
//		/** @brief Returns an error code as a string */
//		std::string errorString(vk::Result errorCode);
//	}
//}
//
//#endif // !GVULKAN_VALIDATE
