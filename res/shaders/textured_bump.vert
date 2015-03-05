#version 430

// The transformation matrix
uniform mat4 MVP;
// The model transformation
uniform mat4 M;
// The normal matrix
uniform mat3 N;
// The direction of the light
uniform vec3 light_dir;

uniform float TextureScale;

// Incoming position
layout(location = 0) in vec3 position;
// Incoming normal
layout(location = 2) in vec3 normal;
// Incoming binormal
layout(location = 3) in vec3 binormal;
// Incoming tangent
layout(location = 4) in vec3 tangent;
// Incoming texture coordinate
layout(location = 10) in vec2 tex_coord_in;

// Outgoing vertex position
layout(location = 0) out vec3 vertex_position;
// Outgoing texture coordinate
layout(location = 1) out vec2 tex_coord_out;
// Outgoing TBN matrix
layout(location = 8) out mat3 TBN;

layout(location = 3) out float fade;

void main() {
  // Transform position into screen space
  gl_Position = MVP * vec4(position, 1.0);
  // Transform position into world space
  vertex_position = (M * vec4(position, 1.0)).xyz;
  // Pass through texture coordinate
  tex_coord_out = tex_coord_in * TextureScale;

  // *******************
  // Generate TBN matrix
  // *******************
  TBN = mat3(tangent, binormal, N * normal);
  // *************************************************
  // light_vec is light_dir transformed by this matrix
  // *************************************************
  fade = clamp(100.0 - length(vertex_position), 0.0, 1.0);
}