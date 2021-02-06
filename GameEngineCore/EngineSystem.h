
#ifndef ENGINE_SYSTEM
#define ENGINE_SYSTEM

#include "Singleton.h"

namespace te
{
	template<typename T>
	class EngineSystem : public te::Singleton<T>
	{
	protected:
		EngineSystem()
		{

		}
	};

}
#endif // !ENGINE_SYSTEM


