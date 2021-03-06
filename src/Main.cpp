﻿/* Main.cpp
* Program entry point,
*  holds loader and the main update and render funcs.
* Starts up the graphics Api.
*
* Sam Serrels, Computer Graphics, 2015
*/
#include "main.h"
#include "Particles.h"
#include "libENUgraphics\graphics_framework.h"
#include <glm\glm.hpp>
#include <chrono>
#include <glm\gtx\rotate_vector.hpp>
#include <functional>
#include "DesertGen.h"
#include "Math.h"
#include "mirror.h"
#include "UI.h"
#include "Enviroment.h"
#include "Gimbal.h"
#include "Rendering.h"

using namespace std;
using namespace graphics_framework;
using namespace glm;

// Init the global extern to the graphics instance
Graphics *gfx = nullptr;
bool showUI;

void Graphics::DrawLine(const glm::vec3 &p1, const glm::vec3 &p2) {
  linebuffer.push_back(p1);
  linebuffer.push_back(p2);
}

void Graphics::DrawCross(const glm::vec3 &p1, const float size) {
  DrawLine(p1 + glm::vec3(size, 0, 0), p1 - glm::vec3(size, 0, 0));
  DrawLine(p1 + glm::vec3(0, size, 0), p1 - glm::vec3(0, size, 0));
  DrawLine(p1 + glm::vec3(0, 0, size), p1 - glm::vec3(0, 0, size));
}

bool Graphics::Initialise() {
  // counter = quarter_pi<float>() * 3.0f;
  // Set input mode - hide the cursor
  glfwSetInputMode(renderer::get_window(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);

  // Capture initial mouse position
  glfwGetCursorPos(renderer::get_window(), &cursor_x, &cursor_y);
  showUI = true;
  initUI();
  return true;
}

// Send all light data to SSBOs on the GPU

Gimbal *gimbal;
vec2 lposes[LIGHTS];
default_random_engine drand(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count());
uniform_real_distribution<float> dist;

float crand(){
  return (float)rand() / (float)RAND_MAX;
}

bool Graphics::Load_content() {
  // Lights
 // dlight.set_ambient_intensity(vec4(0.3f, 0.3f, 0.3f, 1.0f));
  dlight.set_ambient_intensity(vec4(0.0f, 0.0f, 0.0f, 1.0f));
  dlight.set_light_colour(vec4(0.8f, 0.8f, 0.8f, 1.0f));
  dlight.set_direction(vec3(1.0f, 1.0f, 1.0f));
  DLights.push_back(&dlight);

  for (unsigned int i = 0; i < LIGHTS; i++)
  {
    point_light* p = new point_light();
    float fi = (((float)i) / ((float)LIGHTS)) * 2.0f* pi<float>();
    p->set_light_colour(vec4(0.5f + (cosf(fi)*0.5f), 0.5f + (-sinf(fi)*0.5f), 0.5f + (sinf(fi)*0.5f), 1.0f));
    p->set_range(40.0f);
    p->set_position(vec3((crand()* 400.0f) - 200.0f, 2.0f, (crand()* 400.0f) - 200.0f));
    PLights.push_back(p);
    lposes[i] = vec2(((crand()* 400.0f) - 200.0f, (crand()* 400.0f) - 200.0f));
  }

  slight.set_position(vec3(-25.0f, 10.0f, -15.0f));
  slight.set_light_colour(vec4(0.0f, 1.0f, 0.0f, 1.0f));
  slight.set_direction(normalize(vec3(1.0f, -1.0f, -1.0f)));
  slight.set_range(20.0f);
  slight.set_power(0.5f);
  SLights.push_back(&slight);

  mirror = mesh(geometry_builder::create_plane(100, 100, true));
  mirror.get_transform().translate(vec3(0.0f, 30.0f, 50.0f));
  mirror.get_transform().scale = vec3(0.5f, 0.5f, 0.5f);
  mirror.get_transform().rotate(vec3(half_pi<float>(), pi<float>(), 0.0f));

  // Create scene
  meshes["box"] = mesh(geometry_builder::create_box());
  meshes["box2"] = mesh(geometry_builder::create_box());
  meshes["box3"] = mesh(geometry_builder::create_box());
  meshes["pyramid"] = mesh(geometry_builder::create_pyramid());
  meshes["box"].get_transform().scale = vec3(5.0f, 5.0f, 5.0f);
  meshes["box"].get_transform().translate(vec3(0, 5.0f, 37.0f));
  meshes["box2"].get_transform().scale = vec3(5.0f, 5.0f, 5.0f);
  meshes["box2"].get_transform().translate(vec3(-20, 5.0f, -37.0f));
  meshes["box3"].get_transform().scale = vec3(5.0f, 5.0f, 5.0f);
  meshes["box3"].get_transform().translate(vec3(20, 5.0f, -37.0f));
  meshes["pyramid"].get_transform().scale = vec3(8.0f, 100.0f, 8.0f);
  meshes["pyramid"].get_transform().translate(vec3(0, 50, 0));
  meshes["box"].get_material().set_diffuse(vec4(1.0f, 0.0f, 0.0f, 1.0f));
  meshes["box"].get_material().set_shininess(25.0f);
  meshes["box2"].get_material().set_diffuse(vec4(0.0f, 1.0f, 0.0f, 1.0f));
  meshes["box2"].get_material().set_shininess(25.0f);
  meshes["box3"].get_material().set_emissive(vec4(0.0f, 0.0f, 1.0f, 1.0f));
  meshes["box3"].get_material().set_diffuse(vec4(1.0f, 1.0f, 0.0f, 1.0f));
  meshes["box3"].get_material().set_shininess(25.0f);
  meshes["pyramid"].get_material().set_diffuse(vec4(0.0f, 0.0f, 1.0f, 1.0f));
  meshes["pyramid"].get_material().set_shininess(25.0f);

  goodsand = mesh(geometry_builder::create_disk(10, vec2(1.0, 1.0)));
  goodsand.get_transform().translate(vec3(0, 0.01f, 0));
  goodsand.get_transform().scale = vec3(300.0f, 1.0f, 300.0f);
  goodsand.get_material().set_diffuse(vec4(1.0f, 1.0f, 1.0f, 1.0f));
  goodsand.get_material().set_specular(vec4(0.0f, 0.0f, 0.0f, 1.0f));
  goodsand.get_material().set_shininess(1000.0f);

  // textures
  checkedTexture = texture("textures\\checked.gif");
  sandTexture = texture("textures\\sand_512_1.png");
  goodsandTexture = texture("textures\\Goodsand_1024.jpg");
  goodsandTextureBump = texture("textures\\Goodsand_bump_1024.jpg");
  noise16 = texture("textures\\tex16.png");
  // shaders
  phongEffect.add_shader("shaders\\phong.vert", GL_VERTEX_SHADER);
  phongEffect.add_shader("shaders\\phong.frag", GL_FRAGMENT_SHADER);
  phongEffect.build();

  texturedBumpEffect.add_shader("shaders\\textured_bump.vert", GL_VERTEX_SHADER);
  texturedBumpEffect.add_shader("shaders\\textured_bump.frag", GL_FRAGMENT_SHADER);
  texturedBumpEffect.build();
  // bind shader to light ssbo

  simpleEffect.add_shader("shaders\\basic.vert", GL_VERTEX_SHADER);
  simpleEffect.add_shader("shaders\\basic.frag", GL_FRAGMENT_SHADER);
  simpleEffect.build();

  gouradEffect.add_shader("shaders\\gouraud.vert", GL_VERTEX_SHADER);
  gouradEffect.add_shader("shaders\\gouraud.frag", GL_FRAGMENT_SHADER);
  gouradEffect.build();

  texturedEffect.add_shader("shaders\\simple_texture.vert", GL_VERTEX_SHADER);
  texturedEffect.add_shader("shaders\\simple_texture.frag", GL_FRAGMENT_SHADER);
  texturedEffect.build();

  nullEffect.add_shader("shaders\\null.vert", GL_VERTEX_SHADER);
  nullEffect.add_shader("shaders\\null.frag", GL_FRAGMENT_SHADER);
  nullEffect.build();

  sphereMesh = mesh(geometry_builder::create_sphere(16, 16, vec3(1.0f, 1.0f, 1.0f)));
  std::vector<vec2> v = {vec2(-1, -1), vec2(-1, 1), vec2(1, 1), vec2(1, 1), vec2(1, -1), vec2(-1, -1)};
  planegeo.add_buffer(v, 0);
  // Set camera properties
  cam.set_position(vec3(0.0f, 10.0f, 0.0f));
  cam.set_target(vec3(0.0f, 0.0f, 0.0f));
  aspect = static_cast<float>(renderer::get_screen_width()) / static_cast<float>(renderer::get_screen_height());
  cam.set_projection(quarter_pi<float>(), aspect, 2.414f, 25000.0f);
  activeCam = &cam;

  // build
  DesertGen::CreateDesert();
  desertM = &DesertGen::farMesh;
  desertM->get_material().set_emissive(vec4(0.0f, 0.0f, 0.0f, 1.0f));
  desertM->get_material().set_diffuse(vec4(1.0f, 1.0f, 1.0f, 1.0f));
  desertM->get_material().set_specular(vec4(0.0f, 0.0f, 0.0f, 1.0f));
  desertM->get_material().set_shininess(1000.0f);

  Enviroment::Load();
  gimbal = new Gimbal(16);
  SetupMirror();

  SetMode(FORWARD, NORMAL);
  EnableSSBO(false);
  InitParticles();
  return true;
}

bool Graphics::Update(float delta_time) {
  // torus heirarchy
  realtime += delta_time;
  counter += (delta_time * 0.08f);
  //  mirror.get_transform().rotate(vec3(delta_time*-0.2f, 0, 0.0f));
  gimbal->Update(delta_time);
  Enviroment::Update(delta_time);
  meshes["box2"].get_transform().rotate(vec3(delta_time * -0.2f, 0, 0.0f));
  meshes["box3"].get_transform().rotate(vec3(0, 0, delta_time * -0.2f));

  float s = sinf(counter);
  float c = cosf(counter);
  if (counter > pi<float>()) {
    counter = 0;
  }

  for (unsigned int i = 0; i < PLights.size(); i++){
    unsigned int j = i+1;
    if (j == PLights.size()){j = 0;}
    const vec3 target = vec3(lposes[i].x, 2.0f, lposes[i].y);
    const vec3 dir = target - PLights[i]->get_position();
    if (glm::length2(dir) < 25.0f){
      lposes[i] = vec2((crand()* 400.0f) - 200.0f, (crand()* 400.0f) - 200.0f);
    }
    PLights[i]->set_position(normalize(dir)*10.0f* delta_time + PLights[i]->get_position());
  }

  { // Camera update
    static double ratio_width = quarter_pi<float>() / static_cast<float>(renderer::get_screen_width());
    static double ratio_height = (quarter_pi<float>() * (static_cast<float>(renderer::get_screen_height()) /
                                                         static_cast<float>(renderer::get_screen_width()))) /
                                 static_cast<float>(renderer::get_screen_height());

    double current_x;
    double current_y;
    // Get the current cursor position
    glfwGetCursorPos(renderer::get_window(), &current_x, &current_y);
    if (glfwGetInputMode(renderer::get_window(), GLFW_CURSOR) == GLFW_CURSOR_DISABLED) {
      // Calculate delta of cursor positions from last frame
      double delta_x = current_x - cursor_x;
      double delta_y = current_y - cursor_y;
      // Multiply deltas by ratios - gets actual change in orientation
      delta_x *= ratio_width;
      delta_y *= -ratio_height;

      // Rotate cameras by delta
      // delta_y - x-axis rotation
      // delta_x - y-axis rotation
      cam.rotate(static_cast<float>(delta_x), static_cast<float>(delta_y));
      cursor_x = current_x;
      cursor_y = current_y;
    } else {
      cursor_x = current_x;
      cursor_y = current_y;
    }

    // Use keyboard to move the camera
    vec3 translation(0.0f, 0.0f, 0.0f);
    if (glfwGetKey(renderer::get_window(), 'W'))
      translation.z += 5.0f * delta_time;
    if (glfwGetKey(renderer::get_window(), 'S'))
      translation.z -= 5.0f * delta_time;
    if (glfwGetKey(renderer::get_window(), 'A'))
      translation.x -= 5.0f * delta_time;
    if (glfwGetKey(renderer::get_window(), 'D'))
      translation.x += 5.0f * delta_time;

    if (glfwGetKey(renderer::get_window(), 'U')) {
      FreezeMirror(true);
    }
    if (glfwGetKey(renderer::get_window(), 'I')) {
      FreezeMirror(false);
    }

    float moveSpeed = 2.0f;
    if (glfwGetKey(renderer::get_window(), GLFW_KEY_LEFT_SHIFT))
      moveSpeed = 6.0f;

    cam.move(translation * moveSpeed);
    cam.update(delta_time);
  }
  static int keystate;
  int newkeystate = glfwGetKey(renderer::get_window(), 'N');
  if (newkeystate && newkeystate != keystate) {
    showUI = !showUI;
  }
  keystate = newkeystate;

  if (showUI) {
    UpdateUI();
  }
  UpdateLights();

  //  UpdateParticles(delta_time);
  return true;
}

void Graphics::Rendermesh(mesh &m, texture &t) {

  effect &eff = BasicEffect();
  // Bind effect
  renderer::bind(eff);
  // Create MVP matrix
  auto M = m.get_transform().get_transform_matrix();
  auto V = activeCam->get_view();
  auto P = activeCam->get_projection();
  auto MVP = P * V * M;
  // Set MVP matrix uniform
  glUniformMatrix4fv(eff.get_uniform_location("MVP"), 1, GL_FALSE, value_ptr(MVP));
  // Set M matrix uniform
  glUniformMatrix4fv(eff.get_uniform_location("M"), 1, GL_FALSE, value_ptr(M));
  // Set N matrix uniform
  glUniformMatrix3fv(eff.get_uniform_location("N"), 1, GL_FALSE, value_ptr(m.get_transform().get_normal_matrix()));
  GLint pos = eff.get_uniform_location("TextureScale");
  if (pos != -1) {
    glUniform1f(pos, 1.0f);
  }
  // Set eye position
  pos = eff.get_uniform_location("eye_pos");
  if (pos != -1) {
    glUniform3fv(pos, 1, value_ptr(activeCam->get_position()));
  }
  pos = eff.get_uniform_location("sunnyD");
  if (pos != -1) {
    glUniform3fv(pos, 1, value_ptr(dlight.get_direction()));
  }

  // Bind material
  renderer::bind(m.get_material(), "mat");
  // Bind texture
  renderer::bind(t, 0);
  // Set tex uniform
  glUniform1i(eff.get_uniform_location("tex"), 0);
  // Render mesh
  renderer::render(m);
}

// Renders a mesh with a bump map
void Graphics::RendermeshB(mesh &m, const texture &t, const texture &tb, const float scale) {
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
                     1,                               // Number of values - 1 mat4
                     GL_FALSE,                        // Transpose the matrix?
                     value_ptr(MVP));                 // Pointer to matrix data
  // Set M matrix uniform
  glUniformMatrix4fv(eff.get_uniform_location("M"), 1, GL_FALSE, value_ptr(M));
  // Set N matrix uniform
  glUniformMatrix3fv(eff.get_uniform_location("N"), 1, GL_FALSE, value_ptr(m.get_transform().get_normal_matrix()));
  // Bind material
  renderer::bind(m.get_material(), "mat");
  // **********
  // Bind light
  // **********
  renderer::bind(dlight, "light");
  // Bind texture
  renderer::bind(t, 0);
  renderer::bind(tb, 1);
  // Set tex uniform
  glUniform1i(eff.get_uniform_location("tex"), 0);
  glUniform1i(eff.get_uniform_location("normal_map"), 1);
  glUniform1f(eff.get_uniform_location("TextureScale"), scale);
  // Set eye position
  glUniform3fv(eff.get_uniform_location("eye_pos"), 1, value_ptr(activeCam->get_position()));

  glUniform3fv(eff.get_uniform_location("light_dir"), 1, value_ptr(dlight.get_direction()));
  // Render mesh
  renderer::render(m);
}

void Graphics::ProcessLines() {
  if (linebuffer.size() < 1) {
    return;
  }

  renderer::bind(simpleEffect);
  auto V = activeCam->get_view();
  auto P = activeCam->get_projection();
  auto VP = P * V;
  // Set MVP matrix uniform
  glUniformMatrix4fv(simpleEffect.get_uniform_location("MVP"), // Location of uniform
                     1,                                        // Number of values - 1 mat4
                     GL_FALSE,                                 // Transpose the matrix?
                     value_ptr(VP));                           // Pointer to matrix data
  renderer::RenderLines(linebuffer, 1);
  linebuffer.clear();
}

void Graphics::DrawScene() {
  for (auto &e : meshes) {
    Rendermesh(e.second, checkedTexture);
  }
  Rendermesh(*desertM, sandTexture);

  // RendermeshB(goodsand, goodsandTexture, goodsandTextureBump, 10.0f);
  Enviroment::RenderSky();
  gimbal->Render();
  DrawCross(vec3(0.0, 0.0, 0.0f), 10.0f);
}

bool Graphics::Render() {
  NewFrame();

  BeginOpaque();
  glCullFace(GL_BACK);
  DrawScene();
  RenderMirror(mirror);
  EndOpaque();

  BeginTransparent();
  EndTransparent();

  BeginPost();
  EndPost();

  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  ProcessLines();
  // RenderParticles();
  if (showUI) {
    DrawUI();
  }
  return true;
}

Graphics::Graphics() {}

Graphics::~Graphics() {
  for (auto p : PLights)
  {
    delete(p);
  }
}

void main() {
  // Create application
  app application(SCREENWIDTH, SCREENHEIGHT, 0);
  gfx = new Graphics();
  // Set load content, update and render methods
  application.set_load_content(std::bind(&Graphics::Load_content, gfx));
  application.set_initialise(std::bind(&Graphics::Initialise, gfx));
  application.set_update(std::bind(&Graphics::Update, gfx, std::placeholders::_1));
  application.set_render(std::bind(&Graphics::Render, gfx));

  // Run application
  application.run();


  delete gimbal;
  delete gfx;
  gfx = nullptr;
  ShutDownUI();
}