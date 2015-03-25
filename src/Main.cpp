/* Main.cpp
* Program entry point,
*  holds loader and the main update and render funcs.
* Starts up the graphics Api.
*
* Sam Serrels, Computer Graphics, 2015
*/
#include "main.h"
#include "libENUgraphics\graphics_framework.h"
#include <glm\glm.hpp>
#include <glm\gtx\rotate_vector.hpp>
#include "DesertGen.h"
#include "Math.h"
#include "mirror.h"
#include <functional>

using namespace std;
using namespace graphics_framework;
using namespace glm;

// Init the global extern to the graphics instance
Graphics *gfx = nullptr;

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
  // Set input mode - hide the cursor
  glfwSetInputMode(renderer::get_window(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);

  // Capture initial mouse position
  glfwGetCursorPos(renderer::get_window(), &cursor_x, &cursor_y);

  return true;
}

// Send all light data to SSBOs on the GPU
void Graphics::UpdateLights() {
  // Directional lights
  {
    struct S_Dlight {
      vec4 ambient_intensity;
      vec4 light_colour;
      vec4 light_dir;
    };
    vector<S_Dlight> S_DLights;
    for (auto L : DLights) {
      S_Dlight sd;
      sd.ambient_intensity = L.get_ambient_intensity();
      sd.light_colour = L.get_light_colour();
      sd.light_dir = vec4(L.get_direction(), 0);
      S_DLights.push_back(sd);
    }
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, dLightSSBO);
    // this might not be the best way to copy data, but it works.
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(S_Dlight) * S_DLights.size(), &S_DLights[0],
                 GL_DYNAMIC_COPY);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
  }
  //  point lights
  {
    struct S_Plight {
      vec4 light_colour;
      vec4 position; // position is a vec3, padded in a vec4
      vec4 falloff;  //(constant, linear, quadratic, NULL)
    };
    vector<S_Plight> S_PLights;
    for (auto L : PLights) {
      S_Plight sd;
      sd.light_colour = L.get_light_colour();
      sd.position = vec4(L.get_position(), 0);
      sd.falloff = vec4(0, 0, 0, 0);
      sd.falloff.x = L.get_constant_attenuation();
      sd.falloff.y = L.get_linear_attenuation();
      sd.falloff.z = L.get_quadratic_attenuation();
      S_PLights.push_back(sd);
    }
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, pLightSSBO);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(S_Plight) * S_PLights.size(), &S_PLights[0],
                 GL_DYNAMIC_COPY);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
  }
  // spot lights
  {
    struct S_Slight {
      vec4 light_colour;
      vec4 position;  // position is a vec3, padded in a vec4
      vec4 direction; // direction is a vec3, padded in a vec4
      vec4 falloff;   //(constant, linear, quadratic, power)
    };
    vector<S_Slight> S_SLights;
    for (auto L : SLights) {
      S_Slight sd;
      sd.light_colour = L.get_light_colour();
      sd.position = vec4(L.get_position(), 0);
      sd.direction = vec4(L.get_direction(), 0);
      sd.falloff = vec4(0, 0, 0, 0);
      sd.falloff.x = L.get_constant_attenuation();
      sd.falloff.y = L.get_linear_attenuation();
      sd.falloff.z = L.get_quadratic_attenuation();
      sd.falloff.w = L.get_power();
      S_SLights.push_back(sd);
    }
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, sLightSSBO);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(S_Slight) * S_SLights.size(), &S_SLights[0],
                 GL_DYNAMIC_COPY);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
  }

  glBindBuffer(GL_SHADER_STORAGE_BUFFER, LightSSBO);
  glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(NumberOfLights), &NumberOfLights, GL_DYNAMIC_COPY);
  glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
}

#define rings 16
void Graphics::MakeGyroscope() {
  meshes["torus0"] = mesh(geometry_builder::create_torus(32, 32, 0.5f, rings));
  meshes["torus0"].get_transform().translate(vec3(10.0f, rings + 2.0f, -30.0f));
  for (unsigned int i = 1; i < rings; i++) {
    meshes["torus" + std::to_string(i)] =
        mesh(geometry_builder::create_torus(20, 20, 0.5f, rings - i));
    meshes["torus" + std::to_string(i)].get_transform().parent =
        &meshes["torus" + std::to_string(i - 1)].get_transform();
    meshes["torus" + std::to_string(i)].get_material().set_emissive(vec4(0.2f, 0.2f, 0.2f, 1.0f));
    meshes["torus" + std::to_string(i)].get_material().set_diffuse(
        vec4(((i - 3) % 9) / 9.0f, (i % 9) / 9.0f, ((i - 6) % 9) / 9.0f, 1.0f));
    meshes["torus" + std::to_string(i)].get_material().set_specular(vec4(1.0f, 1.0f, 1.0f, 1.0f));
  }
}

void Graphics::UpdateGyroscope(float delta_time) {
  for (unsigned int i = 0; i < rings; i++) {
    vec3 rot;
    switch (i % 2) {
    case (0):
      rot = vec3(0, 0, 1);
      break;
    case (1):
      rot = vec3(-1, 0, 0);
      break;
    case (2):
      rot = vec3(0, 1, 0);
      break;
    case (3):
      rot = vec3(0, 0, -1);
      break;
    case (4):
      rot = vec3(1, 0, 0);
      break;
    case (5):
      rot = vec3(0, -1, 0);
      break;
    }
    meshes["torus" + std::to_string(i)].get_transform().rotate((delta_time)*0.6f * rot);
  }
}

bool Graphics::Load_content() {
  // Lights
  dlight.set_ambient_intensity(vec4(0.3f, 0.3f, 0.3f, 1.0f));
  dlight.set_light_colour(vec4(0.8f, 0.8f, 0.8f, 1.0f));
  dlight.set_direction(vec3(1.0f, 1.0f, -1.0f));

  DLights.push_back(dlight);
  plight.set_light_colour(vec4(0.8f, 0.6f, 1.0f, 1.0f));
  plight.set_range(40.0f);
  plight.set_position(vec3(30.0f, 15.0f, 30.0f));
  PLights.push_back(plight);

  slight.set_position(vec3(-25.0f, 10.0f, -15.0f));
  slight.set_light_colour(vec4(0.0f, 1.0f, 0.0f, 1.0f));
  slight.set_direction(normalize(vec3(1.0f, -1.0f, -1.0f)));
  slight.set_range(20.0f);
  slight.set_power(0.5f);
  SLights.push_back(slight);

  NumberOfLights = vec4(1, 1, 1, 0);
  // directional light SSBO
  {
    assert(GL_SHADER_STORAGE_BUFFER);
    assert(GL_ARB_shader_storage_buffer_object);
    assert(GL_ARB_uniform_buffer_object);
    glGenBuffers(1, &dLightSSBO);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, dLightSSBO);
    glGenBuffers(1, &pLightSSBO);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, pLightSSBO);
    glGenBuffers(1, &sLightSSBO);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, sLightSSBO);
    glGenBuffers(1, &LightSSBO);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, LightSSBO);
    UpdateLights();
  }

  mirror = mesh(geometry_builder::create_plane(100, 100, true));
  mirror.get_transform().translate(vec3(0.0f, 10.0f, 30.0f));
  mirror.get_transform().scale = vec3(0.5f, 0.5f, 0.5f);
  mirror.get_transform().rotate(vec3(half_pi<float>(), pi<float>(), 0.0f));

  // Create scene
  meshes["box"] = mesh(geometry_builder::create_box());
  meshes["pyramid"] = mesh(geometry_builder::create_pyramid());
  // meshes["torus2"].get_transform().translate(vec3(10.0f, 6.0f, -30.0f));
  // meshes["torus3"].get_transform().translate(vec3(10.0f, 6.0f, -30.0f));
  meshes["box"].get_transform().scale = vec3(5.0f, 5.0f, 5.0f);
  meshes["box"].get_transform().translate(vec3(-10.0f, 2.5f, -30.0f));
  meshes["pyramid"].get_transform().scale = vec3(8.0f, 100.0f, 8.0f);
  meshes["pyramid"].get_transform().translate(vec3(0, 50, 0));
  meshes["box"].get_material().set_emissive(vec4(0.0f, 0.0f, 0.0f, 1.0f));
  meshes["box"].get_material().set_diffuse(vec4(1.0f, 0.0f, 0.0f, 1.0f));
  meshes["box"].get_material().set_specular(vec4(1.0f, 1.0f, 1.0f, 1.0f));
  meshes["box"].get_material().set_shininess(25.0f);
  meshes["pyramid"].get_material().set_emissive(vec4(0.0f, 0.0f, 0.0f, 1.0f));
  meshes["pyramid"].get_material().set_diffuse(vec4(0.0f, 0.0f, 1.0f, 1.0f));
  meshes["pyramid"].get_material().set_specular(vec4(1.0f, 1.0f, 1.0f, 1.0f));
  meshes["pyramid"].get_material().set_shininess(25.0f);
  MakeGyroscope();
  SetupMirror();

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

  skyeffect.add_shader("shaders\\sky.vert", GL_VERTEX_SHADER);
  skyeffect.add_shader("shaders\\sky.frag", GL_FRAGMENT_SHADER);
  skyeffect.build();

  skyeffect2.add_shader("shaders\\sky2.vert", GL_VERTEX_SHADER);
  skyeffect2.add_shader("shaders\\sky2.frag", GL_FRAGMENT_SHADER);
  skyeffect2.build();

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

  //std::vector<vec2> v = { vec2(-1, -1), vec2(-1, 1), vec2(1, 1), vec2(1, 1), vec2(1, -1), vec2(-1, -1) };
  skygeo = mesh(geometry_builder::create_sphere(32, 15, vec3(900.0f, 900.0f, 900.0f)));
  return true;
}
float realtime;
bool Graphics::Update(float delta_time) {
  // torus heirarchy
  realtime += delta_time;
  counter += (delta_time * 0.09f);
  // mirror.get_transform().rotate(vec3(delta_time*-0.2f, 0, 0.0f));
  UpdateGyroscope(delta_time);
  float s = sinf(counter);
  float c = cosf(counter);
  if (counter > pi<float>()) {
    counter = 0;
  }

  // counter		0			          pi/2			        pi
  //            midday   Dusk   midnight  Dawn   Midday
  // dayscale	  1.0			 0.5    0				   0.5   1.0
  // sunscale   1.0      0      0          0     1.0
  // daymode    0        0      0          1     1
  const float oldd = dayscale;
  dayscale = fabs(counter - half_pi<float>()) / half_pi<float>();
  daymode = (dayscale > oldd);
  sunscale = (dayscale - 0.5f) / 0.5f;
  
  printf("%f\t\t%f\t\t%d\n", dayscale, sunscale, daymode);

  vec3 rot = glm::rotateZ(vec3(1.0f, 1.0f, -1.0f), counter);
  dlight.set_direction(glm::rotateZ(rot, counter));

  if (true) {
    dlight.set_light_colour(
      mix(vec4(0, 0, 0, 1.0f), vec4(0.8f, 0.8f, 0.8f, 1.0f), sunscale + 0.2f));
  } else {
    dlight.set_light_colour(vec4(0.8f, 0.8f, 0.8f, 1.0f));
  }
  UpdateLights();
  // The ratio of pixels to rotation - remember the fov
  static double ratio_width =
      quarter_pi<float>() / static_cast<float>(renderer::get_screen_width());
  static double ratio_height =
      (quarter_pi<float>() * (static_cast<float>(renderer::get_screen_height()) /
                              static_cast<float>(renderer::get_screen_width()))) /
      static_cast<float>(renderer::get_screen_height());

  double current_x;
  double current_y;
  // Get the current cursor position
  glfwGetCursorPos(renderer::get_window(), &current_x, &current_y);
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

  float moveSpeed = 2.0f;
  if (glfwGetKey(renderer::get_window(), GLFW_KEY_LEFT_SHIFT))
    moveSpeed = 6.0f;

  cam.move(translation * moveSpeed);
  cam.update(delta_time);

  cursor_x = current_x;
  cursor_y = current_y;

  return true;
}

void Graphics::Rendermesh(mesh &m, texture &t) {
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
                     1,                               // Number of values - 1 mat4
                     GL_FALSE,                        // Transpose the matrix?
                     value_ptr(MVP));                 // Pointer to matrix data
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
  renderer::bind(dlight, "light");
  // Bind texture
  renderer::bind(t, 0);
  // Set tex uniform
  glUniform1i(eff.get_uniform_location("tex"), 0);
  // Set eye position
  glUniform3fv(eff.get_uniform_location("eye_pos"), 1, value_ptr(activeCam->get_position()));

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
  glUniformMatrix3fv(eff.get_uniform_location("N"), 1, GL_FALSE,
                     value_ptr(m.get_transform().get_normal_matrix()));
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

void Graphics::RenderSky() {

  renderer::bind(skyeffect2);
  //vert uniforms
  mat4 modelMatrix = mat4(1.0f);
  auto V = activeCam->get_view();
  auto P = activeCam->get_projection();
  mat4 MVP = P * V * modelMatrix;
  //frag uniforms
  
  float luminance = 1.0f;
  float turbidity = 10.0f;
  float reileigh = 4.0f;
  float mieCoefficient = 0.005f;
  float mieDirectionalG = 0.8f;
  //
  float inclination = (daymode ? dayscale : (1.0f - dayscale));//0.49f; // elevation / inclination
  float azimuth = (daymode ? 0.75f : 0.25f);//0.25f; // Facing front
  float distance = 900.0f;
  float theta = pi<float>() * (inclination - 0.5);
  float phi = 2 * pi<float>() * (azimuth - 0.5);
  vec3 sunPosition = vec3(distance * cos(phi), distance * sin(phi) * sin(theta), distance * sin(phi) * cos(theta));

  //set uniforms
  {
    //v
    glUniformMatrix4fv(skyeffect2.get_uniform_location("MVP"), 1, GL_FALSE, value_ptr(MVP)); 
    glUniformMatrix4fv(skyeffect2.get_uniform_location("modelMatrix"), 1, GL_FALSE, value_ptr(modelMatrix));
    //f
    glUniform3fv(skyeffect2.get_uniform_location("sunPosition"), 1, &sunPosition[0]);
    glUniform1f(skyeffect2.get_uniform_location("luminance"), luminance);
    glUniform1f(skyeffect2.get_uniform_location("turbidity"), turbidity);
    glUniform1f(skyeffect2.get_uniform_location("reileigh"), reileigh);
    glUniform1f(skyeffect2.get_uniform_location("mieCoefficient"), mieCoefficient);
    glUniform1f(skyeffect2.get_uniform_location("mieDirectionalG"), mieDirectionalG);
  }

  glDisable(GL_CULL_FACE);
  renderer::render(skygeo);
  glEnable(GL_CULL_FACE);

  return;  
}

bool Graphics::Render() {
  glEnable(GL_FOG); // enable the fog
  GLfloat density = 0.3f;
  GLfloat fogColor[4] = {0.5, 0.5, 0.5, 1.0};
  glFogi(GL_FOG_MODE, GL_EXP2); // set the fog mode to GL_EXP2
  glFogfv(GL_FOG_COLOR, fogColor);
  glFogf(GL_FOG_DENSITY, density);
  glHint(GL_FOG_HINT, GL_NICEST);

  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  // Render meshes
  for (auto &e : meshes) {
    Rendermesh(e.second, checkedTexture);
  }

  Rendermesh(*desertM, sandTexture);

  RendermeshB(goodsand, goodsandTexture, goodsandTextureBump, 10.0f);

  RenderSky();

  RenderMirror(mirror);

  DrawCross(vec3(0.0, 0.0, 0.0f), 10.0f);

  ProcessLines();
  return true;
}

Graphics::Graphics() {}

Graphics::~Graphics() {}

void main() {
  // Create application
  app application(1280, 720, 0);
  gfx = new Graphics();
  // Set load content, update and render methods
  application.set_load_content(std::bind(&Graphics::Load_content, gfx));
  application.set_initialise(std::bind(&Graphics::Initialise, gfx));
  application.set_update(std::bind(&Graphics::Update, gfx, std::placeholders::_1));
  application.set_render(std::bind(&Graphics::Render, gfx));

  // Run application
  application.run();
  delete gfx;
  gfx = nullptr;
}