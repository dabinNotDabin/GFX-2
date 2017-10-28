#include "global.h"

// fairly standard constructor, though note the failure cases
SimpleTexture::SimpleTexture(uint w, uint h, GLenum f) {

	glGenTextures(1, &id);	// generate the texture and sampler
	glGenSamplers(1, &sampler);

	width = w;			// store these for later use
	height = h;
	format = f;

	switch (f) {			// how many channels do we have?

	case GL_R8:
	case GL_R16:
	case GL_R16F:
	case GL_R32F:
		perPixelChan = 1;
		break;

	case GL_RGB8:
	case GL_BGR:
		perPixelChan = 3;
		break;

	case GL_RGBA8:
	case GL_RGBA16F:
	case GL_RGBA32F:
		perPixelChan = 4;
		break;

	default:		// default: invalidate this texture!
		perPixelChan = 0;
		break;
	}

}

// just trash the texture
SimpleTexture::~SimpleTexture() { glDeleteTextures(1, &id); }

// allocate blank storage, if possible
bool SimpleTexture::load() {

	if (hasStorage || (perPixelChan == 0))		// if allocated or invalid, throw an error
		return false;

	glBindTexture(type, id);
	if (OpenGL::error("glBindTexture"))
		return false;

	glTexStorage2D(type, 1, format, width, height);
	if (OpenGL::error("glTexStorage2D"))
		return false;

	glBindTexture(type, 0);			// unbind and mark as ready to go
	hasStorage = true;

	return true;
}

// stuff some values into the texture
bool SimpleTexture::load(vector<float> data) {

	if (hasStorage || (perPixelChan == 0))		// if allocated or invalid, throw an error
		return false;

	uint storage = width * height * perPixelChan;
	if (data.size() != storage)			// given the proper number of pixels?
	{
		cout << "Simple Texture::load, width*height*perPixelChan != data.size()" << endl;
//		return false;
	}

	// determine the proper format
	GLenum components;
	switch (perPixelChan) {

	case 1:
		components = GL_RED;
		break;
	case 3:
		components = GL_RGB;
		break;
	case 4:
		components = GL_RGBA;
		break;
	default:
		return false;
	}

	glBindTexture(type, id);
	if (OpenGL::error("glBindTexture"))
		return false;

	glTexImage2D(type, 0, format, width, height, 0, GL_BGR, GL_FLOAT, data.data());
	if (OpenGL::error("glTexImage2D"))
		return false;

	glBindTexture(type, 0);			// unbind and mark as ready to go
	hasStorage = true;

	return true;

}

// handle sampler settings
bool SimpleTexture::setDownsampler(GLenum value) {

	// check for validity
	switch (value) {

	case GL_NEAREST:
	case GL_LINEAR:
	case GL_NEAREST_MIPMAP_NEAREST:
	case GL_LINEAR_MIPMAP_NEAREST:
	case GL_NEAREST_MIPMAP_LINEAR:
	case GL_LINEAR_MIPMAP_LINEAR:

		downsampling = value;
		return true;
	}

	return false;
}

bool SimpleTexture::setUpsampler(GLenum value) {

	// check for validity
	switch (value) {

	case GL_NEAREST:
	case GL_LINEAR:

		upsampling = value;
		return true;
	}

	return false;
}

bool SimpleTexture::setWrapping(GLenum value) {

	// check for validity
	switch (value) {

	case GL_CLAMP_TO_BORDER:
	case GL_CLAMP_TO_EDGE:
	case GL_MIRRORED_REPEAT:
	case GL_MIRROR_CLAMP_TO_EDGE:
	case GL_REPEAT:

		wrapping = value;
		return true;
	}

	return false;
}

// load the sampler settings
bool SimpleTexture::loadSampler(GLuint unit) {

	// bind the sampler first
	glBindSampler(unit, sampler);
	if (OpenGL::error("glBindSampler"))
		return false;

	// set up the parameters
	glSamplerParameteri(sampler, GL_TEXTURE_MIN_FILTER, downsampling);
	glSamplerParameteri(sampler, GL_TEXTURE_MAG_FILTER, upsampling);
	glSamplerParameteri(sampler, GL_TEXTURE_WRAP_S, wrapping);
	glSamplerParameteri(sampler, GL_TEXTURE_WRAP_T, wrapping);
	glSamplerParameteri(sampler, GL_TEXTURE_WRAP_R, wrapping);

	// cheat a bit and only error check here
	return !OpenGL::error("glSamplerParameter");

}
