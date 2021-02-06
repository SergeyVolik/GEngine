#ifndef GE_ENTIRY
#define GE_ENTIRY

#include <unordered_map>

namespace te
{
	class Component;
	class Transform;

	enum class PrimitiveType { CUBE, PLANE, SPHERE };


	class Entity
	{

	private:

		static inline const float cubeVertexPositions[] =
		{
			-0.5f, -0.5f, -0.5f,
			-0.5f, 0.5f, -0.5f,
			 0.5f, -0.5f, -0.5f,
			 0.5f, 0.5f, -0.5f,
			 0.5f, -0.5f, 0.5f,
			 0.5f, 0.5f, 0.5f,
			 -0.5f, -0.5f, 0.5f,
			 -0.5f, 0.5f, 0.5f
		};

		static inline const uint16_t cubeIndexes[] =
		{
			0, 1, 2,
			2, 1, 3,
			2, 3, 4,
			4, 3, 5,
			4, 5, 6,
			6, 5, 7,
			6, 7, 0,
			0, 7, 1,
			6, 0, 2,
			2, 4, 6,
			7, 5, 3,
			7, 3, 1

		};
		std::unordered_map<size_t, te::Component*> components;
		te::Transform* transform;
	public:

		Entity();
		~Entity();

		te::Transform* getTransform() {
			return transform;
		}

		
		bool addComponent(Component*);

		//typeid(T).has_hash for get type hash
		Component* getComponent(size_t componentHash);

		void awakeAllComponents();
		void startAllComponents();
		void updateAllComponents();

		void CreatePrimitive(PrimitiveType primitive)
		{
			
		}
		
	};


}
#endif // !GE_ENTIRY
