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
#include "Scene.h"
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

        //te::SceneManager::getInstance()->getCurrentScene()->awakeAllEntities();
        //te::SceneManager::getInstance()->getCurrentScene()->startAllEntities();

        while (!window->windowShouldClose()) {

            te::Time::calcDelta();
            std::string fpsData = std::to_string(static_cast<int>(te::Time::getDelta() * 1000)) + " ms | " + std::to_string(static_cast<int>(1.0f / te::Time::getDelta())) + " fps";
            window->setTitle(fpsData.c_str());
           
            te::InputManager::pollEvents();

            
            te::VulkanRenderManager::getInstance()->drawFrame();
           // te::SceneManager::getInstance()->getCurrentScene()->updateAllEntities();
           
            if (te::InputManager::getKey(te::KeyCode::KEY_LEFT_ALT) && te::InputManager::getKeyDown(te::KeyCode::KEY_ENTER))
            {
                window->setFullScreen(!window->isFullScreen());
            }

        }

        // �������� ����� ��������� ��'�����
        vkDeviceWaitIdle(te::VulkanRenderManager::getInstance()->device);
    }


    void cleanup() {

        te::SceneManager::getInstance()->terminate();
        te::VulkanRenderManager::getInstance()->terminate();
       

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
