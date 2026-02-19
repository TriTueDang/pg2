// icp.cpp
// author: JJ

#include <iostream>
#include <chrono>
#include <stack>
#include <random>

// #include <opencv4/opencv2/opencv.hpp>
#include <opencv2/opencv.hpp>

// OpenGL Extension Wrangler: allow all multiplatform GL functions
#include <GL/glew.h>
// WGLEW = Windows GL Extension Wrangler (change for different platform)
// platform specific functions (in this case Windows)
#include <GL/wglew.h>

// GLFW toolkit
// Uses GL calls to open GL context, i.e. GLEW __MUST__ be first.
#include <GLFW/glfw3.h>

// OpenGL math (and other additional GL libraries, at the end)
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "assets.hpp"
#include "app.hpp"


// // OpenGL debug callback (Task 1)
static void GLAPIENTRY gl_debug_callback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam) {
    if (type == GL_DEBUG_TYPE_ERROR)
        std::cerr << "GL ERROR: " << message << std::endl;
}

// // GLFW error callback (Task 2)
static void glfw_error_callback(int error, const char* description) {
    std::cerr << "GLFW Error " << error << ": " << description << std::endl;
}

App::App()
{
    // default constructor
    // nothing to do here (so far...)
    std::cout << "Constructed...\n";
}

bool App::init() {

    try {
        // -------------------------
        // GLFW INIT
        // -------------------------
        glfwSetErrorCallback(glfw_error_callback);

        if (!glfwInit())
            throw std::runtime_error("GLFW initialization failed.");

        // OpenGL 4.6 core
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);

        window = glfwCreateWindow(800, 600, "OpenGL context", nullptr, nullptr);
        if (!window)
            throw std::runtime_error("Window creation failed.");

        glfwMakeContextCurrent(window);
        glfwSwapInterval(1); // V-Sync ON

        // -------------------------
        // GLEW INIT
        // -------------------------
        glewExperimental = GL_TRUE;

        if (glewInit() != GLEW_OK)
            throw std::runtime_error("GLEW initialization failed.");

        wglewInit();

        if (!GLEW_ARB_direct_state_access)
            throw std::runtime_error("No Direct State Access support :-(");

        // -------------------------
        // OPENGL DEBUG
        // -------------------------
        if (GLEW_ARB_debug_output) {
            glEnable(GL_DEBUG_OUTPUT);
            glDebugMessageCallback(gl_debug_callback, nullptr);
        }

        // -------------------------
        // VIEWPORT + BASIC STATE
        // -------------------------
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);
        glViewport(0, 0, width, height);

        glEnable(GL_DEPTH_TEST);

        // -------------------------
        // PRINT GL INFO
        // -------------------------
        std::cout << "OpenGL Vendor:   " << glGetString(GL_VENDOR) << std::endl;
        std::cout << "OpenGL Renderer: " << glGetString(GL_RENDERER) << std::endl;
        std::cout << "OpenGL Version:  " << glGetString(GL_VERSION) << std::endl;
        std::cout << "GLSL Version:    " << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;

        std::cout << "Initialized...\n";

        init_assets();
        return true;
    }
    catch (std::exception const& e) {
        std::cerr << "Init failed: " << e.what() << std::endl;
        return false;
    }
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


int App::run() {

    GLfloat r = 0.0f, g = 0.0f, b = 1.0f, a = 1.0f;

    glUseProgram(shader_prog_ID);

    GLint uniform_color_location =
        glGetUniformLocation(shader_prog_ID, "uniform_Color");

    if (uniform_color_location == -1)
        std::cerr << "Uniform 'uniform_Color' not found.\n";

    double lastTime = glfwGetTime();
    int nbFrames = 0;

    while (!glfwWindowShouldClose(window)) {

        // -------------------------
        // FPS COUNTER
        // -------------------------
        double currentTime = glfwGetTime();
        nbFrames++;

        if (currentTime - lastTime >= 1.0) {
            double fps = double(nbFrames) / (currentTime - lastTime);
            std::string title = "OpenGL context - FPS: " +
                                std::to_string(int(fps));
            glfwSetWindowTitle(window, title.c_str());
            nbFrames = 0;
            lastTime = currentTime;
        }

        // -------------------------
        // RENDER
        // -------------------------
        glClearColor(0.1f, 0.1f, 0.15f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glUniform4f(uniform_color_location, r, g, b, a);

        glBindVertexArray(VAO_ID);
        glDrawArrays(GL_TRIANGLES, 0,
                     static_cast<GLsizei>(triangle_vertices.size()));

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwDestroyWindow(window);
    glfwTerminate();

    return EXIT_SUCCESS;
}


App::~App()
{
    // clean-up
    cv::destroyAllWindows();
    std::cout << "Bye...\n";
}
