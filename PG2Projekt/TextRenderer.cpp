#include "TextRenderer.hpp"
#include <iostream>

TextRenderer::TextRenderer() : width(800), height(600) {
}

TextRenderer::~TextRenderer() {
    // Úklid OpenGL objektů
    for (auto& c : characters) {
        glDeleteTextures(1, &c.second.textureID);
    }
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
}

bool TextRenderer::init(int screenWidth, int screenHeight) {
    // Uložení rozměrů obrazovky
    width = screenWidth;
    height = screenHeight;

    try {
        // Načtení shaderů pro text
        textShader = ShaderProgram("resources/shaders/text.vert", "resources/shaders/text.frag");

        // Nastavení projekční matice (ortogonální projekce)
        glm::mat4 projection = glm::ortho(0.0f, static_cast<float>(width), 0.0f, static_cast<float>(height));
        textShader.activate();
        textShader.setUniform("projection", projection);

        // Vytvoření VAO a VBO pro text
        glCreateVertexArrays(1, &VAO);
        glCreateBuffers(1, &VBO);

        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        // Dynamická alokace pro VBO (budeme často měnit data pro různé znaky)
        glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, NULL, GL_DYNAMIC_DRAW);

        // Nastavení vertex attributů
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);

        // Vytvoření fontů
        createSimpleFont();

        return true;
    }
    catch (const std::exception& e) {
        std::cerr << "Text renderer initialization error: " << e.what() << std::endl;
        return false;
    }
}

void TextRenderer::updateScreenSize(int screenWidth, int screenHeight) {
    width = screenWidth;
    height = screenHeight;

    // Aktualizace projekční matice
    glm::mat4 projection = glm::ortho(0.0f, static_cast<float>(width), 0.0f, static_cast<float>(height));
    textShader.activate();
    textShader.setUniform("projection", projection);
}

// Zjednodušená implementace vytvoření fontu pomocí OpenCV
void TextRenderer::createSimpleFont() {
    const int charSize = 32; // Velikost znaku (pixely)
    const int fontSize = 24; // Velikost fontu

    // Seznam znaků, které chceme vytvořit
    const std::string charset = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789.:,;'\"!?()[]{}";

    for (char c : charset) {
        // Vytvoření prázdného obrázku pro znak
        cv::Mat charImg(charSize, charSize, CV_8UC1, cv::Scalar(0));

        // Vykreslení znaku do obrázku
        int baseline = 0;
        cv::Size textSize = cv::getTextSize(std::string(1, c), cv::FONT_HERSHEY_SIMPLEX, 1.0, 1, &baseline);

        // Centrování znaku v obrázku
        cv::Point textOrg((charSize - textSize.width) / 2, (charSize + textSize.height) / 2);

        // Vykreslení znaku bílou barvou
        cv::putText(charImg, std::string(1, c), textOrg, cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(255), 1, cv::LINE_AA);

        // Generování textury pro znak
        GLuint texture = generateCharTexture(charImg);

        // Vytvoření záznamu pro znak
        Character character = {
            texture,
            glm::ivec2(textSize.width, textSize.height),
            glm::ivec2(0, baseline),
            static_cast<GLuint>(textSize.width + 2) // Přidání malého mezery mezi znaky
        };

        // Uložení znaku do mapy
        characters.insert(std::pair<char, Character>(c, character));
    }
}

// Generování textury pro jeden znak
GLuint TextRenderer::generateCharTexture(const cv::Mat& charImage) {
    GLuint texture;
    glCreateTextures(GL_TEXTURE_2D, 1, &texture);

    // Nastavení texturových parametrů
    glTextureParameteri(texture, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTextureParameteri(texture, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTextureParameteri(texture, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTextureParameteri(texture, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // Alokace a nahrání dat textury
    glTextureStorage2D(texture, 1, GL_R8, charImage.cols, charImage.rows);
    glTextureSubImage2D(texture, 0, 0, 0, charImage.cols, charImage.rows, GL_RED, GL_UNSIGNED_BYTE, charImage.data);

    return texture;
}

// Vykreslení textu na obrazovku
void TextRenderer::renderText(const std::string& text, float x, float y, float scale, glm::vec3 color) {
    // Aktivace shaderu a nastavení uniformů
    textShader.activate();
    textShader.setUniform("textColor", color);

    glActiveTexture(GL_TEXTURE0);
    glBindVertexArray(VAO);

    // Iterace přes všechny znaky textu
    for (char c : text) {
        // Kontrola, zda máme znak v mapě
        if (characters.find(c) == characters.end()) {
            // Pokud ne, pokračujeme dalším znakem
            x += characters['?'].advance * scale; // Použijeme otazník jako náhradu
            continue;
        }

        Character ch = characters[c];

        float xpos = x + ch.bearing.x * scale;
        float ypos = y - (ch.size.y - ch.bearing.y) * scale;

        float w = ch.size.x * scale;
        float h = ch.size.y * scale;

        // Aktualizace VBO pro každý znak
        float vertices[6][4] = {
            { xpos,     ypos + h,   0.0f, 0.0f },
            { xpos,     ypos,       0.0f, 1.0f },
            { xpos + w, ypos,       1.0f, 1.0f },

            { xpos,     ypos + h,   0.0f, 0.0f },
            { xpos + w, ypos,       1.0f, 1.0f },
            { xpos + w, ypos + h,   1.0f, 0.0f }
        };

        // Navázání textury znaku
        glBindTextureUnit(0, ch.textureID);

        // Aktualizace obsahu VBO
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        // Vykreslení obdélníku znaku
        glDrawArrays(GL_TRIANGLES, 0, 6);

        // Posun pozice pro další znak
        x += (ch.advance * scale);
    }

    // Uvolnění VAO a textury
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);

    // Deaktivace shaderu
    textShader.deactivate();
}