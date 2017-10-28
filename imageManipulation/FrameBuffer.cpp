#include "global.h"

// initialize this framebuffer
Framebuffer::Framebuffer(shared_ptr<SimpleTexture> t) {

	/* defer on assigning a name to this framebuffer, as it's
	*  guaranteed to fail before we get a context. */
	tex = t;
}

// or destroy it
Framebuffer::~Framebuffer() {

	// ensure we're clear
	Framebuffer::unbind();

	if (id != 0)
		glDeleteFramebuffers(1, &id);

}

// boring, all the work is done in bind()
void Framebuffer::setTexture(shared_ptr<SimpleTexture> t) { tex = t; }

// make this framebuffer the buffer we're drawing into
bool Framebuffer::bind() {

	// sanity checks
	if (!tex || !tex->isLoaded())
		return false;

	// do we need a name?
	if (id == 0) {

		glGenFramebuffers(1, &id);
		if (OpenGL::error("glGenFramebuffers"))
			return false;
	}


	// first up, bind ourselves as the active buffer
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, id);
	if (OpenGL::error("glBindFramebuffer"))
		return false;

	// next, associate the texture
	glFramebufferTexture(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, tex->id, 0);
	if (OpenGL::error("glFramebufferTexture2D"))
		return false;

	// and tell OpenGL where to draw
	glDrawBuffer(GL_COLOR_ATTACHMENT0);
	if (OpenGL::error("glDrawBuffer"))
		return false;

	// finally, check the framebuffer is ready to go
	switch (glCheckFramebufferStatus(GL_DRAW_FRAMEBUFFER)) {

	case GL_FRAMEBUFFER_COMPLETE:
		return true;

	case GL_FRAMEBUFFER_UNDEFINED:
		cerr << "ERROR: The framebuffer is undefined." << endl;
		return false;

	case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
		cerr << "ERROR: Incomplete framebuffer attachement." << endl;
		return false;

	case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
		cerr << "ERROR: The framebuffer does not have an image attached." << endl;
		return false;

	case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER:
		cerr << "ERROR: The framebuffer's output is missing a target." << endl;
		return false;

	case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER:
		cerr << "ERROR: The framebuffer is not set up for reading." << endl;
		return false;

	case GL_FRAMEBUFFER_UNSUPPORTED:
		cerr << "ERROR: Invalid set of parameters for this framebuffer." << endl;
		return false;

	case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE:
		cerr << "ERROR: Inconsistent sampling parameters for this framebuffer." << endl;
		return false;

	case GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS:
		cerr << "ERROR: Framebuffer layers missing." << endl;
		return false;

	default:
		cerr << "ERROR: Unknown framebuffer error." << endl;
	}

	return false;	// in case the C++ compiler complains
}

// release any attached framebuffers
void Framebuffer::unbind() { glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0); }
