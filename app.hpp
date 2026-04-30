// app.hpp
// Public interface for the application class
// author: JJ

#pragma once

// Standard headers
#include <vector>   // dynamic arrays used by vertex list
#include <string>   // std::string for window title

// Third-party headers
#include "GLFW/glfw3.h"   // GLFW types in callbacks

// Project headers
#include "assets.hpp"     // vertex definition, assets management
#include "ShaderProgram.hpp"
#include "Model.hpp"
#include <memory>

#include "camera.hpp"
#include "Physics.hpp"
#include "Spline.hpp"

// Light data structures
struct DirectionalLight {
    glm::vec3 direction = glm::vec3(0.0f, -1.0f, -1.0f); // Light direction
    glm::vec3 ambient = glm::vec3(0.2f, 0.2f, 0.2f);
    glm::vec3 diffuse = glm::vec3(0.8f, 0.8f, 0.8f);
    glm::vec3 specular = glm::vec3(1.0f, 1.0f, 1.0f);
};

struct PointLight {
    glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f);
    glm::vec3 ambient = glm::vec3(0.2f, 0.2f, 0.2f);
    glm::vec3 diffuse = glm::vec3(0.8f, 0.8f, 0.8f);
    glm::vec3 specular = glm::vec3(1.0f, 1.0f, 1.0f);
    float radius = 100.0f; // Attenuation radius
};

struct SpotLight {
    glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f);
    glm::vec3 direction = glm::vec3(0.0f, -1.0f, 0.0f);
    glm::vec3 ambient = glm::vec3(0.2f, 0.2f, 0.2f);
    glm::vec3 diffuse = glm::vec3(0.8f, 0.8f, 0.8f);
    glm::vec3 specular = glm::vec3(1.0f, 1.0f, 1.0f);
    float cutoff = 12.5f; // Inner cone angle
    float outer_cutoff = 17.5f; // Outer cone angle
};

class App {

protected:
    // projection related variables
    int width{0}, height{0};
    float fov = 60.0f;
    // store projection matrix here, update only on callbacks
    glm::mat4 projection_matrix = glm::identity<glm::mat4>();
    // all objects of the scene
    std::unordered_map<std::string, Model> scene;

    // Mouse control (Task 2, point 4)
    glm::vec3 camera_front{ 0.0f, 0.0f, -1.0f };
    float yaw = -90.0f;
    float pitch = 0.0f;
    float lastX = 400, lastY = 300;
    bool firstMouse = true;

    glm::mat4 view_matrix = glm::identity<glm::mat4>();

    Camera camera;
    // remember last cursor position, move relative to that in the next frame
    double cursorLastX{ 0 };
    double cursorLastY{ 0 };


private:
    bool show_imgui{true};

    int window_width = 800;
    int window_height = 600;
    std::string window_title = "OpenGL context ";
    bool is_vsync_on = true;
    //new GL stuff
    bool fullscreen_enabled = false;
    int saved_window_x = 0;
    int saved_window_y = 0;
    int saved_window_width = 800;
    int saved_window_height = 600;

    bool is_multisample_on = true;

    void toggle_fullscreen();
    void take_screenshot();

    GLFWwindow* window = nullptr;

    std::shared_ptr<ShaderProgram> shader_prog;
    std::shared_ptr<Model> city_model;
    std::shared_ptr<Model> player_model;
    std::vector<std::shared_ptr<Model>> bandits;
    
    // cv07 & cv08 Shaders
    std::shared_ptr<ShaderProgram> skybox_shader;
    std::shared_ptr<ShaderProgram> post_process_shader;
    std::shared_ptr<ShaderProgram> billboard_shader;

    // FBO (cv07)
    GLuint fbo = 0;
    GLuint fbo_texture = 0;
    GLuint rbo = 0;

    // Skybox (cv08)
    GLuint skybox_vao = 0;
    GLuint skybox_vbo = 0;
    GLuint skybox_texture = 0;

    // Billboards (cv08)
    struct Billboard {
        glm::vec3 position;
        glm::vec2 scale;
        glm::vec3 tint;
    };

    struct BillboardInstance {
        glm::vec4 worldPos; // xyz=pos, w=scaleX
        glm::vec4 tint_scaleY; // rgb=tint, a=scaleY
    };

    struct BanditInstance {
        glm::vec4 pos;      // xyz = position, w = unused
        glm::vec4 rotScale; // xyz = eulerAngles, w = scale
    };

    std::vector<Billboard> billboards;
    std::shared_ptr<Texture> billboard_tex;
    GLuint billboard_vao = 0;
    GLuint billboard_vbo = 0;
    GLuint billboard_ssbo = 0;

    // Lighting
    DirectionalLight dir_light;
    std::vector<PointLight> point_lights;
    std::vector<SpotLight> spot_lights;
    int active_light_type = 0; // 0 = directional, 1 = point
    int active_light_index = 0; // For point and spot lights

    // Application state
    float bg_r = 0.1f, bg_g = 0.1f, bg_b = 0.15f;
    bool fps_mode = true;
    float ground_height = -218.70f;
    float walk_anim_time = 0.0f;
    bool is_moving = false;
    bool show_post_process = true;
    float shoot_anim_time = 0.0f;
    glm::vec3 playerPos = glm::vec3(-121.64f, -218.70f, 63.23f);

    // Gameplay
    float player_health = 100.0f;
    bool is_player_dead = false;
    float bandit_chase_dist = 50.0f;
    float bandit_attack_dist = 4.0f;
    float bandit_damage_rate = 30.0f; // health per second
    float bandit_speed = 16.0f;


    // Physics
    float velocity_y = 0.0f;
    bool is_on_ground = false;
    const float gravity = -25.0f;
    const float jump_force = 15.0f;

    // BVH Physics Engine
    PG2::PhysicsEngine physics;
    glm::vec3 last_safe_pos = glm::vec3(-121.64f, -218.70f, 63.23f);

    void build_physics();
    float get_ground_height(glm::vec3 pos, float ray_depth = 500.0f);

    void spawn_bandit_wave(int count);
    void spawn_whiskey_pickups();
    std::vector<int> bandit_health;

    // Gameplay Objects
    enum class AIState { CHASE, SEEK_COVER, PATROL, SHOOTING };
    std::vector<AIState> bandit_states;
    std::vector<glm::vec3> bandit_target_positions;
    std::vector<float> bandit_state_timers;

    int wave_number = 1;
    int frame_count = 0;
    struct Dynamite {
        glm::vec3 position;
        glm::vec3 velocity;
        float timer = 2.0f;
        bool on_ground = false;
    };
    int score = 0;
    std::vector<float> bandit_shoot_timers;
    std::vector<Dynamite> active_dynamites;


    std::shared_ptr<Model> dynamite_model;
    std::shared_ptr<Model> bandit_base_model;
    std::vector<float> bandit_throw_timers;
    std::vector<float> bandit_velocities_y;
    std::vector<glm::vec3> bandit_safe_positions;
    std::vector<glm::vec3> bandit_last_positions;
    std::vector<float> bandit_stuck_timers;

    struct Bullet {
        glm::vec3 position;
        glm::vec3 velocity;
        float life = 3.0f;
        bool isFromPlayer = true;
    };

    struct Particle {
        glm::vec3 position;
        glm::vec3 velocity;
        glm::vec4 color;
        float life; // 1.0 to 0.0
        float size;
    };

    struct ParticleInstance {
        glm::vec4 pos_size;  // (x, y, z, size)
        glm::vec4 color_life; // (r, g, b, life)
    };

    std::vector<Particle> active_particles;
    std::shared_ptr<ShaderProgram> particle_shader;
    
    GLuint particle_vbo = 0;
    ParticleInstance* mapped_particles = nullptr;
    const int MAX_PARTICLES = 2000;

    void spawn_particles(glm::vec3 pos, glm::vec3 color, int count, float size = 0.5f);

    struct WhiskeyPickup {
        glm::vec3 position;
        float rotation = 0.0f;
        bool active = true;
    };
    std::vector<WhiskeyPickup> whiskey_pickups;
    float whiskey_respawn_timer = 0.0f;
    void spawn_single_whiskey();
    std::shared_ptr<Model> whiskey_model;

    std::vector<Bullet> active_bullets;
    std::shared_ptr<Model> bullet_model;

    float bandit_throw_cooldown = 4.0f;
    const float dynamite_damage = 40.0f;
    const float dynamite_radius = 12.0f;

    // --- Výkonnostní optimalizace ---
    GLuint bandit_ssbo = 0;
    struct Frustum {
        glm::vec4 planes[6];
    };
    Frustum extract_frustum(const glm::mat4& viewProj);
    bool is_inside_frustum(const Frustum& f, glm::vec3 pos, float radius);

    // Cinematic Camera (cv09)
    enum class AppCameraState { GAMEPLAY, CINEMATIC, TRANSITION };
    AppCameraState cam_state = AppCameraState::CINEMATIC;
    PG2::CatmullRomSpline intro_spline;
    float intro_time = 0.0f;
    float intro_duration = 12.0f; // seconds
    bool intro_done = false; // Persistent flag to prevent replay on window resize
    
    struct {
        glm::vec3 start_pos, end_pos;
        glm::vec3 start_front, end_front;
        float progress = 0.0f;
        float duration = 1.5f; // seconds
    } cam_transition;
    
    bool is_first_wave = true;
    float invulnerability_timer = 0.0f;
    float wave_info_timer = 0.0f;

    // Performance Optimization Cache
    std::vector<std::string> point_light_pos_names;
    std::vector<std::string> point_light_amb_names;
    std::vector<std::string> point_light_diff_names;
    std::vector<std::string> point_light_spec_names;

    // cv07 & cv08 helpers
    void init_fbo();
    void init_skybox();
    void init_billboards();
    void render_skybox();
    void render_billboards();
    GLuint post_process_vao = 0;
    GLuint msaa_fbo = 0, msaa_color_rbo = 0, msaa_depth_rbo = 0;
    bool msaa_dirty = false; // Flag to recreate FBOs if MSAA state changes
    void render_post_process();
    void render_particles();
    GLuint load_cubemap(std::vector<std::string> faces);

    // initialization helpers
    void init_imgui(void);          // set up ImGUI context and bindings
    void init_opencv(void);        // placeholder for OpenCV setup
    void init_glfw(void);          // initialize GLFW and create window
    void init_glew(void);          // initialize GLEW and check extensions
    void init_gl_debug(void);      // enable OpenGL debug callbacks

    // information display routines
    void print_opencv_info(void);  // log OpenCV version
    void print_glfw_info(void);    // log GLFW version
    void print_glm_info(void);     // log GLM version (if available)
    void print_gl_info(void);      // log GPU/GL driver info

public:
    App();

    bool init(void);
    void init_assets(void);
    bool load_config(const std::string& filename);
    int run(void);

    // Callbacks
    static void glfw_key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
    static void glfw_fbsize_callback(GLFWwindow* window, int width, int height);
    static void glfw_mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
    static void glfw_cursor_position_callback(GLFWwindow* window, double xpos, double ypos);
    static void cursorPositionCallback(GLFWwindow* window, double xpos, double ypos);
    static void glfw_scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
    void update_projection_matrix(void);

    void destroy(void);
    ~App();
private:
};

