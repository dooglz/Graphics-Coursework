#version 430

uniform mat4 modelMatrix;
uniform mat4 MVP;

layout (location = 0) in vec3 position; //vertex_in
layout(location = 0) out vec3 vWorldPosition;

void main() {

  vec4 worldPosition = modelMatrix * vec4(position, 1.0);
  vWorldPosition = worldPosition.xyz;
  gl_Position = MVP * vec4(position, 1.0);
}