#pragma once
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <vector>
#include <opencv2/opencv.hpp>
#include "assets.hpp"
#include "ShaderProgram.hpp"
#include "Model.hpp"
#include "Camera.hpp"

class App {
public:
    App();
    ~App();
    bool init(GLFWwindow* window);
    void init_assets();
    bool run();
    // Na�ten� textury z obr�zku pomoc� OpenCV
    GLuint textureInit(const std::filesystem::path& filepath);
    // Callback metody
    static void fbsize_callback(GLFWwindow* window, int width, int height);
    static void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
    static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
    static void cursor_position_callback(GLFWwindow* window, double xpos, double ypos);
    static void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
private:
    ShaderProgram shader;
    Model* triangle{ nullptr };
    // Bludi�t�
    cv::Mat maze_map;
    std::vector<Model*> maze_walls;
    std::vector<GLuint> wall_textures;

    GLuint lampVAO;
    glm::vec3 pointLightPosition;

    // P�ID�NO PRO �KOL 1: Transparentn� kr�l�ci
    std::vector<Model*> transparent_bunnies;
    void createTransparentBunnies();

    // Metody pro pr�ci s bludi�t�m
    void genLabyrinth(cv::Mat& map);
    uchar getmap(cv::Mat& map, int x, int y);
    void createMazeModel();

    GLFWwindow* window{ nullptr };
    // Projek�n� matice a souvisej�c� hodnoty
    int width{ 800 }, height{ 600 };
    float fov{ 60.0f };
    const float DEFAULT_FOV = 60.0f;
    glm::mat4 projection_matrix{ glm::identity<glm::mat4>() };
    // Kamera
    Camera camera{ glm::vec3(0.0f, 0.0f, 3.0f) };
    double lastX{ 400.0 }, lastY{ 300.0 }; // Posledn� pozice kurzoru
    bool firstMouse{ true };             // Prom�nn� pro inicializaci pozice kurzoru
    // Metoda pro aktualizaci projek�n� matice
    void update_projection_matrix();
    // Pomocn� metoda pro generov�n� OpenGL textury z OpenCV obr�zku
    GLuint gen_tex(cv::Mat& image);
};