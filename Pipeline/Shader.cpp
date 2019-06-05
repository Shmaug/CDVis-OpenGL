#include "Shader.hpp"

#include <fstream>
#include <sstream>
#include <cassert>
#include <vector>
#include <algorithm>

#ifdef WINDOWS
#include <Windows.h>
#undef max
#endif

using namespace std;
using namespace glm;

Shader::Shader() {}
Shader::~Shader() {
	for (const auto& p : mPrograms) {
		glDeleteProgram(p.second.mProgram);
		for (const auto& s : p.second.mShaders)
			glDeleteShader(s);
	}
}

void Shader::Uniform(GLuint program, const GLchar* name, int x) {
	glUniform1i(glGetUniformLocation(program, name), x);
}
void Shader::Uniform(GLuint program, const GLchar* name, float x){
	glUniform1f(glGetUniformLocation(program, name), x);
}
void Shader::Uniform(GLuint program, const GLchar* name, const vec2& v){
	glUniform2f(glGetUniformLocation(program, name), v.x, v.y);
}
void Shader::Uniform(GLuint program, const GLchar* name, const vec3& v) {
	glUniform3f(glGetUniformLocation(program, name), v.x, v.y, v.z);
}
void Shader::Uniform(GLuint program, const GLchar* name, const vec4& v) {
	glUniform4f(glGetUniformLocation(program, name), v.x, v.y, v.z, v.w);
}
void Shader::Uniform(GLuint program, const GLchar* name, const mat4& m){
	glUniformMatrix4fv(glGetUniformLocation(program, name), 1, GL_FALSE, &m[0][0]);
}

void Shader::EnableKeyword(string kw) {
	if (mAvailableKeywords.count(kw))
		mActiveKeywords.insert(kw);
}
void Shader::DisableKeyword(string kw) {
	mActiveKeywords.erase(kw);
}

GLuint Shader::Use() {
	string kw = "";
	for (const auto& k : mActiveKeywords)
		kw += k + " ";
	assert(mPrograms.count(kw));

	GLuint p = mPrograms.at(kw).mProgram;
	glUseProgram(p);
	return p;
}

void Shader::AddShaderFile(GLenum type, string filename) {
	ifstream file(filename);
	if (!file) return;

	stringstream sstr;
	sstr << file.rdbuf();

	file.close();

	ShaderSource src;
	src.mFile = filename;
	src.mSource = sstr.str();

	mShadersToLink.emplace(type, src);
}
void Shader::AddShaderSource(GLenum type, string source) {
	ShaderSource src;
	src.mFile = "";
	src.mSource = source;
	mShadersToLink.emplace(type, src);
}

GLuint Shader::CompileShader(GLenum type, const ShaderSource& source, const vector<string>& keywords) {
	stringstream file(source.mSource);

	stringstream sstr;

	// read the first line, check if it's a #version line
	// if so, keep it at the top, if not, insert after the #define statements
	bool insertLine = false;
	string firstLine;
	if (getline(file, firstLine)) {
		if (firstLine.substr(0, 8) == "#version")
			sstr << firstLine << endl;
		else
			insertLine = true;
	}

	for (const auto& kw : keywords)
		sstr << "#define " << kw.c_str() << endl;

	if (insertLine) sstr << firstLine << endl;
	sstr << file.rdbuf();

	string str = sstr.str();
	const char* src = str.c_str();

	GLuint shader = glCreateShader(type);
	glShaderSource(shader, 1, &src, 0);
	glCompileShader(shader);

	GLint isCompiled = 0;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &isCompiled);
	if (isCompiled == GL_FALSE) {
		GLint maxLength = 0;
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &maxLength);

		vector<GLchar> info(maxLength);
		glGetShaderInfoLog(shader, maxLength, &maxLength, info.data());

		string kw = "";
		for (const auto& it : keywords) kw += it + " ";
		printf("Error compiling %s with keywords %s: ", source.mFile.c_str(), kw.c_str());

		// red error text
		#ifdef WINDOWS
		HANDLE console = GetStdHandle(STD_OUTPUT_HANDLE);
		SetConsoleTextAttribute(console, FOREGROUND_RED);
		#endif

		printf("%s\n", info.data());

		#ifdef WINDOWS
		SetConsoleTextAttribute(console, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
		#endif

		glDeleteShader(shader);

		assert(false);
		return 0;
	} else {
		return shader;
	}
}
Shader::ShaderProgram Shader::LinkShader(const vector<string>& keywords) {
	ShaderProgram pgm;

	// compile shaders with keywords
	for (const auto& it : mShadersToLink)
		pgm.mShaders.push_back(CompileShader(it.first, it.second, keywords));

	pgm.mProgram = glCreateProgram();
	for (const auto& s : pgm.mShaders)
		glAttachShader(pgm.mProgram, s);
	glLinkProgram(pgm.mProgram);

	GLint isLinked = 0;
	glGetProgramiv(pgm.mProgram, GL_LINK_STATUS, (int*)&isLinked);
	if (isLinked == GL_FALSE) {
		GLint maxLength = 0;
		glGetProgramiv(pgm.mProgram, GL_INFO_LOG_LENGTH, &maxLength);

		std::vector<GLchar> info(maxLength);
		glGetProgramInfoLog(pgm.mProgram, maxLength, &maxLength, info.data());

		printf("Error linking shader program: ");

		// red error text
		#ifdef WINDOWS
		HANDLE console = GetStdHandle(STD_OUTPUT_HANDLE);
		SetConsoleTextAttribute(console, FOREGROUND_RED);
		#endif

		printf("%s\n", info.data());

		#ifdef WINDOWS
		SetConsoleTextAttribute(console, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
		#endif

		glDeleteProgram(pgm.mProgram);
		for (const auto& s : pgm.mShaders)
			glDeleteShader(s);

		assert(false);
	} else {
		for (const auto& s : pgm.mShaders)
			glDetachShader(pgm.mProgram, s);
	}

	return pgm;
}

void Shader::CompileAndLink() {
	vector<vector<string>> keywords;
	keywords.push_back(vector<string>());
	keywords[0].push_back("");

	// create keyword variants
	for (const auto& it : mShadersToLink) {
		stringstream file(it.second.mSource);
		// scan for keywords
		string line;
		while (getline(file, line)) {
			unsigned int kwc = keywords.size();
			stringstream ss(line);
			string token;
			int mode = 0;
			while (getline(ss, token, ' ')) {
				if (mode == 2) {
					// create variants
					for (unsigned int i = 0; i < kwc; i++) {
						vector<string> k(keywords[i]);
						k.push_back(token);
						keywords.push_back(k);
					}
					mAvailableKeywords.insert(token);
				} else {
					if (token == "#pragma")
						mode = 1;
					else if (mode == 1 && token == "multi_compile")
						mode = 2;
				}
			}
		}
	}

	for (const auto& it : mShadersToLink)
		printf("%s ", it.second.mFile.c_str());
	printf(": Compiling %d shader variants\n", (int)keywords.size());

	for (auto& it : keywords) {
		it.erase(it.begin());
		string kw = "";
		for (const auto& k : it)
			kw += k + " ";
		mPrograms.emplace(kw, LinkShader(it));
	}
}