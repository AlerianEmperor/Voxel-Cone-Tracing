#ifndef _MODEL_H_
#define _MODEL_H_

#define STB_IMAGE_IMPLEMENTATION

#include "stb_image.h"

#include "Mesh.h"

#include <assimp\Importer.hpp>
#include <assimp\scene.h>
#include <assimp\postprocess.h>

#include <string>
#include <fstream>
#include <sstream>

using namespace Assimp;

struct Model
{
	vector<Texture> textures_loaded;
	vector<Mesh> meshes;

	string direction;

	Model() {}
	Model(const string& file_path)
	{
		LoadModel(file_path);
	}

	void Draw(Shader& shader)
	{
		for (unsigned int i = 0; i < meshes.size(); ++i)
			meshes[i].Draw_Mesh(shader);
	}

	void LoadModel(const string& filepath)
	{
		Assimp::Importer import;

		const aiScene* scn = import.ReadFile(filepath, aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_FlipUVs | aiProcess_CalcTangentSpace);

		if (!scn || scn->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scn->mRootNode)
		{
			cout << "Assimp Error Loading : " << import.GetErrorString() << "\n";
			
			//getchar();
			return;
		}

		direction = filepath.substr(0, filepath.find_last_of('/'));

		ProcessNode(scn->mRootNode, scn);

		cout << "Num Meshes: "<<meshes.size() << "\n";
		cout << "Num Textures: " << textures_loaded.size() << "\n";

		cout << "Load Object Successful!\n";
	}

	void ProcessNode(aiNode* node, const aiScene* scn)
	{
		for (unsigned int i = 0; i < node->mNumMeshes; ++i)
		{
			aiMesh* mesh = scn->mMeshes[node->mMeshes[i]];
			meshes.push_back(ProcessMesh(mesh, scn));
		}

		for (unsigned int i = 0; i < node->mNumChildren; ++i)
			ProcessNode(node->mChildren[i], scn);
	}

	Mesh ProcessMesh(aiMesh* mesh, const aiScene* scn)
	{
		vector<Vertex> vertices;
		vector<unsigned int> indices;
		vector<Texture> textures;

		for (unsigned int i = 0; i < mesh->mNumVertices; ++i)
		{
			Vertex vertex;

			//vec3 position(mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z);

			//vertex.Position = position;

			vertex.Position = vec3(mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z);

			if (mesh->HasNormals())
			{
				//vec3 normal(mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z);
				//vertex.Normal = normal;

				vertex.Normal = vec3(mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z);
			}

			if (mesh->mTextureCoords[0])
			{
				vec2 texcoord(mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y);
				vertex.TexCoords = texcoord;

				vec3 tangent(mesh->mTangents[i].x, mesh->mTangents[i].y, mesh->mTangents[i].z);
				vertex.Tangents = tangent;

				vec3 bi_tangent(mesh->mBitangents[i].x, mesh->mBitangents[i].y, mesh->mBitangents[i].z);
				vertex.Bi_Tangents = bi_tangent;
			}
			else
				vertex.TexCoords = vec2(0.0f, 0.0f);

			vertices.push_back(vertex);
		}

		for (unsigned int i = 0; i < mesh->mNumFaces; ++i)
		{
			aiFace face = mesh->mFaces[i];

			for (unsigned int j = 0; j < face.mNumIndices; ++j)
				indices.push_back(face.mIndices[j]);
		}

		aiMaterial* material = scn->mMaterials[mesh->mMaterialIndex];

		vector<Texture> diffuseMaps = LoadTexture(material, aiTextureType_DIFFUSE, "texture_diffuse");
		textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());

		vector<Texture> specularMaps = LoadTexture(material, aiTextureType_SPECULAR, "texture_specular");
		textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());

		vector<Texture> normalMaps = LoadTexture(material, aiTextureType_HEIGHT, "texture_normal");
		textures.insert(textures.end(), normalMaps.begin(), normalMaps.end());

		vector<Texture> heightMaps = LoadTexture(material, aiTextureType_AMBIENT, "texture_height");
		textures.insert(textures.end(), heightMaps.begin(), heightMaps.end());

		return Mesh(vertices, indices, textures);
	}

	unsigned int TextureFromFile(const char* path, string& file_direction, int& width, int& height)
	{
		string file_name = string(path);

		file_name = file_direction + '/' + file_name;

		unsigned int textureID;
		glGenTextures(1, &textureID);

		int w, h, n;

		unsigned char* data = stbi_load(file_name.c_str(), &w, &h, &n, 0);

		width = w;
		height = h;

		if (data)
		{
			GLenum format;

			if (n == 1)
				format = GL_RED;
			else if (n == 3)
				format = GL_RGB;
			else if (n == 4)
				format = GL_RGBA;

			glBindTexture(GL_TEXTURE_2D, textureID);
			glTexImage2D(GL_TEXTURE_2D, 0, format, w, h, 0, format, GL_UNSIGNED_BYTE, data);
			glGenerateMipmap(GL_TEXTURE_2D);

			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);//gl_linear

			stbi_image_free(data);
		}
		else
		{
			cout << "Failed To Load Texture " << file_name << "\n";
			stbi_image_free(data);
		}

		return textureID;
	}

	vector<Texture> LoadTexture(aiMaterial* mat, aiTextureType type, string typeName)
	{
		vector<Texture> textures;

		for (unsigned int i = 0; i < mat->GetTextureCount(type); ++i)
		{
			aiString path;

			mat->GetTexture(type, i, &path);

			bool skip = false;

			for (unsigned int j = 0; j < textures_loaded.size(); ++j)
			{
				if (strcmp(textures_loaded[j].path.data(), path.C_Str()) == 0)
				{
					textures.push_back(textures_loaded[j]);
					skip = true;
					break;
				}
			}
			if (!skip)
			{
				Texture texture;
				int width = 0, height = 0;

				texture.id = TextureFromFile(path.C_Str(), direction, width, height);
				texture.path = path.C_Str();
				texture.type = typeName;
				texture.width = width;
				texture.height = height;
				textures.push_back(texture);
				textures_loaded.push_back(texture);
			}
		}

		return textures;
	}
};



#endif // !_MODEL_H_

