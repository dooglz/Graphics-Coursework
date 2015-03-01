#include "stdafx.h"
#include "matrix_camera.h"
#include <glm\glm.hpp>

namespace graphics_framework {
  // Updates the target camera
  void matrix_camera::set_view(const glm::mat4& value){
    _view = value;
    _position = glm::vec3(_view[3]);
    _target = glm::vec3(_view[2]);
    _up = glm::vec3(_view[1]);
  }
}