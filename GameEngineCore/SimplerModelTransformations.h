#ifndef GSIMPLE_MODEL_TRANS
#define GSIMPLE_MODEL_TRANS
#include "Component.h"
#include "InputManager.h"
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include "Entity.h"
#include "Transform.h"
namespace te
{

	class SimplerModelTransformations : public te::Component
	{
	public:
		SimplerModelTransformations() {}
		SimplerModelTransformations(Entity* e) : Component(e) {}

		void onUpdate() override {
            auto gTransform = getEntity()->getTransform();
            auto oldPos = gTransform->getPosition();

            if (InputManager::getKey(KeyCode::KEY_1))
            {
                gTransform->setPosition(glm::vec3(oldPos.x, oldPos.y, oldPos.z + te::Time::getDelta() * 20));
            }
            if (InputManager::getKey(KeyCode::KEY_2))
            {
                gTransform->setPosition(glm::vec3(oldPos.x, oldPos.y, oldPos.z - te::Time::getDelta() * 20));
            }
            if (InputManager::getKeyDown(KeyCode::KEY_3))
            {
                gTransform->rotateByX(30);
            }
            if (InputManager::getKeyDown(KeyCode::KEY_4))
            {
                gTransform->rotateByX(-30);
            }
            if (InputManager::getKeyDown(KeyCode::KEY_5))
            {
                gTransform->setScale(glm::vec3(2, 2, 2));
            }
            if (InputManager::getKeyDown(KeyCode::KEY_6))
            {
                gTransform->setScale(glm::vec3(1, 1, 1));
            }
		}
	};

}
#endif // !GMATERIAL
