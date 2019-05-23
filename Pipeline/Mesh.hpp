#pragma once

#include <gl/glew.h>

class Mesh {
public:
	Mesh();
	~Mesh();

	void BindVAO() const;
	void BindVBO() const;
	void BindIBO() const;

	inline void ElementCount(unsigned int x) {
		mElementCount = x;
	}
	inline unsigned int ElementCount() const {
		return mElementCount;
	}

private:
	unsigned int mElementCount;
	GLuint mVAO;
	GLuint mVBO;
	GLuint mIBO;
};