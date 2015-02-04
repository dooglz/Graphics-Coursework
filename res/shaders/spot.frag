#version 400

// Spot light data
struct spot_light
{
	vec4 light_colour;
	vec3 position;
	vec3 direction;
	float constant;
	float linear;
	float quadratic;
	float power;
};

// Material data
struct material
{
	vec4 emissive;
	vec4 diffuse_reflection;
	vec4 specular_reflection;
	float shininess;
};

// Spot light being used in the scene
uniform spot_light spot;
// Material of the object being rendered
uniform material mat;
// Position of the eye (camera)
uniform vec3 eye_pos;
// Texture to sample from
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
	// ********************************
	// Calculate direction to the light
	// ********************************
	vec3 light_dir = normalize(spot.position - position);
	// ***************************
	// Calculate distance to light
	// ***************************
	float d = distance(spot.position, position);
	// ***************************
	// Calculate attenuation value
	// ***************************
	float attenuation = spot.constant + spot.linear * d + spot.quadratic * d * d;
	// ******************************
	// Calculate spot light intensity
	// ******************************
	float sp = pow(max(dot(light_dir, -spot.direction), 0.0), spot.power);
	// **********************
	// Calculate light colour
	// **********************
	vec4 light_colour = (sp / attenuation) * spot.light_colour;
	// ************************
	// Calculate view direction
	// ************************
	vec3 view_dir = normalize(eye_pos - position);

	// ******************************************************************************
	// Now use standard phong shading but using calculated light colour and direction
	// - note no ambient
	// ******************************************************************************
	vec4 diffuse = (mat.diffuse_reflection * light_colour) * max(dot(normal, light_dir), 0.0);
	vec3 half_vector = normalize(light_dir + view_dir);
	vec4 specular = (mat.specular_reflection * light_colour) * pow(max(dot(normal, half_vector), 0.0), mat.shininess);
	vec4 tex_colour = texture(tex, tex_coord);
	colour = ((mat.emissive + diffuse) * tex_colour) + specular;
	colour.a = 1.0;
}