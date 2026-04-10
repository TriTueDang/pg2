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
    auto rango_tex = std::make_shared<Texture>("assets/rango/source/tex_65.png");
    auto revolver_tex = std::make_shared<Texture>("assets/38-special-revolver/textures/rev_d.tga.png");
    auto bullet_tex = std::make_shared<Texture>("assets/9mm-bullet/textures/Bullet Normal.png");
    auto city_tex = std::make_shared<Texture>("assets/chicken-gun-western-reupload/textures/PolygonWestern_Texture_01_A.png");
    auto bandit_tex = std::make_shared<Texture>("assets/bobrito-bandito-game-ready-3d-model-free/textures/Costume_Base_color.png");
    auto dynamite_tex = std::make_shared<Texture>("assets/dynamite/textures/Dynamite_Black_Dm.png");

    // Load City
    try {
        city_model = std::make_shared<Model>("assets/chicken-gun-western-reupload/source/western.obj", shader_prog, city_tex);
        city_model->scale = glm::vec3(0.05f); // Increased default scale
    } catch (...) { std::cerr << "Failed to load western city\n"; }

    // Load Rango (Player Model)
    try {
        player_model = std::make_shared<Model>("assets/rango/source/Rango.obj", shader_prog, rango_tex);
        player_model->scale = glm::vec3(4.0f);
    } catch (...) { std::cerr << "Failed to load Rango\n"; }

    // Load Revolver (attached to player)
    try {
        weapon_model = std::make_shared<Model>("assets/38-special-revolver/source/rev_anim.obj.obj", shader_prog, revolver_tex);
        weapon_model->scale = glm::vec3(0.005f); // Adjust scale for hand size
    } catch (...) { std::cerr << "Failed to load revolver\n"; }

    // Build collision grid for the city immediately after loading city model 
    // to have bounds for bandit spawning
    build_collision_grid();

    // Load Bandits
    try {
        auto bandit_base = std::make_shared<Model>("assets/bobrito-bandito-game-ready-3d-model-free/source/Offensive Idle.obj", shader_prog, bandit_tex);
        bandits.clear();
        bandit_base_model = bandit_base; // Store base model for wave spawning
        
        bandits.clear();
        bandit_throw_timers.clear();
        spawn_bandit_wave(wave_number);
    } catch (...) { std::cerr << "Failed to load bandits\n"; }

    // Load Dynamite
    try {
        dynamite_model = std::make_shared<Model>("assets/dynamite/source/Dynamite.obj", shader_prog, dynamite_tex);
        dynamite_model->scale = glm::vec3(0.5f);
    } catch (...) { std::cerr << "Failed to load dynamite model\n"; }

    // Load Bullet
    try {
        // Use Dynamite model for bullets too for optimization (it's low poly)
        bullet_model = std::make_shared<Model>("assets/dynamite/source/Dynamite.obj", shader_prog, city_tex);
        bullet_model->scale = glm::vec3(0.15f); // Increased scale for visibility
    } catch (...) { std::cerr << "Failed to load bullet model\n"; }

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
    // build_collision_grid();
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
		playerPos = glm::vec3(-121.64f, -215.70f, 63.23f); // Calibrated spawn point (spawn slightly higher to fall onto it)
		float initial_ground = get_ground_height(playerPos);
		if (initial_ground > -300.0f) playerPos.y = initial_ground; 
		else playerPos.y = -218.70f;
		
		camera.Position = playerPos + glm::vec3(0, 8.0f, 20.0f); 
		camera.MovementSpeed = 15.0f; // Faster movement for larger scale
		camera.Mode = CameraMode::POV_LOCKED; // To disable internal movement but keep mouse rotation
		double last_frame_time = glfwGetTime();

		while (!glfwWindowShouldClose(window))
		{
			// ImGui prepare render (only if required)
			if (show_imgui) {
				ImGui_ImplOpenGL3_NewFrame();
				ImGui_ImplGlfw_NewFrame();
				ImGui::NewFrame();
				ImGui::SetNextWindowPos(ImVec2(10, 10));
				ImGui::SetNextWindowSize(ImVec2(300, 150));

				ImGui::Begin("Info", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);
				ImGui::Text("FPS: %.1f", FPS);
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
					if (ImGui::Button("RESPAWN")) {
						player_health = 100.0f;
						is_player_dead = false;
						// Reset playerPos to start? (Optional)
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

				ImGui::SetNextWindowPos(ImVec2(10, 200));
				ImGui::SetNextWindowSize(ImVec2(300, 150));
				ImGui::Begin("Debug", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);
				ImGui::Text("Player X: %.2f", playerPos.x);
				ImGui::Text("Player Y: %.2f", playerPos.y);
				ImGui::Text("Player Z: %.2f", playerPos.z);
				if (ImGui::SliderFloat("Manual Ground Y", &ground_height, -800.0f, 50.0f)) {
					// playerPos.y = ground_height; // Disabled auto-override to keep physics
				}
				if (city_model) {
					float current_scale = city_model->scale.x;
					if (ImGui::SliderFloat("City Scale", &current_scale, 0.001f, 0.5f, "%.3f")) {
						city_model->scale = glm::vec3(current_scale);
						build_collision_grid(); // Rebuild grid when scale changes
					}
				}
				ImGui::End();
			}

			//
			// UPDATE: recompute objects state, players position etc.
			//
			now = glfwGetTime();
			float delta_t = static_cast<float>(now - last_frame_time);
			last_frame_time = now;

			//########## react to user  ##########
			camera.ProcessInput(window, delta_t); // process keys etc.

			// --- Player and Camera Logic (3rd Person) ---
			
			// Move playerPos instead of camera
			glm::vec3 moveDir(0.0f);
			if (!is_player_dead) {
				if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) moveDir += camera.Front;
				if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) moveDir -= camera.Front;
				if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) moveDir -= camera.Right;
				if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) moveDir += camera.Right;
			}
			
			is_moving = false;
			if (glm::length(moveDir) > 0.0f) {
				moveDir.y = 0.0f; // Don't move up/down
				if (glm::length(moveDir) > 0.0f) { // Re-check after zeroing Y
					glm::vec3 nextPos = playerPos + glm::normalize(moveDir) * (float)(camera.MovementSpeed * delta_t);
					float nextGround = get_ground_height(nextPos);
					
					// Wall collision: only move if the height difference is small (step height)
					if (nextGround - playerPos.y < 2.0f) { 
						playerPos = nextPos;
						is_moving = true;
					}
				}
			}

			// Gravity and Physics
			velocity_y += gravity * delta_t;
			playerPos.y += velocity_y * delta_t;

			float current_ground = get_ground_height(playerPos);
			if (playerPos.y <= current_ground) {
				playerPos.y = current_ground;
				velocity_y = 0.0f;
				is_on_ground = true;
			} else {
				is_on_ground = false;
			}

			// Safety floor check: if we fall somehow, snap back
			if (playerPos.y < -300.0f) {
				playerPos.y = -218.70f;
				velocity_y = 0;
				is_on_ground = true;
			}
			
			// Update bandit heights and implement AI
			for (size_t i = 0; i < bandits.size(); ++i) {
				auto& bandit = bandits[i];
				
				// Optimization: Only check height every 10 frames or if moving
				if (frame_count % 10 == 0) {
					bandit->pivot_position.y = get_ground_height(bandit->pivot_position);
				}

				float dist = glm::distance(playerPos, bandit->pivot_position);
				
				// Chase AI - only move if too far away to throw
				if (dist > 35.0f && !is_player_dead) {
					glm::vec3 dir = glm::normalize(playerPos - bandit->pivot_position);
					dir.y = 0; // Keep on ground XZ
					bandit->pivot_position += dir * (bandit_speed * delta_t);
					bandit->pivot_position.y = get_ground_height(bandit->pivot_position);
				}

				// Dynamite AI
				if (dist < 50.0f && dist > 5.0f && !is_player_dead) {
					bandit_throw_timers[i] -= delta_t;
					if (bandit_throw_timers[i] <= 0) {
						// Throw dynamite
						Dynamite d;
						d.position = bandit->pivot_position + glm::vec3(0, 5.0f, 0);
						glm::vec3 dir = glm::normalize(playerPos - d.position);
						d.velocity = dir * 25.0f + glm::vec3(0, 10.0f, 0); // Upward arc
						active_dynamites.push_back(d);
						bandit_throw_timers[i] = bandit_throw_cooldown;
					}
				}

				// Melee attack removed as requested
			}

			// Update Dynamites
			for (auto it = active_dynamites.begin(); it != active_dynamites.end(); ) {
				if (!it->on_ground) {
					glm::vec3 nextPos = it->position + it->velocity * delta_t;
					float nextGround = get_ground_height(nextPos);
					
					// Wall collision: if something is suddenly high in front of it
					if (nextGround > it->position.y + 0.5f) {
						it->timer = 0; // Detonate at wall
					}
					
					it->velocity.y += gravity * delta_t;
					it->position += it->velocity * delta_t;

					float ground = get_ground_height(it->position);
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
					if (dist < dynamite_radius) {
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
				glm::vec3 nextPos = it->position + it->velocity * delta_t;
				float nextGround = get_ground_height(nextPos);
				
				// Wall/Ground collision for bullets
				if (nextGround > it->position.y + 0.1f) {
					it = active_bullets.erase(it);
					continue;
				}
				
				it->position = nextPos;
				it->life -= delta_t;

				bool hit = false;
				// Collision with bandits
				for (size_t i = 0; i < bandits.size(); ++i) {
					float dist = glm::distance(it->position, bandits[i]->pivot_position + glm::vec3(0, 5.0f, 0));
					if (dist < 3.0f) { // Collision radius
						bandits.erase(bandits.begin() + i);
						if (i < bandit_throw_timers.size()) {
							bandit_throw_timers.erase(bandit_throw_timers.begin() + i);
						}
						hit = true;
						break;
					}
				}

				if (hit || it->life <= 0) {
					it = active_bullets.erase(it);
				} else {
					++it;
				}
			}

			// Wave progression
			if (bandits.empty() && !is_player_dead) {
				wave_number++;
				spawn_bandit_wave(wave_number);
			}

			// Update Frame Count for optimization
			frame_count++;

			// Jump handling
			if (is_on_ground && glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
				velocity_y = jump_force;
				is_on_ground = false;
			}

			// Walking animation (procedural bobbing and swaying)
			float bobbing = 0.0f;
			float sway = 0.0f;
			if (is_moving && is_on_ground) {
				walk_anim_time += delta_t * 12.0f; // Faster animation
				bobbing = sin(walk_anim_time) * 0.15f; // More bobbing
				sway = sin(walk_anim_time * 0.5f) * 12.0f; // More sway
			} else if (is_on_ground) {
				walk_anim_time += delta_t * 2.5f;
				bobbing = sin(walk_anim_time) * 0.02f; 
				sway = 0.0f;
			}

			// Adjusting player model (Rango)
			if (player_model) {
				// Offset SIGNIFICANTLY (waist-deep fix)
				player_model->pivot_position = playerPos + glm::vec3(0, 7.5f, 0);
				player_model->pivot_position.y += bobbing; 
				player_model->eulerAngles.y = camera.Yaw + 90.0f;
				player_model->eulerAngles.z = sway; // Apply swaying tilt
			}

			// Update weapon position (attach to player hand area)
			if (weapon_model && player_model) {
				// Offset scaled for Rango scale 4.0
				weapon_model->pivot_position = player_model->pivot_position + camera.Right * 1.2f + camera.Up * 3.5f + camera.Front * 1.5f;
				weapon_model->eulerAngles.y = camera.Yaw + 180.0f;
				weapon_model->eulerAngles.x = -camera.Pitch;
				weapon_model->eulerAngles.z = player_model->eulerAngles.z; // Follow sway
				
				if (shoot_anim_time > 0.0f) {
					weapon_model->eulerAngles.x += sin(shoot_anim_time * 15.0f) * 10.0f;
					shoot_anim_time -= delta_t;
				}
			}

			// Position camera behind player
			glm::vec3 idealCameraPos = playerPos + (-camera.Front * 18.0f + camera.Up * 8.0f);
			
			// Camera collision: if ideal pos is underground or inside a building
			float camGround = get_ground_height(idealCameraPos);
			if (idealCameraPos.y < camGround + 2.0f) {
				// Slide camera closer to player if blocked
				idealCameraPos = playerPos + (-camera.Front * 5.0f + camera.Up * 4.0f); 
			}
			camera.Position = idealCameraPos;

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
				glm::vec3 original_pos = bandit->pivot_position;
				bandit->pivot_position.y += 5.0f; // Increased rendering offset
				bandit->draw();
				bandit->pivot_position = original_pos; // Restore for AI
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

			// Draw bullets
			if (bullet_model) {
				for (auto& b : active_bullets) {
					bullet_model->pivot_position = b.position;
					// Align bullet to velocity
					if (glm::length(b.velocity) > 0.01f) {
						glm::vec3 dir = glm::normalize(b.velocity);
						bullet_model->eulerAngles.y = glm::degrees(atan2(dir.x, dir.z));
						bullet_model->eulerAngles.x = glm::degrees(asin(dir.y));
					}
					bullet_model->draw();
				}
			}

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
				this_inst->shoot_anim_time = 0.3f; // 0.3 seconds animation
				
				// Spawn Bullet
				Bullet b;
				b.position = this_inst->player_model->pivot_position + this_inst->camera.Up * 6.0f + this_inst->camera.Front * 3.0f;
				b.velocity = this_inst->camera.Front * 150.0f; // Fast bullet
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

    app->camera.ProcessMouseMovement(xpos - app->cursorLastX, (ypos - app->cursorLastY) * -1.0);
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

void App::build_collision_grid() {
    if (!city_model) return;

    auto triangles = city_model->getTriangles();
    if (triangles.empty()) return;

    // Find bounds
    glm::vec3 min(1e10f), max(-1e10f);
    for (const auto& tri : triangles) {
        min = glm::min(min, glm::min(tri.v0, glm::min(tri.v1, tri.v2)));
        max = glm::max(max, glm::max(tri.v0, glm::max(tri.v1, tri.v2)));
    }

    collision_grid.min_bound = glm::vec2(min.x, min.z);
    collision_grid.max_bound = glm::vec2(max.x, max.z);
    collision_grid.cells.assign(collision_grid.resolution * collision_grid.resolution, {});

    collision_grid.cell_size_x = (collision_grid.max_bound.x - collision_grid.min_bound.x) / collision_grid.resolution;
    collision_grid.cell_size_z = (collision_grid.max_bound.y - collision_grid.min_bound.y) / collision_grid.resolution;

    for (const auto& tri : triangles) {
        // Find cell range for this triangle
        float tri_min_x = std::min({tri.v0.x, tri.v1.x, tri.v2.x});
        float tri_max_x = std::max({tri.v0.x, tri.v1.x, tri.v2.x});
        float tri_min_z = std::min({tri.v0.z, tri.v1.z, tri.v2.z});
        float tri_max_z = std::max({tri.v0.z, tri.v1.z, tri.v2.z});

        int start_x = std::max(0, (int)((tri_min_x - collision_grid.min_bound.x) / collision_grid.cell_size_x));
        int end_x = std::min(collision_grid.resolution - 1, (int)((tri_max_x - collision_grid.min_bound.x) / collision_grid.cell_size_x));
        int start_z = std::max(0, (int)((tri_min_z - collision_grid.min_bound.y) / collision_grid.cell_size_z));
        int end_z = std::min(collision_grid.resolution - 1, (int)((tri_max_z - collision_grid.min_bound.y) / collision_grid.cell_size_z));

        for (int ix = start_x; ix <= end_x; ++ix) {
            for (int iz = start_z; iz <= end_z; ++iz) {
                collision_grid.cells[ix + iz * collision_grid.resolution].push_back(tri);
            }
        }
    }
    std::cout << "Collision grid built with " << triangles.size() << " triangles.\n";
}

float App::get_ground_height(glm::vec3 pos) {
    int ix = (int)((pos.x - collision_grid.min_bound.x) / collision_grid.cell_size_x);
    int iz = (int)((pos.z - collision_grid.min_bound.y) / collision_grid.cell_size_z);

    if (ix < 0 || ix >= collision_grid.resolution || iz < 0 || iz >= collision_grid.resolution) return -1000.0f;

    const auto& cell_triangles = collision_grid.cells[ix + iz * collision_grid.resolution];
    float max_y = -1000.0f;

    for (const auto& tri : cell_triangles) {
        // Möller-Trumbore ray-triangle intersection (ray is vertical down)
        glm::vec3 edge1 = tri.v1 - tri.v0;
        glm::vec3 edge2 = tri.v2 - tri.v0;
        glm::vec3 h = glm::cross(glm::vec3(0, -1, 0), edge2);
        float a = glm::dot(edge1, h);

        if (a > -0.00001f && a < 0.00001f) continue;

        float f = 1.0f / a;
        glm::vec3 s = (pos + glm::vec3(0, 10.0f, 0)) - tri.v0; // Cast from higher up for stability
        float u = f * glm::dot(s, h);

        if (u < 0.0f || u > 1.0f) continue;

        glm::vec3 q = glm::cross(s, edge1);
        float v = f * glm::dot(glm::vec3(0, -1, 0), q);

        if (v < 0.0f || u + v > 1.0f) continue;

        float t = f * glm::dot(edge2, q);
        if (t > 0.0f) {
            float hit_y = (pos.y + 3.0f) - t;
            if (hit_y > max_y) max_y = hit_y;
        }
    }

    return max_y;
}
void App::spawn_bandit_wave(int count) {
    if (!bandit_base_model) return;
    
    std::default_random_engine generator((unsigned)time(0));
    std::uniform_real_distribution<float> dist_x(-600.0f, 400.0f);
    std::uniform_real_distribution<float> dist_z(-400.0f, 600.0f);

    for (int i = 0; i < count; ++i) {
        auto bandit = std::make_shared<Model>(*bandit_base_model);
        
        // Spawn far away from player
        glm::vec3 spawn_pos;
        int attempts = 0;
        do {
            spawn_pos = glm::vec3(dist_x(generator), -218.0f, dist_z(generator));
            attempts++;
        } while (glm::distance(spawn_pos, playerPos) < 150.0f && attempts < 10);

        bandit->pivot_position = spawn_pos;
        bandit->pivot_position.y = get_ground_height(bandit->pivot_position);
        if (bandit->pivot_position.y < -500.0f) bandit->pivot_position.y = -218.70f;
        
        bandit->scale = glm::vec3(0.04f); // Increased scale
        bandits.push_back(bandit);
        bandit_throw_timers.push_back(2.0f + (float)(rand() % 4)); 
    }
    std::cout << "Wave started with " << count << " bandits.\n";
}
