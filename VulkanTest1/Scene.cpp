#include "Scene.h"
#include "Entity.h"

void te::Scene::addEntity(te::Entity* entity)
{
	entities.push_back(entity);
}

void te::Scene::deleteEntity(te::Entity* entity)
{
	entities.remove(entity);
}
