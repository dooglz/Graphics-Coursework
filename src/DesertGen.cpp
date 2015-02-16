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


#define resolution 256

graphics_framework::geometry* DesertGen::CreateDesert()
{

    std::vector<glm::vec3> positions;
    std::vector<glm::vec3> normals;
    std::vector<glm::vec2> tex_coords;

   // unsigned char* heights = new unsigned char[(resolution*resolution)];
    graphics_framework::geometry_builder::create_plane_r(resolution, resolution, positions, normals, tex_coords);

    std::vector<glm::vec3> NEWpositions;
    std::vector<GLuint> indices;
    ComputeBuffers(positions, NEWpositions, indices);
    positions.clear();

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

    _geo.add_buffer(NEWpositions, graphics_framework::BUFFER_INDEXES::POSITION_BUFFER);
    _geo.add_index_buffer(indices);
   // _geo.add_buffer(normals, graphics_framework::BUFFER_INDEXES::NORMAL_BUFFER);
   // _geo.add_buffer(tex_coords, graphics_framework::BUFFER_INDEXES::TEXTURE_COORDS_0);

    // Generate tangent and binormal data
    //graphics_framework::generate_tb(_geo, normals);
    return &_geo;
}

