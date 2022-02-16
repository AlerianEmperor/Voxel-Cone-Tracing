/*#ifndef _SHADER_H_
#define _SHADER_H_

#include <gl\glew.h>
#include <glm\glm.hpp>
#include <iostream>
#include <string>
#include <fstream>

using namespace std;
using namespace glm;

class Shader
{
public:
	unsigned int id;

	Shader() {}

	Shader(string vs_name, string fs_name, string gs_name = "")
	{
		GLuint vs = CreateShader(GL_VERTEX_SHADER, vs_name);
		GLuint fs = CreateShader(GL_FRAGMENT_SHADER, fs_name);

		id = glCreateProgram();

		glAttachShader(id, vs);
		glAttachShader(id, fs);

		if (gs_name != "")
		{
			GLuint gs = CreateShader(GL_GEOMETRY_SHADER, gs_name);
			glAttachShader(id, gs);
		}

		id = LinkingProgram(id);
	}

	string Read_Shader_Source(const string& filename)
	{
		ifstream file(filename, ios::in);
		string line = "";
		string result = "";
		while (!file.eof())
		{
			//cout << line << "\n";
			getline(file, line);
			result += line + "\n";
		}
		//cout << result;
		return result;
	}


	GLuint CreateShader(int shaderType, const string& ShaderPath)
	{
		GLint ShaderCompiled;
		string ShaderSource = Read_Shader_Source(ShaderPath);
		const char* source = ShaderSource.c_str();
		GLuint ShaderReference = glCreateShader(shaderType);
		glShaderSource(ShaderReference, 1, &source, NULL);

		glCompileShader(ShaderReference);
		CheckOpenGLError();
		glGetShaderiv(ShaderReference, GL_COMPILE_STATUS, &ShaderCompiled);

		if (ShaderCompiled != 1)
		{
			if (shaderType == 35633) cout << "Vertex ";
			if (shaderType == 36488) cout << "Tess Control ";
			if (shaderType == 36487) cout << "Tess Eval ";
			if (shaderType == 36313) cout << "Geometry ";
			if (shaderType == 35632) cout << "Fragment ";
			cout << "shader compilation error." << endl;
			cout << ShaderPath << "\n";
			PrintShaderLog(ShaderReference);
		}
		return ShaderReference;
	}

	GLuint CreateProgram(const string& vs_name, const string& fs_name)
	{
		GLuint vs = CreateShader(GL_VERTEX_SHADER, vs_name);
		GLuint fs = CreateShader(GL_FRAGMENT_SHADER, fs_name);

		GLuint program = glCreateProgram();

		glAttachShader(program, vs);
		glAttachShader(program, fs);

		//glLinkProgram(program);

		program = LinkingProgram(program);

		return program;
	}

	GLuint CreateProgram(const string& vs_name, const string& fs_name, const string& gs_name)
	{
		GLuint vs = CreateShader(GL_VERTEX_SHADER, vs_name);
		GLuint fs = CreateShader(GL_FRAGMENT_SHADER, fs_name);
		GLuint gs = CreateShader(GL_GEOMETRY_SHADER, gs_name);

		GLuint program = glCreateProgram();

		glAttachShader(program, vs);
		glAttachShader(program, fs);
		glAttachShader(program, gs);

		//glLinkProgram(program);

		program = LinkingProgram(program);

		return program;
	}

	//Check Errors
	bool CheckOpenGLError()
	{
		bool foundError = false;
		int glErr = glGetError();
		if (glErr != GL_NO_ERROR)
		{
			//cout << "glErr: " << glErr << "\n";
			foundError = true;
			glErr = glGetError();
			//getchar();
		}
		return foundError;
	}

	void PrintShaderLog(GLuint shader)
	{
		int len = 0;
		int char_Written = 0;
		char* log;
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &len);
		if (len > 0)
		{
			log = (char*)malloc(len);
			glGetShaderInfoLog(shader, len, &char_Written, log);
			cout << "Shader Info Log " << log << "\n";
			free(log);
			//getchar();
		}
	}

	void PrintProgramLog(GLuint program)
	{
		int len = 0;
		int char_Written = 0;
		char* log;
		glGetProgramiv(program, GL_INFO_LOG_LENGTH, &len);
		if (len > 0)
		{
			log = (char*)malloc(len);
			glGetProgramInfoLog(program, len, &char_Written, log);
			cout << "Program Info Log " << log << "\n";
			free(log);
			//getchar();
		}
	}

	GLuint LinkingProgram(GLuint program)
	{
		GLint linked;
		glLinkProgram(program);
		CheckOpenGLError();
		glGetProgramiv(program, GL_LINK_STATUS, &linked);
		if (!linked)
		{
			cout << "linking failed!" << "\n";
			PrintProgramLog(program);
			getchar();
		}
		return program;
	}

	void use()
	{
		glUseProgram(id);
	}
	
	void setBool(const string& name, bool value)
	{
		glUniform1i(glGetUniformLocation(id, name.c_str()), value);
	}
	
	void setInt(const string& name, int value)
	{
		glUniform1i(glGetUniformLocation(id, name.c_str()), value);
	}
	
	void setFloat(const string& name, float value)
	{
		glUniform1f(glGetUniformLocation(id, name.c_str()), value);
	}
	
	void setVec2(const string& name, const vec2& value)
	{
		glUniform2fv(glGetUniformLocation(id, name.c_str()), 1, &value[0]);
	}
	
	void setVec2(const string& name, float x, float y)
	{
		glUniform2f(glGetUniformLocation(id, name.c_str()), x, y);
	}
	
	void setVec3(const string& name, const vec3& value)
	{
		glUniform3fv(glGetUniformLocation(id, name.c_str()), 1, &value[0]);
	}
	
	void setVec3(const string& name, float x, float y, float z)
	{
		glUniform3f(glGetUniformLocation(id, name.c_str()), x, y, z);
	}

	void setVec4(const string& name, const vec4& value)
	{
		glUniform4fv(glGetUniformLocation(id, name.c_str()), 1, &value[0]);
	}
	
	void setVec4(const string& name, float x, float y, float z, float w)
	{
		glUniform4f(glGetUniformLocation(id, name.c_str()), x, y, z, w);
	}

	void setMat2(const string& name, const mat2& mat)
	{
		glUniformMatrix2fv(glGetUniformLocation(id, name.c_str()), 1, GL_FALSE, &mat[0][0]);
	}

	void setMat3(const string& name, const mat3& mat)
	{
		glUniformMatrix3fv(glGetUniformLocation(id, name.c_str()), 1, GL_FALSE, &mat[0][0]);
	}

	void setMat4(const string& name, const mat4& mat)
	{
		glUniformMatrix4fv(glGetUniformLocation(id, name.c_str()), 1, GL_FALSE, &mat[0][0]);
	}
};

#endif // !_SHADER_H_
*/

#ifndef SHADER_H
#define SHADER_H

//#include "glad.h"
#include <gl\glew.h>
#include <glm/glm.hpp>

#include <string.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>
#include <gl\freeglut.h>

using namespace std;
using namespace glm;

class Shader
{
public:
	unsigned int id;
	// constructor generates the shader on the fly
	// ------------------------------------------------------------------------
	Shader() {}
	Shader(const char* vertexPath, const char* fragmentPath, const char* geometryPath = nullptr)
	{
		// 1. retrieve the vertex/fragment source code from filePath
		std::string vertexCode;
		std::string fragmentCode;
		std::string geometryCode;
		std::ifstream vShaderFile;
		std::ifstream fShaderFile;
		std::ifstream gShaderFile;
		// ensure ifstream objects can throw exceptions:
		vShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
		fShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
		gShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
		try
		{
			// open files
			vShaderFile.open(vertexPath);
			fShaderFile.open(fragmentPath);
			std::stringstream vShaderStream, fShaderStream;
			// read file's buffer contents into streams
			vShaderStream << vShaderFile.rdbuf();
			fShaderStream << fShaderFile.rdbuf();
			// close file handlers
			vShaderFile.close();
			fShaderFile.close();
			// convert stream into string
			vertexCode = vShaderStream.str();
			fragmentCode = fShaderStream.str();
			// if geometry shader path is present, also load a geometry shader
			if (geometryPath != nullptr)
			{
				gShaderFile.open(geometryPath);
				std::stringstream gShaderStream;
				gShaderStream << gShaderFile.rdbuf();
				gShaderFile.close();
				geometryCode = gShaderStream.str();
			}
		}
		catch (std::ifstream::failure& e)
		{
			std::cout << "ERROR::SHADER::FILE_NOT_SUCCESFULLY_READ" << std::endl;
		}
		const char* vShaderCode = vertexCode.c_str();
		const char* fShaderCode = fragmentCode.c_str();
		// 2. compile shaders
		unsigned int vertex, fragment;
		// vertex shader
		vertex = glCreateShader(GL_VERTEX_SHADER);
		glShaderSource(vertex, 1, &vShaderCode, NULL);
		glCompileShader(vertex);
		checkCompileErrors(vertex, "VERTEX", vertexPath);
		// fragment Shader
		fragment = glCreateShader(GL_FRAGMENT_SHADER);
		glShaderSource(fragment, 1, &fShaderCode, NULL);
		glCompileShader(fragment);
		checkCompileErrors(fragment, "FRAGMENT", fragmentPath);
		// if geometry shader is given, compile geometry shader
		unsigned int geometry;
		if (geometryPath != nullptr)
		{
			const char* gShaderCode = geometryCode.c_str();
			geometry = glCreateShader(GL_GEOMETRY_SHADER);
			glShaderSource(geometry, 1, &gShaderCode, NULL);
			glCompileShader(geometry);
			checkCompileErrors(geometry, "GEOMETRY", geometryPath);
		}
		// shader Program
		id = glCreateProgram();
		glAttachShader(id, vertex);
		glAttachShader(id, fragment);
		if (geometryPath != nullptr)
			glAttachShader(id, geometry);
		glLinkProgram(id);
		checkCompileErrors(id, "PROGRAM", "program");
		// delete the shaders as they're linked into our program now and no longer necessery
		glDeleteShader(vertex);
		glDeleteShader(fragment);
		if (geometryPath != nullptr)
			glDeleteShader(geometry);

	}
	// activate the shader
	// ------------------------------------------------------------------------
	void use()
	{
		glUseProgram(id);
	}
	// utility uniform functions
	// ------------------------------------------------------------------------
	void setBool(const std::string& name, bool value) const
	{
		glUniform1i(glGetUniformLocation(id, name.c_str()), (int)value);
	}
	// ------------------------------------------------------------------------
	void setInt(const std::string& name, int value) const
	{
		glUniform1i(glGetUniformLocation(id, name.c_str()), value);
	}
	// ------------------------------------------------------------------------
	void setFloat(const std::string& name, float value) const
	{
		glUniform1f(glGetUniformLocation(id, name.c_str()), value);
	}
	// ------------------------------------------------------------------------
	void setVec2(const std::string& name, const glm::vec2& value) const
	{
		glUniform2fv(glGetUniformLocation(id, name.c_str()), 1, &value[0]);
	}
	void setVec2(const std::string& name, float x, float y) const
	{
		glUniform2f(glGetUniformLocation(id, name.c_str()), x, y);
	}
	// ------------------------------------------------------------------------
	void setVec3(const std::string& name, const glm::vec3& value) const
	{
		glUniform3fv(glGetUniformLocation(id, name.c_str()), 1, &value[0]);
	}
	void setVec3(const std::string& name, float x, float y, float z) const
	{
		glUniform3f(glGetUniformLocation(id, name.c_str()), x, y, z);
	}
	// ------------------------------------------------------------------------
	void setVec4(const std::string& name, const glm::vec4& value) const
	{
		glUniform4fv(glGetUniformLocation(id, name.c_str()), 1, &value[0]);
	}
	void setVec4(const std::string& name, float x, float y, float z, float w)
	{
		glUniform4f(glGetUniformLocation(id, name.c_str()), x, y, z, w);
	}
	// ------------------------------------------------------------------------
	void setMat2(const std::string& name, const glm::mat2& mat) const
	{
		glUniformMatrix2fv(glGetUniformLocation(id, name.c_str()), 1, GL_FALSE, &mat[0][0]);
	}
	// ------------------------------------------------------------------------
	void setMat3(const std::string& name, const glm::mat3& mat) const
	{
		glUniformMatrix3fv(glGetUniformLocation(id, name.c_str()), 1, GL_FALSE, &mat[0][0]);
	}
	// ------------------------------------------------------------------------
	void setMat4(const std::string& name, const glm::mat4& mat) const
	{
		glUniformMatrix4fv(glGetUniformLocation(id, name.c_str()), 1, GL_FALSE, &mat[0][0]);
	}

private:
	// utility function for checking shader compilation/linking errors.
	// ------------------------------------------------------------------------
	void checkCompileErrors(GLuint shader, std::string type, const char *path)
	{
		GLint success;
		int infoLogLength;
		printf("checking %s ...\n", path);
		if (type != "PROGRAM")
		{
			glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
			glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLogLength);
			if (infoLogLength > 0) {
				std::vector<char> errorMessage(infoLogLength + 1);
				glGetShaderInfoLog(shader, infoLogLength, NULL, &errorMessage[0]);
				printf("%s\n", &errorMessage[0]);
			}
		}
		else
		{
			glGetProgramiv(shader, GL_LINK_STATUS, &success);
			glGetProgramiv(shader, GL_INFO_LOG_LENGTH, &infoLogLength);
			if (infoLogLength > 0) {
				std::vector<char> ProgramErrorMessage(infoLogLength + 1);
				glGetProgramInfoLog(shader, infoLogLength, NULL, &ProgramErrorMessage[0]);
				printf("%s\n", &ProgramErrorMessage[0]);
			}
		}
	}
};
#endif
