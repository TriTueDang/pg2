#pragma once

#include <filesystem>
#include <string>
#include <vector>
#include <memory> 

#include <GL/glew.h>
#include <glm/glm.hpp> 

#include "assets.hpp"
#include "Mesh.hpp"
#include "ShaderProgram.hpp"
#include "OBJloader.hpp"

class Model {
public:
    // origin point of whole model
    glm::vec3 pivot_position{}; // [0,0,0] of the object
    glm::vec3 eulerAngles{};    // pitch, yaw, roll
    glm::vec3 scale{1.0f};

    // mesh related data
    struct mesh_package {
        std::shared_ptr<Mesh> mesh;         // geometry & topology, vertex attributes
        std::shared_ptr<ShaderProgram> shader;     // which shader to use to draw this part of the model
        
        glm::vec3 origin;                   // mesh origin relative to origin of the whole model
        glm::vec3 eulerAngles;              // mesh rotation relative to orientation of the whole model
        glm::vec3 scale;                    // mesh scale relative to scale of the whole model

        mesh_package(std::shared_ptr<Mesh> m, std::shared_ptr<ShaderProgram> s, glm::vec3 o, glm::vec3 e, glm::vec3 sc)
            : mesh(m), shader(s), origin(o), eulerAngles(e), scale(sc) {}
    };
    std::vector<mesh_package> meshes;
    
    Model() = default;
    Model(const std::filesystem::path & filename, std::shared_ptr<ShaderProgram> shader) {
        std::vector<Vertex> vertices;
        std::vector<GLuint> indices;
        if (loadOBJ(filename, vertices, indices)) {
            auto mesh = std::make_shared<Mesh>(vertices, indices, GL_TRIANGLES);
            addMesh(mesh, shader);
        }
    }

    void addMesh(std::shared_ptr<Mesh> mesh,
                 std::shared_ptr<ShaderProgram> shader, 
                 glm::vec3 origin = glm::vec3(0.0f),      
                 glm::vec3 eulerAngles = glm::vec3(0.0f), 
                 glm::vec3 scale = glm::vec3(1.0f)       
                 ) {
        meshes.emplace_back(mesh, shader, origin, eulerAngles, scale);
    }

    // update based on running time
    void update(const float delta_t) {
    }
    
    void draw() {
        // call draw() on mesh (all meshes)
        for (auto const& mesh_pkg : meshes) {
            mesh_pkg.shader->use(); // select proper shader
            mesh_pkg.mesh->draw();   // draw mesh
        }
    }
};
