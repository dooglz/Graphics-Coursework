#version 430

layout (location = 0) in vec3 Position; 
layout (location = 2) in vec3 Normal; 
layout (location = 10) in vec2 TexCoord; 

uniform mat4 MVP;
uniform mat4 M;
uniform mat3 N;

out vec2 TexCoord0; 
out vec3 Normal0; 
out vec3 WorldPos0; 

void main()
{ 
    gl_Position = MVP * vec4(Position, 1.0);
    TexCoord0 = TexCoord; 
    Normal0 = (N * Normal); 
    WorldPos0 = (M * vec4(Position, 1.0)).xyz;
}