#pragma once

#include <vector>
#include <random>
#include <glm/glm.hpp>
#include "ShaderProgram.hpp"
#include "Model.hpp"

// Třída reprezentující jednu částici fontány
class Particle {
public:
    glm::vec3 position;       // Aktuální pozice částice
    glm::vec3 velocity;       // Aktuální rychlost částice
    glm::vec3 rotation;       // Aktuální rotace částice (v radiánech)
    glm::vec3 rotationSpeed;  // Rychlost rotace (v radiánech za sekundu)
    glm::vec3 scale;          // Měřítko částice
    float lifetime;           // Zbývající životnost částice (v sekundách)
    float maxLifetime;        // Maximální životnost částice (v sekundách)
    bool active;              // Zda je částice aktivní
    float alpha;              // Průhlednost částice
    bool exploded;            // Zda částice již explodovala
    std::vector<Particle> fragments; // Fragmenty, které vzniknou při explozi

    // Konstruktor
    Particle() :
        position(0.0f),
        velocity(0.0f),
        rotation(0.0f),
        rotationSpeed(0.0f),
        scale(1.0f),
        lifetime(0.0f),
        maxLifetime(0.0f),
        active(false),
        alpha(1.0f),
        exploded(false) {}
};

// Třída pro systém částic fontány
class ParticleSystem {
private:
    std::vector<Particle> particles;            // Částice fontány
    Model* particleModel;                       // Model pro částice
    ShaderProgram& shader;                      // Reference na shader
    glm::vec3 emitterPosition;                  // Pozice emiteru částic
    float particleSize;                         // Základní velikost částice

    // Generátor náhodných čísel
    std::mt19937 rng;
    std::uniform_real_distribution<float> angleDistribution;
    std::uniform_real_distribution<float> velocityDistribution;
    std::uniform_real_distribution<float> lifetimeDistribution;
    std::uniform_real_distribution<float> rotSpeedDistribution;

    // Počet aktivních částic
    int activeParticles;

    // Maximální počet fragmentů při explozi
    const int MAX_FRAGMENTS = 8;

public:
    // Maximální počet částic v systému
    const int MAX_PARTICLES = 100;

    // Konstruktor
    ParticleSystem(Model* model, ShaderProgram& shaderProgram, const glm::vec3& position, float size = 0.3f);

    // Destruktor
    ~ParticleSystem();

    // Emituje novou částici
    void EmitParticle();

    // Aktualizuje všechny částice
    void Update(float deltaTime);

    // Vykreslí všechny částice
    void Draw();

    // Exploduj částici při dopadu
    void ExplodeParticle(int index);

    // Settery a gettery
    void SetEmitterPosition(const glm::vec3& position) { emitterPosition = position; }
    glm::vec3 GetEmitterPosition() const { return emitterPosition; }

    void SetParticleSize(float size) { particleSize = size; }
    float GetParticleSize() const { return particleSize; }

    int GetActiveParticles() const { return activeParticles; }
};