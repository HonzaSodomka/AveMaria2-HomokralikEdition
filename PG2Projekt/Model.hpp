#pragma once
#include <filesystem>
#include <string>
#include <vector> 
#include <glm/glm.hpp> 
#include <glm/gtc/matrix_transform.hpp>
#include "assets.hpp"
#include "Mesh.hpp"
#include "ShaderProgram.hpp"
#include "OBJloader.hpp"

class Model {
public:
    std::vector<Mesh> meshes;
    std::string name;

    // Transformaèní vlastnosti
    glm::vec3 origin{ 0.0f, 0.0f, 0.0f };
    glm::vec3 orientation{ 0.0f, 0.0f, 0.0f };  // v radiánech
    glm::vec3 scale{ 1.0f, 1.0f, 1.0f };
    glm::mat4 local_model_matrix{ 1.0f }; // identita (žádná transformace)

    ShaderProgram shader;

    Model(const std::filesystem::path& filename, ShaderProgram shader) {
        this->shader = shader;
        this->name = filename.stem().string();

        // Naètení OBJ souboru
        std::vector<glm::vec3> vertices;
        std::vector<glm::vec2> uvs;
        std::vector<glm::vec3> normals;
        bool res = loadOBJ(filename.string(), vertices, uvs, normals);
        if (!res) {
            throw std::runtime_error("Failed to load OBJ file: " + filename.string());
        }

        // Pøevod naètených dat do formátu, který používá naše struktura vertex
        std::vector<vertex> mesh_vertices;
        for (size_t i = 0; i < vertices.size(); i++) {
            vertex v;
            v.position = vertices[i];
            if (i < uvs.size()) {
                v.texCoord = uvs[i];
            }
            else {
                v.texCoord = glm::vec2(0.0f);
            }
            if (i < normals.size()) {
                v.normal = normals[i];
            }
            else {
                v.normal = glm::vec3(0.0f, 0.0f, 1.0f);
            }
            mesh_vertices.push_back(v);
        }

        // Vytvoøení indexù - jednoduché sekvenèní indexování
        std::vector<GLuint> indices;
        for (GLuint i = 0; i < mesh_vertices.size(); i++) {
            indices.push_back(i);
        }

        // Vytvoøení meshe
        Mesh mesh(GL_TRIANGLES, shader, mesh_vertices, indices, glm::vec3(0.0f), glm::vec3(0.0f));
        meshes.push_back(mesh);
    }

    // update position etc. based on running time
    void update(const float delta_t) {
        // Zde mùžete implementovat automatické animace
        // Napø.: orientation.y += 1.0f * delta_t; // otáèení kolem Y osy
    }

    // Metoda pro získání model matice
    glm::mat4 getModelMatrix() const {
        // Výpoèet kompletní transformaèní matice
        glm::mat4 t = glm::translate(glm::mat4(1.0f), origin);
        glm::mat4 rx = glm::rotate(glm::mat4(1.0f), orientation.x, glm::vec3(1.0f, 0.0f, 0.0f));
        glm::mat4 ry = glm::rotate(glm::mat4(1.0f), orientation.y, glm::vec3(0.0f, 1.0f, 0.0f));
        glm::mat4 rz = glm::rotate(glm::mat4(1.0f), orientation.z, glm::vec3(0.0f, 0.0f, 1.0f));
        glm::mat4 s = glm::scale(glm::mat4(1.0f), scale);

        return local_model_matrix * s * rz * ry * rx * t;
    }

    void draw(glm::vec3 const& offset = glm::vec3(0.0f),
        glm::vec3 const& rotation = glm::vec3(0.0f),
        glm::vec3 const& scale_change = glm::vec3(1.0f)) {

        // Aktivace shaderu
        shader.activate();

        // Vykreslení všech meshù
        for (auto& mesh : meshes) {
            mesh.draw();
        }
    }

    // Pøetížený draw s pøímým zadáním model matice
    void draw(glm::mat4 const& model_matrix) {
        shader.activate();
        for (auto& mesh : meshes) {
            mesh.draw();
        }
    }
};