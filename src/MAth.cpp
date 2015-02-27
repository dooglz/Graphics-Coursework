#include "Math.h"
#include "glm\gtc\quaternion.hpp"

glm::vec3 GetForwardVector(glm::quat& q) {

  return glm::vec3(2.0f * (q.x * q.z + q.w * q.y),
    2.0f * (q.y * q.z - q.w * q.x),
    1.0f - 2.0f * (q.x * q.x + q.y * q.y));
}

glm::vec3 GetUpVector(glm::quat& q) {
  return glm::vec3(2 * (q.x * q.y - q.w * q.z),
    1.0f - 2.0f * (q.x * q.x + q.z * q.z),
    2.0f * (q.y * q.z + q.w * q.x));
}

glm::vec3 GetRightVector(glm::quat& q) {
  return glm::vec3(1.0f - 2.0f * (q.y * q.y + q.z * q.z),
    2.0f * (q.x * q.y + q.w * q.z),
    2.0f * (q.x * q.z - q.w * q.y));
}