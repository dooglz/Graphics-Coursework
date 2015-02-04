#include <libENUgraphics/graphics_framework.h>
#include <glm\glm.hpp>

using namespace std;
using namespace graphics_framework;
using namespace glm;

geometry geom;
effect eff;
target_camera cam;

const int num_points = 500;

void create_sierpinski(geometry &geom)
{
	vector<vec3> points;
	vector<vec4> colours;
	// Three corners of the triangle
	array<vec3, 3> v =
	{
		vec3(-2.0f, -2.0f, 0.0f),
		vec3(0.0f, 2.0f, 0.0f),
		vec3(2.0f, -2.0f, 0.0f)
	};
	// Create random engine - generates random numbers
	default_random_engine e;
	// Create a distribution.  3 points in array so want 0-2
	uniform_int_distribution<int> dist(0, 2);
	// Add first point to the geometry
	points.push_back(vec3(0.25f, 0.5f, 0.0f));
	// Add first colour to the geometry
	colours.push_back(vec4(1.0f, 0.0f, 0.0f, 1.0f));
	// Add random points using distribution
	for (auto i = 1; i < num_points; ++i)
	{
		// Add random point
		points.push_back((points[i - 1] + v[dist(e)]) / 2.0f);
		// Add colour
		colours.push_back(vec4(1.0f, 0.0f, 0.0f, 1.0f));
	}
	// Add buffers to geometry
	geom.add_buffer(points, BUFFER_INDEXES::POSITION_BUFFER);
	geom.add_buffer(colours, BUFFER_INDEXES::COLOUR_BUFFER);
}

bool load_content()
{
	// Set to points type
	geom.set_type(GL_POINTS);
	// Create sierpinski gasket
	create_sierpinski(geom);

	// Load in shaders
	eff.add_shader(
		"shaders\\basic.vert", // filename
		GL_VERTEX_SHADER); // type
	eff.add_shader(
		"shaders\\basic.frag", // filename
		GL_FRAGMENT_SHADER); // type
	// Build effect
	eff.build();

	// Set camera properties
	cam.set_position(vec3(2.0f, 2.0f, 2.0f));
	cam.set_target(vec3(0.0f, 0.0f, 0.0f));
	auto aspect = static_cast<float>(renderer::get_screen_width()) / static_cast<float>(renderer::get_screen_height());
	cam.set_projection(quarter_pi<float>(), aspect, 2.414f, 1000.0f);
	return true;
}

bool update(float delta_time)
{
	// Update the camera
	cam.update(delta_time);
	return true;
}

bool render()
{
	// Bind effect
	renderer::bind(eff);
	// Create MVP matrix
	mat4 M(1.0f);
	auto V = cam.get_view();
	auto P = cam.get_projection();
	auto MVP = P * V * M;
	// Set MVP matrix uniform
	glUniformMatrix4fv(
		eff.get_uniform_location("MVP"), // Location of uniform
		1, // Number of values - 1 mat4
		GL_FALSE, // Transpose the matrix?
		value_ptr(MVP)); // Pointer to matrix data
	// Render geometry
	glDisable(GL_POINT_SPRITE_ARB);
	glDisable(GL_POINT_SPRITE);
	renderer::render(geom);
	return true;
}

void main()
{
	// Create application
	app application;
	// Set load content, update and render methods
	application.set_load_content(load_content);
	application.set_update(update);
	application.set_render(render);
	// Run application
	application.run();
}