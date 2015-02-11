#version 440

uniform float topdot;
uniform float bottomdot;

layout (location = 0) in vec2 vertexIn;
layout (location = 0) out vec4 out_colour;

vec3 topcol = vec3(1,0,0);
vec3 bottomcol = vec3(0,1,0);

void main()
{
	if(vertexIn.y >0){
		out_colour = vec4(mix(bottomcol,topcol,topdot),1.0);
	}else{
		out_colour = vec4(mix(bottomcol,topcol,bottomdot),1.0);
	}
	// Calculate screen position of vertex
	gl_Position = vec4(vertexIn.xy,0.99999,1.0);
}