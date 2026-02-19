// icp.cpp
// author: JJ

// #include <iostream>
// #include <opencv2/opencv.hpp>

// #include "app.hpp"
#include <iostream>
#include <opencv4/opencv2/opencv.hpp>

#include "app.hpp"
#include <chrono>
#include <stack>
 #include <random>
 #include "app.hpp"

// // OpenGL Extension Wrangler: allow all multiplatform GL functions
// #include <GL/glew.h>
//WGLEW = Windows GL Extension Wrangler (change for different platform)
// // platform specific functions (in this case Windows)
 // #include <GL/wglew.h>

// // GLFW toolkit
// // Uses GL calls to open GL context, i.e. GLEW __MUST__ be first.
// #include <GLFW/glfw3.h>

// // OpenGL debug callback (Task 1)
static void GLAPIENTRY gl_debug_callback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam) {
    if (type == GL_DEBUG_TYPE_ERROR)
        std::cerr << "GL ERROR: " << message << std::endl;
}

// // GLFW error callback (Task 2)
static void glfw_error_callback(int error, const char* description) {
    std::cerr << "GLFW Error " << error << ": " << description << std::endl;
}

// OpenGL math (and other additional GL libraries, at the end)
// #include <glm/glm.hpp>
// #include <glm/gtc/type_ptr.hpp>

#include "assets.hpp"
#include "app.hpp"
App::App()
{
    // default constructor
    // nothing to do here (so far...)
    std::cout << "Constructed...\n";
}

bool App::init()
{
    try {
        // initialization code
        //...

        // Task 2: GLFW error callback
        glfwSetErrorCallback(glfw_error_callback);

        // Task 1: OpenGL debug output
        if (GLEW_ARB_debug_output) {
            glEnable(GL_DEBUG_OUTPUT);
            glDebugMessageCallback(gl_debug_callback, nullptr);
        }

        // Register GLFW callbacks (minimal, empty)
        if (window) {
            glfwSetKeyCallback(window, [](GLFWwindow*, int, int, int, int){});
            glfwSetFramebufferSizeCallback(window, [](GLFWwindow*, int, int){});
            glfwSetMouseButtonCallback(window, [](GLFWwindow*, int, int, int){});
            glfwSetCursorPosCallback(window, [](GLFWwindow*, double, double){});
            glfwSetScrollCallback(window, [](GLFWwindow*, double, double){});
        }

        // some init
        // if (not_success)
        //  throw std::runtime_error("something went bad");
    }

    init_assets();
    return true;
}
void App::init_assets(void) {
    //
    // Initialize pipeline: compile, link and use shaders
    //

    //SHADERS - define & compile & link
    const char* vertex_shader =
        "#version 460 core\n"
        "in vec3 attribute_Position;"
        "void main() {"
        "  gl_Position = vec4(attribute_Position, 1.0);"
        "}";

    const char* fragment_shader =
        "#version 460 core\n"
        "uniform vec4 uniform_Color;"
        "out vec4 FragColor;"
        "void main() {"
        "  FragColor = uniform_Color;"
        "}";

    GLuint vs = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vs, 1, &vertex_shader, NULL);
    glCompileShader(vs);

    GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fs, 1, &fragment_shader, NULL);
    glCompileShader(fs);

    shader_prog_ID = glCreateProgram();
    glAttachShader(shader_prog_ID, fs);
    glAttachShader(shader_prog_ID, vs);
    glLinkProgram(shader_prog_ID);

    //now we can delete shader parts (they can be reused, if you have more shaders)
    //the final shader program already linked and stored separately
    glDetachShader(shader_prog_ID, fs);
    glDetachShader(shader_prog_ID, vs);
    glDeleteShader(vs);
    glDeleteShader(fs);

    //
    // Create and load data into GPU using OpenGL DSA (Direct State Access)
    //
vertex v;
    // Create VAO + data description (similar to container)
    glCreateVertexArrays(1, &VAO_ID);

    GLint position_attrib_location = glGetAttribLocation(shader_prog_ID, "attribute_Position");

    glEnableVertexArrayAttrib(VAO_ID, position_attrib_location);
    glVertexArrayAttribFormat(VAO_ID, position_attrib_location, v.position.length(), GL_FLOAT, GL_FALSE, offsetof(vertex, position));
    glVertexArrayAttribBinding(VAO_ID, position_attrib_location, 0); // (GLuint vaobj, GLuint attribindex, GLuint bindingindex)

    // Create and fill data
    glCreateBuffers(1, &VBO_ID);
    glNamedBufferData(VBO_ID, triangle_vertices.size()*sizeof(vertex), triangle_vertices.data(), GL_STATIC_DRAW);

    // Connect together
    glVertexArrayVertexBuffer(VAO_ID, 0, VBO_ID, 0, sizeof(vertex)); // (GLuint vaobj, GLuint bindingindex, GLuint buffer, GLintptr offset, GLsizei stride)
}


int App::run(void)
{
    try {
        double lastTime = glfwGetTime();
        int nbFrames = 0;
        while (!glfwWindowShouldClose(window)) {
            double currentTime = glfwGetTime();
            nbFrames++;
            if (currentTime - lastTime >= 1.0) {
                double fps = double(nbFrames) / (currentTime - lastTime);
                std::string title = "App - FPS: " + std::to_string(int(fps));
                glfwSetWindowTitle(window, title.c_str());
                nbFrames = 0;
                lastTime = currentTime;
            }
            glfwPollEvents();
        }
    }
    catch (std::exception const& e) {
        std::cerr << "App failed : " << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    while (!glfwWindowShouldClose(window)) {
        // clear canvas
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        //set uniform parameter for shader
        // (try to change the color in key callback)
        glUniform4f(uniform_color_location, r, g, b, a);

        //bind 3d object data
        glBindVertexArray(VAO_ID);

        // draw all VAO data
        glDrawArrays(GL_TRIANGLES, 0, triangle_vertices.size());

        // poll events, call callbacks, flip back<->front buffer
        glfwPollEvents();
        glfwSwapBuffers(window);
    }
    return EXIT_SUCCESS;
}

App::~App()
{
    // clean-up
    cv::destroyAllWindows();
    std::cout << "Bye...\n";
}
