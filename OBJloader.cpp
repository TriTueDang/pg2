#include <string>
#include <algorithm>
#include <GL/glew.h> 
#include <glm/glm.hpp>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>

#include "OBJloader.hpp"

bool loadOBJ(const std::filesystem::path& filename, std::vector<Vertex>& vertices, std::vector<GLuint>& indices)
{
	std::cout << "Opening OBJ file: " << filename.string() << std::endl;

	std::vector< glm::vec3 > temp_vertices;
	std::vector< glm::vec2 > temp_uvs;
	std::vector< glm::vec3 > temp_normals;

	vertices.clear();
	indices.clear();

	std::ifstream file(filename);
	if (!file.is_open()) {
		std::cerr << "CRITICAL: Could not open file: " << filename << " (Checking in: " << std::filesystem::current_path() << ")" << std::endl;
		return false;
	}

	std::string line;
	int lineNum = 0;
	while (std::getline(file, line)) {
		lineNum++;
		if (line.empty() || line[0] == '#') continue;
		
		std::stringstream ss(line);
		std::string head;
		ss >> head;

		if (head == "v") {
			glm::vec3 vertex;
			if (!(ss >> vertex.x >> vertex.y >> vertex.z)) {
				std::cerr << "Error parsing vertex at line " << lineNum << std::endl;
				continue;
			}
			temp_vertices.push_back(vertex);
		}
		else if (head == "vt") {
			glm::vec2 uv;
			if (!(ss >> uv.x >> uv.y)) {
				// vt can have 3 components (u, v, w), we only need 2
				ss.clear();
				ss.str(line);
				ss >> head >> uv.x >> uv.y;
			}
			temp_uvs.push_back(uv);
		}
		else if (head == "vn") {
			glm::vec3 normal;
			if (!(ss >> normal.x >> normal.y >> normal.z)) {
				std::cerr << "Error parsing normal at line " << lineNum << std::endl;
				continue;
			}
			temp_normals.push_back(normal);
		}
		else if (head == "f") {
			std::string vertexStr;
			for (int i = 0; i < 3; i++) {
				if (!(ss >> vertexStr)) {
					std::cerr << "Error reading face vertex at line " << lineNum << std::endl;
					break;
				}
				
				unsigned int vIdx = 0, tIdx = 0, nIdx = 0;
				// OBJ faces can be: v, v/t, v//n, v/t/n
				size_t firstSlash = vertexStr.find('/');
				if (firstSlash == std::string::npos) {
					vIdx = std::stoi(vertexStr);
				} else {
					vIdx = std::stoi(vertexStr.substr(0, firstSlash));
					size_t secondSlash = vertexStr.find('/', firstSlash + 1);
					if (secondSlash == std::string::npos) {
						tIdx = std::stoi(vertexStr.substr(firstSlash + 1));
					} else {
						if (secondSlash > firstSlash + 1) {
							tIdx = std::stoi(vertexStr.substr(firstSlash + 1, secondSlash - firstSlash - 1));
						}
						nIdx = std::stoi(vertexStr.substr(secondSlash + 1));
					}
				}

				Vertex v;
				v.Position = temp_vertices[vIdx - 1];
				v.Normal = (nIdx > 0 && nIdx <= temp_normals.size()) ? temp_normals[nIdx - 1] : glm::vec3(0, 0, 1);
				v.TexCoords = (tIdx > 0 && tIdx <= temp_uvs.size()) ? temp_uvs[tIdx - 1] : glm::vec2(0, 0);

				auto it = std::find(vertices.begin(), vertices.end(), v);
				if (it == vertices.end()) {
					vertices.push_back(v);
					indices.push_back(vertices.size() - 1);
				} else {
					indices.push_back(std::distance(vertices.begin(), it));
				}
			}
		}
	}
	
	std::cout << "Successfully loaded: " << filename.string() << " (" << vertices.size() << " vertices, " << indices.size() << " indices)" << std::endl;
	return true;
}
