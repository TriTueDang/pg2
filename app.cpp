// icp.cpp
// author: JJ

#include <iostream>
#include <chrono>
#include <stack>
#include <random>

#if __has_include(<opencv2/opencv.hpp>)
    #include <opencv2/opencv.hpp>
#elif __has_include(<opencv4/opencv2/opencv.hpp>)
    #include <opencv4/opencv2/opencv.hpp>
#else
    #error "OpenCV hlavičkové soubory nebyly nalezeny!"
#endif

// OpenGL Extension Wrangler: allow all multiplatform GL functions
#include <GL/glew.h>
// WGLEW = Windows GL Extension Wrangler (change for different platform)
// platform specific functions (in this case Windows)

// GLFW toolkit
// Uses GL calls to open GL context, i.e. GLEW __MUST__ be first.
#include <GLFW/glfw3.h>

// OpenGL math (and other additional GL libraries, at the end)
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "assets.hpp"
#include "app.hpp"

// json
#include <fstream>
#include <nlohmann/json.hpp>
using json = nlohmann::json;

// OpenGL debug callback (Task 1) - from lecture
static void GLAPIENTRY gl_debug_callback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam) {
    auto const src_str = [source]() {
        switch (source) {
        case GL_DEBUG_SOURCE_API: return "API";
        case GL_DEBUG_SOURCE_WINDOW_SYSTEM: return "WINDOW SYSTEM";
        case GL_DEBUG_SOURCE_SHADER_COMPILER: return "SHADER COMPILER";
        case GL_DEBUG_SOURCE_THIRD_PARTY: return "THIRD PARTY";
        case GL_DEBUG_SOURCE_APPLICATION: return "APPLICATION";
        case GL_DEBUG_SOURCE_OTHER: return "OTHER";
        default: return "Unknown";
        }
    }();

    auto const type_str = [type]() {
        switch (type) {
        case GL_DEBUG_TYPE_ERROR: return "ERROR";
        case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: return "DEPRECATED_BEHAVIOR";
        case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR: return "UNDEFINED_BEHAVIOR";
        case GL_DEBUG_TYPE_PORTABILITY: return "PORTABILITY";
        case GL_DEBUG_TYPE_PERFORMANCE: return "PERFORMANCE";
        case GL_DEBUG_TYPE_MARKER: return "MARKER";
        case GL_DEBUG_TYPE_OTHER: return "OTHER";
        default: return "Unknown";
        }
    }();

    auto const severity_str = [severity]() {
        switch (severity) {
        case GL_DEBUG_SEVERITY_NOTIFICATION: return "NOTIFICATION";
        case GL_DEBUG_SEVERITY_LOW: return "LOW";
        case GL_DEBUG_SEVERITY_MEDIUM: return "MEDIUM";
        case GL_DEBUG_SEVERITY_HIGH: return "HIGH";
        default: return "Unknown";
        }
    }();

    std::cout << "[GL CALLBACK]: " <<
        "source = " << src_str <<
        ", type = " << type_str <<
        ", severity = " << severity_str <<
        ", ID = '" << id << '\'' <<
        ", message = '" << message << '\'' << std::endl;
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

        load_config("config.json");
        window = glfwCreateWindow(window_width,
                          window_height,
                          window_title.c_str(),
                          nullptr,
                          nullptr);
        if (!window)
            throw std::runtime_error("Window creation failed.");

        glfwMakeContextCurrent(window);
        glfwSwapInterval(vsync_enabled ? 1 : 0);

        // Register additional callbacks (Task 3)
        glfwSetWindowUserPointer(window, this);
        glfwSetKeyCallback(window, key_callback);
        glfwSetFramebufferSizeCallback(window, fbsize_callback);
        glfwSetMouseButtonCallback(window, mouse_button_callback);
        glfwSetCursorPosCallback(window, cursor_position_callback);
        glfwSetScrollCallback(window, scroll_callback);

        // -------------------------
        // GLEW INIT
        // -------------------------
        glewExperimental = GL_TRUE;

        if (glewInit() != GLEW_OK)
            throw std::runtime_error("GLEW initialization failed.");

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
bool App::load_config(const std::string& filename)
{
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Could not open config file. Using defaults.\n";
        return false;
    }

    json config;
    file >> config;

    if (config.contains("window")) {
        window_width  = config["window"].value("width", 800);
        window_height = config["window"].value("height", 600);
        window_title  = config["window"].value("title", "OpenGL context");
    }

    vsync_enabled = config.value("vsync", true);

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


int App::run() {

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
                    std::to_string(int(fps)) +
                    " - VSync: " +
                    (vsync_enabled ? "ON" : "OFF");
            glfwSetWindowTitle(window, title.c_str());
            nbFrames = 0;
            lastTime = currentTime;
        }

        // -------------------------
        // TIME-BASED COLOR (Task 3)
        // -------------------------
        tri_r = (float)sin(currentTime) * 0.5f + 0.5f;
        tri_g = (float)cos(currentTime) * 0.5f + 0.5f;
        tri_b = (float)sin(currentTime * 0.5f) * 0.5f + 0.5f;

        // -------------------------
        // RENDER
        // -------------------------
        glClearColor(bg_r, bg_g, bg_b, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glUniform4f(uniform_color_location, tri_r, tri_g, tri_b, 1.0f);

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

// ----------------------------------------------------------------------------
// CALLBACKS IMPLEMENTATION
// ----------------------------------------------------------------------------

void App::key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    App* app = static_cast<App*>(glfwGetWindowUserPointer(window));

    if ((action == GLFW_PRESS) || (action == GLFW_REPEAT)) {
        switch (key) {
        case GLFW_KEY_ESCAPE:
            glfwSetWindowShouldClose(window, GLFW_TRUE);
            break;
        case GLFW_KEY_C:
            // Task 3: Background color change
            app->bg_r = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
            app->bg_g = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
            app->bg_b = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
            break;
        case GLFW_KEY_V:
            app->vsync_enabled = !app->vsync_enabled;

            if (app->vsync_enabled) {
                glfwSwapInterval(1);   // Enable VSync
            } else {
                glfwSwapInterval(0);   // Disable VSync
            }
            break;

        default:
            break;
        }
    }
}

void App::fbsize_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

void App::mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
    if (action == GLFW_PRESS)
        std::cout << "Mouse button pressed: " << button << std::endl;
}

void App::cursor_position_callback(GLFWwindow* window, double xpos, double ypos) {
    // Optional: Log or use for interaction
}

void App::scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    if (yoffset > 0.0) {
        std::cout << "wheel up...\n";
    } else if (yoffset < 0.0) {
        std::cout << "wheel down...\n";
    }
}
