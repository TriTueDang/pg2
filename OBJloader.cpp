#include <string>
#include <algorithm>
#include <GL/glew.h> 
#include <glm/glm.hpp>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <map>

#include "OBJloader.hpp"

std::vector<MeshData> loadOBJ(const std::filesystem::path& filename)
{
	std::cout << "Opening OBJ file: " << filename.string() << std::endl;

	std::vector<MeshData> results;
	std::vector< glm::vec3 > temp_vertices;
	std::vector< glm::vec2 > temp_uvs;
	std::vector< glm::vec3 > temp_normals;

	std::ifstream file(filename);
	if (!file.is_open()) {
		std::cerr << "CRITICAL: Could not open file: " << filename << " (Checking in: " << std::filesystem::current_path() << ")" << std::endl;
		return results;
	}

	MeshData currentMesh;
	currentMesh.name = "default";
	std::map<Vertex, GLuint> vertexToIndex;

	std::string line;
	int lineNum = 0;

	while (std::getline(file, line)) {
		lineNum++;
		if (line.empty() || line[0] == '#') continue;

		std::stringstream ss(line);
		std::string head;
		ss >> head;

		if (head == "o" || head == "g") {
			// If we have a current mesh with faces, save it before starting a new one
			if (!currentMesh.indices.empty()) {
				results.push_back(std::move(currentMesh));
				currentMesh = MeshData();
				vertexToIndex.clear();
			}
			ss >> currentMesh.name;
		}
		else if (head == "v") {
			glm::vec3 vertex;
			ss >> vertex.x >> vertex.y >> vertex.z;
			temp_vertices.push_back(vertex);
		}
		else if (head == "vt") {
			glm::vec2 uv;
			ss >> uv.x >> uv.y;
			temp_uvs.push_back(uv);
		}
		else if (head == "vn") {
			glm::vec3 normal;
			ss >> normal.x >> normal.y >> normal.z;
			temp_normals.push_back(normal);
		}
		else if (head == "f") {
			std::string vertexStr;
			for (int i = 0; i < 3; i++) {
				if (!(ss >> vertexStr)) break;

				unsigned int vIdx = 0, tIdx = 0, nIdx = 0;
				size_t firstSlash = vertexStr.find('/');
				if (firstSlash == std::string::npos) {
					vIdx = std::stoi(vertexStr);
				}
				else {
					vIdx = std::stoi(vertexStr.substr(0, firstSlash));
					size_t secondSlash = vertexStr.find('/', firstSlash + 1);
					if (secondSlash == std::string::npos) {
						tIdx = std::stoi(vertexStr.substr(firstSlash + 1));
					}
					else {
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

				auto [it, inserted] = vertexToIndex.try_emplace(v, static_cast<GLuint>(currentMesh.vertices.size()));
				if (inserted) {
					currentMesh.vertices.push_back(v);
				}
				currentMesh.indices.push_back(it->second);
			}
		}
	}

	// Push last mesh
	if (!currentMesh.indices.empty()) {
		results.push_back(std::move(currentMesh));
	}

	std::cout << "Successfully loaded: " << filename.string() << " with " << results.size() << " meshes." << std::endl;
	return results;
}

