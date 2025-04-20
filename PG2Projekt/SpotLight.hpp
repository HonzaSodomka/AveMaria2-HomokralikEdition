#pragma once

#include <glm/glm.hpp>
#include "ShaderProgram.hpp"

// Třída pro bodové světlo s kuželem (spotlight) - čelová baterka
class SpotLight {
private:
    glm::vec3 _position;      // Pozice světla
    glm::vec3 _direction;     // Směr světla
    glm::vec3 _ambient;       // Ambient složka
    glm::vec3 _diffuse;       // Diffuse složka
    glm::vec3 _specular;      // Specular složka

    float _constant;          // Konstantní složka útlumu
    float _linear;            // Lineární složka útlumu
    float _quadratic;         // Kvadratická složka útlumu

    float _cutOff;            // Vnitřní úhel kužele (v cos)
    float _outerCutOff;       // Vnější úhel kužele (v cos)

public:
    // Konstruktor s výchozími hodnotami
    SpotLight(
        const glm::vec3& position = glm::vec3(0.0f, 0.0f, 0.0f),
        const glm::vec3& direction = glm::vec3(0.0f, 0.0f, -1.0f),
        const glm::vec3& ambient = glm::vec3(0.1f, 0.1f, 0.1f),
        const glm::vec3& diffuse = glm::vec3(1.0f, 1.0f, 1.0f),
        const glm::vec3& specular = glm::vec3(1.0f, 1.0f, 1.0f),
        float constant = 1.0f,
        float linear = 0.09f,
        float quadratic = 0.032f,
        float cutOff = glm::cos(glm::radians(12.5f)),        // Vnitřní úhel kužele (12.5 stupňů)
        float outerCutOff = glm::cos(glm::radians(15.0f))    // Vnější úhel kužele (15 stupňů)
    ) :
        _position(position),
        _direction(glm::normalize(direction)),
        _ambient(ambient),
        _diffuse(diffuse),
        _specular(specular),
        _constant(constant),
        _linear(linear),
        _quadratic(quadratic),
        _cutOff(cutOff),
        _outerCutOff(outerCutOff) {}

    // Gettery a settery pro vlastnosti světla
    glm::vec3 GetPosition() const { return _position; }
    void SetPosition(const glm::vec3& position) { _position = position; }

    glm::vec3 GetDirection() const { return _direction; }
    void SetDirection(const glm::vec3& direction) { _direction = glm::normalize(direction); }

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

    float GetCutOff() const { return _cutOff; }
    void SetCutOff(float cutOffInDegrees) { _cutOff = glm::cos(glm::radians(cutOffInDegrees)); }

    float GetOuterCutOff() const { return _outerCutOff; }
    void SetOuterCutOff(float outerCutOffInDegrees) { _outerCutOff = glm::cos(glm::radians(outerCutOffInDegrees)); }

    // Metoda pro nastavení úhlů kužele ve stupních
    void SetConeAngles(float innerAngleDegrees, float outerAngleDegrees) {
        _cutOff = glm::cos(glm::radians(innerAngleDegrees));
        _outerCutOff = glm::cos(glm::radians(outerAngleDegrees));
    }

    // Nastavení barvy světla (ovlivní všechny složky s různou intenzitou)
    void SetColor(const glm::vec3& color) {
        _ambient = color * 0.1f;   // Nízká intenzita pro ambient
        _diffuse = color * 0.8f;   // Střední intenzita pro diffuse
        _specular = color;         // Plná intenzita pro specular
    }

    // Nastavení všech uniforem pro světlo do shaderu
    void SetUniforms(ShaderProgram& shader) const {
        shader.activate();

        shader.setUniform("spotLight.position", _position);
        shader.setUniform("spotLight.direction", _direction);
        shader.setUniform("spotLight.ambient", _ambient);
        shader.setUniform("spotLight.diffuse", _diffuse);
        shader.setUniform("spotLight.specular", _specular);
        shader.setUniform("spotLight.constant", _constant);
        shader.setUniform("spotLight.linear", _linear);
        shader.setUniform("spotLight.quadratic", _quadratic);
        shader.setUniform("spotLight.cutOff", _cutOff);
        shader.setUniform("spotLight.outerCutOff", _outerCutOff);
    }
};