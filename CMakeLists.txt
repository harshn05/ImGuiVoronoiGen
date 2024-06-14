cmake_minimum_required(VERSION 3.8)
project(VoronoiGenerator)

set(CMAKE_CXX_STANDARD 11)



set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})


set(STB_IMAGE_WRITE_URL "https://raw.githubusercontent.com/nothings/stb/master/stb_image_write.h")
set(STB_IMAGE_WRITE_FILE "${CMAKE_CURRENT_SOURCE_DIR}/stb_image_write.h")

if(NOT EXISTS ${STB_IMAGE_WRITE_FILE})
    file(DOWNLOAD ${STB_IMAGE_WRITE_URL} ${STB_IMAGE_WRITE_FILE}
         SHOW_PROGRESS
         STATUS status
         LOG log)

    list(GET status 0 status_code)
    list(GET status 1 status_string)

    if(NOT status_code EQUAL 0)
        message(FATAL_ERROR "Error downloading file: ${status_string}")
    endif()
endif()

set(STB_IMAGE_URL "https://raw.githubusercontent.com/nothings/stb/master/stb_image.h")
set(STB_IMAGE_FILE "${CMAKE_CURRENT_SOURCE_DIR}/stb_image.h")

if(NOT EXISTS ${STB_IMAGE_FILE})
    file(DOWNLOAD ${STB_IMAGE_URL} ${STB_IMAGE_FILE}
         SHOW_PROGRESS
         STATUS status
         LOG log)

    list(GET status 0 status_code)
    list(GET status 1 status_string)

    if(NOT status_code EQUAL 0)
        message(FATAL_ERROR "Error downloading file: ${status_string}")
    endif()
endif()

find_package(OpenGL REQUIRED)
find_package(OpenMP REQUIRED)
#FOR GLFW
include(ExternalProject)
ExternalProject_Add(
  GLFW_LIB
  GIT_REPOSITORY https://github.com/glfw/glfw.git
  GIT_TAG master
  SOURCE_DIR "${CMAKE_SOURCE_DIR}/glfw"  
  CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${CMAKE_BINARY_DIR}  -DCMAKE_BUILD_TYPE=Release -DGLFW_BUILD_TESTS=FALSE -DGLFW_BUILD_EXAMPLES=FALSE -DGLFW_BUILD_DOCS=FALSE
)

include_directories(${CMAKE_BINARY_DIR}/include)
link_directories(${CMAKE_BINARY_DIR}/lib)


execute_process(COMMAND git clone --depth 1 https://github.com/ocornut/imgui.git -b docking)



set(IMGUI_DIR ${CMAKE_CURRENT_SOURCE_DIR}/imgui)
include_directories(${IMGUI_DIR}  ${IMGUI_DIR}/backends)

file(GLOB IMGUI_SRC 
    "${IMGUI_DIR}/*.cpp"
    "${IMGUI_DIR}/*.h")
    
set(IMGUI_BACKEND_SRC
    ${IMGUI_DIR}/backends/imgui_impl_glfw.h
    ${IMGUI_DIR}/backends/imgui_impl_glfw.cpp
    ${IMGUI_DIR}/backends/imgui_impl_opengl3.h
    ${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp)

add_library(imgui ${IMGUI_SRC} ${IMGUI_BACKEND_SRC})
add_dependencies(imgui GLFW_LIB)

add_library(tinyfiledialogs tinyfiledialogs.h tinyfiledialogs.c)

add_executable(ImGuiVoronoiGen tinyfiledialogs.c main.cpp)
add_dependencies(ImGuiVoronoiGen GLFW_LIB)

target_link_libraries(ImGuiVoronoiGen tinyfiledialogs ${OPENGL_gl_LIBRARY} glfw3 imgui OpenMP::OpenMP_CXX)

include_directories(${OPENGL_INCLUDE_DIR})