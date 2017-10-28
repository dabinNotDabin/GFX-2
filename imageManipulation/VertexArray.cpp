#include "global.h"

// shared routine for the constructors
void VertexArray::initHelper(vector<GLfloat>& data) {

	// generate a named vertex array and bind to it
	glGenVertexArrays(1, &id);
	if (OpenGL::error("glGenVertexArrays"))
		return;

	glBindVertexArray(id);
	if (OpenGL::error("glBindVertexArray"))
		return;

	// generate some storage space
	glGenBuffers(1, &bufID);
	if (OpenGL::error("glGenBuffers"))
		return;

	glBindBuffer(GL_ARRAY_BUFFER, bufID);
	if (OpenGL::error("glBindBuffer"))
		return;


	// transfer over the data
	GLenum type = GL_DYNAMIC_DRAW;
	if (isStatic)
		type = GL_STATIC_DRAW;

	glBufferData(GL_ARRAY_BUFFER, data.size() * sizeof(GLfloat), data.data(), type);
	if (OpenGL::error("glBufferData"))
		return;

	// tell OpenGL how it is formatted
	glVertexAttribPointer(0, dim, GL_FLOAT, GL_FALSE, 0, 0);
	if (OpenGL::error("glVertexAttribPointer"))
		return;

	glEnableVertexAttribArray(0);
	if (OpenGL::error("glEnableVertexAttribArray"))
		return;


	// unbind ourselves
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	// one last bit of tidying up
	numVertices = data.size() / dim;
	storage = nullptr;

}

// create a static VertexArray
VertexArray::VertexArray(uchar d, vector<GLfloat> data) {

	// slam on the brakes if fishy things are happening
	if ((d < 1) || (data.size() == 0))
		return;

	dim = d;		// assign variables and call our helper
	isStatic = true;
	initHelper(data);

}

// create a dynamic VertexArray
VertexArray::VertexArray(uchar d, uint count) {

	// slam on the brakes if fishy things are happening
	if ((d < 1) || (count < 1))
		return;

	dim = d;		// assign variables
	isStatic = false;

	// start padding out a vector
	vector<GLfloat> verticies;
	verticies.reserve(count);
	for (uint it = 0; it < count; it++)
		verticies.push_back(0.0);

	initHelper(verticies);

}

// what type of primitive do we represent?
bool VertexArray::setType(GLenum t, uchar s) {

	// branch, based on type
	switch (t) {

		// extra checks for patches
	case GL_PATCHES:

		if (s == 0)
			return false;

		patchSize = s;


		// fall through!

	case GL_POINTS:
	case GL_LINE_STRIP:
	case GL_LINE_LOOP:
	case GL_LINES:
	case GL_LINE_STRIP_ADJACENCY:
	case GL_LINES_ADJACENCY:
	case GL_TRIANGLE_STRIP:
	case GL_TRIANGLE_FAN:
	case GL_TRIANGLES:
	case GL_TRIANGLE_STRIP_ADJACENCY:
	case GL_TRIANGLES_ADJACENCY:

		prim = t;
		break;

	default:	// anything else is an error
		return false;

	}

	return true;
}

// map data into someplace accessible
bool VertexArray::map() {

	// can't map static data! can't map if mapped!
	if (isStatic || (storage != nullptr))
		return false;

	// otherwise, bind then map
	glBindVertexArray(id);
	if (OpenGL::error("glBindVertexArray")) {

		glBindVertexArray(0);
		return false;
	}

	glBindBuffer(GL_ARRAY_BUFFER, bufID);
	if (OpenGL::error("glBindBuffer")) {

		glBindVertexArray(0);
		return false;
	}


	storage = glMapBuffer(GL_ARRAY_BUFFER, GL_READ_WRITE);
	if (OpenGL::error("glMapBuffer")) {

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);
		return false;
	}

	return (storage != nullptr);

}

// tweak a single value (allow editing)
GLfloat& VertexArray::operator[](std::size_t index) {

	// throw an exception if static, unmapped, or too big
	if (isStatic)
		throw out_of_range("Vertex Array: reading from static!");
	if (storage == nullptr)
		throw out_of_range("Vertex Array: nothing mapped!");
	if (index >= numVertices)
		throw out_of_range("Vertex Array: asked for invalid index!");

	// otherwise, return what's there
	return ((GLfloat*)storage)[index];

}

// read a single value (editing disallowed)
const GLfloat& VertexArray::operator[](size_t index) const {

	// throw an exception if static, unmapped, or too big
	if (isStatic)
		throw out_of_range("Vertex Array: reading from static!");
	if (storage == nullptr)
		throw out_of_range("Vertex Array: nothing mapped!");
	if (index >= numVertices)
		throw out_of_range("Vertex Array: asked for invalid index!");

	// otherwise, return what's there
	return ((GLfloat*)storage)[index];

}

// release the data back to OpenGL
bool VertexArray::unmap() {

	// can't map static data! can't unmap if not mapped!
	if (isStatic || (storage == nullptr))
		return false;

	// otherwise, attempt to unmap and un-bind
	bool retVal = glUnmapBuffer(GL_ARRAY_BUFFER);	// this can error out in rare instances!
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	// if successful, flag that we're safe to draw
	if (retVal && !OpenGL::error("unmap"))
		storage = nullptr;

	return (storage == nullptr);

}

// draw the vertex array!
bool VertexArray::draw() {

	// don't draw if we're mapped or contain no verts
	if ((storage != nullptr) || (numVertices == 0))
		return false;

	// otherwise, bind, draw, and unbind
	glBindVertexArray(id);
	if (OpenGL::error("glBindVertexArray"))
		return false;

	if (prim == GL_PATCHES) {

		glPatchParameteri(GL_PATCH_VERTICES, patchSize);
		if (OpenGL::error("glPatchParameteri")) {

			glBindVertexArray(0);
			return false;
		}
	}

	glDrawArrays(prim, 0, numVertices);
	if (OpenGL::error("glDrawArrays"))
		return false;

	glBindVertexArray(0);

	return !OpenGL::error("VertexArray::draw() assert");

}
