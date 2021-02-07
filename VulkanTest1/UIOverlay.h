#ifndef EDITO_UI
#define EDITO_UI

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <vector>
#include <sstream>
#include <iomanip>

#include "vulkan/vulkan.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL

#include <glm/glm.hpp>

#include "../external/imgui/imgui.h"

namespace te
{

	class UIOverlay
	{
	public:
		//vks::VulkanDevice* device; пофиксить
		VkQueue queue;

		vk::SampleCountFlagBits rasterizationSamples = vk::SampleCountFlagBits::e1;
		uint32_t subpass = 0;

		//vks::Buffer vertexBuffer; !!!!!need fix
		//vks::Buffer indexBuffer; !!!!!need fix

		int32_t vertexCount = 0;
		int32_t indexCount = 0;

		std::vector<vk::PipelineShaderStageCreateInfo> shaders;

		vk::DescriptorPool descriptorPool;
		vk::DescriptorSetLayout descriptorSetLayout;
		vk::DescriptorSet descriptorSet;
		vk::PipelineLayout pipelineLayout;
		vk::Pipeline pipeline;

		vk::DeviceMemory fontMemory = nullptr;
		vk::Image fontImage = nullptr;
		vk::ImageView fontView = nullptr;
		vk::Sampler sampler;

		struct PushConstBlock {
			glm::vec2 scale;
			glm::vec2 translate;
		} pushConstBlock;

		bool visible = true;
		bool updated = false;
		float scale = 1.0f;

		UIOverlay();
		~UIOverlay();


	};
}

#endif // !EDITO_UI

