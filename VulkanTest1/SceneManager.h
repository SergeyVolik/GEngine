#ifndef GE_SCENE_MANAGER
#define GE_SCENE_MANAGER

#include "Singleton.h"
#include <list>

namespace te
{
    class Scene;

    class SceneManager : public te::Singleton<SceneManager>
    {
    private:
        te::Scene* _current;
        std::list<te::Scene*> _scenes;

    public:
        te::Scene* getCurrentScene() {
            return _current;
        }
        te::Scene* createScene(const char* name);
        void changeScene(int index);

        
        static void terminate();
        static void initialize();

    };
}
#endif // !GE_SCENE_MANAGER
