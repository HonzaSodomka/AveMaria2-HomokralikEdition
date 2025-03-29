#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>
#include <glm/glm.hpp>
#include "OBJloader.hpp"

bool loadOBJ(
    const std::string& path,
    std::vector<glm::vec3>& out_vertices,
    std::vector<glm::vec2>& out_uvs,
    std::vector<glm::vec3>& out_normals
) {
    std::cout << "Loading OBJ file: " << path << std::endl;

    // Vyèištìní výstupních vektorù
    out_vertices.clear();
    out_uvs.clear();
    out_normals.clear();

    // Doèasné vektory pro naèítání
    std::vector<unsigned int> vertexIndices, uvIndices, normalIndices;
    std::vector<glm::vec3> temp_vertices;
    std::vector<glm::vec2> temp_uvs;
    std::vector<glm::vec3> temp_normals;

    // Otevøení souboru
    std::ifstream file(path);
    if (!file.is_open()) {
        std::cerr << "Impossible to open the file: " << path << std::endl;
        return false;
    }

    // Naèítání souboru po øádcích
    std::string line;
    while (std::getline(file, line)) {
        std::istringstream iss(line);
        std::string prefix;
        iss >> prefix;

        if (prefix == "v") {
            // Vrchol
            glm::vec3 vertex;
            iss >> vertex.x >> vertex.y >> vertex.z;
            temp_vertices.push_back(vertex);
        }
        else if (prefix == "vt") {
            // Texturovací koordináty
            glm::vec2 uv;
            iss >> uv.x >> uv.y;
            temp_uvs.push_back(uv);
        }
        else if (prefix == "vn") {
            // Normála
            glm::vec3 normal;
            iss >> normal.x >> normal.y >> normal.z;
            temp_normals.push_back(normal);
        }
        else if (prefix == "f") {
            // Ploška (trojúhelník)
            unsigned int vertexIndex[3], uvIndex[3], normalIndex[3];
            char slash;

            // Parsování formátu "f v1/vt1/vn1 v2/vt2/vn2 v3/vt3/vn3"
            for (int i = 0; i < 3; i++) {
                iss >> vertexIndex[i] >> slash >> uvIndex[i] >> slash >> normalIndex[i];

                // Konverze z 1-based na 0-based indexování
                vertexIndices.push_back(vertexIndex[i] - 1);
                uvIndices.push_back(uvIndex[i] - 1);
                normalIndices.push_back(normalIndex[i] - 1);
            }
        }
        // Ignorujeme ostatní typy øádkù (komentáøe, názvy skupin atd.)
    }

    // Zpracování indexovaných dat do lineárního pole
    for (unsigned int i = 0; i < vertexIndices.size(); i++) {
        unsigned int vertexIndex = vertexIndices[i];
        unsigned int uvIndex = uvIndices[i];
        unsigned int normalIndex = normalIndices[i];

        // Kontrola platnosti indexù
        if (vertexIndex >= temp_vertices.size() ||
            uvIndex >= temp_uvs.size() ||
            normalIndex >= temp_normals.size()) {
            std::cerr << "Invalid index in OBJ file: " << path << std::endl;
            return false;
        }

        out_vertices.push_back(temp_vertices[vertexIndex]);
        out_uvs.push_back(temp_uvs[uvIndex]);
        out_normals.push_back(temp_normals[normalIndex]);
    }

    std::cout << "OBJ loaded successfully: " << out_vertices.size() << " vertices" << std::endl;
    return true;
}