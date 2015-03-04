#include "main.h"
#include "libENUgraphics\graphics_framework.h"
#include <glm\glm.hpp>
#include <glm\gtx\rotate_vector.hpp>
#include <glm\gtx\quaternion.hpp>
#include <glm\gtc\matrix_transform.hpp>
#include "DesertGen.h"
#include "Math.h"
#include "water.h"
#include <functional>

using namespace std;
using namespace graphics_framework;
using namespace glm;

double cursor_x = 0.0;
double cursor_y = 0.0;

static std::vector<const glm::vec3> linebuffer;
static float counter = 0;
void graphics::DrawLine(const glm::vec3 &p1, const glm::vec3 &p2) {
  linebuffer.push_back(p1);
  linebuffer.push_back(p2);
}

void graphics::DrawCross(const glm::vec3 &p1, const float size) {
  DrawLine(p1 + glm::vec3(size, 0, 0), p1 - glm::vec3(size, 0, 0));
  DrawLine(p1 + glm::vec3(0, size, 0), p1 - glm::vec3(0, size, 0));
  DrawLine(p1 + glm::vec3(0, 0, size), p1 - glm::vec3(0, 0, size));
}

bool graphics::initialise() {
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

bool graphics::load_content() {

  // Lights
  light.set_ambient_intensity(vec4(0.3f, 0.3f, 0.3f, 1.0f));
  light.set_light_colour(vec4(0.8f, 0.8f, 0.8f, 1.0f));
  light.set_direction(vec3(1.0f, 1.0f, -1.0f));

  shader_data_t shader_data;
  shader_data.ambient_intensity = light.get_ambient_intensity();
  shader_data.light_colour = light.get_light_colour();
  shader_data.light_dir = light.get_direction();

  assert(GL_SHADER_STORAGE_BUFFER);
  glGenBuffers(1, &ssbo);
  glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo);
  //target, index, buffer, offset, size
  //glBindBufferRange(GL_SHADER_STORAGE_BUFFER, 2, ssbo, 0, 128);
  glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(shader_data), &shader_data, GL_DYNAMIC_COPY);
  glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

  //update ssbo with data
  glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo);
  GLvoid* p = glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_WRITE_ONLY);
  memcpy(p, &shader_data, sizeof(shader_data));
  glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);

  mirror = mesh(geometry_builder::create_plane(100, 100, true));
  mirror.get_transform().translate(vec3(0.0f, 10.0f, 30.0f));
  mirror.get_transform().scale = vec3(0.5f, 0.5f, 0.5f);
  mirror.get_transform().rotate(vec3(half_pi<float>(), pi<float>(), 0.0f));

  // Create scene
  meshes["box"] = mesh(geometry_builder::create_box());
  meshes["pyramid"] = mesh(geometry_builder::create_pyramid());
  meshes["torus"] = mesh(geometry_builder::create_torus(20, 20, 1.0f, 5.0f));
  meshes["box"].get_transform().scale = vec3(5.0f, 5.0f, 5.0f);
  meshes["box"].get_transform().translate(vec3(-10.0f, 2.5f, -30.0f));
  meshes["pyramid"].get_transform().scale = vec3(8.0f, 100.0f, 8.0f);
  meshes["pyramid"].get_transform().translate(vec3(0, 50, 0));
  meshes["torus"].get_transform().translate(vec3(-25.0f, 10.0f, -25.0f));
  meshes["torus"].get_transform().rotate(vec3(half_pi<float>(), 0.0f, 0.0f));
  meshes["box"].get_material().set_emissive(vec4(0.0f, 0.0f, 0.0f, 1.0f));
  meshes["box"].get_material().set_diffuse(vec4(1.0f, 0.0f, 0.0f, 1.0f));
  meshes["box"].get_material().set_specular(vec4(1.0f, 1.0f, 1.0f, 1.0f));
  meshes["box"].get_material().set_shininess(25.0f);
  meshes["pyramid"].get_material().set_emissive(vec4(0.0f, 0.0f, 0.0f, 1.0f));
  meshes["pyramid"].get_material().set_diffuse(vec4(0.0f, 0.0f, 1.0f, 1.0f));
  meshes["pyramid"].get_material().set_specular(vec4(1.0f, 1.0f, 1.0f, 1.0f));
  meshes["pyramid"].get_material().set_shininess(25.0f);
  meshes["torus"].get_material().set_emissive(vec4(0.0f, 0.0f, 0.0f, 1.0f));
  meshes["torus"].get_material().set_diffuse(vec4(1.0f, 1.0f, 1.0f, 1.0f));
  meshes["torus"].get_material().set_specular(vec4(1.0f, 1.0f, 1.0f, 1.0f));
  meshes["torus"].get_material().set_shininess(25.0f);

  setupWater();

  goodsand = mesh(geometry_builder::create_disk(10, vec2(1.0, 1.0)));
  goodsand.get_transform().translate(vec3(0, 0.01f, 0));
  goodsand.get_transform().scale = vec3(300.0f, 1.0f, 300.0f);
  goodsand.get_material().set_emissive(vec4(0.0f, 0.0f, 0.0f, 1.0f));
  goodsand.get_material().set_diffuse(vec4(1.0f, 1.0f, 1.0f, 1.0f));
  goodsand.get_material().set_specular(vec4(0.0f, 0.0f, 0.0f, 1.0f));
  goodsand.get_material().set_shininess(1000.0f);

  // textures
  checkedTexture = texture("textures\\checked.gif");
  sandTexture = texture("textures\\sand_512_1.png");
  goodsandTexture = texture("textures\\Goodsand_1024.jpg");
  goodsandTextureBump = texture("textures\\Goodsand_bump_1024.jpg");

  // shaders
  phongEffect.add_shader("shaders\\phong.vert", GL_VERTEX_SHADER);
  phongEffect.add_shader("shaders\\phong.frag", GL_FRAGMENT_SHADER);
  phongEffect.build();

  texturedBumpEffect.add_shader("shaders\\textured_bump.vert",
                                GL_VERTEX_SHADER);
  texturedBumpEffect.add_shader("shaders\\textured_bump.frag",
                                GL_FRAGMENT_SHADER);
  texturedBumpEffect.build();
  //bind shader to light ssbo
  GLuint block_index = 0;
  block_index = glGetProgramResourceIndex(texturedBumpEffect.get_program(), GL_SHADER_STORAGE_BLOCK, "ssbo_directional_lights");
  glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, ssbo);


  simpleEffect.add_shader("shaders\\basic.vert", GL_VERTEX_SHADER);
  simpleEffect.add_shader("shaders\\basic.frag", GL_FRAGMENT_SHADER);
  simpleEffect.build();

  gouradEffect.add_shader("shaders\\gouraud.vert", GL_VERTEX_SHADER);
  gouradEffect.add_shader("shaders\\gouraud.frag", GL_FRAGMENT_SHADER);
  gouradEffect.build();

  texturedEffect.add_shader("shaders\\simple_texture.vert", GL_VERTEX_SHADER);
  texturedEffect.add_shader("shaders\\simple_texture.frag", GL_FRAGMENT_SHADER);
  texturedEffect.build();

  skyeffect.add_shader("shaders\\sky.vert", GL_VERTEX_SHADER);
  skyeffect.add_shader("shaders\\sky.frag", GL_FRAGMENT_SHADER);
  skyeffect.build();

  // Set camera properties
  cam.set_position(vec3(0.0f, 10.0f, 0.0f));
  cam.set_target(vec3(0.0f, 0.0f, 0.0f));
  aspect = static_cast<float>(renderer::get_screen_width()) /
           static_cast<float>(renderer::get_screen_height());
  cam.set_projection(quarter_pi<float>(), aspect, 2.414f, 2000.0f);
  activeCam = &cam;

  // build
  DesertGen::CreateDesert();
  desertM = &DesertGen::farMesh;
  desertM->get_material().set_emissive(vec4(0.0f, 0.0f, 0.0f, 1.0f));
  desertM->get_material().set_diffuse(vec4(1.0f, 1.0f, 1.0f, 1.0f));
  desertM->get_material().set_specular(vec4(0.0f, 0.0f, 0.0f, 1.0f));
  desertM->get_material().set_shininess(1000.0f);

  return true;
}

float dayscale;
bool graphics::update(float delta_time) {
  counter += (delta_time * 0.16f);
  // mirror.get_transform().rotate(vec3(delta_time*-0.2f, 0, 0.0f));

  float s = sinf(counter);
  float c = cosf(counter);
  if (counter > pi<float>()) {
    counter = 0;
  }

  // counter		0			pi/2			 pi
  //            midday		midnight		midday
  // dayscale	  1.0			0				1.0
  dayscale = fabs(counter - half_pi<float>()) / half_pi<float>();
  // printf("%f\n", dayscale);

  vec3 rot = glm::rotateZ(vec3(1.0f, 1.0f, -1.0f), counter);
  light.set_direction(glm::rotateZ(rot, counter));

  if (dayscale < 0.5) {
    light.set_light_colour(mix(vec4(0, 0, 0, 1.0f),
                               vec4(0.8f, 0.8f, 0.8f, 1.0f), dayscale / 0.5f));
  } else {
    light.set_light_colour(vec4(0.8f, 0.8f, 0.8f, 1.0f));
  }

  // The ratio of pixels to rotation - remember the fov
  static double ratio_width =
      quarter_pi<float>() / static_cast<float>(renderer::get_screen_width());
  static double ratio_height =
      (quarter_pi<float>() *
       (static_cast<float>(renderer::get_screen_height()) /
        static_cast<float>(renderer::get_screen_width()))) /
      static_cast<float>(renderer::get_screen_height());

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
  cam.rotate(static_cast<float>(delta_x), static_cast<float>(delta_y));

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

  float moveSpeed = 2.0f;
  if (glfwGetKey(renderer::get_window(), GLFW_KEY_LEFT_SHIFT))
    moveSpeed = 6.0f;

  cam.move(translation * moveSpeed);
  cam.update(delta_time);

  cursor_x = current_x;
  cursor_y = current_y;

  return true;
}

void graphics::rendermesh(mesh &m, texture &t) {
  effect eff = phongEffect;
  // Bind effect
  renderer::bind(eff);
  // Create MVP matrix
  auto M = m.get_transform().get_transform_matrix();
  auto V = activeCam->get_view();
  auto P = activeCam->get_projection();
  auto MVP = P * V * M;
  // Set MVP matrix uniform
  glUniformMatrix4fv(eff.get_uniform_location("MVP"), // Location of uniform
                     1,               // Number of values - 1 mat4
                     GL_FALSE,        // Transpose the matrix?
                     value_ptr(MVP)); // Pointer to matrix data
  // Set M matrix uniform
  glUniformMatrix4fv(eff.get_uniform_location("M"), 1, GL_FALSE, value_ptr(M));
  // Set N matrix uniform
  glUniformMatrix3fv(eff.get_uniform_location("N"), 1, GL_FALSE,
                     value_ptr(m.get_transform().get_normal_matrix()));
  // Bind material
  renderer::bind(m.get_material(), "mat");
  // **********
  // Bind light
  // **********
  renderer::bind(light, "light");
  // Bind texture
  renderer::bind(t, 0);
  // Set tex uniform
  glUniform1i(eff.get_uniform_location("tex"), 0);
  // Set eye position
  glUniform3fv(eff.get_uniform_location("eye_pos"), 1,
               value_ptr(activeCam->get_position()));

  // Render mesh
  renderer::render(m);
}

void graphics::rendermeshB(mesh &m, texture &t, texture &tb,
                           const float scale) {
  effect eff = texturedBumpEffect;
  // Bind effect
  renderer::bind(eff);
  // Create MVP matrix
  auto M = m.get_transform().get_transform_matrix();
  auto V = activeCam->get_view();
  auto P = activeCam->get_projection();
  auto MVP = P * V * M;
  // Set MVP matrix uniform
  glUniformMatrix4fv(eff.get_uniform_location("MVP"), // Location of uniform
                     1,               // Number of values - 1 mat4
                     GL_FALSE,        // Transpose the matrix?
                     value_ptr(MVP)); // Pointer to matrix data
  // Set M matrix uniform
  glUniformMatrix4fv(eff.get_uniform_location("M"), 1, GL_FALSE, value_ptr(M));
  // Set N matrix uniform
  glUniformMatrix3fv(eff.get_uniform_location("N"), 1, GL_FALSE,
                     value_ptr(m.get_transform().get_normal_matrix()));
  // Bind material
  renderer::bind(m.get_material(), "mat");
  // **********
  // Bind light
  // **********
  renderer::bind(light, "light");
  // Bind texture
  renderer::bind(t, 0);
  renderer::bind(tb, 1);
  // Set tex uniform
  glUniform1i(eff.get_uniform_location("tex"), 0);
  glUniform1i(eff.get_uniform_location("normal_map"), 1);
  glUniform1f(eff.get_uniform_location("TextureScale"), scale);
  // Set eye position
  glUniform3fv(eff.get_uniform_location("eye_pos"), 1,
               value_ptr(activeCam->get_position()));

  glUniform3fv(eff.get_uniform_location("light_dir"), 1,
               value_ptr(light.get_direction()));
  // Render mesh
  renderer::render(m);
}

void graphics::processLines() {
  if (linebuffer.size() < 1) {
    return;
  }

  renderer::bind(simpleEffect);
  auto V = activeCam->get_view();
  auto P = activeCam->get_projection();
  auto VP = P * V;
  // Set MVP matrix uniform
  glUniformMatrix4fv(
      simpleEffect.get_uniform_location("MVP"), // Location of uniform
      1,                                        // Number of values - 1 mat4
      GL_FALSE,                                 // Transpose the matrix?
      value_ptr(VP));                           // Pointer to matrix data
  renderer::RenderLines(linebuffer, 1);
  linebuffer.clear();
}

void graphics::renderSky() {
  // vertaical fov = 25.3125deg = 0.441786467 radians
  float verticleFov = 0.2208932335f; // vfov/2 in radians

  vec3 camview = normalize(activeCam->get_target() - activeCam->get_position());
  vec3 camUp = normalize(activeCam->get_up());

  float r = atanf(verticleFov) * camview.length();

  vec3 topofscreentoplayer = normalize((r * camUp) + camview);
  vec3 bottomofscreentoplayer = normalize((-r * camUp) + camview);

  float topDot = dot(topofscreentoplayer, vec3(0, 1.0, 0));
  float bottomDot = dot(bottomofscreentoplayer, vec3(0, 1.0, 0));

  renderer::bind(skyeffect);
  float p = pi<float>();

  vec3 bottomcol;
  if (dayscale < 0.6f) {
    bottomcol = mix(vec3(0.94, 0.427, 0.117), vec3(0.73, 0.796, 0.99),
                    dayscale / 0.6f) *
                dayscale;
  } else {
    bottomcol = vec3(0.73, 0.796, 0.99) * dayscale;
  }
  vec3 topcol = vec3(0.067, 0.129, 0.698) * dayscale;
  // vec3 bottomcol = vec3(0.73, 0.796, 0.99) * dayscale;

  glUniform3fv(skyeffect.get_uniform_location("topcol"), 1, &topcol[0]);
  glUniform3fv(skyeffect.get_uniform_location("bottomcol"), 1, &bottomcol[0]);
  glUniform1f(skyeffect.get_uniform_location("topdot"), topDot);
  glUniform1f(skyeffect.get_uniform_location("bottomdot"), bottomDot);
  glUniform3f(skyeffect.get_uniform_location("playerview"), camview.x,
              camview.y, camview.z);

  graphics_framework::geometry geo;
  std::vector<vec2> v = {vec2(-1, -1), vec2(-1, 1), vec2(1, 1),
                         vec2(1, 1),   vec2(1, -1), vec2(-1, -1)};
  geo.add_buffer(v, 0);
  glDisable(GL_CULL_FACE);
  renderer::render(geo);
  glEnable(GL_CULL_FACE);
}

bool graphics::render() {
  glEnable(GL_FOG); // enable the fog
  GLfloat density = 0.3;
  GLfloat fogColor[4] = {0.5, 0.5, 0.5, 1.0};
  glFogi(GL_FOG_MODE, GL_EXP2); // set the fog mode to GL_EXP2
  glFogfv(GL_FOG_COLOR, fogColor);
  glFogf(GL_FOG_DENSITY, density);
  glHint(GL_FOG_HINT, GL_NICEST);

  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  // Render meshes
  for (auto &e : meshes) {
    rendermesh(e.second, checkedTexture);
  }

  rendermesh(*desertM, sandTexture);

  rendermeshB(goodsand, goodsandTexture, goodsandTextureBump, 10.0f);

  renderSky();

  renderWater(mirror);

  DrawCross(vec3(0.0, 0.0, 0.0f), 10.0f);

  processLines();
  return true;
}

graphics::graphics() {}

graphics::~graphics() {}

graphics *gfx = nullptr;

void main() {
  // Create application
  app application(1280, 720, 0);
  gfx = new graphics();
  // Set load content, update and render methods
  application.set_load_content(std::bind(&graphics::load_content, gfx));
  application.set_initialise(std::bind(&graphics::initialise, gfx));
  application.set_update(
      std::bind(&graphics::update, gfx, std::placeholders::_1));
  application.set_render(std::bind(&graphics::render, gfx));

  // Run application
  application.run();
}