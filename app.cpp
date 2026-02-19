// icp.cpp
// author: JJ

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
 #include <GL/wglew.h>

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
    catch (std::exception const& e) {
        std::cerr << "Init failed : " << e.what() << std::endl;
        throw;
    }
    std::cout << "Initialized...\n";
    return true;
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

    std::cout << "Finished OK...\n";
    return EXIT_SUCCESS;
}

App::~App()
{
    // clean-up
    cv::destroyAllWindows();
    std::cout << "Bye...\n";
}
