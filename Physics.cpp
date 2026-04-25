#include "Physics.hpp"
#include "camera.hpp"

void Camera::HandleCollision(const PG2::PhysicsEngine& physics) {
    if (Mode != CameraMode::POV_LOCKED || TargetPosition == nullptr) return;

    glm::vec3 headPos = *TargetPosition + POVOffset;
    
    // Multi-ray check for robustness in tight corners (avoid clipping through thin walls/ceilings)
    // We check center, left, right, up, down rays.
    float minDistFound = desiredDistance;
    glm::vec3 checkDirs[5] = {
        -Front,
        glm::normalize(-Front + Right * 0.25f),
        glm::normalize(-Front - Right * 0.25f),
        glm::normalize(-Front + Up * 0.25f),
        glm::normalize(-Front - Up * 0.25f)
    };

    for (int i = 0; i < 5; ++i) {
        auto hit = physics.raycast(headPos, checkDirs[i], desiredDistance);
        if (hit.hit) {
            // Adjust distance based on cosine to keep it spherical-ish
            float cosAngle = glm::dot(checkDirs[i], -Front);
            float actualDist = hit.distance * cosAngle;
            if (actualDist < minDistFound) minDistFound = actualDist;
        }
    }
    
    float targetDist = glm::max(minDistance, minDistFound - 0.4f);

    // Smoothly interpolate current distance for premium feel. 
    // Increased stability to avoid "choppy" jitter in tight spaces.
    float interpolationSpeed = (targetDist < currentDistance) ? 0.22f : 0.10f;
    currentDistance = currentDistance * (1.0f - interpolationSpeed) + targetDist * interpolationSpeed;
    
    this->Position = headPos - Front * currentDistance;
}

namespace PG2 {

void PhysicsEngine::build_bvh(const std::vector<Model::Triangle>& triangles) {
    if (triangles.empty()) return;
    nodes.clear();
    std::vector<Model::Triangle> tris_copy = triangles;
    build_recursive(tris_copy);
    std::cout << "[Physics] BVH built with " << nodes.size() << " nodes.\n";
}

int PhysicsEngine::build_recursive(std::vector<Model::Triangle>& tris) {
    int node_idx = (int)nodes.size();
    nodes.push_back({});
    
    glm::vec3 min(1e10f), max(-1e10f);
    for (const auto& tri : tris) {
        min = glm::min(min, glm::min(tri.v0, glm::min(tri.v1, tri.v2)));
        max = glm::max(max, glm::max(tri.v0, glm::max(tri.v1, tri.v2)));
    }
    nodes[node_idx].min_bound = min;
    nodes[node_idx].max_bound = max;

    if (tris.size() <= 64) {
        nodes[node_idx].triangles = tris;
        return node_idx;
    }

    glm::vec3 size = max - min;
    int axis = 0;
    if (size.y > size.x && size.y > size.z) axis = 1;
    else if (size.z > size.x && size.z > size.y) axis = 2;

    // Use average centroid to avoid empty splits
    float sum_centroids = 0;
    for (const auto& tri : tris) {
        sum_centroids += (tri.v0[axis] + tri.v1[axis] + tri.v2[axis]) / 3.0f;
    }
    float split_pos = sum_centroids / tris.size();

    std::vector<Model::Triangle> left_tris, right_tris;
    for (const auto& tri : tris) {
        float centroid = (tri.v0[axis] + tri.v1[axis] + tri.v2[axis]) / 3.0f;
        if (centroid < split_pos) left_tris.push_back(tri);
        else right_tris.push_back(tri);
    }

    // Fallback if one side is still empty (should be rare with avg centroid)
    if (left_tris.empty() || right_tris.empty()) {
        // Just split half-and-half by count
        left_tris.assign(tris.begin(), tris.begin() + tris.size() / 2);
        right_tris.assign(tris.begin() + tris.size() / 2, tris.end());
    }

    int left_child = build_recursive(left_tris);
    int right_child = build_recursive(right_tris);
    nodes[node_idx].left = left_child;
    nodes[node_idx].right = right_child;
    
    return node_idx;
}

float PhysicsEngine::get_ground_height(const glm::vec3& pos, float ray_depth) const {
    if (nodes.empty()) return -1000.0f;
    float max_y = -1000.0f;
    traverse_bvh_ground(0, pos, ray_depth, max_y);
    return max_y;
}

void PhysicsEngine::traverse_bvh_ground(int node_idx, const glm::vec3& pos, float ray_depth, float& max_y) const {
    const auto& node = nodes[node_idx];
    if (pos.x < node.min_bound.x - 3.0f || pos.x > node.max_bound.x + 3.0f ||
        pos.z < node.min_bound.z - 3.0f || pos.z > node.max_bound.z + 3.0f) return;

    if (node.is_leaf()) {
        for (const auto& tri : node.triangles) {
            float t, u, v;
            float ray_lift = 6.0f; 
            glm::vec3 ray_origin(pos.x, pos.y + ray_lift, pos.z);
            if (intersect_ray_triangle(ray_origin, glm::vec3(0, -1, 0), tri, t, u, v)) {
                if (t > 0.0f && t < ray_depth + ray_lift) {
                    float hit_y = ray_origin.y - t;
                    if (hit_y > max_y) max_y = hit_y;
                }
            }
        }
    } else {
        if (node.left != -1) traverse_bvh_ground(node.left, pos, ray_depth, max_y);
        if (node.right != -1) traverse_bvh_ground(node.right, pos, ray_depth, max_y);
    }
}

float PhysicsEngine::get_ceiling_height(const glm::vec3& pos, float ray_depth) const {
    if (nodes.empty()) return 1000.0f;
    float min_y = 1000.0f;
    traverse_bvh_ceiling(0, pos, ray_depth, min_y);
    return min_y;
}

void PhysicsEngine::traverse_bvh_ceiling(int node_idx, const glm::vec3& pos, float ray_depth, float& min_y) const {
    const auto& node = nodes[node_idx];
    // Robust margins for 4x scale character
    if (pos.x < node.min_bound.x - 3.0f || pos.x > node.max_bound.x + 3.0f ||
        pos.z < node.min_bound.z - 3.0f || pos.z > node.max_bound.z + 3.0f) return;

    if (node.is_leaf()) {
        for (const auto& tri : node.triangles) {
            float t, u, v;
            // Raycast UPWARDS from pos
            if (intersect_ray_triangle(pos, glm::vec3(0, 1, 0), tri, t, u, v)) {
                if (t > 0.0f && t < ray_depth) {
                    float hit_y = pos.y + t;
                    if (hit_y < min_y) min_y = hit_y;
                }
            }
        }
    } else {
        if (node.left != -1) traverse_bvh_ceiling(node.left, pos, ray_depth, min_y);
        if (node.right != -1) traverse_bvh_ceiling(node.right, pos, ray_depth, min_y);
    }
}

PhysicsEngine::RaycastHit PhysicsEngine::raycast(const glm::vec3& origin, const glm::vec3& direction, float max_dist) const {
    RaycastHit hit;
    if (nodes.empty()) return hit;
    
    float t_max = max_dist;
    glm::vec3 dir = glm::normalize(direction);
    traverse_bvh_ray(0, origin, dir, t_max, hit);
    return hit;
}

void PhysicsEngine::traverse_bvh_ray(int node_idx, const glm::vec3& origin, const glm::vec3& direction, float& t_max, RaycastHit& best_hit) const {
    const auto& node = nodes[node_idx];
    
    // Ray-AABB intersection for BVH pruning
    float tmin = 0.0f, tmax = t_max;
    for (int i = 0; i < 3; ++i) {
        if (std::abs(direction[i]) < 1e-6f) {
            if (origin[i] < node.min_bound[i] || origin[i] > node.max_bound[i]) return;
        } else {
            float invD = 1.0f / direction[i];
            float t0 = (node.min_bound[i] - origin[i]) * invD;
            float t1 = (node.max_bound[i] - origin[i]) * invD;
            if (invD < 0.0f) std::swap(t0, t1);
            tmin = t0 > tmin ? t0 : tmin;
            tmax = t1 < tmax ? t1 : tmax;
            if (tmax <= tmin) return;
        }
    }

    if (node.is_leaf()) {
        for (const auto& tri : node.triangles) {
            float t, u, v;
            if (intersect_ray_triangle(origin, direction, tri, t, u, v)) {
                if (t > 0.0f && t < t_max) {
                    t_max = t;
                    best_hit.hit = true;
                    best_hit.distance = t;
                    best_hit.point = origin + direction * t;
                    best_hit.normal = glm::normalize(glm::cross(tri.v1 - tri.v0, tri.v2 - tri.v0));
                    // Flip normal if it's facing away from ray
                    if (glm::dot(best_hit.normal, direction) > 0) best_hit.normal = -best_hit.normal;
                }
            }
        }
    } else {
        // Simple order (could be improved by checking which child is closer)
        if (node.left != -1) traverse_bvh_ray(node.left, origin, direction, t_max, best_hit);
        if (node.right != -1) traverse_bvh_ray(node.right, origin, direction, t_max, best_hit);
    }
}

bool PhysicsEngine::intersect_ray_triangle(const glm::vec3& orig, const glm::vec3& dir, const Model::Triangle& tri, float& t, float& u, float& v) const {
    glm::vec3 edge1 = tri.v1 - tri.v0;
    glm::vec3 edge2 = tri.v2 - tri.v0;
    glm::vec3 pvec = glm::cross(dir, edge2);
    float det = glm::dot(edge1, pvec);
    if (std::abs(det) < 1e-8f) return false;
    float invDet = 1.0f / det;
    glm::vec3 tvec = orig - tri.v0;
    u = glm::dot(tvec, pvec) * invDet;
    if (u < 0.0f || u > 1.0f) return false;
    glm::vec3 qvec = glm::cross(tvec, edge1);
    v = glm::dot(dir, qvec) * invDet;
    if (v < 0.0f || u + v > 1.0f) return false;
    t = glm::dot(edge2, qvec) * invDet;
    return true;
}

glm::vec3 PhysicsEngine::resolve_sphere_collision(const glm::vec3& pos, float radius) const {
    if (nodes.empty()) return pos;
    
    glm::vec3 current_pos = pos;
    // Sequential resolution is much better for corners and tight spaces
    for (int iter = 0; iter < 4; ++iter) {
        std::vector<const Model::Triangle*> overlaps;
        traverse_bvh_sphere(0, current_pos, radius, overlaps);
        if (overlaps.empty()) break;
        
        bool resolved = false;
        for (const auto* tri : overlaps) {
            glm::vec3 p_closest = closest_point_on_triangle(current_pos, *tri);
            glm::vec3 delta = current_pos - p_closest;
            float dist_sq = glm::dot(delta, delta);
            
            if (dist_sq < radius * radius && dist_sq > 1e-8f) {
                float dist = std::sqrt(dist_sq);
                glm::vec3 n = delta / dist;
                
                if (std::abs(n.y) < 0.7f) {
                    current_pos += n * (radius - dist) * 1.05f; // Slight push-out
                    resolved = true;
                }
            }
        }
        if (!resolved) break;
    }
    return current_pos;
}

glm::vec3 PhysicsEngine::closest_point_on_triangle(const glm::vec3& p, const Model::Triangle& tri) const {
    glm::vec3 edge0 = tri.v1 - tri.v0;
    glm::vec3 edge1 = tri.v2 - tri.v0;
    glm::vec3 v0 = tri.v0 - p;

    float a = glm::dot(edge0, edge0);
    float b = glm::dot(edge0, edge1);
    float c = glm::dot(edge1, edge1);
    float d = glm::dot(edge0, v0);
    float e = glm::dot(edge1, v0);

    float det = a * c - b * b;
    float s = b * e - c * d;
    float t = b * d - a * e;

    if (s + t <= det) {
        if (s < 0.0f) {
            if (t < 0.0f) {
                if (d < 0.0f) {
                    s = glm::clamp(-d / a, 0.0f, 1.0f);
                    t = 0.0f;
                } else {
                    s = 0.0f;
                    t = glm::clamp(-e / c, 0.0f, 1.0f);
                }
            } else {
                s = 0.0f;
                t = glm::clamp(-e / c, 0.0f, 1.0f);
            }
        } else if (t < 0.0f) {
            s = glm::clamp(-d / a, 0.0f, 1.0f);
            t = 0.0f;
        } else {
            float invDet = 1.0f / det;
            s *= invDet;
            t *= invDet;
        }
    } else {
        if (s < 0.0f) {
            float tmp0 = b + d;
            float tmp1 = c + e;
            if (tmp1 > tmp0) {
                float numer = tmp1 - tmp0;
                float denom = a - 2.0f * b + c;
                s = glm::clamp(numer / denom, 0.0f, 1.0f);
                t = 1.0f - s;
            } else {
                t = glm::clamp(-e / c, 0.0f, 1.0f);
                s = 0.0f;
            }
        } else if (t < 0.0f) {
            if (a + d > b + e) {
                float numer = c + e - b - d;
                float denom = a - 2.0f * b + c;
                s = glm::clamp(numer / denom, 0.0f, 1.0f);
                t = 1.0f - s;
            } else {
                s = glm::clamp(-d / a, 0.0f, 1.0f);
                t = 0.0f;
            }
        } else {
            float numer = c + e - b - d;
            float denom = a - 2.0f * b + c;
            s = glm::clamp(numer / denom, 0.0f, 1.0f);
            t = 1.0f - s;
        }
    }

    return tri.v0 + s * edge0 + t * edge1;
}

void PhysicsEngine::traverse_bvh_sphere(int node_idx, const glm::vec3& pos, float radius, std::vector<const Model::Triangle*>& triangle_hits) const {
    const auto& node = nodes[node_idx];
    
    if (pos.x < node.min_bound.x - radius - 2.0f || pos.x > node.max_bound.x + radius + 2.0f ||
        pos.z < node.min_bound.z - radius - 2.0f || pos.z > node.max_bound.z + radius + 2.0f ||
        pos.y < node.min_bound.y - radius - 10.0f || pos.y > node.max_bound.y + radius + 10.0f) return;

    if (node.is_leaf()) {
        for (const auto& tri : node.triangles) {
            triangle_hits.push_back(&tri);
        }
    } else {
        if (node.left != -1) traverse_bvh_sphere(node.left, pos, radius, triangle_hits);
        if (node.right != -1) traverse_bvh_sphere(node.right, pos, radius, triangle_hits);
    }
}


PhysicsEngine::KCCResult PhysicsEngine::update_character(
    const glm::vec3& current_pos, 
    const glm::vec3& movement_delta,
    float current_velocity_y, 
    float gravity, 
    float step_height, 
    float radius,
    float delta_t
) {
    KCCResult res;
    res.new_velocity_y = current_velocity_y + gravity * delta_t;
    
    // --- Step 1: Horizontal Movement with Step-Up support ---
    // Reduced lift to 1.0f for better ceiling clearance in buildings
    float collider_lift = 1.0f;
    glm::vec3 desired_pos = current_pos + movement_delta;
    glm::vec3 collision_center = desired_pos + glm::vec3(0, collider_lift, 0);
    glm::vec3 resolved_center = resolve_sphere_collision(collision_center, radius);
    glm::vec3 horizontal_pos = resolved_center - glm::vec3(0, collider_lift, 0);
    
    float dist_slid = glm::distance(glm::vec2(current_pos.x, current_pos.z), glm::vec2(horizontal_pos.x, horizontal_pos.z));
    float dist_wanted = glm::distance(glm::vec2(current_pos.x, current_pos.z), glm::vec2(desired_pos.x, desired_pos.z));
    
    if (dist_slid < dist_wanted * 0.9f && dist_wanted > 0.001f) {
        // Step-Up: use a safer height that doesn't trigger ceiling collision
        glm::vec3 lifted_start = current_pos + glm::vec3(0, step_height, 0);
        glm::vec3 lifted_end = lifted_start + movement_delta;
        
        glm::vec3 stepped_center = lifted_end + glm::vec3(0, collider_lift, 0);
        glm::vec3 resolved_stepped = resolve_sphere_collision(stepped_center, radius);
        glm::vec3 stepped_horizontal = resolved_stepped - glm::vec3(0, collider_lift, 0);
        
        // Search deep for the ground (multi-story buildings)
        float ground_at_step = get_ground_height(stepped_horizontal, 25.0f);
        
        if (ground_at_step > -500.0f && ground_at_step <= current_pos.y + step_height + 0.5f) {
            horizontal_pos = stepped_horizontal;
            horizontal_pos.y = ground_at_step;
        }
    }
    
    res.new_position = horizontal_pos;
    
    // --- Step 2: Vertical movement ---
    res.new_position.y += res.new_velocity_y * delta_t;
    
    // --- Step 3: Ceiling Handling ---
    float head_y = res.new_position.y + 7.5f; 
    float ceiling = get_ceiling_height(res.new_position + glm::vec3(0, 2.0f, 0), 10.0f);
    if (head_y > ceiling - 0.2f) {
        float snapped_y = ceiling - 7.7f;
        if (snapped_y > current_pos.y - 4.0f) { // Allow more compression but prevent clipping
            res.new_position.y = snapped_y;
            if (res.new_velocity_y > 0) res.new_velocity_y = 0; 
        }
    }

    // --- Step 4: Ground and Snap-to-floor ---
    // Use a large search depth (50.0) to catch falls, but the ray now starts 
    // at waist height (handled inside traverse_bvh_ground), so it ignores roofs.
    float ground_reach = 50.0f;
    float target_ground = get_ground_height(res.new_position, ground_reach);
    
    // Multi-sample the ground around the feet for better stability on edges
    float probe_radius = radius * 0.5f;
    float sample_offsets[] = { probe_radius, -probe_radius };
    for (float ox : sample_offsets) {
        for (float oz : sample_offsets) {
            float g = get_ground_height(res.new_position + glm::vec3(ox, 0, oz), ground_reach);
            if (g > -500.0f && g > target_ground) target_ground = g;
        }
    }

    // SNAP LOGIC: Catch anything within reach BELOW the waist-start
    bool is_jumping = res.new_velocity_y > 0.01f;

    if (target_ground > -500.0f && res.new_position.y <= target_ground + 1.0f) {
        if (!is_jumping) {
            // Only snap if we are falling OR if the ground is within step height (walking up)
            res.new_position.y = target_ground;
            res.new_velocity_y = 0.0f;
            res.is_on_ground = true;
        } else {
            // We are jumping up! Do NOT snap to ground and cancel the jump.
            res.is_on_ground = false;
        }
    } else {
        res.is_on_ground = false;
    }

    return res;
}

} // namespace PG2
