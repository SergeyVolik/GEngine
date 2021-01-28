//#define GLFW_INCLUDE_VULKAN
//#include <GLFW/glfw3.h>

//#define GLM_FORCE_RADIANS
//#define GLM_FORCE_DEPTH_ZERO_TO_ONE
//#define GLM_ENABLE_EXPERIMENTAL
//#include <glm/glm.hpp>
//#include <glm/gtc/matrix_transform.hpp>
//#include <glm/gtx/hash.hpp>

#define STB_IMAGE_IMPLEMENTATION

//#include <stb_image.h>

#define TINYOBJLOADER_IMPLEMENTATION
//#include <tiny_obj_loader.h>


#include "VulkanRenderManager.h"
#include "Window.h"
#include "InputManager.h"
#include "SceneManager.h"
#include "Time.h"

class GEngine {

public:
    void run() {
        initSystems();
       
        mainLoop();
        cleanup();
    }

private:
   
    te::Window* window;

    void initSystems() {


        te::WindowManager::initialize();
        window = new te::Window(WIDTH, HEIGHT, "Vulkan");
        te::InputManager::initialize(window->getWindow());
        te::VulkanRenderManager::initialize(window);
        te::SceneManager::initialize();
        te::SceneManager::getInstance()->createScene("demo");

    }


    void mainLoop() {

        while (!window->windowShouldClose()) {

            te::Time::calcDelta();
            window->setTitle(std::to_string(te::Time::getDelta()).c_str());
           
            te::InputManager::pollEvents();

            te::VulkanRenderManager::getInstance()->drawFrame();

            if (te::InputManager::getKey(te::KeyCode::KEY_LEFT_ALT) && te::InputManager::getKeyDown(te::KeyCode::KEY_ENTER))
            {
                window->setFullScreen(!window->isFullScreen());
            }

        }

        // ожидание конца отрисовки об'ектов
        vkDeviceWaitIdle(te::VulkanRenderManager::getInstance()->device);
    }


    void cleanup() {

        te::VulkanRenderManager::terminate();

        window->terminateWindow();
        te::WindowManager::getInstance()->terminate();


    }



};

int main() {


    GEngine app;

    try {
        app.run();
    }
    catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }


    return EXIT_SUCCESS;
}
