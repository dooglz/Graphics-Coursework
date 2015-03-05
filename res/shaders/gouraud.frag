#version 430

// Texture to sample from
uniform sampler2D tex;

// Incoming primary colour
layout (location = 0) in vec4 primary;
// Incoming secondary colour
layout (location = 1) in vec4 secondary;
// Incoming texture coordinate
layout (location = 2) in vec2 tex_coord;

// Outgoing colour
layout (location = 0) out vec4 colour;

void main()
{
	// **************
	// Sample texture
	// **************
	vec4 tex_colour = texture(tex, tex_coord);
	// ****************
	// Calculate colour
	// ****************
	colour = primary * tex_colour + secondary;
}