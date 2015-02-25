#include "GeoUtils.h"
#include <set>
#include <unordered_map>

void CalculateNormals(std::vector<glm::vec3> &verts,
                      std::vector<glm::vec3> &normals,
                      std::vector<unsigned int> &indices) {

  if (normals.size() != verts.size()) {
    normals.resize(verts.size());
  }

  // for each vert
  for (unsigned int i = 0; i < verts.size(); i++) {
    // find every face that uses it
    std::vector<unsigned int> faces;
    for (unsigned int j = 0; j < indices.size(); j++) {
      if (indices[j] == i) {
        unsigned int rem = j % 3;
        if (rem == 0) {
          faces.push_back(j);
          faces.push_back(j + 1);
          faces.push_back(j + 2);
        } else if (rem == 1) {
          faces.push_back(j - 1);
          faces.push_back(j);
          faces.push_back(j + 1);
        } else if (rem == 3) {
          faces.push_back(j - 2);
          faces.push_back(j - 1);
          faces.push_back(j);
        }
      }
    }
    // for each face, add it's nomrmal.
    normals[i] = glm::vec3(0, 0, 0);
    for (unsigned int j = 0; j < faces.size(); j += 3) {
      normals[i] +=
          glm::normalize(glm::cross(verts[faces[j]] - verts[faces[j + 1]],
                                    verts[faces[j]] - verts[faces[j + 2]]));
    }
    normals[i] = glm::normalize(normals[i]);
  }
}

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

// Takes an ordered list of verts, returns a list of indices and verts
void IndexVerticies(std::vector<glm::vec3> &Vinput,
                    std::vector<glm::vec3> &Vout,
                    std::vector<unsigned int> &indices) {
  std::set<VPair, CmpClass> vertices;
  int index = 0;

  for (unsigned int i = 0; i < Vinput.size(); i++) {
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

// Verifies the indices and vertices match. If fix, will attempt to fix errors

void VerifyIndices(std::vector<glm::vec3> &verts,
                   std::vector<glm::vec3> *normals,
                   std::vector<glm::vec2> *tex_coords,
                   std::vector<unsigned int> &indices, const bool fix) {
  std::set<VPair, CmpClass> vertices;
  std::vector<unsigned int> removedVerts;
  std::unordered_map<unsigned int, unsigned int> remappedIndices;
  bool *vcheck = new bool[verts.size()];

  if (!fix && normals != NULL && normals->size() != verts.size()) {
    printf("Amount of normals[%i] != amount of verticies[%i]\n",
           normals->size(), verts.size());
  }
  if (!fix && tex_coords != NULL && tex_coords->size() != verts.size()) {
    printf("Amount of TexCoords[%i] != amount of verticies[%i]\n",
           tex_coords->size(), verts.size());
  }

  // check all indices are valid
  for (unsigned int i = 0; i < indices.size(); i++) {
    unsigned int ind = indices[i];
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
    if (it != vertices.end()) {
      if (!fix) {
        printf("duplicate vert [%i] and [%u], at (%f,%f,%f)\n", it->second, i,
               it->first.x, it->first.y, it->first.z);
      } else {
        // TODO check remapped doesn't already have this
        remappedIndices.insert(std::make_pair(i, (unsigned int)it->second));
        removedVerts.push_back(i);
      }
    } else {
      vertices.insert(std::make_pair(verts[i], i));
    }
    // check all verts are indiced atleast once
    if (vcheck[i] != true) {
      // we have a vert that is never indiced.
      if (!fix) {
        printf("Vert[%u], is never indiced!\n", i);
      } else {
        removedVerts.push_back(i);
      }
    }
  }

  for (auto itr = remappedIndices.begin(); itr != remappedIndices.end();
       ++itr) {
    unsigned int find = (*itr).first;
    unsigned int replace = (*itr).second;
    for (auto &i : indices) {
      if (i == find) {
        i = replace;
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