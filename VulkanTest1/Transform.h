#ifndef GE_TRANSFORM
#define GE_TRANSFORM

#include <glm/glm.hpp>
#include "Component.h"


namespace te
{
	class Component;
	class Entity;

	class Transform : public te::Component
	{
	private:
		glm::vec3 position;
		glm::vec4 rotation;

		//glm::vec3 localPosition;
		//glm::vec4 localRotation;

		glm::vec3 scale;
	public:
		Transform() { }
		Transform(te::Entity* ent) : te::Component(ent),
			position(0.0f, 0.0f, 0.0f),
			rotation(0.0f, 0.0f, 0.0f, 0.0f),
			scale(1.0f, 1.0f, 1.0f) { }

		glm::vec3 getPosition() {
			return position;
		}
		glm::vec4 getRotation() { return rotation; }
		glm::vec3 getScale() { return scale; }
	};
}

#endif // !GE_TRANSFORM


