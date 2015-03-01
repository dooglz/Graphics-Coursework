#version 430

// Sampler used to get texture colour
uniform sampler2D tex;
uniform vec4 clip_plane;
uniform mat4 o2v_projection_reflection;

layout(location = 1) in vec3 interpolatedVertexEye;
layout(location = 2) in vec3 interpolatedVertexObject;

// Outgoing colour
layout(location = 0) out vec4 out_colour;

void main() {
  float clipPos = dot(interpolatedVertexEye, clip_plane.xyz) + clip_plane.w;
  if (clipPos < 0.0) {
   //discard;
  }
  vec4 vClipReflection =
      o2v_projection_reflection * vec4(interpolatedVertexObject.xy, 0.0, 1.0);
  vec2 vDeviceReflection = vClipReflection.st / vClipReflection.q;
  vec2 vTextureReflection = vec2(0.5, 0.5) + 0.5 * vDeviceReflection;

  vec4 reflectionTextureColor = texture2D(tex, vTextureReflection);

  // Framebuffer reflection can have alpha > 1
  reflectionTextureColor.a = 1.0;

  out_colour = reflectionTextureColor;
  //out_colour = vec4(1.0,0.1,0.2,1.0);;
}