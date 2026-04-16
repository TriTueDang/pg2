#pragma once

#include <filesystem>
#include <string>
#include <vector>
#include <memory> 

#include <GL/glew.h>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp> 
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/euler_angles.hpp>

#include "assets.hpp"
#include "Mesh.hpp"
#include "ShaderProgram.hpp"
#include "OBJloader.hpp"
#include "Texture.hpp"

class Model {
public:
    // origin point of whole model
    glm::vec3 pivot_position{}; // [0,0,0] of the object
    glm::vec3 eulerAngles{};    // pitch, yaw, roll
    glm::vec3 scale{1.0f};

    // mesh related data
    struct mesh_package {
        std::shared_ptr<Mesh> mesh;
        std::shared_ptr<ShaderProgram> shader;
        glm::vec3 origin;
        glm::vec3 eulerAngles;
        glm::vec3 scale;
        std::shared_ptr<Texture> texture;

        mesh_package(std::shared_ptr<Mesh> m, std::shared_ptr<ShaderProgram> s, glm::vec3 o, glm::vec3 e, glm::vec3 sc, std::shared_ptr<Texture> t = nullptr)
            : mesh(m), shader(s), origin(o), eulerAngles(e), scale(sc), texture(t) {}
    };
    std::vector<mesh_package> meshes;
    
    Model() = default;
    Model(const std::filesystem::path & filename, std::shared_ptr<ShaderProgram> shader, std::shared_ptr<Texture> texture = nullptr) {
        auto loadedMeshes = loadOBJ(filename);
        for (auto & mData : loadedMeshes) {
            auto mesh = std::make_shared<Mesh>(mData.vertices, mData.indices, GL_TRIANGLES);
            if (texture) mesh->setTexture(texture);
            addMesh(mesh, shader, glm::vec3(0.0f), glm::vec3(0.0f), glm::vec3(1.0f), texture);
        }
    }


    void addMesh(std::shared_ptr<Mesh> mesh,
                 std::shared_ptr<ShaderProgram> shader, 
                 glm::vec3 origin = glm::vec3(0.0f),      
                 glm::vec3 eulerAngles = glm::vec3(0.0f), 
                 glm::vec3 scale = glm::vec3(1.0f),
                 std::shared_ptr<Texture> texture = nullptr
                 ) {
        meshes.emplace_back(mesh, shader, origin, eulerAngles, scale, texture);
    }

    // update based on running time
    void update(const float delta_t) {
    }
    
    struct Triangle {
        glm::vec3 v0, v1, v2;
    };

    std::vector<Triangle> getTriangles() const {
        std::vector<Triangle> world_triangles;
        
        // Base model matrix
        glm::mat4 T = glm::translate(glm::mat4(1.0f), pivot_position);
        glm::mat4 R = glm::yawPitchRoll(glm::radians(eulerAngles.y), glm::radians(eulerAngles.x), glm::radians(eulerAngles.z));
        glm::mat4 S = glm::scale(glm::mat4(1.0f), scale);
        glm::mat4 model_matrix = T * R * S;

        for (auto const& mesh_pkg : meshes) {
            // Mesh-local transformation
            glm::mat4 mT = glm::translate(glm::mat4(1.0f), mesh_pkg.origin);
            glm::mat4 mR = glm::yawPitchRoll(glm::radians(mesh_pkg.eulerAngles.y), glm::radians(mesh_pkg.eulerAngles.x), glm::radians(mesh_pkg.eulerAngles.z));
            glm::mat4 mS = glm::scale(glm::mat4(1.0f), mesh_pkg.scale);
            glm::mat4 final_m = model_matrix * (mT * mR * mS);

            const auto& verts = mesh_pkg.mesh->getVertices();
            const auto& indices = mesh_pkg.mesh->getIndices();

            if (indices.empty()) {
                for (size_t i = 0; i + 2 < verts.size(); i += 3) {
                    world_triangles.push_back({
                        glm::vec3(final_m * glm::vec4(verts[i].Position, 1.0f)),
                        glm::vec3(final_m * glm::vec4(verts[i+1].Position, 1.0f)),
                        glm::vec3(final_m * glm::vec4(verts[i+2].Position, 1.0f))
                    });
                }
            } else {
                for (size_t i = 0; i + 2 < indices.size(); i += 3) {
                    world_triangles.push_back({
                        glm::vec3(final_m * glm::vec4(verts[indices[i]].Position, 1.0f)),
                        glm::vec3(final_m * glm::vec4(verts[indices[i+1]].Position, 1.0f)),
                        glm::vec3(final_m * glm::vec4(verts[indices[i+2]].Position, 1.0f))
                    });
                }
            }
        }
        return world_triangles;
    }

    struct Frustum {
        glm::vec4 planes[6]; // Left, Right, Bottom, Top, Near, Far
    };

    static bool is_aabb_in_frustum(const Frustum& f, glm::vec3 min, glm::vec3 max) {
        for (int i = 0; i < 6; i++) {
            glm::vec3 p = min;
            if (f.planes[i].x >= 0) p.x = max.x;
            if (f.planes[i].y >= 0) p.y = max.y;
            if (f.planes[i].z >= 0) p.z = max.z;
            if (glm::dot(f.planes[i], glm::vec4(p, 1.0f)) < 0) return false;
        }
        return true;
    }

    void draw(bool use_shader = true, std::shared_ptr<ShaderProgram> external_shader = nullptr, const Frustum* frustum = nullptr) {
        // Calculate base model matrix for the whole model
        glm::mat4 T = glm::translate(glm::mat4(1.0f), pivot_position);
        glm::mat4 R = glm::yawPitchRoll(glm::radians(eulerAngles.y), glm::radians(eulerAngles.x), glm::radians(eulerAngles.z));
        glm::mat4 S = glm::scale(glm::mat4(1.0f), scale);
        glm::mat4 model_matrix = T * R * S;

        // call draw() on mesh (all meshes)
        for (auto const& mesh_pkg : meshes) {
            // Calculate mesh-local transformation
            glm::mat4 mT = glm::translate(glm::mat4(1.0f), mesh_pkg.origin);
            glm::mat4 mR = glm::yawPitchRoll(glm::radians(mesh_pkg.eulerAngles.y), glm::radians(mesh_pkg.eulerAngles.x), glm::radians(mesh_pkg.eulerAngles.z));
            glm::mat4 mS = glm::scale(glm::mat4(1.0f), mesh_pkg.scale);
            glm::mat4 mesh_local_matrix = mT * mR * mS;
            glm::mat4 final_model_matrix = model_matrix * mesh_local_matrix;

            // --- PER-MESH FRUSTUM CULLING (Optimized 8-corner transformation) ---
            if (frustum) {
                glm::vec3 corners[8] = {
                    mesh_pkg.mesh->aabb.min,
                    glm::vec3(mesh_pkg.mesh->aabb.max.x, mesh_pkg.mesh->aabb.min.y, mesh_pkg.mesh->aabb.min.z),
                    glm::vec3(mesh_pkg.mesh->aabb.min.x, mesh_pkg.mesh->aabb.max.y, mesh_pkg.mesh->aabb.min.z),
                    glm::vec3(mesh_pkg.mesh->aabb.max.x, mesh_pkg.mesh->aabb.max.y, mesh_pkg.mesh->aabb.min.z),
                    glm::vec3(mesh_pkg.mesh->aabb.min.x, mesh_pkg.mesh->aabb.min.y, mesh_pkg.mesh->aabb.max.z),
                    glm::vec3(mesh_pkg.mesh->aabb.max.x, mesh_pkg.mesh->aabb.min.y, mesh_pkg.mesh->aabb.max.z),
                    glm::vec3(mesh_pkg.mesh->aabb.min.x, mesh_pkg.mesh->aabb.max.y, mesh_pkg.mesh->aabb.max.z),
                    mesh_pkg.mesh->aabb.max
                };

                glm::vec3 worldMin(1e10f), worldMax(-1e10f);
                for(int i=0; i<8; i++) {
                    glm::vec3 p = glm::vec3(final_model_matrix * glm::vec4(corners[i], 1.0f));
                    worldMin = glm::min(worldMin, p);
                    worldMax = glm::max(worldMax, p);
                }

                if (!is_aabb_in_frustum(*frustum, worldMin, worldMax)) {
                    continue; // Skip this mesh!
                }
            }

            auto active_shader = use_shader ? mesh_pkg.shader : external_shader;

            if (active_shader) {
                // Only call use() if we are using individual mesh shaders
                if (use_shader) active_shader->use();
                // Set final model matrix uniform
                active_shader->setUniform("uM_m", final_model_matrix);
                active_shader->setUniform("uUseInstancing", false);
            }

            mesh_pkg.mesh->draw();   // draw mesh
        }
    }

    void drawInstanced(GLsizei instanceCount) {
        if (instanceCount == 0) return;

        for (auto const& mesh_pkg : meshes) {
            mesh_pkg.shader->use();
            
            // Calculate mesh-local transformation
            glm::mat4 mT = glm::translate(glm::mat4(1.0f), mesh_pkg.origin);
            glm::mat4 mR = glm::yawPitchRoll(glm::radians(mesh_pkg.eulerAngles.y), glm::radians(mesh_pkg.eulerAngles.x), glm::radians(mesh_pkg.eulerAngles.z));
            glm::mat4 mS = glm::scale(glm::mat4(1.0f), mesh_pkg.scale);
            glm::mat4 mesh_local_matrix = mT * mR * mS;

            mesh_pkg.shader->setUniform("uMeshLocal", mesh_local_matrix);
            mesh_pkg.shader->setUniform("uUseInstancing", true);
            mesh_pkg.mesh->drawInstanced(instanceCount);
            mesh_pkg.shader->setUniform("uUseInstancing", false);
        }
    }

};
