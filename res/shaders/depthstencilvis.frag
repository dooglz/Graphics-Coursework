#version 420
#extension GL_EXT_gpu_shader4 : enable 

uniform sampler2D depthTex;
uniform usampler2D stencilTex;
uniform vec2 gScreenSize;

vec2 CalcTexCoord() { return gl_FragCoord.xy / gScreenSize; }

layout (location = 0) out vec3 depthCol; 
layout (location = 1) out vec3 stencilCol; 

void main() {

	vec2 TexCoord = CalcTexCoord();
	float stencil = float(texture2D(stencilTex, TexCoord).x);
	stencilCol = vec3(stencil * 1000.0 ,stencil * 100.0,stencil * 10.0);
	//stencilCol = vec3(1.0,0,0);

	float depth = texture2D(depthTex, TexCoord).x;
	depthCol = vec3(depth,depth,depth);
	//depthCol = vec3(0,1.00,0);
	//depthCol = vec3(1000,1000.00,1000);
}
