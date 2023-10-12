#ifndef _MESH_H_
#define _MESH_H_

#include "Camera.h"
#include <string>
#include <vector>
#include <glm\glm.hpp>

using namespace glm;
using namespace std;

struct Vertex
{
	vec3 Position;
	vec3 Normal;
	vec2 TexCoords;
	vec3 Tangents;
	vec3 Bi_Tangents;
};

struct Texture
{
	unsigned int id;
	string type;
	string path;
	int width;
	int height;
};

struct Mesh
{
	vector<Vertex> vertices;
	vector<unsigned int> indices;
	vector<Texture> textures;

	unsigned int VAO;
	unsigned int VBO;
	unsigned int IBO;

	Mesh() {}
	Mesh(vector<Vertex> vertices_, vector<unsigned int> indices_, vector<Texture> textures_)
	{
		vertices = vertices_;
		indices = indices_;
		textures = textures_;
		setup_Mesh();
	}

	void setup_Mesh()
	{
		glGenVertexArrays(1, &VAO);
		glGenBuffers(1, &VBO);
		glGenBuffers(1, &IBO);

		//VAO
		glBindVertexArray(VAO);

		//VBO
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);

		//IBO
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);

		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Normal));

		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, TexCoords));

		glEnableVertexAttribArray(3);
		glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Tangents));

		glEnableVertexAttribArray(4);
		glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Bi_Tangents));

		glBindVertexArray(0);
	}

	void Draw_Mesh(Shader& shader)
	{
		glUniform1f(glGetUniformLocation(shader.id, "Shininess"), 20.0f);
		glUniform1f(glGetUniformLocation(shader.id, "Opacity"), 1.0f);

		for (unsigned int i = 0; i < textures.size(); ++i)
		{
			glActiveTexture(GL_TEXTURE0 + i);

			string name = textures[i].type;
			
			if (name == "texture_diffuse")
			{
				glUniform1i(glGetUniformLocation(shader.id, "DiffuseTexture"), i);
				glUniform2f(glGetUniformLocation(shader.id, "DiffuseTextureSize"), textures[i].width, textures[i].height);
			}
			else if (name == "texture_specular")
			{
				glUniform1i(glGetUniformLocation(shader.id, "SpecularTexture"), i);
				glUniform2f(glGetUniformLocation(shader.id, "SpecularTextureSize"), textures[i].width, textures[i].height);
			}
			else if (name == "texture_height")
			{
				glUniform1i(glGetUniformLocation(shader.id, "HeightTexture"), i);
				glUniform2f(glGetUniformLocation(shader.id, "HeightTextureSize"), textures[i].width, textures[i].height);
			}
			glBindTexture(GL_TEXTURE_2D, textures[i].id);
		}

		//Draw Mesh
		glBindVertexArray(VAO);
		glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);

		glActiveTexture(GL_TEXTURE0);
	}
};


#endif // !_MES_H_

