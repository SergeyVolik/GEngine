#ifndef GE_ENTIRY
#define GE_ENTIRY

#include <unordered_map>

namespace te
{
	class Component;
	class Transform;

	class Entity
	{
	private:
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

		
		
	};


}
#endif // !GE_ENTIRY
