#pragma once

#include <gl/glew.h>

#include <string>

class Texture {
public:
	Texture(const std::string& filename);
	Texture(unsigned int width, unsigned int height, GLenum internalFormat, GLenum format, GLenum type, GLenum filter);
	Texture(unsigned int width, unsigned int height, GLenum internalFormat, GLenum format, GLenum type, GLenum filter, void* data);
	Texture(unsigned int width, unsigned int height, unsigned int depth, GLenum internalFormat, GLenum format, GLenum type, GLenum filter);
	Texture(unsigned int width, unsigned int height, unsigned int depth, GLenum internalFormat, GLenum format, GLenum type, GLenum filter, void* data);
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