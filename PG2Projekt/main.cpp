#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include "app.hpp"

App myApp;

// Callback funkce pro klávesy
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GLFW_TRUE);

    // Zmáèknutí kláves R, G, B pro zmìnu barvy
    if ((key == GLFW_KEY_R || key == GLFW_KEY_G || key == GLFW_KEY_B) &&
        (action == GLFW_PRESS || action == GLFW_REPEAT)) {
        myApp.change_color(key);
    }

    // Pøidáno: Pøepínání VSync klávesou F10
    if (key == GLFW_KEY_F10 && action == GLFW_PRESS) {
        static bool vsync = false; // Zaèínáme s vypnutým VSync
        vsync = !vsync;
        glfwSwapInterval(vsync ? 1 : 0);
        std::cout << "VSync: " << (vsync ? "ON" : "OFF") << std::endl;
    }
}

int main() {
    try {
        // Inicializace GLFW
        if (!glfwInit()) {
            throw std::runtime_error("Failed to initialize GLFW");
        }

        // Nastavení OpenGL verze a profilu
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

        // Vytvoøení okna
        GLFWwindow* window = glfwCreateWindow(800, 600, "OpenGL Triangle Demo", NULL, NULL);
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