#ifndef GENGINE_CORE
#define GENGINE_CORE

class VkInstance;
class GLFWWindow;

namespace te
{
    class Window;
  

    enum class EngineState { Paused, Stoped, Playing };

    class GEngine {

    public:
        void initSystems();
        void mainLoop();
        void cleanup();
        void drawFrame();
        void setWindow(GLFWWindow* wnd);
        EngineState state;
    private:

        te::Window* window;
        
       

    };
}



#endif // !GENGINE_CORE

