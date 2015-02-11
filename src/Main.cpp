#include "libENUgraphics\graphics_framework.h"
#include <glm\glm.hpp>
#include <glm\gtx\rotate_vector.hpp>

using namespace std;
using namespace graphics_framework;
using namespace glm;

map<string, mesh> meshes;
effect eff;
effect skyeffect;
texture tex;
free_camera cam;
mesh quadmesh;
double cursor_x = 0.0;
double cursor_y = 0.0;

bool initialise()
{
	// ********************************
	// Set input mode - hide the cursor
	// ********************************
	glfwSetInputMode(renderer::get_window(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	// ******************************
	// Capture initial mouse position
	// ******************************
	glfwGetCursorPos(renderer::get_window(), &cursor_x, &cursor_y);

	return true;
}

bool load_content()
{
	// Create plane mesh
	meshes["plane"] = mesh(geometry_builder::create_plane());

	// Create scene
	meshes["box"] = mesh(geometry_builder::create_box());
	meshes["tetra"] = mesh(geometry_builder::create_tetrahedron());
	meshes["pyramid"] = mesh(geometry_builder::create_pyramid());
	meshes["disk"] = mesh(geometry_builder::create_disk(20));
	meshes["cylinder"] = mesh(geometry_builder::create_cylinder(20, 20));
	meshes["sphere"] = mesh(geometry_builder::create_sphere(20, 20));
	meshes["torus"] = mesh(geometry_builder::create_torus(20, 20, 1.0f, 5.0f));

	// Transform objects
	meshes["box"].get_transform().scale = vec3(5.0f, 5.0f, 5.0f);
	meshes["box"].get_transform().translate(vec3(-10.0f, 2.5f, -30.0f));
	meshes["tetra"].get_transform().scale = vec3(4.0f, 4.0f, 4.0f);
	meshes["tetra"].get_transform().translate(vec3(-30.0f, 10.0f, -10.0f));
	meshes["pyramid"].get_transform().scale = vec3(5.0f, 50.0f, 5.0f);
	meshes["pyramid"].get_transform().translate(vec3(0, 25, 0));
	meshes["disk"].get_transform().scale = vec3(3.0f, 1.0f, 3.0f);
	meshes["disk"].get_transform().translate(vec3(-10.0f, 11.5f, -30.0f));
	meshes["disk"].get_transform().rotate(vec3(half_pi<float>(), 0.0f, 0.0f));
	meshes["cylinder"].get_transform().scale = vec3(5.0f, 5.0f, 5.0f);
	meshes["cylinder"].get_transform().translate(vec3(-25.0f, 2.5f, -25.0f));
	meshes["sphere"].get_transform().scale = vec3(2.5f, 2.5f, 2.5f);
	meshes["sphere"].get_transform().translate(vec3(-25.0f, 10.0f, -25.0f));
	meshes["torus"].get_transform().translate(vec3(-25.0f, 10.0f, -25.0f));
	meshes["torus"].get_transform().rotate(vec3(half_pi<float>(), 0.0f, 0.0f));

	// Load texture
	tex = texture("textures\\checked.gif");

	// Load in shaders
	eff.add_shader("shaders\\simple_texture.vert", GL_VERTEX_SHADER);
	eff.add_shader("shaders\\simple_texture.frag", GL_FRAGMENT_SHADER);
	// Build effect
	eff.build();

	// Set camera properties
	cam.set_position(vec3(0.0f, 10.0f, 0.0f));
	cam.set_target(vec3(0.0f, 0.0f, 0.0f));
	auto aspect = static_cast<float>(renderer::get_screen_width()) / static_cast<float>(renderer::get_screen_height());
	cam.set_projection(quarter_pi<float>(), aspect, 2.414f, 1000.0f);


	//sky shader props.
	skyeffect.add_shader("shaders\\sky.vert", GL_VERTEX_SHADER);
	skyeffect.add_shader("shaders\\sky.frag", GL_FRAGMENT_SHADER);
	// Build effect
	skyeffect.build();

	graphics_framework::geometry geo;
	std::vector<vec2> v = { vec2(-1, -1), vec2(-1, 1), vec2(1, 1) };
	geo.add_buffer(v,0);
	quadmesh.set_geometry(geo);

	glDisable(GL_CULL_FACE);
	return true;
}

bool update(float delta_time)
{
	// The ratio of pixels to rotation - remember the fov
	static double ratio_width = quarter_pi<float>() / static_cast<float>(renderer::get_screen_width());
	static double ratio_height = (quarter_pi<float>() * (static_cast<float>(renderer::get_screen_height()) / static_cast<float>(renderer::get_screen_width()))) / static_cast<float>(renderer::get_screen_height());

	double current_x;
	double current_y;
	// *******************************
	// Get the current cursor position
	// *******************************
	glfwGetCursorPos(renderer::get_window(), &current_x, &current_y);

	// ***************************************************
	// Calculate delta of cursor positions from last frame
	// ***************************************************
	double delta_x = current_x - cursor_x;
	double delta_y = current_y - cursor_y;

	// *************************************************************
	// Multiply deltas by ratios - gets actual change in orientation
	// *************************************************************
	delta_x *= ratio_width;
	delta_y *= -ratio_height;

	// *************************
	// Rotate cameras by delta
	// delta_y - x-axis rotation
	// delta_x - y-axis rotation
	// *************************
	cam.rotate(delta_x, delta_y);

	// *******************************
	// Use keyboard to move the camera
	// - WSAD
	// *******************************
	vec3 translation(0.0f, 0.0f, 0.0f);
	if (glfwGetKey(renderer::get_window(), 'W'))
		translation.z += 5.0f * delta_time;
	if (glfwGetKey(renderer::get_window(), 'S'))
		translation.z -= 5.0f * delta_time;
	if (glfwGetKey(renderer::get_window(), 'A'))
		translation.x -= 5.0f * delta_time;
	if (glfwGetKey(renderer::get_window(), 'D'))
		translation.x += 5.0f * delta_time;

	// ***********
	// Move camera
	// ***********
	cam.move(translation);

	// *****************
	// Update the camera
	// *****************
	cam.update(delta_time);

	// *****************
	// Update cursor pos
	// *****************
	cursor_x = current_x;
	cursor_y = current_y;

	return true;
}

bool render()
{
	// Render meshes
	for (auto &e : meshes)
	{
		auto m = e.second;
		// Bind effect
		renderer::bind(eff);
		// Create MVP matrix
		auto M = m.get_transform().get_transform_matrix();
		auto V = cam.get_view();
		auto P = cam.get_projection();
		auto MVP = P * V * M;
		// Set MVP matrix uniform
		glUniformMatrix4fv(
			eff.get_uniform_location("MVP"), // Location of uniform
			1, // Number of values - 1 mat4
			GL_FALSE, // Transpose the matrix?
			value_ptr(MVP)); // Pointer to matrix data

		// Bind and set texture
		renderer::bind(tex, 0);
		glUniform1i(eff.get_uniform_location("tex"), 0);

		// Render mesh
		renderer::render(m);
	}
	//Render Sky
	{
		float verticleFov = 0.392699082f; //45/2 in radians

		vec3 camview = cam.get_target() - cam.get_position();
		camview = normalize(camview);

		vec3 topofscreentoplayer = glm::rotate(camview, verticleFov * 0.5f, vec3(0,0,1.0f));
		topofscreentoplayer = normalize(topofscreentoplayer);
		vec3 bottomofscreentoplayer = glm::rotate(camview, verticleFov * -0.5f, vec3(0,0,1.0f));
		bottomofscreentoplayer = normalize(bottomofscreentoplayer);

		float topDot = dot(topofscreentoplayer, vec3(0, 1.0, 0));
		float bottomDot = dot(bottomofscreentoplayer, vec3(0, 1.0, 0));


		float dt = dot(camview, vec3(0, 1.0, 0));

		//camview *= 1 / pi<float>();
		//printf("x: %f y: %f z:%f\n", camview.x, camview.y, camview.z);
		//printf("dot: %f\n", dt);

		renderer::bind(skyeffect);
		glUniform1f(skyeffect.get_uniform_location("topDot"), topDot);
		glUniform1f(skyeffect.get_uniform_location("bottomDot"), bottomDot);
		glUniform3f(skyeffect.get_uniform_location("playerview"), camview.x, camview.y, camview.z);

		graphics_framework::geometry geo;
		std::vector<vec2> v = { vec2(-1, -1), vec2(-1, 1), vec2(1, 1),	vec2(1, 1), vec2(1, -1), vec2(-1, -1) };
		geo.add_buffer(v, 0);

		renderer::render(geo);
	}

	return true;
}

void main()
{
	// Create application
	app application(1280,720,0);
	// Set load content, update and render methods
	application.set_load_content(load_content);
	application.set_initialise(initialise);
	application.set_update(update);
	application.set_render(render);
	// Run application
	application.run();
}