#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <iostream>

#include <string>

namespace chicken {
    class chickenWindow{
        public:
        chickenWindow();
        ~chickenWindow();

        void initWindow();
        void mainLoop();

        GLFWwindow* window;
		
        private:
        const int width = 800, height = 600;
        std::string name;

    };
}
