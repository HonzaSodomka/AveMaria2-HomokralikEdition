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
#include "ParticleSystem.hpp"
#include "TextRenderer.hpp" // Přidáno: Include pro TextRenderer

// Struktura pro směrové světlo
struct DirectionalLight {
    glm::vec3 direction;
    glm::vec3 ambient;
    glm::vec3 diffuse;
    glm::vec3 specular;
};

class App {
public:
    App();
    ~App();
    bool init(GLFWwindow* window);
    void init_assets();
    bool run();
    // Načtení textury z obrázku pomocí OpenCV
    GLuint textureInit(const std::filesystem::path& filepath);
    // Callback metody
    static void fbsize_callback(GLFWwindow* window, int width, int height);
    static void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
    static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
    static void cursor_position_callback(GLFWwindow* window, double xpos, double ypos);
    static void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);

    void createFountain();
    // Metoda pro detekci kolizí
    bool checkCollision(const glm::vec3& position, float radius = 0.3f);

    // Nová metoda pro přepínání celoobrazovkového/okenního režimu
    void toggleFullscreen();

    // Metoda pro uložení konfigurace (pozice a velikost okna)
    void saveWindowConfig();

    // Přidáno: Metoda pro zobrazení FPS
    void renderFPS(int fps);

private:
    ShaderProgram shader;
    ShaderProgram lightingShader; // Nový shader pro osvětlení

    Model* triangle{ nullptr };
    // Bludiště
    cv::Mat maze_map;
    std::vector<Model*> maze_walls;
    std::vector<GLuint> wall_textures;

    // Transparentní králíci
    std::vector<Model*> transparent_bunnies;
    void createTransparentBunnies();

    // Osvětlení
    DirectionalLight dirLight; // Směrové světlo (slunce)
    Model* sunModel{ nullptr }; // Model slunce
    void initLighting();      // Inicializace osvětlení
    void updateLighting(float deltaTime); // Aktualizace parametrů osvětlení v čase
    void setupLightingUniforms(); // Nastavení uniforms pro osvětlení
    void createSunModel();    // Vytvoření modelu slunce

    // Metody pro práci s bludištěm
    void genLabyrinth(cv::Mat& map);
    uchar getmap(cv::Mat& map, int x, int y);
    void createMazeModel();

    GLFWwindow* window{ nullptr };
    // Projekční matice a související hodnoty
    int width{ 800 }, height{ 600 };
    float fov{ 60.0f };
    const float DEFAULT_FOV = 60.0f;
    glm::mat4 projection_matrix{ glm::identity<glm::mat4>() };
    // Kamera
    Camera camera{ glm::vec3(0.0f, 0.0f, 3.0f) };
    double lastX{ 400.0 }, lastY{ 300.0 }; // Poslední pozice kurzoru
    bool firstMouse{ true };             // Proměnná pro inicializaci pozice kurzoru
    // Metoda pro aktualizaci projekční matice
    void update_projection_matrix();
    // Pomocná metoda pro generování OpenGL textury z OpenCV obrázku
    GLuint gen_tex(cv::Mat& image);

    Model* particleModel;
    ParticleSystem* fountain;

    // Proměnné pro správu celoobrazovkového režimu
    bool isFullscreen{ false };
    int windowedX{ 100 };     // Pozice okna X před přepnutím do celoobrazovkového režimu
    int windowedY{ 100 };     // Pozice okna Y před přepnutím do celoobrazovkového režimu
    int windowedWidth{ 800 }; // Šířka okna před přepnutím do celoobrazovkového režimu
    int windowedHeight{ 600 }; // Výška okna před přepnutím do celoobrazovkového režimu

    // Přidáno: Objekt pro vykreslování textu
    TextRenderer textRenderer;
    // Přidáno: Proměnná pro zobrazování FPS
    bool showFPS{ true };
};