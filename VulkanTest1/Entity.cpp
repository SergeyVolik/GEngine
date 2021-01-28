
#include <typeinfo>
#include <assert.h>

#include "Entity.h"
#include "Component.h";
#include "Transform.h"


#ifndef NDEBUG
#include <iostream>
#endif

te::Entity::Entity() {
	addComponent(new te::Transform(this));
	
}

te::Entity::~Entity() {
	for (std::map<size_t, te::Component*>::iterator it = components.begin(); it != components.end(); ++it)
		delete it->second;
}

bool te::Entity::addComponent(Component* comp)
{
	
	if (!getComponent(typeid(*comp).hash_code()))
	{
		#ifndef NDEBUG
		std::cout << "[Entity.addComponent] component " << typeid(*comp).name() << " added" << std::endl;
		#endif // DEBUG

		comp->setEntity(this);
		components.insert(std::pair<size_t, te::Component*>(typeid(*comp).hash_code(), comp));
		return true;
	}
	
		#ifndef NDEBUG
			std::cout << "[Entity.addComponent] component " << typeid(*comp).name() << "exits can't be added!" << std::endl;
		#endif // DEBUG
	
	return false;
}


te::Component* te::Entity::getComponent(size_t typeHash)
{
	std::map<size_t, Component*>::iterator it = components.find(typeHash);
	if (it != components.end())
	{
		#ifndef NDEBUG
				std::cout << "[Entity.getComponent] component exist" << typeHash << std::endl;
		#endif // DEBUG
		return it->second;
	}

	#ifndef NDEBUG
		std::cout << "[Entity.getComponent] can't get component with hash" << typeHash <<std::endl;
	#endif // DEBUG
	return nullptr;
	
}