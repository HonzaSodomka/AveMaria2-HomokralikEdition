#include <iostream>
#include <string>
#include <cmath>
// OpenGL includes
#include <GL/glew.h>
#include <GL/wglew.h>
#include <GLFW/glfw3.h>
// GLM includes
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "app.hpp"

App::App() :
    lastX(400.0),
    lastY(300.0),
    firstMouse(true),
    fov(DEFAULT_FOV)
{
    // Nastavení poèáteèní pozice kamery
    camera = Camera(glm::vec3(0.0f, 0.0f, 10.0f));
}

App::~App() {
    // Úklid
    shader.clear();
    if (triangle) {
        delete triangle;
        triangle = nullptr;
    }
}

bool App::init(GLFWwindow* win) {
    window = win;

    // Ujistíme se, že FOV má správnou hodnotu
    fov = DEFAULT_FOV;

    if (!GLEW_ARB_direct_state_access) {
        std::cerr << "No DSA :-(" << std::endl;
        return false;
    }

    // Nastavení ukazatele na tøídu App pro callbacky
    glfwSetWindowUserPointer(window, this);

    // Nastavení callbackù
    glfwSetFramebufferSizeCallback(window, fbsize_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetKeyCallback(window, key_callback);
    glfwSetCursorPosCallback(window, cursor_position_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);

    // Zakázání zobrazení kurzoru a jeho omezení na okno
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // Povolení Z-bufferu pro správné vykreslování 3D modelù
    glEnable(GL_DEPTH_TEST);

    // Získání velikosti framebufferu
    glfwGetFramebufferSize(window, &width, &height);
    glViewport(0, 0, width, height);

    // Naètení assets
    init_assets();

    // První aktualizace projekèní matice
    update_projection_matrix();

    // Explicitní nastavení view matice
    if (shader.getID() != 0) {
        shader.activate();
        shader.setUniform("uV_m", camera.GetViewMatrix());
    }

    return true;
}

void App::init_assets() {
    // Naètení shaderù
    try {
        std::cout << "Loading shaders..." << std::endl;
        shader = ShaderProgram("resources/shaders/basic.vert", "resources/shaders/basic.frag");
        std::cout << "Shaders loaded successfully" << std::endl;
    }
    catch (const std::exception& e) {
        std::cerr << "Shader loading error: " << e.what() << std::endl;
        throw;
    }

    // Naètení modelu
    try {
        std::cout << "Loading model..." << std::endl;
        triangle = new Model("resources/models/bunny_tri_vnt.obj", shader);
        std::cout << "Model loaded successfully. Vertices: "
            << triangle->meshes[0].vertices.size() << std::endl;

        // Nastavení výchozí pozice modelu
        triangle->origin = glm::vec3(0.0f, 0.0f, 0.0f);
        triangle->scale = glm::vec3(1.0f, 1.0f, 1.0f);
    }
    catch (const std::exception& e) {
        std::cerr << "Model loading error: " << e.what() << std::endl;
        throw;
    }
}

bool App::run() {
    if (!window) {
        std::cerr << "No active GLFW window!" << std::endl;
        return false;
    }

    // Ujistíme se, že FOV má správnou hodnotu
    if (fov <= 0.0f) {
        fov = DEFAULT_FOV;
    }

    // Aktivace shader programu
    shader.activate();

    // Inicializace projekèní a pohledové matice
    update_projection_matrix();
    shader.setUniform("uV_m", camera.GetViewMatrix());

    // Explicitní vykreslení prvního snímku
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if (triangle) {
        // Aktualizace model matice
        triangle->orientation.y = glm::radians(static_cast<float>(glfwGetTime() * 50.0));

        // Nastavení model matice v shaderu
        shader.setUniform("uM_m", triangle->getModelMatrix());

        // Vykreslení modelu
        triangle->draw();
    }

    // Výmìna bufferù
    glfwSwapBuffers(window);

    // Promìnné pro mìøení FPS a deltaTime
    double lastTime = glfwGetTime();
    double lastFrameTime = lastTime;
    int frameCount = 0;
    std::string title = "OpenGL Model Demo";

    // Hlavní smyèka
    while (!glfwWindowShouldClose(window)) {
        // Výpoèet deltaTime
        double currentTime = glfwGetTime();
        float deltaTime = static_cast<float>(currentTime - lastFrameTime);
        lastFrameTime = currentTime;

        // Mìøení FPS
        frameCount++;
        if (currentTime - lastTime >= 1.0) {
            std::string fpsTitle = title + " | FPS: " + std::to_string(frameCount);
            glfwSetWindowTitle(window, fpsTitle.c_str());
            frameCount = 0;
            lastTime = currentTime;
        }

        // Zpracování vstupu z klávesnice pro pohyb kamery
        glm::vec3 direction = camera.ProcessKeyboard(window, deltaTime);
        camera.Move(direction);

        // Aktualizace pohledové matice
        shader.setUniform("uV_m", camera.GetViewMatrix());

        // Vyèištìní obrazovky
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Vykreslení modelu
        if (triangle) {
            // Animace rotace králíka
            triangle->orientation.y = glm::radians(static_cast<float>(currentTime * 50.0));

            // Nastavení model matice v shaderu
            shader.setUniform("uM_m", triangle->getModelMatrix());

            triangle->draw();
        }

        // Výmìna bufferù a zpracování událostí
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    return true;
}

void App::update_projection_matrix() {
    if (height < 1)
        height = 1;   // Prevence dìlení nulou

    // Kontrola, zda fov má platnou hodnotu
    if (fov <= 0.0f) {
        fov = DEFAULT_FOV;
    }

    float ratio = static_cast<float>(width) / height;

    projection_matrix = glm::perspective(
        glm::radians(fov),   // Vertikální zorné pole ve stupních, pøevedeno na radiány
        ratio,               // Pomìr stran okna
        0.1f,                // Near clipping plane
        20000.0f             // Far clipping plane
    );

    // Nastavení uniform v shaderu
    if (shader.getID() != 0) {
        shader.activate();
        shader.setUniform("uP_m", projection_matrix);
    }
}

void App::fbsize_callback(GLFWwindow* window, int width, int height) {
    App* app = static_cast<App*>(glfwGetWindowUserPointer(window));
    app->width = width;
    app->height = height;

    // Nastavení viewportu
    glViewport(0, 0, width, height);

    // Aktualizace projekèní matice
    app->update_projection_matrix();
}

void App::scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    App* app = static_cast<App*>(glfwGetWindowUserPointer(window));
    app->fov += -5.0f * static_cast<float>(yoffset);  // Zoom in/out
    app->fov = glm::clamp(app->fov, 20.0f, 170.0f);  // Omezení FOV na rozumné hodnoty
    app->update_projection_matrix();
}

void App::key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    App* app = static_cast<App*>(glfwGetWindowUserPointer(window));

    if (action == GLFW_PRESS || action == GLFW_REPEAT) {
        switch (key) {
        case GLFW_KEY_ESCAPE:
            glfwSetWindowShouldClose(window, GLFW_TRUE);
            break;
        }
    }
}

void App::cursor_position_callback(GLFWwindow* window, double xpos, double ypos) {
    App* app = static_cast<App*>(glfwGetWindowUserPointer(window));

    if (app->firstMouse) {
        app->lastX = xpos;
        app->lastY = ypos;
        app->firstMouse = false;
    }

    double xoffset = xpos - app->lastX;
    double yoffset = app->lastY - ypos; // Pøevráceno, protože y-souøadnice jdou od shora dolù

    app->lastX = xpos;
    app->lastY = ypos;

    app->camera.ProcessMouseMovement(static_cast<float>(xoffset), static_cast<float>(yoffset));
}

void App::mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
    App* app = static_cast<App*>(glfwGetWindowUserPointer(window));

    if (button == GLFW_MOUSE_BUTTON_MIDDLE && action == GLFW_PRESS) {
        // Reset FOV na výchozí hodnotu
        app->fov = app->DEFAULT_FOV;
        app->update_projection_matrix();
        std::cout << "Zoom reset to default" << std::endl;
    }
}