#version 430
#extension GL_ARB_shader_storage_buffer_object : require

struct directional_light {
  vec4 ambient_intensity;
  vec4 light_colour;
  // Light_dir is a vec3, padded in a vec4
  vec4 light_dir;
};

// Point light information
struct point_light {
  vec4 light_colour;
  vec4 position; // position is a vec3, padded in a vec4
  vec4 falloff;  //(constant, linear, quadratic, NULL)
};

// Spot light data
struct spot_light {
  vec4 light_colour;
  vec4 position;  // position is a vec3, padded in a vec4
  vec4 direction; // direction is a vec3, padded in a vec4
  vec4 falloff;   //(constant, linear, quadratic, power)
};
// A material structure
struct material {
  vec4 emissive;
  vec4 diffuse_reflection;
  vec4 specular_reflection;
  float shininess;
};

// Position of the camera
uniform vec3 eye_pos;
// Texture
uniform sampler2D tex;
// bump Texture
uniform sampler2D normal_map;
// Material of the object
uniform material mat;

// Incoming position
layout(location = 0) in vec3 position;
// Incoming texture coordinate
layout(location = 1) in vec2 tex_coord;
// alpha fade
layout(location = 2) in float fade;
// Incoming mat4(tangent, binormal, N * normal)
in mat3 TBN;

// Outgoing colour
layout(location = 0) out vec4 colour;

layout(std430, binding = 1) buffer DirectionalLights { directional_light DLights[]; };
layout(std430, binding = 2) buffer PointsLights { point_light PLights[]; };
layout(std430, binding = 3) buffer SpotLights { spot_light SLights[]; };
// AMD cards can't do loops, so we pass the length data
layout(std430, binding = 4) buffer LightStats { vec4 lightNumbers; };

vec4 calculate_dir(in directional_light light, in vec3 pos, in vec3 normal, in vec3 view_dir,
                   in vec4 tex_colour) {
  // Normalize Light Vector
  vec3 lv = normalize(vec3(light.light_dir) * TBN);
  // Calculate half vector
  vec3 half_vector = normalize(lv + view_dir);
  // Calculate ambient component
  vec4 ambient = mat.diffuse_reflection * light.ambient_intensity;
  // Calculate diffuse component
  vec4 diffuse = (mat.diffuse_reflection * light.light_colour) * max(dot(normal, lv), 0);
  // Calculate specular component
  vec4 specular = (mat.specular_reflection * light.light_colour) *
                  pow(max(dot(normal, half_vector), 0), mat.shininess);

  vec4 colour = (mat.emissive + ambient + diffuse) * tex_colour + specular;
  return colour;
}

// Spot light calculation
vec4 calculate_point(in point_light point, in vec3 pos, in vec3 normal, in vec3 view_dir,
                     in vec4 tex_colour) {
  // Get distance between point light and vertex
  float d = distance(vec3(point.position), position);
  // Calculate attenuation factor
  float attenuation = point.falloff.x + (point.falloff.y * d) + (point.falloff.z * d * d);
  // Calculate light colour
  vec4 light_colour = point.light_colour / attenuation;
  light_colour.a = 1.0;
  // Calculate light dir
  vec3 light_dir = normalize(vec3(point.position) - position);
  // Now use standard phong shading but using calculated light colour and
  // direction  - note no ambient
  vec4 diffuse = (mat.diffuse_reflection * light_colour) * max(dot(normal, light_dir), 0);
  vec3 half_vector = normalize(light_dir + view_dir);
  vec4 specular = (mat.specular_reflection * light_colour) *
                  pow(max(dot(normal, half_vector), 0), mat.shininess);
  vec4 primary = mat.emissive + diffuse;
  vec4 colour = primary * tex_colour + specular;
  colour.a = 1.0;
  return colour;
}

vec4 calculate_spot(in spot_light spot, in vec3 pos, in vec3 normal, in vec3 view_dir,
                    in vec4 tex_colour) {
  // Calculate direction to the light
  vec3 light_dir = normalize(vec3(spot.position) - position);
  // Calculate distance to light
  float d = distance(vec3(spot.position), position);
  // Calculate attenuation value
  float attenuation = spot.falloff.x + spot.falloff.y * d + spot.falloff.z * d * d;
  // Calculate spot light intensity
  float sp = pow(max(dot(light_dir, -vec3(spot.direction)), 0.0), spot.falloff.w);
  // Calculate light colour
  vec4 light_colour = (sp / attenuation) * spot.light_colour;
  // Now use standard phong shading but using calculated light colour and
  // direction  - note no ambient
  vec4 diffuse = (mat.diffuse_reflection * light_colour) * max(dot(normal, light_dir), 0.0);
  vec3 half_vector = normalize(light_dir + view_dir);
  vec4 specular = (mat.specular_reflection * light_colour) *
                  pow(max(dot(normal, half_vector), 0.0), mat.shininess);

  vec4 colour = ((mat.emissive + diffuse) * tex_colour) + specular;
  colour.a = 1.0;

  return colour;
}

vec4 CoolFunc() { return vec4(1, 0, 0, 1); }
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
  // for (int i = 0; i < DLights.length() ; i++) {
  for (int i = 0; i < lightNumbers.x; i++) {
    colour += calculate_dir(DLights[i], position, normal, view_dir, tex_colour);
  }
  // point light
  // for (int i = 0; i < PLights.length(); i++) {
  for (int i = 0; i < lightNumbers.y; i++) {
    colour += calculate_point(PLights[i], position, normal, view_dir, tex_colour);
  }
  // spotlight
  // for (int i = 0; i < SLights.length(); i++)
  for (int i = 0; i < lightNumbers.z; i++) {
    colour += calculate_spot(SLights[i], position, normal, view_dir, tex_colour);
  }

  colour.a = fade;
}