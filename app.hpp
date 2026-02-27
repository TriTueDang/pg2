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

class App {
private:
    bool show_imgui{true};

    int window_width = 800;
    int window_height = 600;
    std::string window_title = "OpenGL context ";
    bool vsync_enabled = true;
    //new GL stuff
    bool fullscreen_enabled = false;
    int saved_window_x = 0;
    int saved_window_y = 0;
    int saved_window_width = 800;
    int saved_window_height = 600;

    void toggle_fullscreen();

    GLFWwindow* window = nullptr;

    GLuint shader_prog_ID{ 0 };
    GLuint VBO_ID{ 0 };
    GLuint VAO_ID{ 0 };

    std::vector<vertex> triangle_vertices =
    {
    	{{0.0f,  0.5f,  0.0f}},
    	{{0.5f, -0.5f,  0.0f}},
    	{{-0.5f, -0.5f,  0.0f}}
    };

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
    static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
    static void fbsize_callback(GLFWwindow* window, int width, int height);
    static void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
    static void cursor_position_callback(GLFWwindow* window, double xpos, double ypos);
    static void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);

    void destroy(void);
    ~App();
private:
};

