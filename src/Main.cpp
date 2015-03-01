#include "libENUgraphics\graphics_framework.h"
#include <glm\glm.hpp>
#include <glm\gtx\rotate_vector.hpp>
#include "DesertGen.h"
#include "Math.h"

using namespace std;
using namespace graphics_framework;
using namespace glm;

map<string, mesh> meshes;

effect gouradEffect;
effect texturedEffect;
effect skyeffect;
effect waterEffect;
texture checkedTexture;
texture sandTexture;

free_camera cam;
target_camera bouncecam;
camera *activeCam;

GLuint FramebufferName = 0;
GLuint renderedTexture = 0;
GLuint depthrenderbuffer = 0;

mesh *desertM;
mesh mirror;

directional_light light;

double cursor_x = 0.0;
double cursor_y = 0.0;

bool initialise() {
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

bool load_content() {
  mirror = mesh(geometry_builder::create_plane(100, 100, true));
  mirror.get_transform().translate(vec3(25.0f, 1.0f, 25.0f));
  mirror.get_transform().scale = vec3(0.25f, 0.25f, 0.25f);
  // mirror.get_transform().rotate(vec3(half_pi<float>() * 0.5f, pi<float>(),
  // 0.0f));

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

  //create mirror FB
  {
    // Create the FBO
    glGenFramebuffers(1, &FramebufferName);
    //Bind to FBO
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, FramebufferName);

    // Create the textures in the fbo
    glGenTextures(1, &renderedTexture);
    //bind it
    glBindTexture(GL_TEXTURE_2D, renderedTexture);
    //set dimensions,  Give an empty image to OpenGL ( the last "0" )
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 1280, 720, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);
    //tex filtering
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    //unbind
    glBindTexture(GL_TEXTURE_2D, 0);

    //Attach a level of a texture object as a logical buffer of a framebuffer object
    //target moust be GL_READ_FRAMEBUFFER, or GL_FRAMEBUFFER
    //attachment must be GL_COLOR_ATTACHMENTi, GL_DEPTH_ATTACHMENT, GL_STENCIL_ATTACHMENT or GL_DEPTH_STENCIL_ATTACHMENT.
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, renderedTexture, 0);

    //Do the same for depth, but use a renderbuffer rather than texture
    glGenRenderbuffers(1, &depthrenderbuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, depthrenderbuffer);
    //set dimensions, fill with undefined
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, 1280, 720);
    //attach to fbo depth output
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthrenderbuffer);

    //unbind
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
  }

  // Lights
  light.set_ambient_intensity(vec4(0.3f, 0.3f, 0.3f, 1.0f));
  light.set_light_colour(vec4(1.0f, 1.0f, 1.0f, 1.0f));
  light.set_direction(vec3(1.0f, 1.0f, -1.0f));

  // textures
  checkedTexture = texture("textures\\checked.gif");
  sandTexture = texture("textures\\sand_512_1.png");

  // shaders
  gouradEffect.add_shader("shaders\\gouraud.vert", GL_VERTEX_SHADER);
  gouradEffect.add_shader("shaders\\gouraud.frag", GL_FRAGMENT_SHADER);
  gouradEffect.build();

  texturedEffect.add_shader("shaders\\simple_texture.vert", GL_VERTEX_SHADER);
  texturedEffect.add_shader("shaders\\simple_texture.frag", GL_FRAGMENT_SHADER);
  texturedEffect.build();

  waterEffect.add_shader("shaders\\water.vert", GL_VERTEX_SHADER);
  waterEffect.add_shader("shaders\\water.frag", GL_FRAGMENT_SHADER);
  waterEffect.build();

  skyeffect.add_shader("shaders\\sky.vert", GL_VERTEX_SHADER);
  skyeffect.add_shader("shaders\\sky.frag", GL_FRAGMENT_SHADER);
  skyeffect.build();

  // Set camera properties
  cam.set_position(vec3(0.0f, 10.0f, 0.0f));
  cam.set_target(vec3(0.0f, 0.0f, 0.0f));
  auto aspect = static_cast<float>(renderer::get_screen_width()) /
                static_cast<float>(renderer::get_screen_height());
  cam.set_projection(quarter_pi<float>(), aspect, 2.414f, 2000.0f);
  activeCam = &cam;

  // build
  DesertGen::CreateDesert();
  desertM = &DesertGen::farMesh;
  desertM->get_material().set_emissive(vec4(0.0f, 0.0f, 0.0f, 1.0f));
  desertM->get_material().set_diffuse(vec4(1.0f, 1.0f, 1.0f, 1.0f));
  desertM->get_material().set_specular(vec4(1.0f, 1.0f, 1.0f, 1.0f));
  desertM->get_material().set_shininess(1000.0f);

  return true;
}

bool update(float delta_time) {
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

void rendermesh(mesh &m, texture &t) {
  effect eff = gouradEffect;
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

void renderSky() {
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

void renderWater() {
  // render the reflected scene into a texture
  {
    // TODO, add free camera get target.
     bouncecam.set_position(
        vec3(cam.get_position().x,
             cam.get_position().y - ((mirror.get_transform().position.y -
             cam.get_position().y)*2),
             cam.get_position().z));
     bouncecam.set_target(
        vec3(cam.get_target().x,
             cam.get_target().y - (cam.get_target().y +
             mirror.get_transform().position.y * 2),
             cam.get_target().z));
     bouncecam.update(0);

    //vec3 mirrorPos = mirror.get_transform().position;
    //vec3 mirrorNormal =
    //    normalize(GetUpVector(mirror.get_transform().orientation));
    //vec3 vectorToMirror = vec3(mirrorPos.x - cam.get_position().x,
    //                           mirrorPos.y - cam.get_position().y,
    //                           mirrorPos.z - cam.get_position().z);
    //vec3 mirrorReflectionVector =
    //    normalize(vectorToMirror -
    //              (2 * (dot(vectorToMirror, mirrorNormal)) * mirrorNormal));

    //bouncecam.set_position(vec3(
    //    cam.get_position().x,
    //    cam.get_position().y -
    //        ((mirror.get_transform().position.y - cam.get_position().y) * 2),
    //    cam.get_position().z));
    //bouncecam.set_target(bouncecam.get_position() + mirrorReflectionVector);

    bouncecam.update(1);
    // renderer::set_render_target(*mirrorFB);
    glBindFramebuffer(GL_FRAMEBUFFER, FramebufferName);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // Clear the depth and colour buffers  
    //activeCam = &bouncecam;

    //Rerender scene

    //glDisable(GL_CULL_FACE);
    //glDisable(GL_DEPTH_TEST);
    for (auto &e : meshes) {
      rendermesh(e.second, checkedTexture);
    }
    rendermesh(*desertM, sandTexture);
     renderSky();

    // end render
    //glEnable(GL_CULL_FACE);
    //glEnable(GL_DEPTH_TEST);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    activeCam = &cam;
  }

  // render texture onto plane
  renderer::bind(waterEffect);

  auto M = mirror.get_transform().get_transform_matrix();
  auto V = activeCam->get_view();
  auto P = activeCam->get_projection();
  auto MVP = P * V * M;

  // Bind texture
  // renderer::bind(checkedTexture, 0);

  // Bind our texture in Texture Unit 0
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, renderedTexture);
  // Set our "renderedTexture" sampler to user Texture Unit 0
  glUniform1i(waterEffect.get_uniform_location("tex"), 0);

  // Set MVP matrix uniform
  glUniformMatrix4fv(waterEffect.get_uniform_location("MVP"), 1, GL_FALSE,
                     value_ptr(MVP));

  renderer::render(mirror);
}

bool render() {
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  // Render meshes
  for (auto &e : meshes) {
    rendermesh(e.second, checkedTexture);
  }

  rendermesh(*desertM, sandTexture);

  renderSky();

  renderWater();

  return true;
}

void main() {
  // Create application
  app application(1280, 720, 0);
  // Set load content, update and render methods
  application.set_load_content(load_content);
  application.set_initialise(initialise);
  application.set_update(update);
  application.set_render(render);
  // Run application
  application.run();
}