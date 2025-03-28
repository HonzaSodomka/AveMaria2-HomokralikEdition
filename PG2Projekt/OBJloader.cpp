#include "OBJloader.hpp"

bool loadOBJ(
    const std::string& path,
    std::vector<glm::vec3>& out_vertices,
    std::vector<glm::vec2>& out_uvs,
    std::vector<glm::vec3>& out_normals
) {
    std::cout << "Loading OBJ file: " << path << std::endl;

    std::vector<unsigned int> vertexIndices, uvIndices, normalIndices;
    std::vector<glm::vec3> temp_vertices;
    std::vector<glm::vec2> temp_uvs;
    std::vector<glm::vec3> temp_normals;

    std::ifstream file(path);
    if (!file.is_open()) {
        std::cerr << "Impossible to open the file!" << std::endl;
        return false;
    }

    std::string line;
    while (std::getline(file, line)) {
        std::istringstream iss(line);
        std::string prefix;
        iss >> prefix;

        if (prefix == "v") {
            glm::vec3 vertex;
            iss >> vertex.x >> vertex.y >> vertex.z;
            temp_vertices.push_back(vertex);
        }
        else if (prefix == "vt") {
            glm::vec2 uv;
            iss >> uv.x >> uv.y;
            temp_uvs.push_back(uv);
        }
        else if (prefix == "vn") {
            glm::vec3 normal;
            iss >> normal.x >> normal.y >> normal.z;
            temp_normals.push_back(normal);
        }
        else if (prefix == "f") {
            std::string vertex1, vertex2, vertex3;
            unsigned int vertexIndex[3], uvIndex[3], normalIndex[3];

            // Formát: f v1/vt1/vn1 v2/vt2/vn2 v3/vt3/vn3
            // v - vertex, vt - texture coord, vn - normal
            char slash;

            for (int i = 0; i < 3; i++) {
                iss >> vertexIndex[i] >> slash >> uvIndex[i] >> slash >> normalIndex[i];

                // OBJ indexy zaèínají od 1, ne 0
                vertexIndices.push_back(vertexIndex[i] - 1);
                uvIndices.push_back(uvIndex[i] - 1);
                normalIndices.push_back(normalIndex[i] - 1);
            }
        }
    }

    // Pro každý vrchol trojúhelníku
    for (unsigned int i = 0; i < vertexIndices.size(); i++) {
        out_vertices.push_back(temp_vertices[vertexIndices[i]]);
        out_uvs.push_back(temp_uvs[uvIndices[i]]);
        out_normals.push_back(temp_normals[normalIndices[i]]);
    }

    return true;
}