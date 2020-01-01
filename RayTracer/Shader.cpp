#define GLFW_INCLUDE_NONE

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <regex>	

#include "Shader.h"


const std::regex r("(#include <)([a-zA-Z]+)(\.slib>)");

void IncludePreprocess(std::string& code)
{
	std::smatch m;
	while (std::regex_search(code, m, r))
	{
		std::ifstream file;
		std::stringstream code_stream;
		file.open("./res/SHADERS/" + (std::string)m[2] + ".slib");
		if (file.is_open())
		{
			code_stream << file.rdbuf();
			file.close();
			std::regex file_reg("(#include <)" + (std::string)m[2] + "(\.slib>)");
			code = std::regex_replace(code, file_reg, code_stream.str());
		}
	}
}

Shader::Shader(const std::string& file, bool geo)
{
	// 1. retrieve the vertex/fragment source code from filePath
	std::string vertexCode;
	std::string fragmentCode;
	std::ifstream vShaderFile;
	std::ifstream fShaderFile;
	// ensure ifstream objects can throw exceptions:
	vShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
	fShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);

	std::string vertexPath   = "./res/SHADERS/" + file + ".vs";
	std::string fragmentPath = "./res/SHADERS/" + file + ".fs";
	try
	{
		// open 
		vShaderFile.open(vertexPath);
		fShaderFile.open(fragmentPath);
		std::stringstream vShaderStream, fShaderStream;
		// read file's buffer contents into streams
		vShaderStream << vShaderFile.rdbuf();
		fShaderStream << fShaderFile.rdbuf();
		// close file handlers
		vShaderFile.close();
		fShaderFile.close();
		// convert stream into string
		vertexCode = vShaderStream.str();
		fragmentCode = fShaderStream.str();

		IncludePreprocess(vertexCode);
		IncludePreprocess(fragmentCode);
	}
	catch (std::ifstream::failure e)
	{
		std::cout << "ERROR LOADING SHADER: " << file << "\n";
	}

	const char* vShaderCode = vertexCode.c_str();
	const char* fShaderCode = fragmentCode.c_str();

	int vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader, 1, &vShaderCode, NULL);
	glCompileShader(vertexShader);
	// check for shader compile errors
	int success;
	char infoLog[512];
	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
		std::cout << "ERROR LOADING VERT SHADER: " << file << " " << infoLog << "\n";
	}
	// fragment shader
	int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &fShaderCode, NULL);
	glCompileShader(fragmentShader);
	// check for shader compile errors
	glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
		std::cout << "ERROR LOADING FRAG SHADER: " << file << " " << infoLog << "\n";
	}

	// link shaders
	m_Program = glCreateProgram();
	glAttachShader(m_Program, vertexShader);
	glAttachShader(m_Program, fragmentShader);
	glLinkProgram(m_Program);
	// check for linking errors
	glGetProgramiv(m_Program, GL_LINK_STATUS, &success);
	if (!success) {
		glGetProgramInfoLog(m_Program, 512, NULL, infoLog);
		std::cout << "ERROR LINKING SHADER: " << file << " " << infoLog << "\n";
	}
	else
	{
		std::cout << "LOADED SHADER: " << file << "\n";
	}
	m_frag = fragmentShader;
	m_vert = vertexShader;
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);
}

Shader::~Shader()
{
	glDetachShader(m_Program, m_frag);
	glDetachShader(m_Program, m_vert);
	glDeleteProgram(m_Program);
}

void Shader::Bind()
{
	glUseProgram(m_Program);
}

GLint Shader::GetUniformLocation(const std::string& name) const
{
	auto itr = m_uniform_location_cache.find(name);
	if ( itr != m_uniform_location_cache.end())
		return itr->second;
	GLint location = glGetUniformLocation(m_Program, name.c_str());
	m_uniform_location_cache[name] = location;
	return location;
}

// utility uniform functions
// ------------------------------------------------------------------------
void Shader::setUniformBlock(const std::string& name, int loc) const
{
  auto index = glGetUniformBlockIndex(m_Program, name.c_str());
  if (index != GL_INVALID_INDEX)
  glUniformBlockBinding(m_Program, index, loc);
}

void Shader::setBool(const std::string &name, bool value) const
{
	glUniform1i(GetUniformLocation(name), (int)value);
}
// ------------------------------------------------------------------------
void Shader::setInt(const std::string &name, int value) const
{
	glUniform1i(GetUniformLocation(name), value);
}
// ------------------------------------------------------------------------
void Shader::setFloat(const std::string &name, float value) const
{
	glUniform1f(GetUniformLocation(name), value);
}
#if 1
// ------------------------------------------------------------------------
void Shader::setVec2(const std::string &name, const glm::vec2 &value) const
{
	glUniform2fv(GetUniformLocation(name), 1, &value[0]);
}
void Shader::setVec2(const std::string &name, float x, float y) const
{
	glUniform2f(GetUniformLocation(name), x, y);
}
// ------------------------------------------------------------------------
void Shader::setVec3(const std::string &name, const glm::vec3 &value) const
{
	glUniform3fv(GetUniformLocation(name), 1, &value[0]);
}
void Shader::setVec3(const std::string &name, float x, float y, float z) const
{
	glUniform3f(GetUniformLocation(name), x, y, z);
}
// ------------------------------------------------------------------------
void Shader::setVec4(const std::string &name, const glm::vec4 &value) const
{
	glUniform4fv(GetUniformLocation(name), 1, &value[0]);
}
void Shader::setVec4(const std::string &name, float x, float y, float z, float w)
{
	glUniform4f(GetUniformLocation(name), x, y, z, w);
}
// ------------------------------------------------------------------------
void Shader::setMat2(const std::string &name, const glm::mat2 &mat) const
{
	glUniformMatrix2fv(GetUniformLocation(name), 1, GL_FALSE, &mat[0][0]);
}
// ------------------------------------------------------------------------
void Shader::setMat3(const std::string &name, const glm::mat3 &mat) const
{
	glUniformMatrix3fv(GetUniformLocation(name), 1, GL_FALSE, &mat[0][0]);
}
// ------------------------------------------------------------------------
void Shader::setMat4(const std::string &name, const glm::mat4 &mat) const
{
	glUniformMatrix4fv(GetUniformLocation(name), 1, GL_FALSE, &mat[0][0]);
}
#endif
