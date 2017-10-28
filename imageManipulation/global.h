#ifndef GLOBAL_H
#define GLOBAL_H

/****************************************************************************
*
* DIFFERENCE, or a quick demo of loading images from a file. Uses the STB
*  library (https://github.com/nothings/stb).
*
* Author: Haysn Hornbeck.
*
*/

// ***** INCLUDES

// Note that it is crucial to add the lib files to the linker/input/additional dependencies
// Right click solution name, properties, linker, input, additional dependencies.
// In same tab, under Visual C++ also specify additional lib and include directories.


#include <GLEW/glew.h>
#include <glm/glm.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtx/matrix_transform_2D.hpp>
#include <glm/gtc/type_ptr.hpp>


#include <windows.h>
#include <GL/gl.h>

// This define clause was causing the inconsistent dll linkage errors.
// Most likely contains definitions of the same functions that the glew pointers are searching for?
//#define GLFW_INCLUDE_GLCOREARB
#define GL_GLEXT_PROTOTYPES
#include <GLFW/glfw3.h>


using namespace glm;





#include <cmath>
using std::fabs;
using std::isnan;
using std::log10;
using std::sqrt;

#include <cstddef>
using std::size_t;

//#include <unistd.h>	
// usleep
#undef max


#include <algorithm>
using std::sort;

#include <atomic>
using std::atomic;

#include <chrono>
using std::chrono::duration;
using std::chrono::duration_cast;
using std::chrono::high_resolution_clock;
using std::chrono::microseconds;
using std::chrono::steady_clock;
using std::chrono::time_point;
using seconds = duration<float>;
using std::chrono::nanoseconds;


#include <iostream>
using std::cerr;
using std::cout;
using std::endl;

#include <limits>
using std::numeric_limits;

#include <memory>
using std::make_shared;
using std::shared_ptr;

#include <mutex>
using std::mutex;

#include <string>
using std::string;

#include <thread>
using std::thread;

#include <vector>
using std::vector;

// handle STB in a source file, not here


#include <array>
using std::array;

#include <list>
using std::list;

#include <fstream>
using std::fstream;
using std::ifstream;

#include <map>
using std::map;

#include <random>
using std::mt19937;
using std::uniform_int_distribution;

#include <stdexcept>
using std::out_of_range;

#include <string>
using std::getline;
using std::string;



#include <stdio.h>
#include <iomanip>
#include <sstream>



// ***** DATA DECLARATIONS

typedef unsigned char uchar;
typedef unsigned int uint;
typedef unsigned long ulong;

// helpful for representing results
typedef struct DR {

	uint x;				// necessary for sorting
	uint y;

	float progress = 0.;
	double psnr;			// so we can be algorithmically lazy
	double mae;
	double rmse;

	// let it be sortable
	bool operator<(const DR& other) const;

} DiffResult;



// ***** CLASSES

// represent a pixel
class Pixel {
	// the components contained within
	vector<float> c;
	float maxAll;

public:
	// must fix the number of components from the get-go
	Pixel(uchar components);

	float r() const;	// handy shortcuts
	float g() const;
	float b() const;

	bool r(const float&);
	bool g(const float&);
	bool b(const float&);

	// workhorse functions
	bool set(const uchar index, const float& value);
	float get(const uchar index) const;

	ulong pixels() const;	// how many pixels?
	uint width() const;	//  and so on...
	uint height() const;
	uchar channels() const;

	float max() const;	// the maximal value

						// no square brackets, so we can properly update the max value

};

// represent a scanline
class Scanline {

	uint w = 0;		// width of the scanline
	uint c = 0;		// # of components

	vector<Pixel> data;	// store data

public:
	// must fix these from the get-go
	Scanline(uint width, uchar components);

	ulong pixels() const;	// how many pixels?
	uint width() const;	//  and so on...
	uint height() const;
	uchar channels() const;

	float max() const;	// the maximal value

						// access via square brackets; throws exceptions!
	Pixel& operator[](size_t index);
	const Pixel& operator[](size_t index) const;

};

// represent an image
class Image {

	uint w = 0;		// width, height, you get it
	uint h = 0;
	uint c = 0;

	vector<Scanline> data;	// store data

public:
	// must fix these from the get-go
	Image(uint width, uint height, uchar components);

	ulong pixels() const;	// how many pixels?
	uint width() const;	//  and so on...
	uint height() const;
	uchar channels() const;

	float max() const;	// the maximal value

						// access via square brackets; throws exceptions!
	Scanline& operator[](size_t index);
	const Scanline& operator[](size_t index) const;

};





class SimpleTexture;
class VertexArray;

// A java-ish container for program code
class Difference {

//	static vector<shared_ptr<Image>> imageVector;	// allow multiple comparisons
//	static vector<DiffResult> state;		// what are the results?
//	static mutex dataLock;				// protect the above

//	static atomic<int> activeThreads;		// how many loading routines are running?

//	int linearize(uint x, uint y);		// turn this into a linear index

//	static const double maxPrecisLoss;	// how much precision are we willing to lose?

public:
	// the ACTUAL main routine
	int run(const int argc, const char** argv);

	// load the given image
//	static void loadImage(const char*, uint index);


	static shared_ptr<SimpleTexture> loadImageDataIntoTexture(const char *, uint index);

	shared_ptr<VertexArray> vertexArray;	// a full-screen polygon


	vector<GLfloat> getVectorOfImageData(const char* file);


	// calculate the metrics
//	static void calcMetrics(uint x, uint y);

};







class Framebuffer;      // so we can declare friendship

class SimpleTexture {
	// so we can read SimpleTexture's private data from Framebuffer and ShaderProgram
	friend class Framebuffer;
	friend class ShaderProgram;

private:
	GLuint id = 0;          // named texture
	uint width;             // dimensions
	uint height;

	// how to handle sampling
	GLenum upsampling = GL_LINEAR;
	GLenum downsampling = GL_LINEAR_MIPMAP_LINEAR;
	GLenum wrapping = GL_REPEAT;
	GLuint sampler = 0;	// the named reference

	GLenum format;          // pixel format
							// what type of texture? 1D, 2D, RECTANGLE, ...
	GLenum type = GL_TEXTURE_2D;

	uint perPixelChan = 0;	// cache the number of channels per pixel
	bool hasStorage = false;// allocated storage space?

	bool loadSampler(GLuint);			// set up the proper sampling

public:
	SimpleTexture(uint width, uint height, GLenum format);
	~SimpleTexture();

	bool load(vector<float> data);	// load up the texture with external data
	bool load();				// internally allocate some space
	bool isLoaded() { return hasStorage && (perPixelChan != 0); }

	bool setDownsampler(GLenum);		// configure the samplers
	bool setUpsampler(GLenum);
	bool setWrapping(GLenum);

	uint getWidth() const { return width; }
	uint getHeight() const { return height; }

};

// Represent a framebuffer
class Framebuffer {

private:
	GLuint id = 0;		// the named ID of this framebuffer
	shared_ptr<SimpleTexture> tex = nullptr;

public:
	// standard constructors/destructors
	Framebuffer(shared_ptr<SimpleTexture> = nullptr);
	~Framebuffer();

	// associate a texture with this buffer
	void setTexture(shared_ptr<SimpleTexture>);
	bool bind();				// make us THE output buffer
	static void unbind();			// restore the default buffer

};

// Handle GLSL shaders
class ShaderProgram {

private:
	GLint id = -1;
	map<GLenum, GLuint> shaders;	// use a map to track all valid shaders
	bool valid = false;		// once valid, read-only

	uchar textureSlot = 0;		// allows automatic tracking of texture slots

public:
	ShaderProgram();				// grab a named program
	~ShaderProgram();				// free up said program

	// Attach shader by path to shader source
	bool addShader(string path, GLenum type);

									// attach a shader. False on failure/read-only
	bool attachShader(string data, GLenum type);
	bool hasShader(GLenum type);		// do we have this shader saved?
	bool link();				// link this collection of shaders
	bool isValid() { return valid; }	// did we link? And have vertex/fragment shaders?

	bool setInt(string, GLint);		// set the value of a Uniform
	bool setFloat(string, GLfloat);
	bool setVec2(string, GLfloat, GLfloat);
	bool setMat4(string, mat4);

	// attach an input texture at the given slot
	bool setTexture(string, shared_ptr<SimpleTexture>);

	bool bind();				// make this program active

	static void unbind();			// detach any active program
									// read in a shader from a file
	static string readShader(string file);

};

// A simple class for vertex data
class VertexArray {

private:
	GLuint id = 0;			// the named vertex array
	GLuint bufID = 0;			//  and the buffer object underneath

	uchar dim = 0;			// number of dimensions
	bool isStatic = true;			// can we edit these?
	uint numVertices = 0;			// how many verticies? Also a canary
	void* storage = nullptr;		// true = lock from drawing

	GLenum prim = GL_TRIANGLES;		// the type of primitive to draw
	uchar patchSize = 0;			// if using patches, how many?

	void initHelper(vector<GLfloat>&);

public:
	VertexArray(uchar dim, vector<GLfloat> data);	// static storage
	VertexArray(uchar dim, uint count);		// dynamic storage

	bool setType(GLenum type, uchar size = 0);	// set the type of primitive

	bool map();				// allow read/write access
							// if mapped, read (if not or OOB, throws "out_of_range")
	GLfloat& operator[](size_t index);
	const GLfloat& operator[](size_t index) const;
	bool unmap();				// finish read/write access

	bool draw();				// draw this VA (false if mapped)

};




// handy for queuing keyboard actions
typedef struct {

	int key;
	int scancode;
	int action;
	int mods;

} keyAction;



// A java-ish container for the program logic
class GOL {

public:

	bool initGLFW();			// set up GLFW3 for use
	bool initShaders();			//  you get the idea
	bool initGeometry();
	bool initTextures();			// generate a specific board layout


	void render();				// the main rendering loop
	void keyInput();			// process key input
	void iterate();				// maybe do some iterations of the board
	void iterateOne();			// do one iteration, no matter what

	shared_ptr<SimpleTexture> texture;
	Framebuffer frameBuffer;			// for offline rendering


private:
	bool live = true;			// should we stay alive?

	time_point<steady_clock> last;		// store our last time
	bool active = true;		// should we run?
	bool turbo = true;		//  should we run quickly?
	static const float tic;			// how long until we maybe render again?
	float delayTics = 1.0;			// rate-limit the simulation or render rate
	float elapsed = 0.0;			// how many seconds have elapsed?

	float zoom = 1.0;			// allow zooming in
	float maxZoom = 1.0 / 16.0;		//  cap the zoom limit

									// store some board presets
	static const vector<vector<vector<float>>> presets;

	shared_ptr<VertexArray> vertexArray;	// a full-screen polygon
	ShaderProgram CurveDrawingProgram;		// handle the shaders
	ShaderProgram TextureProgram;
	ShaderProgram ImageDisplayProgram;

	shared_ptr<SimpleTexture> bufferDst;	// the two board buffers
	shared_ptr<SimpleTexture> bufferSrc;


	mt19937 RNG;				// contains a RNG state ...
	uniform_int_distribution<short> dist;	// ... and distribution for creating random scenes

	GLFWwindow* window = nullptr;		// a handle to the active context
	int width = 1024;			// cache the window dimensions
	int height = 768;
	static list<keyAction> keyQueue;	// store incoming key presses
	static mutex keyQueueLock;		//  ensure we don't trip over ourselves

	bool terminate(string message);	// a helper to ease quitting on error
	shared_ptr<SimpleTexture> genBoard(uint);
	void cleanup();				// clean up after the render loop is done


	static const string vertexShader;	// strings to represent the shaders!
	static const string CurveDrawingShader;
	static const string TextureShader;
	static const string FragmentShader;

public:
	GOL();					// initialize the VA and textures
	int run(int argc, const char** argv);	// the main routine to run
											// handle GLFW error callbacks
	static void errorCallback(int, const char*);
	// process key input
	static void keyCallback(GLFWwindow*, int, int, int, int);

};




// helper routines for low-level OpenGL functions
class OpenGL {

public:
	static bool error(string tag);	// was there an error at "tag"?

	static void finish() { glFinish(); }	// pause until all commands finished

};





// ***** STATIC FUNCTIONS

int main(const int argc, const char** argv);		// the main routine

#endif
