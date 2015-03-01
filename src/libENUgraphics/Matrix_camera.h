#pragma once

#include "stdafx.h"
#include "camera.h"

namespace graphics_framework {
  /*
  An object representing a target camera
  */
  class matrix_camera : public camera {
  public:
    // Creates a target camera
    matrix_camera() : camera() {}
    // Default copy constructor and assignment operator
    matrix_camera(const matrix_camera &other) = default;
    matrix_camera &operator=(const matrix_camera &rhs) = default;
    // Destroys the target camera
    ~matrix_camera() {}
    // Updates the camera
    void update(float delta_time){}
    // Overide the projection matrix
    void ovvereide_projection(const glm::mat4& value){ _projection = value; }
    // Overide the view Matrix
    void set_view(const glm::mat4& value);
  };
}