#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <stdexcept>
#include <string>
#include <fstream>
#include <nlohmann/json.hpp>
#include "app.hpp"

using json = nlohmann::json;

// Glob�ln� instance aplikace
App myApp;

// Callback funkce pro kl�vesy
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    // Ukon�en� aplikace kl�vesou Escape
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GLFW_TRUE);

    // P�ep�n�n� VSync kl�vesou F10
    if (key == GLFW_KEY_F10 && action == GLFW_PRESS) {
        static bool vsync = false;
        vsync = !vsync;
        glfwSwapInterval(vsync ? 1 : 0);
        std::cout << "VSync: " << (vsync ? "ON" : "OFF") << std::endl;
    }
}

// �KOL 2: Implementace na��t�n� a validace antialiasingu

// Funkce pro vytvo�en� v�choz� konfigurace
void create_default_config() {
    json config;
    config["window"] = {
        {"width", 800},
        {"height", 600},
        {"title", "OpenGL Maze Demo"}
    };

    // P�id�n� konfigurace pro antialiasing
    config["graphics"] = {
        {"antialiasing", {
            {"enabled", false},
            {"samples", 4}
        }}
    };

    std::ofstream file("config.json");
    if (!file.is_open()) {
        std::cerr << "Nelze vytvo�it konfigura�n� soubor!" << std::endl;
        return;
    }
    file << config.dump(4); // Form�tovan� JSON se 4 mezerami
}

// Funkce pro na�ten� konfigurace
json load_config() {
    try {
        std::ifstream file("config.json");
        if (!file.is_open()) {
            std::cout << "Konfigura�n� soubor neexistuje, vytv���m v�choz�." << std::endl;
            create_default_config();
            file.open("config.json");
            if (!file.is_open()) {
                throw std::runtime_error("Nelze vytvo�it konfigura�n� soubor");
            }
        }
        return json::parse(file);
    }
    catch (const std::exception& e) {
        std::cerr << "Chyba p�i na��t�n� konfigurace: " << e.what() << std::endl;
        // Vr�t�me v�choz� konfiguraci
        json default_config;
        default_config["window"] = {
            {"width", 800},
            {"height", 600},
            {"title", "OpenGL Maze Demo"}
        };
        default_config["graphics"] = {
            {"antialiasing", {
                {"enabled", false},
                {"samples", 4}
            }}
        };
        return default_config;
    }
}

// Funkce pro validaci nastaven� antialiasingu
bool validate_antialiasing_settings(const json& config, bool& antialiasing_enabled, int& samples) {
    bool valid = true;

    // Kontrola, zda jsou nastaven� antialiasingu v konfiguraci
    if (!config.contains("graphics") || !config["graphics"].contains("antialiasing")) {
        std::cerr << "Varov�n�: Chyb� nastaven� antialiasingu v konfiguraci." << std::endl;
        antialiasing_enabled = false;
        samples = 0;
        return false;
    }

    // Z�sk�n� hodnot z konfigurace
    const auto& aa_config = config["graphics"]["antialiasing"];
    antialiasing_enabled = aa_config.value("enabled", false);
    samples = aa_config.value("samples", 0);

    // Kontrola po�tu vzork� pokud je antialiasing povolen
    if (antialiasing_enabled) {
        if (samples <= 1) {
            std::cerr << "Varov�n�: Antialiasing je povolen, ale po�et vzork� je <= 1. "
                << "Nastavuji po�et vzork� na 4." << std::endl;
            samples = 4;
            valid = false;
        }
        else if (samples > 8) {
            std::cerr << "Varov�n�: P��li� vysok� po�et vzork� antialiasingu (> 8). "
                << "Nastavuji po�et vzork� na 8." << std::endl;
            samples = 8;
            valid = false;
        }
    }

    return valid;
}

int main() {
    try {
        // Inicializace GLFW
        if (!glfwInit()) {
            throw std::runtime_error("Failed to initialize GLFW");
        }

        // �KOL 2: Na�ten� konfigurace a nastaven� antialiasingu
        json config = load_config();
        int width = config["window"]["width"];
        int height = config["window"]["height"];
        std::string title = config["window"]["title"];

        // Validace a nastaven� antialiasingu
        bool antialiasing_enabled;
        int samples;
        validate_antialiasing_settings(config, antialiasing_enabled, samples);

        // Nastaven� OpenGL verze a profilu
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

        // Nastaven� antialiasingu podle konfigurace
        if (antialiasing_enabled) {
            std::cout << "Antialiasing povolen, po�et vzork�: " << samples << std::endl;
            glfwWindowHint(GLFW_SAMPLES, samples);
        }
        else {
            std::cout << "Antialiasing vypnut" << std::endl;
            glfwWindowHint(GLFW_SAMPLES, 0);
        }

        // Vytvo�en� okna
        GLFWwindow* window = glfwCreateWindow(width, height, title.c_str(), NULL, NULL);
        if (!window) {
            glfwTerminate();
            throw std::runtime_error("Failed to create GLFW window");
        }

        // Aktivace OpenGL kontextu
        glfwMakeContextCurrent(window);

        // Vypnut� VSync pro maxim�ln� FPS
        glfwSwapInterval(0);

        // Inicializace GLEW
        glewExperimental = GL_TRUE;
        GLenum err = glewInit();
        if (err != GLEW_OK) {
            glfwTerminate();
            throw std::runtime_error(std::string("Failed to initialize GLEW: ") +
                (const char*)glewGetErrorString(err));
        }

        // Registrace callback funkce pro kl�vesy
        glfwSetKeyCallback(window, key_callback);

        // Povolen� multisamplingu, pokud je antialiasing aktivn�
        if (antialiasing_enabled) {
            glEnable(GL_MULTISAMPLE);
        }

        // Inicializace aplikace
        if (!myApp.init(window)) {
            glfwTerminate();
            throw std::runtime_error("Failed to initialize application");
        }

        // Spu�t�n� hlavn� smy�ky
        myApp.run();

        // �klid
        glfwTerminate();
        return 0;
    }
    catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        glfwTerminate();
        return -1;
    }
}