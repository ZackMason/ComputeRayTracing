#pragma once
class VertexArray
{
public:
	VertexArray();
	~VertexArray();

	operator unsigned int()
	{
		return m_VAO;
	}

	void Bind();
	void Unbind();
	void Draw();

	int m_Size;
	unsigned int m_VAO=0;
};

