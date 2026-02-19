// icp.cpp
// author: JJ

#include <iostream>
#include <opencv2/opencv.hpp>

#include "app.hpp"
// #include <iostream>
// #include <chrono>
// #include <stack>
// #include <random>
// #include "app.hpp"
// // OpenCV (does not depend on GL)
// #include <opencv2\opencv.hpp>

// // OpenGL Extension Wrangler: allow all multiplatform GL functions
// #include <GL/glew.h>
// // WGLEW = Windows GL Extension Wrangler (change for different platform)
// // platform specific functions (in this case Windows)
// #include <GL/wglew.h>

// // GLFW toolkit
// // Uses GL calls to open GL context, i.e. GLEW __MUST__ be first.
// #include <GLFW/glfw3.h>

// // OpenGL math (and other additional GL libraries, at the end)
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
        // app code
        //...
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
