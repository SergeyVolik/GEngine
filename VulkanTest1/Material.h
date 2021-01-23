#ifndef GMATERIAL
#define GMATERIAL
#include "Component.h"

namespace te
{
	
	class Material : public te::Component
	{
	public:
		Material() {}
		Material(Entity* e) : Component(e) {}
	};

}
#endif // !GMATERIAL
