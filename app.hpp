// app.hpp
// Public interface for the application class
// author: JJ

#pragma once

// Standard headers
#include <vector>   // dynamic arrays used by vertex list
#include <string>   // std::string for window title

// Third-party headers
#include "GLFW/glfw3.h"   // GLFW types in callbacks

// Project headers
#include "assets.hpp"     // vertex definition, assets management
#include "ShaderProgram.hpp"
#include "Model.hpp"
#include <memory>

#include "camera.hpp"
class App {

protected:
    // projection related variables
    int width{0}, height{0};
    float fov = 60.0f;
    // store projection matrix here, update only on callbacks
    glm::mat4 projection_matrix = glm::identity<glm::mat4>();
    // all objects of the scene
    std::unordered_map<std::string, Model> scene;

    // Mouse control (Task 2, point 4)
    glm::vec3 camera_front{ 0.0f, 0.0f, -1.0f };
    float yaw = -90.0f;
    float pitch = 0.0f;
    float lastX = 400, lastY = 300;
    bool firstMouse = true;

    glm::mat4 view_matrix = glm::identity<glm::mat4>();

    Camera camera;
    // remember last cursor position, move relative to that in the next frame
    double cursorLastX{ 0 };
    double cursorLastY{ 0 };


private:
    bool show_imgui{true};

    int window_width = 800;
    int window_height = 600;
    std::string window_title = "OpenGL context ";
    bool is_vsync_on = true;
    //new GL stuff
    bool fullscreen_enabled = false;
    int saved_window_x = 0;
    int saved_window_y = 0;
    int saved_window_width = 800;
    int saved_window_height = 600;

    void toggle_fullscreen();

    GLFWwindow* window = nullptr;

    std::shared_ptr<ShaderProgram> shader_prog;
    std::shared_ptr<Model> model;

    // Application state
    float bg_r = 0.1f, bg_g = 0.1f, bg_b = 0.15f;
    float tri_r = 0.0f, tri_g = 0.0f, tri_b = 1.0f;

    // initialization helpers
    void init_imgui(void);          // set up ImGUI context and bindings
    void init_opencv(void);        // placeholder for OpenCV setup
    void init_glfw(void);          // initialize GLFW and create window
    void init_glew(void);          // initialize GLEW and check extensions
    void init_gl_debug(void);      // enable OpenGL debug callbacks

    // information display routines
    void print_opencv_info(void);  // log OpenCV version
    void print_glfw_info(void);    // log GLFW version
    void print_glm_info(void);     // log GLM version (if available)
    void print_gl_info(void);      // log GPU/GL driver info

public:
    App();

    bool init(void);
    void init_assets(void);
    bool load_config(const std::string& filename);
    int run(void);

    // Callbacks
    static void glfw_key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
    static void glfw_fbsize_callback(GLFWwindow* window, int width, int height);
    static void glfw_mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
    static void glfw_cursor_position_callback(GLFWwindow* window, double xpos, double ypos);
    static void cursorPositionCallback(GLFWwindow* window, double xpos, double ypos);
    static void glfw_scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
    void update_projection_matrix(void);

    void destroy(void);
    ~App();
private:
};

