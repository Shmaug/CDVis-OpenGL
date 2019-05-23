#pragma once

#include <gl/glew.h>
#include <glm/glm.hpp>

#include <memory>
#include <string>

#include "../Pipeline/Texture.hpp"

// Win32 LoadImage macro
#ifdef LoadImage
#undef LoadImage
#endif

class ImageLoader {
public:
	static std::shared_ptr<Texture> LoadImage(const std::string& imagePath, glm::vec3& size);
	static std::shared_ptr<Texture> LoadVolume(const std::string& folder, glm::vec3& size);
	static void LoadMask(const std::string& folder, const std::shared_ptr<Texture>& texture);
};