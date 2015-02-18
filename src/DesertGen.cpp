#include "DesertGen.h"
#include "Perlin.h"
#include <glm\glm.hpp>
#include <climits>
#include <set>
#include "libENUgraphics\graphics_framework.h"

using namespace graphics_framework;
geometry DesertGen::_farGeo;
geometry DesertGen::_nearGeo;
mesh DesertGen::farMesh;

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
                   std::vector<glm::vec3> *normals,
                   std::vector<glm::vec2> *tex_coords,
                   std::vector<GLuint> &indices, const bool fix) {
  std::set<VPair, CmpClass> vertices;
  std::vector<unsigned int> removedVerts;
  bool *vcheck = new bool[verts.size()];

  
  if (!fix && normals != NULL && normals->size() != verts.size()) {
    printf("Amount of normals[%i] != amount of verticies[%i]\n", normals->size(), verts.size());
  }
  if (!fix && tex_coords != NULL && tex_coords->size() != verts.size()) {
    printf("Amount of TexCoords[%i] != amount of verticies[%i]\n", tex_coords->size(), verts.size());
  }

  // check all indices are valid
  for (int i = 0; i < indices.size(); i++) {
    GLuint ind = indices[i];
    if (ind > verts.size()) {
      if (!fix) {
        printf("Indice[%i] = %i, out of range\n", i, ind);
      }
    }
    vcheck[ind] = true;
  }

  // check for duplicate verts
  for (unsigned int i = 0; i < verts.size(); i++) {
    std::set<VPair>::iterator it = vertices.find(std::make_pair(verts[i], 0));
    if (!fix) {
      if (it != vertices.end()) {
        printf("duplicate vert [%i] and [%u], at (%f,%f,%f)\n", it->second, i,
               it->first.x, it->first.y, it->first.z);
      } else {
        vertices.insert(std::make_pair(verts[i], i));
      }
    }
    // check all verts are indiced atleast once
    if (vcheck[i] != true) {
      // we have a vert that is never indiced.
      if (!fix) {
        printf("Vert[%u], is never indiced!\n", i);
      }else{
        removedVerts.push_back(i);
      }
    }
  }

  // remove unused verts
  if (removedVerts.size() > 0 && fix) {
    for (unsigned int i = 0; i < removedVerts.size(); i++) {
      // first, remove the actual vert. -i is beacuse the index moves the more
      // we remove
      if (normals != NULL) {
        normals->erase(normals->begin() + (removedVerts[i] - i));
      }
      if (tex_coords != NULL) {
        tex_coords->erase(tex_coords->begin() + (removedVerts[i] - i));
      }
      verts.erase(verts.begin() + (removedVerts[i] - i));
    }
    // now we have to shift down every index that was higher, can't put this
    // in the above loop as the decrement accumulates from it's original value
    for (unsigned int j = 0; j < indices.size(); j++) {
      int decrement = 0;
      for (unsigned int i = 0; i < removedVerts.size(); i++) {
        if (indices[j] >= removedVerts[i]) {
          decrement++;
        }
      }
      indices[j] -= decrement;
    }
  }

  removedVerts.clear();
  vertices.clear();
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

#define farSize 256
#define farRes 0.25f
#define prelinscaleXZ 0.15f
#define prelinscaleY 3.0f

void create_plane2(const unsigned int &width, const unsigned int &depth, const float &resolution, const float &uvScale,
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

      float perlin = raw_noise_2d(x*prelinscaleXZ, z*prelinscaleXZ)*prelinscaleY;
      if (perlin < 0){ perlin = 0;}

      glm::vec3 pos = glm::vec3((xf / amountWf) * width - (width / 2), 0, (zf / amountDf) * depth - (depth / 2));
      //distance from center
      float distance = glm::length(pos);

      //scale perlin by length
      if (distance >(farSize / 5)){
        pos.y = perlin;
      }

      positions.push_back(pos);
      tex_coords.push_back(glm::vec2((xf*uvScale) / amountWf, (zf*uvScale) / amountDf));
      normals.push_back(glm::vec3(0.0f, 1.0f, 0.0f));
    }
  }

  // 0----1
  // | \  |
  // |  \ |
  // 2----3
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
    const glm::vec3 n1 = glm::normalize(glm::cross(v2 - v0, v3 - v0));
    const glm::vec3 n2 = glm::normalize(glm::cross(v3 - v0, v1 - v0));
    const glm::vec3 center1 = (v0 + v3 + v2) / 3.0f;
    const glm::vec3 center2 = (v0 + v1 + v3) / 3.0f;
    float dt1 = glm::dot(n1, center1);
    float dt2 = glm::dot(n2, center2);

    if (dt1 < -0.5f){
      //remove this face
      k++;
    }else{
      // Triangle 1 0-3-2
      indices.push_back(j);
      indices.push_back(j + amountW + 1);
      indices.push_back(j + amountW);
    }

    if (dt2 < -0.5f){
      //remove this face
      k++;
    }else{
      // Triangle 2 0-1-3
      indices.push_back(j);
      indices.push_back(j + 1);
      indices.push_back(j + amountW + 1);
    }

    j++;
  }
  printf("%i faces removed", k);
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

  /*
  //Displace with perlin
  const unsigned int amountD = (unsigned int)(farRes * (float)farSize);
  const unsigned int amountW = amountD;

  for (unsigned int i = 0; i < amountD; i++) {
    for (unsigned int j = 0; j < amountW; j++) {
      float perlin = raw_noise_2d(i*prelinscaleXZ, j*prelinscaleXZ)*prelinscaleY;
      if (perlin < 0) continue;
      //distance from center
      float distance = glm::length(positions[j + (i*amountD)]);
      //scale perlin by length
      if (distance >(farSize/5))
        positions[j + (i*amountD)].y = perlin;
    }
  }

  VerifyIndices(positions, indices);
  int k = 0;
  //remove hidden faces
  for (unsigned int i = 0; i < indices.size(); i+=3) {
    const glm::vec3 const &a = positions[indices[i]];
    const glm::vec3 const &b = positions[indices[i + 1]];
    const glm::vec3 const &c = positions[indices[i + 2]];
    const glm::vec3 normal = glm::normalize(glm::cross(c - a, b - a));
    const glm::vec3 center = (a+b+c)/3.0f;
    float dt = glm::dot(normal, center);
    if (dt < -0.5f){
      //remove this face
      indices.erase(indices.begin() + i, indices.begin() + i + 3);
      i-=3;
      k++;
    }
  }
  printf("%i faces removed",k);
  */
  VerifyIndices(positions, &normals, &tex_coords, indices, true);
  printf("yololo\n");
  VerifyIndices(positions,NULL,NULL, indices, 0);
  //return
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
