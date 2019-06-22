#include "Mesh.hpp"

#include <fstream>

#include "../ThirdParty/tiny_obj_loader.hpp"

#pragma warning(disable:26451)

using namespace glm;
using namespace std;

Mesh::Mesh() : mElementCount(0), mBounds(AABB(glm::vec3(), glm::vec3())) {
	glGenVertexArrays(1, &mVAO);
	glGenBuffers(1, &mVBO);
	glGenBuffers(1, &mIBO);
}
Mesh::Mesh(const string& filename) : Mesh() {
	tinyobj::attrib_t attrib;
	vector<tinyobj::shape_t> shapes;
	vector<tinyobj::material_t> materials;
	string err;
	if (tinyobj::LoadObj(&attrib, &shapes, &materials, &err, filename.c_str())) {
		BindVAO();

		struct vertex {
			vec3 pos;
			vec3 normal;
			vec2 uv;
		};
		vector<vertex> vertices;
		vector<GLuint> indices;

		GLuint index = 0;
		vec3 mn;
		vec3 mx;

		for (const auto& s : shapes) {
			for (const auto& i : s.mesh.indices) {
				vec3& v = *(vec3*)&attrib.vertices[3 * i.vertex_index];
				vec3& vn = i.normal_index > -1 ?  *(vec3*)&attrib.normals[3 * i.normal_index] : vec3();
				vec2& t = i.texcoord_index > -1 ? *(vec2*)&attrib.texcoords[2 * i.texcoord_index] : vec2();

				if (index == 0)
					mn = mx = v;
				else {
					mn = min(mn, v);
					mx = max(mx, v);
				}

				vertices.push_back({ v, vn, t});
				indices.push_back(index++);
			}
		}
		BindVBO();
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertex) * vertices.size(), vertices.data(), GL_STATIC_DRAW);

		BindIBO();
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * indices.size(), indices.data(), GL_STATIC_DRAW);

		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(vertex), (void*)offsetof(vertex, pos));
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(vertex), (void*)offsetof(vertex, normal));
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(vertex), (void*)offsetof(vertex, uv));

		ElementCount(indices.size());
		glBindVertexArray(0);

		Bounds(::AABB((mn + mx) * .5f, (mx - mn) * .5f));

		printf("%s: %d verts %d tris / %fx%fx%f\n", filename.c_str(), (int)vertices.size(), (int)indices.size() / 3, mx.x - mn.x, mx.y - mn.y, mx.z - mn.z);
	} else {
		printf("Failed to load %s: %s\n", filename.c_str(), err.c_str());
	}
}

Mesh::~Mesh() {
	glDeleteVertexArrays(1, &mVAO);
	glDeleteBuffers(1, &mVBO);
	glDeleteBuffers(1, &mIBO);
}

void Mesh::BindVAO() const {
	glBindVertexArray(mVAO);
}
void Mesh::BindVBO() const {
	glBindBuffer(GL_ARRAY_BUFFER, mVBO);
}
void Mesh::BindIBO() const {
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mIBO);
}