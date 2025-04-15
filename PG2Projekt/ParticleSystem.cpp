#include "ParticleSystem.hpp"
#include <algorithm>
#include <iostream>
#include <glm/gtc/matrix_transform.hpp>

ParticleSystem::ParticleSystem(Model* model, ShaderProgram& shaderProgram, const glm::vec3& position, float size) :
    particleModel(model),
    shader(shaderProgram),
    emitterPosition(position),
    particleSize(size),
    activeParticles(0)
{
    // Inicializace generátoru náhodných čísel
    std::random_device rd;
    rng = std::mt19937(rd());

    // Inicializace distribucí pro náhodné hodnoty
    angleDistribution = std::uniform_real_distribution<float>(0.0f, 2.0f * 3.14159f);
    velocityDistribution = std::uniform_real_distribution<float>(3.0f, 6.0f);
    lifetimeDistribution = std::uniform_real_distribution<float>(3.0f, 5.0f);
    rotSpeedDistribution = std::uniform_real_distribution<float>(1.0f, 5.0f);

    // Inicializace částic
    particles.resize(MAX_PARTICLES);

    // Všechny částice jsou neaktivní na začátku
    for (auto& particle : particles) {
        particle.active = false;
    }
}

ParticleSystem::~ParticleSystem() {
    // Zde není potřeba uvolňovat particleModel, protože jsme jen dostali pointer
}

void ParticleSystem::EmitParticle() {
    // Najdeme první neaktivní částici
    for (int i = 0; i < MAX_PARTICLES; i++) {
        if (!particles[i].active) {
            // Inicializace částice
            Particle& p = particles[i];
            p.active = true;
            p.position = emitterPosition;

            // Náhodný úhel ve válcových souřadnicích
            float angle = angleDistribution(rng);
            float speed = velocityDistribution(rng);

            // Přepočet na kartézské souřadnice
            p.velocity.x = speed * std::cos(angle) * 0.5f; // Zmenšení rychlosti v horizontální rovině
            p.velocity.z = speed * std::sin(angle) * 0.5f;
            p.velocity.y = speed * 2.0f;  // Hlavně vystřelujeme nahoru

            // Náhodná rotace
            p.rotation = glm::vec3(angleDistribution(rng), angleDistribution(rng), angleDistribution(rng));

            // Náhodná rychlost rotace
            p.rotationSpeed = glm::vec3(
                rotSpeedDistribution(rng) * (rand() % 2 ? 1.0f : -1.0f),
                rotSpeedDistribution(rng) * (rand() % 2 ? 1.0f : -1.0f),
                rotSpeedDistribution(rng) * (rand() % 2 ? 1.0f : -1.0f)
            );

            // Náhodná životnost
            p.lifetime = p.maxLifetime = lifetimeDistribution(rng);

            // Měřítko částice
            p.scale = glm::vec3(particleSize);

            // Plná průhlednost na začátku
            p.alpha = 1.0f;

            // Částice není explodovaná
            p.exploded = false;
            p.fragments.clear();

            activeParticles++;
            return;
        }
    }
}

void ParticleSystem::ExplodeParticle(int index) {
    if (index < 0 || index >= particles.size() || !particles[index].active || particles[index].exploded) {
        return;
    }

    Particle& parent = particles[index];
    parent.exploded = true;

    // Vytvoříme fragmenty (menší kostičky)
    int numFragments = rand() % MAX_FRAGMENTS + 3; // 3 až MAX_FRAGMENTS

    for (int i = 0; i < numFragments; i++) {
        Particle fragment;
        fragment.active = true;
        fragment.position = parent.position;

        // Náhodná rychlost ve všech směrech
        fragment.velocity.x = (rand() % 100 - 50) / 50.0f * 3.0f;
        fragment.velocity.y = (rand() % 100) / 50.0f * 2.0f; // Směrem nahoru
        fragment.velocity.z = (rand() % 100 - 50) / 50.0f * 3.0f;

        // Náhodná rotace
        fragment.rotation = glm::vec3(angleDistribution(rng), angleDistribution(rng), angleDistribution(rng));

        // Rychlejší rotace fragmentů
        fragment.rotationSpeed = glm::vec3(
            rotSpeedDistribution(rng) * 2.0f * (rand() % 2 ? 1.0f : -1.0f),
            rotSpeedDistribution(rng) * 2.0f * (rand() % 2 ? 1.0f : -1.0f),
            rotSpeedDistribution(rng) * 2.0f * (rand() % 2 ? 1.0f : -1.0f)
        );

        // Kratší životnost fragmentů
        fragment.lifetime = fragment.maxLifetime = lifetimeDistribution(rng) * 0.5f;

        // Menší fragmenty
        fragment.scale = parent.scale * 0.3f;

        fragment.alpha = 0.8f;
        fragment.exploded = true; // Fragment už nemůže dále explodovat

        parent.fragments.push_back(fragment);
    }
}

void ParticleSystem::Update(float deltaTime) {
    // Gravitační konstanta
    const float gravity = 9.8f;
    const float groundY = 0.01f; // Úroveň země (mírně nad 0 pro přesnost)

    // Emisní rychlost - 2 částice za frame při 60 FPS
    static float emissionTimer = 0.0f;
    emissionTimer += deltaTime;

    // Emitujeme částice v pravidelných intervalech
    if (emissionTimer >= 0.03f) {
        emissionTimer = 0.0f;
        if (activeParticles < MAX_PARTICLES) {
            EmitParticle();
        }
    }

    // Aktualizace všech částic
    for (int i = 0; i < particles.size(); i++) {
        Particle& p = particles[i];

        if (!p.active) {
            continue;
        }

        // Aktualizace životnosti
        p.lifetime -= deltaTime;

        // Pokud částice není explodovaná a narazila na zem, exploduje
        if (!p.exploded && p.position.y <= groundY && p.velocity.y < 0) {
            ExplodeParticle(i);
        }

        // Aktualizace fragmentů (pokud částice explodovala)
        if (p.exploded) {
            for (auto& fragment : p.fragments) {
                if (fragment.active) {
                    // Aktualizace pozice
                    fragment.velocity.y -= gravity * deltaTime; // Gravitace
                    fragment.position += fragment.velocity * deltaTime;

                    // Aktualizace rotace
                    fragment.rotation += fragment.rotationSpeed * deltaTime;

                    // Aktualizace životnosti
                    fragment.lifetime -= deltaTime;

                    // Postupné zmenšování a zvýšení průhlednosti když se blíží konec životnosti
                    if (fragment.lifetime < 0.5f) {
                        float factor = fragment.lifetime / 0.5f;
                        fragment.scale = fragment.scale * (0.9f + 0.1f * factor);
                        fragment.alpha = factor;
                    }

                    // Deaktivace fragmentu po vypršení životnosti
                    if (fragment.lifetime <= 0.0f) {
                        fragment.active = false;
                    }
                }
            }

            // Kontrola, zda jsou všechny fragmenty neaktivní
            bool allFragmentsInactive = true;
            for (const auto& fragment : p.fragments) {
                if (fragment.active) {
                    allFragmentsInactive = false;
                    break;
                }
            }

            // Pokud jsou všechny fragmenty neaktivní a částice je explodovaná, deaktivujeme ji
            if (allFragmentsInactive) {
                p.active = false;
                p.fragments.clear();
                activeParticles--;
            }
        }
        else {
            // Aktualizace pozice (gravitace)
            p.velocity.y -= gravity * deltaTime;
            p.position += p.velocity * deltaTime;

            // Aktualizace rotace
            p.rotation += p.rotationSpeed * deltaTime;

            // Postupné zmenšování a zvýšení průhlednosti když se blíží konec životnosti
            if (p.lifetime < 1.0f && p.lifetime > 0.0f) {
                p.alpha = p.lifetime;
            }

            // Deaktivace částice po vypršení životnosti
            if (p.lifetime <= 0.0f) {
                p.active = false;
                activeParticles--;
            }
        }
    }
}

void ParticleSystem::Draw() {
    // Nastavení potřebných OpenGL stavů
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Vykreslení všech aktivních částic
    for (const auto& p : particles) {
        if (p.active) {
            // Pokud částice explodovala, vykreslíme její fragmenty
            if (p.exploded) {
                for (const auto& fragment : p.fragments) {
                    if (fragment.active) {
                        // Vytvoření modelové matice pro fragment
                        glm::mat4 model = glm::mat4(1.0f);
                        model = glm::translate(model, fragment.position);

                        // Aplikace rotace ve všech osách
                        model = glm::rotate(model, fragment.rotation.x, glm::vec3(1.0f, 0.0f, 0.0f));
                        model = glm::rotate(model, fragment.rotation.y, glm::vec3(0.0f, 1.0f, 0.0f));
                        model = glm::rotate(model, fragment.rotation.z, glm::vec3(0.0f, 0.0f, 1.0f));

                        model = glm::scale(model, fragment.scale);

                        // Nastavení modelové matice ve shaderu
                        shader.setUniform("uM_m", model);

                        // Nastavení průhlednosti
                        glm::vec4 color = glm::vec4(0.8f, 0.2f, 0.2f, fragment.alpha); // Červená barva pro fragmenty
                        shader.setUniform("u_diffuse_color", color);

                        // Vykreslení modelu částice
                        particleModel->draw();
                    }
                }
            }
            else {
                // Vytvoření modelové matice
                glm::mat4 model = glm::mat4(1.0f);
                model = glm::translate(model, p.position);

                // Aplikace rotace ve všech osách
                model = glm::rotate(model, p.rotation.x, glm::vec3(1.0f, 0.0f, 0.0f));
                model = glm::rotate(model, p.rotation.y, glm::vec3(0.0f, 1.0f, 0.0f));
                model = glm::rotate(model, p.rotation.z, glm::vec3(0.0f, 0.0f, 1.0f));

                model = glm::scale(model, p.scale);

                // Nastavení modelové matice ve shaderu
                shader.setUniform("uM_m", model);

                // Nastavení průhlednosti
                glm::vec4 color = glm::vec4(0.2f, 0.4f, 0.9f, p.alpha); // Modrá barva pro hlavní částice
                shader.setUniform("u_diffuse_color", color);

                // Vykreslení modelu částice
                particleModel->draw();
            }
        }
    }

    // Obnovení původních nastavení OpenGL
    glDisable(GL_BLEND);
}