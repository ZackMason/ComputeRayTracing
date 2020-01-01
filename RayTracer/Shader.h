#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>

#include <unordered_map>
#include <string>
#include <functional>

class Shader
{
public:
	Shader() = delete;
	~Shader();
	Shader(const std::string& file, bool geo = false);
  //	Shader(const std::string& file,std::function<void(void)> unifn);

	operator int() { return m_Program; }

	std::string LoadShader(const std::string& fileName);

	void Bind();
	GLint GetUniformLocation(const std::string& name) const;

	mutable std::unordered_map<std::string, GLint> m_uniform_location_cache;

  //	std::function<void(void)> m_BindUniFunctor;
	
	int m_Program;
	int m_frag;
	int m_vert;

	// utility uniform functions
    // --------------------------------------------------------------------------

    void setUniformBlock(const std::string &name, int loc) const;
	void setBool  (const std::string &name, bool  value) const					;
	void setInt   (const std::string &name, int   value) const				        ;
	void setFloat (const std::string &name, float value) const					;
	void setVec2  (const std::string &name, float x, float y) const				;
	void setVec3  (const std::string &name, float x, float y, float z) const	;
	void setVec4  (const std::string &name, float x, float y, float z, float w)	;
	void setVec2  (const std::string &name, const glm::vec2 &value) const		;
	void setVec3  (const std::string &name, const glm::vec3 &value) const		;
	void setVec4  (const std::string &name, const glm::vec4 &value) const		;
	void setMat2  (const std::string &name, const glm::mat2 &mat) const			;
	void setMat3  (const std::string &name, const glm::mat3 &mat) const			;
	void setMat4  (const std::string &name, const glm::mat4 &mat) const			;
	// --------------------------------------------------------------------------
};

