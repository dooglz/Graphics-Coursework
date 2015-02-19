#include "DesertGen.h"
#include "Perlin.h"
#include <glm\glm.hpp>
#include "GeoUtils.h"
#include <climits>

#include "libENUgraphics\graphics_framework.h"

using namespace graphics_framework;
geometry DesertGen::_farGeo;
geometry DesertGen::_nearGeo;
mesh DesertGen::farMesh;

DesertGen::DesertGen() {}

DesertGen::~DesertGen() {}



void create_plane(const unsigned int &width, const unsigned int &depth,
                  const float &resolution, const float &uvScale,
                  std::vector<glm::vec3> &positions,
                  std::vector<glm::vec3> &normals,
                  std::vector<glm::vec2> &tex_coords,
                  std::vector<GLuint> &indices) {

  // amount of verts in each dimension
  unsigned int amountD = (unsigned int)((float)depth * resolution);
  unsigned int amountW = (unsigned int)((float)width * resolution);
  float amountDf = static_cast<float>(amountD);
  float amountWf = static_cast<float>(amountW);
  unsigned int amountQuads = pow(sqrt(amountD * amountW) - 1, 2);

  for (unsigned int x = 0; x < amountD; ++x) {
    float xf = static_cast<float>(x);
    for (unsigned int z = 0; z < amountW; ++z) {
      float zf = static_cast<float>(z);
      positions.push_back(glm::vec3((xf / amountWf) * width - (width/2), 0, (zf / amountDf) * depth - (depth/2)));
      tex_coords.push_back(glm::vec2((xf*uvScale) / amountWf, (zf*uvScale) / amountDf));
      normals.push_back(glm::vec3(0.0f, 1.0f, 0.0f));
    }
  }

  // 0----1
  // | \  |
  // |  \ |
  // 2----3
  // make some triangles
  unsigned int j = 0;
  for (unsigned int i = 0; i < amountQuads; ++i) {
    if (j > 1 && (j + 1) % amountW == 0) {
      j++;
    }
    // Triangle 1 0-3-2
    indices.push_back(j);
    indices.push_back(j + amountW + 1);
    indices.push_back(j + amountW);

    // Triangle 2 0-1-3
    indices.push_back(j);
    indices.push_back(j + 1);
    indices.push_back(j + amountW + 1);
    j++;
  }
}

#define farSize 256
#define farRes 0.25f
#define prelinscaleXZ 0.15f
#define prelinscaleY 3.0f
#define dunedistance (farSize / 5)
#define groundcutoff (farSize / 9)

void create_plane2(const unsigned int &width, const unsigned int &depth,
                   const float &resolution, const float &uvScale,
                   std::vector<glm::vec3> &positions,
                   std::vector<glm::vec3> &normals,
                   std::vector<glm::vec2> &tex_coords,
                   std::vector<GLuint> &indices) {

  // amount of verts in each dimension
  unsigned int amountD = (unsigned int)((float)depth * resolution);
  unsigned int amountW = (unsigned int)((float)width * resolution);
  float amountDf = static_cast<float>(amountD);
  float amountWf = static_cast<float>(amountW);
  unsigned int amountQuads = pow(sqrt(amountD * amountW) - 1, 2);

  for (unsigned int x = 0; x < amountD; ++x) {
    float xf = static_cast<float>(x);
    for (unsigned int z = 0; z < amountW; ++z) {
      float zf = static_cast<float>(z);

      float perlin =
          raw_noise_2d(x * prelinscaleXZ, z * prelinscaleXZ) * prelinscaleY;
      if (perlin < 0) {
        perlin = 0;
      }

      glm::vec3 pos = glm::vec3((xf / amountWf) * width - (width / 2), 0,
                                (zf / amountDf) * depth - (depth / 2));
      // distance from center
      float distance = glm::length(pos);

      // scale perlin by length
      if (distance > dunedistance) {
        pos.y = perlin;
      }

      positions.push_back(pos);
      tex_coords.push_back(
          glm::vec2((xf * uvScale) / amountWf, (zf * uvScale) / amountDf));
      normals.push_back(glm::vec3(0.0f, 0.0f, 0.0f));
    }
  }
  // make some triangles
  int k = 0;
  unsigned int j = 0;
  for (unsigned int i = 0; i < amountQuads; ++i) {
    if (j > 1 && (j + 1) % amountW == 0) {
      j++;
    }
    const glm::vec3 v0 = positions[j];
    const glm::vec3 v1 = positions[j + 1];
    const glm::vec3 v2 = positions[j + amountW];
    const glm::vec3 v3 = positions[j + amountW + 1];

    // Triangle 1 0-3-2
    const glm::vec3 n1 = glm::normalize(glm::cross(v2 - v0, v3 - v0));
    const glm::vec3 center1 = (v0 + v3 + v2) / 3.0f;

    if (abs(center1.x) > groundcutoff || abs(center1.z) > groundcutoff) {
      const float dt1 = glm::dot(n1, center1);
      if (dt1 < -0.5f) {
        k++;
      } else {
        indices.push_back(j);
        indices.push_back(j + amountW + 1);
        indices.push_back(j + amountW);
        const glm::vec3 normal = glm::normalize(glm::cross(v0 - v3, v0 - v2));
        normals[j] += normal;
        normals[j + amountW + 1] += normal;
        normals[j + amountW] += normal;
      }
    }else {
      k++;
    }

    // Triangle 2 0-1-3
    const glm::vec3 n2 = glm::normalize(glm::cross(v3 - v0, v1 - v0));
    const glm::vec3 center2 = (v0 + v1 + v3) / 3.0f;
    if (abs(center2.x) > groundcutoff || abs(center2.z) > groundcutoff) {
      const float dt2 = glm::dot(n2, center2);
      if (dt2 < -0.5f) {
        k++;
      } else {

        indices.push_back(j);
        indices.push_back(j + 1);
        indices.push_back(j + amountW + 1);
        const glm::vec3 normal = glm::normalize(glm::cross(v0 - v1, v0 - v3));
        normals[j] += normal;
        normals[j + 1] += normal;
        normals[j + amountW + 1] += normal;
      }
    } else {
      k++;
    }
    j++;
  }
  printf("%i faces removed\n", k);
  for (unsigned int i = 0; i < normals.size(); ++i) {
    normals[i] = glm::normalize(normals[i]);
  }
}

#define resolution 256

void DesertGen::makefarGeometry()
{
  std::vector<glm::vec3> positions;
  std::vector<glm::vec3> normals;
  std::vector<glm::vec2> tex_coords;
  std::vector<GLuint> indices;
  //get a flat plane
  create_plane2(farSize, farSize, farRes, 24.0f, positions, normals, tex_coords, indices);
  VerifyIndices(positions, &normals, &tex_coords, indices, true);
  printf("yololo\n");
  VerifyIndices(positions,NULL,NULL, indices, 0);


  _farGeo = geometry();
  _farGeo.add_buffer(positions, BUFFER_INDEXES::POSITION_BUFFER);
  _farGeo.add_index_buffer(indices);
  _farGeo.add_buffer(normals, BUFFER_INDEXES::NORMAL_BUFFER);
  _farGeo.add_buffer(tex_coords, BUFFER_INDEXES::TEXTURE_COORDS_0);
  farMesh = mesh(_farGeo);
  farMesh.get_transform().scale = glm::vec3(10.0f, 10.0f, 10.0f);
}


void DesertGen::CreateDesert() {
  makefarGeometry();
}
