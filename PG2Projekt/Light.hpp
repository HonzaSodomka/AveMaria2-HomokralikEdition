#pragma once

#include <glm/glm.hpp>
#include "ShaderProgram.hpp"

// Třída pro bodové světlo (point light) - světelný zdroj který svítí do všech směrů
class PointLight {
private:
    glm::vec3 _position;      // Pozice světla
    glm::vec3 _color;         // Barva světla

    // Koeficienty útlumu světla se vzdáleností
    float _constant;          // Konstantní složka útlumu
    float _linear;            // Lineární složka útlumu
    float _quadratic;         // Kvadratická složka útlumu

public:
    // Konstruktor s výchozími hodnotami
    PointLight(
        const glm::vec3& position = glm::vec3(0.0f, 0.0f, 0.0f),
        const glm::vec3& color = glm::vec3(1.0f, 1.0f, 1.0f),
        float constant = 1.0f,
        float linear = 0.09f,
        float quadratic = 0.032f
    ) :
        _position(position),
        _color(color),
        _constant(constant),
        _linear(linear),
        _quadratic(quadratic) {}

    // Gettery a settery pro vlastnosti světla
    glm::vec3 GetPosition() const { return _position; }
    void SetPosition(const glm::vec3& position) { _position = position; }

    glm::vec3 GetColor() const { return _color; }
    void SetColor(const glm::vec3& color) { _color = color; }

    float GetConstant() const { return _constant; }
    void SetConstant(float constant) { _constant = constant; }

    float GetLinear() const { return _linear; }
    void SetLinear(float linear) { _linear = linear; }

    float GetQuadratic() const { return _quadratic; }
    void SetQuadratic(float quadratic) { _quadratic = quadratic; }

    // Nastavení všech uniforem pro světlo do shaderu - upraveno pro index světla
    void SetUniforms(ShaderProgram& shader, int lightIndex) const {
        shader.activate();

        // Použití indexu pro nastavení správného světla v poli
        std::string lightPrefix = "lights[" + std::to_string(lightIndex) + "]";

        shader.setUniform(lightPrefix + ".position", _position);
        shader.setUniform(lightPrefix + ".color", _color);
        shader.setUniform(lightPrefix + ".constant", _constant);
        shader.setUniform(lightPrefix + ".linear", _linear);
        shader.setUniform(lightPrefix + ".quadratic", _quadratic);
    }
};