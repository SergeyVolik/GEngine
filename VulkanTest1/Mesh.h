#ifndef GMESH
#define GMESH

#include <vector>
namespace te
{
	struct Vertex;
	class Mesh
	{
		std::vector<te::Vertex> _vertices;
		std::vector<uint32_t> _indices;
		
		public:
			
			Mesh(std::vector<uint32_t> indices, std::vector<te::Vertex> vertices) : _indices(indices), _vertices(vertices){ }

			void setVertices(std::vector<Vertex> vertices) { _vertices = vertices; }
			std::vector<Vertex> getVertices() { return _vertices; }

			void setIndices(std::vector<uint32_t> indices) { _indices = indices; }
			std::vector<uint32_t> getIndices() { return _indices; }
			
	};
}

#endif // !GMESH
