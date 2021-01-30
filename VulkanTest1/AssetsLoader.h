#ifndef GE_ASSETS_LOADER
#define GE_ASSETS_LOADER

#include "Singleton.h"
#include <vector>
#include "Vertex.h"
namespace te
{
	 //Vertex;
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
		void loadModel(const char* path, std::vector<te::Vertex> &vertices, std::vector<uint32_t> &indices);
		void loadTextureToGPU(const char* path, VkImage image);

	};
}

#endif // !GE_ASSETS_LOADER
