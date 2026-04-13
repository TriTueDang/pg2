#pragma once

#include <vector>
#include <filesystem>
#include <GL/glew.h>

#include "assets.hpp"

struct MeshData {
    std::string name;
    std::vector<Vertex> vertices;
    std::vector<GLuint> indices;
};

std::vector<MeshData> loadOBJ(const std::filesystem::path& filename);

