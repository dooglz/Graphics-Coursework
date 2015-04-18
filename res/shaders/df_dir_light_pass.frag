#version 330

struct directional_light {
  vec4 ambient_intensity;
  vec4 light_colour;
  vec3 light_dir;
};
// A material structure
struct material {
  vec4 emissive;
  vec4 diffuse_reflection;
  vec4 specular_reflection;
  float shininess;
};

uniform sampler2D gPositionMap;
uniform sampler2D gColorMap;
uniform sampler2D gNormalMap;
uniform sampler2D gMaterialMap;
uniform sampler2D gInfoMap;

uniform directional_light gDirectionalLight;
uniform vec3 gEyeWorldPos;
uniform vec2 gScreenSize;

material mat;

vec4 calculate_dir(in vec3 position, in vec3 normal, in vec3 view_dir, in vec4 tex_colour) {
  vec3 light_dir = -gDirectionalLight.light_dir;

  vec4 ambient = mat.diffuse_reflection * gDirectionalLight.ambient_intensity;
  vec4 diffuse = vec4(0, 0, 0, 0);
  vec4 specular = vec4(0, 0, 0, 0);

  float DiffuseFactor = dot(normal, light_dir);

  if (DiffuseFactor > 0.0) {
    diffuse = (mat.diffuse_reflection * gDirectionalLight.light_colour) * DiffuseFactor;
    vec3 half_vector = normalize(light_dir + view_dir);

    float SpecularFactor = dot(normal, half_vector);
    SpecularFactor = pow(SpecularFactor, mat.shininess);
    if (SpecularFactor > 0.0) {
      specular = (mat.specular_reflection * gDirectionalLight.light_colour) * SpecularFactor;
    }
  }

  vec4 colour = (mat.emissive + ambient + diffuse) * tex_colour + specular;
  colour.a = 1.0;
  return colour;
}

vec2 CalcTexCoord() { return gl_FragCoord.xy / gScreenSize; }

out vec4 FragColor;

void main() {
  vec2 TexCoord = CalcTexCoord();
  vec3 WorldPos = texture(gPositionMap, TexCoord).xyz;
  vec3 Color = texture(gColorMap, TexCoord).xyz;
  vec3 Normal = texture(gNormalMap, TexCoord).xyz;
  Normal = normalize(Normal);
  vec3 Info = texture(gInfoMap, TexCoord).xyz;
  vec3 view_dir = normalize(gEyeWorldPos - WorldPos);

  // reconstruct material
  mat.shininess = Info.z;
  mat.emissive = vec4(0, 0, 0, 0);
  mat.diffuse_reflection = texture(gMaterialMap, TexCoord);
  mat.specular_reflection = vec4(0, 0, 0, 0);

  if (Info.x == 1.0) {
    FragColor = vec4(Color, 1.0);
  } else {
    // FragColor = vec4(Color, 1.0) * CalcDirectionalLight(WorldPos, Normal);
    FragColor = calculate_dir(WorldPos, Normal, view_dir, vec4(Color, 1.0));
  }
}
