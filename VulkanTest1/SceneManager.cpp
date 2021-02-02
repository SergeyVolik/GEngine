#include "SceneManager.h"
#include "Scene.h"
#include "Entity.h"
#include "Camera.h"
#include "Renderer.h"

#include <typeinfo>
#include <iostream>

te::Scene* te::SceneManager::createScene(const char* name) {

    auto scene = new te::Scene(name);
   
    _scenes.push_back(scene);
    return scene;
}

void te::SceneManager::changeScene(int index) {
    int curIdx = 0;
    for (const auto& scene : instance->_scenes)
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
        for (auto& scene : instance->_scenes)
            delete scene;

        delete instance;
    }
}
void  te::SceneManager::initialize() {
    if (!te::SceneManager::isInitialized())
    {

        instance = new te::SceneManager();

    }
}