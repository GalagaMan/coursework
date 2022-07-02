#include "Mesh.h"


Vertex::Vertex(glm::vec4 position, glm::vec4 normal, glm::vec4 color)
	:position(position), normal(normal), color(color)
{
	
}

Vertex::Vertex(Vertex const& vertex)
{
	this->position = vertex.position;
	this->normal = vertex.normal;
	this->color = vertex.color;
}


Mesh::Mesh(std::vector<Vertex> vertices)
	:vertices(std::move(vertices))
{

}

Mesh::Mesh(const Mesh& mesh)
{
	vertices = mesh.vertices;
}
