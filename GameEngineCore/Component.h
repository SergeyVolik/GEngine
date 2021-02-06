#ifndef GE_COMPONENT
#define GE_COMPONENT

namespace te
{
	class Entity;

	class Component
	{
	
	private:
		te::Entity* _entity;

	protected:
		Component(te::Entity* e) { _entity = e; }
		Component() {  }
	public:

		te::Entity* getEntity() {
			return _entity;
		}
		void setEntity(te::Entity* entity) {
			_entity = entity;
		}
		virtual void onUpdate() {}
		virtual void onAwake() {}
		virtual void onStart() {}
		virtual void onDestroy() {}
		virtual void onVisible() {}
		virtual void onInvisible() {}
	};

}
#endif // !GE_COMPONENT
