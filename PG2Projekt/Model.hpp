#pragma once

#include <filesystem>
#include <string>
#include <vector> 
#include <glm/glm.hpp> 

#include "assets.hpp"
#include "Mesh.hpp"
#include "ShaderProgram.hpp"
#include "OBJloader.hpp"

class Model {
public:
    std::vector<Mesh> meshes;
    std::string name;
    glm::vec3 origin{};
    glm::vec3 orientation{};
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
        // origin += glm::vec3(3,0,0) * delta_t; // s = s0 + v*dt
    }

    void draw(glm::vec3 const& offset = glm::vec3(0.0), glm::vec3 const& rotation = glm::vec3(0.0f)) {
        // call draw() on mesh (all meshes)
        for (auto const& mesh : meshes) {
            mesh.draw(origin + offset, orientation + rotation);
        }
    }
};