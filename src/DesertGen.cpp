#include "DesertGen.h"
#include "Perlin.h"
#include <glm\glm.hpp>
#include <climits>
#include <set>
#include "libENUgraphics\graphics_framework.h"

graphics_framework::geometry DesertGen::_geo;

DesertGen::DesertGen() {}

DesertGen::~DesertGen() {}

typedef std::pair<glm::vec3, int> VPair;
float eps = 0.001f;
struct CmpClass // class comparing vertices in the set
    {
  bool operator()(const VPair &p1, const VPair &p2) const {
    if (fabs(p1.first.x - p2.first.x) > eps)
      return p1.first.x < p2.first.x;
    if (fabs(p1.first.y - p2.first.y) > eps)
      return p1.first.y < p2.first.y;
    if (fabs(p1.first.z - p2.first.z) > eps)
      return p1.first.z < p2.first.z;
    return false;
  }
};

void ComputeBuffers(std::vector<glm::vec3> &Vinput,
                    std::vector<glm::vec3> &Vout,
                    std::vector<GLuint> &indices) {
  std::set<VPair, CmpClass> vertices;
  int index = 0;

  for (int i = 0; i < Vinput.size(); i++) {
    std::set<VPair>::iterator it = vertices.find(
        std::make_pair(Vinput[i], 0 /*this value doesn't matter*/));
    if (it != vertices.end())
      indices.push_back(it->second);
    else {
      vertices.insert(std::make_pair(Vinput[i], index));
      indices.push_back(index++);
    }
  }

  // Notice that the vertices in the set are not sorted by the index
  // so you'll have to rearrange them like this:
  Vout.resize(vertices.size());
  for (std::set<VPair>::iterator it = vertices.begin(); it != vertices.end();
       ++it)
    Vout[it->second] = it->first;
}

void VerifyIndices(std::vector<glm::vec3> &verts,
                   std::vector<GLuint> &indices) {
  std::set<VPair, CmpClass> vertices;
  bool *vcheck = new bool[verts.size()];

  // check all indices are valid
  for (int i = 0; i < indices.size(); i++) {
    GLuint ind = indices[i];
    assert(ind < verts.size()); // indice points to a non-existant vert
    vcheck[ind] = true;
  }

  // check for duplicate verts
//
  for (unsigned int i = 0; i < verts.size(); i++) {
    std::set<VPair>::iterator it = vertices.find(
        std::make_pair(verts[i], 0 /*this value doesn't matter*/));
    if (it != vertices.end()) {
      printf("duplicate vert [%i] and [%u], at (%f,%f,%f)\n", it->second, i,
             it->first.x, it->first.y, it->first.z);
    } else {
      vertices.insert(std::make_pair(verts[i], i));
    }
    // check all verts are indiced atleast once
    if (vcheck[i] != true) {
      // we have a vert that is never indiced.
      printf("Vert[%u], is never indiced!\n", i);
    }
  }

  delete[] vcheck;
}

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

#define resolution 256
#define prelinscaleXZ 0.15f
#define prelinscaleY 2.0f

graphics_framework::geometry *DesertGen::CreateDesert() {

  std::vector<glm::vec3> positions;
  std::vector<glm::vec3> normals;
  std::vector<glm::vec2> tex_coords;
  std::vector<GLuint> indices;

  create_plane(resolution, resolution, 0.25f, 16.0f, positions, normals, tex_coords,
               indices);
  VerifyIndices(positions, indices);

  // ComputeBuffers(positions, NEWpositions, indices);
  //  positions.clear();
  unsigned int amountD = (unsigned int)(0.25f * (float)resolution);
  unsigned int amountW = amountD;

  for (unsigned int i = 0; i < amountD; i++) {
    for (unsigned int j = 0; j < amountW; j++) {
      positions[j + (i*amountD)].y = raw_noise_2d(i*prelinscaleXZ, j*prelinscaleXZ)*prelinscaleY;
    }
  }

  _geo = graphics_framework::geometry();

  _geo.add_buffer(positions,
                  graphics_framework::BUFFER_INDEXES::POSITION_BUFFER);
  _geo.add_index_buffer(indices);
  _geo.add_buffer(normals, graphics_framework::BUFFER_INDEXES::NORMAL_BUFFER);
  _geo.add_buffer(tex_coords,graphics_framework::BUFFER_INDEXES::TEXTURE_COORDS_0);

  // Generate tangent and binormal data
  // graphics_framework::generate_tb(_geo, normals);
  return &_geo;
}
