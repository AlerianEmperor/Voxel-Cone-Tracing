#ifndef _VOXEL_CONE_TRACING_H_
#define _VOXEL_CONE_TRACING_H_

#include "Model.h"
#include <gl\glfw3.h>
#include <gl\freeglut.h>

Camera camera(glm::vec3(0.0f, 4.0f, 0.0f));


struct Voxel_Cone_Tracing
{
	//Global Properties
	vec3 lightDirection = vec3(0.0f, 1.0f, 0.25f);

	const int VoxelDimensions = 128;
	const float VoxelGridWorldSize = 150.0f;

	//Camera and Window
	//Camera* camera;

	GLFWwindow* window;

	int screen_width = 1280;
	int screen_height = 720;

	//Shader
	Shader VoxelizeShader;
	Shader ShadowShader;
	Shader VoxelConeTracingShader;

	//Shadow
	GLuint Depth_FBO;
	GLuint Depth_Texture;
	GLuint ShadowMapSize = 4096;

	//Voxel
	GLuint Texture3D_VAO;
	GLuint VoxelTexture;

	//Matrix;
	mat4 DepthViewProjectionMatrix;
	mat4 ProjX;
	mat4 ProjY;
	mat4 ProjZ;

	//Model
	Model model;

	//Display Properties
	bool ShowDiffuse = true, ShowIndirectDiffuse = true, ShowSpecular = true, ShowIndirectSpecular = true, ShowAmbientOcclusion = true;

	float AmbientFactor = 0.1f;

	Voxel_Cone_Tracing() {}

	Voxel_Cone_Tracing(int screen_width_, int screen_height_, GLFWwindow*& window_)// , Camera* camera_)
	{
		screen_width = screen_width_;
		screen_height = screen_height_;

		window = window_;

		//camera = camera_;
	}

	void init_voxel_cone_tracing()
	{
		ShadowMapSize = 4096;

		VoxelizeShader = Shader("Voxelization.vs", "Voxelization.fs", "Voxelization.gs");
		ShadowShader = Shader("Shadow.vs", "Shadow.fs");
		VoxelConeTracingShader = Shader("VoxelConeTracing.vs", "VoxelConeTracing.fs");

		//model = Model("E://Models//crytek_sponza//crytek_sponza.obj");

		model = Model("E://Voxel_Cone_Tracing//models//sponza.obj");

		//init Depth Texture

		glGenFramebuffers(1, &Depth_FBO);
		glBindFramebuffer(GL_FRAMEBUFFER, Depth_FBO);

		mat4 vMat = lookAt(lightDirection, vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 1.0f, 0.0f));
		mat4 pMat = ortho<float>(-120, 120, -120, 120, -100, 100);
		DepthViewProjectionMatrix = pMat * vMat;

		glGenTextures(1, &Depth_Texture);
		glBindTexture(GL_TEXTURE_2D, Depth_Texture);
		
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, Depth_Texture, 0);
		glDrawBuffer(GL_NONE);

		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		{
			cout << "Error Creating Depth FrameBuffer\n";
			return;
		}

		//Voxel intialize
		glEnable(GL_TEXTURE_3D);

		glGenTextures(1, &VoxelTexture);
		glBindTexture(GL_TEXTURE_3D, VoxelTexture);
		
		int numVoxels = VoxelDimensions * VoxelDimensions * VoxelDimensions;
		GLubyte* data = new GLubyte[4 * numVoxels];
		
		delete[] data;

		//missing
		//increase mipmap level at each sample step
		//SVO create this step perfectly with less space require
		glGenerateMipmap(GL_TEXTURE_3D);

		float size = VoxelGridWorldSize;

		mat4 voxelize_pMat = ortho<float>(-size * 0.5f, size * 0.5f, -size * 0.5f, size * 0.5f, size * 0.5f, size * 1.5f);

		ProjX = voxelize_pMat * lookAt(vec3(size, 0.0f, 0.0f), vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 1.0f, 0.0f));
		ProjY = voxelize_pMat * lookAt(vec3(0.0f, size, 0.0f), vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 0.0f, -1.0f));
		ProjZ = voxelize_pMat * lookAt(vec3(0.0f, 0.0f, size), vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 1.0f, 0.0f));

		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		DrawDepthTexture();
		DrawVoxelTexture();
	}

	//GLFWwindow* window;

	

	void Render()
	{
		//Draw Scene
		glfwGetFramebufferSize(window, &screen_width, &screen_height);
		glEnable(GL_CULL_FACE);
		glEnable(GL_DEPTH_TEST);

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glViewport(0, 0, screen_width, screen_height);

		if (AmbientFactor < 0.5f)
			glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
		else
			glClearColor(1.0f, 1.0f, 1.0f, 1.0f);

		mat4 vMat = camera.GetViewMatrix();
		mat4 pMat = perspective(radians(camera.Zoom), (float)screen_width / (float)screen_height, 0.1f, 1000.0f);
		vec3 camera_Position = camera.position;

		VoxelConeTracingShader.use();

		VoxelConeTracingShader.setVec3("CameraPosition", camera_Position);
		VoxelConeTracingShader.setVec3("LightDirection", lightDirection);
		VoxelConeTracingShader.setFloat("VoxelGridWorldSize", VoxelGridWorldSize);
		
		glActiveTexture(GL_TEXTURE0 + 5);
		glBindTexture(GL_TEXTURE_2D, Depth_Texture);
		VoxelConeTracingShader.setInt("ShadowMap", 5);

		glActiveTexture(GL_TEXTURE0 + 6);
		glBindTexture(GL_TEXTURE_3D, VoxelTexture);
		VoxelConeTracingShader.setInt("VoxelTexture", 6);

		//mat4 mMat = mat4(1.0f);
		mat4 mMat = glm::translate(glm::scale(glm::mat4(1.0f), glm::vec3(0.05f, 0.05f, 0.05f)), glm::vec3(0.0f, 0.0f, 0.0f));
		
		VoxelConeTracingShader.setMat4("DepthModelViewProjectionMatrix", DepthViewProjectionMatrix * mMat);

		model.Draw(VoxelConeTracingShader);
	}

	void DrawDepthTexture()
	{
		glEnable(GL_CULL_FACE);
		glEnable(GL_DEPTH_TEST);

		glBindFramebuffer(GL_FRAMEBUFFER, Depth_FBO);
		glViewport(0, 0, ShadowMapSize, ShadowMapSize);
		glClearColor(0, 0, 0, 1);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		ShadowShader.use();

		mat4 mMat = glm::translate(glm::scale(glm::mat4(1.0f), glm::vec3(0.05f, 0.05f, 0.05f)), glm::vec3(0.0f, 0.0f, 0.0f));
		ShadowShader.setMat4("DepthModelViewProjectionMatrix", DepthViewProjectionMatrix * mMat);

		model.Draw(ShadowShader);

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glViewport(0, 0, screen_width, screen_height);
	}

	void DrawVoxelTexture()
	{
		glDisable(GL_CULL_FACE);
		glDisable(GL_DEPTH_TEST);

		glViewport(0, 0, VoxelDimensions, VoxelDimensions);
		glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		VoxelizeShader.use();

		VoxelizeShader.setInt("VoxelDimensions", VoxelDimensions);

		VoxelizeShader.setMat4("ProjX", ProjX);
		VoxelizeShader.setMat4("ProjY", ProjY);
		VoxelizeShader.setMat4("ProjZ", ProjZ);
		
		VoxelizeShader.setInt("ShadowMap", 5);
	

		//change mMat to scale, rotate, translate
		//not use at this time
		//mat4 mMat = mat4(1.0f);
		mat4 mMat = glm::translate(glm::scale(glm::mat4(1.0f), glm::vec3(0.05f, 0.05f, 0.05f)), glm::vec3(0.0f, 0.0f, 0.0f));
		VoxelizeShader.setMat4("ModelMatrix", mMat);
		VoxelizeShader.setMat4("DepthModelViewProjectionMatrix", DepthViewProjectionMatrix * mMat);
		
		model.Draw(VoxelizeShader);
		
		glGenerateMipmap(GL_TEXTURE_3D);
		glViewport(0, 0, screen_width, screen_height);
	}

};

#endif // !_VOXEL_CONE_TRACING_H_

