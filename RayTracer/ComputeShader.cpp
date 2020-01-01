#include "ComputeShader.h"
#include <glad/glad.h>

#include <iostream>
#include <string>
#include <fstream>
#include <sstream>

ComputeShader::ComputeShader(const std::string& file)
{
	std::string code;
	std::ifstream shaderFile;

	shaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
	
	std::string path = "./res/SHADERS/" + file + ".cs";

	try
	{
		// open 
		shaderFile.open(path);
		std::stringstream shaderStream;
		// read file's buffer contents into streams
		shaderStream << shaderFile.rdbuf();
		// close file handlers
		shaderFile.close();
		// convert stream into string
		code = shaderStream.str();
	}
	catch (std::ifstream::failure e)
	{
		std::cout << "ERROR::SHADER::FILE_NOT_SUCCESFULLY_READ: " << file << "\n";
	}
	const char* shaderCode = code.c_str();

	int shader = glCreateShader(GL_COMPUTE_SHADER);
	glShaderSource(shader, 1, &shaderCode, NULL);
	glCompileShader(shader);
	// check for shader compile errors
	int success;
	char infoLog[512];
	glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(shader, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED: " << file << " " << infoLog << "\n";
	}
	m_Program = glCreateProgram();
	glAttachShader(m_Program, shader);
	glLinkProgram(m_Program);
	glGetProgramiv(m_Program, GL_LINK_STATUS, &success);
	if (!success) {
		glGetProgramInfoLog(m_Program, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED: " << file << " " << infoLog << "\n";
	}
	else
	{
		std::cout << "LOADED SHADER: " << file << "\n";
	}
	m_shader = shader;

	glDeleteShader(shader);
}

void ComputeShader::SetTex()
{
	glGenTextures(1, &m_PosTex);
	glGenTextures(1, &m_VelTex);
	glGenTextures(1, &m_StatsTex);
	glBindTexture(GL_TEXTURE_2D, m_PosTex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, 1024, 1024, 0, GL_RGBA, GL_FLOAT, NULL);
	glBindImageTexture(0, m_PosTex, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);

	glBindTexture(GL_TEXTURE_2D, m_VelTex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, 1024, 1024, 0, GL_RGBA, GL_FLOAT, NULL);
	glBindImageTexture(1, m_VelTex, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);

	glBindTexture(GL_TEXTURE_2D, m_StatsTex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, 1024, 1024, 0, GL_RGBA, GL_FLOAT, NULL);
	glBindImageTexture(2, m_StatsTex, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);

}

void ComputeShader::RunTex(float dt)
{
	glUseProgram(m_Program);

	glBindImageTexture(0, m_PosTex, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);
	glBindImageTexture(1, m_VelTex, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);
	glBindImageTexture(2, m_StatsTex, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);
	glUniform1i(glGetUniformLocation(m_Program, "Pos"), 0);
	glUniform1i(glGetUniformLocation(m_Program, "Vel"), 1);
	glUniform1i(glGetUniformLocation(m_Program, "Life"), 2);
	glDispatchCompute(1024, 1024, 1);
	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
}

ComputeShader::~ComputeShader()
{
	glDetachShader(m_Program, m_shader);
	glDeleteProgram(m_Program);
}

void ComputeShader::Bind(int count, 
	unsigned int pos,
	unsigned int vel,
	unsigned int stats,
	float dt)
{

	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, pos);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, vel);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, stats);
	glUseProgram(m_Program);
	glUniform1f(glGetUniformLocation(m_Program, "dt"), dt);
	//glUniform1i(glGetUniformLocation(m_Program, "Pos"), 0);
	//glUniform1i(glGetUniformLocation(m_Program, "Vel"), 1);
	//glUniform1i(glGetUniformLocation(m_Program, "Life"), 2);
	glDispatchCompute(count, 1, 1);
	//glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
	glMemoryBarrier(GL_ALL_BARRIER_BITS);
}
