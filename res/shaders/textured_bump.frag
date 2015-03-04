#version 430

layout(std430, binding = 2) buffer ssbo_directional_lights
{
  vec4 s_ambient_intensity;
  vec4 s_light_colour;
  vec3 s_light_dir;
};

layout(std430, binding = 3) buffer MyBuffer
{
  vec4 lotsOfFloats[];
};


// A directional light structure
struct directional_light
{
  vec4 ambient_intensity;
  vec4 light_colour;
  vec3 light_dir;
};

// A material structure
struct material
{
  vec4 emissive;
  vec4 diffuse_reflection;
  vec4 specular_reflection;
  float shininess;
};


// Material of the object
uniform material mat;
// Position of the camera
uniform vec3 eye_pos;
// Texture
uniform sampler2D tex;
// bump Texture
uniform sampler2D normal_map;


// Incoming position
layout (location = 0) in vec3 position;
// Incoming texture coordinate
layout (location = 1) in vec2 tex_coord;
// Incoming light vector
layout (location = 2) in vec3 light_vec;

layout (location = 3) in float fade;

// Outgoing colour
layout (location = 0) out vec4 colour;

void main()
{
  // Extract the normal from the normal map  
  vec3 normal2 = normalize((texture(normal_map, tex_coord).xyz) * 2.0 - 1.0);
  vec3 lv = normalize(light_vec);
  // Calculate ambient component
  vec4 ambient = mat.diffuse_reflection * s_ambient_intensity;
  // Calculate diffuse component
  vec4 diffuse = (mat.diffuse_reflection * s_light_colour) * max(dot(normal2, lv), 0);
  // Calculate view direction
  vec3 view_dir = normalize(eye_pos - position);
  // Calculate half vector
  vec3 half_vector = normalize(lv + view_dir);
  // Calculate specular component
  vec4 specular = (mat.specular_reflection * s_light_colour) * pow(max(dot(normal2, half_vector), 0), mat.shininess);
  // Sample texture
  vec4 tex_colour = texture(tex, tex_coord);
  // Calculate primary colour component
  vec4 primary = mat.emissive + ambient + diffuse;
  // Calculate final colour
  colour = primary * tex_colour + specular;
  colour.a = fade;

  for (int i = 0; i < lotsOfFloats.length(); i++){
    colour += lotsOfFloats[i];
  }
  //colour.a = fade;
}