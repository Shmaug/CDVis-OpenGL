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

	void AddShaderSource(GLenum type, std::string source);
	void AddShaderFile(GLenum type, std::string filename);
	void CompileAndLink();

	GLuint Use();

	inline void ClearKeywords() { mActiveKeywords.clear(); mKeywordListDirty = false; mKeywordList = ""; }
	void EnableKeyword(const std::string& keyword);
	void DisableKeyword(const std::string& keyword);

	// global utility functions
	static void Uniform(GLuint program, const GLchar* name, int x);
	static void Uniform(GLuint program, const GLchar* name, float x);
	static void Uniform(GLuint program, const GLchar* name, const glm::vec2& x);
	static void Uniform(GLuint program, const GLchar* name, const glm::vec3& x);
	static void Uniform(GLuint program, const GLchar* name, const glm::vec4& x);
	static void Uniform(GLuint program, const GLchar* name, const glm::mat4& x);

private:
	struct ShaderSource {
		std::string mFile;
		std::string mSource;
	};
	struct ShaderProgram {
		GLuint mProgram;
		std::vector<GLuint> mShaders;
	};

	std::unordered_set<std::string> mAvailableKeywords;
	std::unordered_set<std::string> mActiveKeywords;

	bool mKeywordListDirty;
	std::string mKeywordList;

	// indexed by keyword combo
	std::unordered_map<std::string, ShaderProgram> mPrograms;

	std::unordered_map<GLenum, ShaderSource> mShadersToLink;

	GLuint CompileShader(GLenum type, const ShaderSource& source, const std::vector<std::string>& keywords);
	ShaderProgram LinkShader(const std::vector<std::string>& keywords);
};