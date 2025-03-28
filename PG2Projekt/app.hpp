#pragma once

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <vector>
#include "assets.hpp"
#include "ShaderProgram.hpp"
#include "Model.hpp"

class App {
public:
    App();
    ~App();
    
    bool init(GLFWwindow* window);
    void init_assets();
    bool run();
    
    void change_color(int key);
    
private:
    ShaderProgram shader;
    Model* triangle{nullptr};
    
    GLfloat r{ 1.0f }, g{ 0.0f }, b{ 0.0f }, a{ 1.0f };
    
    GLFWwindow* window{nullptr};
};