#include "Mesh.hpp"

#include "../ThirdParty/tiny_obj_loader.hpp"

using namespace std;

Mesh::Mesh(const string& filename) {

}

Mesh::Mesh() : mElementCount(6), mBounds(AABB(glm::vec3(), glm::vec3())) {
	glGenVertexArrays(1, &mVAO);
	glGenBuffers(1, &mVBO);
	glGenBuffers(1, &mIBO);
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