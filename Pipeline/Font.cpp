#include "Font.hpp"

#include "../ThirdParty/stb_truetype.hpp"

using namespace std;

Font::Font(const string& filename) {

}

bool Font::GetGlyph(char c, FontGlyph &g) const {
	if (mGlyphs[mGlyphIndices[c]].character) {
		g = mGlyphs[mGlyphIndices[c]];
		return true;
	}
	return false;
}