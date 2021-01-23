#ifndef GE_RENDERER
#define GE_RENDERER

#include "Component.h"

namespace te
{
	
	class Renderer : public te::Component
	{
	public:
		Renderer() {}
		Renderer(Entity* e) : Component(e) {}
	};
}
#endif // !GE_RENDERER
