#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vector>
#include <iostream>
#include <string>
#include <fstream>

namespace chicken {

    class chickenRenderer{
        public:
        chickenRenderer();
        ~chickenRenderer();
        bool vk_render();


        private:
        VkInstance instance;
        VkSurfaceKHR surface;
        VkPhysicalDevice physicalDevice[10];
        VkPhysicalDevice gpuIntel;
        VkDevice device;
        VkSwapchainKHR swapchain;
        VkSurfaceFormatKHR surfaceFormat;
        VkQueue graphicsQueue;
        VkCommandPool commandPool;
        VkSemaphore submitSemaphore;
        VkSemaphore acquireSemaphore;
        VkDebugUtilsMessengerEXT debugMessenger;
        VkRenderPass renderpass;
        VkExtent2D screensize;
        VkPipeline pipeline;
        VkPipelineLayout pipelineLayout;

        int graphicsIdx;

        uint32_t scImgCount;
        VkImage scImages[5] ;
        VkImageView scImageViews[5];
        VkFramebuffer framebuffers[5];

        static std::vector<char> readFile(const std::string &filepath);
        void createInstance();
        void createSurface();
        bool pickPhysicalDevice();
        void createLogicalDevice();
        void createSwapChain();
    };
}
