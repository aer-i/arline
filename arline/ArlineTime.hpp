#pragma once
#define GLFW_INCLUDE_NONE
#include "ArlineTypes.hpp"
#include <GLFW/glfw3.h>

namespace arline::time
{
    inline auto get() noexcept -> f64 { return glfwGetTime(); }
}