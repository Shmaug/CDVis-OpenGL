#include "Texture.hpp"

#include "../ThirdParty/stb_image.hpp"

using namespace std;

Texture::Texture(const string& filename) {
	int x, y, channels;
	if (stbi_uc* res = stbi_load(filename.c_str(), &x, &y, &channels, 0)) {
		mWidth = x;
		mHeight = x;
		mDepth = 1;

		switch (channels) {
		case 1:
			mInternalFormat = GL_R8;
			mFormat = GL_R;
			break;
		case 2:
			mInternalFormat = GL_RG8;
			mFormat = GL_RG;
			break;
		case 3:
			mInternalFormat = GL_RGB8;
			mFormat = GL_RGB;
			break;
		default:
		case 4:
			mInternalFormat = GL_RGBA8;
			mFormat = GL_RGBA;
			break;
		}
		mType = GL_UNSIGNED_BYTE;

		glGenTextures(1, &mTexture);
		glBindTexture(GL_TEXTURE_2D, mTexture);
		glTexImage3D(GL_TEXTURE_2D, 0, mInternalFormat, mWidth, mHeight, mDepth, 0, mFormat, mType, res);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		glBindTexture(GL_TEXTURE_2D, 0);

		stbi_image_free(res);

	} else {
		printf("Failed to load %s: %s\n", filename.c_str(), stbi_failure_reason());
		mInternalFormat = GL_RGBA8;
		mFormat = GL_RGBA;
		mType = GL_UNSIGNED_BYTE;
		mWidth = 0;
		mHeight = 0;
		mDepth = 0;
		mTexture = 0;
	}
}

Texture::Texture(unsigned int width, unsigned int height, GLenum internalFormat, GLenum format, GLenum type, GLenum filter)
	: mTexture(0), mInternalFormat(internalFormat), mFormat(format), mType(type), mWidth(width), mHeight(height), mDepth(0) {
	glGenTextures(1, &mTexture);
	glBindTexture(GL_TEXTURE_2D, mTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, mInternalFormat, mWidth, mHeight, 0, mFormat, mType, nullptr);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter);

	glBindTexture(GL_TEXTURE_2D, 0);
}
Texture::Texture(unsigned int width, unsigned int height, GLenum internalFormat, GLenum format, GLenum type, GLenum filter, void* data)
	: mTexture(0), mInternalFormat(internalFormat), mFormat(format), mType(type), mWidth(width), mHeight(height), mDepth(0) {
	glGenTextures(1, &mTexture);
	glBindTexture(GL_TEXTURE_2D, mTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, mInternalFormat, mWidth, mHeight, 0, mFormat, mType, data);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter);

	glBindTexture(GL_TEXTURE_2D, 0);
}

Texture::Texture(unsigned int width, unsigned int height, unsigned int depth, GLenum internalFormat, GLenum format, GLenum type, GLenum filter)
	: mTexture(0), mInternalFormat(internalFormat), mFormat(format), mType(type), mWidth(width), mHeight(height), mDepth(depth) {
	glGenTextures(1, &mTexture);
	glBindTexture(GL_TEXTURE_3D, mTexture);
	glTexImage3D(GL_TEXTURE_3D, 0, mInternalFormat, mWidth, mHeight, mDepth, 0, mFormat, mType, nullptr);

	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_REPEAT);

	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, filter);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, filter);

	glBindTexture(GL_TEXTURE_3D, 0);
}
Texture::Texture(unsigned int width, unsigned int height, unsigned int depth, GLenum internalFormat, GLenum format, GLenum type, GLenum filter, void* data)
	: mTexture(0), mInternalFormat(internalFormat), mFormat(format), mType(type), mWidth(width), mHeight(height), mDepth(depth) {
	glGenTextures(1, &mTexture);
	glBindTexture(GL_TEXTURE_3D, mTexture);
	glTexImage3D(GL_TEXTURE_3D, 0, mInternalFormat, mWidth, mHeight, mDepth, 0, mFormat, mType, data);

	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_REPEAT);

	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, filter);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, filter);

	glBindTexture(GL_TEXTURE_3D, 0);
}

Texture::~Texture() {
	glDeleteTextures(1, &mTexture);
}