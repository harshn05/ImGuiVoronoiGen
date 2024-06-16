#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <GLFW/glfw3.h>
#include <vector>
#include <cstdlib>
#include <cmath>
#include <limits>
#include <omp.h>
#include <string>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "tinyfiledialogs.h"

std::vector<unsigned char> currentImage;
std::string savePath;
std::vector<int> voronoiCellSizes;
std::vector<int> voronoiCells;

struct Point
{
    int x, y;
    unsigned char r, g, b;
};

GLuint LoadTextureFromMemory(const std::vector<unsigned char> &img, int width, int height)
{
    GLuint texture;
    glGenTextures(1, &texture);

    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, img.data());

    return texture;
}

GLuint LoadTextureFromFile(const char *filename, int &width, int &height)
{
    GLuint texture;
    glGenTextures(1, &texture);

    int n;
    unsigned char *data = stbi_load(filename, &width, &height, &n, 0);
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

    voronoiCellSizes.resize(numPoints, 0);
    voronoiCells.resize(width * height, -1);
 
#pragma omp parallel for
    for (int y = 0; y < height; ++y)
    {
        for (int x = 0; x < width; ++x)
        {
            Point p{x, y};
            float minDist = std::numeric_limits<float>::max();
            int closestPointIndex = -1;

            for (int i = 0; i < numPoints; ++i)
            {
                const Point &point = points[i];
                float dist = std::pow(p.x - point.x, 2) + std::pow(p.y - point.y, 2);
                if (dist < minDist)
                {
                    minDist = dist;
                    closestPointIndex = i;
                }
            }

            voronoiCells[y * width + x] = closestPointIndex;
            voronoiCellSizes[closestPointIndex]++;
            const Point &closestPoint = points[closestPointIndex];

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
    GLFWwindow *window = glfwCreateWindow(1280, 720, "ImGuiVoronoiGen", NULL, NULL);
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // Enable vsync

    // Setup ImGui context
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    (void)io;

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
    int styleIdx = 1;

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
            ImGui::Begin("Input Parameters");

            const char *styles[] = {"Classic", "Dark", "Light"};
            ImGui::Combo("Style", &styleIdx, styles, IM_ARRAYSIZE(styles));

            if (styleIdx == 0)
            {
                ImGui::StyleColorsClassic();
            }
            else if (styleIdx == 1)
            {
                ImGui::StyleColorsDark();
            }
            else if (styleIdx == 2)
            {
                ImGui::StyleColorsLight();
            }

            ImGui::InputInt("Width", &width);
            ImGui::InputInt("Height", &height);
            ImGui::InputInt("Number of points", &numPoints);
            ImGui::InputInt("Seed", &seed);

            if (ImGui::Button("Generate"))
            {
                // std::vector<unsigned char> img = generateVoronoiDiagram(width, height, numPoints, seed);
                // stbi_write_png("voronoi.png", width, height, 3, img.data(), width * 3);
                // my_image_texture = LoadTextureFromFile("voronoi.png", width, height);
                // my_image_texture = LoadTextureFromMemory(img, width, height);
                currentImage = generateVoronoiDiagram(width, height, numPoints, seed);
                my_image_texture = LoadTextureFromMemory(currentImage, width, height);
            }

            if (ImGui::Button("Save"))
            {
                const char *filterPatterns[1] = {"*.png"};
                const char *filePath = tinyfd_saveFileDialog("Save Image", "", 1, filterPatterns, NULL);
                if (filePath != NULL)
                {
                    savePath = filePath;
                    stbi_write_png(savePath.c_str(), width, height, 3, currentImage.data(), width * 3);
                }
            }

            if (ImGui::BeginPopupModal("Save Image", NULL, ImGuiWindowFlags_AlwaysAutoResize))
            {
                ImGui::Text("Enter the path where you want to save the image:");
                ImGui::InputTextWithHint("", "Path", &savePath[0], savePath.size() + 1);
                if (ImGui::Button("OK", ImVec2(120, 0)))
                {
                    stbi_write_png(savePath.c_str(), width, height, 3, currentImage.data(), width * 3);
                    ImGui::CloseCurrentPopup();
                }
                ImGui::SetItemDefaultFocus();
                ImGui::EndPopup();
            }
            ImGui::End();
            ImGui::SetNextWindowSize(ImVec2(600, 600), ImGuiCond_FirstUseEver);
            ImGui::Begin("Voronoi Tessellation");
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

            ImGui::Image((void *)(intptr_t)my_image_texture, image_size);

            if (ImGui::IsItemHovered() && !currentImage.empty())
            {
                ImVec2 mouse_pos = ImGui::GetMousePos();
                ImVec2 image_pos = ImGui::GetItemRectMin();
                ImVec2 mouse_pos_in_image = ImVec2(mouse_pos.x - image_pos.x, mouse_pos.y - image_pos.y);
                mouse_pos_in_image.x /= image_size.x;
                mouse_pos_in_image.y /= image_size.y;

                int pixel_x = static_cast<int>(mouse_pos_in_image.x * width);
                int pixel_y = static_cast<int>(mouse_pos_in_image.y * height);

                if (pixel_x >= 0 && pixel_x < width && pixel_y >= 0 && pixel_y < height && (pixel_y * width + pixel_x) * 3 + 2 < currentImage.size())
                {
                    unsigned char r = currentImage[(pixel_y * width + pixel_x) * 3 + 0];
                    unsigned char g = currentImage[(pixel_y * width + pixel_x) * 3 + 1];
                    unsigned char b = currentImage[(pixel_y * width + pixel_x) * 3 + 2];

                    ImGui::BeginTooltip();
                    int cell = voronoiCells[pixel_y * width + pixel_x];
                    int cellSize = voronoiCellSizes[cell];
                    ImGui::Text("Cell id: %d", cell);
                    ImGui::Text("Cell size: %d pixels", cellSize);
                    ImGui::Text("Pixel: (%d, %d)", pixel_x, pixel_y);
                    ImGui::Text("Color: (%d, %d, %d)", r, g, b);
                    ImGui::EndTooltip();
                }
            }
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