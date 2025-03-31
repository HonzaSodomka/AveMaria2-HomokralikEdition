#pragma once

#include <glm/glm.hpp>
#include "ShaderProgram.hpp"

// Třída pro bodové světlo (point light) - světelný zdroj který svítí do všech směrů
class PointLight {
private:
    glm::vec3 _position;      // Pozice světla
    glm::vec3 _ambient;       // Ambient složka (základní okolní osvětlení)
    glm::vec3 _diffuse;       // Diffuse složka (rozptýlené světlo)
    glm::vec3 _specular;      // Specular složka (odlesky)

    // Koeficienty útlumu světla se vzdáleností
    float _constant;          // Konstantní složka útlumu
    float _linear;            // Lineární složka útlumu
    float _quadratic;         // Kvadratická složka útlumu

public:
    // Konstruktor s výchozími hodnotami
    PointLight(
        const glm::vec3& position = glm::vec3(0.0f, 0.0f, 0.0f),
        const glm::vec3& ambient = glm::vec3(0.1f, 0.1f, 0.1f),
        const glm::vec3& diffuse = glm::vec3(0.8f, 0.8f, 0.8f),
        const glm::vec3& specular = glm::vec3(1.0f, 1.0f, 1.0f),
        float constant = 1.0f,
        float linear = 0.09f,
        float quadratic = 0.032f
    ) :
        _position(position),
        _ambient(ambient),
        _diffuse(diffuse),
        _specular(specular),
        _constant(constant),
        _linear(linear),
        _quadratic(quadratic) {}

    // Gettery a settery pro vlastnosti světla
    glm::vec3 GetPosition() const { return _position; }
    void SetPosition(const glm::vec3& position) { _position = position; }

    glm::vec3 GetAmbient() const { return _ambient; }
    void SetAmbient(const glm::vec3& ambient) { _ambient = ambient; }

    glm::vec3 GetDiffuse() const { return _diffuse; }
    void SetDiffuse(const glm::vec3& diffuse) { _diffuse = diffuse; }

    glm::vec3 GetSpecular() const { return _specular; }
    void SetSpecular(const glm::vec3& specular) { _specular = specular; }

    float GetConstant() const { return _constant; }
    void SetConstant(float constant) { _constant = constant; }

    float GetLinear() const { return _linear; }
    void SetLinear(float linear) { _linear = linear; }

    float GetQuadratic() const { return _quadratic; }
    void SetQuadratic(float quadratic) { _quadratic = quadratic; }

    // Nastavení barvy světla (ovlivní všechny složky s různou intenzitou)
    void SetColor(const glm::vec3& color) {
        _ambient = color * 0.1f;  // Nízká intenzita pro ambient
        _diffuse = color * 0.8f;  // Střední intenzita pro diffuse
        _specular = color;        // Plná intenzita pro specular
    }

    // Nastavení všech uniforem pro světlo do shaderu
    void SetUniforms(ShaderProgram& shader) const {
        shader.Use();

        shader.SetUniform("light.position", _position);
        shader.SetUniform("light.ambient", _ambient);
        shader.SetUniform("light.diffuse", _diffuse);
        shader.SetUniform("light.specular", _specular);
        shader.SetUniform("light.constant", _constant);
        shader.SetUniform("light.linear", _linear);
        shader.SetUniform("light.quadratic", _quadratic);
    }
};