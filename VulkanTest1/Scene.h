#ifndef GE_SCENE
#define GE_SCENE

#include <list>

namespace te
{
	class Entity;
	class Scene
	{
	private:
		const char* name;
		std::list<te::Entity*> entities;
	public:

		Scene(const char* _name) : name(_name), entities({ }) {}

		void addEntity(te::Entity*);
		void deleteEntity(te::Entity*);

		~Scene() 
		{
			for (auto& entity : entities)
				delete entity;
		}
	};

}
#endif // !GE_SCENE
