#pragma once

#include "Font.hpp"
#include "Mesh.hpp"
#include "Shader.hpp"
#include "Texture.hpp"

class AssetDatabase {
public:
	static void LoadAssets();
	static void Cleanup();

	static std::shared_ptr<Mesh> gCubeMesh;
	static std::shared_ptr<Mesh> gPenMesh;
	static std::shared_ptr<Mesh> gDialMesh;

	static std::shared_ptr<Texture> gPenTexture;
	static std::shared_ptr<Texture> gDialTexture;
	static std::shared_ptr<Texture> gPieIconTexture;
	static std::shared_ptr<Texture> gIconTexture;

	static std::shared_ptr<Shader> gBlitShader;
	static std::shared_ptr<Shader> gPieShader;
	static std::shared_ptr<Shader> gTexturedShader;
	static std::shared_ptr<Shader> gVolumeShader;
	static std::shared_ptr<Shader> gVolumeComputeShader;
};