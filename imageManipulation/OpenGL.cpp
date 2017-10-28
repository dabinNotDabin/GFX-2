#include "global.h"

// check if the last command resulted in an error
bool OpenGL::error(string t) {

	// retrieve the error code
	GLenum errorCode = glGetError();

	// exit early, if nothing's wrong
	if (errorCode == GL_NO_ERROR)
		return false;

	// while we have errors
	while (errorCode != GL_NO_ERROR) {

		string errorMsg;
		switch (errorCode) {

		case GL_INVALID_ENUM:

			errorMsg = "Invalid GLenum";
			break;

		case GL_INVALID_VALUE:

			errorMsg = "Illegal value";
			break;

		case GL_INVALID_OPERATION:

			errorMsg = "Invalid operation";
			break;

		case GL_STACK_OVERFLOW:

			errorMsg = "Stack overflow";
			break;

		case GL_STACK_UNDERFLOW:

			errorMsg = "Removal from empty stack";
			break;

		case GL_OUT_OF_MEMORY:	// this one is special

			cerr << "ERROR: Out of memory";
			if (!t.empty())
				cerr << " at " << t;
			cerr << endl;

			// die here? OOM is usually fatal

			errorCode = glGetError();
			continue;

		case GL_INVALID_FRAMEBUFFER_OPERATION:

			errorMsg = "Invalid framebuffer operation";
			break;

		case GL_CONTEXT_LOST:

			errorMsg = "Graphics context lost";
			break;

		default:

			errorMsg = "Unknown error";
			break;

		}

		// actually print out the error message
		cerr << "WARNING: " << errorMsg;
		if (!t.empty())
			cerr << " at " << t;
		cerr << endl;

		// keep processing, there may be more
		errorCode = glGetError();
	}

	// we can only reach this line if an error was detected
	return true;

}
