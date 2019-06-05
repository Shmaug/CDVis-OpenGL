#include "MeshRenderer.hpp"

class TextRenderer : MeshRenderer {
public:
	TextRenderer();
	~TextRenderer();

	inline void Text(std::string text) { mDirty = true; mText = text; };
	inline std::string Text() const { return mText; }

private:
	bool mDirty;
	std::string mText;
};