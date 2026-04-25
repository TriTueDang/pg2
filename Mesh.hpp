#pragma once

#include <string>
#include <vector>

#include <GL/glew.h>
#include <glm/glm.hpp> 
#include <glm/ext.hpp>

#include "assets.hpp"
#include "non_copyable.hpp"
#include "Texture.hpp"
#include <memory>

    struct AABB {
        glm::vec3 min{1e10f};
        glm::vec3 max{-1e10f};
    };

class Mesh: private NonCopyable
{
public:
    AABB aabb;
    // force attribute slots in shaders for all meshes, shaders etc.
    static constexpr GLuint attribute_location_position{0};
    static constexpr GLuint attribute_location_normal{1};
    static constexpr GLuint attribute_location_texture_coords{2};

    // No default constructor. RAII - if constructed, it will be correctly initialized
    // and can be rendered. OpenGL resources are guaranteed to be deallocated using destructor. 
    // Double-free errors are prevented by making class non-copyable (therefore 
    // double destruction of the same OpenGL buffer is prevented). 
    Mesh() = delete; 
    
    // Simple mesh from vertices
    Mesh(std::vector<Vertex> const &vertices, GLenum primitive_type) : primitive_type_{primitive_type}, vertices_{vertices}
    {
        for (const auto& v : vertices_) {
            aabb.min = glm::min(aabb.min, v.Position);
            aabb.max = glm::max(aabb.max, v.Position);
        }
        // REQ: no DSA (Direct State Access) instafail => use DSA (glCreate*, glNamed*, glVertexArray*)
        glCreateVertexArrays(1, &vao_);
        glCreateBuffers(1, &vbo_);

        glNamedBufferStorage(vbo_, sizeof(Vertex) * vertices_.size(), vertices_.data(), 0);

        glVertexArrayVertexBuffer(vao_, 0, vbo_, 0, sizeof(Vertex));

        glEnableVertexArrayAttrib(vao_, attribute_location_position);
        glEnableVertexArrayAttrib(vao_, attribute_location_normal);
        glEnableVertexArrayAttrib(vao_, attribute_location_texture_coords);

        glVertexArrayAttribFormat(vao_, attribute_location_position, 3, GL_FLOAT, GL_FALSE, offsetof(Vertex, Position));
        glVertexArrayAttribFormat(vao_, attribute_location_normal, 3, GL_FLOAT, GL_FALSE, offsetof(Vertex, Normal));
        glVertexArrayAttribFormat(vao_, attribute_location_texture_coords, 2, GL_FLOAT, GL_FALSE, offsetof(Vertex, TexCoords));

        glVertexArrayAttribBinding(vao_, attribute_location_position, 0);
        glVertexArrayAttribBinding(vao_, attribute_location_normal, 0);
        glVertexArrayAttribBinding(vao_, attribute_location_texture_coords, 0);
    }
         
    // Mesh with indirect vertex addressing. Needs compiled shader for attributes setup. 
    Mesh(std::vector<Vertex> const &vertices, std::vector<GLuint> const &indices, GLenum primitive_type) :
        Mesh{vertices, primitive_type}
    {
        indices_ = indices;
        glCreateBuffers(1, &ebo_);
        glNamedBufferStorage(ebo_, sizeof(GLuint) * indices_.size(), indices_.data(), 0);
        glVertexArrayElementBuffer(vao_, ebo_);
    }    

    void draw() {
        if (texture_) {
            texture_->bind(0);
        }
        glBindVertexArray(vao_);
        
    	if (ebo_ == 0) {
    		glDrawArrays(primitive_type_, 0, (GLsizei)vertices_.size());
    	} else {
    		glDrawElements(primitive_type_, static_cast<GLsizei>(indices_.size()), GL_UNSIGNED_INT, nullptr);
    	}
    }

    void drawInstanced(GLsizei instanceCount) {
        if (texture_) {
            texture_->bind(0);
        }
        glBindVertexArray(vao_);
        
        if (ebo_ == 0) {
            glDrawArraysInstanced(primitive_type_, 0, (GLsizei)vertices_.size(), instanceCount);
        } else {
            glDrawElementsInstanced(primitive_type_, static_cast<GLsizei>(indices_.size()), GL_UNSIGNED_INT, nullptr, instanceCount);
        }
    }

    void setTexture(std::shared_ptr<Texture> texture) {
        texture_ = texture;
    }

    const std::vector<Vertex>& getVertices() const { return vertices_; }
    const std::vector<GLuint>& getIndices() const { return indices_; }

    ~Mesh() {
    	glDeleteBuffers(1, &ebo_);
    	glDeleteBuffers(1, &vbo_);
    	glDeleteVertexArrays(1, &vao_);
    };
private:
    // safe defaults
    GLenum primitive_type_{GL_POINTS}; 

    // keep the data
    std::vector<Vertex> vertices_;
    std::vector<GLuint> indices_;
    
    // OpenGL buffer IDs
    // ID = 0 is reserved (i.e. uninitalized)
    GLuint vao_{0};
    GLuint vbo_{0};
    GLuint ebo_{0};

    std::shared_ptr<Texture> texture_;
};
