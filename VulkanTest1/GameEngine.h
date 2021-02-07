
#ifndef GAME_ENGINE
#define GAME_ENGINE

#include <string>

namespace te
{
    class Window;
    class GEngine {



    public:
        void run();
        static void exitFatal(const std::string& message, int32_t exitCode);
    private:

        te::Window* window;

        void initSystems();


        void mainLoop();


        void cleanup();

      



    };
}
#endif // !1GAME_ENGINE
