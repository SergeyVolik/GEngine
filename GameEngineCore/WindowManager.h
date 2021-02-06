
#ifndef GE_WINDOW_SYSTEM

#define GE_WINDOW_SYSTEM

#include "EngineSystem.h"


#include <GLFW/glfw3.h>

namespace te
{

    class WindowManager : public te::EngineSystem<WindowManager>
    {

    public:

        static void terminate() {
            if (WindowManager::isInitialized())
            {
#ifndef TIME_ENGINE_EDITOR
                    glfwTerminate();
    #endif // TIME_ENGINE_EDITOR

              
                delete instance;
            }
        };
        static void initialize() {
            if (!WindowManager::isInitialized())
            {
#ifndef TIME_ENGINE_EDITOR
                glfwInit();
                glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
#endif // TIME_ENGINE_EDITOR
                instance = new WindowManager();
                
            }
        };



    };
}
#endif // !GE_WINDOW_SYSTEM

