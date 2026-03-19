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
        glDisable(GL_CULL_FACE); // Ensure triangle is visible from both sides

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

    is_vsync_on = config.value("vsync", true);

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
	glfwSwapInterval(is_vsync_on ? 1 : 0);

	// Task 1.2: initial mouse capture
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	// Register callbacks
	glfwSetWindowUserPointer(window, this);
	glfwSetKeyCallback(window, glfw_key_callback);
	glfwSetFramebufferSizeCallback(window, glfw_fbsize_callback);
	glfwSetMouseButtonCallback(window, glfw_mouse_button_callback);
	glfwSetCursorPosCallback(window, glfw_cursor_position_callback);
	glfwSetScrollCallback(window, glfw_scroll_callback);
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
    shader_prog = ShaderProgram::from_files("shader.vert", "shader.frag");
    model = std::make_shared<Model>("cube_triangles.obj", shader_prog);
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
        shader_prog->use();

        double now = glfwGetTime();
        // FPS related
        double fps_last_displayed = now;
        int fps_counter_frames = 0;
        double FPS = 0.0;

        // animation related
        double frame_begin_timepoint = now;
        double previous_frame_render_time = 0.0; // This will act as our delta_t

        // Clear color saved to OpenGL state machine
        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

        // --- NEW INIT CODE ---
        glCullFace(GL_BACK);
        glEnable(GL_CULL_FACE);

        // disable cursor, so that it can not leave window, and we can process movement
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        // get first position of mouse cursor
        glfwGetCursorPos(window, &cursorLastX, &cursorLastY);

        update_projection_matrix();
        glViewport(0, 0, width, height);

        camera.Position = glm::vec3(0, 0, 1000); // Assuming the attribute is capitalized based on your previous Camera class
        // ---------------------

        while (!glfwWindowShouldClose(window))
        {
            // Clear OpenGL canvas, both color buffer and Z-buffer
            // Moved this to the top of the loop where it belongs
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            // ImGui prepare render (only if required)
            if (show_imgui) {
                ImGui_ImplOpenGL3_NewFrame();
                ImGui_ImplGlfw_NewFrame();
                ImGui::NewFrame();
                //ImGui::ShowDemoWindow(); // Enable mouse when using Demo!
                ImGui::SetNextWindowPos(ImVec2(10, 10));
                ImGui::SetNextWindowSize(ImVec2(250, 100));

                ImGui::Begin("Info", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);
                ImGui::Text("V-Sync: %s", is_vsync_on ? "ON" : "OFF");
                ImGui::Text("FPS: %.1f", FPS);
                ImGui::Text("(press RMB to release mouse)");
                ImGui::Text("(hit D to show/hide info)");
                ImGui::End();
            }

            //
            // UPDATE: recompute objects state, players position etc.
            //
            now = glfwGetTime();

            // --- NEW CAMERA UPDATE ---
            // Process keys etc. using previous_frame_render_time as delta_t
            camera.ProcessInput(window, (GLfloat)previous_frame_render_time);
            // -------------------------

            // Time-based color animation
            float tri_r = (float)sin(now) * 0.5f + 0.5f;
            float tri_g = (float)cos(now) * 0.5f + 0.5f;
            float tri_b = (float)sin(now * 0.5f) * 0.5f + 0.5f;

            //
            // RENDER: GL drawCalls
            //

            // --- NEW MATRIX SHADER UNIFORMS ---
            // create and set View Matrix according to camera settings
            shader_prog->setUniform("uV_m", camera.GetViewMatrix());
            shader_prog->setUniform("uP_m", projection_matrix); // Assuming projection_matrix is an accessible class member
            // ----------------------------------

            // Set triangle color and draw
            shader_prog->setUniform("color", glm::vec4(tri_r, tri_g, tri_b, 1.0f));

            if (model->meshes.empty()) {
                static bool once = true;
                if (once) { std::cerr << "WARNING: Model has no meshes!\n"; once = false; }
            }
            model->draw();

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
            previous_frame_render_time = now - frame_begin_timepoint; // compute delta_t for the NEXT frame
            frame_begin_timepoint = now; // set new start

            fps_counter_frames++;
            if (now - fps_last_displayed >= 1.0) {
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
    if (ImGui::GetCurrentContext()) {
	    ImGui_ImplOpenGL3_Shutdown();
	    ImGui_ImplGlfw_Shutdown();
	    ImGui::DestroyContext();
    }

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

void App::glfw_key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	auto this_inst = static_cast<App*>(glfwGetWindowUserPointer(window));
	if ((action == GLFW_PRESS) || (action == GLFW_REPEAT)) {
		switch (key) {
		case GLFW_KEY_ESCAPE:
			// Task 1.2: capture/release mouse OR exit
			{
				int mode = glfwGetInputMode(window, GLFW_CURSOR);
				if (mode == GLFW_CURSOR_DISABLED) {
					// first ESC uvolní kurzor
					glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
				}
				else {
					// druhý ESC (nebo při uvolněném) ukončí aplikaci
					glfwSetWindowShouldClose(window, GLFW_TRUE);
				}
			}
			break;
		case GLFW_KEY_C:
			// Task 3: Background color change
			this_inst->bg_r = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
			this_inst->bg_g = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
			this_inst->bg_b = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
			break;
		case GLFW_KEY_V:
			this_inst->is_vsync_on = !this_inst->is_vsync_on;
			glfwSwapInterval(this_inst->is_vsync_on ? 1 : 0);
			std::cout << "VSync: " << (this_inst->is_vsync_on ? "ON" : "OFF") << "\n";
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

void App::glfw_fbsize_callback(GLFWwindow* window, int width, int height) {
    // glViewport(0, 0, width, height);
		auto this_inst = static_cast<App*>(glfwGetWindowUserPointer(window));
    this_inst->width = width;
    this_inst->height = height;

    // set viewport
    glViewport(0, 0, width, height);
    //now your canvas has [0,0] in bottom left corner, and its size is [width x height]

    this_inst->update_projection_matrix();
}

void App::glfw_mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
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

void App::glfw_cursor_position_callback(GLFWwindow* window, double xpos, double ypos) {
    auto app = static_cast<App*>(glfwGetWindowUserPointer(window));

    app->camera.ProcessMouseMovement(xpos - app->cursorLastX, (ypos - app->cursorLastY) * -1.0);
    app->cursorLastX = xpos;
    app->cursorLastY = ypos;
}

void App::glfw_scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    // if (yoffset > 0.0) {
    //     std::cout << "wheel up...\n";
    // } else if (yoffset < 0.0) {
    //     std::cout << "wheel down...\n";
    // }
		auto this_inst = static_cast<App*>(glfwGetWindowUserPointer(window));
    this_inst->fov += 10*yoffset; // yoffset is mostly +1 or -1; one degree difference in fov is not visible
    this_inst->fov = std::clamp(this_inst->fov, 20.0f, 170.0f); // limit FOV to reasonable values...

    this_inst->update_projection_matrix();
}

void App::update_projection_matrix(void)
{
    if (height < 1)
        height = 1;   // avoid division by 0

    float ratio = static_cast<float>(width) / height;

    projection_matrix = glm::perspective(
        glm::radians(fov),   // The vertical Field of View, in radians: the amount of "zoom". Think "camera lens". Usually between 90° (extra wide) and 30° (quite zoomed in)
        ratio,               // Aspect Ratio. Depends on the size of your window.
        0.1f,                // Near clipping plane. Keep as big as possible, or you'll get precision issues.
        20000.0f             // Far clipping plane. Keep as little as possible.
    );
}

void App::toggle_fullscreen() {
    if (!fullscreen_enabled) {
        // Switch to fullscreen
		int xpos, ypos, width, height;
		glfwGetWindowPos(window, &xpos, &ypos);
		glfwGetWindowSize(window, &width, &height);

		// Find current monitor (the one where the window center is)
		int monitor_count;
		GLFWmonitor** monitors = glfwGetMonitors(&monitor_count);
		GLFWmonitor* current_monitor = glfwGetPrimaryMonitor();

		int center_x = xpos + width / 2;
		int center_y = ypos + height / 2;

		for (int i = 0; i < monitor_count; i++) {
			int mx, my;
			const GLFWvidmode* mode = glfwGetVideoMode(monitors[i]);
			glfwGetMonitorPos(monitors[i], &mx, &my);
			if (center_x >= mx && center_x < mx + mode->width &&
				center_y >= my && center_y < my + mode->height) {
				current_monitor = monitors[i];
				break;
			}
		}

        const GLFWvidmode* mode = glfwGetVideoMode(current_monitor);

        saved_window_x = xpos;
		saved_window_y = ypos;
        saved_window_width = width;
		saved_window_height = height;

        glfwSetWindowMonitor(window, current_monitor, 0, 0, mode->width, mode->height, mode->refreshRate);
        fullscreen_enabled = true;
    } else {
        // Switch back to windowed
        glfwSetWindowMonitor(window, nullptr, saved_window_x, saved_window_y, saved_window_width, saved_window_height, 0);
        fullscreen_enabled = false;
    }
}
