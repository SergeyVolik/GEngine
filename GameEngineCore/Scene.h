#ifndef GE_SCENE
#define GE_SCENE

#include <list>

namespace te
{
	class Entity;
	class Camera;
	class Scene
	{
	private:
		const char* name;
		std::list<te::Entity*> entities;
		Camera* mainCamera;
	public:

		Scene(const char* _name) : name(_name), entities({ }) {}

		void addEntity(te::Entity*);
		void deleteEntity(te::Entity*);
		inline Camera* getMainCamera() { return mainCamera; }
		inline void setMainCamera(Camera* camera) { mainCamera = camera; }

		void awakeAllEntities();
		void startAllEntities();
		void updateAllEntities();
			
		~Scene() 
		{
			for (auto& entity : entities)
				delete entity;
		}
	};

}
#endif // !GE_SCENE
