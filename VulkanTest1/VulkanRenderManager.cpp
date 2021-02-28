
#include "VulkanRenderManager.h"
#include "Vertex.h"
#include "FileReader.h"
#include "VulkanHelper.h"
#include "AssetsLoader.h"
#include "Mesh.h"
#include "SwapChain.h"
//#include <vk_mem_alloc.h>
#include <array>
#pragma region vulkan instancing

void te::VulkanRenderManager::initialize(te::Window* wnd) {


    instance = new VulkanRenderManager();
    instance->window = wnd;
    VULKAN_HPP_DEFAULT_DISPATCHER.init(vkGetInstanceProcAddr);

    instance->createInstance();

    VULKAN_HPP_DEFAULT_DISPATCHER.init(instance->vulkanDevice->instance);

    instance->setupDebugMessenger();
    instance->createSurface();
    instance->pickPhysicalDevice();
    instance->createLogicalDevice();

    instance->createVulkanMemoryAllocator();


    instance->createSwapchain();
    instance->createFramebuffers();

    instance->createDescriptorSetLayout();
    instance->createPipelineCache();
    instance->createGraphicsPipeline();
    instance->createCommandPool();
    instance->createDepthResources();


    instance->texture = new ::vkh::Texture2D(instance->vulkanDevice);
    instance->skyboxData.skybox = new ::vkh::TextureCubeMap(instance->vulkanDevice);
    instance->texture->loadFromFile(TEXTURE_PATH, vk::Format::eR8G8B8A8Srgb, instance->vulkanQueues.graphicsQueue);
    instance->skyboxData.skybox->loadFromFile(instance->skyboxPaths, vk::Format::eR8G8B8A8Srgb, instance->vulkanQueues.graphicsQueue);
    /* instance->createTextureImage(instance->textureImage, instance->mipLevels);
     instance->createTextureImageView(instance->textureImageView, instance->textureImage, instance->mipLevels);
     instance->createTextureSampler();*/
    instance->loadModel();
    instance->gObject = new Entity();
    instance->gTransform = (Transform*)instance->gObject->getComponent(typeid(Transform).hash_code());
    instance->gTransform->onAwake();

    instance->createVertexBuffer(
        instance->vertices,
        instance->vertexBuffer,
        instance->vertexBufferMemory

    );

    instance->vulkanDevice->createVertexBuffer(
        instance->skyboxData.vertices,
        instance->skyboxData.vertexBuffer,
        instance->skyboxData.vertexBufferMemory,
        instance->vulkanDevice->commandPool,
        instance->vulkanQueues.graphicsQueue
    );

    instance->createIndexBuffer(
        instance->_indices,
        instance->_indexBuffer,
        instance->_indexBufferMemory
    );

    instance->vulkanDevice->createIndexBuffer(
        instance->skyboxData._indices,
        instance->skyboxData._indexBuffer,
        instance->skyboxData._indexBufferMemory,
        instance->vulkanDevice->commandPool,
        instance->vulkanQueues.graphicsQueue
    );

    instance->createUniformBuffers();
    instance->createDescriptorPool();
    instance->createDescriptorSets();
    instance->createCommandBuffers();
    instance->createSyncObjects();

}

void te::VulkanRenderManager::createInstance()
{
    if (enableValidationLayers && !checkValidationLayerSupport()) {
        throw std::runtime_error("validation layers requested, but not available!");
    }

    vk::ApplicationInfo appInfo{};
   
    appInfo.pApplicationName = "Hello Triangle";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "No Engine";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_2;
   
    vk::InstanceCreateInfo createInfo{};   
    createInfo.pApplicationInfo = &appInfo;

    auto extensions = getRequiredExtensions();
    createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
    createInfo.ppEnabledExtensionNames = extensions.data();

    vk::DebugUtilsMessengerCreateInfoEXT debugCreateInfo;

    if (enableValidationLayers) {
        createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
        createInfo.ppEnabledLayerNames = validationLayers.data();

        populateDebugMessengerCreateInfo(debugCreateInfo);
        createInfo.pNext = &debugCreateInfo;
        

    }
    else {
        createInfo.enabledLayerCount = 0;

        createInfo.pNext = nullptr;
    }

    vulkanDevice = new te::vkh::VulkanDevice();
   
    if (vk::createInstance(&createInfo, nullptr, &vulkanDevice->instance) != vk::Result::eSuccess) {
        throw std::runtime_error("failed to create instance!");
    }
}

#pragma endregion


#pragma region selection vulkanDevice->logicalDevice

void te::VulkanRenderManager::pickPhysicalDevice()
{

    std::vector<vk::PhysicalDevice> devices = vulkanDevice->instance.enumeratePhysicalDevices();

    for (const auto& pDevice : devices) {
        if (isDeviceSuitable(pDevice)) {
            vulkanDevice->setupPhysicalDevice(pDevice);
           
            break;
        }
    }

    if ( vulkanDevice->physicalDevice == VK_NULL_HANDLE) {
        throw std::runtime_error("failed to find a suitable GPU!");
    }


}

void te::VulkanRenderManager::createLogicalDevice()
{
    te::vkh::QueueFamilyIndices indices = te::vkh::findQueueFamilies(surface, vulkanDevice->physicalDevice);

    std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos;
    std::set<uint32_t> uniqueQueueFamilies = { indices.graphicsFamily.value(), indices.presentFamily.value(), indices.transferFamily.value() };

    float queuePriority = 1.0f;
    for (uint32_t queueFamily : uniqueQueueFamilies) {
        vk::DeviceQueueCreateInfo queueCreateInfo{};
        
        queueCreateInfo.queueFamilyIndex = queueFamily;
        queueCreateInfo.queueCount = 1;
        queueCreateInfo.pQueuePriorities = &queuePriority;
        queueCreateInfos.push_back(queueCreateInfo);
    }

    vk::PhysicalDeviceFeatures deviceFeatures{};
    deviceFeatures.samplerAnisotropy = VK_TRUE;

    vk::DeviceCreateInfo createInfo{};
   
    createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
    createInfo.pQueueCreateInfos = queueCreateInfos.data();

    createInfo.pEnabledFeatures = &deviceFeatures;

    createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
    createInfo.ppEnabledExtensionNames = deviceExtensions.data();

    if (enableValidationLayers) {
        createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
        createInfo.ppEnabledLayerNames = validationLayers.data();
       
    }
    else {
        createInfo.enabledLayerCount = 0;
    }
   
    if ( vulkanDevice->physicalDevice.createDevice(&createInfo, nullptr, &vulkanDevice->logicalDevice) != vk::Result::eSuccess) {
        throw std::runtime_error("failed to create logical  vulkanDevice->logicalDevice!");
    }
    
     vulkanDevice->logicalDevice.getQueue(indices.graphicsFamily.value(), 0, &vulkanQueues.graphicsQueue);
     vulkanDevice->logicalDevice.getQueue(indices.presentFamily.value(), 0, &vulkanQueues.presentQueue);
     vulkanDevice->logicalDevice.getQueue(indices.transferFamily.value(), 0, &vulkanQueues.transferQueue);
   
}
#pragma endregion

void te::VulkanRenderManager::createSwapchain()
{
    mySwapChain = new vkGame::VulkanSwapChain(surface, vulkanDevice);
    int width, height;
    window->getFramebufferSize(&width, &height);
    mySwapChain->createSwapChain(width, height);
}



#pragma region vulkan validation layer (debug)
void te::VulkanRenderManager::setupDebugMessenger()
{
    if (!enableValidationLayers) return;

    vk::DebugUtilsMessengerCreateInfoEXT createInfo;
    populateDebugMessengerCreateInfo(createInfo);

    if (vulkanDevice->instance.createDebugUtilsMessengerEXT(&createInfo, nullptr, &debugMessenger) != vk::Result::eSuccess) {
        throw std::runtime_error("failed to set up debug messenger!");
    }

}

void te::VulkanRenderManager::createSurface()
{
    window->createSurface(vulkanDevice->instance, &surface, nullptr);
}



#pragma endregion


void te::VulkanRenderManager::createDescriptorSetLayout()
{
    vk::DescriptorSetLayoutBinding uboLayoutBinding{};
    uboLayoutBinding.binding = 0;
    uboLayoutBinding.descriptorCount = 1;
    uboLayoutBinding.descriptorType = vk::DescriptorType::eUniformBuffer;
    uboLayoutBinding.pImmutableSamplers = nullptr;
    uboLayoutBinding.stageFlags = vk::ShaderStageFlagBits::eVertex;

    vk::DescriptorSetLayoutBinding samplerLayoutBinding{};
    samplerLayoutBinding.binding = 1;
    samplerLayoutBinding.descriptorCount = 1;
    samplerLayoutBinding.descriptorType = vk::DescriptorType::eCombinedImageSampler;
    samplerLayoutBinding.pImmutableSamplers = nullptr;
    samplerLayoutBinding.stageFlags = vk::ShaderStageFlagBits::eFragment;

    std::array<vk::DescriptorSetLayoutBinding, 2> bindings = { uboLayoutBinding, samplerLayoutBinding };
    vk::DescriptorSetLayoutCreateInfo layoutInfo{}; 
    layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
    layoutInfo.pBindings = bindings.data();

    if ( vulkanDevice->logicalDevice.createDescriptorSetLayout(&layoutInfo, nullptr, &descriptorSetLayout) != vk::Result::eSuccess) {
        throw std::runtime_error("failed to create descriptor set layout!");
    }
}

vk::PipelineShaderStageCreateInfo te::VulkanRenderManager::loadShader(std::string path, vk::ShaderStageFlagBits stage)
{
    auto shaderCode = te::FileReader::readFile(path);
    vk::ShaderModule shaderModule = vulkanDevice->createShaderModule(shaderCode);

    vk::PipelineShaderStageCreateInfo vertShaderStageInfo{};

    vertShaderStageInfo.stage = stage;
    vertShaderStageInfo.module = shaderModule;
    vertShaderStageInfo.pName = "main";

    loadedShaderModules.push_back(shaderModule);

    return vertShaderStageInfo;
}
void te::VulkanRenderManager::createGraphicsPipeline()
{
  
    std::array<vk::PipelineShaderStageCreateInfo, 2> shaderStages;

    //--------------определение структуры для ввода вертексов в шейдер-----------------------
    vk::PipelineVertexInputStateCreateInfo vertexInputInfo{};

    auto bindingDescription = te::Vertex::getBindingDescription();
    auto attributeDescriptions = te::Vertex::getAttributeDescriptions();

    vertexInputInfo.vertexBindingDescriptionCount = 1;
    vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
    vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
    vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

    
    //------------------создание структуры котороя определяет в какой последовательности и какие финуры будут читаться из вертексов-------------
    vk::PipelineInputAssemblyStateCreateInfo inputAssembly{};
    
    inputAssembly.topology = vk::PrimitiveTopology::eTriangleList;
    inputAssembly.primitiveRestartEnable = VK_FALSE;

    //--------------------------------настройка вывода изображения на полотно------------------------

    auto swapChainExtent = mySwapChain->getExtent();

    vk::Viewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width =  (float)swapChainExtent.width;
    viewport.height =  (float)swapChainExtent.height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    vk::Rect2D scissor{};
    scissor.offset = vk::Offset2D{ 0, 0 };
    scissor.extent = swapChainExtent;


    vk::PipelineViewportStateCreateInfo viewportState{};
  
    viewportState.viewportCount = 1;
    viewportState.pViewports = &viewport;
    viewportState.scissorCount = 1;
    viewportState.pScissors = &scissor;


    //-----------------------------настройка уровня растеризации-------------------------
    vk::PipelineRasterizationStateCreateInfo rasterizer{};

    rasterizer.depthClampEnable = VK_FALSE;
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    rasterizer.polygonMode = vk::PolygonMode::eFill;
    rasterizer.lineWidth = 1.0f;
    rasterizer.cullMode = vk::CullModeFlagBits::eBack;
    rasterizer.frontFace = vk::FrontFace::eCounterClockwise;
    rasterizer.depthBiasEnable = VK_FALSE;


    //------------------------------настройка мультисемплинга(сглаживания)--------------------------------
    vk::PipelineMultisampleStateCreateInfo multisampling{};
   
    multisampling.sampleShadingEnable = VK_FALSE;
    multisampling.rasterizationSamples = vk::SampleCountFlagBits::e1;


    //------------------------------настройка глубины и трафарета------------------------------
    vk::PipelineDepthStencilStateCreateInfo depthStencil{};
  
    depthStencil.depthTestEnable = VK_TRUE;
    depthStencil.depthWriteEnable = VK_TRUE;
    depthStencil.depthCompareOp = vk::CompareOp::eLess;
    depthStencil.depthBoundsTestEnable = VK_FALSE;
    depthStencil.stencilTestEnable = VK_FALSE;

    //------------------------------смешивание цветов-------------------------------------------
    vk::PipelineColorBlendAttachmentState colorBlendAttachment{};
    colorBlendAttachment.colorWriteMask = vk::ColorComponentFlagBits::eR |
                                            vk::ColorComponentFlagBits::eG |
                                            vk::ColorComponentFlagBits::eB |
                                            vk::ColorComponentFlagBits::eA;
       
    colorBlendAttachment.blendEnable = VK_FALSE;

    vk::PipelineColorBlendStateCreateInfo colorBlending{};
  
    colorBlending.logicOpEnable = VK_FALSE;
    colorBlending.logicOp = vk::LogicOp::eCopy;
    colorBlending.attachmentCount = 1;
    colorBlending.pAttachments = &colorBlendAttachment;
    colorBlending.blendConstants[0] = 0.0f;
    colorBlending.blendConstants[1] = 0.0f;
    colorBlending.blendConstants[2] = 0.0f;
    colorBlending.blendConstants[3] = 0.0f;



    vk::PipelineLayoutCreateInfo pipelineLayoutInfo{};
   
    pipelineLayoutInfo.setLayoutCount = 1;
    pipelineLayoutInfo.pSetLayouts = &descriptorSetLayout;

    if ( vulkanDevice->logicalDevice.createPipelineLayout(&pipelineLayoutInfo, nullptr, &pipelineLayout) != vk::Result::eSuccess) {
        throw std::runtime_error("failed to create pipeline layout!");
    }

    vk::GraphicsPipelineCreateInfo pipelineInfo{};

    pipelineInfo.stageCount = shaderStages.size();
    pipelineInfo.pStages = shaderStages.data();
    pipelineInfo.pVertexInputState = &vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pMultisampleState = &multisampling;
    pipelineInfo.pDepthStencilState = &depthStencil;
    pipelineInfo.pColorBlendState = &colorBlending;
    pipelineInfo.layout = pipelineLayout;
    pipelineInfo.renderPass = mySwapChain->getRenderPass();
    pipelineInfo.subpass = 0;
    pipelineInfo.basePipelineHandle = nullptr;

    shaderStages[0] = loadShader("shaders/vert.spv", vk::ShaderStageFlagBits::eVertex);
    shaderStages[1] = loadShader("shaders/frag.spv", vk::ShaderStageFlagBits::eFragment);

    if ( vulkanDevice->logicalDevice.createGraphicsPipelines(pipelineCache, 1, &pipelineInfo, nullptr, &graphicsPipeline) != vk::Result::eSuccess) {
        throw std::runtime_error("failed to create graphics pipeline!");
    }

    shaderStages[0] = loadShader("shaders/skybox.vert.spv", vk::ShaderStageFlagBits::eVertex);
    shaderStages[1] = loadShader("shaders/skybox.frag.spv", vk::ShaderStageFlagBits::eFragment);
    rasterizer.cullMode = vk::CullModeFlagBits::eFront;
     
    if (vulkanDevice->logicalDevice.createGraphicsPipelines(pipelineCache, 1, &pipelineInfo, nullptr, &skyboxData.skyboxPipeline) != vk::Result::eSuccess) {
        throw std::runtime_error("failed to create graphics pipeline!");
    }

    for (int i = 0; i < loadedShaderModules.size(); i++)
        vulkanDevice->logicalDevice.destroyShaderModule(loadedShaderModules[i]);

    loadedShaderModules.clear();

}

void te::VulkanRenderManager::createCommandPool()
{
    te::vkh::QueueFamilyIndices queueFamilyIndices =  te::vkh::findQueueFamilies(surface, vulkanDevice->physicalDevice);

    vk::CommandPoolCreateInfo poolInfo{};
   
    poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();
    poolInfo.flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer; // для сброса всех команд просле прохода рендеринга
    if ( vulkanDevice->logicalDevice.createCommandPool(&poolInfo, nullptr, &vulkanDevice->commandPool) != vk::Result::eSuccess) {
        throw std::runtime_error("failed to create graphics command pool!");
    }
}

void te::VulkanRenderManager::createDepthResources()
{
   
}

void te::VulkanRenderManager::createFramebuffers()
{
    // Four attachments (3 color, 1 depth)
    vkGame::AttachmentCreateInfo attachmentInfo{};
  
    auto swapChainExtent = mySwapChain->getExtent();
    
    attachmentInfo.width = swapChainExtent.width;
    attachmentInfo.height = swapChainExtent.height;
    attachmentInfo.layerCount = 1;  
       
    mySwapChain->initFrameBuffers();

    for (int i = 0; i < mySwapChain->getChainsCount(); i++)
    {
        //attachmentInfo.usage = vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eSampled;
        //// Color attachments
        //// Attachment 0: (World space) Positions
        //attachmentInfo.format = vk::Format::eR16G16B16A16Sfloat;
        //mySwapChain->swapChainFramebuffers[i]->addAttachment(attachmentInfo);

        //// Attachment 1: (World space) Normals
        //attachmentInfo.format = vk::Format::eR16G16B16A16Sfloat;
        //mySwapChain->swapChainFramebuffers[i]->addAttachment(attachmentInfo);

        //// Attachment 2: Albedo (color)
        //attachmentInfo.format = vk::Format::eR8G8B8A8Unorm;
        //mySwapChain->swapChainFramebuffers[i]->addAttachment(attachmentInfo);

        // Depth attachment
        // Find a suitable depth format

        attachmentInfo.format = findDepthFormat();
        attachmentInfo.usage = vk::ImageUsageFlagBits::eDepthStencilAttachment;
        attachmentInfo.uniqueAttachment = true;

        if (i == 0)
        {
            mySwapChain->framebuffers[i]->addAttachment(attachmentInfo);
            mySwapChain->framebuffers[i]->createSampler(vk::Filter::eNearest, vk::Filter::eNearest, vk::SamplerAddressMode::eClampToEdge);
            mySwapChain->framebuffers[i]->createRenderPass(mySwapChain->getImageFormat());
        }
        else {
            mySwapChain->framebuffers[i]->attachments.push_back(mySwapChain->framebuffers[0]->attachments[0]);
            mySwapChain->framebuffers[i]->sampler = mySwapChain->framebuffers[0]->sampler;
            mySwapChain->framebuffers[i]->renderPass = mySwapChain->framebuffers[0]->renderPass;
        }


    }

    
    mySwapChain->createFramebuffers();
    
}



void te::VulkanRenderManager::loadModel()
{
    Mesh mesh =  te::AssetsLoader::getInstance()->loadModel(MODEL_PATH.c_str());
   
    vertices = mesh.getVertices();
    _indices = mesh.getIndices();


}

void te::VulkanRenderManager::createVulkanMemoryAllocator()
{
   /* VmaAllocatorCreateInfo allocatorInfo = {};
    allocatorInfo.vulkanApiVersion = VK_API_VERSION_1_0;
    allocatorInfo. vulkanDevice->physicalDevice =  vulkanDevice->physicalDevice;
    allocatorInfo. vulkanDevice->logicalDevice =  vulkanDevice->logicalDevice;
    allocatorInfo.instance = instance;
   
    VmaAllocator allocator;
    vmaCreateAllocator(&allocatorInfo, &allocator);*/
}


void te::VulkanRenderManager::createUniformBuffers()
{
    vk::DeviceSize bufferSize = sizeof(UniformBufferObject);

    uniformBuffers.resize(mySwapChain->getChainsCount());
    uniformBuffersMemory.resize(mySwapChain->getChainsCount());
    skyboxData.skyboxUBOBuffers.resize(mySwapChain->getChainsCount());
    skyboxData.skyboxUBOBuffersMemory.resize(mySwapChain->getChainsCount());

    for (size_t i = 0; i < mySwapChain->getChainsCount(); i++) {
         vulkanDevice->createBuffer(
            bufferSize,
            vk::BufferUsageFlagBits::eUniformBuffer,
            vk::MemoryPropertyFlagBits::eHostVisible  |
            vk::MemoryPropertyFlagBits::eHostCoherent,
            uniformBuffers[i], uniformBuffersMemory[i]
        );

         vulkanDevice->createBuffer(
             bufferSize,
             vk::BufferUsageFlagBits::eUniformBuffer,
             vk::MemoryPropertyFlagBits::eHostVisible |
             vk::MemoryPropertyFlagBits::eHostCoherent,
             skyboxData.skyboxUBOBuffers[i], skyboxData.skyboxUBOBuffersMemory[i]
         );
    }
}

void te::VulkanRenderManager::createDescriptorPool()
{
    std::array<vk::DescriptorPoolSize, 2> poolSizes{};
    poolSizes[0].type = vk::DescriptorType::eUniformBuffer;
    poolSizes[0].descriptorCount = static_cast<uint32_t>(mySwapChain->getChainsCount());
    poolSizes[1].type = vk::DescriptorType::eCombinedImageSampler;
    poolSizes[1].descriptorCount = static_cast<uint32_t>(mySwapChain->getChainsCount());

    vk::DescriptorPoolCreateInfo poolInfo{};
   
    poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
    poolInfo.pPoolSizes = poolSizes.data();
    poolInfo.maxSets = static_cast<uint32_t>(mySwapChain->getChainsCount() * 2);

    if ( vulkanDevice->logicalDevice.createDescriptorPool(&poolInfo, nullptr, &descriptorPool) != vk::Result::eSuccess) {
        throw std::runtime_error("failed to create descriptor pool!");
    }
}

void te::VulkanRenderManager::createDescriptorSets()
{
    std::vector<vk::DescriptorSetLayout> layouts(mySwapChain->getChainsCount(), descriptorSetLayout);
    vk::DescriptorSetAllocateInfo allocInfo{};
   
    allocInfo.descriptorPool = descriptorPool;
    allocInfo.descriptorSetCount = static_cast<uint32_t>(mySwapChain->getChainsCount());
    allocInfo.pSetLayouts = layouts.data();

    descriptorSets.resize(mySwapChain->getChainsCount());
    if ( vulkanDevice->logicalDevice.allocateDescriptorSets(&allocInfo, descriptorSets.data()) != vk::Result::eSuccess) {
        throw std::runtime_error("failed to allocate descriptor sets!");
    }

    skyboxData.skyboxDescriptorSets.resize(mySwapChain->getChainsCount());
    if (vulkanDevice->logicalDevice.allocateDescriptorSets(&allocInfo, skyboxData.skyboxDescriptorSets.data()) != vk::Result::eSuccess) {
        throw std::runtime_error("failed to allocate descriptor sets!");
    }

    for (size_t i = 0; i < mySwapChain->getChainsCount(); i++) {

        vk::DescriptorBufferInfo bufferInfo{};
        bufferInfo.buffer = uniformBuffers[i];
        bufferInfo.offset = 0;
        bufferInfo.range = sizeof(UniformBufferObject);

        vk::DescriptorImageInfo imageInfo{};
        imageInfo.imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
        imageInfo.imageView = texture->textureImageView;
        imageInfo.sampler = texture->textureSampler;

        std::array<vk::WriteDescriptorSet, 2> descriptorWrites{};

        descriptorWrites[0].dstSet = descriptorSets[i];
        descriptorWrites[0].dstBinding = 0;
        descriptorWrites[0].dstArrayElement = 0;
        descriptorWrites[0].descriptorType = vk::DescriptorType::eUniformBuffer;
        descriptorWrites[0].descriptorCount = 1;
        descriptorWrites[0].pBufferInfo = &bufferInfo;

        descriptorWrites[1].dstSet = descriptorSets[i];
        descriptorWrites[1].dstBinding = 1;
        descriptorWrites[1].dstArrayElement = 0;
        descriptorWrites[1].descriptorType = vk::DescriptorType::eCombinedImageSampler;
        descriptorWrites[1].descriptorCount = 1;
        descriptorWrites[1].pImageInfo = &imageInfo;

         vulkanDevice->logicalDevice.updateDescriptorSets(static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);

         //skybox 

         bufferInfo.buffer = skyboxData.skyboxUBOBuffers[i];

         imageInfo.imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;// skyboxData.skybox->textureImageLoyout;
         imageInfo.imageView = skyboxData.skybox->textureImageView;
         imageInfo.sampler = skyboxData.skybox->textureSampler;

         descriptorWrites[0].dstSet = skyboxData.skyboxDescriptorSets[i];
         descriptorWrites[0].pBufferInfo = &bufferInfo;

         descriptorWrites[1].dstSet = skyboxData.skyboxDescriptorSets[i];
         descriptorWrites[1].pImageInfo = &imageInfo;

         vulkanDevice->logicalDevice.updateDescriptorSets(static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
    }
}

void te::VulkanRenderManager::createCommandBuffers()
{
    vulkanDevice->commandBuffers.resize(mySwapChain->getChainsCount());

    vk::CommandBufferAllocateInfo allocInfo{};
    allocInfo.commandPool = vulkanDevice->commandPool;
    allocInfo.level = vk::CommandBufferLevel::ePrimary;
    allocInfo.commandBufferCount = (uint32_t)vulkanDevice->commandBuffers.size();

    if ( vulkanDevice->logicalDevice.allocateCommandBuffers(&allocInfo, vulkanDevice->commandBuffers.data()) != vk::Result::eSuccess) {
        throw std::runtime_error("failed to allocate command buffers!");
    }

    for (size_t i = 0; i < vulkanDevice->commandBuffers.size(); i++) {
        vk::CommandBufferBeginInfo beginInfo{};

        if (vulkanDevice->commandBuffers[i].begin(&beginInfo) != vk::Result::eSuccess) {
            throw std::runtime_error("failed to begin recording command buffer!");
        }

        vk::RenderPassBeginInfo renderPassInfo{};
        renderPassInfo.renderPass = mySwapChain->getRenderPass();
        renderPassInfo.framebuffer = mySwapChain->getFrameBufferByIndex(i);
        renderPassInfo.renderArea.offset = vk::Offset2D{ 0, 0 };
        renderPassInfo.renderArea.extent = mySwapChain->getExtent();

        std::array<vk::ClearValue, 2> clearValues{};
        clearValues[0].color = vk::ClearColorValue{ std::array<float,4>{ 0.0f, 0.0f, 0.0f, 1.0f} };
        clearValues[1].depthStencil = vk::ClearDepthStencilValue{ 1.0f, 0 };

        renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
        renderPassInfo.pClearValues = clearValues.data();

        vulkanDevice->commandBuffers[i].beginRenderPass(&renderPassInfo, vk::SubpassContents::eInline);

       

      
        vk::DeviceSize offsets[] = { 0 };

        vk::Buffer skyboxVertexBuffers[] = { skyboxData.vertexBuffer };
        vk::Buffer vertexBuffers[] = { vertexBuffer };
        vulkanDevice->commandBuffers[i].bindPipeline(vk::PipelineBindPoint::eGraphics, graphicsPipeline);
        vulkanDevice->commandBuffers[i].bindVertexBuffers(0, 1, vertexBuffers, offsets);
        vulkanDevice->commandBuffers[i].bindIndexBuffer(_indexBuffer, 0, vk::IndexType::eUint32);
        vulkanDevice->commandBuffers[i].bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipelineLayout, 0, 1, &descriptorSets[i], 0, nullptr);
        vulkanDevice->commandBuffers[i].drawIndexed(static_cast<uint32_t>(_indices.size()), 1, 0, 0, 0);
       
        //skybox
        vulkanDevice->commandBuffers[i].bindPipeline(vk::PipelineBindPoint::eGraphics, skyboxData.skyboxPipeline);
           
        vulkanDevice->commandBuffers[i].bindVertexBuffers(0, 1, skyboxVertexBuffers, offsets);
        vulkanDevice->commandBuffers[i].bindIndexBuffer(skyboxData._indexBuffer, 0, vk::IndexType::eUint32);
        vulkanDevice->commandBuffers[i].bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipelineLayout, 0, 1, &skyboxData.skyboxDescriptorSets[i], 0, nullptr);
        vulkanDevice->commandBuffers[i].drawIndexed(static_cast<uint32_t>(skyboxData._indices.size()), 1, 0, 0, 0);
        


        vulkanDevice->commandBuffers[i].endRenderPass();
      
        vulkanDevice->commandBuffers[i].end();

    }
}

void te::VulkanRenderManager::createSyncObjects()
{
    synch.imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    synch.renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    synch.inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);
    synch.imagesInFlight.resize(mySwapChain->getChainsCount(), nullptr);

    vk::SemaphoreCreateInfo semaphoreInfo{};
   
    vk::FenceCreateInfo fenceInfo{};
  
    fenceInfo.flags = vk::FenceCreateFlagBits::eSignaled;

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        if ( vulkanDevice->logicalDevice.createSemaphore(&semaphoreInfo, nullptr, &synch.imageAvailableSemaphores[i]) != vk::Result::eSuccess ||
             vulkanDevice->logicalDevice.createSemaphore(&semaphoreInfo, nullptr, &synch.renderFinishedSemaphores[i]) != vk::Result::eSuccess ||
             vulkanDevice->logicalDevice.createFence(&fenceInfo, nullptr, &synch.inFlightFences[i]) != vk::Result::eSuccess) {
            throw std::runtime_error("failed to create synchronization objects for a frame!");
        }
    }
}

void te::VulkanRenderManager::createPipelineCache()
{
    vk::PipelineCacheCreateInfo pipelineCacheCreateInfo = {}; 
     vulkanDevice->logicalDevice.createPipelineCache(&pipelineCacheCreateInfo, nullptr, &pipelineCache);
}

void te::VulkanRenderManager::updateUniformBuffer(uint32_t currentImage)
{

    UniformBufferObject ubo{};

    
    gTransform->onUpdate();

    ubo.model = gTransform->getTransformation();
    //ubo.model = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    //ubo.model = glm::rotate(glm::mat4(1.0f), 1 * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    ubo.view = glm::lookAt(glm::vec3(3.0f, 3.0f, 3.0f), gTransform->getPosition(), glm::vec3(0.0f, 0.0f, 1.0f));
    auto swapChainExtent = mySwapChain->getExtent();
    ubo.proj = glm::perspective(glm::radians(45.0f),  swapChainExtent.width / (float)swapChainExtent.height, 0.1f, 100.0f);
    ubo.proj[1][1] *= -1;
    // ортогональна проекция
   /* const float aspect = (float)window->getFramebufferWidth() / (float)window->getFramebufferWidth();
    ubo.proj = glm::ortho(-1.0f, 1.0f, -1.0f * aspect, 1.0f * aspect, 0.1f, 100.0f);
    ubo.proj[1][1] *= -1;*/

    void* data;
    
     vulkanDevice->logicalDevice.mapMemory(uniformBuffersMemory[currentImage], 0, sizeof(ubo), {}, &data);
     memcpy(data, &ubo, sizeof(ubo));
     vulkanDevice->logicalDevice.unmapMemory(uniformBuffersMemory[currentImage]);

     vulkanDevice->logicalDevice.mapMemory(skyboxData.skyboxUBOBuffersMemory[currentImage], 0, sizeof(ubo), {}, &data);
     memcpy(data, &ubo, sizeof(ubo));
     vulkanDevice->logicalDevice.unmapMemory(skyboxData.skyboxUBOBuffersMemory[currentImage]);
}

void te::VulkanRenderManager::populateDebugMessengerCreateInfo(vk::DebugUtilsMessengerCreateInfoEXT& createInfo)
{
    createInfo = vk::DebugUtilsMessengerCreateInfoEXT{};
   
    createInfo.messageSeverity = vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose |
        vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning |
        vk::DebugUtilsMessageSeverityFlagBitsEXT::eError;

    createInfo.messageType = vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral |
        vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation |
        vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance;

    createInfo.pfnUserCallback = debugCallback;
}

std::vector<const char*> te::VulkanRenderManager::getRequiredExtensions()
{
    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions;
    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

    if (enableValidationLayers) {
        extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }

    return extensions;
}

bool te::VulkanRenderManager::isDeviceSuitable(vk::PhysicalDevice physicalDevice)
{
    te::vkh::QueueFamilyIndices indices =  te::vkh::findQueueFamilies(surface, physicalDevice);

    bool extensionsSupported = checkDeviceExtensionSupport(physicalDevice);

    bool swapChainAdequate = false;
    if (extensionsSupported) {
        te::vkh::SwapChainSupportDetails swapChainSupport = te::vkh::querySwapChainSupport(surface, physicalDevice);
        swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
    }

    vk::PhysicalDeviceFeatures supportedFeatures = physicalDevice.getFeatures();
   
    return indices.isComplete() && extensionsSupported && swapChainAdequate && supportedFeatures.samplerAnisotropy;
}

bool te::VulkanRenderManager::checkDeviceExtensionSupport(vk::PhysicalDevice  physicalDevice)
{
    
    std::vector<vk::ExtensionProperties> availableExtensions = physicalDevice.enumerateDeviceExtensionProperties();

    std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

    for (const auto& extension : availableExtensions) {
        requiredExtensions.erase(extension.extensionName);
    }

    return requiredExtensions.empty();
}





VKAPI_ATTR VkBool32 VKAPI_CALL te::VulkanRenderManager::debugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
    void* pUserData
)
{
    std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;

    return VK_FALSE;
}

vk::Format te::VulkanRenderManager::findDepthFormat()
{
    return findSupportedFormat(
        { 
            vk::Format::eD32Sfloat,
            vk::Format::eD32SfloatS8Uint,
            vk::Format::eD24UnormS8Uint 
        },
        vk::ImageTiling::eOptimal,
        vk::FormatFeatureFlagBits::eDepthStencilAttachment
    );
}


vk::Format te::VulkanRenderManager::findSupportedFormat(const std::vector<vk::Format>& candidates, vk::ImageTiling tiling, vk::FormatFeatureFlags features)
{
    for (vk::Format format : candidates) {
       
       
        vk::FormatProperties props =  vulkanDevice->physicalDevice.getFormatProperties(format);
        if (tiling ==vk::ImageTiling::eLinear  && (props.linearTilingFeatures & features) == features) {
            return format;
        }
        else if (tiling == vk::ImageTiling::eOptimal && (props.optimalTilingFeatures & features) == features) {
            return format;
        }
    }

    throw std::runtime_error("failed to find supported format!");
}




bool te::VulkanRenderManager::checkValidationLayerSupport()
{

    std::vector<vk::LayerProperties> availableLayers = vk::enumerateInstanceLayerProperties();

    for (const char* layerName : validationLayers) {
        bool layerFound = false;

        for (const auto& layerProperties : availableLayers) {
            if (strcmp(layerName, layerProperties.layerName) == 0) {
                layerFound = true;
                break;
            }
        }

        if (!layerFound) {
            return false;
        }
    }

    return true;
}



#pragma region load model data to GPU

void te::VulkanRenderManager::createVertexBuffer(std::vector<te::Vertex> vertices, vk::Buffer& vertexBuffer, vk::DeviceMemory& vertexBufferMemory)
{
     vulkanDevice->createVertexBuffer(
        vertices,
        vertexBuffer,
        vertexBufferMemory,
        instance->vulkanDevice->commandPool,
        instance->vulkanQueues.graphicsQueue      
    );
}

void te::VulkanRenderManager::createIndexBuffer(std::vector<uint32_t> indices, vk::Buffer& indexBuffer, vk::DeviceMemory& indexBufferMemory)
{
     vulkanDevice->createIndexBuffer(
        indices,
        indexBuffer,
        indexBufferMemory,
        instance->vulkanDevice->commandPool,
        instance->vulkanQueues.graphicsQueue
    );
}


#pragma endregion

void te::VulkanRenderManager::recreateSwapChain()
{
    window->windowResizing();

     vulkanDevice->logicalDevice.waitIdle();
    //vkDeviceWaitIdle( vulkanDevice->logicalDevice);

    cleanupSwapChain();

    int width, height;
    window->getFramebufferSize(&width, &height);
    mySwapChain->createSwapChain(width, height);
    createFramebuffers();
    //createSwapChain();
    //createSwapChainImageViews();

    createGraphicsPipeline();
    createDepthResources();
    //createFramebuffers();
    createUniformBuffers();
    createDescriptorPool();
    createDescriptorSets();
    createCommandBuffers();
}



void te::VulkanRenderManager::drawFrame()
{
     vulkanDevice->logicalDevice.waitForFences(1, &synch.inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);
    
    uint32_t imageIndex;
   
    vk::Result result =  vulkanDevice->logicalDevice.acquireNextImageKHR(mySwapChain->getSwapchain(), UINT64_MAX, synch.imageAvailableSemaphores[currentFrame], nullptr, &imageIndex);
   
    if (result == vk::Result::eErrorOutOfDateKHR) {
        recreateSwapChain();
        return;
    }

    else if (result != vk::Result::eSuccess && result !=  vk::Result::eSuboptimalKHR) {
        throw std::runtime_error("failed to acquire swap chain image!");
    }

    updateUniformBuffer(imageIndex);

    if (synch.imagesInFlight[imageIndex] != VK_NULL_HANDLE) {
         vulkanDevice->logicalDevice.waitForFences(1, &synch.imagesInFlight[imageIndex], VK_TRUE, UINT64_MAX);
    }

    synch.imagesInFlight[imageIndex] = synch.inFlightFences[currentFrame];

    vk::SubmitInfo submitInfo{};
  
    vk::Semaphore waitSemaphores[] = { synch.imageAvailableSemaphores[currentFrame] };
    vk::PipelineStageFlags waitStages[] = { vk::PipelineStageFlagBits::eColorAttachmentOutput };
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;

    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &vulkanDevice->commandBuffers[imageIndex];

    vk::Semaphore signalSemaphores[] = { synch.renderFinishedSemaphores[currentFrame] };
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

     vulkanDevice->logicalDevice.resetFences(1, &synch.inFlightFences[currentFrame]);

    if (vulkanQueues.graphicsQueue.submit(1, &submitInfo, synch.inFlightFences[currentFrame]) != vk::Result::eSuccess) {
        throw std::runtime_error("failed to submit draw command buffer!");
    }

    vk::PresentInfoKHR presentInfo{};

    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;

    vk::SwapchainKHR swapChains[] = { mySwapChain->getSwapchain() };
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;

    presentInfo.pImageIndices = &imageIndex;

    result = vulkanQueues.presentQueue.presentKHR(&presentInfo);

    if (result == vk::Result::eErrorOutOfDateKHR || result == vk::Result::eSuboptimalKHR || window->windowResized()) {
        window->windowResizedClear();
        recreateSwapChain();
    }
    else if (result != vk::Result::eSuccess) {
        throw std::runtime_error("failed to present swap chain image!");
    }

    currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}

void te::VulkanRenderManager::cleanupSwapChain()
{


    vulkanDevice->logicalDevice.freeCommandBuffers(vulkanDevice->commandPool, static_cast<uint32_t>(vulkanDevice->commandBuffers.size()), vulkanDevice->commandBuffers.data());

    mySwapChain->destroyFramebuffers();

    vulkanDevice->logicalDevice.destroyPipeline(graphicsPipeline, nullptr);
    vulkanDevice->logicalDevice.destroyPipeline(skyboxData.skyboxPipeline, nullptr);
    vulkanDevice->logicalDevice.destroyPipelineLayout(pipelineLayout, nullptr);


    mySwapChain->destroySwapchain();

    for (size_t i = 0; i < mySwapChain->getChainsCount(); i++) {
        vulkanDevice->logicalDevice.destroyBuffer(uniformBuffers[i], nullptr);
        vulkanDevice->logicalDevice.freeMemory(uniformBuffersMemory[i], nullptr);

        vulkanDevice->logicalDevice.destroyBuffer(skyboxData.skyboxUBOBuffers[i], nullptr);
        vulkanDevice->logicalDevice.freeMemory(skyboxData.skyboxUBOBuffersMemory[i], nullptr);

    }

    vulkanDevice->logicalDevice.destroyDescriptorPool(descriptorPool, nullptr);
}

void te::VulkanRenderManager::terminate()
{
    instance->cleanupSwapChain();

    // удаление даных о текстуре в  GPU

    delete instance->texture;
    delete instance->skyboxData.skybox;


    instance->vulkanDevice->logicalDevice.destroyDescriptorSetLayout(instance->descriptorSetLayout, nullptr);

    instance->vulkanDevice->logicalDevice.destroyBuffer(instance->_indexBuffer, nullptr);
    instance->vulkanDevice->logicalDevice.freeMemory(instance->_indexBufferMemory, nullptr);

    instance->vulkanDevice->logicalDevice.destroyBuffer(instance->vertexBuffer, nullptr);
    instance->vulkanDevice->logicalDevice.freeMemory(instance->vertexBufferMemory, nullptr);

    //destroy skybox cube buffers
    instance->vulkanDevice->logicalDevice.destroyBuffer(instance->skyboxData._indexBuffer, nullptr);
    instance->vulkanDevice->logicalDevice.freeMemory(instance->skyboxData._indexBufferMemory, nullptr);

    instance->vulkanDevice->logicalDevice.destroyBuffer(instance->skyboxData.vertexBuffer, nullptr);
    instance->vulkanDevice->logicalDevice.freeMemory(instance->skyboxData.vertexBufferMemory, nullptr);

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        instance->vulkanDevice->logicalDevice.destroySemaphore(instance->synch.renderFinishedSemaphores[i], nullptr);
        instance->vulkanDevice->logicalDevice.destroySemaphore(instance->synch.imageAvailableSemaphores[i], nullptr);
        instance->vulkanDevice->logicalDevice.destroyFence(instance->synch.inFlightFences[i], nullptr);
    }

    instance->vulkanDevice->logicalDevice.destroyPipelineCache(instance->pipelineCache, nullptr);
    instance->vulkanDevice->logicalDevice.destroyCommandPool(instance->vulkanDevice->commandPool, nullptr);

    instance->vulkanDevice->logicalDevice.destroy();

    if (enableValidationLayers) {
        instance->vulkanDevice->instance.destroyDebugUtilsMessengerEXT(instance->debugMessenger, nullptr);
    }

    delete instance->mySwapChain;

    instance->vulkanDevice->instance.destroy();

    delete instance->vulkanDevice;

    delete instance;

}