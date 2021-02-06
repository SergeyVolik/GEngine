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

void te::Scene::awakeAllEntities()
{
	for (auto& entity : entities)
		entity->awakeAllComponents();
}

void te::Scene::startAllEntities()
{
	for (auto& entity : entities)
		entity->startAllComponents();
}

void te::Scene::updateAllEntities()
{
	for (auto& entity : entities)
		entity->updateAllComponents();
}
