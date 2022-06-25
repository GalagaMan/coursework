#pragma once
#include <glm.hpp>
#include <vector>
#include <utility>

struct Vertex
{
	glm::vec4 position{};
	glm::vec4 normal{};
	glm::uvec4 color{};
public:
	Vertex(glm::vec4 position, glm::vec4 normal, glm::vec4 color);
	Vertex(Vertex const& vertex);
};

struct Mesh
{
	std::vector<Vertex> vertices;

	Mesh(std::vector<Vertex> vertices);
	Mesh(const Mesh& mesh);
};

