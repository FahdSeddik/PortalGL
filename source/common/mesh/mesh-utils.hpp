#pragma once

#include "mesh.hpp"
#include <string>
#include <reactphysics3d/reactphysics3d.h>
namespace r3d = reactphysics3d;

namespace our::mesh_utils {
    // Load an ".obj" file into the mesh
    Mesh* loadOBJ(const std::string& filename);
    // Create a sphere (the vertex order in the triangles are CCW from the outside)
    // Segments define the number of divisions on the both the latitude and the longitude
    Mesh* sphere(const glm::ivec2& segments);

    // create a triangle with the given vertices
    Mesh* triangle(const r3d::Vector3& v0,const r3d::Vector3& v1,const r3d::Vector3& v2, const uint32_t& c0, const uint32_t& c1, const uint32_t& c2);

    // create line
    Mesh* line(const r3d::Vector3& v0,const r3d::Vector3& v1, const uint32_t& c0, const uint32_t& c1);
}