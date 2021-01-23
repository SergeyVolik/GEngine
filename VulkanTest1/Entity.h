#ifndef GE_ENTIRY
#define GE_ENTIRY

#include <map>


namespace te
{
	class Component;
	class Transform;

	class Entity
	{
	private:
		std::map<size_t, te::Component*> components;
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
		
		
	};


}
#endif // !GE_ENTIRY
