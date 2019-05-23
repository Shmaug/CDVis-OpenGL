#pragma once

#include <gl/glew.h>
#include <glm/glm.hpp>

#include <unordered_set>
#include <unordered_map>
#include <string>
#include <variant>

#include "Texture.hpp"

class Shader {
public:
	Shader();
	~Shader();

	void AddShaderFile(GLenum type, std::string filename);
	void CompileAndLink();

	GLuint Use();

	void EnableKeyword(std::string keyword);
	void DisableKeyword(std::string keyword);

	// global utility functions
	static void Uniform(GLuint program, const GLchar* name, int x);
	static void Uniform(GLuint program, const GLchar* name, float x);
	static void Uniform(GLuint program, const GLchar* name, const glm::vec3& x);
	static void Uniform(GLuint program, const GLchar* name, const glm::mat4& x);

private:
	struct ShaderProgram {
		GLuint mProgram;
		std::vector<GLuint> mShaders;
	};

	std::unordered_set<std::string> mAvailableKeywords;
	std::unordered_set<std::string> mActiveKeywords;

	// indexed by keyword combo
	std::unordered_map<std::string, ShaderProgram> mPrograms;

	std::unordered_map<GLenum, std::string> mShadersToLink;

	ShaderProgram LinkShader(const std::vector<std::string>& keywords);
};