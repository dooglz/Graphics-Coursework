#version 430
// A material structure
struct material {
  vec4 emissive;
  vec4 diffuse_reflection;
  vec4 specular_reflection;
  float shininess;
};

in vec2 TexCoord0; 
in vec3 Normal0; 
in vec3 WorldPos0; 

layout (location = 0) out vec3 WorldPosOut; 
layout (location = 1) out vec3 DiffuseOut; 
layout (location = 2) out vec3 NormalOut; 
layout (location = 3) out vec3 TexCoordOut; 
layout (location = 4) out vec3 InfoOut; 

uniform sampler2D gColorMap; 
// Material of the object, TODO: make a material buffer
uniform material mat;

void main() 
{ 
    WorldPosOut = WorldPos0; 
    DiffuseOut = texture(gColorMap, TexCoord0).xyz; 
    NormalOut = normalize(Normal0); 
    TexCoordOut = vec3(TexCoord0, 0.0); 
    InfoOut = vec3(0,0.5,mat.shininess); 
}