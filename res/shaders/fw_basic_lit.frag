#version 430
#extension GL_ARB_shader_storage_buffer_object : require

struct directional_light {
  vec4 ambient_intensity;
  vec4 light_colour;
  vec3 light_dir;
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

// Position of the camera
uniform vec3 eye_pos;
// Texture
uniform sampler2D tex;
// Material of the object
uniform material mat;

// Incoming position
layout(location = 0) in vec3 position;
// Incoming normal
layout(location = 1) in vec3 normal;
// Incoming texture coordinate
layout(location = 2) in vec2 tex_coord;
// Outgoing colour
layout(location = 0) out vec4 colour;

uniform directional_light DLights[20];
uniform point_light PLights[20];
uniform spot_light SLights[20];
uniform vec4 lightNumbers;

vec4 calculate_dir(in directional_light light, in vec3 position, in vec3 normal, in vec3 view_dir, in vec4 tex_colour) {
  const vec3 light_dir = -light.light_dir;

  vec4 ambient = mat.diffuse_reflection * light.ambient_intensity;
  vec4 diffuse = vec4(0, 0, 0, 0);
  vec4 specular = vec4(0, 0, 0, 0);

  float DiffuseFactor = dot(normal, light_dir);

  if (DiffuseFactor > 0.0) {
    diffuse = (mat.diffuse_reflection * light.light_colour) * DiffuseFactor;
    // Calculate half vector
    vec3 half_vector = normalize(light_dir + view_dir);

    float SpecularFactor = dot(normal, half_vector);
    SpecularFactor = pow(SpecularFactor, mat.shininess);
    if (SpecularFactor > 0.0) {
      specular = (mat.specular_reflection * light.light_colour) * SpecularFactor;
    }
  }

  vec4 colour = (mat.emissive + ambient + diffuse) * tex_colour + specular;
  colour.a = 1.0;
  return colour;
}

// Spot light calculation
vec4 calculate_point(in point_light point, in vec3 position, in vec3 normal, in vec3 view_dir, in vec4 tex_colour) {
  // Get distance between point light and vertex
  float d = distance(vec3(point.position), position);
  // Calculate attenuation factor
  float attenuation = point.constant + (point.linear * d) + (point.quadratic * d * d);
  // Calculate light colour
  vec4 light_colour = point.light_colour / attenuation;
  light_colour.a = 1.0;
  // Calculate light dir
  vec3 light_dir = normalize(vec3(point.position) - position);
  // Now use standard phong shading but using calculated light colour and direction
  // - note no ambient
  vec4 diffuse = (mat.diffuse_reflection * light_colour) * max(dot(normal, light_dir), 0);
  vec3 half_vector = normalize(light_dir + view_dir);
  vec4 specular = (mat.specular_reflection * light_colour) * pow(max(dot(normal, half_vector), 0), mat.shininess);
  vec4 primary = mat.emissive + diffuse;
  vec4 colour = primary * tex_colour + specular;
  colour.a = 1.0;
  return colour;
}

vec4 calculate_spot(in spot_light spot, in vec3 position, in vec3 normal, in vec3 view_dir, in vec4 tex_colour) {
  // Calculate direction to the light
  vec3 light_dir = normalize(vec3(spot.position) - position);

  // Calculate distance to light
  float d = distance(vec3(spot.position), position);
  // Calculate attenuation value
  float attenuation = spot.constant + spot.linear * d + spot.quadratic * d * d;
  // Calculate spot light intensity
  float sp = pow(max(dot(light_dir, -vec3(spot.direction)), 0.0), spot.power);
  // Calculate light colour
  vec4 light_colour = (sp / attenuation) * spot.light_colour;
  // Now use standard phong shading but using calculated light colour and direction
  // - note no ambient
  vec4 diffuse = (mat.diffuse_reflection * light_colour) * max(dot(normal, light_dir), 0.0);
  vec3 half_vector = normalize(light_dir + view_dir);
  vec4 specular = (mat.specular_reflection * light_colour) * pow(max(dot(normal, half_vector), 0.0), mat.shininess);

  vec4 colour = ((mat.emissive + diffuse) * tex_colour) + specular;
  colour.a = 1.0;

  return colour;
}

//---------------------------------
void main() {
  // Calculate view direction
  vec3 view_dir = normalize(eye_pos - position);
  // Sample texture
  vec4 tex_colour = texture(tex, tex_coord);

  colour = vec4(0, 0, 0, 1);

  // for each directional light
  for (int i = 0; i < lightNumbers.x; i++) {
    colour += calculate_dir(DLights[i], position, normal, view_dir, tex_colour);
  }
  // point light
  for (int i = 0; i < lightNumbers.y; i++) {
    colour += calculate_point(PLights[i], position, normal, view_dir, tex_colour);
  }
  // spotlight
  for (int i = 0; i < lightNumbers.z; i++) {
    colour += calculate_spot(SLights[i], position, normal, view_dir, tex_colour);
  }

  colour.a = 1.0;
}