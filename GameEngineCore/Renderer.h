#ifndef GE_RENDERER
#define GE_RENDERER

#include "Component.h"
#include <vector>

#include <vulkan/vulkan.hpp>
namespace te
{
	class Mesh;
	class Entity;
	

	class Renderer : public te::Component
	{
		vk::Buffer _vertexBuffer;
		vk::DeviceMemory _vertexBufferMemory;
		vk::Buffer _indexBuffer;
		vk::DeviceMemory _indexBufferMemory;
		
		Mesh* _mesh;



	public:
		Renderer() {}
		~Renderer();
		Renderer(Entity* e);
		void setMesh(Mesh* mesh);

		void onAwake() override;
        

	};
}
#endif // !GE_RENDERER
