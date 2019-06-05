#pragma once

#include "Texture.hpp"

#include <memory>
#include <unordered_map>

class Font {
public:
	struct FontGlyph {
		char character;
		unsigned int advance;
		int ox;
		int oy;
		// texture rect
		unsigned int tx0;
		unsigned int ty0;
		unsigned int tw;
		unsigned int th;

		FontGlyph() : character('\0'), advance(0), ox(0), oy(0), tx0(0), ty0(0), tw(0), th(0) {};
		~FontGlyph() {};
	};
	Font(const std::string& filename);

	inline std::shared_ptr<Texture> GetTexture() const { return mTexture; };

	inline int GetKerning(const char from, const char to) const {
		auto& it = mKerning.find((uint32_t)((unsigned int)to | ((unsigned int)from << 16)));
		return (it == mKerning.end()) ? 0 : (*it).second;
	};
	bool GetGlyph(const char c, FontGlyph& g) const;

	inline unsigned int GetSize() const { return mSize; };
	inline unsigned int GetLineSpacing() const { return mLineSpace; };
	inline unsigned int GetHeight() const { return mHeight; };
	inline unsigned int GetAscender() const { return mAscender; };
	inline unsigned int GetDescender() const { return mDescender; };
	inline unsigned int GetTextureDpi() const { return mTexDpi; }

private:
	struct FontKerning {
		char from;
		char to;
		int offset;
	};

	unsigned int mSize;
	unsigned int mHeight;
	unsigned int mAscender;
	unsigned int mDescender;
	unsigned int mLineSpace;
	unsigned int mTexDpi;

	std::vector<FontGlyph> mGlyphs;
	std::vector<unsigned int> mGlyphIndices;
	std::unordered_map<uint32_t, int> mKerning;
	std::shared_ptr<Texture> mTexture;
};

