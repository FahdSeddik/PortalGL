#pragma once

#include "mesh.hpp"
#include <string>
#include <reactphysics3d/reactphysics3d.h>
namespace r3d = reactphysics3d;

namespace portal::mesh_utils {
    // Load an ".obj" file into the mesh
    Mesh* loadOBJ(const std::string& filename);
    // Create a sphere (the vertex order in the triangles are CCW from the outside)
    // Segments define the number of divisions on the both the latitude and the longitude
    Mesh* sphere(const glm::ivec2& segments);

    // This will only load the data from the obj file into the vectors
    // without creating a mesh object
    std::pair<std::vector<portal::Vertex>*, std::vector<GLuint>*> loadOBJData(const std::string& filename);
}