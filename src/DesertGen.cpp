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



  for (float x = 0; x < width; x += ires) {
    for (float z = 0; z < depth; z += ires) {
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

      // Triangle 1
      positions.push_back(verts[0]);
      tex_coords.push_back(glm::vec2(x, z) / 10.0f);
      positions.push_back(verts[3]);
      tex_coords.push_back(glm::vec2(x + 1, z + 1) / 10.0f);
      positions.push_back(verts[2]);
      tex_coords.push_back(glm::vec2(x, z + 1) / 10.0f);

      // Triangle 2
      positions.push_back(verts[0]);
      tex_coords.push_back(glm::vec2(x, z) / 10.0f);
      positions.push_back(verts[1]);
      tex_coords.push_back(glm::vec2(x + 1, z) / 10.0f);
      positions.push_back(verts[3]);
      tex_coords.push_back(glm::vec2(x + 1, z + 1) / 10.0f);


      // Add normals and colours
      for (unsigned int i = 0; i < 6; ++i) {
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

    std::vector<glm::vec3> NEWpositions;

    ComputeBuffers(positions, NEWpositions, indices);
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

