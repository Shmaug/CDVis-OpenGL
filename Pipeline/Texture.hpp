#pragma once

#include <gl/glew.h>

class Texture {
public:
	Texture(GLenum internalFormat, GLenum format, GLenum type, unsigned int width, unsigned int height);
	Texture(GLenum internalFormat, GLenum format, GLenum type, unsigned int width, unsigned int height, unsigned int depth);
	Texture(GLenum internalFormat, GLenum format, GLenum type, unsigned int width, unsigned int height, void* data);
	Texture(GLenum internalFormat, GLenum format, GLenum type, unsigned int width, unsigned int height, unsigned int depth, void* data);
	~Texture();

	unsigned int Width() const { return mWidth; }
	unsigned int Height() const { return mHeight; }
	unsigned int Depth() const { return mDepth; }
	GLuint GLTexture() const { return mTexture; }

private:
	unsigned int mWidth;
	unsigned int mHeight;
	unsigned int mDepth;
	GLenum mFormat;
	GLenum mType;
	GLenum mInternalFormat;
	GLuint mTexture;
};