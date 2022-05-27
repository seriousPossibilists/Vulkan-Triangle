#include "vulkan_renderer.hpp"
#include "vulkan_window.hpp"

#include <stdexcept>
#include <iostream>
#include <vector>
#include <map>

using namespace chicken;

extern chickenWindow wndClass;

static VKAPI_ATTR VkBool32 VKAPI_CALL vk_debug_callback(
    VkDebugUtilsMessageSeverityFlagBitsEXT msgSeverity,
    VkDebugUtilsMessageTypeFlagsEXT msgFlags,
    const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
    void *pUserData)
{
    std::cout << "Validation Error: " << pCallbackData->pMessage << std::endl;
    return false;
}

   std::vector<char> chickenRenderer::readFile(const std::string &filepath)
   {
       std::ifstream file(filepath, std::ios::ate | std::ios::binary);

        if (!file.is_open()) {
            throw std::runtime_error("failed to open file!");
        }

        size_t fileSize = (size_t) file.tellg();
        std::vector<char> buffer(fileSize);

        file.seekg(0);
        file.read(buffer.data(), fileSize);

        file.close();

        return buffer;
   }

   chickenRenderer::chickenRenderer()
{
    chickenRenderer::createInstance();
    chickenRenderer::createSurface();
    chickenRenderer::pickPhysicalDevice();
    chickenRenderer::createLogicalDevice();
    chickenRenderer::createSwapChain();
}

chickenRenderer::~chickenRenderer()
{
    vkDestroySurfaceKHR(instance, surface, nullptr);
    vkDestroyInstance(instance, nullptr);
    vkDestroyDevice(device, nullptr);
}

void chickenRenderer::createInstance()
{
    VkApplicationInfo appInfo{};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "ChickenWindow";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "No Engine";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_0;



    uint32_t count;
    const char **extensionsGLFW = glfwGetRequiredInstanceExtensions(&count);


    char *layers[]{
        "VK_LAYER_KHRONOS_validation"};
   
    VkInstanceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;
    createInfo.ppEnabledExtensionNames = extensionsGLFW;
    createInfo.enabledExtensionCount = count;
    createInfo.ppEnabledLayerNames = layers;
    createInfo.enabledLayerCount = sizeof(layers) / sizeof(layers[0]);

    VkResult result = vkCreateInstance(&createInfo, nullptr, &instance);

    if (result != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create instance!");
        std::cout << result << "\n";
    }
    else
    {
        std::cout << "Vulkan Instance Creation Success. \n";
    }


    auto vkCreateDebugUtilsMessengerEXT = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");

    if (vkCreateDebugUtilsMessengerEXT)
    {
        VkDebugUtilsMessengerCreateInfoEXT debugInfo = {};
        debugInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        debugInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT;
        debugInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT;
        debugInfo.pfnUserCallback = vk_debug_callback;

        vkCreateDebugUtilsMessengerEXT(instance, &debugInfo, 0, &debugMessenger);
    }
    else
    {
        std::cout << "Validation layers failed" << std::endl;
    }

}

void chickenRenderer::createSurface()
{
    if (glfwCreateWindowSurface(instance, wndClass.window, nullptr, &surface) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create window surface!");
    }
    else
    {
        std::cout << "GLFW window surface created successfully. \n";
    }
}

bool chickenRenderer::pickPhysicalDevice()
{
    uint32_t graphicsIdx = -1;

    uint32_t gpuCount = 0;
    //TODO: Suballocation from Main Allocation
    VkPhysicalDevice gpus[10];
    vkEnumeratePhysicalDevices(instance, &gpuCount, 0);
    vkEnumeratePhysicalDevices(instance, &gpuCount, gpus);

    for (uint32_t i = 0; i < gpuCount; i++)
    {
        VkPhysicalDevice gpu = gpus[i];

        uint32_t queueFamilyCount = 0;
        //TODO: Suballocation from Main Allocation

        VkQueueFamilyProperties queueProps[10];
        vkGetPhysicalDeviceQueueFamilyProperties(gpu, &queueFamilyCount, 0);
        vkGetPhysicalDeviceQueueFamilyProperties(gpu, &queueFamilyCount, queueProps);

        for (uint32_t j = 0; j < queueFamilyCount; j++)
        {
            if (queueProps[j].queueFlags & VK_QUEUE_GRAPHICS_BIT)
            {
                VkBool32 surfaceSupport = VK_FALSE;
                vkGetPhysicalDeviceSurfaceSupportKHR(gpu, j, surface, &surfaceSupport);

                if (surfaceSupport)
                {
                    graphicsIdx = j;
                    gpuIntel = gpu;
                    VkPhysicalDeviceProperties physicalProperties = {};

                    break;
                }
            }
        }
    }

    if (graphicsIdx < 0)
    {
        std::cout << "No GPU support found. \n";
    }
    return true;
    std::cout << "Successfully connected with physical device, \n";
}

void chickenRenderer::createLogicalDevice()
{
    float queuePriority = 1.0;

    VkDeviceQueueCreateInfo queueInfo = {};
    queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueInfo.queueFamilyIndex = graphicsIdx;
    queueInfo.pQueuePriorities = &queuePriority;

    std::vector<const char *> enabledExtensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};

    VkDeviceCreateInfo deviceInfo{};
    deviceInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    deviceInfo.enabledExtensionCount = enabledExtensions.size();
    deviceInfo.ppEnabledExtensionNames = enabledExtensions.data();
    deviceInfo.pEnabledFeatures = NULL;
    deviceInfo.pQueueCreateInfos = &queueInfo;
    deviceInfo.queueCreateInfoCount = 1;

    if (vkCreateDevice(gpuIntel, &deviceInfo, nullptr, &device) != VK_SUCCESS)
    {
        std::cout << "Failed to create device. \n";
    }

    vkGetDeviceQueue(device, graphicsIdx, 0, &graphicsQueue);
}

void chickenRenderer::createSwapChain()
{
    int width, height;
    glfwGetWindowSize(wndClass.window, &width, &height);
    screensize.width = width;
    screensize.height = height;
    uint32_t formatCount = 0;
    VkSurfaceFormatKHR surfaceFormats[10];
    vkGetPhysicalDeviceSurfaceFormatsKHR(gpuIntel, surface, &formatCount, 0);
    vkGetPhysicalDeviceSurfaceFormatsKHR(gpuIntel, surface, &formatCount, surfaceFormats);

    for(uint32_t i = 0; i < formatCount; i++)
    {
        VkSurfaceFormatKHR format = surfaceFormats[i];

        if(format.format == VK_FORMAT_B8G8R8A8_SRGB)
        {
            surfaceFormat = format;
            break;
        }
    }

    VkSurfaceCapabilitiesKHR surfaceCapibilities;
    VkResult res = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(gpuIntel, surface, &surfaceCapibilities);
    uint32_t imgCount = 0;
    imgCount = surfaceCapibilities.minImageCount + 1;
    imgCount > surfaceCapibilities.maxImageCount ? imgCount - 1: imgCount;

    VkSwapchainCreateInfoKHR scInfo = {};
    scInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    scInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    scInfo.surface = surface;
    scInfo.imageFormat = surfaceFormat.format;
    scInfo.preTransform = surfaceCapibilities.currentTransform;
    scInfo.imageExtent = surfaceCapibilities.currentExtent;
    scInfo.minImageCount = imgCount;
    scInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    scInfo.imageArrayLayers = 1;

    if (vkCreateSwapchainKHR(device, &scInfo, nullptr, &swapchain) != VK_SUCCESS)
    {
        std::cout << "Failed to createSwapChain. \n";
    }
    else
    {
        std::cout << "Created swapchain successfully. \n";
    }


    vkGetSwapchainImagesKHR(device, swapchain, &scImgCount, 0);
    vkGetSwapchainImagesKHR(device, swapchain, &scImgCount, scImages);

    //Create image scImageViews
    {
        VkImageViewCreateInfo viewInfo = {};
        viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        viewInfo.format = surfaceFormat.format;
        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        viewInfo.subresourceRange.layerCount = 1;
        viewInfo.subresourceRange.levelCount = 1;

        for (int i = 0; i < scImgCount; i++)
        {
            viewInfo.image = scImages[i];
            vkCreateImageView(device, &viewInfo, 0, &scImageViews[i]);
        }
    }

    //RenderPass
    {
        VkAttachmentDescription attachment = {};
        attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
        attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        attachment.samples = VK_SAMPLE_COUNT_1_BIT;
        attachment.format = surfaceFormat.format;


        VkAttachmentReference colorAttachmentRef = {};
        colorAttachmentRef.attachment = 0;
        colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;



        VkSubpassDescription subpassDesc = {};
        subpassDesc.colorAttachmentCount = 1;
        subpassDesc.pColorAttachments = &colorAttachmentRef;

        VkAttachmentDescription attachments[] = {
            attachment
        };

        VkRenderPassCreateInfo rpInfo = {};
        rpInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        rpInfo.pAttachments = attachments;
        rpInfo.attachmentCount= sizeof(attachments) / sizeof(attachments[0]);
        rpInfo.subpassCount = 1;
        rpInfo.pSubpasses = &subpassDesc;

        vkCreateRenderPass(device, &rpInfo, 0, &renderpass);
    }

    //FrameBuffer
    {
        VkFramebufferCreateInfo fbInfo = {};
        fbInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        fbInfo.width = screensize.width;
        fbInfo.height = screensize.height;
        fbInfo.renderPass = renderpass;
        fbInfo.layers = 1;
        fbInfo.attachmentCount = 1;

        for (int i = 0; i < scImgCount; i++)
        {
            fbInfo.pAttachments = &scImageViews[i];
            vkCreateFramebuffer(device, &fbInfo, 0, &framebuffers[i]);
        }
    }

    //Pipeline Layout
    {
        VkPipelineLayoutCreateInfo layoutCreateInfo = {};
        layoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        if(vkCreatePipelineLayout(device, &layoutCreateInfo, 0, &pipelineLayout) != VK_SUCCESS)
        {
            std::cout << "Failed to create pipeline layout \n" << std::endl;
        }
        else{
            std::cout << "Created pipeline layout successfully \n" << std::endl;
        }
    }

    //Pipeline
    {   
        //Vk shader modules
        VkShaderModule vertexShader, fragmentShader;

        //loading shder code using readFile function
        auto vertCode = readFile("shaders/simple_shader.vert.spv");
        auto fragCode = readFile("shaders/simple_shader.frag.spv");

        //shaderInfo struct required for shader creation
        VkShaderModuleCreateInfo shaderInfo = {};
        shaderInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        shaderInfo.pCode = reinterpret_cast<const uint32_t*>(vertCode.data());
        shaderInfo.codeSize = vertCode.size();
        //create the vertex shader module
        if(vkCreateShaderModule(device, &shaderInfo, 0, &vertexShader) != VK_SUCCESS)
        {
            std::cout << "Failed to create shader module" << std::endl;
        }
        else{
            std::cout << "created shader module successfully." << std::endl;
        }

        shaderInfo.pCode = reinterpret_cast<const uint32_t*>(fragCode.data());
        shaderInfo.codeSize = fragCode.size();
        //create the fragment shader module
        vkCreateShaderModule(device, &shaderInfo, 0, &fragmentShader);

        //the vertex stage in the pipeline
        VkPipelineShaderStageCreateInfo vertexStage = {};
        vertexStage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        vertexStage.stage = VK_SHADER_STAGE_VERTEX_BIT;
        vertexStage.pName = "main";
        vertexStage.module = vertexShader;
        
        //the fragment stage in the pipeline
        VkPipelineShaderStageCreateInfo fragStage = {};
        fragStage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        fragStage.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        fragStage.pName = "main";
        fragStage.module = fragmentShader;

        VkPipelineShaderStageCreateInfo shaderStages[] = {
            vertexStage,
            fragStage,
        };

        //Vertex input buffer
        VkPipelineVertexInputStateCreateInfo vertexInputStage = {};
        vertexInputStage.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

        VkPipelineColorBlendAttachmentState colorAttachment = {};
        colorAttachment.blendEnable = VK_FALSE;
        colorAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;

        //Color blending stage
        VkPipelineColorBlendStateCreateInfo colorBlendState = {};
        colorBlendState.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        colorBlendState.pAttachments = &colorAttachment; 
        colorBlendState.attachmentCount = 1;

        VkPipelineRasterizationStateCreateInfo rasterizationState = {};
        rasterizationState.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        rasterizationState.frontFace = VK_FRONT_FACE_CLOCKWISE;
        rasterizationState.cullMode = VK_CULL_MODE_BACK_BIT;
        //Look at this if evrything is invisible!!!
        rasterizationState.polygonMode = VK_POLYGON_MODE_FILL;
        rasterizationState.lineWidth = 1.0f;

        VkPipelineMultisampleStateCreateInfo multiSampleStage = {};
        multiSampleStage.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        multiSampleStage.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

        VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};
        inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

        VkRect2D scissor = {};
        VkViewport viewport = {};

        VkPipelineViewportStateCreateInfo viewportState = {};
        viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        viewportState.scissorCount = 1;
        viewportState.pScissors = &scissor;
        viewportState.viewportCount = 1;
        viewportState.pViewports = &viewport;

        VkDynamicState dynamicStates[] = {
            VK_DYNAMIC_STATE_VIEWPORT,
            VK_DYNAMIC_STATE_SCISSOR
        };

        VkPipelineDynamicStateCreateInfo dynamicState = {};
        dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        dynamicState.pDynamicStates = dynamicStates;
        dynamicState.dynamicStateCount = sizeof(dynamicStates) / sizeof(dynamicStates[0]);
        std::cout << "dynamicState Created \n" << std::endl;

        VkGraphicsPipelineCreateInfo pipeInfo = {};
        pipeInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        pipeInfo.layout = pipelineLayout;
        pipeInfo.renderPass = renderpass;
        pipeInfo.pVertexInputState = &vertexInputStage;
        pipeInfo.pColorBlendState = &colorBlendState;
        pipeInfo.pStages = shaderStages;
        pipeInfo.stageCount = sizeof(shaderStages) / sizeof(shaderStages[0]);
        pipeInfo.pRasterizationState = &rasterizationState;
        pipeInfo.pViewportState = &viewportState;
        pipeInfo.pDynamicState = &dynamicState;
        pipeInfo.pMultisampleState = &multiSampleStage;
        pipeInfo.pInputAssemblyState = &inputAssembly;
        std::cout << "dynamicState Created \n" << std::endl;


        if(vkCreateGraphicsPipelines(device, 0, 1, &pipeInfo, 0, &pipeline) != VK_SUCCESS)
        {
            std::cout << "Failed to create graphics pipeline" << std::endl;
        }
        else{
            std::cout << "Created graphics pipeline successfully" << std::endl;
        }

        vkDestroyShaderModule(device, vertexShader, 0);
        vkDestroyShaderModule(device, fragmentShader, 0);
    }

    {
        VkCommandPoolCreateInfo poolInfo = {};
        poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        poolInfo.queueFamilyIndex = graphicsIdx;
        vkCreateCommandPool(device, &poolInfo, 0, &commandPool);
    }

    {
        VkSemaphoreCreateInfo semainfo = {};
        semainfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
        vkCreateSemaphore(device, &semainfo, 0, &acquireSemaphore);
        vkCreateSemaphore(device, &semainfo, 0, &submitSemaphore);
    }

}

bool chickenRenderer::vk_render()
{


    uint32_t imgIdx;
    vkAcquireNextImageKHR(device, swapchain, 0, acquireSemaphore, 0,&imgIdx);

    VkCommandBuffer cmd;

    VkCommandBufferAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandBufferCount = 1;
    allocInfo.commandPool = commandPool;
    if(vkAllocateCommandBuffers(device, &allocInfo, &cmd) != VK_SUCCESS)
    {
        std::cout << "Failed to allocate command buffer \n" << std::endl;
    }

    VkCommandBufferBeginInfo beginInfo = {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    
        
    if(vkBeginCommandBuffer(cmd, &beginInfo) != VK_SUCCESS)
    {
        std::cout << "Command buffer creation failed \n" << std::endl;
    }

    VkClearValue clearValue = {}; 
    clearValue.color = {0, 0, 0, 1};

    VkRenderPassBeginInfo rpBeginInfo = {};
    rpBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    rpBeginInfo.renderPass = renderpass;
    rpBeginInfo.renderArea.extent = screensize;
    rpBeginInfo.framebuffer = framebuffers[imgIdx];
    rpBeginInfo.pClearValues = &clearValue;
    rpBeginInfo.clearValueCount = 1;

    vkCmdBeginRenderPass(cmd, &rpBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
   

    {
        VkRect2D scissor = {};
        scissor.extent = screensize;

        VkViewport viewport = {};
        viewport.width = screensize.width;
        viewport.height = screensize.height;
        viewport.maxDepth = 1.0f;

        vkCmdSetScissor(cmd, 0, 1, &scissor);
        vkCmdSetViewport(cmd, 0, 1, &viewport);

        vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
        vkCmdDraw(cmd, 3, 1, 0, 0);
    }

    vkCmdEndRenderPass(cmd);

    vkEndCommandBuffer(cmd);

    VkPipelineStageFlags waitStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.pWaitDstStageMask = &waitStage;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &cmd;
    submitInfo.pSignalSemaphores = &submitSemaphore;
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = &acquireSemaphore;
    submitInfo.waitSemaphoreCount = 1;

    if(vkQueueSubmit(graphicsQueue, 1, &submitInfo, 0) != VK_SUCCESS)
    {
        std::cout << "Failed to submit queue \n" << std::endl;
    }
   

    VkPresentInfoKHR presentInfo = {};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.pSwapchains = &swapchain;
    presentInfo.swapchainCount = 1;
    presentInfo.pImageIndices  = &imgIdx;
    presentInfo.pWaitSemaphores = &submitSemaphore;
    presentInfo.waitSemaphoreCount = 1;
    vkQueuePresentKHR(graphicsQueue, &presentInfo);

    vkDeviceWaitIdle(device);

    vkFreeCommandBuffers(device, commandPool, 1, &cmd);

    return true;
}
