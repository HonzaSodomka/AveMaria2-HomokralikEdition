#include <iostream>
#include <GL/glew.h>
#include <GL/wglew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <cmath>
#include "app.hpp"
#include <glm/gtc/matrix_transform.hpp>


App::App() {}

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

    if (!GLEW_ARB_direct_state_access) {
        std::cerr << "No DSA :-(" << std::endl;
        return false;
    }

    // Povolen� Z-bufferu pro spr�vn� vykreslov�n� 3D model�
    glEnable(GL_DEPTH_TEST);

    init_assets();
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
    }
    catch (const std::exception& e) {
        std::cerr << "Model loading error: " << e.what() << std::endl;
        throw;
    }

    // Povolen� depth testingu
    glEnable(GL_DEPTH_TEST);
}

bool App::run() {
    if (!window) {
        std::cerr << "No active GLFW window!" << std::endl;
        return false;
    }

    // Aktivace shader programu
    shader.activate();

    // Prom�nn� pro m��en� FPS
    double lastTime = glfwGetTime();
    int frameCount = 0;
    std::string title = "OpenGL Model Demo";

    while (!glfwWindowShouldClose(window)) {
        // M��en� FPS
        double currentTime = glfwGetTime();
        frameCount++;
        if (currentTime - lastTime >= 1.0) {
            std::string fpsTitle = title + " | FPS: " + std::to_string(frameCount) +
                " | Color: R=" + std::to_string(r) + " G=" + std::to_string(g) + " B=" + std::to_string(b);
            glfwSetWindowTitle(window, fpsTitle.c_str());
            frameCount = 0;
            lastTime = currentTime;
        }

        // Vy�i�t�n� obrazovky
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Nastaven� barvy
        //shader.setUniform("uniform_Color", glm::vec4(r, g, b, a));

        shader.setUniform("time", static_cast<float>(glfwGetTime()));

        // Vykreslen� modelu
        if (triangle) {
            triangle->draw();
        }

        // V�m�na buffer� a zpracov�n� ud�lost�
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    return true;
}

void App::change_color(int key) {
    const float step = 0.1f;

    switch (key) {
    case GLFW_KEY_R:
        r += step;
        if (r > 1.0f) r = 0.0f;
        break;
    case GLFW_KEY_G:
        g += step;
        if (g > 1.0f) g = 0.0f;
        break;
    case GLFW_KEY_B:
        b += step;
        if (b > 1.0f) b = 0.0f;
        break;
    }
}