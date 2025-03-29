#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <stdexcept>
#include <string>
#include <fstream>
#include <nlohmann/json.hpp>
#include "app.hpp"

using json = nlohmann::json;

// Globální instance aplikace
App myApp;

// Callback funkce pro klávesy
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    // Ukonèení aplikace klávesou Escape
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GLFW_TRUE);

    // Pøepínání VSync klávesou F10
    if (key == GLFW_KEY_F10 && action == GLFW_PRESS) {
        static bool vsync = false;
        vsync = !vsync;
        glfwSwapInterval(vsync ? 1 : 0);
        std::cout << "VSync: " << (vsync ? "ON" : "OFF") << std::endl;
    }
}

// ÚKOL 2: Implementace naèítání a validace antialiasingu

// Funkce pro vytvoøení výchozí konfigurace
void create_default_config() {
    json config;
    config["window"] = {
        {"width", 800},
        {"height", 600},
        {"title", "OpenGL Maze Demo"}
    };

    // Pøidání konfigurace pro antialiasing
    config["graphics"] = {
        {"antialiasing", {
            {"enabled", false},
            {"samples", 4}
        }}
    };

    std::ofstream file("config.json");
    if (!file.is_open()) {
        std::cerr << "Nelze vytvoøit konfiguraèní soubor!" << std::endl;
        return;
    }
    file << config.dump(4); // Formátovaný JSON se 4 mezerami
}

// Funkce pro naètení konfigurace
json load_config() {
    try {
        std::ifstream file("config.json");
        if (!file.is_open()) {
            std::cout << "Konfiguraèní soubor neexistuje, vytváøím výchozí." << std::endl;
            create_default_config();
            file.open("config.json");
            if (!file.is_open()) {
                throw std::runtime_error("Nelze vytvoøit konfiguraèní soubor");
            }
        }
        return json::parse(file);
    }
    catch (const std::exception& e) {
        std::cerr << "Chyba pøi naèítání konfigurace: " << e.what() << std::endl;
        // Vrátíme výchozí konfiguraci
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

// Funkce pro validaci nastavení antialiasingu
bool validate_antialiasing_settings(const json& config, bool& antialiasing_enabled, int& samples) {
    bool valid = true;

    // Kontrola, zda jsou nastavení antialiasingu v konfiguraci
    if (!config.contains("graphics") || !config["graphics"].contains("antialiasing")) {
        std::cerr << "Varování: Chybí nastavení antialiasingu v konfiguraci." << std::endl;
        antialiasing_enabled = false;
        samples = 0;
        return false;
    }

    // Získání hodnot z konfigurace
    const auto& aa_config = config["graphics"]["antialiasing"];
    antialiasing_enabled = aa_config.value("enabled", false);
    samples = aa_config.value("samples", 0);

    // Kontrola poètu vzorkù pokud je antialiasing povolen
    if (antialiasing_enabled) {
        if (samples <= 1) {
            std::cerr << "Varování: Antialiasing je povolen, ale poèet vzorkù je <= 1. "
                << "Nastavuji poèet vzorkù na 4." << std::endl;
            samples = 4;
            valid = false;
        }
        else if (samples > 8) {
            std::cerr << "Varování: Pøíliš vysoký poèet vzorkù antialiasingu (> 8). "
                << "Nastavuji poèet vzorkù na 8." << std::endl;
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

        // ÚKOL 2: Naètení konfigurace a nastavení antialiasingu
        json config = load_config();
        int width = config["window"]["width"];
        int height = config["window"]["height"];
        std::string title = config["window"]["title"];

        // Validace a nastavení antialiasingu
        bool antialiasing_enabled;
        int samples;
        validate_antialiasing_settings(config, antialiasing_enabled, samples);

        // Nastavení OpenGL verze a profilu
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

        // Nastavení antialiasingu podle konfigurace
        if (antialiasing_enabled) {
            std::cout << "Antialiasing povolen, poèet vzorkù: " << samples << std::endl;
            glfwWindowHint(GLFW_SAMPLES, samples);
        }
        else {
            std::cout << "Antialiasing vypnut" << std::endl;
            glfwWindowHint(GLFW_SAMPLES, 0);
        }

        // Vytvoøení okna
        GLFWwindow* window = glfwCreateWindow(width, height, title.c_str(), NULL, NULL);
        if (!window) {
            glfwTerminate();
            throw std::runtime_error("Failed to create GLFW window");
        }

        // Aktivace OpenGL kontextu
        glfwMakeContextCurrent(window);

        // Vypnutí VSync pro maximální FPS
        glfwSwapInterval(0);

        // Inicializace GLEW
        glewExperimental = GL_TRUE;
        GLenum err = glewInit();
        if (err != GLEW_OK) {
            glfwTerminate();
            throw std::runtime_error(std::string("Failed to initialize GLEW: ") +
                (const char*)glewGetErrorString(err));
        }

        // Registrace callback funkce pro klávesy
        glfwSetKeyCallback(window, key_callback);

        // Povolení multisamplingu, pokud je antialiasing aktivní
        if (antialiasing_enabled) {
            glEnable(GL_MULTISAMPLE);
        }

        // Inicializace aplikace
        if (!myApp.init(window)) {
            glfwTerminate();
            throw std::runtime_error("Failed to initialize application");
        }

        // Spuštìní hlavní smyèky
        myApp.run();

        // Úklid
        glfwTerminate();
        return 0;
    }
    catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        glfwTerminate();
        return -1;
    }
}