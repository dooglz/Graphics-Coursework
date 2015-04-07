/* Main.h
* Encapsulates global varaibles fom main.cpp into a class for safer accees from
* other areas of code.
* Singleton pattern, instance stored in "gfx"
*
* Sam Serrels, Computer Graphics, 2015
*/

#pragma once

#include <string>
#include <map>
#include "libENUgraphics\mesh.h"
#include "libENUgraphics\texture.h"
#include "libENUgraphics\effect.h"
#include "libENUgraphics\free_camera.h"
#include "libENUgraphics\directional_light.h"
#include "libENUgraphics\spot_light.h"
#include "libENUgraphics\point_light.h"

using namespace graphics_framework;

#define SCREENHEIGHT 720
#define SCREENWIDTH 1280

class Graphics {
public:
  bool Render();
  bool Update(float delta_time);
  bool Load_content();
  bool Initialise();

  Graphics();
  ~Graphics();

  std::map<std::string, mesh> meshes;

  double cursor_x = 0.0;
  double cursor_y = 0.0;
  float aspect;
  float counter = 0;
  float realtime;

  // The current active camera, used for rendering
  camera *activeCam;
  // The main camera used in the scene
  free_camera cam;

  texture checkedTexture;
  texture sandTexture;
  texture goodsandTexture;
  texture goodsandTextureBump;
  texture noise16;

  effect simpleEffect;
  effect gouradEffect;
  effect phongEffect;
  effect texturedEffect;
  effect texturedBumpEffect;
  effect geoPassEffect;
  effect nullEffect;


  mesh *desertM;
  mesh mirror;
  mesh goodsand;
  mesh sphereMesh;
  geometry planegeo;


  // Light stuff
  directional_light dlight;
  point_light plight;
  spot_light slight;
  glm::vec4 NumberOfLights; // Amd workaround
  GLuint dLightSSBO;
  GLuint pLightSSBO;
  GLuint sLightSSBO;
  GLuint LightSSBO;
  std::vector<directional_light*> DLights;
  std::vector<point_light*> PLights;
  std::vector<spot_light*> SLights;



  std::vector<const glm::vec3> linebuffer;
  void DrawLine(const glm::vec3 &p1, const glm::vec3 &p2);
  void DrawCross(const glm::vec3 &p1, const float size);
  void ProcessLines();
  void MakeGyroscope();
  void UpdateGyroscope(float delta_time);
  void Rendermesh(mesh &m, texture &t);
  // Renders a mesh with a bump map
  void RendermeshB(mesh &m, const texture &t, const texture &tb, const float scale);
  void DrawScene();
  void RenderSky();
  // Send all light data to SSBOs on the GPU
  void UpdateLights();
  void UpdateTorus(float delta_time);
};

extern Graphics *gfx;