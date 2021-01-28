#ifndef GE_WINDOW
#define GE_WINDOW

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include "WindowManager.h"
#include <iostream>

#include <stdexcept>

namespace te
{

	class Window
	{

	private:

		GLFWwindow* window;
		bool framebufferResized = false;

		static void framebufferResizeCallback(GLFWwindow* window, int width, int height) {
			auto app = reinterpret_cast<Window*>(glfwGetWindowUserPointer(window));
			app->framebufferResized = true;
		};


	public:

		Window(uint32_t width, uint32_t height, const char* title);

		GLFWwindow* getWindow()
		{
			return window;
		}

		bool isFullScreen();
		bool windowResized();
		void windowResizedClear() { framebufferResized = false; }
		bool windowShouldClose();
		void closeWindow();

		void terminateWindow();

		void getFramebufferSize(int* width, int* height);
		void windowResizing();
		void createSurface(VkInstance instance, VkSurfaceKHR* pSurface, VkAllocationCallbacks* pAllocator);
		void setFullScreen(bool);
		void setWindowSize(int width, int height);
		inline void setTitle(const char* title) {
			glfwSetWindowTitle(window, title);
		}
		~Window();
	};
}

#endif // GE_WINDOW