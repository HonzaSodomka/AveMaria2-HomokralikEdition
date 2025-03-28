#include <iostream>
#include <fstream>
#include <sstream>

#include <glm/glm.hpp>
#include <glm/ext.hpp>

#include "ShaderProgram.hpp"

// set uniform according to name 
// https://docs.gl/gl4/glUniform

ShaderProgram::ShaderProgram(const std::filesystem::path& VS_file, const std::filesystem::path& FS_file)
{
	std::vector<GLuint> shader_ids;

	shader_ids.push_back(compile_shader(VS_file, GL_VERTEX_SHADER));
	shader_ids.push_back(compile_shader(FS_file, GL_FRAGMENT_SHADER));

	ID = link_shader(shader_ids);
}

void ShaderProgram::setUniform(const std::string& name, const float val) {
	auto loc = glGetUniformLocation(ID, name.c_str());
	if (loc == -1) {
		std::cerr << "no uniform with name:" << name << '\n';
		return;
	}
	glUniform1f(loc, val);
}

void ShaderProgram::setUniform(const std::string& name, const int val) {
	auto loc = glGetUniformLocation(ID, name.c_str());
	if (loc == -1) {
		std::cerr << "no uniform with name:" << name << '\n';
		return;
	}
	glUniform1i(loc, val);
}

void ShaderProgram::setUniform(const std::string& name, const glm::vec3 val) {
	auto loc = glGetUniformLocation(ID, name.c_str());
	if (loc == -1) {
		std::cerr << "no uniform with name:" << name << '\n';
		return;
	}
	glUniform3fv(loc, 1, glm::value_ptr(val));
}

void ShaderProgram::setUniform(const std::string& name, const glm::vec4 val) {
	auto loc = glGetUniformLocation(ID, name.c_str());
	if (loc == -1) {
		std::cerr << "no uniform with name:" << name << '\n';
		return;
	}
	glUniform4fv(loc, 1, glm::value_ptr(val));
}

void ShaderProgram::setUniform(const std::string& name, const glm::mat3 val) {
	auto loc = glGetUniformLocation(ID, name.c_str());
	if (loc == -1) {
		std::cerr << "no uniform with name:" << name << '\n';
		return;
	}
	glUniformMatrix3fv(loc, 1, GL_FALSE, glm::value_ptr(val));
}

void ShaderProgram::setUniform(const std::string& name, const glm::mat4 val) {
	auto loc = glGetUniformLocation(ID, name.c_str());
	if (loc == -1) {
		std::cerr << "no uniform with name:" << name << '\n';
		return;
	}
	glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(val));
}

std::string ShaderProgram::getShaderInfoLog(const GLuint obj) {
	int logLength = 0;
	glGetShaderiv(obj, GL_INFO_LOG_LENGTH, &logLength);
	if (logLength > 0) {
		std::vector<char> log(logLength);
		glGetShaderInfoLog(obj, logLength, NULL, log.data());
		return std::string(log.begin(), log.end());
	}
	return "";
}

std::string ShaderProgram::getProgramInfoLog(const GLuint obj) {
	int logLength = 0;
	glGetProgramiv(obj, GL_INFO_LOG_LENGTH, &logLength);
	if (logLength > 0) {
		std::vector<char> log(logLength);
		glGetProgramInfoLog(obj, logLength, NULL, log.data());
		return std::string(log.begin(), log.end());
	}
	return "";
}

GLuint ShaderProgram::compile_shader(const std::filesystem::path& source_file, const GLenum type) {
	// Naètení obsahu souboru
	std::string shaderSource;
	try {
		shaderSource = textFileRead(source_file);
	}
	catch (const std::exception& e) {
		throw std::runtime_error("Couldn't load shader file: " + source_file.string() + "\nError: " + e.what());
	}

	// Vytvoøení a kompilace shaderu
	GLuint shader_h = glCreateShader(type);
	const char* source = shaderSource.c_str();
	glShaderSource(shader_h, 1, &source, NULL);
	glCompileShader(shader_h);

	// Kontrola úspìšnosti kompilace
	GLint success = 0;
	glGetShaderiv(shader_h, GL_COMPILE_STATUS, &success);

	// Získání log informací bez ohledu na výsledek (mohou tam být varování)
	std::string log = getShaderInfoLog(shader_h);
	if (!log.empty()) {
		std::cout << "Shader compilation log for " << source_file.filename() << ":\n" << log << std::endl;
	}

	if (success == GL_FALSE) {
		glDeleteShader(shader_h);
		throw std::runtime_error("Shader compilation failed for: " + source_file.string());
	}

	return shader_h;
}

GLuint ShaderProgram::link_shader(const std::vector<GLuint> shader_ids) {
	GLuint prog_h = glCreateProgram();

	for (const auto& id : shader_ids)
		glAttachShader(prog_h, id);

	glLinkProgram(prog_h);

	// Kontrola úspìšnosti linkování
	GLint success = 0;
	glGetProgramiv(prog_h, GL_LINK_STATUS, &success);

	// Získání log informací bez ohledu na výsledek
	std::string log = getProgramInfoLog(prog_h);
	if (!log.empty()) {
		std::cout << "Shader program linking log:\n" << log << std::endl;
	}

	// Odpojení shaderù po linkování
	for (const auto& id : shader_ids) {
		glDetachShader(prog_h, id);
		glDeleteShader(id); // Už nepotøebujeme jednotlivé shadery
	}

	if (success == GL_FALSE) {
		glDeleteProgram(prog_h);
		throw std::runtime_error("Shader program linking failed");
	}

	return prog_h;
}

std::string ShaderProgram::textFileRead(const std::filesystem::path& filename)
{
	std::ifstream file(filename);
	if (!file.is_open())
		throw std::runtime_error("Error opening file.\n");
	std::stringstream ss;
	ss << file.rdbuf();
	return ss.str();
}
