#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <string>
#include <nlohmann/json.hpp>
#include <fstream>

using json = nlohmann::json;

// Vertex shader
const char* vertexShaderSource = R"(
    #version 460 core
    layout (location = 0) in vec3 aPos;
    void main() {
        gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);
    }
)";

// Fragment shader
const char* fragmentShaderSource = R"(
    #version 460 core
    out vec4 FragColor;
    uniform vec3 triangleColor; // Barva trojúhelníku jako uniform
    void main() {
        FragColor = vec4(triangleColor, 1.0f);
    }
)";

// Globální proměnné pro barvu trojúhelníku
float r = 1.0f, g = 0.5f, b = 0.2f;
GLuint shaderProgram;

// Callback funkce pro chyby GLFW
void error_callback(int error, const char* description) {
    std::cerr << "GLFW Error " << error << ": " << description << std::endl;
}

// Callback funkce pro stisk kláves
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GLFW_TRUE);

    // Ovládání barev pomocí kláves
    if (action == GLFW_PRESS || action == GLFW_REPEAT) {
        switch (key) {
        case GLFW_KEY_R: // Zvýšení červené složky
            r += 0.1f;
            if (r > 1.0f) r = 0.0f;
            break;
        case GLFW_KEY_G: // Zvýšení zelené složky
            g += 0.1f;
            if (g > 1.0f) g = 0.0f;
            break;
        case GLFW_KEY_B: // Zvýšení modré složky
            b += 0.1f;
            if (b > 1.0f) b = 0.0f;
            break;
        }
    }
}

// Callback funkce pro změnu velikosti framebuffer
void fbsize_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

// Funkce pro kompilaci shaderů
GLuint compileShader(GLenum type, const char* source) {
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, NULL);
    glCompileShader(shader);

    // Kontrola kompilace
    GLint success;
    GLchar infoLog[512];
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(shader, 512, NULL, infoLog);
        std::cerr << "ERROR::SHADER::COMPILATION_FAILED\n" << infoLog << std::endl;
    }

    return shader;
}

// Funkce pro vytvoření konfiguračního souboru, pokud neexistuje
void create_default_config() {
    json config;
    config["window"] = {
        {"width", 800},
        {"height", 600},
        {"title", "OpenGL Lab 03"}
    };

    std::ofstream file("config.json");
    file << config.dump(4);
}

// Funkce pro načtení konfigurace
json load_config() {
    try {
        std::ifstream file("config.json");
        if (!file.is_open()) {
            create_default_config();
            file.open("config.json");
            if (!file.is_open()) {
                throw std::runtime_error("Failed to create config file");
            }
        }
        return json::parse(file);
    }
    catch (const std::exception& e) {
        std::cerr << "Error loading config: " << e.what() << std::endl;
        // Vrať defaultní konfiguraci
        json default_config;
        default_config["window"] = {
            {"width", 800},
            {"height", 600},
            {"title", "OpenGL Lab 03"}
        };
        return default_config;
    }
}

int main(void) {
    try {
        // Nastavení GLFW error callback
        glfwSetErrorCallback(error_callback);

        // Inicializace GLFW
        if (!glfwInit()) {
            throw std::runtime_error("Failed to initialize GLFW");
        }

        // Načtení konfigurace
        json config = load_config();
        int width = config["window"]["width"];
        int height = config["window"]["height"];
        std::string title = config["window"]["title"];

        // Nastavení OpenGL verze a profilu
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

        // Vytvoření okna
        GLFWwindow* window = glfwCreateWindow(width, height, title.c_str(), NULL, NULL);
        if (!window) {
            glfwTerminate();
            throw std::runtime_error("Failed to create GLFW window");
        }

        // Nastavení kontextu
        glfwMakeContextCurrent(window);
        glfwSwapInterval(1); // Zapnutí VSync

        // Inicializace GLEW
        glewExperimental = GL_TRUE;
        GLenum err = glewInit();
        if (err != GLEW_OK) {
            glfwTerminate();
            throw std::runtime_error(std::string("Failed to initialize GLEW: ") +
                (const char*)glewGetErrorString(err));
        }

        // Kontrola verze OpenGL
        int major, minor;
        glGetIntegerv(GL_MAJOR_VERSION, &major);
        glGetIntegerv(GL_MINOR_VERSION, &minor);
        std::cout << "OpenGL Version: " << major << "." << minor << std::endl;

        // Kontrola profilu
        int profile;
        glGetIntegerv(GL_CONTEXT_PROFILE_MASK, &profile);
        if (profile & GL_CONTEXT_CORE_PROFILE_BIT) {
            std::cout << "OpenGL Core Profile is active" << std::endl;
        }
        else if (profile & GL_CONTEXT_COMPATIBILITY_PROFILE_BIT) {
            std::cout << "OpenGL Compatibility Profile is active" << std::endl;
        }

        // Registrace callback funkcí
        glfwSetKeyCallback(window, key_callback);
        glfwSetFramebufferSizeCallback(window, fbsize_callback);

        // Nastavení viewportu
        int framebufferWidth, framebufferHeight;
        glfwGetFramebufferSize(window, &framebufferWidth, &framebufferHeight);
        glViewport(0, 0, framebufferWidth, framebufferHeight);

        // Kompilace shaderů
        GLuint vertexShader = compileShader(GL_VERTEX_SHADER, vertexShaderSource);
        GLuint fragmentShader = compileShader(GL_FRAGMENT_SHADER, fragmentShaderSource);

        // Vytvoření shader programu
        shaderProgram = glCreateProgram();
        glAttachShader(shaderProgram, vertexShader);
        glAttachShader(shaderProgram, fragmentShader);
        glLinkProgram(shaderProgram);

        // Kontrola linkování programu
        GLint success;
        GLchar infoLog[512];
        glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
        if (!success) {
            glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
            std::cerr << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
        }

        // Uvolnění shaderů (jsou již součástí programu)
        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);

        // Definice vrcholů trojúhelníku
        float vertices[] = {
            -0.5f, -0.5f, 0.0f, // levý spodní
             0.5f, -0.5f, 0.0f, // pravý spodní
             0.0f,  0.5f, 0.0f  // vrchní
        };

        // Vytvoření Vertex Array Object a Vertex Buffer Object
        GLuint VAO, VBO;
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);

        // Vazba VAO
        glBindVertexArray(VAO);

        // Vazba a naplnění VBO
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

        // Nastavení atributů vrcholů
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);

        // Odpojení VAO a VBO
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);

        // Proměnné pro měření FPS
        double lastTime = glfwGetTime();
        int frameCount = 0;

        // Hlavní smyčka
        while (!glfwWindowShouldClose(window)) {
            // Měření FPS
            double currentTime = glfwGetTime();
            frameCount++;
            // Aktualizace FPS každou sekundu
            if (currentTime - lastTime >= 1.0) {
                // Aktualizace titulku
                std::string fpsTitle = title + " | FPS: " + std::to_string(frameCount);
                glfwSetWindowTitle(window, fpsTitle.c_str());

                // Reset čítače
                frameCount = 0;
                lastTime = currentTime;
            }

            // Vyčištění obrazovky
            glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT);

            // Použití shader programu
            glUseProgram(shaderProgram);

            // Nastavení barvy trojúhelníku
            int colorLoc = glGetUniformLocation(shaderProgram, "triangleColor");
            glUniform3f(colorLoc, r, g, b);

            // Vykreslení trojúhelníku
            glBindVertexArray(VAO);
            glDrawArrays(GL_TRIANGLES, 0, 3);

            // Výměna bufferů
            glfwSwapBuffers(window);

            // Zpracování událostí
            glfwPollEvents();
        }

        // Úklid
        glDeleteVertexArrays(1, &VAO);
        glDeleteBuffers(1, &VBO);
        glDeleteProgram(shaderProgram);

        glfwTerminate();
        return 0;
    }
    catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        glfwTerminate();
        return -1;
    }
}