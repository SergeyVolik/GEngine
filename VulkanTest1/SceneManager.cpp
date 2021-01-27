#include "SceneManager.h"
#include "Scene.h"
#include "Entity.h"
#include "Camera.h"
#include "Renderer.h"

#include <typeinfo>
#include <iostream>

te::Scene* te::SceneManager::createScene(const char* name) {

    auto scene = new te::Scene(name);
    auto camera = new te::Entity();
    auto cameraComp = new te::Camera();
    camera->addComponent(cameraComp);
  

    scene->addEntity(camera);

    auto renderer = new te::Entity();
    auto meshRenderer = new te::Renderer();
    renderer->addComponent(meshRenderer);

    scene->addEntity(renderer);

    _scenes.push_back(scene);
    return scene;
}

void te::SceneManager::changeScene(int index) {
    int curIdx = 0;
    for (const auto& scene : _instance->_scenes)
    {
        if (curIdx == index)
        {
            _current = scene;
            return;
        }
        curIdx++;
    }


}

void te::SceneManager::terminate() {
    if (te::SceneManager::isInitialized())
    {
        for (auto& scene : _instance->_scenes)
            delete scene;

        delete _instance;
    }
}
void  te::SceneManager::initialize() {
    if (!te::SceneManager::isInitialized())
    {

        _instance = new te::SceneManager();

    }
}