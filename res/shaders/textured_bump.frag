#version 430
// A directional light structure
struct directional_light {
  vec4 ambient_intensity;
  vec4 light_colour;
  vec3 light_dir;
};

layout(std430, binding = 1) buffer DirectionalLights {
  directional_light DLights[];
};

// A material structure
struct material {
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
layout(location = 0) in vec3 position;
// Incoming texture coordinate

layout(location = 1) in vec2 tex_coord;
// Incoming light vector
layout(location = 8) in mat3 TBN;

layout(location = 3) in float fade;

// Outgoing colour
layout(location = 0) out vec4 colour;

//----------------------
vec4 calculate_dir(in directional_light light, in material mat,
                   in vec3 position, in vec3 normal, in vec3 view_dir,
                   in vec4 tex_colour) {
  // Normalize Light Vector
  vec3 lv = normalize(light.light_dir * TBN);
  // Calculate half vector
  vec3 half_vector = normalize(lv + view_dir);
  // Calculate ambient component
  vec4 ambient = mat.diffuse_reflection * light.ambient_intensity;
  // Calculate diffuse component
  vec4 diffuse =
      (mat.diffuse_reflection * light.light_colour) * max(dot(normal, lv), 0);
  // Calculate specular component
  vec4 specular = (mat.specular_reflection * light.light_colour) *
                  pow(max(dot(normal, half_vector), 0), mat.shininess);

  vec4 colour = (mat.emissive + ambient + diffuse) * tex_colour + specular;
  return colour;
}

//---------------------------------
void main() {
  // Extract the normal from the normal map
  vec3 normal = normalize((texture(normal_map, tex_coord).xyz) * 2.0 - 1.0);
  // Calculate view direction
  vec3 view_dir = normalize(eye_pos - position);
  // Sample texture
  vec4 tex_colour = texture(tex, tex_coord);

  colour = vec4(0, 0, 0, 1);

  // for each directional light
  for (int i = 0; i < DLights.length() + 1; i++) {
    colour +=
        calculate_dir(DLights[i], mat, position, normal, view_dir, tex_colour);
  }

  colour.a = fade;
}