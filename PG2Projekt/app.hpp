#pragma once

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <filesystem>
#include <vector>
#include <opencv2/opencv.hpp>
#include <glm/glm.hpp>
#include "ShaderProgram.hpp"
#include "Camera.hpp"
#include "Model.hpp"

class App {
public:
    App();
    ~App();

    bool init(GLFWwindow* win);
    bool run();

private:
    // Konstanty
    const float DEFAULT_FOV = 70.0f;

    // GLFW window
    GLFWwindow* window = nullptr;
    int width = 800, height = 600;

    // Shader program
    ShaderProgram shader;

    // Model a kamera
    Model* triangle = nullptr;
    Camera camera;

    // Světlo
    glm::vec3 pointLightPosition;
    GLuint lampVAO = 0;
    Model* sunModel = nullptr;

    // Mapa a objekty
    cv::Mat maze_map;
    std::vector<Model*> maze_walls;
    std::vector<Model*> transparent_bunnies;
    std::vector<GLuint> wall_textures;

    // Projekční matice
    glm::mat4 projection_matrix = glm::mat4(1.0f);

    // Proměnné pro pohyb kamery
    float lastX, lastY;
    bool firstMouse;
    float fov;

    // Inicializace a asset management
    void init_assets();
    void createMazeModel();
    void createTransparentBunnies();
    void createSunModel();
    GLuint textureInit(const std::filesystem::path& filepath);
    GLuint gen_tex(cv::Mat& image);
    uchar getmap(cv::Mat& map, int x, int y);
    void genLabyrinth(cv::Mat& map);
    void update_projection_matrix();

    // Callback funkce (statické, předávající volání instanci třídy)
    static void fbsize_callback(GLFWwindow* window, int width, int height);
    static void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
    static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
    static void cursor_position_callback(GLFWwindow* window, double xpos, double ypos);
    static void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
};