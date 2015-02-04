#version 400

// Point light information
struct point_light
{
	vec4 light_colour;
	vec3 position;
	float constant;
	float linear;
	float quadratic;
};

// Material information
struct material
{
	vec4 emissive;
	vec4 diffuse_reflection;
	vec4 specular_reflection;
	float shininess;
};

// Point light for the scene
uniform point_light point;
// Material for the object
uniform material mat;
// Eye position
uniform vec3 eye_pos;
// Texture
uniform sampler2D tex;

// Incoming position
layout (location = 0) in vec3 position;
// Incoming normal
layout (location = 1) in vec3 normal;
// Incoming texture coordinate
layout (location = 2) in vec2 tex_coord;

// Outgoing colour
layout (location = 0) out vec4 colour;

void main()
{
	// *******************************************
	// Get distance between point light and vertex
	// *******************************************
	float d = distance(point.position, position);
	// ****************************
	// Calculate attenuation factor
	// ****************************
	float attenuation = point.constant + (point.linear * d) + (point.quadratic * d * d);
	// **********************
	// Calculate light colour
	// **********************
	vec4 light_colour = point.light_colour / attenuation;
	light_colour.a = 1.0;
	// *******************
	// Calculate light dir
	// *******************
	vec3 light_dir = normalize(point.position - position);
	// ******************************************************************************
	// Now use standard phong shading but using calculated light colour and direction
	// - note no ambient
	// ******************************************************************************
	vec4 diffuse = (mat.diffuse_reflection * light_colour) * max(dot(normal, light_dir), 0);
	vec3 view_dir = normalize(eye_pos - position);
	vec3 half_vector = normalize(light_dir + view_dir);
	vec4 specular = (mat.specular_reflection * light_colour) * pow(max(dot(normal, half_vector), 0), mat.shininess);
	vec4 tex_colour = texture(tex, tex_coord);
	vec4 primary = mat.emissive + diffuse;
	colour = primary * tex_colour + specular;
	colour.a = 1.0;
}