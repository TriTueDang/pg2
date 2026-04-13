#pragma once

#include <vector>
#include <memory>
#include <functional>
#include <iostream>
#include <algorithm>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "Model.hpp"

namespace PG2 {

struct BVHNode {
    glm::vec3 min_bound, max_bound;
    int left = -1, right = -1;
    std::vector<Model::Triangle> triangles;
    bool is_leaf() const { return left == -1 && right == -1; }
};

class PhysicsEngine {
public:
    PhysicsEngine() = default;

    // BVH acceleration structure
    void build_bvh(const std::vector<Model::Triangle>& triangles);
    
    // Core ground and ceiling detection
    float get_ground_height(const glm::vec3& pos, float ray_depth = 500.0f) const;
    float get_ceiling_height(const glm::vec3& pos, float ray_depth = 20.0f) const;

    struct RaycastHit {
        bool hit = false;
        float distance = 1e10f;
        glm::vec3 point;
        glm::vec3 normal;
    };

    // Raycasting
    RaycastHit raycast(const glm::vec3& origin, const glm::vec3& direction, float max_dist) const;

    // Kinematic Character Controller Update
    struct KCCResult {
        glm::vec3 new_position;
        float new_velocity_y;
        bool is_on_ground;
    };

    KCCResult update_character(
        const glm::vec3& current_pos, 
        const glm::vec3& movement_delta,
        float current_velocity_y, 
        float gravity, 
        float step_height, 
        float radius,
        float delta_t
    );

    // General sphere collision resolution (sliding)
    glm::vec3 resolve_sphere_collision(const glm::vec3& pos, float radius) const;

    // Getters for debugging
    size_t get_node_count() const { return nodes.size(); }
    bool is_ready() const { return !nodes.empty(); }

private:
    std::vector<BVHNode> nodes;
    
    void traverse_bvh_ray(int node_idx, const glm::vec3& origin, const glm::vec3& direction, float& t_max, RaycastHit& best_hit) const;
    void traverse_bvh_sphere(int node_idx, const glm::vec3& pos, float radius, std::vector<const Model::Triangle*>& triangle_hits) const;
    void traverse_bvh_ground(int node_idx, const glm::vec3& pos, float ray_depth, float& max_y) const;
    void traverse_bvh_ceiling(int node_idx, const glm::vec3& pos, float ray_depth, float& min_y) const;
    int build_recursive(std::vector<Model::Triangle>& tris);

    // Helpers
    glm::vec3 closest_point_on_triangle(const glm::vec3& p, const Model::Triangle& tri) const;
    bool intersect_ray_triangle(
        const glm::vec3& orig, const glm::vec3& dir,
        const Model::Triangle& tri,
        float& t, float& u, float& v
    ) const;
};

} // namespace PG2
