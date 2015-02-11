#pragma once

#include "stdafx.h"
#include "shadow_map.h"

namespace graphics_framework
{
    // Set shadow bias matrix
    glm::mat4 shadow_map::bias =
        glm::mat4(0.5f, 0.0f, 0.0f, 0.0f,
        0.0f, 0.5f, 0.0f, 0.0f,
        0.0f, 0.0f, 0.5f, 0.0f,
        0.5f, 0.5f, 0.5f, 1.0f);
}