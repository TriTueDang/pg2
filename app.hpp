// icp.cpp
// author: JJ


#pragma once

#include <vector>
#include "GLFW/glfw3.h"
#include "assets.hpp"

class App {
private:

    //new GL stuff
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

public:
    App();

    bool init(void);
    void init_assets(void);
    int run(void);

    // Callbacks
    static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
    static void fbsize_callback(GLFWwindow* window, int width, int height);
    static void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
    static void cursor_position_callback(GLFWwindow* window, double xpos, double ypos);
    static void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);

    ~App();
private:
};

