// icp.cpp
// author: JJ


#pragma once

#include "assets.hpp"
#include "GLFW/glfw3.h"

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
public:
    App();

    bool init(void);
    int run(void);

    ~App();
private:
};

