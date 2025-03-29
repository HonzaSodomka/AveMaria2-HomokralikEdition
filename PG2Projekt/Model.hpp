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

    // Transforma�n� vlastnosti
    glm::vec3 origin{ 0.0f, 0.0f, 0.0f };
    glm::vec3 orientation{ 0.0f, 0.0f, 0.0f };  // v radi�nech
    glm::vec3 scale{ 1.0f, 1.0f, 1.0f };
    glm::mat4 local_model_matrix{ 1.0f }; // identita (��dn� transformace)

    ShaderProgram shader;

    Model(const std::filesystem::path& filename, ShaderProgram shader) {
        this->shader = shader;
        this->name = filename.stem().string();

        // Na�ten� OBJ souboru
        std::vector<glm::vec3> vertices;
        std::vector<glm::vec2> uvs;
        std::vector<glm::vec3> normals;
        bool res = loadOBJ(filename.string(), vertices, uvs, normals);
        if (!res) {
            throw std::runtime_error("Failed to load OBJ file: " + filename.string());
        }

        // P�evod na�ten�ch dat do form�tu, kter� pou��v� na�e struktura vertex
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

        // Vytvo�en� index� - jednoduch� sekven�n� indexov�n�
        std::vector<GLuint> indices;
        for (GLuint i = 0; i < mesh_vertices.size(); i++) {
            indices.push_back(i);
        }

        // Vytvo�en� meshe
        Mesh mesh(GL_TRIANGLES, shader, mesh_vertices, indices, glm::vec3(0.0f), glm::vec3(0.0f));
        meshes.push_back(mesh);
    }

    // update position etc. based on running time
    void update(const float delta_t) {
        // Zde m��ete implementovat automatick� animace
        // Nap�.: orientation.y += 1.0f * delta_t; // ot��en� kolem Y osy
    }

    // Metoda pro z�sk�n� model matice
    glm::mat4 getModelMatrix() const {
        // V�po�et kompletn� transforma�n� matice
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

        // Vykreslen� v�ech mesh�
        for (auto& mesh : meshes) {
            mesh.draw();
        }
    }

    // P�et�en� draw s p��m�m zad�n�m model matice
    void draw(glm::mat4 const& model_matrix) {
        shader.activate();
        for (auto& mesh : meshes) {
            mesh.draw();
        }
    }
};