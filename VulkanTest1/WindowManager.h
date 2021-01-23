
#ifndef GE_WINDOW_SYSTEM

#define GE_WINDOW_SYSTEM

#include "EngineSystem.h"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

namespace te
{

    class WindowManager : public te::EngineSystem<WindowManager>
    {

    public:

        static void terminate() {
            if (WindowManager::isInitialized())
            {
                glfwTerminate();
                delete _instance;
            }
        };
        static void initialize() {
            if (!WindowManager::isInitialized())
            {
                glfwInit();
                _instance = new WindowManager();
                glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
            }
        };



    };
}
#endif // !GE_WINDOW_SYSTEM

