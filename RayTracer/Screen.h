#pragma once

#include "Shader.h"
#include "VertexArray.h"

class Screen
{
public:
	Screen();
	~Screen();

	void Draw(unsigned int tex);

	VertexArray m_ScreenQuad;
};

