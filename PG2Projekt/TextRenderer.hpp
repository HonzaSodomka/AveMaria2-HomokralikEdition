#pragma once

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <map>
#include <string>
#include <opencv2/opencv.hpp>
#include "ShaderProgram.hpp"

// Třída pro vykreslování textu v OpenGL
class TextRenderer {
public:
    // Konstruktor
    TextRenderer();

    // Destruktor
    ~TextRenderer();

    // Inicializace rendereru s konkrétní velikostí okna
    bool init(int screenWidth, int screenHeight);

    // Vykreslení textu
    void renderText(const std::string& text, float x, float y, float scale, glm::vec3 color = glm::vec3(1.0f, 1.0f, 1.0f));

    // Aktualizace velikosti okna (při resize)
    void updateScreenSize(int screenWidth, int screenHeight);

private:
    struct Character {
        GLuint textureID;   // ID textury znaku
        glm::ivec2 size;    // Velikost znaku
        glm::ivec2 bearing; // Offset od baseline k levému hornímu rohu
        GLuint advance;     // Offset pro další znak
    };

    // Shader program pro vykreslování textu
    ShaderProgram textShader;

    // Mapa znaků (písmena a jejich vlastnosti)
    std::map<char, Character> characters;

    // VAO a VBO pro vykreslování textu
    GLuint VAO, VBO;

    // Rozměry okna
    int width, height;

    // Vytvoření základní bitmapy fontu pomocí OpenCV (jednoduché řešení bez FreeType)
    void createSimpleFont();

    // Generování textury znaku
    GLuint generateCharTexture(const cv::Mat& charImage);
};