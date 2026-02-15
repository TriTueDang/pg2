cmake_minimum_required(VERSION 3.30)

project(pg2 VERSION 2025.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(fmt CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)
find_package(GLEW REQUIRED)
find_package(glfw3 CONFIG REQUIRED)

set(PROJECT_SOURCES
    helloworld.cpp
)

add_executable(${PROJECT_NAME} ${PROJECT_SOURCES})

target_link_libraries(${PROJECT_NAME} PRIVATE
    fmt::fmt
    GLEW::GLEW
    glfw
    glm::glm
    nlohmann_json::nlohmann_json
)



