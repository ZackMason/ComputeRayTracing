#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>

#include <string>

class ComputeShader
{
public:
	ComputeShader();
	~ComputeShader();
	ComputeShader(const std::string& file);

	void SetTex();

	void RunTex(float dt);

	void InitSSBO(int size);

	void Bind(int count, 
		unsigned int pos,
		unsigned int vel,
		unsigned int stats, float dt);

	std::string LoadShader(const std::string& fileName);

	unsigned int m_PosTex;
	unsigned int m_VelTex;
	unsigned int m_StatsTex;

	int m_Program;
	int m_shader;
};

