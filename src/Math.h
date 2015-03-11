#include "glm\glm.hpp"

glm::vec3 GetForwardVector(glm::quat &q);
glm::vec3 GetUpVector(glm::quat &q);
glm::vec3 GetRightVector(glm::quat &q);

glm::mat4 MirrorMatrix(const glm::vec3 &Normal, const glm::vec3 &A);

glm::vec4 CameraSpacePlane(const glm::mat4 &camproj, const glm::vec3 &pos, const glm::vec3 &normal,
                           const float sideSign);

// Extended sign: returns -1, 0 or 1 based on sign of a
inline float sgn(float a) {
  if (a > 0.0f)
    return 1.0f;
  if (a < 0.0f)
    return -1.0f;
  return 0.0f;
}

void CalculateObliqueMatrix(glm::mat4 &projection, const glm::vec4 &clipPlane);