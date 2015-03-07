#version 430

// Sampler used to get texture colour
uniform sampler2D tex;
//could be the mvp of the reflection camera
uniform mat4 reflected_MVP;

// Incoming texture coordinate
layout(location = 0) in vec2 tex_coord;
// Outgoing colour
layout(location = 0) out vec4 out_colour;

layout(location = 1) in vec3 position;

void main()
{
  vec4 reflectedPos = reflected_MVP * vec4(position.xyz, 1.0);
  vec2 transformedUV;
  transformedUV.x = reflectedPos.x/reflectedPos.w/2.0f + 0.5f;
  transformedUV.y = reflectedPos.y/reflectedPos.w/2.0f + 0.5f;
  
  vec4 reflectionTextureColor = texture2D (tex, transformedUV);

  reflectionTextureColor.a = 1.0;
  out_colour = reflectionTextureColor;
  //out_colour = texture2D (tex, tex_coord);
}