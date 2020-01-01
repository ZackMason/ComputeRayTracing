#include <iostream>
#include <fstream>
#include <sstream>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "VertexArray.h"
#include "Shader.h"
#include "ComputeShader.h"

#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image.h"
#include "stb_image_write.h"

#define MAXFLOAT 1000.f

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow *window);

// settings
const unsigned int SCR_WIDTH  = 512 * 2;
const unsigned int SCR_HEIGHT = 512 * 2;

int main()
{
	// glfw: initialize and configure
	// ------------------------------
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 4);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// glfw window creation
	// --------------------
	GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "RayTracer", NULL, NULL);
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

	// glad: load all OpenGL function pointers
	// ---------------------------------------
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}

	// dimensions of the image
	int tex_w = SCR_WIDTH, tex_h = SCR_HEIGHT;
	GLuint ray_frame;
	glGenTextures(1, &ray_frame);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, ray_frame);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, tex_w, tex_h, 0, GL_RGBA, GL_FLOAT,
		NULL);
	glBindImageTexture(0, ray_frame, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);

	GLuint ray_o;
	glGenTextures(1, &ray_o);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, ray_o);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, tex_w, tex_h, 0, GL_RGBA, GL_FLOAT,
		NULL);
	glBindImageTexture(1, ray_o, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);

	GLuint ray_d;
	glGenTextures(1, &ray_d);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, ray_d);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, tex_w, tex_h, 0, GL_RGBA, GL_FLOAT,
		NULL);
	glBindImageTexture(2, ray_d, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);

	GLuint final_image;
	glGenTextures(1, &final_image);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, final_image);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, tex_w, tex_h, 0, GL_RGBA, GL_FLOAT,
		NULL);
	glBindImageTexture(3, final_image, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);


	double* blank_image = new double[tex_h*tex_w*4];
	memset(blank_image, 1., 4*tex_h*tex_w * sizeof(float));

	glBindTexture(GL_TEXTURE_2D, ray_frame);
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, tex_w, tex_h, GL_RGBA, GL_FLOAT, blank_image);

	ComputeShader *ray_program = new ComputeShader("ray");
	ComputeShader *final_image_program = new ComputeShader("final_image");

	VertexArray screen;
	Shader *screen_shader = new Shader("screen");
	screen_shader->Bind();
	screen_shader->setInt("uTexture1", 0);
	screen_shader->setInt("uTexture2", 1);

	int iter = 0;
	int max_iter = 10000;
	float window_size = 128.f;
	bool window_input_flag = true;
	bool iter_notification_flag = true;

	// render loop
	// -----------
	while (!glfwWindowShouldClose(window)) 
	{
		if (iter < max_iter)
		{
			iter++;

			if (iter % 500 == 0)
				std::cout << iter << " samples.\n";

			// compute loop
			// -----------
			for (int i = 0; i < 9; i++)
			{ 

				glUseProgram(ray_program->m_Program);
				glBindTexture(GL_TEXTURE_2D, ray_frame);

				auto l2 = glGetUniformLocation(ray_program->m_Program, "time");
				glUniform1f(l2, glfwGetTime());
				auto l3 = glGetUniformLocation(ray_program->m_Program, "depth");
				glUniform1i(l3, i);

				glDispatchCompute((GLuint)tex_w, (GLuint)tex_h, 1);
				glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
			}

			glUseProgram(final_image_program->m_Program);

			auto l1 = glGetUniformLocation(final_image_program->m_Program, "iterations");
			glUniform1i(l1, iter);

			glDispatchCompute((GLuint)tex_w, (GLuint)tex_h, 1);
			glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

			
		}
		else if (iter_notification_flag)
		{
			iter_notification_flag = false;
			std::cout << "Finished " << max_iter << " samples.\n";
		}

		// Draw Results
		// -----------
		{
			glClear(GL_COLOR_BUFFER_BIT);
			screen_shader->Bind();
			screen_shader->setFloat("window_size", window_size);
			screen.Bind();
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, final_image);
			glActiveTexture(GL_TEXTURE0 + 1);
			glBindTexture(GL_TEXTURE_2D, ray_o);
			screen.Draw();
		}

		// Input
		// -----------
		glfwPollEvents();
		
		if (GLFW_PRESS == glfwGetKey(window, GLFW_KEY_ESCAPE)) 
			glfwSetWindowShouldClose(window, 1);

		if (GLFW_PRESS == glfwGetKey(window, GLFW_KEY_W) && window_input_flag)
		{
			window_size = (window_size > 4.) ? 4.f : 128.f;
			window_input_flag = false;
		}
		else if (GLFW_RELEASE == glfwGetKey(window, GLFW_KEY_W))
			window_input_flag = true;

		if (GLFW_PRESS == glfwGetKey(window, GLFW_KEY_S))
		{
			double* d = new double[tex_h * tex_w * 4];
			glBindTexture(GL_TEXTURE_2D, final_image);
			glReadPixels(0, 0, tex_w, tex_h, GL_RGBA32F, GL_FLOAT, d);
			stbi_write_png("result.png", tex_w, tex_h, 4, d, tex_w * sizeof(double));
			stbi_write_jpg("result.jpg", tex_w, tex_h, 4, d, 100);
			delete d;
		}

		if (GLFW_PRESS == glfwGetKey(window, GLFW_KEY_F))
			iter = 0;
		if (GLFW_PRESS == glfwGetKey(window, GLFW_KEY_SPACE))
			iter = 2001;

		if (GLFW_PRESS == glfwGetKey(window, GLFW_KEY_R)) 
		{
			delete screen_shader;
			screen_shader = new Shader("screen");
			screen_shader->Bind();
			screen_shader->setInt("uTexture1", 0);
			screen_shader->setInt("uTexture2", 1);

			delete ray_program;
			delete final_image_program;

			ray_program = new ComputeShader("ray");
			final_image_program = new ComputeShader("final_image");

			iter = 0;
			iter_notification_flag = true;

			glBindTexture(GL_TEXTURE_2D, ray_d);
			glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, tex_w, tex_h, GL_RGBA, GL_FLOAT, blank_image);
			glBindTexture(GL_TEXTURE_2D, ray_o);
			glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, tex_w, tex_h, GL_RGBA, GL_FLOAT, blank_image);
			glBindTexture(GL_TEXTURE_2D, ray_frame);
			glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, tex_w, tex_h, GL_RGBA, GL_FLOAT, blank_image);
			glBindTexture(GL_TEXTURE_2D, final_image);
			glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, tex_w, tex_h, GL_RGBA, GL_FLOAT, blank_image);
		}
		glfwSwapBuffers(window);
	}
	glfwTerminate();
	return 0;
}

void processInput(GLFWwindow *window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}
