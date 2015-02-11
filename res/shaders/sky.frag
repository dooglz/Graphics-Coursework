#version 440
//direction the player is facing
uniform vec3 playerview;

// Incoming vertex colour
layout (location = 0) in vec4 in_colour;

// Outgoing pixel colour
layout (location = 0) out vec4 out_colour;

vec3 up = vec3(0,1,0);
//vec3 topcol = vec3(0.55,0.75,0.83);
//vec3 bottomcol = vec3(0.5,0.78,0.98);




void main()
{
	//float f = dot(playerview,up);
	// Simply set outgoing colour
	//out_colour = vec4(mix(bottomcol,topcol,f),1.0);
	out_colour = in_colour;
}