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
    // Nastaven� po��te�n� pozice kamery
    camera = Camera(glm::vec3(0.0f, 0.0f, 10.0f));
}

App::~App() {
    // �klid
    shader.clear();
    if (triangle) {
        delete triangle;
        triangle = nullptr;
    }
}

bool App::init(GLFWwindow* win) {
    window = win;

    // Ujist�me se, �e FOV m� spr�vnou hodnotu
    fov = DEFAULT_FOV;

    if (!GLEW_ARB_direct_state_access) {
        std::cerr << "No DSA :-(" << std::endl;
        return false;
    }

    // Nastaven� ukazatele na t��du App pro callbacky
    glfwSetWindowUserPointer(window, this);

    // Nastaven� callback�
    glfwSetFramebufferSizeCallback(window, fbsize_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetKeyCallback(window, key_callback);
    glfwSetCursorPosCallback(window, cursor_position_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);

    // Zak�z�n� zobrazen� kurzoru a jeho omezen� na okno
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // Povolen� Z-bufferu pro spr�vn� vykreslov�n� 3D model�
    glEnable(GL_DEPTH_TEST);

    // Z�sk�n� velikosti framebufferu
    glfwGetFramebufferSize(window, &width, &height);
    glViewport(0, 0, width, height);

    // Na�ten� assets
    init_assets();

    // Prvn� aktualizace projek�n� matice
    update_projection_matrix();

    // Explicitn� nastaven� view matice
    if (shader.getID() != 0) {
        shader.activate();
        shader.setUniform("uV_m", camera.GetViewMatrix());
    }

    return true;
}

void App::init_assets() {
    // Na�ten� shader�
    try {
        std::cout << "Loading shaders..." << std::endl;
        shader = ShaderProgram("resources/shaders/basic.vert", "resources/shaders/basic.frag");
        std::cout << "Shaders loaded successfully" << std::endl;
    }
    catch (const std::exception& e) {
        std::cerr << "Shader loading error: " << e.what() << std::endl;
        throw;
    }

    // Na�ten� modelu
    try {
        std::cout << "Loading model..." << std::endl;
        triangle = new Model("resources/models/bunny_tri_vnt.obj", shader);
        std::cout << "Model loaded successfully. Vertices: "
            << triangle->meshes[0].vertices.size() << std::endl;

        // Nastaven� v�choz� pozice modelu
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

    // Ujist�me se, �e FOV m� spr�vnou hodnotu
    if (fov <= 0.0f) {
        fov = DEFAULT_FOV;
    }

    // Aktivace shader programu
    shader.activate();

    // Inicializace projek�n� a pohledov� matice
    update_projection_matrix();
    shader.setUniform("uV_m", camera.GetViewMatrix());

    // Explicitn� vykreslen� prvn�ho sn�mku
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if (triangle) {
        // Aktualizace model matice
        triangle->orientation.y = glm::radians(static_cast<float>(glfwGetTime() * 50.0));

        // Nastaven� model matice v shaderu
        shader.setUniform("uM_m", triangle->getModelMatrix());

        // Vykreslen� modelu
        triangle->draw();
    }

    // V�m�na buffer�
    glfwSwapBuffers(window);

    // Prom�nn� pro m��en� FPS a deltaTime
    double lastTime = glfwGetTime();
    double lastFrameTime = lastTime;
    int frameCount = 0;
    std::string title = "OpenGL Model Demo";

    // Hlavn� smy�ka
    while (!glfwWindowShouldClose(window)) {
        // V�po�et deltaTime
        double currentTime = glfwGetTime();
        float deltaTime = static_cast<float>(currentTime - lastFrameTime);
        lastFrameTime = currentTime;

        // M��en� FPS
        frameCount++;
        if (currentTime - lastTime >= 1.0) {
            std::string fpsTitle = title + " | FPS: " + std::to_string(frameCount);
            glfwSetWindowTitle(window, fpsTitle.c_str());
            frameCount = 0;
            lastTime = currentTime;
        }

        // Zpracov�n� vstupu z kl�vesnice pro pohyb kamery
        glm::vec3 direction = camera.ProcessKeyboard(window, deltaTime);
        camera.Move(direction);

        // Aktualizace pohledov� matice
        shader.setUniform("uV_m", camera.GetViewMatrix());

        // Vy�i�t�n� obrazovky
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Vykreslen� modelu
        if (triangle) {
            // Animace rotace kr�l�ka
            triangle->orientation.y = glm::radians(static_cast<float>(currentTime * 50.0));

            // Nastaven� model matice v shaderu
            shader.setUniform("uM_m", triangle->getModelMatrix());

            triangle->draw();
        }

        // V�m�na buffer� a zpracov�n� ud�lost�
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    return true;
}

void App::update_projection_matrix() {
    if (height < 1)
        height = 1;   // Prevence d�len� nulou

    // Kontrola, zda fov m� platnou hodnotu
    if (fov <= 0.0f) {
        fov = DEFAULT_FOV;
    }

    float ratio = static_cast<float>(width) / height;

    projection_matrix = glm::perspective(
        glm::radians(fov),   // Vertik�ln� zorn� pole ve stupn�ch, p�evedeno na radi�ny
        ratio,               // Pom�r stran okna
        0.1f,                // Near clipping plane
        20000.0f             // Far clipping plane
    );

    // Nastaven� uniform v shaderu
    if (shader.getID() != 0) {
        shader.activate();
        shader.setUniform("uP_m", projection_matrix);
    }
}

void App::fbsize_callback(GLFWwindow* window, int width, int height) {
    App* app = static_cast<App*>(glfwGetWindowUserPointer(window));
    app->width = width;
    app->height = height;

    // Nastaven� viewportu
    glViewport(0, 0, width, height);

    // Aktualizace projek�n� matice
    app->update_projection_matrix();
}

void App::scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    App* app = static_cast<App*>(glfwGetWindowUserPointer(window));
    app->fov += -5.0f * static_cast<float>(yoffset);  // Zoom in/out
    app->fov = glm::clamp(app->fov, 20.0f, 170.0f);  // Omezen� FOV na rozumn� hodnoty
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
    double yoffset = app->lastY - ypos; // P�evr�ceno, proto�e y-sou�adnice jdou od shora dol�

    app->lastX = xpos;
    app->lastY = ypos;

    app->camera.ProcessMouseMovement(static_cast<float>(xoffset), static_cast<float>(yoffset));
}

void App::mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
    App* app = static_cast<App*>(glfwGetWindowUserPointer(window));

    if (button == GLFW_MOUSE_BUTTON_MIDDLE && action == GLFW_PRESS) {
        // Reset FOV na v�choz� hodnotu
        app->fov = app->DEFAULT_FOV;
        app->update_projection_matrix();
        std::cout << "Zoom reset to default" << std::endl;
    }
}