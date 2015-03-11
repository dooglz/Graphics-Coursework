/* Main.cpp
* Utilities for handeling Mesh arrays.
*
* Sam Serrels, Computer Graphics, 2015
*/
#pragma once
#include <vector>
#include <glm\glm.hpp>

// Takes an ordered list of verts, returns a list of indices and verts
void IndexVerticies(std::vector<glm::vec3> &Vinput, std::vector<glm::vec3> &Vout,
                    std::vector<unsigned int> &indices);

// Verifies the indices and vertices match. If fix, will attempt to fix errors
void VerifyIndices(std::vector<glm::vec3> &verts, std::vector<glm::vec3> *normals,
                   std::vector<glm::vec2> *tex_coords, std::vector<unsigned int> &indices,
                   const bool fix);

void CalculateNormals(std::vector<glm::vec3> &verts, std::vector<glm::vec3> &normals,
                      std::vector<unsigned int> &indices);
