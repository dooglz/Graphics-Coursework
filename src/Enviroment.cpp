#include "Enviroment.h"
#include "Main.h"
#include "libENUgraphics\graphics_framework.h"
#include <glm\glm.hpp>
#include <glm\gtx\rotate_vector.hpp>
using namespace std;
using namespace graphics_framework;
using namespace glm;

static float EnviromentCounter = 0;
effect skyeffect;
mesh skygeo;

float Enviroment::dayscale;
bool Enviroment::daymode;
bool Enviroment::enabled;
float Enviroment::speed;

void Enviroment::Load() {
  skyeffect.add_shader("shaders\\sky2.vert", GL_VERTEX_SHADER);
  skyeffect.add_shader("shaders\\sky2.frag", GL_FRAGMENT_SHADER);
  skyeffect.build();
  skygeo = mesh(geometry_builder::create_sphere(32, 15, vec3(2000.0f, 2000.0f, 2000.0f)));
  dayscale = 0;
  daymode = false;
  speed = 0.08f;
  enabled = 0;
}

void Enviroment::Update(float delta_time) {
  if (enabled) {
    EnviromentCounter += (delta_time * speed);
    if (EnviromentCounter > pi<float>()) {
      EnviromentCounter = 0;
    }
  }
  // counter		0			          pi/2			        pi
  //            midday   Dusk   midnight  Dawn   Midday
  // dayscale	  1.0			 0.5    0				   0.5    1.0
  // sunscale   0.5      1      0          0      0.5
  // daymode    0        0      0          1      1

  const float oldd = dayscale;
  dayscale = fabs(EnviromentCounter - half_pi<float>()) / half_pi<float>();
  daymode = (dayscale > oldd);
  float sunscale = (daymode ? dayscale - 0.5f : 0.5f + (1.0f - dayscale));
  sunscale = (sunscale > 1.0f ? 1.0f : sunscale);
  sunscale = (sunscale < 0.0f ? 0.0f : sunscale);

  // printf("dayscale %f\tsunscale %f\tdaymode %d\n", dayscale, sunscale, daymode);
  gfx->dlight.set_direction(glm::rotateX(vec3(0, 0, 1.0), (1.0f - sunscale) * pi<float>()));
  // gfx->dlight.set_direction(vec3(0.0f, -1.0f, 0.0f));

  gfx->DrawCross(vec3(10, 6, 10), 2.0f);
  gfx->DrawCross(vec3(10, 6, 10) + (gfx->dlight.get_direction() * 2.0f), 1.0f);
  gfx->DrawLine(vec3(10, 6, 10), vec3(10, 6, 10) + (gfx->dlight.get_direction() * 2.0f));
  gfx->dlight.set_light_colour(mix(vec4(0, 0, 0, 1.0f), vec4(0.8f, 0.8f, 0.8f, 1.0f), dayscale));
}

void Enviroment::RenderSky() {

  renderer::bind(skyeffect);
  // vert uniforms
  mat4 modelMatrix = mat4(1.0f);
  auto V = gfx->activeCam->get_view();
  auto P = gfx->activeCam->get_projection();
  mat4 MVP = P * V * modelMatrix;
  // frag uniforms

  float luminance = 1.0f;
  float turbidity = 10.0f;
  float reileigh = 4.0f;
  float mieCoefficient = 0.005f;
  float mieDirectionalG = 0.8f;
  //
  float inclination = (daymode ? dayscale : (1.0f - dayscale)); // 0.49f; // elevation / inclination
  float azimuth = (daymode ? 0.75f : 0.25f);                    // 0.25f; // Facing front
  float distance = 900.0f;
  float theta = pi<float>() * (inclination - 0.5);
  float phi = 2 * pi<float>() * (azimuth - 0.5);
  vec3 sunPosition = vec3(distance * cos(phi), distance * sin(phi) * sin(theta), distance * sin(phi) * cos(theta));

  // set uniforms
  {
    // v
    glUniformMatrix4fv(skyeffect.get_uniform_location("MVP"), 1, GL_FALSE, value_ptr(MVP));
    glUniformMatrix4fv(skyeffect.get_uniform_location("modelMatrix"), 1, GL_FALSE, value_ptr(modelMatrix));
    // f
    glUniform3fv(skyeffect.get_uniform_location("sunPosition"), 1, &sunPosition[0]);
    glUniform1f(skyeffect.get_uniform_location("luminance"), luminance);
    glUniform1f(skyeffect.get_uniform_location("turbidity"), turbidity);
    glUniform1f(skyeffect.get_uniform_location("reileigh"), reileigh);
    glUniform1f(skyeffect.get_uniform_location("mieCoefficient"), mieCoefficient);
    glUniform1f(skyeffect.get_uniform_location("mieDirectionalG"), mieDirectionalG);
  }
  glDisable(GL_CULL_FACE);
  glDepthMask(false);
  renderer::render(skygeo);
  glDepthMask(true);
  glEnable(GL_CULL_FACE);
  return;
}