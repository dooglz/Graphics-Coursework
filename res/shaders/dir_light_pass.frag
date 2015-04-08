#version 330

struct BaseLight {
  vec3 Color;
  float AmbientIntensity;
  float DiffuseIntensity;
};

struct DirectionalLight {
  BaseLight Base;
  vec3 Direction;
};

uniform sampler2D gPositionMap;
uniform sampler2D gColorMap;
uniform sampler2D gNormalMap;
uniform sampler2D gInfoMap;
uniform DirectionalLight gDirectionalLight;
uniform vec3 gEyeWorldPos;
uniform float gMatSpecularIntensity;
uniform float gSpecularPower;
uniform vec2 gScreenSize;

vec4 CalcLightInternal(BaseLight Light, vec3 LightDirection, vec3 WorldPos, vec3 Normal) {
  vec4 AmbientColor = vec4(Light.Color, 1.0) * Light.AmbientIntensity;
  float DiffuseFactor = dot(Normal, -LightDirection);

  vec4 DiffuseColor = vec4(0, 0, 0, 0);
  vec4 SpecularColor = vec4(0, 0, 0, 0);

  if (DiffuseFactor > 0.0) {
    DiffuseColor = vec4(Light.Color, 1.0) * Light.DiffuseIntensity * DiffuseFactor;

    vec3 VertexToEye = normalize(gEyeWorldPos - WorldPos);
    vec3 LightReflect = normalize(reflect(LightDirection, Normal));
    float SpecularFactor = dot(VertexToEye, LightReflect);
    SpecularFactor = pow(SpecularFactor, gSpecularPower);
    if (SpecularFactor > 0.0) {
      SpecularColor = vec4(Light.Color, 1.0) * gMatSpecularIntensity * SpecularFactor;
    }
  }
  //return (AmbientColor + DiffuseColor);
  return (AmbientColor + DiffuseColor + SpecularColor);
}

vec4 CalcDirectionalLight(vec3 WorldPos, vec3 Normal) {
  return CalcLightInternal(gDirectionalLight.Base, gDirectionalLight.Direction, WorldPos, Normal);
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

  if(Info.x ==  1.0){
    FragColor = vec4(Color, 1.0);
  }else{
    FragColor = vec4(Color, 1.0) * CalcDirectionalLight(WorldPos, Normal);
  }
}