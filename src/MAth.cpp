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

glm::mat4 MirrorMatrix(const glm::vec3& Normal, const glm::vec3& A) {
  glm::mat4 Mtx;

  // X

  Mtx[0][0] = 1 - 2 * Normal.x * Normal.x;
  Mtx[1][0] = -2 * Normal.y * Normal.x;
  Mtx[2][0] = -2 * Normal.z * Normal.x;
  Mtx[3][0] = 2 * glm::dot(A, Normal) * Normal.x;

  // y

  Mtx[0][1] = -2 * Normal.x * Normal.y;
  Mtx[1][1] = 1 - 2 * Normal.y * Normal.y;
  Mtx[2][1] = -2 * Normal.z * Normal.y;
  Mtx[3][1] = 2 * glm::dot(A, Normal) * Normal.y;

  // z

  Mtx[0][2] = -2 * Normal.x * Normal.z;
  Mtx[1][2] = -2 * Normal.y * Normal.z;
  Mtx[2][2] = 1 - 2 * Normal.z * Normal.z;
  Mtx[3][2] = 2 * glm::dot(A, Normal) * Normal.z;

  // W

  Mtx[0][3] = 0;
  Mtx[1][3] = 0;
  Mtx[2][3] = 0;
  Mtx[3][3] = 1;

  return Mtx;
}

glm::vec4 CameraSpacePlane(const glm::mat4& camproj, const glm::vec3& pos,
                           const glm::vec3& normal, const float sideSign) {
  glm::vec3 offsetPos = pos + normal * 0.07f; // magic number
  glm::vec3 cpos = glm::vec3(camproj * glm::vec4(offsetPos, 1.0f));
  glm::vec3 cnormal =
      glm::normalize(glm::vec3(camproj * glm::vec4(normal, 1.0f))) * sideSign;
  return glm::vec4(cnormal.x, cnormal.y, cnormal.z, -glm::dot(cpos, cnormal));
}

void CalculateObliqueMatrix(glm::mat4& projection, const glm::vec4& clipPlane)
{
  glm::vec4 q = glm::inverse(projection) * glm::vec4(
    sgn(clipPlane.x),
    sgn(clipPlane.y),
    1.0f,
    1.0f
    );
  glm::vec4 c = clipPlane * (2.0F / (glm::dot(clipPlane, q)));
  // third row = clip plane - fourth row
  projection[2][0] = c.x - projection[3][0];
  projection[2][1] = c.y - projection[3][1];
  projection[2][2] = c.z - projection[3][2];
  projection[2][3] = c.w - projection[3][3];
}