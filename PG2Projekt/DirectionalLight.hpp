#pragma once

#include <glm/glm.hpp>
#include "ShaderProgram.hpp"

// Třída pro směrové světlo (directional light) - světlo které svítí ve stejném směru na celou scénu
// jako slunce nebo měsíc
class DirectionalLight {
private:
    glm::vec3 _direction;     // Směr světla (odkud světlo přichází)
    glm::vec3 _ambient;       // Ambient složka (základní okolní osvětlení)
    glm::vec3 _diffuse;       // Diffuse složka (rozptýlené světlo)
    glm::vec3 _specular;      // Specular složka (odlesky)
    glm::vec3 _position;      // Pozice světla pro vizualizaci (nemá vliv na osvětlení)

public:
    // Konstruktor s výchozími hodnotami
    DirectionalLight(
        const glm::vec3& direction = glm::vec3(0.0f, -1.0f, 0.0f),  // Výchozí směr - svítí shora dolů
        const glm::vec3& ambient = glm::vec3(0.2f, 0.2f, 0.2f),
        const glm::vec3& diffuse = glm::vec3(0.8f, 0.8f, 0.7f),
        const glm::vec3& specular = glm::vec3(1.0f, 1.0f, 0.9f),
        const glm::vec3& position = glm::vec3(7.5f, 10.0f, 7.5f)    // Výchozí pozice nad středem mapy
    ) :
        _direction(glm::normalize(direction)),  // Normalizace směru pro zajištění jednotkového vektoru
        _ambient(ambient),
        _diffuse(diffuse),
        _specular(specular),
        _position(position) {}

    // Gettery a settery pro vlastnosti světla
    glm::vec3 GetDirection() const { return _direction; }
    void SetDirection(const glm::vec3& direction) { _direction = glm::normalize(direction); }

    glm::vec3 GetPosition() const { return _position; }
    void SetPosition(const glm::vec3& position) { _position = position; }

    glm::vec3 GetAmbient() const { return _ambient; }
    void SetAmbient(const glm::vec3& ambient) { _ambient = ambient; }

    glm::vec3 GetDiffuse() const { return _diffuse; }
    void SetDiffuse(const glm::vec3& diffuse) { _diffuse = diffuse; }

    glm::vec3 GetSpecular() const { return _specular; }
    void SetSpecular(const glm::vec3& specular) { _specular = specular; }

    // Nastavení barvy světla (ovlivní všechny složky s různou intenzitou)
    void SetColor(const glm::vec3& color) {
        _ambient = color * 0.2f;  // Nízká intenzita pro ambient
        _diffuse = color * 0.8f;  // Střední intenzita pro diffuse
        _specular = color;        // Plná intenzita pro specular
    }

    // Nastavení všech uniforem pro světlo do shaderu
    void SetUniforms(ShaderProgram& shader) const {
        shader.Use();

        shader.SetUniform("dirLight.direction", _direction);
        shader.SetUniform("dirLight.ambient", _ambient);
        shader.SetUniform("dirLight.diffuse", _diffuse);
        shader.SetUniform("dirLight.specular", _specular);
    }
};