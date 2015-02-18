#include "DesertGen.h"
#include <glm\glm.hpp>
#include <climits> 
#include <set>
#include "libENUgraphics\graphics_framework.h"

graphics_framework::geometry DesertGen::_geo;

DesertGen::DesertGen()
{

}

DesertGen::~DesertGen()
{

}



typedef std::pair<glm::vec3, int> VPair;
float eps = 0.001f;
struct CmpClass // class comparing vertices in the set
{
  bool operator() (const VPair& p1, const VPair& p2) const
  {
    if (fabs(p1.first.x - p2.first.x) > eps) return p1.first.x < p2.first.x;
    if (fabs(p1.first.y - p2.first.y) > eps) return p1.first.y < p2.first.y;
    if (fabs(p1.first.z - p2.first.z) > eps) return p1.first.z < p2.first.z;
    return false;
  }
};

void ComputeBuffers(std::vector<glm::vec3> &Vinput, std::vector<glm::vec3> &Vout, std::vector<GLuint> &indices)
{
  std::set<VPair, CmpClass> vertices;
  int index = 0;

  for (int i = 0; i<Vinput.size(); i++)
  {
    std::set<VPair>::iterator it = vertices.find(std::make_pair(Vinput[i], 0/*this value doesn't matter*/));
    if (it != vertices.end()) indices.push_back(it->second);
    else
    {
      vertices.insert(std::make_pair(Vinput[i], index));
      indices.push_back(index++);
    }
  }

  // Notice that the vertices in the set are not sorted by the index
  // so you'll have to rearrange them like this:
  Vout.resize(vertices.size());
  for (std::set<VPair>::iterator it = vertices.begin(); it != vertices.end(); it++)
    Vout[it->second] = it->first;
}


void VerifyIndices(std::vector<glm::vec3> &verts, std::vector<GLuint> &indices)
{
  std::set<VPair, CmpClass> vertices;
  bool* vcheck = new bool[verts.size()];

  //check all indices are valid
  for (int i = 0; i<indices.size(); i++)
  {
    GLuint ind = indices[i];
    assert(ind < verts.size()); //indice points to a non-existant vert
    vcheck[ind] = true;
  }

  //check for duplicate verts
  for (int i = 0; i<verts.size(); i++)
  {
    std::set<VPair>::iterator it = vertices.find(std::make_pair(verts[i], 0/*this value doesn't matter*/));
    if (it != vertices.end()){
      printf("duplicate vert [%u] and [%u], at (%f,%f,%f)\n", it->second, i, it->first.x, it->first.y, it->first.z);
    }else{
      vertices.insert(std::make_pair(verts[i], i));
    }
    //check all verts are indiced atleast once
    assert(vcheck[i] == true); // we have a vert that is never indiced.
  }

  delete vcheck;
}


void create_plane(const unsigned int &width, const unsigned int &depth,
                  const float &resolution, std::vector<glm::vec3> &positions,
                  std::vector<glm::vec3> &normals,
                  std::vector<glm::vec2> &tex_coords,
                  std::vector<GLuint> &indices) {
  // Minimal and maximal points
  glm::vec3 minimal(0.0f, 0.0f, 0.0f);
  glm::vec3 maximal(0.0f, 0.0f, 0.0f);

  // Iterate through each vertex and add to geometry
  std::array<glm::vec3, 4> verts;

  float depthf = static_cast<float>(depth);
  depthf = depthf / 2.0f;
  float widthf = static_cast<float>(width);
  widthf = -widthf / 2.0f;
  assert(resolution != 0);
  float ires = (1 / resolution);


  unsigned int linebreak = (depth*(resolution * 2))+2;
  unsigned int c = 0;

  for (float x = 0; x < width; x += ires) {
    for (float z = 0; z < depth; z += ires) {
    //  c++;
    //  if (c>1){break;}
      // Calculate vertex positions
      verts[0] = glm::vec3(widthf + x, 0.0f, depthf - z);
      verts[1] = glm::vec3(widthf + (x + ires), 0.0f, depthf - z);
      verts[2] = glm::vec3(widthf + x, 0.0f, depthf - (z + ires));
      verts[3] = glm::vec3(widthf + (x + ires), 0.0f, depthf - (z + ires));

      // Recalculate minimal and maximal
      for (auto &v : verts) {
        minimal = glm::min(minimal, v);
        maximal = glm::max(maximal, v);
      }

      unsigned int currIndex = positions.size(); //Todo: don't do this, resize at start
      unsigned int zeroindex;
      // Triangle 1 0-3-2

      //We have 4 verts, making 2 triangles, 0-3-2 and 0-1-3
      //vert 0 and 1 are most likely duplicates from the last loop
      //we check for this first

      unsigned int zeroindice;
      unsigned int oneindice;
      // do we already have vert 0?
      if (currIndex < 2 || positions[currIndex - 2] != verts[0]) {
        // not on previous triangle, what about row before?
        if (x >= ires && positions[(currIndex - linebreak) + 1] == verts[0]) {
          // ha! we have this on the pervious row
          zeroindice = (currIndex - linebreak) + 1;
        } 
        else if (x >= ires && positions[(currIndex - linebreak) + 2] == verts[0]) {
          // ha! we have this on the pervious row
          zeroindice = (currIndex - linebreak) + 2;
        }else {
          positions.push_back(verts[0]);
          tex_coords.push_back(glm::vec2(x, z) / 10.0f);
          zeroindice = currIndex;
          currIndex++;
        }
      } else {
        zeroindice = currIndex - 2;
      }

      // what about vert 1?
      if (currIndex < 4 || positions[currIndex - 1] != verts[1]) {
        positions.push_back(verts[1]);
        tex_coords.push_back(glm::vec2(x + 1, z) / 10.0f);
        oneindice = currIndex;
        currIndex++;
      } else {
        oneindice = currIndex - 1;
      }

      unsigned int twoindice;
      //vert2 - could be on the previous line
      if (x >= ires && positions[(currIndex - linebreak) + 2] == verts[2]) {
        twoindice = (currIndex - linebreak) + 2;
      } else if (x >= ires &&
                 positions[(currIndex - linebreak) + 3] == verts[2]) {
        twoindice = (currIndex - linebreak) + 3;
      } else {
        positions.push_back(verts[2]);
        tex_coords.push_back(glm::vec2(x, z + 1) / 10.0f);
        twoindice = currIndex;
        currIndex++;
      }

      //vert 3
      positions.push_back(verts[3]);
      tex_coords.push_back(glm::vec2(x + 1, z + 1) / 10.0f);
      unsigned int threeindice = currIndex;
      currIndex++;

      //now to setup the indices
      // Triangle 1 0-3-2
      indices.push_back(zeroindice);
      indices.push_back(threeindice);
      indices.push_back(twoindice);
      // Triangle 2 0-1-3
      indices.push_back(zeroindice);
      indices.push_back(oneindice);
      indices.push_back(threeindice);

      // Add normals and colours
      while (normals.size() < positions.size()) {
        normals.push_back(glm::vec3(0.0f, 1.0f, 0.0f));
      }
    }
  }
}


#define resolution 128

graphics_framework::geometry* DesertGen::CreateDesert()
{

    std::vector<glm::vec3> positions;
    std::vector<glm::vec3> normals;
    std::vector<glm::vec2> tex_coords;
    std::vector<GLuint> indices;

   // unsigned char* heights = new unsigned char[(resolution*resolution)];
    create_plane(resolution, resolution, 0.25f, positions, normals, tex_coords, indices);
    VerifyIndices(positions, indices);
    std::vector<glm::vec3> NEWpositions;

   // ComputeBuffers(positions, NEWpositions, indices);
  //  positions.clear();

    for (unsigned int i = 0; i < resolution; i++)
    {
        for (unsigned int j = 0; j < resolution; j++)
        {
            //positions[(i*resolution) + j].y = (unsigned char)((cos((j / resolution)* 3.141593f) + 1.0f)* 0.5f * ((float)UCHAR_MAX));
      //      positions[(i*resolution) + j].y = 4.0f;
        }
    }
   
    _geo = graphics_framework::geometry();
    // Add buffers to geometry

    _geo.add_buffer(positions, graphics_framework::BUFFER_INDEXES::POSITION_BUFFER);
    _geo.add_index_buffer(indices);
   // _geo.add_buffer(normals, graphics_framework::BUFFER_INDEXES::NORMAL_BUFFER);
   // _geo.add_buffer(tex_coords, graphics_framework::BUFFER_INDEXES::TEXTURE_COORDS_0);

    // Generate tangent and binormal data
    //graphics_framework::generate_tb(_geo, normals);
    return &_geo;
}

