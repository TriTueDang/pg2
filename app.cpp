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
#include <sstream>
#include <iomanip>

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
#include "Texture.hpp"

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
        glfwGetFramebufferSize(window, &width, &height);
        update_projection_matrix();
        glViewport(0, 0, width, height);


        glEnable(GL_DEPTH_TEST);
        glDisable(GL_CULL_FACE); // Ensure triangle is visible from both sides
        glEnable(GL_MULTISAMPLE); // Task 1: Enable multisampling by default

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

	glfwWindowHint(GLFW_SAMPLES, 4); // Task 1: set MSAA level to 4

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
	glfwSetCursorPosCallback(window, cursorPositionCallback);
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
    
    // Load textures
    auto rango_tex = std::make_shared<Texture>("assets/rango/source/tex_65.png", Texture::Interpolation::linear_mipmap_linear, true); // Re-enabled flip
    auto revolver_tex = std::make_shared<Texture>("assets/38-special-revolver/textures/rev_d.tga.png");
    auto bullet_tex = std::make_shared<Texture>("assets/9mm-bullet/textures/Bullet Normal.png");
    auto city_tex = std::make_shared<Texture>("assets/chicken-gun-western-reupload/textures/PolygonWestern_Texture_01_A.png");
    auto bandit_tex = std::make_shared<Texture>("assets/bobrito-bandito-game-ready-3d-model-free/textures/Costume_Base_color.png");
    auto dynamite_tex = std::make_shared<Texture>("assets/dynamite/textures/Dynamite_Black_Dm.png");
    auto whiskey_tex = std::make_shared<Texture>("assets/Whiskey/material12.png");

    // Load City
    try {
        city_model = std::make_shared<Model>("assets/chicken-gun-western-reupload/source/western.obj", shader_prog, city_tex);
        city_model->scale = glm::vec3(0.05f); // Restored original scale
    } catch (...) { std::cerr << "Failed to load western city\n"; }

    // Load Rango (Player Model)
    try {
        player_model = std::make_shared<Model>("assets/rango/source/Rango.obj", shader_prog, rango_tex);
        player_model->scale = glm::vec3(4.0f); // Restored original scale
    } catch (...) { std::cerr << "Failed to load Rango\n"; }

    // Load Revolver (attached to player)
    try {
        weapon_model = std::make_shared<Model>("assets/38-special-revolver/source/rev_anim.obj.obj", shader_prog, revolver_tex);
        weapon_model->scale = glm::vec3(0.005f); // Restored original scale
    } catch (...) { std::cerr << "Failed to load revolver\n"; }

    // Build BVH physics for the city immediately after loading city model
    build_physics();

    // Load Bandits
    try {
        auto bandit_base = std::make_shared<Model>("assets/bobrito-bandito-game-ready-3d-model-free/source/Offensive Idle.obj", shader_prog, bandit_tex);
        bandits.clear();
        bandit_base_model = bandit_base; // Store base model for wave spawning
        
        bandits.clear();
        bandit_throw_timers.clear();
        spawn_bandit_wave(5); // Start with 5 bandits for challenge
    } catch (...) { std::cerr << "Failed to load bandits\n"; }






    // Load Dynamite
    try {
        dynamite_model = std::make_shared<Model>("assets/dynamite/source/Dynamite.obj", shader_prog, dynamite_tex);
        dynamite_model->scale = glm::vec3(0.5f);
    } catch (...) { std::cerr << "Failed to load dynamite model\n"; }

    // Load Bullet
    try {
        bullet_model = std::make_shared<Model>("assets/dynamite/source/Dynamite.obj", shader_prog, city_tex);
        bullet_model->scale = glm::vec3(0.15f);
    } catch (...) { std::cerr << "Failed to load bullet model\n"; }

    // Load Waypoint
    try {
        waypoint_shader = ShaderProgram::from_files("waypoint.vert", "waypoint.frag"); 
        waypoint_model = std::make_shared<Model>("triangle.obj", waypoint_shader);
        waypoint_model->scale = glm::vec3(2.5f);
    } catch (...) { std::cerr << "Failed to load waypoint assets\n"; }

    // Load Whiskey
    try {
        whiskey_model = std::make_shared<Model>("assets/Whiskey/MushroomPotion.obj", shader_prog, whiskey_tex);
        whiskey_model->scale = glm::vec3(4.0f); // Increased size as requested
    } catch (...) { std::cerr << "Failed to load whiskey model\n"; }

    shader_prog->use();
    shader_prog->setUniform("uTexture", 0);

    // Initialize directional light (sun)
    dir_light.direction = glm::normalize(glm::vec3(1.0f, -1.0f, -0.5f));
    dir_light.ambient = glm::vec3(0.3f, 0.3f, 0.3f);
    dir_light.diffuse = glm::vec3(0.8f, 0.8f, 0.8f); // Neutral white sun light
    dir_light.specular = glm::vec3(0.5f, 0.5f, 0.5f);

    // Initialize one point light for testing
    PointLight light1;
    light1.position = glm::vec3(5.0f, 5.0f, 5.0f);
    light1.ambient = glm::vec3(0.1f, 0.1f, 0.1f);
    light1.diffuse = glm::vec3(1.0f, 0.8f, 0.4f);
    light1.specular = glm::vec3(1.0f, 1.0f, 1.0f);
    point_lights.push_back(light1);

    // Initialize spot light (headlight/flashlight)
    SpotLight headlight;
    headlight.position = glm::vec3(0.0f, 0.0f, 0.0f);
    headlight.direction = glm::vec3(0.0f, 0.0f, -1.0f);
    headlight.ambient = glm::vec3(0.0f, 0.0f, 0.0f);
    headlight.diffuse = glm::vec3(1.0f, 1.0f, 0.9f);
    headlight.specular = glm::vec3(1.0f, 1.0f, 1.0f);
    headlight.cutoff = 10.0f;
    headlight.outer_cutoff = 15.0f;
    spot_lights.push_back(headlight);

    // Build collision grid for the city
    // Now done earlier in this function
    // --- Cinematic Spline Initialization (cv09) ---
    // Start far in the street, fly through to the player spawn
    glm::vec3 player_start = glm::vec3(-121.64f, -215.70f, 63.23f);
    float shift_x = 40.0f; // Shift to the right
    std::vector<glm::vec3> intro_points = {
        glm::vec3(-121.64f + shift_x, -120.0f, -400.0f), // Far, high end of street
        glm::vec3(-121.64f + shift_x, -160.0f, -200.0f), // Descending
        glm::vec3(-121.64f + shift_x, -200.0f, 0.0f),    // Mid-street
        glm::vec3(-121.64f + shift_x, -210.0f, 40.0f),   // Just in front of spawn
        player_start + glm::vec3(shift_x, 6.0f, 0)       // Final point shifted
    };
    intro_spline = PG2::CatmullRomSpline(intro_points, false); // Not cyclic
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
		double previous_frame_render_time{};

		// Clear color saved to OpenGL state machine

		glClearColor(0.2f, 0.3f, 0.4f, 1.0f); // Sky blue clear color

		glEnable(GL_DEPTH_TEST);
		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK);

		// disable cursor, so that it can not leave window, and we can process movement
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
		// get first position of mouse cursor
		glfwGetCursorPos(window, &cursorLastX, &cursorLastY);

		update_projection_matrix();
		glViewport(0, 0, width, height);

		// Starting position
		playerPos = glm::vec3(-121.64f, -215.70f, 63.23f); 
		float initial_ground = physics.get_ground_height(playerPos);
		if (initial_ground > -500.0f) playerPos.y = initial_ground; 
		
		camera.AttachTo(&playerPos, glm::vec3(0, 6.0f, 0), 12.0f); // Attach with offset and distance
		camera.MovementSpeed = 20.0f; 
		double last_frame_time = glfwGetTime();

		while (!glfwWindowShouldClose(window))
		{
			// ImGui prepare render (only if required)
			if (show_imgui) {
				ImGui_ImplOpenGL3_NewFrame();
				ImGui_ImplGlfw_NewFrame();
				ImGui::NewFrame();
				ImGui::SetNextWindowPos(ImVec2(10, 10));
				ImGui::SetNextWindowSize(ImVec2(300, 180));

				ImGui::Begin("Info", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);
				ImGui::Text("FPS: %.1f", FPS);
				ImGui::Text("SCORE: %d", score);
				ImGui::Text("WAVE: %d", wave_number);
				ImGui::Text("BANDITS LEFT: %d", (int)bandits.size());
				if (invulnerability_timer > 0.0f) {
					ImGui::TextColored(ImVec4(0, 1, 1, 1), "SHIELD: %.1fs", invulnerability_timer);
				}
				ImGui::Text("V-Sync: %s (hit V to toggle)", is_vsync_on ? "ON" : "OFF");
				ImGui::Text("Multisample (AA): %s (hit M to toggle)", is_multisample_on ? "ON" : "OFF");

				
				// Health Bar
				ImGui::Spacing();
				ImGui::Text("HEALTH");
				ImVec4 health_color = ImVec4(1.0f - (player_health / 100.0f), player_health / 100.0f, 0.0f, 1.0f);
				ImGui::PushStyleColor(ImGuiCol_PlotHistogram, health_color);
				ImGui::ProgressBar(player_health / 100.0f, ImVec2(-1, 0), "");
				ImGui::PopStyleColor();

				if (is_player_dead) {
					ImGui::SetNextWindowPos(ImVec2(width / 2.0f - 100, height / 2.0f - 50));
					ImGui::Begin("YOU DIED", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize);
					ImGui::TextColored(ImVec4(1, 0, 0, 1), "WASTED");
					if (ImGui::Button("RESPAWN (R)")) {
						// Full Reset
						player_health = 100.0f;
						is_player_dead = false;
						score = 0;
						wave_number = 1;
						bandits.clear();
						bandit_throw_timers.clear();
						bandit_shoot_timers.clear();
						bandit_velocities_y.clear();
						bandit_states.clear();
						bandit_target_positions.clear();
						bandit_state_timers.clear();
						spawn_bandit_wave(5); // Start fresh with 5
						playerPos = glm::vec3(-121.64f, -215.70f, 63.23f);
						velocity_y = 0.0f;
					}


					ImGui::End();
				}

				ImGui::End();

				// Crosshair
				ImGui::GetForegroundDrawList()->AddCircle(
					ImVec2(width / 2.0f, height / 2.0f), 
					10.0f, 
					IM_COL32(255, 255, 255, 150), 
					16, 
					2.0f
				);

				// Debug window removed by user request
			}

			//
			// UPDATE: recompute objects state, players position etc.
			//
			now = glfwGetTime();
			float delta_t = static_cast<float>(now - last_frame_time);
			last_frame_time = now;

			// --- Camera State Machine (cv09) ---
			if (cam_state == AppCameraState::CINEMATIC) {
				intro_time += delta_t * (intro_spline.getMaxT() / intro_duration);
				if (intro_time >= intro_spline.getMaxT()) {
					// Prepare Transition
					cam_state = AppCameraState::TRANSITION;
					cam_transition.progress = 0.0f;
					cam_transition.start_pos = camera.Position;
					cam_transition.start_front = camera.Front;
					
					// Calculate standard target (simulate a frame of gameplay logic)
					camera.HandleCollision(physics);
					cam_transition.end_pos = camera.Position;
					cam_transition.end_front = camera.Front;
				} else {
					camera.Position = intro_spline.evaluate(intro_time);
					camera.Front = glm::normalize(intro_spline.evaluateTangent(intro_time));
				}
			} else if (cam_state == AppCameraState::TRANSITION) {
				cam_transition.progress += delta_t / cam_transition.duration;
				if (cam_transition.progress >= 1.0f) {
					cam_state = AppCameraState::GAMEPLAY;
					invulnerability_timer = 3.0f; // 3 seconds of invulnerability after intro
				} else {
					// Smooth interpolation
					float t = cam_transition.progress;
					float smooth_t = t * t * (3 - 2 * t); // Smoothstep
					
					// We need the gameplay target to lerp towards it
					camera.HandleCollision(physics); 
					cam_transition.end_pos = camera.Position;
					cam_transition.end_front = camera.Front;

					camera.Position = glm::mix(cam_transition.start_pos, cam_transition.end_pos, smooth_t);
					camera.Front = glm::normalize(glm::mix(cam_transition.start_front, cam_transition.end_front, smooth_t));
				}
			}

			if (invulnerability_timer > 0.0f) {
				invulnerability_timer -= delta_t;
			}
			if (wave_info_timer > 0.0f) {
				wave_info_timer -= delta_t;
			}

			//########## react to user  ##########
			if (cam_state == AppCameraState::GAMEPLAY) {
				camera.ProcessInput(window, delta_t); // process keys etc.
			}

			// --- Player and Camera Logic (3rd Person) ---
			
			glm::vec3 moveDir(0.0f);
			if (!is_player_dead && cam_state == AppCameraState::GAMEPLAY) {
				if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) moveDir += camera.Front;
				if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) moveDir -= camera.Front;
				if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) moveDir -= camera.Right;
				if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) moveDir += camera.Right;
			}
			
			is_moving = false;
			glm::vec3 movement_delta(0.0f);
			if (glm::length(moveDir) > 0.0f) {
				moveDir.y = 0.0f; 
				if (glm::length(moveDir) > 0.0f) {
					movement_delta = glm::normalize(moveDir) * (float)(camera.MovementSpeed * delta_t);
					is_moving = true;
				}
			}

			// Integrated Physics Update for Player (always update physics to avoid falling through floor)
			auto kcc = physics.update_character(
				playerPos, 
				movement_delta, 
				velocity_y, 
				gravity, 
				8.0f, // Higher step height for stairs
				0.6f, // Smaller radius for narrow steps
				delta_t
			);

			playerPos = kcc.new_position;
			velocity_y = kcc.new_velocity_y;
			is_on_ground = kcc.is_on_ground;

			// Handle Camera Collision and Positioning
			if (cam_state == AppCameraState::GAMEPLAY) {
				camera.HandleCollision(physics);
			}

			// Safety floor check: if we fall somehow, snap back. 
			// Threshold increased to avoid false positives from the new robust KCC.
			if (playerPos.y < -1500.0f) {
				playerPos = glm::vec3(-121.64f, -215.70f, 63.23f);
				velocity_y = 0;
			}
			
			// Update bandit heights and implement AI
			for (size_t i = 0; i < bandits.size(); ++i) {
				auto& bandit = bandits[i];
				
				if (frame_count % 10 == 0) {
					float bg = physics.get_ground_height(bandit->pivot_position, 30.0f);
					if (bg > -500.0f) bandit->pivot_position.y = bg;
				}

				float dist = glm::distance(playerPos, bandit->pivot_position);
				
				// --- SMARTER AI STATE MACHINE ---
				if (i < bandit_states.size()) {
					bandit_state_timers[i] -= delta_t;
					
					// Transitions
					if (bandit_states[i] == AIState::CHASE && (bandit_state_timers[i] <= 0 || dist < 12.0f)) {
						bool found_cover = false;
						for (int attempt = 0; attempt < 8; ++attempt) {
							float angle = (float)(rand() % 360) * 0.0174f;
							float d_cover = 10.0f + (float)(rand() % 15);
							glm::vec3 candidate = bandit->pivot_position + glm::vec3(cos(angle) * d_cover, 0, sin(angle) * d_cover);
							auto ray_hit = physics.raycast(candidate + glm::vec3(0, 5, 0), glm::normalize(playerPos - candidate), glm::distance(candidate, playerPos));
							if (ray_hit.hit) {
								bandit_target_positions[i] = candidate;
								bandit_states[i] = AIState::SEEK_COVER;
								bandit_state_timers[i] = 10.0f;
								found_cover = true;
								break;
							}
						}
						bandit_state_timers[i] = found_cover ? 10.0f : 2.0f;
					} else if (bandit_states[i] == AIState::SEEK_COVER) {
						if (glm::distance(bandit->pivot_position, bandit_target_positions[i]) < 2.5f || bandit_state_timers[i] <= 0) {
							bandit_states[i] = AIState::SHOOTING;
							bandit_state_timers[i] = 4.0f + (float)(rand() % 5);
						}
					} else if (bandit_states[i] == AIState::SHOOTING && (bandit_state_timers[i] <= 0 || dist < 8.0f)) {
						bandit_states[i] = AIState::CHASE;
						bandit_state_timers[i] = 6.0f + (float)(rand() % 6);
					}

					// Movement
					glm::vec3 banditMoveDir(0.0f);
					if (bandit_states[i] == AIState::CHASE && dist > 15.0f) {
						glm::vec3 baseDir = glm::normalize(playerPos - bandit->pivot_position);
						glm::vec3 sideDir = glm::vec3(-baseDir.z, 0.0f, baseDir.x);
						float sideInfluence = sin((float)now * 4.0f + (float)i) * 0.8f;
						banditMoveDir = glm::normalize(baseDir + sideDir * sideInfluence);
					} else if (bandit_states[i] == AIState::SEEK_COVER) {
						banditMoveDir = glm::normalize(bandit_target_positions[i] - bandit->pivot_position);
					}

					if (glm::length(banditMoveDir) > 0.01f) {
						float b_step = 7.0f; 
						auto banditKcc = physics.update_character(bandit->pivot_position, banditMoveDir * (bandit_speed * delta_t), bandit_velocities_y[i], gravity, b_step, 0.6f, delta_t);
						bandit->pivot_position = banditKcc.new_position;
						bandit_velocities_y[i] = banditKcc.new_velocity_y;
					}
				}

				if (!is_player_dead) {
					// Attacks
					if (dist < 50.0f && dist > 10.0f) {
						bandit_throw_timers[i] -= delta_t;
						if (bandit_throw_timers[i] <= 0) {
							Dynamite d; d.position = bandit->pivot_position + glm::vec3(0, 5, 0); 
							d.velocity = glm::normalize(playerPos - d.position) * 25.0f + glm::vec3(0, 10, 0); 
							active_dynamites.push_back(d); bandit_throw_timers[i] = 4.0f + (float)(rand() % 2);
						}
					}
					if (dist < 100.0f) {
						bandit_shoot_timers[i] -= delta_t;
						if (bandit_shoot_timers[i] <= 0) {
							Bullet b; b.position = bandit->pivot_position + glm::vec3(0, 6, 0);
							b.velocity = glm::normalize(playerPos + glm::vec3(0, 4, 0) - b.position) * 80.0f;
							b.life = 2.0f; b.isFromPlayer = false; active_bullets.push_back(b);
							bandit_shoot_timers[i] = 2.0f + (float)(rand() % 3);
						}
					}
				}
			}


			// Update Dynamites
			for (auto it = active_dynamites.begin(); it != active_dynamites.end(); ) {
				if (!it->on_ground) {
					glm::vec3 nextPos = it->position + it->velocity * delta_t;
					// Wall/Ground collision for dynamites using raycast
					auto hit = physics.raycast(it->position, it->velocity, glm::length(it->velocity * delta_t));
					if (hit.hit) {
						it->timer = 0; // Detonate on impact with wall/ceiling
					}
					
					it->velocity.y += gravity * delta_t;
					it->position += it->velocity * delta_t;

					float ground = physics.get_ground_height(it->position, 2.0f);
					if (it->position.y < ground) {
						it->position.y = ground;
						it->velocity = glm::vec3(0); // Stop at ground
						it->on_ground = true;
					}
				}
				it->timer -= delta_t;

				if (it->timer <= 0) {
					// Explosion!
					float dist = glm::distance(it->position, playerPos);
					if (dist < dynamite_radius && cam_state == AppCameraState::GAMEPLAY && invulnerability_timer <= 0.0f) {
						player_health -= dynamite_damage * (1.0f - (dist / dynamite_radius));
						if (player_health <= 0) is_player_dead = true;
					}
					it = active_dynamites.erase(it);
				} else {
					++it;
				}
			}

			// Update Bullets
			for (auto it = active_bullets.begin(); it != active_bullets.end(); ) {
				// Wall/Ground collision for bullets using raycast
				float dist_step = glm::length(it->velocity * delta_t);
				auto hit = physics.raycast(it->position, it->velocity, dist_step);
				
				if (hit.hit) {
					it = active_bullets.erase(it);
					continue;
				}
				
				it->position += it->velocity * delta_t;
				it->life -= delta_t;

				bool has_collided = false;
				// Collision with bandits (ONLY if bullet is from player)
				for (size_t i = 0; i < bandits.size() && it->isFromPlayer; ++i) {

					// Bandit center height is roughly 4-6 units
					float dist = glm::distance(it->position, bandits[i]->pivot_position + glm::vec3(0, 4.0f, 0));
					if (dist < 4.0f) { 
						bandit_health[i]--;
						if (bandit_health[i] <= 0) {
							bandits.erase(bandits.begin() + i);
							if (i < bandit_throw_timers.size()) bandit_throw_timers.erase(bandit_throw_timers.begin() + i);
							if (i < bandit_shoot_timers.size()) bandit_shoot_timers.erase(bandit_shoot_timers.begin() + i);
							if (i < bandit_velocities_y.size()) bandit_velocities_y.erase(bandit_velocities_y.begin() + i);
							if (i < bandit_states.size()) bandit_states.erase(bandit_states.begin() + i);
							if (i < bandit_target_positions.size()) bandit_target_positions.erase(bandit_target_positions.begin() + i);
							if (i < bandit_state_timers.size()) bandit_state_timers.erase(bandit_state_timers.begin() + i);
							if (i < bandit_health.size()) bandit_health.erase(bandit_health.begin() + i);
							
							score += 100; // Points!
						}
						has_collided = true;
						break;
					}
				}

				// Collision with player (only if bullet is NOT from player)
				if (!has_collided && !it->isFromPlayer) {
					float distToPlayer = glm::distance(it->position, playerPos + glm::vec3(0, 5.0f, 0));
					if (distToPlayer < 5.0f && cam_state == AppCameraState::GAMEPLAY && invulnerability_timer <= 0.0f) {
						player_health -= 5.0f; // Take damage
						if (player_health <= 0) is_player_dead = true;
						has_collided = true;
					}
				}

				if (has_collided || it->life <= 0) {
					it = active_bullets.erase(it);
				} else {
					++it;
				}

			}

			// Wave progression: spawn new wave when only 1 or 0 bandits remain
			if (bandits.size() <= 1 && !is_player_dead) {
				wave_number++;
				spawn_bandit_wave(3 + wave_number); // Scale up with wave number
			}

			// Whiskey logic: rotation and collision
			for (auto& wp : whiskey_pickups) {
				if (!wp.active) continue;
				wp.rotation += delta_t * 90.0f; // 90 deg/sec
				wp.position.y += sin(glfwGetTime() * 2.0f) * 0.01f; // Bobbing effect
				
				float d = glm::distance(playerPos, wp.position);
				if (d < 5.0f) {
					player_health = std::min(100.0f, player_health + 25.0f);
					wp.active = false;
					std::cout << "Healed! Health: " << player_health << "\n";
				}
			}

			// Update Frame Count for optimization
			frame_count++;

			// Jump handling
			if (is_on_ground && glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
				velocity_y = jump_force;
				is_on_ground = false;
			}

			// Walking animation (procedural bobbing)
			float bobbing = 0.0f;
			if (is_moving && is_on_ground) {
				walk_anim_time += delta_t * 12.0f; // Faster animation
				bobbing = sin(walk_anim_time) * 0.15f; // More bobbing
			} else if (is_on_ground) {
				walk_anim_time += delta_t * 2.5f;
				bobbing = sin(walk_anim_time) * 0.02f; 
			}

			// Adjusting player model (Rango)
			if (player_model) {
				player_model->pivot_position = playerPos; // Position exactly on ground
				player_model->pivot_position.y += bobbing; 
				player_model->eulerAngles.y = camera.Yaw + 90.0f;
				player_model->eulerAngles.z = 0.0f; 

				// Procedural Leg Animation
				if (is_moving && is_on_ground) {
					float leg_swing = sin(walk_anim_time * 2.0f) * 25.0f;
					if (player_model->meshes.size() > 2) {
						player_model->meshes[1].eulerAngles.x = leg_swing;
						player_model->meshes[2].eulerAngles.x = -leg_swing;
					}
				} else {
					if (player_model->meshes.size() > 2) {
						player_model->meshes[1].eulerAngles.x = 0;
						player_model->meshes[2].eulerAngles.x = 0;
					}
				}
			}

			// Update weapon position
			if (weapon_model && player_model) {
				// Use tunable offsets
				weapon_model->pivot_position = player_model->pivot_position + camera.Right * weapon_offset.x + camera.Up * weapon_offset.y + camera.Front * weapon_offset.z;
				
				// Standard rotation based on camera
				weapon_model->eulerAngles.y = camera.Yaw + weapon_rotation.y;
				weapon_model->eulerAngles.x = -camera.Pitch + weapon_rotation.x;
				weapon_model->eulerAngles.z = weapon_rotation.z; 

				if (shoot_anim_time > 0.0f) {
					weapon_model->eulerAngles.x += sin(shoot_anim_time * 15.0f) * 10.0f;
					shoot_anim_time -= delta_t;
				}
			}

			// Camera positioning is now fully handled by camera.HandleCollision(physics)
			// which is called earlier in the loop (line 603).



			// Orient bandits towards player
			for (auto& bandit : bandits) {
				glm::vec3 dir = glm::normalize(camera.Position - bandit->pivot_position);
				float angle = glm::degrees(atan2(dir.x, dir.z));
				bandit->eulerAngles.y = angle;
			}

			// Update spotlight - attach to camera (headlight)
			if (!spot_lights.empty()) {
				spot_lights[0].position = camera.Position;
				spot_lights[0].direction = camera.Front;
			}

			//
			// RENDER: GL drawCalls
			//

			// Clear OpenGL canvas, both color buffer and Z-buffer
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			// Time-based color animation
			float tri_r = (float)sin(now) * 0.5f + 0.5f;
			float tri_g = (float)cos(now) * 0.5f + 0.5f;
			float tri_b = (float)sin(now * 0.5f) * 0.5f + 0.5f;

			//########## create and set View Matrix according to camera settings  ##########
			shader_prog->use(); // VZDY musitme aktivovat nas shader pred kreslenim, protoze ImGui (kreslene na konci smycky) prehazuje na svuj shader!
			shader_prog->setUniform("uV_m", camera.GetViewMatrix());
			shader_prog->setUniform("uP_m", projection_matrix);

			// Set up DIRECTIONAL LIGHT uniforms
			shader_prog->setUniform("dir_light_direction", dir_light.direction);
			shader_prog->setUniform("dir_light_ambient", dir_light.ambient);
			shader_prog->setUniform("dir_light_diffuse", dir_light.diffuse);
			shader_prog->setUniform("dir_light_specular", dir_light.specular);

			// Set up POINT LIGHTS uniforms
			shader_prog->setUniform("num_point_lights", (int)point_lights.size());
			for (size_t i = 0; i < point_lights.size() && i < 3; i++) {
				std::string idx = std::to_string(i);
				shader_prog->setUniform("point_light_position[" + idx + "]", point_lights[i].position);
				shader_prog->setUniform("point_light_ambient[" + idx + "]", point_lights[i].ambient);
				shader_prog->setUniform("point_light_diffuse[" + idx + "]", point_lights[i].diffuse);
				shader_prog->setUniform("point_light_specular[" + idx + "]", point_lights[i].specular);
			}

			// Set up SPOTLIGHT uniforms
			if (!spot_lights.empty()) {
				shader_prog->setUniform("spot_light_position", spot_lights[0].position);
				shader_prog->setUniform("spot_light_direction", spot_lights[0].direction);
				shader_prog->setUniform("spot_light_ambient", spot_lights[0].ambient);
				shader_prog->setUniform("spot_light_diffuse", spot_lights[0].diffuse);
				shader_prog->setUniform("spot_light_specular", spot_lights[0].specular);
				shader_prog->setUniform("spot_light_cutoff", spot_lights[0].cutoff);
				shader_prog->setUniform("spot_light_outer_cutoff", spot_lights[0].outer_cutoff);
			}

			// draw all (pokud bys mel objekty v poli scene)
			for (auto& [name, model_obj] : scene) {
				model_obj.draw();
			}

			// Draw city
			if (city_model) {
				city_model->draw();
			}

			// Draw player (Rango)
			if (player_model) {
				player_model->draw();
			}

		// Draw bandits
			for (auto& bandit : bandits) {
				bandit->draw();
			}


			// Draw dynamites
			if (dynamite_model) {
				for (auto& d : active_dynamites) {
					dynamite_model->pivot_position = d.position;
					// Add some spin
					dynamite_model->eulerAngles.x += 200.0f * delta_t;
					dynamite_model->draw();
				}
			}

			// Draw Whiskey bottles
			if (whiskey_model) {
				for (auto& wp : whiskey_pickups) {
					if (!wp.active) continue;
					whiskey_model->pivot_position = wp.position;
					whiskey_model->eulerAngles.y = wp.rotation;
					// Add a subtle secondary rotation for more "magic" feel
					whiskey_model->eulerAngles.x = 15.0f * sin(glfwGetTime());
					whiskey_model->draw();
				}
			}

			// Draw bullets
			if (bullet_model) {
				for (auto& b : active_bullets) {
					bullet_model->pivot_position = b.position;
					if (glm::length(b.velocity) > 0.01f) {
						glm::vec3 dir = glm::normalize(b.velocity);
						bullet_model->eulerAngles.y = glm::degrees(atan2(dir.x, dir.z));
						bullet_model->eulerAngles.x = glm::degrees(asin(dir.y));
					}
					bullet_model->draw();
				}
			}

			// Draw Waypoints for bandits
			if (waypoint_shader && !bandits.empty()) {
				waypoint_shader->use();
				waypoint_shader->setUniform("uV_m", camera.GetViewMatrix());
				waypoint_shader->setUniform("uP_m", projection_matrix);
				waypoint_shader->setUniform("waypointColor", glm::vec3(1.0f, 1.0f, 0.0f)); // Yellow
				
				for (auto& b : bandits) {
					glm::mat4 m = glm::translate(glm::mat4(1.0f), b->pivot_position + glm::vec3(0, 15.0f, 0));
					m = glm::rotate(m, (float)now * 2.0f, glm::vec3(0, 1, 0));
					m = glm::scale(m, glm::vec3(2.5f)); // Make it visible
					waypoint_shader->setUniform("uM_m", m);
					if (waypoint_model) waypoint_model->draw();
				}
				shader_prog->use();
			}

			// --- Debug Render Intro Spline Path (cv09) ---
			// Disabled by default (was used for debugging the street path)
			/*
			if (waypoint_shader && path_vao != 0) {
				waypoint_shader->use();
				waypoint_shader->setUniform("uV_m", camera.GetViewMatrix());
				waypoint_shader->setUniform("uP_m", projection_matrix);
				waypoint_shader->setUniform("waypointColor", glm::vec3(0.0f, 1.0f, 1.0f));
				waypoint_shader->setUniform("uM_m", glm::mat4(1.0f));
				glBindVertexArray(path_vao);
				glDrawArrays(GL_LINE_STRIP, 0, path_vertex_count);
				shader_prog->use();
			}
			*/

			// Draw weapon (last to be on top?)
			if (weapon_model) {
				// Clear depth before drawing weapon to avoid clipping into walls? 
				// Actually might not be good for all cases, but works for HUD.
				// glClear(GL_DEPTH_BUFFER_BIT); 
				weapon_model->draw();
			}

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
		case GLFW_KEY_R:
			// Respawn / Restart
			this_inst->player_health = 100.0f;
			this_inst->is_player_dead = false;
			this_inst->score = 0;
			this_inst->wave_number = 1;
			this_inst->active_bullets.clear();
			this_inst->active_dynamites.clear();
			this_inst->bandits.clear();
			this_inst->bandit_throw_timers.clear();
			this_inst->bandit_shoot_timers.clear();
			this_inst->spawn_bandit_wave(3);
			this_inst->playerPos = glm::vec3(-121.64f, -215.70f, 63.23f);
			this_inst->velocity_y = 0.0f;
			std::cout << "Game Reset!\n";

			break;
		case GLFW_KEY_G:

			// Toggle ImGUI display
			this_inst->show_imgui = !this_inst->show_imgui;
			break;
		case GLFW_KEY_M:
			// Task 1: Toggle Multisampling
			this_inst->is_multisample_on = !this_inst->is_multisample_on;
			if (this_inst->is_multisample_on) glEnable(GL_MULTISAMPLE);
			else glDisable(GL_MULTISAMPLE);
			std::cout << "Multisampling: " << (this_inst->is_multisample_on ? "ON" : "OFF") << "\n";
			break;
		case GLFW_KEY_P:
			// Task 2: Take Screenshot
			this_inst->take_screenshot();
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
				// we are already inside our game: shoot!
				auto this_inst = static_cast<App*>(glfwGetWindowUserPointer(window));
				
				// Block shooting during cinematic (cv09)
				if (this_inst->cam_state != AppCameraState::GAMEPLAY) return;

				this_inst->shoot_anim_time = 0.3f; // 0.3 seconds animation
				
				// Spawn Bullet
				Bullet b;
				// Bullet starts from head/shoulder level (5.0f) to match new crosshair
				b.position = this_inst->player_model->pivot_position + this_inst->camera.Up * 5.0f + this_inst->camera.Front * 3.0f;

				b.velocity = this_inst->camera.Front * 150.0f; // Fast bullet
				b.life = 0.4f; // Drastically reduced range (60 units) to match/under-range bandits
				b.isFromPlayer = true;



				this_inst->active_bullets.push_back(b);



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

void App::cursorPositionCallback(GLFWwindow* window, double xpos, double ypos) {
    auto app = static_cast<App*>(glfwGetWindowUserPointer(window));

    if (app->firstMouse) {
        app->cursorLastX = xpos;
        app->cursorLastY = ypos;
        app->firstMouse = false;
    }

    if (app->cam_state == AppCameraState::GAMEPLAY) {
        app->camera.ProcessMouseMovement(xpos - app->cursorLastX, (ypos - app->cursorLastY) * -1.0);
    }
    app->cursorLastX = xpos;
    app->cursorLastY = ypos;
}

void App::glfw_cursor_position_callback(GLFWwindow* window, double xposIn, double yposIn) {
	auto this_inst = static_cast<App*>(glfwGetWindowUserPointer(window));

	// Only move camera if cursor is disabled (Task 2, point 4)
	if (glfwGetInputMode(window, GLFW_CURSOR) != GLFW_CURSOR_DISABLED)
		return;

	float xpos = static_cast<float>(xposIn);
	float ypos = static_cast<float>(yposIn);

	if (this_inst->firstMouse) {
		this_inst->lastX = xpos;
		this_inst->lastY = ypos;
		this_inst->firstMouse = false;
	}

	float xoffset = xpos - this_inst->lastX;
	float yoffset = this_inst->lastY - ypos; // reversed since y-coordinates go from bottom to top
	this_inst->lastX = xpos;
	this_inst->lastY = ypos;

	float sensitivity = 0.1f;
	xoffset *= sensitivity;
	yoffset *= sensitivity;

	this_inst->yaw += xoffset;
	this_inst->pitch += yoffset;

	// make sure that when pitch is out of bounds, screen doesn't get flipped
	if (this_inst->pitch > 89.0f)
		this_inst->pitch = 89.0f;
	if (this_inst->pitch < -89.0f)
		this_inst->pitch = -89.0f;

	glm::vec3 front;
	front.x = cos(glm::radians(this_inst->yaw)) * cos(glm::radians(this_inst->pitch));
	front.y = sin(glm::radians(this_inst->pitch));
	front.z = sin(glm::radians(this_inst->yaw)) * cos(glm::radians(this_inst->pitch));
	this_inst->camera_front = glm::normalize(front);
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

// ---------------------------------------------------------------------------
// Screenshot logic
// ---------------------------------------------------------------------------
void App::take_screenshot() {
	// Generate unique filename
	auto now = std::chrono::system_clock::now();
	auto time_t_now = std::chrono::system_clock::to_time_t(now);
	std::stringstream ss;
	ss << "screenshot_" << std::put_time(std::localtime(&time_t_now), "%Y%m%d_%H%M%S")
	   << (is_multisample_on ? "_aa" : "_noaa") << ".png";
	std::string filename = ss.str();

	// Allocate OpenCV matrix
	cv::Mat img(height, width, CV_8UC3);

	// Read pixels from GL FRONT buffer or back buffer.
	// Make sure we are aligned to 1 byte
	glPixelStorei(GL_PACK_ALIGNMENT, 1);
	glReadPixels(0, 0, width, height, GL_BGR, GL_UNSIGNED_BYTE, img.data);

	// OpenGL origin is bottom-left, OpenCV is top-left
	cv::flip(img, img, 0);

	// Write to file
	if (cv::imwrite(filename, img)) {
		std::cout << "Screenshot saved to: " << filename << std::endl;
	} else {
		std::cerr << "Failed to save screenshot: " << filename << std::endl;
	}
}

void App::build_physics() {
    if (!city_model) return;
    physics.build_bvh(city_model->getTriangles());
}

float App::get_ground_height(glm::vec3 pos, float ray_depth) {
    return physics.get_ground_height(pos, ray_depth);
}


void App::spawn_bandit_wave(int count) {
    if (!bandit_base_model) return;
    
    // Trigger Cinematic Intro (cv09) only on first wave
    if (is_first_wave) {
        cam_state = AppCameraState::CINEMATIC;
        intro_time = 0.0f;
        is_first_wave = false;
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED); 
    }
    
    wave_info_timer = 4.0f; // Show "New Wave" for 4 seconds
    
    // Clear old state for fresh wave (except score/level tracker)
    bandit_states.clear();
    bandit_target_positions.clear();
    bandit_state_timers.clear();
    bandit_health.clear();
    bandit_throw_timers.clear();
    bandit_shoot_timers.clear();
    bandit_velocities_y.clear();
    bandit_safe_positions.clear();
    
    // We already cleared 'bandits' vector in reset or before calling if needed, 
    // but here we are spawning a NEW wave, so we clear it now.
    bandits.clear();

    std::default_random_engine generator((unsigned)time(0));
    std::uniform_real_distribution<float> dist_x(-400.0f, 400.0f);
    std::uniform_real_distribution<float> dist_z(-400.0f, 400.0f);

    for (int i = 0; i < count; ++i) {
        auto bandit = std::make_shared<Model>(*bandit_base_model);
        
        // Spawn randomly across the city
        glm::vec3 spawn_pos;
        int attempts = 0;
        float h = 0.0f;
        do {
            spawn_pos = glm::vec3(dist_x(generator), -218.0f, dist_z(generator));
            h = get_ground_height(spawn_pos);
            attempts++;
            
            // Dispersion check: ensure not too close to other bandits
            bool too_close = false;
            for (auto const& existing : bandits) {
                if (glm::distance(spawn_pos, existing->pivot_position) < 15.0f) {
                    too_close = true;
                    break;
                }
            }
            if (too_close && attempts < 15) { h = -1000.0f; } // Force retry
            
        } while ((glm::distance(spawn_pos, playerPos) < 50.0f || h > -215.0f) && attempts < 20);

        bandit->pivot_position = spawn_pos;
        bandit->pivot_position.y = h;
        if (bandit->pivot_position.y < -500.0f) bandit->pivot_position.y = -218.70f;
        
        bandit->scale = glm::vec3(0.04f); 
        bandits.push_back(bandit);
        bandit_throw_timers.push_back(2.0f + (float)(rand() % 4)); 
        bandit_shoot_timers.push_back(1.0f + (float)(rand() % 3));
        bandit_velocities_y.push_back(0.0f);
        bandit_safe_positions.push_back(bandit->pivot_position);

        // State Machine Init
        bandit_states.push_back(AIState::CHASE);
        bandit_target_positions.push_back(bandit->pivot_position);
        bandit_state_timers.push_back(3.0f + (float)(rand() % 10));
        bandit_health.push_back(3); // 3 hits to kill
    }
    std::cout << "Wave " << wave_number << " started with " << count << " bandits.\n";

    // Refresh Whiskey pickups every wave
    spawn_whiskey_pickups();
}

void App::spawn_whiskey_pickups() {
    whiskey_pickups.clear();
    std::default_random_engine generator((unsigned)time(0));
    std::uniform_real_distribution<float> dist_pos(-250.0f, 250.0f);

    int count = 3; // 3 bottles per wave
    for (int i = 0; i < count; ++i) {
        int attempts = 0;
        while (attempts < 50) {
            glm::vec3 test_pos(dist_pos(generator), -218.0f, dist_pos(generator));
            float g = physics.get_ground_height(test_pos);
            float c = physics.get_ceiling_height(test_pos);
            
            // INDOOR CHECK: Has a ceiling and the room is reasonably low
            if (g > -500.0f && c > g && (c - g) < 20.0f) {
                WhiskeyPickup wp;
                glm::vec3 candidate_pos = glm::vec3(test_pos.x, g + 2.5f, test_pos.z);
                
                // PUSH OUT OF WALLS: resolve collision with 1.5f radius
                wp.position = physics.resolve_sphere_collision(candidate_pos, 1.5f);
                wp.active = true;
                whiskey_pickups.push_back(wp);
                break;
            }
            attempts++;
        }
    }
}


void App::init_path_visualization() {
    glCreateVertexArrays(1, &path_vao);
    glCreateBuffers(1, &path_vbo);

    update_path_visualization();

    glEnableVertexArrayAttrib(path_vao, 0);
    glVertexArrayAttribFormat(path_vao, 0, 3, GL_FLOAT, GL_FALSE, 0);
    glVertexArrayAttribBinding(path_vao, 0, 0);
    glVertexArrayVertexBuffer(path_vao, 0, path_vbo, 0, sizeof(glm::vec3));
}

void App::update_path_visualization() {
    std::vector<glm::vec3> visualization_points;
    float maxT = intro_spline.getMaxT();
    int segments = 200; // Resolution of the line
    for (int i = 0; i <= segments; ++i) {
        float t = (float)i / (float)segments * maxT;
        visualization_points.push_back(intro_spline.evaluate(t));
    }
    path_vertex_count = (int)visualization_points.size();

    glNamedBufferData(path_vbo, visualization_points.size() * sizeof(glm::vec3), visualization_points.data(), GL_STATIC_DRAW);
}
