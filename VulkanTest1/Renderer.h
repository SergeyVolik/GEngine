#ifndef GE_RENDERER
#define GE_RENDERER

#include "Component.h"
#include <vector>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

namespace te
{
	class Mesh;
	class Entity;
	

	class Renderer : public te::Component
	{
		VkBuffer _vertexBuffer;
		VkDeviceMemory _vertexBufferMemory;
		VkBuffer _indexBuffer;
		VkDeviceMemory _indexBufferMemory;

		

		Mesh* _mesh;
	public:
		Renderer() {}
		Renderer(Entity* e);
		void setMesh(Mesh* mesh);

		void onAwake() override;
        

	};
}
#endif // !GE_RENDERER
