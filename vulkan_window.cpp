#include "vulkan_window.hpp"
#include "vulkan_renderer.hpp"
using namespace chicken;

extern chickenRenderer rendererClass;

chickenWindow::chickenWindow()
{
    chickenWindow::initWindow();
}

chickenWindow::~chickenWindow()
{
    glfwDestroyWindow(window);
    glfwTerminate();
}

void chickenWindow::initWindow()
{
    glfwInit();
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    window = glfwCreateWindow(width, height, "ChickenWindow", nullptr, nullptr);
}

void chickenWindow::mainLoop()
{
    while(!glfwWindowShouldClose(window))
    {
        glfwPollEvents();
        rendererClass.vk_render();
    }
}