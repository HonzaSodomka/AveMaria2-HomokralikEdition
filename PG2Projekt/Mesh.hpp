#pragma once
#include <string>
#include <vector>
#include <iostream>
#include <glm/glm.hpp> 
#include <glm/ext.hpp>
#include "assets.hpp"
#include "ShaderProgram.hpp"

class Mesh {
public:
    // mesh data
    std::vector<vertex> vertices;
    std::vector<GLuint> indices;
    glm::vec3 origin{};
    glm::vec3 orientation{};
    GLuint texture_id{ 0 }; // texture id=0  means no texture
    GLenum primitive_type = GL_POINT;
    ShaderProgram shader;

    // mesh material
    glm::vec4 ambient_material{ 1.0f }; //white, non-transparent 
    glm::vec4 diffuse_material{ 1.0f }; //white, non-transparent 
    glm::vec4 specular_material{ 1.0f }; //white, non-transparent
    float reflectivity{ 1.0f };

    // indirect (indexed) draw 
    Mesh(GLenum primitive_type, ShaderProgram shader, std::vector<vertex> const& vertices,
        std::vector<GLuint> const& indices, glm::vec3 const& origin,
        glm::vec3 const& orientation, GLuint const texture_id = 0) :
        primitive_type(primitive_type),
        shader(shader),
        vertices(vertices),
        indices(indices),
        origin(origin),
        orientation(orientation),
        texture_id(texture_id)
    {
        // Vytvo�en� VAO
        glCreateVertexArrays(1, &VAO);

        // Vytvo�en� VBO a nahr�n� dat vrchol�
        glCreateBuffers(1, &VBO);
        glNamedBufferData(VBO, vertices.size() * sizeof(vertex), vertices.data(), GL_STATIC_DRAW);

        // Vytvo�en� EBO a nahr�n� index�
        glCreateBuffers(1, &EBO);
        glNamedBufferData(EBO, indices.size() * sizeof(GLuint), indices.data(), GL_STATIC_DRAW);

        // Nastaven� atribut�
        // Pozice vrcholu
        GLint position_attrib_location = glGetAttribLocation(shader.getID(), "aPos");
        if (position_attrib_location >= 0) {
            glEnableVertexArrayAttrib(VAO, position_attrib_location);
            glVertexArrayAttribFormat(VAO, position_attrib_location, 3, GL_FLOAT, GL_FALSE,
                offsetof(vertex, position));
            glVertexArrayAttribBinding(VAO, position_attrib_location, 0);
        }

        // Norm�la vrcholu
        GLint normal_attrib_location = glGetAttribLocation(shader.getID(), "aNorm");
        if (normal_attrib_location >= 0) {
            glEnableVertexArrayAttrib(VAO, normal_attrib_location);
            glVertexArrayAttribFormat(VAO, normal_attrib_location, 3, GL_FLOAT, GL_FALSE,
                offsetof(vertex, normal));
            glVertexArrayAttribBinding(VAO, normal_attrib_location, 0);
        }

        // Texturov� koordin�ty
        GLint tex_attrib_location = glGetAttribLocation(shader.getID(), "aTex");
        if (tex_attrib_location >= 0) {
            glEnableVertexArrayAttrib(VAO, tex_attrib_location);
            glVertexArrayAttribFormat(VAO, tex_attrib_location, 2, GL_FLOAT, GL_FALSE,
                offsetof(vertex, texCoord));
            glVertexArrayAttribBinding(VAO, tex_attrib_location, 0);
        }

        // Propojen� VAO s VBO a EBO
        glVertexArrayVertexBuffer(VAO, 0, VBO, 0, sizeof(vertex));
        glVertexArrayElementBuffer(VAO, EBO);
    }

    // Metoda draw s v�choz�mi hodnotami pro argumenty
    void draw(glm::vec3 const& offset = glm::vec3(0.0f), glm::vec3 const& rotation = glm::vec3(0.0f)) const {
        if (VAO == 0) {
            std::cerr << "VAO not initialized!\n";
            return;
        }

        // Aktivace textury, pokud existuje
        if (texture_id != 0) {
            // Nastaven� textury na jednotku 0
            glBindTextureUnit(0, texture_id);

            // P�ed�n� ��sla texturov� jednotky do shaderu
            GLint tex_loc = glGetUniformLocation(shader.getID(), "tex0");
            if (tex_loc >= 0) {
                glUniform1i(tex_loc, 0);
            }
        }

        // Vykreslen� meshe
        glBindVertexArray(VAO);
        glDrawElements(primitive_type, static_cast<GLsizei>(indices.size()), GL_UNSIGNED_INT, 0);
        glBindVertexArray(0); // Unbind VAO
    }

    void clear(void) {
        // Uvoln�n� textury
        if (texture_id != 0) {
            glDeleteTextures(1, &texture_id);
            texture_id = 0;
        }

        primitive_type = GL_POINT;
        vertices.clear();
        indices.clear();
        origin = glm::vec3(0.0f);
        orientation = glm::vec3(0.0f);

        // Uvoln�n� OpenGL objekt�
        if (VAO != 0) {
            glDeleteVertexArrays(1, &VAO);
            VAO = 0;
        }
        if (VBO != 0) {
            glDeleteBuffers(1, &VBO);
            VBO = 0;
        }
        if (EBO != 0) {
            glDeleteBuffers(1, &EBO);
            EBO = 0;
        }
    }

private:
    // OpenGL buffer IDs
    // ID = 0 is reserved (i.e. uninitalized)
    unsigned int VAO{ 0 }, VBO{ 0 }, EBO{ 0 };
};