// app.cpp
// Main application implementation for the pg2 project
// author: JJ

// --- Standard library headers ------------------------------------------------
#include <iostream>   // i/o streams
#include <fstream>    // file input/output (used by JSON loader)
#include <chrono>     // timing utilities
#include <stack>      // example container (unused?)
#include <random>     // random numbers
#include <string>     // std::string used in window title

// --- Third-party libraries ---------------------------------------------------

// OpenCV: conditionally include whichever installed path is available
#if __has_include(<opencv2/opencv.hpp>)
    #include <opencv2/opencv.hpp>
#elif __has_include(<opencv4/opencv2/opencv.hpp>)
    #include <opencv4/opencv2/opencv.hpp>
#else
    #error "OpenCV header files not found!"
#endif

// OpenGL Extension Wrangler (GLEW) - provides modern GL functions
#include <GL/glew.h>
// Note: using WGLEW on Windows; adjust for other platforms if necessary

// GLFW toolkit for window/context creation and input handling
#include <GLFW/glfw3.h>

// GLM: OpenGL mathematics library (commented out until needed)
// #include <glm/glm.hpp>
// #include <glm/gtc/type_ptr.hpp>

// Assets and project-specific headers
#include "assets.hpp"
#include "app.hpp"

// ImGUI: immediate-mode GUI for debug interfaces
#include <imgui.h>               // core
#include <imgui_impl_glfw.h>     // GLFW binding
#include <imgui_impl_opengl3.h>  // OpenGL3 binding

// JSON parsing library
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
        init_glfw();

        load_config("config.json");

        // -------------------------
        // GLEW INIT
        // -------------------------
        init_glew();

        // -------------------------
        // OPENGL DEBUG
        // -------------------------
        init_gl_debug();

        // -------------------------
        // VIEWPORT + BASIC STATE
        // -------------------------
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);
        glViewport(0, 0, width, height);

        glEnable(GL_DEPTH_TEST);

        // -------------------------
        // PRINT INFO
        // -------------------------
        print_glfw_info();
        print_opencv_info();
        print_glm_info();
        print_gl_info();

        std::cout << "Initialized...\n";

		// init assets (models, sounds, textures, level map, ...)
		init_assets();

		// Initialize ImGUI
		init_imgui();

		// Initialize OpenCV (if needed)
		init_opencv();

        // Task 1.3: show window after all is loaded
        glfwShowWindow(window);


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


// ---------------------------------------------------------------------------
// GUI initialization
// ---------------------------------------------------------------------------
void App::init_imgui()
{
	// see https://github.com/ocornut/imgui/wiki/Getting-Started

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init();
	std::cout << "ImGUI version: " << ImGui::GetVersion() << "\n";
}

// ---------------------------------------------------------------------------
// OpenCV support stub
// ---------------------------------------------------------------------------
void App::init_opencv()
{
	// Placeholder for any OpenCV-specific setup (e.g. allocate windows, set
	// parameters).  At the moment the application does not create any
	// OpenCV windows during initialization.
}

// ---------------------------------------------------------------------------
// GLFW initialization and window creation
// ---------------------------------------------------------------------------
void App::init_glfw(void)
{
	// register error callback before any GLFW calls
	glfwSetErrorCallback(glfw_error_callback);

	if (!glfwInit())
		throw std::runtime_error("GLFW initialization failed.");

	// OpenGL 4.6 core
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);

	// Task 1.3: hide window during initialization
	glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);

	window = glfwCreateWindow(window_width,
		window_height,
		window_title.c_str(),
		nullptr,
		nullptr);

	if (!window)
		throw std::runtime_error("Window creation failed.");

	glfwMakeContextCurrent(window);
	glfwSwapInterval(vsync_enabled ? 1 : 0);

	// Register callbacks
	glfwSetWindowUserPointer(window, this);
	glfwSetKeyCallback(window, key_callback);
	glfwSetFramebufferSizeCallback(window, fbsize_callback);
	glfwSetMouseButtonCallback(window, mouse_button_callback);
	glfwSetCursorPosCallback(window, cursor_position_callback);
	glfwSetScrollCallback(window, scroll_callback);
}

// ---------------------------------------------------------------------------
// GLEW initialization
// ---------------------------------------------------------------------------
void App::init_glew(void)
{
	// enable experimental features to get modern functionality
	glewExperimental = GL_TRUE;

	if (glewInit() != GLEW_OK)
		throw std::runtime_error("GLEW initialization failed.");

	// make sure necessary extension for Direct State Access is available
	if (!GLEW_ARB_direct_state_access)
		throw std::runtime_error("No Direct State Access support :-(");
}

// ---------------------------------------------------------------------------
// OpenGL debug output setup
// ---------------------------------------------------------------------------
void App::init_gl_debug()
{
	if (GLEW_ARB_debug_output) {
		// enable synchronous debug messages and register our callback
		glEnable(GL_DEBUG_OUTPUT);
		glDebugMessageCallback(gl_debug_callback, nullptr);
	}
}

// ---------------------------------------------------------------------------
// Logging helpers
// ---------------------------------------------------------------------------
void App::print_gl_info()
{
	std::cout << "OpenGL Vendor:   " << glGetString(GL_VENDOR) << std::endl;
	std::cout << "OpenGL Renderer: " << glGetString(GL_RENDERER) << std::endl;
	std::cout << "OpenGL Version:  " << glGetString(GL_VERSION) << std::endl;
	std::cout << "GLSL Version:    " << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;
}

void App::print_glfw_info(void)
{
	// display the version of GLFW used for diagnostics
	std::cout << "GLFW Version:    " << glfwGetVersionString() << std::endl;
}

void App::print_opencv_info()
{
	// report OpenCV version if OpenCV is in use
	std::cout << "OpenCV Version:  " << CV_VERSION << std::endl;
}

void App::print_glm_info()
{
	// GLM is currently not included, so version info is not available
	// Uncomment glm includes in app.cpp and the header if you need this
	std::cout << "GLM Version:     (not included)" << std::endl;
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
	/*
	* Typical game loop:

			// INIT: Initial positions and state
			while (application_should_not_close)
			{
				// UPDATE: Update game state
				// RENDER: Render content
				// SWAP: Swap back/front buffer
				// VSYNC: Wait for vertical retrace (e.g. 1/60 of a second)
				// POLL: Poll events, dispatch
			}
	*/
	try {
		// Setup shader program and get uniform location
		glUseProgram(shader_prog_ID);
		GLint uniform_color_location = glGetUniformLocation(shader_prog_ID, "uniform_Color");
		if (uniform_color_location == -1)
			std::cerr << "Uniform 'uniform_Color' not found.\n";

		double now = glfwGetTime();
		// FPS related
		double fps_last_displayed = now;
		int fps_counter_frames = 0;
		double FPS = 0.0;

		// animation related
		double frame_begin_timepoint = now;
		double previous_frame_render_time{};

		// Clear color saved to OpenGL state machine: no need to set repeatedly in game loop
		glClearColor(0, 0, 0, 0);

		while (!glfwWindowShouldClose(window))
		{
			// ImGui prepare render (only if required)
			if (show_imgui) {
				ImGui_ImplOpenGL3_NewFrame();
				ImGui_ImplGlfw_NewFrame();
				ImGui::NewFrame();
				//ImGui::ShowDemoWindow(); // Enable mouse when using Demo!
				ImGui::SetNextWindowPos(ImVec2(10, 10));
				ImGui::SetNextWindowSize(ImVec2(250, 100));

				ImGui::Begin("Info", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);
				ImGui::Text("V-Sync: %s", vsync_enabled ? "ON" : "OFF");
				ImGui::Text("FPS: %.1f", FPS);
				ImGui::Text("(press RMB to release mouse)");
				ImGui::Text("(hit D to show/hide info)");
				ImGui::End();
			}

			//
			// UPDATE: recompute objects state, players position etc.
		//
		now = glfwGetTime();
		// Time-based color animation
		float tri_r = (float)sin(now) * 0.5f + 0.5f;
		float tri_g = (float)cos(now) * 0.5f + 0.5f;
		float tri_b = (float)sin(now * 0.5f) * 0.5f + 0.5f;

		//
		// RENDER: GL drawCalls
		//

		// Clear OpenGL canvas, both color buffer and Z-buffer
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Set triangle color and draw
		glUniform4f(uniform_color_location, tri_r, tri_g, tri_b, 1.0f);
		glBindVertexArray(VAO_ID);
		glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(triangle_vertices.size()));
			// ImGui display
			if (show_imgui) {
				ImGui::Render();
				ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
			}

			// SWAP + VSYNC
			glfwSwapBuffers(window);

			// POLL events
			glfwPollEvents();

			// Time/FPS measurement
			now = glfwGetTime();
			previous_frame_render_time = now - frame_begin_timepoint; //compute delta_t
			frame_begin_timepoint = now; // set new start

			fps_counter_frames++;
			if (now - fps_last_displayed >= 1) {
				FPS = fps_counter_frames / (now - fps_last_displayed);
				fps_last_displayed = now;
				fps_counter_frames = 0;
				std::cout << "\r[FPS]" << FPS << "     "; // Compare: FPS with/without ImGUI
			}
		}
	}
	catch (std::exception const& e) {
		std::cerr << "App failed : " << e.what() << std::endl;
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

void App::destroy(void)
{
	// clean up ImGUI
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	// clean up OpenCV
	cv::destroyAllWindows();

	// clean-up GLFW
	if (window) {
		glfwDestroyWindow(window);
		window = nullptr;
	}
	glfwTerminate();
}

App::~App()
{
	destroy();

	std::cout << "Bye...\n";
}

// ----------------------------------------------------------------------------
// CALLBACKS IMPLEMENTATION
// ----------------------------------------------------------------------------

void App::key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	auto this_inst = static_cast<App*>(glfwGetWindowUserPointer(window));
	if ((action == GLFW_PRESS) || (action == GLFW_REPEAT)) {
		switch (key) {
		case GLFW_KEY_ESCAPE:
			// Exit The App
			glfwSetWindowShouldClose(window, GLFW_TRUE);
			break;
		case GLFW_KEY_C:
			// Task 3: Background color change
			this_inst->bg_r = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
			this_inst->bg_g = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
			this_inst->bg_b = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
			break;
		case GLFW_KEY_V:
			this_inst->vsync_enabled = !this_inst->vsync_enabled;
			glfwSwapInterval(this_inst->vsync_enabled ? 1 : 0);
			std::cout << "VSync: " << (this_inst->vsync_enabled ? "ON" : "OFF") << "\n";
			break;
		case GLFW_KEY_F:
			this_inst->toggle_fullscreen();
			break;
		case GLFW_KEY_D:
			// Toggle ImGUI display
			this_inst->show_imgui = !this_inst->show_imgui;
			break;
		case GLFW_KEY_TAB:
			// Task 1.2: capture/release mouse
			{
				int mode = glfwGetInputMode(window, GLFW_CURSOR);
				if (mode == GLFW_CURSOR_DISABLED)
					glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
				else
					glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
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
if (action == GLFW_PRESS) {
		switch (button) {
		case GLFW_MOUSE_BUTTON_LEFT: {
			int mode = glfwGetInputMode(window, GLFW_CURSOR);
			if (mode == GLFW_CURSOR_NORMAL) {
				// we are outside of application, catch the cursor
				glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
			}
			else {
				// we are already inside our game: shoot, click, etc.
				std::cout << "Bang!\n";
			}
			break;
		}
		case GLFW_MOUSE_BUTTON_RIGHT:
            // release the cursor
			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
			break;
		default:
			break;
		}
	}
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

void App::toggle_fullscreen() {
    if (!fullscreen_enabled) {
        // Switch to fullscreen
        GLFWmonitor* monitor = glfwGetPrimaryMonitor();
        const GLFWvidmode* mode = glfwGetVideoMode(monitor);

        glfwGetWindowPos(window, &saved_window_x, &saved_window_y);
        glfwGetWindowSize(window, &saved_window_width, &saved_window_height);

        glfwSetWindowMonitor(window, monitor, 0, 0, mode->width, mode->height, mode->refreshRate);
        fullscreen_enabled = true;
    } else {
        // Switch back to windowed
        glfwSetWindowMonitor(window, nullptr, saved_window_x, saved_window_y, saved_window_width, saved_window_height, 0);
        fullscreen_enabled = false;
    }
}
