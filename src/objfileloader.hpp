#ifndef OBJFILELOADER_HPP
#define OBJFILELOADER_HPP

#include <vector>
#include <fstream>
#include <sstream>
#include <string>
#include <iostream>

//Third Party
#include <glm/vec2.hpp> // glm::vec3
#include <glm/vec3.hpp> // glm::vec3


void LoadObj(const std::string& path, std::vector<glm::vec3>& vertices, std::vector<glm::vec3>& normals, std::vector<glm::vec2>& texCoords, std::vector<glm::ivec3>& faces) {
    std::ifstream file(path);
    if (!file.is_open()) {
        std::cerr << "Failed to open the file: " << path << std::endl;
        return;
    }

    std::string line;
    while (std::getline(file, line)) {
        std::istringstream iss(line);
        std::string type;
        iss >> type;

        if (type == "v") {
            glm::vec3 vertex;
            iss >> vertex.x >> vertex.y >> vertex.z;
            vertices.push_back(vertex);
        } else if (type == "vn") {
            glm::vec3 normal;
            iss >> normal.x >> normal.y >> normal.z;
            normals.push_back(normal);
        } else if (type == "vt") {
            glm::vec2 texCoord;
            iss >> texCoord.x >> texCoord.y;
            texCoords.push_back(texCoord);
        } else if (type == "f") {
            glm::ivec3 face;
            char slash; // To skip the '/' characters
            for (int i = 0; i < 3; ++i) {
                iss >> face.x >> slash >> face.y >> slash >> face.z;
                face.x--; // OBJ indexing starts at 1, but C++ vectors start at 0
                face.y--;
                face.z--;
                faces.push_back(face);
            }
        }
    }
}

#endif
