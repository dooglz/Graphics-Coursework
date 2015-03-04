#version 430
// A directional light structure
struct directional_light {
  vec4 ambient_intensity;
  vec4 light_colour;
  //Light_dir is a vec3, padded in a vec4
  vec4 light_dir;
};

// Point light information
struct point_light {
  vec4 light_colour;
  vec3 position;
  float constant;
  float linear;
  float quadratic;
};

// Spot light data
struct spot_light {
  vec4 light_colour;
  vec3 position;
  vec3 direction;
  float constant;
  float linear;
  float quadratic;
  float power;
};
// A material structure
struct material {
  vec4 emissive;
  vec4 diffuse_reflection;
  vec4 specular_reflection;
  float shininess;
};

layout(std430, binding = 1) buffer DirectionalLights {
  directional_light DLights[];
};
layout(std430, binding = 2) buffer PointsLights { point_light PLights[]; };
layout(std430, binding = 3) buffer SpotLights { spot_light SLights[]; };

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
  vec3 lv = normalize(vec3(light.light_dir) * TBN);
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

// Spot light calculation
vec4 calculate_point(in point_light point, in material mat, in vec3 position,
                     in vec3 normal, in vec3 view_dir, in vec4 tex_colour) {
  // Get distance between point light and vertex
  float d = distance(point.position, position);
  // Calculate attenuation factor
  float attenuation =
      point.constant + (point.linear * d) + (point.quadratic * d * d);
  // Calculate light colour
  vec4 light_colour = point.light_colour / attenuation;
  light_colour.a = 1.0;
  // Calculate light dir
  vec3 light_dir = normalize(point.position - position);
  // Now use standard phong shading but using calculated light colour and
  // direction
  // - note no ambient
  vec4 diffuse =
      (mat.diffuse_reflection * light_colour) * max(dot(normal, light_dir), 0);
  vec3 half_vector = normalize(light_dir + view_dir);
  vec4 specular = (mat.specular_reflection * light_colour) *
                  pow(max(dot(normal, half_vector), 0), mat.shininess);
  vec4 primary = mat.emissive + diffuse;
  vec4 colour = primary * tex_colour + specular;
  colour.a = 1.0;
  return colour;
}

vec4 calculate_spot(in spot_light spot, in material mat, in vec3 position, in vec3 normal, in vec3 view_dir, in vec4 tex_colour)
{
  // Calculate direction to the light
  vec3 light_dir = normalize(spot.position - position);
  // Calculate distance to light
  float d = distance(spot.position, position);
  // Calculate attenuation value
  float attenuation = spot.constant + spot.linear * d + spot.quadratic * d * d;
  // Calculate spot light intensity
  float sp = pow(max(dot(light_dir, -spot.direction), 0.0), spot.power);
  // Calculate light colour
  vec4 light_colour = (sp / attenuation) * spot.light_colour;
  // Now use standard phong shading but using calculated light colour and
  // direction
  // - note no ambient
  vec4 diffuse = (mat.diffuse_reflection * light_colour) *
                 max(dot(normal, light_dir), 0.0);
  vec3 half_vector = normalize(light_dir + view_dir);
  vec4 specular = (mat.specular_reflection * light_colour) *
                  pow(max(dot(normal, half_vector), 0.0), mat.shininess);

  vec4 colour = ((mat.emissive + diffuse) * tex_colour) + specular;
  colour.a = 1.0;

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
  for (int i = 0; i < DLights.length() ; i++) {
    colour +=
        calculate_dir(DLights[i], mat, position, normal, view_dir, tex_colour);
  }
  // point light
  for (int i = 0; i < PLights.length(); i++) {
    colour += calculate_point(PLights[i], mat, position, normal, view_dir,tex_colour);
  }
  // spotlight
  for (int i = 0; i < SLights.length(); i++) {
    colour += calculate_spot(SLights[i], mat, position, normal, view_dir, tex_colour);
  }

  colour.a = fade;
}