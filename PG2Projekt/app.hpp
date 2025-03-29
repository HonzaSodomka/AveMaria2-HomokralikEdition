#pragma once
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <vector>
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

    // Callback metody
    static void fbsize_callback(GLFWwindow* window, int width, int height);
    static void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
    static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
    static void cursor_position_callback(GLFWwindow* window, double xpos, double ypos);
    static void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);

private:
    ShaderProgram shader;
    Model* triangle{ nullptr };

    GLFWwindow* window{ nullptr };

    // Projekèní matice a související hodnoty
    int width{ 800 }, height{ 600 };
    float fov{ 60.0f };
    const float DEFAULT_FOV = 60.0f;
    glm::mat4 projection_matrix{ glm::identity<glm::mat4>() };

    // Kamera
    Camera camera{ glm::vec3(0.0f, 0.0f, 3.0f) };
    double lastX{ 400.0 }, lastY{ 300.0 }; // Poslední pozice kurzoru
    bool firstMouse{ true };             // Promìnná pro inicializaci pozice kurzoru

    // Metoda pro aktualizaci projekèní matice
    void update_projection_matrix();
};