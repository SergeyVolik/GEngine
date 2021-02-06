#ifndef GE_TRANSFORM
#define GE_TRANSFORM

#include <glm/glm.hpp>
#include "Component.h"
#include <glm/ext/matrix_transform.hpp>

#include <glm/gtx/transform.hpp>
//#include <glm/gtx/transform.hpp>
//#include <glm/gtc/matrix_transform.hpp>
namespace te
{
	class Component;
	class Entity;

	class Transform : public te::Component
	{
	private:
		glm::vec3 _position;
		glm::vec3 _rotation;
		glm::vec3 _scale;

		glm::mat4 _translateMatrix;
		glm::mat4 _scalingMatrix;
		glm::mat4 _rotationMatrix;

		glm::mat4 _transformedVector;
	public:
		Transform() { }
		Transform(te::Entity* ent) : te::Component(ent),
			_position(0.0f, 0.0f, 0.0f),
			_rotation(0.0f, 0.0f, 0.0f),
			_scale(1.0f, 1.0f, 1.0f),
			_translateMatrix(glm::translate(glm::mat4(1.0f), _position)),
			_rotationMatrix(glm::rotate(0.0f, glm::vec3(1.0f))),
			_scalingMatrix(glm::scale(glm::mat4(1.0f), _scale)) { }

		glm::vec3 getPosition() {
			return _position;
			
		}
		void setPosition(glm::vec3 newPos)
		{
			_position = newPos;
			_translateMatrix = glm::translate(glm::mat4(1.0f), _position);
		}
		glm::vec3 getRotation() { return _rotation; }

		void rotateByX(float angle) {

			_rotationMatrix = glm::rotate(_rotationMatrix, angle, glm::vec3(1.0f, 0.0f, 0.0f));
		}
		void rotateByY(float angle) {

			_rotationMatrix =  glm::rotate(_rotationMatrix, angle, glm::vec3(0.0f, 1.0f, 0.0f));
		}
		void rotateByZ(float angle) {

			_rotationMatrix =  glm::rotate(_rotationMatrix, angle, glm::vec3(0.0f, 0.0f, 1.0f));
		}
		void rotateByXY(float angle) {

			_rotationMatrix =  glm::rotate(_rotationMatrix, angle, glm::vec3(1.0f, 1.0f, 0.0f));
		}
		void rotateByXZ(float angle) {

			_rotationMatrix =  glm::rotate(_rotationMatrix, angle, glm::vec3(1.0f, 0.0f, 1.0f));
		}

		void rotateByYZ(float angle) {

			_rotationMatrix = glm::rotate(_rotationMatrix, angle, glm::vec3(0.0f, 1.0f, 1.0f));
		}

		glm::vec3 getScale() { return _scale; }

		void setScale(glm::vec3 scale)
		{
			_scale = scale;
			_scalingMatrix = glm::scale(glm::mat4(1.0f), _scale);
		}
		void onAwake() override
		{
			
			_transformedVector = _translateMatrix * _rotationMatrix * _scalingMatrix;
			
		}

		void onUpdate() override
		{
			_transformedVector = _translateMatrix * _rotationMatrix * _scalingMatrix;
			
		}

		glm::mat4 getTransformation()
		{
			return _transformedVector;
		}
		
	};
}

#endif // !GE_TRANSFORM


