
#include "VulkanRenderManager.h"
#include "Vertex.h"
#include "FileReader.h"
#include "VulkanHelper.h"
#include "AssetsLoader.h"
#include "Mesh.h"
//#include <vk_mem_alloc.h>
#include <array>
#pragma region vulkan instancing

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

    if (vk::createInstance(&createInfo, nullptr, &vulkanInstance) != vk::Result::eSuccess) {
        throw std::runtime_error("failed to create instance!");
    }
}

#pragma endregion


#pragma region selection device

void te::VulkanRenderManager::pickPhysicalDevice()
{

    std::vector<vk::PhysicalDevice> devices = te::vkh::VulkanHelper::getPhysicalDevices(vulkanInstance);


    for (const auto& device : devices) {
        if (isDeviceSuitable(device)) {
            physicalDevice = device;
            break;
        }
    }

    if (physicalDevice == VK_NULL_HANDLE) {
        throw std::runtime_error("failed to find a suitable GPU!");
    }

}

void te::VulkanRenderManager::createLogicalDevice()
{
    QueueFamilyIndices indices = findQueueFamilies(physicalDevice);

    std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos;
    std::set<uint32_t> uniqueQueueFamilies = { indices.graphicsFamily.value(), indices.presentFamily.value() };

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
   
    if (physicalDevice.createDevice(&createInfo, nullptr, &device) != vk::Result::eSuccess) {
        throw std::runtime_error("failed to create logical device!");
    }
    
    device.getQueue(indices.graphicsFamily.value(), 0, &graphicsQueue);
    device.getQueue(indices.presentFamily.value(), 0, &presentQueue);
   
}
#pragma endregion

void te::VulkanRenderManager::initialize(te::Window* wnd) {


    instance = new VulkanRenderManager();
    instance->window = wnd;
    VULKAN_HPP_DEFAULT_DISPATCHER.init(vkGetInstanceProcAddr);
   
    instance->createInstance();

    VULKAN_HPP_DEFAULT_DISPATCHER.init(instance->vulkanInstance);

    instance->setupDebugMessenger();
    instance->createSurface();
    instance->pickPhysicalDevice();
    instance->createLogicalDevice();

    instance->createVulkanMemoryAllocator();
    instance->createSwapChain();
    instance->createSwapChainImageViews();
    instance->createRenderPass();
    instance->createDescriptorSetLayout();
    instance->createGraphicsPipeline();
    instance->createCommandPool();
    instance->createDepthResources();
    instance->createFramebuffers();
    instance->createTextureImage(instance->textureImage, instance->mipLevels);
    instance->createTextureImageView(instance->textureImageView, instance->textureImage, instance->mipLevels);
    instance->createTextureSampler();
    instance->loadModel();
    instance->gObject = new Entity();
    instance->gTransform = (Transform*)instance->gObject->getComponent(typeid(Transform).hash_code());
    instance->gTransform->onAwake();

    instance->createVertexBuffer(
        instance->vertices,
        instance->vertexBuffer,
        instance->vertexBufferMemory

    );

    instance->createIndexBuffer(
        instance->_indices,
        instance->_indexBuffer,
        instance->_indexBufferMemory
    );

    instance->createUniformBuffers();
    instance->createDescriptorPool();
    instance->createDescriptorSets();
    instance->createCommandBuffers();
    instance->createSyncObjects();

}
void te::VulkanRenderManager::terminate()
{
    instance->cleanupSwapChain();

    // удаление даных о текстуре в  GPU
    instance->device.destroySampler(instance->textureSampler, nullptr);
    instance->device.destroyImageView(instance->textureImageView, nullptr);
    instance->device.destroyImage(instance->textureImage, nullptr);
    instance->device.freeMemory(instance->textureImageMemory, nullptr);


    instance->device.destroyDescriptorSetLayout(instance->descriptorSetLayout, nullptr);

    instance->device.destroyBuffer(instance->_indexBuffer, nullptr);
    instance->device.freeMemory(instance->_indexBufferMemory, nullptr);

    instance->device.destroyBuffer(instance->vertexBuffer, nullptr);
    instance->device.freeMemory(instance->vertexBufferMemory, nullptr);

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        instance->device.destroySemaphore(instance->renderFinishedSemaphores[i], nullptr);
        instance->device.destroySemaphore(instance->imageAvailableSemaphores[i], nullptr);
        instance->device.destroyFence(instance->inFlightFences[i], nullptr);
    }

    instance->device.destroyCommandPool(instance->commandPool, nullptr);

    instance->device.destroy();

    if (enableValidationLayers) {
        instance->vulkanInstance.destroyDebugUtilsMessengerEXT(instance->debugMessenger, nullptr);
    }


    instance->vulkanInstance.destroySurfaceKHR(instance->surface, nullptr);
    instance->vulkanInstance.destroy();

}
#pragma region vulkan validation layer (debug)
void te::VulkanRenderManager::setupDebugMessenger()
{
    if (!enableValidationLayers) return;

    vk::DebugUtilsMessengerCreateInfoEXT createInfo;
    populateDebugMessengerCreateInfo(createInfo);

    if (vulkanInstance.createDebugUtilsMessengerEXT(&createInfo, nullptr, &debugMessenger) != vk::Result::eSuccess) {
        throw std::runtime_error("failed to set up debug messenger!");
    }

}

#pragma endregion

#pragma region drawing surface

void te::VulkanRenderManager::createSurface()
{
    window->createSurface(vulkanInstance, &surface, nullptr);
}

void te::VulkanRenderManager::createSwapChain()
{
    SwapChainSupportDetails swapChainSupport = querySwapChainSupport(physicalDevice);

    vk::SurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
    vk::PresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
    vk::Extent2D extent = chooseSwapExtent(swapChainSupport.capabilities);

    uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
    if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount) {
        imageCount = swapChainSupport.capabilities.maxImageCount;
    }

    vk::SwapchainCreateInfoKHR createInfo{};
   
    createInfo.surface = surface;

    createInfo.minImageCount = imageCount;
    createInfo.imageFormat = surfaceFormat.format;
    createInfo.imageColorSpace = surfaceFormat.colorSpace;
    createInfo.imageExtent = extent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = vk::ImageUsageFlagBits::eColorAttachment;

    QueueFamilyIndices indices = findQueueFamilies(physicalDevice);
    uint32_t queueFamilyIndices[] = { indices.graphicsFamily.value(), indices.presentFamily.value() };

    if (indices.graphicsFamily != indices.presentFamily) {
        createInfo.imageSharingMode = vk::SharingMode::eConcurrent;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices = queueFamilyIndices;
    }
    else {
        createInfo.imageSharingMode = vk::SharingMode::eExclusive;
    }

    createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
    createInfo.compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque;
    createInfo.presentMode = presentMode;
    createInfo.clipped = VK_TRUE;

    if (device.createSwapchainKHR(&createInfo, nullptr, &swapChain) != vk::Result::eSuccess) {
        throw std::runtime_error("failed to create swap chain!");
    }

    swapChainImages = device.getSwapchainImagesKHR(swapChain);

    swapChainImageFormat = surfaceFormat.format;
    swapChainExtent = extent;
}

void te::VulkanRenderManager::createSwapChainImageViews()
{
    swapChainImageViews.resize(swapChainImages.size());

    for (uint32_t i = 0; i < swapChainImages.size(); i++) {
        swapChainImageViews[i] = createImageView(swapChainImages[i], swapChainImageFormat, vk::ImageAspectFlagBits::eColor, 1);
    }
}

#pragma endregion


#pragma region render pipeline

#pragma endregion


void te::VulkanRenderManager::createRenderPass()
{
    vk::AttachmentDescription colorAttachment{};
    colorAttachment.format = swapChainImageFormat;
    colorAttachment.samples = vk::SampleCountFlagBits::e1;
    colorAttachment.loadOp = vk::AttachmentLoadOp::eClear;
    colorAttachment.storeOp = vk::AttachmentStoreOp::eStore;
    colorAttachment.stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
    colorAttachment.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
    colorAttachment.initialLayout = vk::ImageLayout::eUndefined; 
    colorAttachment.finalLayout = vk::ImageLayout::ePresentSrcKHR;

    vk::AttachmentDescription depthAttachment{};
    depthAttachment.format = findDepthFormat();
    depthAttachment.samples = vk::SampleCountFlagBits::e1;
    depthAttachment.loadOp = vk::AttachmentLoadOp::eClear;
    depthAttachment.storeOp = vk::AttachmentStoreOp::eDontCare;
    depthAttachment.stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
    depthAttachment.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
    depthAttachment.initialLayout = vk::ImageLayout::eUndefined;
    depthAttachment.finalLayout = vk::ImageLayout::eDepthStencilAttachmentOptimal;

    vk::AttachmentReference colorAttachmentRef{};
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = vk::ImageLayout::eColorAttachmentOptimal;

    vk::AttachmentReference depthAttachmentRef{};
    depthAttachmentRef.attachment = 1;
    depthAttachmentRef.layout = vk::ImageLayout::eDepthStencilAttachmentOptimal;

    vk::SubpassDescription subpass{};
    subpass.pipelineBindPoint = vk::PipelineBindPoint::eGraphics;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;
    subpass.pDepthStencilAttachment = &depthAttachmentRef;

    vk::SubpassDependency dependency{};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput | vk::PipelineStageFlagBits::eEarlyFragmentTests; 
    dependency.srcAccessMask = {};
    dependency.dstStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput | vk::PipelineStageFlagBits::eEarlyFragmentTests;
    dependency.dstAccessMask = vk::AccessFlagBits::eColorAttachmentWrite | vk::AccessFlagBits::eDepthStencilAttachmentWrite;

    std::array<vk::AttachmentDescription, 2> attachments = { colorAttachment, depthAttachment };
    vk::RenderPassCreateInfo renderPassInfo{};
  
    renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
    renderPassInfo.pAttachments = attachments.data();
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;
    renderPassInfo.dependencyCount = 1;
    renderPassInfo.pDependencies = &dependency;

    if (device.createRenderPass(&renderPassInfo, nullptr, &renderPass) != vk::Result::eSuccess) {
        throw std::runtime_error("failed to create render pass!");
    }
}

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

    if (device.createDescriptorSetLayout(&layoutInfo, nullptr, &descriptorSetLayout) != vk::Result::eSuccess) {
        throw std::runtime_error("failed to create descriptor set layout!");
    }
}

void te::VulkanRenderManager::createGraphicsPipeline()
{
    auto vertShaderCode = te::FileReader::readFile("shaders/vert.spv");
    auto fragShaderCode = te::FileReader::readFile("shaders/frag.spv");

    vk::ShaderModule vertShaderModule = createShaderModule(vertShaderCode);
    vk::ShaderModule fragShaderModule = createShaderModule(fragShaderCode);

    vk::PipelineShaderStageCreateInfo vertShaderStageInfo{};
   
    vertShaderStageInfo.stage = vk::ShaderStageFlagBits::eVertex;
    vertShaderStageInfo.module = vertShaderModule;
    vertShaderStageInfo.pName = "main";

    vk::PipelineShaderStageCreateInfo fragShaderStageInfo{};
    
    fragShaderStageInfo.stage = vk::ShaderStageFlagBits::eFragment;
    fragShaderStageInfo.module = fragShaderModule;
    fragShaderStageInfo.pName = "main";

    vk::PipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo, fragShaderStageInfo };

    vk::PipelineVertexInputStateCreateInfo vertexInputInfo{};

    auto bindingDescription = te::Vertex::getBindingDescription();
    auto attributeDescriptions = te::Vertex::getAttributeDescriptions();

    vertexInputInfo.vertexBindingDescriptionCount = 1;
    vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
    vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
    vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

    vk::PipelineInputAssemblyStateCreateInfo inputAssembly{};
  
    inputAssembly.topology = vk::PrimitiveTopology::eTriangleList;
    inputAssembly.primitiveRestartEnable = VK_FALSE;

    vk::Viewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = (float)swapChainExtent.width;
    viewport.height = (float)swapChainExtent.height;
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

    vk::PipelineRasterizationStateCreateInfo rasterizer{};

    rasterizer.depthClampEnable = VK_FALSE;
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    rasterizer.polygonMode = vk::PolygonMode::eFill;
    rasterizer.lineWidth = 1.0f;
    rasterizer.cullMode = vk::CullModeFlagBits::eBack;
    rasterizer.frontFace = vk::FrontFace::eCounterClockwise;
    rasterizer.depthBiasEnable = VK_FALSE;

    vk::PipelineMultisampleStateCreateInfo multisampling{};
   
    multisampling.sampleShadingEnable = VK_FALSE;
    multisampling.rasterizationSamples = vk::SampleCountFlagBits::e1;

    vk::PipelineDepthStencilStateCreateInfo depthStencil{};
  
    depthStencil.depthTestEnable = VK_TRUE;
    depthStencil.depthWriteEnable = VK_TRUE;
    depthStencil.depthCompareOp = vk::CompareOp::eLess;
    depthStencil.depthBoundsTestEnable = VK_FALSE;
    depthStencil.stencilTestEnable = VK_FALSE;

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

    if (device.createPipelineLayout(&pipelineLayoutInfo, nullptr, &pipelineLayout) != vk::Result::eSuccess) {
        throw std::runtime_error("failed to create pipeline layout!");
    }

    vk::GraphicsPipelineCreateInfo pipelineInfo{};

    pipelineInfo.stageCount = 2;
    pipelineInfo.pStages = shaderStages;
    pipelineInfo.pVertexInputState = &vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pMultisampleState = &multisampling;
    pipelineInfo.pDepthStencilState = &depthStencil;
    pipelineInfo.pColorBlendState = &colorBlending;
    pipelineInfo.layout = pipelineLayout;
    pipelineInfo.renderPass = renderPass;
    pipelineInfo.subpass = 0;
    pipelineInfo.basePipelineHandle = nullptr;

    if (device.createGraphicsPipelines(nullptr, 1, &pipelineInfo, nullptr, &graphicsPipeline) != vk::Result::eSuccess) {
        throw std::runtime_error("failed to create graphics pipeline!");
    }

    vkDestroyShaderModule(device, fragShaderModule, nullptr);
    vkDestroyShaderModule(device, vertShaderModule, nullptr);
}

void te::VulkanRenderManager::createCommandPool()
{
    QueueFamilyIndices queueFamilyIndices = findQueueFamilies(physicalDevice);

    vk::CommandPoolCreateInfo poolInfo{};
   
    poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();

    if (device.createCommandPool(&poolInfo, nullptr, &commandPool) != vk::Result::eSuccess) {
        throw std::runtime_error("failed to create graphics command pool!");
    }
}

void te::VulkanRenderManager::createDepthResources()
{
    vk::Format depthFormat = findDepthFormat();

    createImage(
        swapChainExtent.width,
        swapChainExtent.height,
        1, depthFormat, vk::ImageTiling::eOptimal,
        vk::ImageUsageFlagBits::eDepthStencilAttachment,
        vk::MemoryPropertyFlagBits::eDeviceLocal,
        depthImage, depthImageMemory
    );
    depthImageView = createImageView(
        depthImage, depthFormat,
        vk::ImageAspectFlagBits::eDepth, 1);
}

void te::VulkanRenderManager::createFramebuffers()
{
    swapChainFramebuffers.resize(swapChainImageViews.size());

    for (size_t i = 0; i < swapChainImageViews.size(); i++) {
        std::array<vk::ImageView, 2> attachments = {
            swapChainImageViews[i],
            depthImageView
        };

        vk::FramebufferCreateInfo framebufferInfo{};
       
        framebufferInfo.renderPass = renderPass;
        framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
        framebufferInfo.pAttachments = attachments.data();
        framebufferInfo.width = swapChainExtent.width;
        framebufferInfo.height = swapChainExtent.height;
        framebufferInfo.layers = 1;

        if (device.createFramebuffer(&framebufferInfo, nullptr, &swapChainFramebuffers[i]) != vk::Result::eSuccess) {
            throw std::runtime_error("failed to create framebuffer!");
        }
    }
}

#pragma region Load texture to gpu


void te::VulkanRenderManager::createTextureImage(vk::Image& textureImage, uint32_t& mipLevels)
{
    int texWidth, texHeight, texChannels;
    stbi_uc* pixels = stbi_load(TEXTURE_PATH.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
    vk::DeviceSize imageSize = texWidth * texHeight * 4;
    mipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(texWidth, texHeight)))) + 1;

    if (!pixels) {
        throw std::runtime_error("failed to load texture image!");
    }

    vk::Buffer stagingBuffer;
    vk::DeviceMemory stagingBufferMemory;
    te::vkh::VulkanHelper::createBuffer(
        imageSize,
        vk::BufferUsageFlagBits::eTransferSrc,
        vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent,
        stagingBuffer, stagingBufferMemory, physicalDevice, device);

    void* data;
    device.mapMemory(stagingBufferMemory, 0, imageSize, {}, &data);
    memcpy(data, pixels, static_cast<size_t>(imageSize));
    device.unmapMemory(stagingBufferMemory);

    stbi_image_free(pixels);

    createImage(
        texWidth, texHeight, mipLevels,
        vk::Format::eR8G8B8A8Srgb,
        vk::ImageTiling::eOptimal,
        vk::ImageUsageFlagBits::eTransferSrc |
        vk::ImageUsageFlagBits::eTransferDst |
        vk::ImageUsageFlagBits::eSampled,
        vk::MemoryPropertyFlagBits::eDeviceLocal,
        textureImage, textureImageMemory
    );


    transitionImageLayout(
        textureImage,
        vk::Format::eR8G8B8A8Srgb,
        vk::ImageLayout::eUndefined,
        vk::ImageLayout::eTransferDstOptimal,
        mipLevels
    );

    te::vkh::VulkanHelper::copyBufferToImage(
        stagingBuffer,
        textureImage,
        static_cast<uint32_t>(texWidth),
        static_cast<uint32_t>(texHeight),
        commandPool,
        graphicsQueue,
        device
    );
   
    vkDestroyBuffer(device, stagingBuffer, nullptr);
    vkFreeMemory(device, stagingBufferMemory, nullptr);

    generateMipmaps(
        textureImage,
        vk::Format::eR8G8B8A8Srgb,
        texWidth,
        texHeight,
        mipLevels
    );
}

void te::VulkanRenderManager::createTextureImageView(vk::ImageView& imgView, const vk::Image textureImage, const uint32_t mipLevels)
{
    imgView = createImageView(
        textureImage,
        vk::Format::eR8G8B8A8Srgb,
        vk::ImageAspectFlagBits::eColor,
        mipLevels
    );
}

void te::VulkanRenderManager::createTextureSampler()
{
    vk::PhysicalDeviceProperties properties = physicalDevice.getProperties();


    vk::SamplerCreateInfo samplerInfo{};
  
    samplerInfo.magFilter = vk::Filter::eLinear;
    samplerInfo.minFilter = vk::Filter::eLinear;
    samplerInfo.addressModeU = vk::SamplerAddressMode::eRepeat;
    samplerInfo.addressModeV = vk::SamplerAddressMode::eRepeat;
    samplerInfo.addressModeW = vk::SamplerAddressMode::eRepeat;
    samplerInfo.anisotropyEnable = VK_TRUE;
    samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;
    samplerInfo.borderColor = vk::BorderColor::eIntOpaqueBlack;;
    samplerInfo.unnormalizedCoordinates = VK_FALSE;
    samplerInfo.compareEnable = VK_FALSE;
    samplerInfo.compareOp = vk::CompareOp::eAlways;;
    samplerInfo.mipmapMode = vk::SamplerMipmapMode::eLinear;;
    samplerInfo.minLod = 0.0f;
    samplerInfo.maxLod = static_cast<float>(mipLevels);
    samplerInfo.mipLodBias = 0.0f;

    if (device.createSampler(&samplerInfo, nullptr, &textureSampler) != vk::Result::eSuccess) {
        throw std::runtime_error("failed to create texture sampler!");
    }
}

void te::VulkanRenderManager::generateMipmaps(vk::Image image, vk::Format imageFormat, int32_t texWidth, int32_t texHeight, uint32_t mipLevels)
{
    vk::FormatProperties formatProperties = physicalDevice.getFormatProperties(imageFormat);  

    if (!(formatProperties.optimalTilingFeatures & vk::FormatFeatureFlagBits::eSampledImageFilterLinear)) {
        throw std::runtime_error("texture image format does not support linear blitting!");
    }

    vk::CommandBuffer commandBuffer = te::vkh::VulkanHelper::beginSingleTimeCommands(commandPool, device);

    vk::ImageMemoryBarrier barrier{};
 
    barrier.image = image;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;
    barrier.subresourceRange.levelCount = 1;

    int32_t mipWidth = texWidth;
    int32_t mipHeight = texHeight;

    for (uint32_t i = 1; i < mipLevels; i++) {
        barrier.subresourceRange.baseMipLevel = i - 1;
        barrier.oldLayout = vk::ImageLayout::eTransferDstOptimal;
        barrier.newLayout = vk::ImageLayout::eTransferSrcOptimal;
        barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
        barrier.dstAccessMask = vk::AccessFlagBits::eTransferRead;

        commandBuffer.pipelineBarrier(
            vk::PipelineStageFlagBits::eTransfer,
            vk::PipelineStageFlagBits::eTransfer,
            {}, 0, nullptr, 0,  nullptr, 1, &barrier
                
        );

        vk::ImageBlit blit{};
        blit.srcOffsets[0] = vk::Offset3D{ 0, 0, 0 };
        blit.srcOffsets[1] = vk::Offset3D{ mipWidth, mipHeight, 1 };
        blit.srcSubresource.aspectMask = vk::ImageAspectFlagBits::eColor; ;
        blit.srcSubresource.mipLevel = i - 1;
        blit.srcSubresource.baseArrayLayer = 0;
        blit.srcSubresource.layerCount = 1;
        blit.dstOffsets[0] = vk::Offset3D{ 0, 0, 0 };
        blit.dstOffsets[1] = vk::Offset3D{ mipWidth > 1 ? mipWidth / 2 : 1, mipHeight > 1 ? mipHeight / 2 : 1, 1 };
        blit.dstSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
        blit.dstSubresource.mipLevel = i;
        blit.dstSubresource.baseArrayLayer = 0;
        blit.dstSubresource.layerCount = 1;

        commandBuffer.blitImage(
            image, vk::ImageLayout::eTransferSrcOptimal,
            image, vk::ImageLayout::eTransferDstOptimal,
            1, &blit, vk::Filter::eLinear
        );



        barrier.oldLayout = vk::ImageLayout::eTransferSrcOptimal;
        barrier.newLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
        barrier.srcAccessMask = vk::AccessFlagBits::eTransferRead;
        barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;
    

        commandBuffer.pipelineBarrier(
            vk::PipelineStageFlagBits::eTransfer,
            vk::PipelineStageFlagBits::eFragmentShader,
            {},
            0, nullptr, 0, nullptr, 1, &barrier

        );

        if (mipWidth > 1) mipWidth /= 2;
        if (mipHeight > 1) mipHeight /= 2;
    }

    barrier.subresourceRange.baseMipLevel = mipLevels - 1;
    barrier.oldLayout = vk::ImageLayout::eTransferDstOptimal; 
    barrier.newLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
    barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite; 
    barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;


    commandBuffer.pipelineBarrier(
        vk::PipelineStageFlagBits::eTransfer,
        vk::PipelineStageFlagBits::eFragmentShader,
        {}, 0, nullptr, 0, nullptr, 1, &barrier
    );

    te::vkh::VulkanHelper::endSingleTimeCommands(commandBuffer, graphicsQueue, commandPool, device);
}

void te::VulkanRenderManager::transitionImageLayout(vk::Image image, vk::Format format, vk::ImageLayout oldLayout, vk::ImageLayout newLayout, uint32_t mipLevels)
{
    vk::CommandBuffer commandBuffer = te::vkh::VulkanHelper::beginSingleTimeCommands(commandPool, device);

    vk::ImageMemoryBarrier barrier{};
   
    barrier.oldLayout = oldLayout;
    barrier.newLayout = newLayout;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = image;
    barrier.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = mipLevels;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;

    vk::PipelineStageFlags sourceStage;
    vk::PipelineStageFlags destinationStage;

    if (oldLayout == vk::ImageLayout::eUndefined &&
        newLayout == vk::ImageLayout::eTransferDstOptimal) {
        barrier.srcAccessMask = {}; 
        barrier.dstAccessMask = vk::AccessFlagBits::eTransferWrite;

        sourceStage = vk::PipelineStageFlagBits::eTopOfPipe; 
        destinationStage = vk::PipelineStageFlagBits::eTransfer;
    }
    else if (oldLayout == vk::ImageLayout::eTransferDstOptimal &&
        newLayout == vk::ImageLayout::eShaderReadOnlyOptimal) {
        barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
        barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;

        sourceStage = vk::PipelineStageFlagBits::eTransfer;
        destinationStage = vk::PipelineStageFlagBits::eFragmentShader;;
    }
    else {
        throw std::invalid_argument("unsupported layout transition!");
    }

    commandBuffer.pipelineBarrier(
        sourceStage,
        destinationStage,
        {},
        0, nullptr, 0, nullptr, 1, &barrier

    );

    te::vkh::VulkanHelper::endSingleTimeCommands(commandBuffer, graphicsQueue, commandPool, device);
}


vk::ImageView te::VulkanRenderManager::createImageView(vk::Image image, vk::Format format, vk::ImageAspectFlags aspectFlags, uint32_t mipLevels)
{
    vk::ImageViewCreateInfo viewInfo{};
  
    viewInfo.image = image;
    viewInfo.viewType = vk::ImageViewType::e2D;
    viewInfo.format = format;
    viewInfo.subresourceRange.aspectMask = aspectFlags;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.levelCount = mipLevels;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount = 1;

    vk::ImageView imageView;
    if (device.createImageView(&viewInfo, nullptr, &imageView) != vk::Result::eSuccess) {
        throw std::runtime_error("failed to create texture image view!");
    }

    return imageView;
}


#pragma endregion




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
    allocatorInfo.physicalDevice = physicalDevice;
    allocatorInfo.device = device;
    allocatorInfo.instance = instance;
   
    VmaAllocator allocator;
    vmaCreateAllocator(&allocatorInfo, &allocator);*/
}


void te::VulkanRenderManager::createUniformBuffers()
{
    vk::DeviceSize bufferSize = sizeof(UniformBufferObject);

    uniformBuffers.resize(swapChainImages.size());
    uniformBuffersMemory.resize(swapChainImages.size());

    for (size_t i = 0; i < swapChainImages.size(); i++) {
        te::vkh::VulkanHelper::createBuffer(
            bufferSize,
            vk::BufferUsageFlagBits::eUniformBuffer,
            vk::MemoryPropertyFlagBits::eHostVisible  |
            vk::MemoryPropertyFlagBits::eHostCoherent,
            uniformBuffers[i], uniformBuffersMemory[i], physicalDevice, device
        );
    }
}

void te::VulkanRenderManager::createDescriptorPool()
{
    std::array<vk::DescriptorPoolSize, 2> poolSizes{};
    poolSizes[0].type = vk::DescriptorType::eUniformBuffer;
    poolSizes[0].descriptorCount = static_cast<uint32_t>(swapChainImages.size());
    poolSizes[1].type = vk::DescriptorType::eCombinedImageSampler;
    poolSizes[1].descriptorCount = static_cast<uint32_t>(swapChainImages.size());

    vk::DescriptorPoolCreateInfo poolInfo{};
   
    poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
    poolInfo.pPoolSizes = poolSizes.data();
    poolInfo.maxSets = static_cast<uint32_t>(swapChainImages.size());

    if (device.createDescriptorPool(&poolInfo, nullptr, &descriptorPool) != vk::Result::eSuccess) {
        throw std::runtime_error("failed to create descriptor pool!");
    }
}

void te::VulkanRenderManager::createDescriptorSets()
{
    std::vector<vk::DescriptorSetLayout> layouts(swapChainImages.size(), descriptorSetLayout);
    vk::DescriptorSetAllocateInfo allocInfo{};
   
    allocInfo.descriptorPool = descriptorPool;
    allocInfo.descriptorSetCount = static_cast<uint32_t>(swapChainImages.size());
    allocInfo.pSetLayouts = layouts.data();

    descriptorSets.resize(swapChainImages.size());
    if (device.allocateDescriptorSets(&allocInfo, descriptorSets.data()) != vk::Result::eSuccess) {
        throw std::runtime_error("failed to allocate descriptor sets!");
    }

    for (size_t i = 0; i < swapChainImages.size(); i++) {
        vk::DescriptorBufferInfo bufferInfo{};
        bufferInfo.buffer = uniformBuffers[i];
        bufferInfo.offset = 0;
        bufferInfo.range = sizeof(UniformBufferObject);

        vk::DescriptorImageInfo imageInfo{};
        imageInfo.imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
        imageInfo.imageView = textureImageView;
        imageInfo.sampler = textureSampler;

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
        descriptorWrites[1].descriptorType = vk::DescriptorType::eCombinedImageSampler;;
        descriptorWrites[1].descriptorCount = 1;
        descriptorWrites[1].pImageInfo = &imageInfo;

        device.updateDescriptorSets(static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
    }
}

void te::VulkanRenderManager::createCommandBuffers()
{
    commandBuffers.resize(swapChainFramebuffers.size());

    vk::CommandBufferAllocateInfo allocInfo{};
    allocInfo.commandPool = commandPool;
    allocInfo.level = vk::CommandBufferLevel::ePrimary;;
    allocInfo.commandBufferCount = (uint32_t)commandBuffers.size();

    if (device.allocateCommandBuffers(&allocInfo, commandBuffers.data()) != vk::Result::eSuccess) {
        throw std::runtime_error("failed to allocate command buffers!");
    }

    for (size_t i = 0; i < commandBuffers.size(); i++) {
        vk::CommandBufferBeginInfo beginInfo{};

        if (commandBuffers[i].begin(&beginInfo) != vk::Result::eSuccess) {
            throw std::runtime_error("failed to begin recording command buffer!");
        }

        vk::RenderPassBeginInfo renderPassInfo{};
        renderPassInfo.renderPass = renderPass;
        renderPassInfo.framebuffer = swapChainFramebuffers[i];
        renderPassInfo.renderArea.offset = vk::Offset2D{ 0, 0 };
        renderPassInfo.renderArea.extent = swapChainExtent;

        std::array<vk::ClearValue, 2> clearValues{};
        clearValues[0].color = vk::ClearColorValue{ std::array<float,4>{ 0.0f, 0.0f, 0.0f, 1.0f} };
        clearValues[1].depthStencil = vk::ClearDepthStencilValue{ 1.0f, 0 };

        renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
        renderPassInfo.pClearValues = clearValues.data();

        commandBuffers[i].beginRenderPass(&renderPassInfo, vk::SubpassContents::eInline);

        commandBuffers[i].bindPipeline(vk::PipelineBindPoint::eGraphics, graphicsPipeline);

        vk::Buffer vertexBuffers[] = { vertexBuffer };
        vk::DeviceSize offsets[] = { 0 };

        commandBuffers[i].bindVertexBuffers(0, 1, vertexBuffers, offsets);

        commandBuffers[i].bindIndexBuffer(_indexBuffer, 0, vk::IndexType::eUint32);
        
        commandBuffers[i].bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipelineLayout, 0, 1, &descriptorSets[i], 0, nullptr);

        commandBuffers[i].drawIndexed(static_cast<uint32_t>(_indices.size()), 1, 0, 0, 0);

        vkCmdEndRenderPass(commandBuffers[i]);

        
        commandBuffers[i].end();

    }
}

void te::VulkanRenderManager::createSyncObjects()
{
    imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);
    imagesInFlight.resize(swapChainImages.size(), nullptr);

    vk::SemaphoreCreateInfo semaphoreInfo{};

    vk::FenceCreateInfo fenceInfo{};
  
    fenceInfo.flags = vk::FenceCreateFlagBits::eSignaled;

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        if (device.createSemaphore(&semaphoreInfo, nullptr, &imageAvailableSemaphores[i]) != vk::Result::eSuccess ||
            device.createSemaphore(&semaphoreInfo, nullptr, &renderFinishedSemaphores[i]) != vk::Result::eSuccess ||
            device.createFence(&fenceInfo, nullptr, &inFlightFences[i]) != vk::Result::eSuccess) {
            throw std::runtime_error("failed to create synchronization objects for a frame!");
        }
    }
}

void te::VulkanRenderManager::updateUniformBuffer(uint32_t currentImage)
{

    UniformBufferObject ubo{};

    
    gTransform->onUpdate();

    ubo.model = gTransform->getTransformation();
    //ubo.model = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    //ubo.model = glm::rotate(glm::mat4(1.0f), 1 * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    ubo.view = glm::lookAt(glm::vec3(3.0f, 3.0f, 3.0f), gTransform->getPosition(), glm::vec3(0.0f, 0.0f, 1.0f));

    ubo.proj = glm::perspective(glm::radians(45.0f), swapChainExtent.width / (float)swapChainExtent.height, 0.1f, 100.0f);
    ubo.proj[1][1] *= -1;
    // ортогональна проекция
   /* const float aspect = (float)window->getFramebufferWidth() / (float)window->getFramebufferWidth();
    ubo.proj = glm::ortho(-1.0f, 1.0f, -1.0f * aspect, 1.0f * aspect, 0.1f, 100.0f);
    ubo.proj[1][1] *= -1;*/

    void* data;
    
    device.mapMemory(uniformBuffersMemory[currentImage], 0, sizeof(ubo), {}, &data);
    memcpy(data, &ubo, sizeof(ubo));
    device.unmapMemory(uniformBuffersMemory[currentImage]);
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

bool te::VulkanRenderManager::isDeviceSuitable(vk::PhysicalDevice device)
{
    QueueFamilyIndices indices = findQueueFamilies(device);

    bool extensionsSupported = checkDeviceExtensionSupport(device);

    bool swapChainAdequate = false;
    if (extensionsSupported) {
        SwapChainSupportDetails swapChainSupport = querySwapChainSupport(device);
        swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
    }

    vk::PhysicalDeviceFeatures supportedFeatures = device.getFeatures();
   
    return indices.isComplete() && extensionsSupported && swapChainAdequate && supportedFeatures.samplerAnisotropy;
}

QueueFamilyIndices te::VulkanRenderManager::findQueueFamilies(vk::PhysicalDevice device)
{
    QueueFamilyIndices indices;


    std::vector<vk::QueueFamilyProperties> queueFamilies = device.getQueueFamilyProperties();
    int i = 0;
    for (const auto& queueFamily : queueFamilies) {
        if (queueFamily.queueFlags & vk::QueueFlagBits::eGraphics) {
            indices.graphicsFamily = i;
        }

        vk::Bool32 presentSupport = false;
        device.getSurfaceSupportKHR(i, surface, &presentSupport);

        if (presentSupport) {
            indices.presentFamily = i;
        }

        if (indices.isComplete()) {
            break;
        }

        i++;
    }

    return indices;
}

SwapChainSupportDetails te::VulkanRenderManager::querySwapChainSupport(vk::PhysicalDevice device)
{
    SwapChainSupportDetails details;
   
    device.getSurfaceCapabilitiesKHR(surface, &details.capabilities);

    details.formats = device.getSurfaceFormatsKHR(surface);

    details.presentModes = device.getSurfacePresentModesKHR(surface);

    return details;
}

vk::SurfaceFormatKHR te::VulkanRenderManager::chooseSwapSurfaceFormat(const std::vector<vk::SurfaceFormatKHR>& availableFormats)
{
    for (const auto& availableFormat : availableFormats) {
        if (availableFormat.format == vk::Format::eB8G8R8A8Srgb &&
            availableFormat.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear ) {
            return availableFormat;
        }
    }

    return availableFormats[0];
}

bool te::VulkanRenderManager::checkDeviceExtensionSupport(vk::PhysicalDevice device)
{
    
    std::vector<vk::ExtensionProperties> availableExtensions = device.enumerateDeviceExtensionProperties();

    std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

    for (const auto& extension : availableExtensions) {
        requiredExtensions.erase(extension.extensionName);
    }

    return requiredExtensions.empty();
}

vk::PresentModeKHR te::VulkanRenderManager::chooseSwapPresentMode(const std::vector<vk::PresentModeKHR>& availablePresentModes)
{
    for (const auto& availablePresentMode : availablePresentModes) {
        if (availablePresentMode == vk::PresentModeKHR::eMailbox) {
            return availablePresentMode;
        }
    }

    return vk::PresentModeKHR::eFifo;
}

vk::Extent2D te::VulkanRenderManager::chooseSwapExtent(const vk::SurfaceCapabilitiesKHR& capabilities)
{
    if (capabilities.currentExtent.width != UINT32_MAX) {
        return capabilities.currentExtent;
    }
    else {
        int width, height;
        window->getFramebufferSize(&width, &height);

        vk::Extent2D actualExtent = {
            static_cast<uint32_t>(width),
            static_cast<uint32_t>(height)
        };

        actualExtent.width = std::max(capabilities.minImageExtent.width, std::min(capabilities.maxImageExtent.width, actualExtent.width));
        actualExtent.height = std::max(capabilities.minImageExtent.height, std::min(capabilities.maxImageExtent.height, actualExtent.height));

        return actualExtent;
    }
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

uint32_t te::VulkanRenderManager::findMemoryType(uint32_t typeFilter, vk::MemoryPropertyFlags properties)
{

    vk::PhysicalDeviceMemoryProperties memProperties = physicalDevice.getMemoryProperties();

    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
        if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
            return i;
        }
    }

    throw std::runtime_error("failed to find suitable memory type!");
}

vk::Format te::VulkanRenderManager::findSupportedFormat(const std::vector<vk::Format>& candidates, vk::ImageTiling tiling, vk::FormatFeatureFlags features)
{
    for (vk::Format format : candidates) {
       
       
        vk::FormatProperties props = physicalDevice.getFormatProperties(format);
        if (tiling ==vk::ImageTiling::eLinear  && (props.linearTilingFeatures & features) == features) {
            return format;
        }
        else if (tiling == vk::ImageTiling::eOptimal && (props.optimalTilingFeatures & features) == features) {
            return format;
        }
    }

    throw std::runtime_error("failed to find supported format!");
}

void te::VulkanRenderManager::createImage(uint32_t width, uint32_t height, uint32_t mipLevels, vk::Format format, vk::ImageTiling tiling, vk::ImageUsageFlags usage, vk::MemoryPropertyFlags properties, vk::Image& image, vk::DeviceMemory& imageMemory)
{
    vk::ImageCreateInfo imageInfo{};
    
    imageInfo.imageType = vk::ImageType::e2D; 
    imageInfo.extent.width = width;
    imageInfo.extent.height = height;
    imageInfo.extent.depth = 1;
    imageInfo.mipLevels = mipLevels;
    imageInfo.arrayLayers = 1;
    imageInfo.format = format;
    imageInfo.tiling = tiling;
    imageInfo.initialLayout = vk::ImageLayout::eUndefined;
    imageInfo.usage = usage;
    imageInfo.samples = vk::SampleCountFlagBits::e1;
    imageInfo.sharingMode = vk::SharingMode::eExclusive;

    if (device.createImage(&imageInfo, nullptr, &image) != vk::Result::eSuccess) {
        throw std::runtime_error("failed to create image!");
    }

    vk::MemoryRequirements memRequirements = device.getImageMemoryRequirements(image);
 

    vk::MemoryAllocateInfo allocInfo{};
   
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);

    if (device.allocateMemory(&allocInfo, nullptr, &imageMemory) != vk::Result::eSuccess) {
        throw std::runtime_error("failed to allocate image memory!");
    }
    device.bindImageMemory(image, imageMemory, 0);
   
}

vk::ShaderModule te::VulkanRenderManager::createShaderModule(const std::vector<char>& code)
{
    vk::ShaderModuleCreateInfo createInfo{};
   
    createInfo.codeSize = code.size();
    createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

    vk::ShaderModule shaderModule;
    if (device.createShaderModule(&createInfo, nullptr, &shaderModule) != vk::Result::eSuccess) {
        throw std::runtime_error("failed to create shader module!");
    }

    return shaderModule;
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
    te::vkh::VulkanHelper::createVertexBuffer(
        vertices,
        vertexBuffer,
        vertexBufferMemory,
        instance->commandPool,
        instance->graphicsQueue,
        instance->physicalDevice,
        instance->device
    );
}

void te::VulkanRenderManager::createIndexBuffer(std::vector<uint32_t> indices, vk::Buffer& indexBuffer, vk::DeviceMemory& indexBufferMemory)
{
    te::vkh::VulkanHelper::createIndexBuffer(
        indices,
        indexBuffer,
        indexBufferMemory,
        instance->commandPool,
        instance->graphicsQueue,
        instance->physicalDevice,
        instance->device
    );
}


#pragma endregion

void te::VulkanRenderManager::recreateSwapChain()
{
    window->windowResizing();

    device.waitIdle();
    //vkDeviceWaitIdle(device);

    cleanupSwapChain();

    createSwapChain();
    createSwapChainImageViews();
    createRenderPass();
    createGraphicsPipeline();
    createDepthResources();
    createFramebuffers();
    createUniformBuffers();
    createDescriptorPool();
    createDescriptorSets();
    createCommandBuffers();
}

void te::VulkanRenderManager::cleanupSwapChain()
{
    device.destroyImageView(depthImageView, nullptr);
   
    device.destroyImage(depthImage, nullptr);
    device.freeMemory(depthImageMemory, nullptr);

    for (auto framebuffer : swapChainFramebuffers) {
        device.destroyFramebuffer(framebuffer, nullptr);
    }

    device.freeCommandBuffers(commandPool, static_cast<uint32_t>(commandBuffers.size()), commandBuffers.data());

    device.destroyPipeline( graphicsPipeline, nullptr);
    device.destroyPipelineLayout( pipelineLayout, nullptr);
    device.destroyRenderPass( renderPass, nullptr);

    for (auto imageView : swapChainImageViews) {
        device.destroyImageView(imageView, nullptr);
    }

    device.destroySwapchainKHR(swapChain, nullptr);

    for (size_t i = 0; i < swapChainImages.size(); i++) {
        device.destroyBuffer(uniformBuffers[i], nullptr);
        device.freeMemory(uniformBuffersMemory[i], nullptr);
    }

    device.destroyDescriptorPool(descriptorPool, nullptr);
}

void te::VulkanRenderManager::drawFrame()
{
    device.waitForFences(1, &inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);
    
    uint32_t imageIndex;
   
    vk::Result result = device.acquireNextImageKHR(swapChain, UINT64_MAX, imageAvailableSemaphores[currentFrame], nullptr, &imageIndex);
   
    if (result == vk::Result::eErrorOutOfDateKHR) {
        recreateSwapChain();
        return;
    }
    else if (result != vk::Result::eSuccess && result !=  vk::Result::eSuboptimalKHR) {
        throw std::runtime_error("failed to acquire swap chain image!");
    }

    updateUniformBuffer(imageIndex);

    if (imagesInFlight[imageIndex] != VK_NULL_HANDLE) {
        device.waitForFences(1, &imagesInFlight[imageIndex], VK_TRUE, UINT64_MAX);
    }
    imagesInFlight[imageIndex] = inFlightFences[currentFrame];

    vk::SubmitInfo submitInfo{};
  
    vk::Semaphore waitSemaphores[] = { imageAvailableSemaphores[currentFrame] };
    vk::PipelineStageFlags waitStages[] = { vk::PipelineStageFlagBits::eColorAttachmentOutput };
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;

    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffers[imageIndex];

    vk::Semaphore signalSemaphores[] = { renderFinishedSemaphores[currentFrame] };
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    device.resetFences(1, &inFlightFences[currentFrame]);

    if (graphicsQueue.submit(1, &submitInfo, inFlightFences[currentFrame]) != vk::Result::eSuccess) {
        throw std::runtime_error("failed to submit draw command buffer!");
    }

    vk::PresentInfoKHR presentInfo{};

    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;

    vk::SwapchainKHR swapChains[] = { swapChain };
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;

    presentInfo.pImageIndices = &imageIndex;

    result = presentQueue.presentKHR(&presentInfo);

    if (result == vk::Result::eErrorOutOfDateKHR || result == vk::Result::eSuboptimalKHR || window->windowResized()) {
        window->windowResizedClear();
        recreateSwapChain();
    }
    else if (result != vk::Result::eSuccess) {
        throw std::runtime_error("failed to present swap chain image!");
    }

    currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}
