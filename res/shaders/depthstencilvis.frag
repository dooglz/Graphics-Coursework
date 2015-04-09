#version 440

uniform sampler2D depthTex;
uniform usampler2D stencilTex;
uniform vec2 gScreenSize;

layout(location = 0) out vec3 depthCol;
layout(location = 1) out vec3 stencilCol;

vec2 CalcTexCoord() { return gl_FragCoord.xy / gScreenSize; }

void main() {

  vec2 TexCoord = CalcTexCoord();

  vec4 texel = texelFetch(stencilTex, ivec2(5, 5), 0);
  uint stencil = texture2D(stencilTex, TexCoord).x;
  float stn = float(stencil);
  stencilCol = vec3(stn / 1.0, stn / 2.0, stn / 10.0);
  // stencilCol = vec3(1.0,0,0);
  // stencilCol = texel.xyz;

  // depth -- bottom half
  const float n = 1.0;   // camera z near
  const float f = 500.0; // camera z far

  float depth = texture(depthTex, TexCoord).x;
  float col = (2.0 * n) / (f + n - depth * (f - n));
  depthCol = vec3(col);
  // depthCol = vec3(0,1.00,0);
  // depthCol = vec3(1000,1000.00,1000);
}
