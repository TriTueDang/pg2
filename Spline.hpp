#pragma once

#include <vector>
#include <glm/glm.hpp>

namespace PG2 {

class CatmullRomSpline {
public:
    std::vector<glm::vec3> points;
    bool cyclic = false;

    CatmullRomSpline() = default;
    CatmullRomSpline(const std::vector<glm::vec3>& p, bool is_cyclic = false) 
        : points(p), cyclic(is_cyclic) {}

    glm::vec3 evaluate(float T) const {
        if (points.size() < 4) return points.empty() ? glm::vec3(0) : points[0];

        int n = static_cast<int>(points.size());
        int i = static_cast<int>(T);
        float t = T - i;

        int i0, i1, i2, i3;
        getIndices(i, n, i0, i1, i2, i3);

        return interpolate(points[i0], points[i1], points[i2], points[i3], t);
    }

    glm::vec3 evaluateTangent(float T) const {
        if (points.size() < 4) return glm::vec3(0, 0, 1);

        int n = static_cast<int>(points.size());
        int i = static_cast<int>(T);
        float t = T - i;

        int i0, i1, i2, i3;
        getIndices(i, n, i0, i1, i2, i3);

        return interpolateTangent(points[i0], points[i1], points[i2], points[i3], t);
    }

    float getMaxT() const {
        if (points.size() < 2) return 0.0f;
        return cyclic ? (float)points.size() : (float)points.size() - 1.0f;
    }

private:
    void getIndices(int i, int n, int& i0, int& i1, int& i2, int& i3) const {
        if (cyclic) {
            i0 = (i - 1 + n) % n;
            i1 = i % n;
            i2 = (i + 1) % n;
            i3 = (i + 2) % n;
        } else {
            i0 = glm::clamp(i - 1, 0, n - 1);
            i1 = glm::clamp(i, 0, n - 1);
            i2 = glm::clamp(i + 1, 0, n - 1);
            i3 = glm::clamp(i + 2, 0, n - 1);
        }
    }

    glm::vec3 interpolate(const glm::vec3& p0, const glm::vec3& p1, const glm::vec3& p2, const glm::vec3& p3, float t) const {
        float t2 = t * t;
        float t3 = t2 * t;

        return 0.5f * (
            2.0f * p1 +
            (-p0 + p2) * t +
            (2.0f * p0 - 5.0f * p1 + 4.0f * p2 - p3) * t2 +
            (-p0 + 3.0f * p1 - 3.0f * p2 + p3) * t3
        );
    }

    glm::vec3 interpolateTangent(const glm::vec3& p0, const glm::vec3& p1, const glm::vec3& p2, const glm::vec3& p3, float t) const {
        float t2 = t * t;

        // Derivative of Catmull-Rom:
        // P'(t) = 0.5 * [ (-p0 + p2) + (4p0 - 10p1 + 8p2 - 2p3)t + (-3p0 + 9p1 - 9p2 + 3p3)t^2 ]
        // Simplified:
        return 0.5f * (
            (-p0 + p2) +
            (2.0f * (2.0f * p0 - 5.0f * p1 + 4.0f * p2 - p3)) * t +
            (3.0f * (-p0 + 3.0f * p1 - 3.0f * p2 + p3)) * t2
        );
    }
};

} // namespace PG2
