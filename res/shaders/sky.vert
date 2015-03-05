#version 430

uniform float topdot;
uniform float bottomdot;
uniform vec3 topcol;
uniform vec3 bottomcol;

layout (location = 0) in vec2 vertexIn;
layout (location = 0) out vec4 out_colour;

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