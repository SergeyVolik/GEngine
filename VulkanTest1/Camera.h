#ifndef GCAMERA
#define GCAMERA
#include "Component.h"

namespace te
{	
	class Camera : public te::Component
	{
	public:
		Camera() {}
		Camera(Entity* e) : Component(e) {}
	};

}
#endif // !GCAMERA
