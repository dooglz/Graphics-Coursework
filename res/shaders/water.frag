#version 430

// Sampler used to get texture colour
uniform sampler2D tex;
//could be the mvp of the reflection camera
uniform mat4 o2v_projection_reflection;

// Incoming texture coordinate
layout(location = 0) in vec2 tex_coord;
// Outgoing colour
layout(location = 0) out vec4 out_colour;

layout(location = 1) in vec3 position;

void main()
{
	vec4 vClipReflection = o2v_projection_reflection * vec4(position.xy, 0.0 , 1.0);
	vec2 vDeviceReflection = vClipReflection.st / vClipReflection.q;
	vec2 vTextureReflection = vec2(0.5, 0.5) + 0.5 * vDeviceReflection;

	vec4 reflectionTextureColor = texture2D (tex, vTextureReflection);

	// Framebuffer reflection can have alpha > 1
	reflectionTextureColor.a = 1.0;

	out_colour = reflectionTextureColor;
	//out_colour = texture2D (tex, tex_coord);
}