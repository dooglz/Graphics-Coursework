#version 440
layout (location = 0) in vec2 vertexIn;

void main()
{
	// Calculate screen position of vertex
	gl_Position = vec4(vertexIn.xy,0.5,1.0);
}