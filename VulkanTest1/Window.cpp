#include "Window.h"



te::Window::Window(uint32_t width, uint32_t height, const char* title)
{
    if (!WindowManager::isInitialized())
    {
        throw std::runtime_error("glfw api not initialized. please use Window::initializeSystem() for init and Window::terminateSystem() for terminate api");
    }

    window = glfwCreateWindow(width, height, "Vulkan", nullptr, nullptr);
    glfwSetWindowUserPointer(window, this);
    glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);
   
    
}


void te::Window::setFullScreen(bool fullScreen)
{
    
    std::cout << fullScreen;
    if (fullScreen)
    {
        const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());

        glfwMaximizeWindow(window);
        glfwSetWindowMonitor(window, glfwGetPrimaryMonitor(), 0, 0, mode->width, mode->height, mode->refreshRate);

    }
    else {

        const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
       
        glfwSetWindowMonitor(window, nullptr, mode->width/2, mode->height / 2,640, 480, mode->refreshRate);
        glfwRestoreWindow(window);

    };
}

void te::Window::setWindowSize(int width, int height)
{
    glfwSetWindowSize(window, width, height);
}

te::Window::~Window()
{
    glfwDestroyWindow(window);
}

bool te::Window::isFullScreen()
{

    return glfwGetWindowAttrib(window, GLFW_MAXIMIZED) == true;
}

bool te::Window::windowResized()
{
    return framebufferResized;
}

bool te::Window::windowShouldClose()
{
    return glfwWindowShouldClose(window);
}

void te::Window::closeWindow()
{
    glfwSetWindowShouldClose(window, true);
}



void te::Window::terminateWindow()
{
    delete this;
}


void te::Window::getFramebufferSize(int* width, int* height)
{
    glfwGetFramebufferSize(window, width, height);
}

void te::Window::windowResizing()
{
    int width = 0, height = 0;
    glfwGetFramebufferSize(window, &width, &height);
    while (width == 0 || height == 0) {
        glfwGetFramebufferSize(window, &width, &height);
        glfwWaitEvents();
    }
}

void te::Window::createSurface(VkInstance instance, VkSurfaceKHR* pSurface, VkAllocationCallbacks* pAllocator)
{
    VkResult result = glfwCreateWindowSurface(instance, window, pAllocator, pSurface);
    if (result != VK_SUCCESS) {
        throw std::runtime_error("failed to create window surface!");
       
    }

   
}
