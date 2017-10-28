#include "global.h"

// create a shader program
ShaderProgram::ShaderProgram() {

	// defer on assigning a name until a context is in place
	shaders.clear();
}

// delete the current program
ShaderProgram::~ShaderProgram() {

	ShaderProgram::unbind();	// unbind first, to prevent side-effects

	if (id)				// now delete this program
		glDeleteProgram(id);

	// finally, unbind any shaders
	for (auto const& shader : shaders)
		glDeleteShader(shader.second);

}


bool ShaderProgram::addShader(string path, GLenum type)
{
	// no valid program ID? Create one
	if (id <= 0) {

		id = glCreateProgram();

		if (OpenGL::error("glCreateProgram"))
			return false;
	}

	// are we considered valid? Then we're read-only
	if (valid)
		return false;


	std::ifstream in(path);
	string data = [&in] {
		std::ostringstream ss{};
		ss << in.rdbuf();
		return ss.str();
	}();

	// null data? Reject that too
		if (data.empty())
			return false;

	// otherwise, begin creation (but don't add unless successful!)
	GLuint shaderName = glCreateShader(type);
	if (OpenGL::error("glCreateShader"))
		return false;

	// create an array of arrays to satisfy OpenGL
	const GLchar* shaderArray[] = { data.c_str() };
	glShaderSource(shaderName, 1, shaderArray, 0);
	if (OpenGL::error("glShaderSource"))
		return false;

	glCompileShader(shaderName);
	if (OpenGL::error("glCompileShader"))
		return false;

	// check the compilation status
	GLint state;
	glGetShaderiv(shaderName, GL_COMPILE_STATUS, &state);
	if (state == GL_FALSE) {

		// didn't compile? the programmer would probably want to know why
		glGetShaderiv(shaderName, GL_INFO_LOG_LENGTH, &state);

		char* buffer = new char[state];
		glGetShaderInfoLog(shaderName, state, &state, buffer);
		buffer[state - 1] = '\0';		// guarantee we're null-terminated

										// finally, spit out the warning
		cerr << "ERROR: Shader " << shaderName << " did not compile. The compile log:" << endl;
		cerr << buffer << endl;

		// and clean up
		glDeleteShader(shaderName);
		return false;
	}

	// a shader of this type already exists? trash it
	if (shaders.find(type) != shaders.end()) {

		glDeleteShader(shaders[type]);
		if (OpenGL::error("glDeleteShader"))
			return false;
	}

	// finally, add this shader in
	glAttachShader(id, shaderName);
	if (OpenGL::error("glAttachShader"))
		return false;

	shaders[type] = shaderName;

	return true;
}




// attach a shader to the existing program via string.
bool ShaderProgram::attachShader(string data, GLenum type) {

	// no valid program ID? Create one
	if (id <= 0) {

		id = glCreateProgram();

		if (OpenGL::error("glCreateProgram"))
			return false;
	}

	// are we considered valid? Then we're read-only
	if (valid)
		return false;

	// null data? Reject that too
	if (data.empty())
		return false;

	// otherwise, begin creation (but don't add unless successful!)
	GLuint shaderName = glCreateShader(type);
	if (OpenGL::error("glCreateShader"))
		return false;

	// create an array of arrays to satisfy OpenGL
	const GLchar* shaderArray[] = { data.c_str() };
	glShaderSource(shaderName, 1, shaderArray, 0);
	if (OpenGL::error("glShaderSource"))
		return false;

	glCompileShader(shaderName);
	if (OpenGL::error("glCompileShader"))
		return false;

	// check the compilation status
	GLint state;
	glGetShaderiv(shaderName, GL_COMPILE_STATUS, &state);
	if (state == GL_FALSE) {

		// didn't compile? the programmer would probably want to know why
		glGetShaderiv(shaderName, GL_INFO_LOG_LENGTH, &state);

		char* buffer = new char[state];
		glGetShaderInfoLog(shaderName, state, &state, buffer);
		buffer[state - 1] = '\0';		// guarantee we're null-terminated

										// finally, spit out the warning
		cerr << "ERROR: Shader " << shaderName << " did not compile. The compile log:" << endl;
		cerr << buffer << endl;

		// and clean up
		glDeleteShader(shaderName);
		return false;
	}

	// a shader of this type already exists? trash it
	if (shaders.find(type) != shaders.end()) {

		glDeleteShader(shaders[type]);
		if (OpenGL::error("glDeleteShader"))
			return false;
	}

	// finally, add this shader in
	glAttachShader(id, shaderName);
	if (OpenGL::error("glAttachShader"))
		return false;

	shaders[type] = shaderName;

	return true;
}

// check if this type of shader already exists for this program
bool ShaderProgram::hasShader(GLenum type) { return shaders.find(type) != shaders.end(); }

// link this program, if possible
bool ShaderProgram::link() {

	// are we already considered valid? Complain
	if (valid || id <= 0)
		return false;

	// do we have the shaders we need? If not, error out
	if ((shaders.find(GL_VERTEX_SHADER) == shaders.end()) ||
		(shaders.find(GL_FRAGMENT_SHADER) == shaders.end()))
		return false;

	// try linking then, and error out if unsuccessful
	glLinkProgram(id);
	if (OpenGL::error("glLinkProgram"))
		return false;

	GLint state = 0;
	glGetProgramiv(id, GL_LINK_STATUS, &state);
	if (state == GL_FALSE) {

		// didn't link? Why the heck not?
		glGetProgramiv(id, GL_INFO_LOG_LENGTH, &state);

		char* buffer = new char[state];
		glGetProgramInfoLog(id, state, &state, buffer);
		buffer[state - 1] = '\0';         // ensure we're null-terminated

										  // finally, spit out the warning
		cerr << "ERROR: Program " << id << " did not link. The program log:" << endl;
		cerr << buffer << endl;

		// don't clean up, allow the programmer to try fiddling with shaders
		return false;
	}

	// success! mark us read-only, too
	valid = true;
	return valid;

}

// set the value of an int uniform
bool ShaderProgram::setInt(string variable, GLint value) {

	// grab the uniform's location, if possible
	GLint location = glGetUniformLocation(id, variable.c_str());
	if (location < 0)
		return false;

	glUniform1i(location, value);
	return true;

}

// set the value of a float uniform
bool ShaderProgram::setFloat(string variable, GLfloat value) {

	// grab the uniform's location, if possible
	GLint location = glGetUniformLocation(id, variable.c_str());
	if (location < 0)
		return false;

	glUniform1f(location, value);
	return true;

}

// set the value of a vec2 uniform
bool ShaderProgram::setVec2(string variable, GLfloat first, GLfloat second) {

	// grab the uniform's location, if possible
	GLint location = glGetUniformLocation(id, variable.c_str());
	if (location < 0)
		return false;

/*
	const GLsizei bufferSize = 50;
	GLchar uniformName[bufferSize];
	GLsizei length;
	glGetActiveUniformName(id, location, bufferSize, &length, uniformName);
	cerr << endl << endl << uniformName << endl;

	GLint params;
	glGetProgramiv(id, GL_ACTIVE_UNIFORMS, &params);

	cout << "num uniforms  " << params << endl;
*/
	glUniform2f(location, first, second);

	return true;

}

bool ShaderProgram::setMat4(string uniformName, mat4 matrix)
{
	GLint viewHandle = glGetUniformLocation(id, uniformName.c_str());

	if (viewHandle < 0)
	{
		cout << "ShaderProgram::seMat4 - getUniformLocation failed.\n";
		return false;
	}

	glUniformMatrix4fv(viewHandle, 1, GL_FALSE, glm::value_ptr(matrix));

	return true;
}

// associate a texture with an input
bool ShaderProgram::setTexture(string variable, shared_ptr<SimpleTexture> tex) {

	// sanity check
	if (tex == nullptr || !tex->isLoaded())
		return false;

	// grab the texture's location, if possible
	GLint location = glGetUniformLocation(id, variable.c_str());
	if (location < 0)
		return false;

	// textures require a bit more work
	glUniform1i(location, textureSlot);

	glActiveTexture(GL_TEXTURE0 + textureSlot);
	if (OpenGL::error("glActiveTexture"))
		return false;

	glBindTexture(tex->type, tex->id);
	if (OpenGL::error("glBindTexture"))
		return false;

	if (tex->loadSampler(textureSlot)) {

		textureSlot++;		// only increment on success
		return true;
	}
	else
		return false;

}

// if we're valid, bind this program
bool ShaderProgram::bind() {

	if (!valid)
		return false;

	glUseProgram(id);
	if (OpenGL::error("glUseProgram"))
		return false;

	textureSlot = 0;		// reset the active texture slot
	return true;

}

// unbind from all programs
void ShaderProgram::unbind() { glUseProgram(0); }

// a helper function for reading in shaders
string ShaderProgram::readShader(string filename) {

	// no empty strings, please
	if (filename.empty()) {

		cerr << "ERROR: No shader filename given!" << endl;
		return "";
	}

	// try opening the file first
	ifstream file;
	file.open(filename, fstream::in);

	// no luck? return a blank string
	if (file.fail()) {

		cerr << "ERROR: Could not open the shader file \"" << filename << "\"!" << endl;
		return "";
	}

	// now try reading it in
	string output = "";
	string buffer;

	getline(file, buffer);
	while (file.good()) {

		output += buffer + '\n';
		getline(file, buffer);
	}

	// all happy? close off the file and return
	file.close();
	return output;

}
