#ifndef GE_ASSETS_LOADER
#define GE_ASSETS_LOADER

#include "Singleton.h"
#include <vector>
#include "Vertex.h"
namespace te
{
	struct Mesh;
	class AssetsLoader : public te::Singleton<AssetsLoader>
	{
	public:
		
		inline static void terminate() { 
			delete _instance;
		};
		inline static void initialize()
		{
			_instance = new AssetsLoader();
		}
		
		te::Mesh loadModel(const char* path);
		void loadTextureToGPU(const char* path, VkImage image);

	};
}

#endif // !GE_ASSETS_LOADER
