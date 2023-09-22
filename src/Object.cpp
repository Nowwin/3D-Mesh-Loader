
#include "Object.hpp"

Object::Object(const std::string& object_file_path) {
    

    std::string line = "";
    std::ifstream myFile(object_file_path.c_str());

    if (myFile.is_open())
    {
        while (std::getline(myFile, line))
        {
            if (line[0] != '#')
            {
                if (line[0] == 'v')
                {
                    std::stringstream ss(line);

                    char v;
                    GLfloat num1, num2, num3;

                    ss >> v >> num1 >> num2 >> num3;

                    this->vertices_.push_back(Vertex(glm::vec3(num1, num2, num3), glm::vec3(0, 0, 0)));
                   
                } else if (line[0] == 'f')
                {
                    std::stringstream ss(line);

                    char v;
                    GLuint num1, num2, num3;

                    ss >> v >> num1 >> num2 >> num3;

                    this->indices_.push_back(num1 - 1);
                    this->indices_.push_back(num2 - 1);
                    this->indices_.push_back(num3 - 1);

                }
                
                
            }
            
        }
        myFile.close();   
    }

    
}

const std::vector<GLfloat> Object::vertices() {
    std::vector<GLfloat> vertexInfo;
    
    for (auto vertex : this->vertices_)
    {
        vertexInfo.push_back(vertex.position_.x);
        vertexInfo.push_back(vertex.position_.y);
        vertexInfo.push_back(vertex.position_.z);
    }

    return vertexInfo;    
}

const std::vector<GLuint> Object::indices() {
    return this->indices_;  
}