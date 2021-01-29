#include "Renderer.h"

#include "Mesh.h"
#include "VulkanRenderManager.h"
#include "Entity.h"


te::Renderer::Renderer(te::Entity* e) : Component(e) { VulkanRenderManager::getInstance()->renderers.push_back(this); }
te::Renderer::~Renderer()  { VulkanRenderManager::getInstance()->renderers.remove(this); }
void te::Renderer::setMesh(Mesh* mesh) { _mesh = mesh; }

void te::Renderer::onAwake()
{
	//te::VulkanRenderManager::getInstance()->createIndexBuffer(_mesh->getIndices(), _indexBuffer, _indexBufferMemory);
}