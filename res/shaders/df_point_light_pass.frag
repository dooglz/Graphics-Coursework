#version 330

struct point_light {
  vec4 light_colour;
  vec3 position;
  float constant;
  float linear;
  float quadratic;
};

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

uniform point_light gPointLight;
uniform vec3 gEyeWorldPos;
uniform vec2 gScreenSize;

material mat;

vec4 calculate_point(in vec3 position, in vec3 normal, in vec3 view_dir, in vec4 tex_colour) {
  // Get distance between point light and vertex
  float d = distance(vec3(gPointLight.position), position);
  // Calculate attenuation factor
  float attenuation = gPointLight.constant + (gPointLight.linear * d) + (gPointLight.quadratic * d * d);
  // Calculate light colour
  vec4 light_colour = gPointLight.light_colour / attenuation;
  light_colour.a = 1.0;
  // Calculate light dir
  vec3 light_dir = normalize(vec3(gPointLight.position) - position);
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

vec2 CalcTexCoord() { return gl_FragCoord.xy / gScreenSize; }

out vec4 FragColor;

void main() {
  vec2 TexCoord = CalcTexCoord();
  vec3 WorldPos = texture(gPositionMap, TexCoord).xyz;
  vec3 Color = texture(gColorMap, TexCoord).xyz;
  vec3 Normal = normalize(texture(gNormalMap, TexCoord).xyz);
  vec3 Info = texture(gInfoMap, TexCoord).xyz;

 vec3 view_dir = normalize(gEyeWorldPos - WorldPos);

  // reconstruct material
  mat.shininess = Info.z;
  mat.emissive = vec4(0, 0, 0, 0);
  mat.diffuse_reflection = texture(gMaterialMap, TexCoord);
  mat.specular_reflection = vec4(0, 0, 0, 0);

  if(Info.x == 1.0){
    FragColor = vec4(Color, 1.0);
  }else{
    //FragColor = vec4(Color, 1.0) * CalcPointLight(WorldPos, Normal);
	FragColor = calculate_point(WorldPos, Normal, view_dir, vec4(Color, 1.0));
  }
}
