#version 440
//direction the player is facing
uniform vec3 playerview;

// Outgoing pixel colour
layout (location = 0) out vec4 out_colour;

void main()
{
	// Simply set outgoing colour
	out_colour = vec4(1.0,0.0,0.0,0.5);
}