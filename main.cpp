#include "vulkan_window.hpp"
#include "vulkan_renderer.hpp"

#include <cstdlib>
#include <iostream>
#include <stdexcept>

chicken::chickenWindow wndClass;

chicken::chickenRenderer rendererClass;


int main()
{

    try {
        wndClass.mainLoop();
    } catch (const std::exception &e) {
        std::cerr << "Error: " << e.what() << "\n" << std::endl;
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
