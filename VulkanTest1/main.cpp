#define STB_IMAGE_IMPLEMENTATION
#define TINYOBJLOADER_IMPLEMENTATION



#include "VulkanRenderManager.h"
#include "WindowManager.h"
#include "Window.h"
#include "InputManager.h"
#include "SceneManager.h"
#include "Time.h"
#include "AssetsLoader.h"
#include "Scene.h"
#include "Entity.h";
#include "Camera.h"
#include "Renderer.h"
#include "AssetsLoader.h"
#include "SimplerModelTransformations.h"


#define VMA_IMPLEMENTATION
#include "vk_mem_alloc.h"

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
        te::AssetsLoader::initialize();

        te::VulkanRenderManager::initialize(window);
       
        te::SceneManager::initialize();
        te::Scene* currentScene = te::SceneManager::getInstance()->createScene("demo");

       /* te::Entity* cameraHolder = new te::Entity();
        te::Camera* camComp = new te::Camera();
        cameraHolder->addComponent(camComp);


        te::Entity* meshHolder = new te::Entity();
        te::Renderer* rendererComp = new te::Renderer();
       
        rendererComp->setMesh(te::AssetsLoader::getInstance()->loadModel())
        meshHolder->addComponent(rendererComp);

        te::SimplerModelTransformations* smtComp = new te::SimplerModelTransformations();
        meshHolder->addComponent(smtComp);


        currentScene->addEntity(cameraHolder);
        currentScene->setMainCamera(camComp);

        currentScene->addEntity(meshHolder);*/

        
       
    }


    void mainLoop() {

        //te::SceneManager::getInstance()->getCurrentScene()->awakeAllEntities();
        //te::SceneManager::getInstance()->getCurrentScene()->startAllEntities();

        while (!window->windowShouldClose()) {

            te::Time::calcDelta();
            std::string fpsData = std::to_string(static_cast<int>(te::Time::getDelta() * 1000)) 
                + " ms | " + std::to_string(static_cast<int>(1.0f / te::Time::getDelta())) + " fps";
            window->setTitle(fpsData.c_str());
           
            te::InputManager::pollEvents();

            
            te::VulkanRenderManager::getInstance()->drawFrame();
           // te::SceneManager::getInstance()->getCurrentScene()->updateAllEntities();
           
            if (te::InputManager::getKey(te::KeyCode::KEY_LEFT_ALT) && te::InputManager::getKeyDown(te::KeyCode::KEY_ENTER))
            {
                window->setFullScreen(!window->isFullScreen());
            }

        }

        // ожидание конца отрисовки об'ектов
        vkDeviceWaitIdle(te::VulkanRenderManager::getInstance()->device);
    }


    void cleanup() {

        te::SceneManager::getInstance()->terminate();
        te::VulkanRenderManager::getInstance()->terminate();
       

        window->terminateWindow();
        te::WindowManager::getInstance()->terminate();

        te::AssetsLoader::terminate();


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
