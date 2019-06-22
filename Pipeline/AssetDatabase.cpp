#include "AssetDatabase.hpp"

using namespace std;
using namespace glm;

shared_ptr<Mesh> AssetDatabase::gWireCubeMesh;
shared_ptr<Mesh> AssetDatabase::gCubeMesh;
shared_ptr<Mesh> AssetDatabase::gLightMesh;
shared_ptr<Mesh> AssetDatabase::gPenMesh;
shared_ptr<Mesh> AssetDatabase::gDialMesh;

shared_ptr<Texture> AssetDatabase::gPenTexture;
shared_ptr<Texture> AssetDatabase::gDialTexture;
shared_ptr<Texture> AssetDatabase::gPieIconTexture;
shared_ptr<Texture> AssetDatabase::gIconTexture;

shared_ptr<Shader> AssetDatabase::gBlitShader;
shared_ptr<Shader> AssetDatabase::gPieShader;
shared_ptr<Shader> AssetDatabase::gTexturedShader;
shared_ptr<Shader> AssetDatabase::gVolumeShader;
shared_ptr<Shader> AssetDatabase::gVolumeComputeShader;

void AssetDatabase::LoadAssets() {
	gLightMesh = shared_ptr<Mesh>(new Mesh("Assets/light.obj"));
	gPenMesh = shared_ptr<Mesh>(new Mesh("Assets/pen.obj"));
	gDialMesh = shared_ptr<Mesh>(new Mesh("Assets/dial.obj"));

	gBlitShader = shared_ptr<Shader>(new Shader());
	gBlitShader->AddShaderFile(GL_VERTEX_SHADER, "Assets/blit.vert");
	gBlitShader->AddShaderFile(GL_FRAGMENT_SHADER, "Assets/blit.frag");
	gBlitShader->CompileAndLink();

	gPieShader = shared_ptr<Shader>(new Shader());
	gPieShader->AddShaderFile(GL_VERTEX_SHADER, "Assets/pie.vert");
	gPieShader->AddShaderFile(GL_FRAGMENT_SHADER, "Assets/pie.frag");
	gPieShader->CompileAndLink();

	gTexturedShader = shared_ptr<Shader>(new Shader());
	gTexturedShader->AddShaderFile(GL_VERTEX_SHADER, "Assets/textured.vert");
	gTexturedShader->AddShaderFile(GL_FRAGMENT_SHADER, "Assets/textured.frag");
	gTexturedShader->CompileAndLink();

	gVolumeShader = shared_ptr<Shader>(new Shader());
	gVolumeShader->AddShaderFile(GL_VERTEX_SHADER, "Assets/volume.vert");
	gVolumeShader->AddShaderFile(GL_FRAGMENT_SHADER, "Assets/volume.frag");
	gVolumeShader->CompileAndLink();

	gVolumeComputeShader = shared_ptr<Shader>(new Shader());
	gVolumeComputeShader->AddShaderFile(GL_COMPUTE_SHADER, "Assets/volume.glsl");
	gVolumeComputeShader->CompileAndLink();

	gDialTexture = shared_ptr<Texture>(new Texture("Assets/dial_diffuse.png"));
	gIconTexture = shared_ptr<Texture>(new Texture("Assets/icons.png"));
	gPenTexture = shared_ptr<Texture>(new Texture("Assets/pen_diffuse.png"));
	gPieIconTexture = shared_ptr<Texture>(new Texture("Assets/pie_icons.png"));

	#pragma region cube
	gCubeMesh = shared_ptr<Mesh>(new Mesh());
	gCubeMesh->BindVAO();

	static const vec3 cubeVertices[8] {
		vec3(-0.5f, -0.5f,  0.5f),
		vec3( 0.5f, -0.5f,  0.5f),
		vec3( 0.5f,  0.5f,  0.5f),
		vec3(-0.5f,  0.5f,  0.5f),
		vec3(-0.5f, -0.5f, -0.5f),
		vec3( 0.5f, -0.5f, -0.5f),
		vec3( 0.5f,  0.5f, -0.5f),
		vec3(-0.5f,  0.5f, -0.5f)
	};
	static const GLuint cubeIndices[36] {
		0, 1, 2, 2, 3, 0,
		1, 5, 6, 6, 2, 1,
		7, 6, 5, 5, 4, 7,
		4, 0, 3, 3, 7, 4,
		4, 5, 1, 1, 0, 4,
		3, 2, 6, 6, 7, 3
	};

	gCubeMesh->BindVBO();
	glBufferData(GL_ARRAY_BUFFER, sizeof(vec3) * 8, cubeVertices, GL_STATIC_DRAW);

	gCubeMesh->BindIBO();
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * 36, cubeIndices, GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 3, nullptr);

	gCubeMesh->ElementCount(36);
	glBindVertexArray(0);

	gCubeMesh->Bounds(::AABB(vec3(0.f), vec3(.5f)));
	#pragma endregion
	
	#pragma region wire cube
	gWireCubeMesh = shared_ptr<Mesh>(new Mesh());
	gWireCubeMesh->BindVAO();

	static const vec3 wireCubeVertices[8] {
		vec3(-1.f,  1.f, -1.f),
		vec3(-1.f,  1.f,  1.f),
		vec3( 1.f,  1.f,  1.f),
		vec3( 1.f,  1.f, -1.f),

		vec3(-1.f, -1.f, -1.f),
		vec3(-1.f, -1.f,  1.f),
		vec3( 1.f, -1.f,  1.f),
		vec3( 1.f, -1.f, -1.f)
	};
	static const GLuint wireCubeIndices[24]{
		0, 1, 1, 2, 2, 3, 3, 0,
		4, 5, 5, 6, 6, 7, 7, 4,
		0, 4, 1, 5, 2, 6, 3, 7
	};

	gWireCubeMesh->BindVBO();
	glBufferData(GL_ARRAY_BUFFER, sizeof(vec3) * 8, wireCubeVertices, GL_STATIC_DRAW);

	gWireCubeMesh->BindIBO();
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * 24, wireCubeIndices, GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 3, nullptr);

	gWireCubeMesh->ElementCount(24);
	glBindVertexArray(0);

	gWireCubeMesh->Bounds(::AABB(vec3(0.f), vec3(1.f)));
	#pragma endregion
}

void AssetDatabase::Cleanup() {
	gWireCubeMesh.reset();
	gCubeMesh.reset();
	gPenMesh.reset();
	gDialMesh.reset();

	gPenTexture.reset();
	gDialTexture.reset();
	gPieIconTexture.reset();
	gIconTexture.reset();

	gBlitShader.reset();
	gPieShader.reset();
	gTexturedShader.reset();
	gVolumeShader.reset();
	gVolumeComputeShader.reset();
}