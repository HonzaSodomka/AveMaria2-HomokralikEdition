#pragma once

#include <glm/glm.hpp>
#include "ShaderProgram.hpp"

// Třída reprezentující materiál objektu
class Material {
public:
    // Vlastnosti materiálu
    glm::vec3 ambient;
    glm::vec3 diffuse;
    glm::vec3 specular;
    float shininess;

    // Konstruktor s výchozími hodnotami
    Material(
        glm::vec3 ambient = glm::vec3(0.2f, 0.2f, 0.2f),
        glm::vec3 diffuse = glm::vec3(0.8f, 0.8f, 0.8f),
        glm::vec3 specular = glm::vec3(0.5f, 0.5f, 0.5f),
        float shininess = 32.0f
    ) : ambient(ambient), diffuse(diffuse), specular(specular), shininess(shininess) {}

    // Nastavení hodnot materiálu do shaderu
    void SetUniforms(ShaderProgram& shader) const {
        shader.Use();

        shader.SetUniform("material.ambient", ambient);
        shader.SetUniform("material.diffuse", diffuse);
        shader.SetUniform("material.specular", specular);
        shader.SetUniform("material.shininess", shininess);
    }

    // Nastavení materiálu pro různé běžné materiály
    static Material Plastic() {
        return Material(
            glm::vec3(0.0f, 0.1f, 0.06f),
            glm::vec3(0.0f, 0.50980392f, 0.50980392f),
            glm::vec3(0.50196078f, 0.50196078f, 0.50196078f),
            32.0f
        );
    }

    static Material Gold() {
        return Material(
            glm::vec3(0.24725f, 0.1995f, 0.0745f),
            glm::vec3(0.75164f, 0.60648f, 0.22648f),
            glm::vec3(0.628281f, 0.555802f, 0.366065f),
            51.2f
        );
    }

    static Material Silver() {
        return Material(
            glm::vec3(0.19225f, 0.19225f, 0.19225f),
            glm::vec3(0.50754f, 0.50754f, 0.50754f),
            glm::vec3(0.508273f, 0.508273f, 0.508273f),
            51.2f
        );
    }

    static Material Pearl() {
        return Material(
            glm::vec3(0.25f, 0.20725f, 0.20725f),
            glm::vec3(1.0f, 0.829f, 0.829f),
            glm::vec3(0.296648f, 0.296648f, 0.296648f),
            11.264f
        );
    }
};