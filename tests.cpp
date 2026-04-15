#include "tests.hpp"
#include <iostream>
#include <fstream>
#include <filesystem>
#include <nlohmann/json.hpp>
#include "Spline.hpp"
#include <glm/glm.hpp>

namespace fs = std::filesystem;
using json = nlohmann::json;

namespace ChickenTests {

    void log_info(const std::string& message) {
        std::cout << "[TEST INFO]: " << message << std::endl;
    }

    void log_error(const std::string& message) {
        std::cerr << "[TEST ERROR]: " << message << std::endl;
    }

    bool run_all_tests() {
        bool success = true;
        
        log_info("Starting Chicken Gun Story Test Suite...");
        log_info("------------------------------------------");

        if (!test_config()) success = false;
        if (!test_assets_existence()) success = false;
        if (!test_math_splines()) success = false;
        if (!test_math_vectors()) success = false;
        if (!test_file_permissions()) success = false;
        if (!test_shader_syntax_basic()) success = false;
        if (!test_window_config()) success = false;

        log_info("------------------------------------------");
        if (success) {
            log_info("ALL TESTS PASSED SUCCESSFULLY!");
        } else {
            log_error("SOME TESTS FAILED. CHECK LOGS ABOVE.");
        }
        
        return success;
    }

    bool test_config() {
        log_info("Testing config.json...");
        if (!fs::exists("config.json")) {
            log_error("config.json not found!");
            return false;
        }

        try {
            std::ifstream f("config.json");
            json data = json::parse(f);
            if (!data.contains("window")) {
                log_error("config.json missing 'window' key!");
                return false;
            }
            log_info("config.json is valid.");
            return true;
        } catch (std::exception& e) {
            log_error(std::string("JSON Parse error: ") + e.what());
            return false;
        }
    }

    bool test_assets_existence() {
        log_info("Testing critical assets existence...");
        
        std::vector<std::string> critical_files = {
            "shader.vert", "shader.frag",
            "skybox.vert", "skybox.frag",
            "post_process.vert", "post_process.frag",
            "billboard.vert", "billboard.frag",
            "assets/chicken-gun-western-reupload/source/western.obj",
            "assets/rango/source/Rango.obj",
            "assets/tumbleweed.png"
        };

        bool all_found = true;
        for (const auto& file : critical_files) {
            if (!fs::exists(file)) {
                log_error("Missing asset: " + file);
                all_found = false;
            }
        }

        if (all_found) log_info("All critical assets found.");
        return all_found;
    }

    bool test_math_splines() {
        log_info("Testing Catmull-Rom Spline math...");
        
        std::vector<glm::vec3> points = {
            glm::vec3(0,0,0),
            glm::vec3(1,0,0),
            glm::vec3(2,0,0),
            glm::vec3(3,0,0)
        };

        try {
            PG2::CatmullRomSpline spline(points, false);
            // evaluate at mid-point of the whole spline (between points 1 and 2)
            // In this implementation, T is index + fraction [0.0, points.size()-1]
            glm::vec3 mid = spline.evaluate(1.5f);
            
            // For these linear points (0,0,0), (1,0,0), (2,0,0), (3,0,0), 
            // the point at T=1.5 should be exactly (1.5, 0, 0)
            if (glm::distance(mid, glm::vec3(1.5f, 0, 0)) > 0.1f) {
                log_error("Spline interpolation error! Expected (1.5, 0, 0), got something else.");
                return false;
            }
            log_info("Spline math is correct.");
            return true;
        } catch (...) {
            log_error("Spline test crashed!");
            return false;
        }
    }

    bool test_math_vectors() {
        log_info("Testing basic GLM math...");
        glm::vec3 a(1, 0, 0);
        glm::vec3 b(0, 1, 0);
        if (glm::dot(a, b) != 0.0f) {
            log_error("GLM Dot product error!");
            return false;
        }
        log_info("Vector math is correct.");
        return true;
    }

    bool test_file_permissions() {
        log_info("Testing file permissions (app_log.txt)...");
        std::ofstream f("app_log.txt", std::ios::app);
        if (!f.is_open()) {
            log_error("Cannot open app_log.txt for writing!");
            return false;
        }
        f << "[TEST]: Permission check at " << time(nullptr) << "\n";
        log_info("Write permission for app_log.txt confirmed.");
        return true;
    }

    bool test_shader_syntax_basic() {
        log_info("Testing basic shader consistency...");
        std::vector<std::string> shaders = {"shader.vert", "shader.frag", "skybox.vert", "skybox.frag"};
        for (const auto& s : shaders) {
            std::ifstream f(s);
            std::string content((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
            if (content.find("#version") == std::string::npos) {
                log_error("Shader " + s + " is missing #version directive!");
                return false;
            }
        }
        log_info("Basic shader structure is valid.");
        return true;
    }

    bool test_window_config() {
        log_info("Testing window configuration limits...");
        if (!fs::exists("config.json")) return false;
        std::ifstream f("config.json");
        json data = json::parse(f);
        int w = data["window"].value("width", 0);
        int h = data["window"].value("height", 0);
        if (w <= 0 || h <= 0 || w > 8000 || h > 8000) {
            log_error("Invalid window dimensions in config.json!");
            return false;
        }
        log_info("Window configuration is healthy.");
        return true;
    }
}
