#pragma once

#include <string>
#include <map>
#include "libENUgraphics\mesh.h"
#include "libENUgraphics\texture.h"
#include "libENUgraphics\effect.h"
#include "libENUgraphics\free_camera.h"
#include "libENUgraphics\directional_light.h"

using namespace graphics_framework;

class graphics {
public:
  bool render();
  bool update(float delta_time);
  bool load_content();
  bool initialise();

  graphics();
  ~graphics();

  std::map<std::string, mesh> meshes;
  float aspect;

  camera *activeCam;
  free_camera cam;

  texture checkedTexture;
  texture sandTexture;
  texture goodsandTexture;
  texture goodsandTextureBump;

  effect simpleEffect;
  effect gouradEffect;
  effect phongEffect;
  effect texturedEffect;
  effect texturedBumpEffect;
  effect skyeffect;

  mesh *desertM;
  mesh mirror;
  mesh goodsand;

  directional_light light;

  void DrawLine(const glm::vec3 &p1, const glm::vec3 &p2);
  void DrawCross(const glm::vec3 &p1, const float size);
  void processLines();
  void rendermesh(mesh &m, texture &t);
  void graphics::rendermeshB(mesh& m, texture& t, texture& tb, const float scale);
  void renderSky();
};

extern graphics *gfx;