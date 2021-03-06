//-------------------------------------------------------------------------------------------------
#include "Shader.h"
#define ERROR_MAX 256

using namespace Triton;
using namespace std;

// declaration of static member variables
Shader* Shader::active = nullptr;

Shader::Shader()
{
	handle = glCreateProgram();
	for (unsigned short i = 0; i < UNIFORM_COUNT + MAX_BONES * 5 + MAX_POINTLIGHTS * 4; ++i)
		uniforms[i] = NULL;
	for (unsigned short i = 0; i < SHADERTYPE_COUNT; ++i)
		m_components[i] = NULL;
	m_componentCount = 0;
}

void Shader::addComponent(string& GLSLstrings, GLenum shaderType)
{
	m_components[m_componentCount] = glCreateShader(shaderType);

	GLuint componentHandle = m_components[m_componentCount++];

	// can't call checkShader error because we don't have a handle
	if (componentHandle == NULL)
	{
		string errorMsg = "Error: Shader creation failed!";
		throw errorMsg;
	}

	// type conversion between c++ types and openGL types
	const GLchar* GLSL_c_strings[1];
	GLint GLSL_character_count[1];

	GLSL_c_strings[0] = GLSLstrings.c_str();
	GLSL_character_count[0] = GLSLstrings.length();

	// sends the shader source code to openGL
	glShaderSource(componentHandle, 1, GLSL_c_strings, GLSL_character_count);
	// compiles the shader
	glCompileShader(componentHandle);

	// error checking to see if the shader compiled correctly
	try{
		checkShaderError(componentHandle, GL_COMPILE_STATUS, false, "Error: Shader Compilation Failed: ");
	}
	catch (const string& errorMessage)
	{
		throw errorMessage;
	}

	glAttachShader(handle, componentHandle);
}

void Shader::compile()
{
	// binds the locations of these variables in the shader program to these attribute locations
	glBindAttribLocation(handle, VERTEX, "vertex");
	glBindAttribLocation(handle, UV, "UV");
	glBindAttribLocation(handle, NORMAL, "normal");
	glBindAttribLocation(handle, TANGENT, "tangent");
	glBindAttribLocation(handle, WEIGHTS, "weights");
	glBindAttribLocation(handle, WEIGHT_INDICES, "weight_indices");
	
	glLinkProgram(handle);
	try{
		checkShaderError(handle, GL_LINK_STATUS, true, "Error: Program Linking Failed: ");
	}
	catch (const string& errorMessage)
	{
		throw errorMessage;
	}

	glValidateProgram(handle);
	try{
		checkShaderError(handle, GL_VALIDATE_STATUS, true, "Error: Program is Invalid: ");
	}
	catch (const string& errorMessage)
	{
		throw errorMessage;
	}
}

void Shader::bind()
{
	glUseProgram(handle);
	active = this;

	uniforms[CAMERA] = glGetUniformLocation(handle, "camera");
	uniforms[CAMERAPOS] = glGetUniformLocation(handle, "eyePos");
	uniforms[OBJECTPOS] = glGetUniformLocation(handle, "objectPos");
	for (unsigned short i = 0; i < MAX_BONES; ++i)
		uniforms[UNIFORM_COUNT + i] = glGetUniformLocation(handle, ("offsets[" + to_string(i) + "]").c_str());
	for (unsigned short i = 0; i < MAX_BONES; ++i)
		uniforms[UNIFORM_COUNT + MAX_BONES + i] = glGetUniformLocation(handle, ("rotations[" + to_string(i) + "]").c_str());
	for (unsigned short i = 0; i < MAX_BONES; ++i)
		uniforms[UNIFORM_COUNT + MAX_BONES*2 + i] = glGetUniformLocation(handle, ("parentIndices[" + to_string(i) + "]").c_str());
	for (unsigned short i = 0; i < MAX_BONES; ++i)
		uniforms[UNIFORM_COUNT + MAX_BONES*3 + i] = glGetUniformLocation(handle, ("bonePos[" + to_string(i) + "]").c_str());
	for (unsigned short i = 0; i < MAX_BONES; ++i)
		uniforms[UNIFORM_COUNT + MAX_BONES * 4 + i] = glGetUniformLocation(handle, ("scales[" + to_string(i) + "]").c_str());
	for (unsigned short i = 0; i < MAX_POINTLIGHTS; ++i)
	{
		uniforms[UNIFORM_COUNT + MAX_BONES * 5 + i * 4] = glGetUniformLocation(handle, ("pointLights[" + to_string(i) + "].color").c_str());
		uniforms[UNIFORM_COUNT + MAX_BONES * 5 + i * 4 + 1] = glGetUniformLocation(handle, ("pointLights[" + to_string(i) + "].atten").c_str());
		uniforms[UNIFORM_COUNT + MAX_BONES * 5 + i * 4 + 2] = glGetUniformLocation(handle, ("pointLights[" + to_string(i) + "].intensity").c_str());
		uniforms[UNIFORM_COUNT + MAX_BONES * 5 + i * 4 + 3] = glGetUniformLocation(handle, ("pointLights[" + to_string(i) + "].position").c_str());
	}
		
}

Shader::~Shader()
{
	for (unsigned short i = 0; i < m_componentCount; ++i)
	{
		glDetachShader(handle, m_components[i]);
		glDeleteShader(m_components[i]);
	}

	glDeleteProgram(handle);
}

void Triton::checkShaderError(GLuint shaderHandle, GLuint errorFlag, bool isProgram,
	const string& errorMessage)
{
	GLint success = 0;
	GLchar error[ERROR_MAX] = { 0 };

	// checks if checking for error on a shader program handle or shader component handle
	if (isProgram)
		// gets info for shader program, and assigns a positive value to success if successful
		glGetProgramiv(shaderHandle, errorFlag, &success);
	else
		// gets info for shader component, and assigns a positive value to success if successful
		glGetShaderiv(shaderHandle, errorFlag, &success);

	// checks if success is not false
	if (!success)
	{
		if (isProgram)
			// gets error mess
			glGetProgramInfoLog(shaderHandle, sizeof(error), NULL, error);
		else
			glGetShaderInfoLog(shaderHandle, sizeof(error), NULL, error);

		// prints errormessage to console
		throw (errorMessage + ": '" + error + "'");
	}
}