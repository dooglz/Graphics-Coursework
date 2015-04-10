#pragma once
#include <map>
#include "libENUgraphics\mesh.h"
#include "libENUgraphics\graphics_framework.h"
using namespace graphics_framework;
using namespace glm;

class Gimbal {
public:
  Gimbal(const unsigned int rings);
  void Update(float delta_time);
  void Render();
  ~Gimbal();

private:
  std::map<std::string, mesh> rings;
};

Gimbal::Gimbal(const unsigned int num_rings) {
  rings["torus0"] = mesh(geometry_builder::create_torus(42, 42, 0.5f, num_rings));
  rings["torus0"].get_transform().translate(vec3(10.0f, num_rings + 2.0f, -30.0f));
  for (unsigned int i = 1; i < num_rings; i++) {
    rings["torus" + std::to_string(i)] = mesh(geometry_builder::create_torus(20, 20, 0.5f, num_rings - i));
    rings["torus" + std::to_string(i)].get_transform().parent = &rings["torus" + std::to_string(i - 1)].get_transform();
    rings["torus" + std::to_string(i)].get_material().set_emissive(vec4(0.2f, 0.2f, 0.2f, 1.0f));
    rings["torus" + std::to_string(i)].get_material().set_diffuse(
        vec4(((i - 3) % 9) / 9.0f, (i % 9) / 9.0f, ((i - 6) % 9) / 9.0f, 1.0f));
    rings["torus" + std::to_string(i)].get_material().set_specular(vec4(1.0f, 1.0f, 1.0f, 1.0f));
  }
}

Gimbal::~Gimbal() {}

void Gimbal::Update(float delta_time) {
  for (unsigned int i = 0; i < rings.size(); i++) {
    glm::vec3 rot;
    switch (i % 2) {
    case (0):
      rot = glm::vec3(0, 0, 1);
      break;
    case (1):
      rot = glm::vec3(-1, 0, 0);
      break;
    case (2):
      rot = glm::vec3(0, 1, 0);
      break;
    case (3):
      rot = glm::vec3(0, 0, -1);
      break;
    case (4):
      rot = glm::vec3(1, 0, 0);
      break;
    case (5):
      rot = glm::vec3(0, -1, 0);
      break;
    }
    rings["torus" + std::to_string(i)].get_transform().rotate((delta_time)*0.2f * rot);
  }
}

void Gimbal::Render() {
  for (auto &e : rings) {
    gfx->Rendermesh(e.second, gfx->checkedTexture);
  }
}