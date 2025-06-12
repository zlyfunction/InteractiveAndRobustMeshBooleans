cmake_minimum_required(VERSION 3.21)

# Add nlohmann/json library
# include(FetchContent)
# FetchContent_Declare(
# json
# URL https://github.com/nlohmann/json/releases/download/v3.11.3/json.tar.xz
# )
# FetchContent_MakeAvailable(json)

# Set the target architecture.
# All modern x86/x64 processors support AVX2.
# Older x86/x64 processors may support SSE2 but not AVX2.
# Very old x86/x64 processors, or non x86/x64
# processors, do not support any of the two.
set(ENABLE_SSE2 True)
set(ENABLE_AVX2 True)

# specify the C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# set the project name
project(mesh_booleans)

set(TBB_TEST OFF CACHE BOOL " " FORCE)
set(TBB_EXAMPLES OFF CACHE BOOL " " FORCE)

# add_subdirectory(arrangements/external/oneTBB)
set(cinolib_DIR ${PROJECT_SOURCE_DIR}/arrangements/external/Cinolib)
set(CINOLIB_USES_OPENGL_GLFW_IMGUI OFF)
set(CINOLIB_USES_SHEWCHUK_PREDICATES ON)

find_package(cinolib REQUIRED)

add_executable(${PROJECT_NAME}_arrangement main_arrangement.cpp)

target_include_directories(${PROJECT_NAME}_arrangement PUBLIC
        ./
        code/
        arrangements/code/
        arrangements/external/Indirect_Predicates/include/
)

target_link_libraries(${PROJECT_NAME}_arrangement cinolib)
target_link_libraries(${PROJECT_NAME}_arrangement TBB::tbb)
target_link_libraries(${PROJECT_NAME}_arrangement nlohmann_json::nlohmann_json)

# target_compile_definitions(${PROJECT_NAME}_arrangement PUBLIC TBB_PARALLEL=1)
target_include_directories(${PROJECT_NAME}_arrangement PUBLIC ${PROJECT_SOURCE_DIR}/arrangements/external/abseil-cpp/)

# target_include_directories(${PROJECT_NAME}_arrangement PUBLIC ${PROJECT_SOURCE_DIR}/arrangements/external/oneTBB/)
