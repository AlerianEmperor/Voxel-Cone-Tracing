#pragma once
#include "Model.h"
//#include "Camera.h"
#include <gl\glfw3.h>
#include <iostream>
#include <vector>
#include <map>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
//#include "glad.h"
#include <gl\freeglut.h>

class Renderer {
public:
	//settings
	const float sponzaScale_ = 0.05f;
	glm::vec3 lightDirection_ = glm::vec3(0, 1, 0.25);
	const int voxelDimensions_ = 128;
	const float voxelGridWorldSize_ = 150.0f;

	int SCR_WIDTH, SCR_HEIGHT;
	unsigned int shadowMapRes;
	Camera* camera;
	GLFWwindow* window;

	Shader* standardShader;
	Shader* shadowShader;
	Shader* voxelizationShader;
	GLuint depthFramebuffer;
	GLuint depthTexture;
	GLuint texture3DVertexArray;
	GLuint voxelTexture;
	glm::mat4 depthViewProjectionMatrix;
	glm::mat4 projX, projY, projZ;
	Model* model;

	bool press1_ = false, press2_ = false, press3_ = false, press4_ = false, press5_ = false;
	bool showDiffuse_ = true, showIndirectDiffuse_ = true, showDirectSpecular_ = true, showIndirectSpecular_ = true, showAmbientOcclusion = true;
	float ambientFactor = 0.1;
	Renderer(const int width, const int height, GLFWwindow* aWindow)
	{
		SCR_WIDTH = width;
		SCR_HEIGHT = height;
		window = aWindow;
		camera = NULL;
	}
	~Renderer()
	{
		delete model;
	}

	bool initialize(Camera * aCamera, unsigned int aShadowMapRes)
	{
		std::cout << "Initializing VCT" << std::endl;

		std::cout << "Loading UI ..." << std::endl;
		/*IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO(); (void)io;
		ImGui_ImplGlfw_InitForOpenGL(window, true);
		ImGui::StyleColorsDark();
		ImGui_ImplOpenGL3_Init("#version 450");*/

		camera = aCamera;
		
		shadowMapRes = aShadowMapRes;

		string s = "E://a_a_a_a_a_a_Voxel_Cone_Tracing//build//Voxel_Cone_Tracing//";

		standardShader = new Shader((s + "standard.vert").c_str(), (s + "standard.frag").c_str());
		voxelizationShader = new Shader((s + "voxelization.vert").c_str(), (s + "voxelization.frag").c_str(), (s + "voxelization.geom").c_str());
		shadowShader = new Shader((s + "shadow.vert").c_str(), (s + "shadow.frag").c_str());

		string s2 = "E://a_a_a_a_a_a_Voxel_Cone_Tracing//build//";
																						/* 加载模型 */
		std::cout << "Loading objects... " << std::endl;
		model = new Model(s2 + "models//sponza.obj");
		std::cout << "Loading done! " << std::endl;
		
		glGenVertexArrays(1, &texture3DVertexArray);
		
		glGenFramebuffers(1, &depthFramebuffer);
		glBindFramebuffer(GL_FRAMEBUFFER, depthFramebuffer);

		glm::mat4 viewMatrix = glm::lookAt(lightDirection_, glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
		glm::mat4 projectionMatrix = glm::ortho	<float>(-120, 120, -120, 120, -100, 100);
		depthViewProjectionMatrix = projectionMatrix * viewMatrix;

		glGenTextures(1, &depthTexture);
		glBindTexture(GL_TEXTURE_2D, depthTexture);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, shadowMapRes,
			shadowMapRes, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);	
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);	
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		
		glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depthTexture, 0);
		glDrawBuffer(GL_NONE);	

		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
			std::cout << "Error creating framebuffer" << std::endl;
			return false;
		}

		glEnable(GL_TEXTURE_3D);
								
		glGenTextures(1, &voxelTexture);
		glBindTexture(GL_TEXTURE_3D, voxelTexture);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		int numVoxels = voxelDimensions_ * voxelDimensions_ * voxelDimensions_;
		GLubyte* data = new GLubyte[numVoxels * 4];
		memset(data, 0, numVoxels * 4);
		
		glTexImage3D(GL_TEXTURE_3D, 0, GL_RGBA8, voxelDimensions_, voxelDimensions_, voxelDimensions_, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);

		delete[] data;	


		glGenerateMipmap(GL_TEXTURE_3D);

		float size = voxelGridWorldSize_;	
		projectionMatrix = glm::ortho(-size * 0.5f, size * 0.5f, -size * 0.5f, size * 0.5f, size * 0.5f, size * 1.5f);
		projX = projectionMatrix * glm::lookAt(glm::vec3(size, 0, 0), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
		projY = projectionMatrix * glm::lookAt(glm::vec3(0, size, 0), glm::vec3(0, 0, 0), glm::vec3(0, 0, -1));
		projZ = projectionMatrix * glm::lookAt(glm::vec3(0, 0, size), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
											
		drawDepthTexture();
		drawVoxelTexture();

		return true;
	}
	/*void updateUI(float deltaTime)
	{
		bool Imgui = true;
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
		
		ImGui::Begin("INFORMATION");
		ImGui::Text("FPS :  %.0fHz ", floor(1 / deltaTime));
		ImGui::Text("Light Direction:\n(%.1fdegrees,%.1fdegrees)", glm::degrees(glm::atan(lightDirection_.x)), glm::degrees(glm::atan(lightDirection_.z)));
		ImGui::Text("Camera Position:\n(%.1f, %.1f, %.1f)", camera->Position.x, camera->Position.y, camera->Position.z);
		float theta_y = glm::asin(camera->Front.y);
		float theta_x = glm::acos(camera->Front.z / glm::cos(theta_y));
		ImGui::Text("Camera Direction:\n(%.1fdegrees,%.1fdegrees)", camera->Front.x > 0.f ? 180 - glm::degrees(theta_x) : glm::degrees(theta_x) - 180, glm::degrees(theta_y));
		ImGui::End();

		ImGui::Begin("USE GUIDE");
		ImGui::Text("W/A/S/D/Q/E for move camera front,left,back,right,down,up;");
		ImGui::Text("1~5 for these toggle buttons in order;");
		ImGui::Text("UP/DOWN/LEFT/RIGHT for change light direction;");
		ImGui::Text("SPACE for switch UI/mouse mode;");
		ImGui::Text("ESCAPE for exit");
		ImGui::End();

		ImGui::Begin("SETTINGS");
		
		ImGui::Text("Direct Light:");

		if (ImGui::Button("1:DIFFUSE"))
			showDiffuse_ = !showDiffuse_;
		ImGui::SameLine();
		ImGui::Text(showDiffuse_ ? "on" : "off");
		ImGui::SameLine();
		if (ImGui::Button("3:SPECULAR"))
			showDirectSpecular_ = !showDirectSpecular_;
		ImGui::SameLine();
		ImGui::Text(showDirectSpecular_ ? "on" : "off");
		
		ImGui::Text("Indirect Light:");

		if (ImGui::Button("2:IN DIFFUSE"))
			showIndirectDiffuse_ = !showIndirectDiffuse_;
		ImGui::SameLine();
		ImGui::Text(showIndirectDiffuse_ ? "on" : "off");
		ImGui::SameLine();
		if (ImGui::Button("4:IN SPECULAR"))
			showIndirectSpecular_ = !showIndirectSpecular_;
		ImGui::SameLine();
		ImGui::Text(showIndirectSpecular_ ? "on" : "off");

		ImGui::Text("Ambient Light:");
		if (ImGui::Button("5:OCCLUSION"))
			showAmbientOcclusion = !showAmbientOcclusion;
		ImGui::SameLine();
		ImGui::Text(showAmbientOcclusion ? "on" : "off");
		ImGui::SliderFloat("Factor", &ambientFactor, 0.f, 1.f, "%.2f");

		ImGui::End();

		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
	}*/
	void updateKeyPress()
	{
		//update the light component toggles
		if (!press1_ && glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS) {
			showDiffuse_ = !showDiffuse_;
			press1_ = true;
		}
		if (!press2_ && glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS) {
			showIndirectDiffuse_ = !showIndirectDiffuse_;
			press2_ = true;
		}
		if (!press3_ && glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS) {
			showDirectSpecular_ = !showDirectSpecular_;
			press3_ = true;
		}
		if (!press4_ && glfwGetKey(window, GLFW_KEY_4) == GLFW_PRESS) {
			showIndirectSpecular_ = !showIndirectSpecular_;
			press4_ = true;
		}
		if (!press5_ && glfwGetKey(window, GLFW_KEY_5) == GLFW_PRESS) {
			showAmbientOcclusion = !showAmbientOcclusion;
			press5_ = true;
		}
		if (glfwGetKey(window, GLFW_KEY_1) == GLFW_RELEASE) {
			press1_ = false;
		}
		if (glfwGetKey(window, GLFW_KEY_2) == GLFW_RELEASE) {
			press2_ = false;
		}
		if (glfwGetKey(window, GLFW_KEY_3) == GLFW_RELEASE) {
			press3_ = false;
		}
		if (glfwGetKey(window, GLFW_KEY_4) == GLFW_RELEASE) {
			press4_ = false;
		}
		if (glfwGetKey(window, GLFW_KEY_5) == GLFW_RELEASE) {
			press5_ = false;
		}

		bool changeLightDir = false;
		if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) {
			lightDirection_.x += 0.01;
			changeLightDir = true;
		}
		if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) {
			lightDirection_.x -= 0.01;
			changeLightDir = true;
		}
		if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) {
			lightDirection_.z -= 0.01;
			changeLightDir = true;
		}
		if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) {
			lightDirection_.z += 0.01;
			changeLightDir = true;
		}
		if (changeLightDir)
		{
			depthViewProjectionMatrix = glm::ortho	<float>(-120, 120, -120, 120, -100, 100) * glm::lookAt(lightDirection_, glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));;
			drawDepthTexture();
			drawVoxelTexture();
		}
	}
	void update(float deltaTime)
	{
		updateKeyPress();
		// ------------------------------------------------------------------- // 
		// --------------------- Draw the scene normally --------------------- //
		// ------------------------------------------------------------------- //
		glfwGetFramebufferSize(window, &SCR_WIDTH, &SCR_HEIGHT);
		glEnable(GL_CULL_FACE);
		glEnable(GL_DEPTH_TEST);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
		
		if (ambientFactor < 0.5)
			glClearColor(0.5, 0.5, 0.5, 1);
		else
			glClearColor(1, 1, 1, 1);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glm::mat4 viewMatrix = camera->GetViewMatrix();
		glm::mat4 projectionMatrix = glm::perspective(glm::radians(camera->Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 1000.0f);
		glm::vec3 camPos = camera->Position;
		standardShader->use();
		standardShader->setVec3("CameraPosition", glm::vec3(camPos.x, camPos.y, camPos.z));
		standardShader->setVec3("LightDirection", glm::vec3(lightDirection_.x, lightDirection_.y, lightDirection_.z));
		standardShader->setFloat("VoxelGridWorldSize", voxelGridWorldSize_);
		standardShader->setInt("VoxelDimensions", voxelDimensions_);
		standardShader->setFloat("ShowDiffuse", showDiffuse_);
		standardShader->setFloat("ShowIndirectDiffuse", showIndirectDiffuse_);
		standardShader->setFloat("ShowDirectSpecular", showDirectSpecular_);
		standardShader->setFloat("ShowIndirectSpecular", showIndirectSpecular_);
		standardShader->setFloat("AmbientOcclusion", showAmbientOcclusion);
		standardShader->setFloat("ambientFactor", ambientFactor);
		standardShader->setInt("shadowMapRes", shadowMapRes);

		glActiveTexture(GL_TEXTURE0 + 5);
		glBindTexture(GL_TEXTURE_2D, depthTexture);
		standardShader->setInt("ShadowMap", 5);

		glActiveTexture(GL_TEXTURE0 + 6);
		glBindTexture(GL_TEXTURE_3D, voxelTexture);
		standardShader->setInt("VoxelTexture", 6);

		glm::mat4 ModelMatrix = glm::translate(glm::scale(glm::mat4(1.0f), glm::vec3(0.05f, 0.05f, 0.05f)), glm::vec3(0.0f, 0.0f, 0.0f));
		standardShader->setMat4("ViewMatrix", viewMatrix);
		standardShader->setMat4("ModelMatrix", ModelMatrix);
		standardShader->setMat4("ModelViewMatrix", viewMatrix * ModelMatrix);
		standardShader->setMat4("ProjectionMatrix", projectionMatrix);
		standardShader->setMat4("DepthModelViewProjectionMatrix", depthViewProjectionMatrix * ModelMatrix);
		model->Draw(*standardShader);

		//updateUI(deltaTime);
	}

protected:

	void drawDepthTexture()
	{
		glEnable(GL_CULL_FACE);
		glEnable(GL_DEPTH_TEST);

		glBindFramebuffer(GL_FRAMEBUFFER, depthFramebuffer);
		glViewport(0, 0, shadowMapRes, shadowMapRes);
		glClearColor(0, 0, 0, 1);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);	
															
		shadowShader->use();
		glm::mat4 modelMatrix = glm::translate(glm::scale(glm::mat4(1.0f), glm::vec3(0.05f, 0.05f, 0.05f)), glm::vec3(0.0f, 0.0f, 0.0f));
		shadowShader->setMat4("ModelViewProjectionMatrix", depthViewProjectionMatrix * modelMatrix);
		model->Draw(*shadowShader);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);	
		glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);		
	}
	void drawVoxelTexture()
	{
		glDisable(GL_CULL_FACE);
		glDisable(GL_DEPTH_TEST);

		glViewport(0, 0, voxelDimensions_, voxelDimensions_);
		glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		voxelizationShader->use();
		
		voxelizationShader->setInt("VoxelDimensions", voxelDimensions_);
		
		voxelizationShader->setMat4("ProjX", projX);
		voxelizationShader->setMat4("ProjY", projY);
		voxelizationShader->setMat4("ProjZ", projZ);

		glActiveTexture(GL_TEXTURE0 + 5);
		glBindTexture(GL_TEXTURE_2D, depthTexture);
		voxelizationShader->setInt("ShadowMap", 5);

		glBindImageTexture(6, voxelTexture, 0, GL_TRUE, 0, GL_WRITE_ONLY, GL_RGBA8);
		voxelizationShader->setInt("VoxelTexture", 6);

		glm::mat4 modelMatrix = glm::translate(glm::scale(glm::mat4(1.0f), glm::vec3(0.05f, 0.05f, 0.05f)), glm::vec3(0.0f, 0.0f, 0.0f));
		voxelizationShader->setMat4("ModelMatrix", modelMatrix);
		voxelizationShader->setMat4("DepthModelViewProjectionMatrix", depthViewProjectionMatrix * modelMatrix);
		voxelizationShader->setInt("shadowMapRes", shadowMapRes);
		model->Draw(*voxelizationShader);
		glActiveTexture(GL_TEXTURE6);
		glBindTexture(GL_TEXTURE_3D, voxelTexture);
		glGenerateMipmap(GL_TEXTURE_3D);
		glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
	}


};
