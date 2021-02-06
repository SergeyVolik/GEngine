#pragma once

namespace te
{
    class Window;

    enum class EngineState { Paused, Stoped, Playing };

    class GEngine {

    public:
        void run() {
            initSystems();
            mainLoop();
            cleanup();
        }

    private:

        te::Window* window;
        void initSystems();
        void mainLoop();
        void cleanup();

    };
}

