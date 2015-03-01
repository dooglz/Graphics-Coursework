#version 440

// Model view projection matrix  = otv
uniform mat4 o2v;
uniform mat4 o2v_projection;

layout(location = 0) in vec3 vertex;
layout(location = 1) out vec3 interpolatedVertexEye;
layout(location = 2) out vec3 interpolatedVertexObject;

void main()
{
  interpolatedVertexObject = vertex;
	vec4 vertexEye = o2v * vec4(vertex.xy, 0.0, 1.0);
	interpolatedVertexEye = vertexEye.xyz;

	gl_Position = o2v_projection * vec4(vertex.xyz, 1.0);
}