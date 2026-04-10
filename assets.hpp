#pragma once

#include <GL/glew.h> 
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

//vertex description
struct Vertex {
    glm::vec3 Position;
    glm::vec3 Normal;
    glm::vec2 TexCoords;

    bool operator==(const Vertex& other) const {
        return Position == other.Position && Normal == other.Normal && TexCoords == other.TexCoords;
    }

    bool operator<(const Vertex& other) const {
        if (Position.x != other.Position.x) return Position.x < other.Position.x;
        if (Position.y != other.Position.y) return Position.y < other.Position.y;
        if (Position.z != other.Position.z) return Position.z < other.Position.z;
        if (Normal.x != other.Normal.x) return Normal.x < other.Normal.x;
        if (Normal.y != other.Normal.y) return Normal.y < other.Normal.y;
        if (Normal.z != other.Normal.z) return Normal.z < other.Normal.z;
        if (TexCoords.x != other.TexCoords.x) return TexCoords.x < other.TexCoords.x;
        return TexCoords.y < other.TexCoords.y;
    }
};

