#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <GLFW/glfw3.h>
#include <vector>
#include <cstdlib>
#include <cmath>
#include <limits>
#include <omp.h>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

struct Point
{
    int x, y;
    unsigned char r, g, b;
};


GLuint LoadTextureFromMemory(const std::vector<unsigned char>& img, int width, int height)
{
    GLuint texture;
    glGenTextures(1, &texture);

    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, img.data());

    return texture;
}


GLuint LoadTextureFromFile(const char* filename, int& width, int& height)
{
    GLuint texture;
    glGenTextures(1, &texture);

    int n;
    unsigned char* data = stbi_load(filename, &width, &height, &n, 0);
    if (data)
    {
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
    }
    stbi_image_free(data);

    return texture;
}

std::vector<unsigned char> generateVoronoiDiagram(int width, int height, int numPoints, int seed)
{
    srand(seed);

    std::vector<unsigned char> img(width * height * 3, 255);

    std::vector<Point> points;
    for (int i = 0; i < numPoints; ++i)
    {
        points.push_back({rand() % width, rand() % height, static_cast<unsigned char>(rand() % 256), static_cast<unsigned char>(rand() % 256), static_cast<unsigned char>(rand() % 256)});
    }

#pragma omp parallel for
    for (int y = 0; y < height; ++y)
    {
        for (int x = 0; x < width; ++x)
        {
            Point p{x, y};
            float minDist = std::numeric_limits<float>::max();
            Point closestPoint;

            for (const Point &point : points)
            {
                float dist = std::pow(p.x - point.x, 2) + std::pow(p.y - point.y, 2);
                if (dist < minDist)
                {
                    minDist = dist;
                    closestPoint = point;
                }
            }

            img[(y * width + x) * 3 + 0] = closestPoint.r;
            img[(y * width + x) * 3 + 1] = closestPoint.g;
            img[(y * width + x) * 3 + 2] = closestPoint.b;
        }
    }

    return img;
}

int main()
{
    // Setup window
    glfwInit();
    GLFWwindow* window = glfwCreateWindow(1280, 720, "ImGuiVoronoiGen", NULL, NULL);
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // Enable vsync

    // Setup ImGui context
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;

    // Setup ImGui style
    ImGui::StyleColorsDark();

    // Setup Platform/Renderer bindings
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 130");

    // Our state
    int width = 1000;
    int height = 1000;
    int numPoints = 100;
    int seed = 0;

    GLuint my_image_texture = 0;
    // Main loop
    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();

        // Start the ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        {
            ImGui::Begin("ImGuiVoronoiGen");

            ImGui::InputInt("Width", &width);
            ImGui::InputInt("Height", &height);
            ImGui::InputInt("Number of points", &numPoints);
            ImGui::InputInt("Seed", &seed);

            if (ImGui::Button("Generate"))
            {
                std::vector<unsigned char> img = generateVoronoiDiagram(width, height, numPoints, seed);
                //stbi_write_png("voronoi.png", width, height, 3, img.data(), width * 3);
                //my_image_texture = LoadTextureFromFile("voronoi.png", width, height);
                my_image_texture = LoadTextureFromMemory(img, width, height);
            }

            ImVec2 available = ImGui::GetContentRegionAvail();
             float window_aspect = available.x / available.y;
            float image_aspect = (float)width / (float)height;

            ImVec2 image_size;
            if (window_aspect > image_aspect)
            {
                image_size.x = available.y * image_aspect;
                image_size.y = available.y;
            }
            else
            {
                image_size.x = available.x;
                image_size.y = available.x / image_aspect;
            }

            ImGui::Image((void*)(intptr_t)my_image_texture, image_size);
            ImGui::End();
        }

        // Rendering
        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(0.45f, 0.55f, 0.60f, 1.00f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
    }

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}